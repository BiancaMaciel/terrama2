/*
  Copyright (C) 2007 National Institute For Space Research (INPE) - Brazil.

  This file is part of TerraMA2 - a free and open source computational
  platform for analysis, monitoring, and alert of geo-environmental extremes.

  TerraMA2 is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License,
  or (at your option) any later version.

  TerraMA2 is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with TerraMA2. See LICENSE. If not, write to
  TerraMA2 Team at <terrama2-team@dpi.inpe.br>.
*/

/*!
  \file terrama2/services/alert/core/RunAlert.hpp

  \brief

  \author Jano Simas
*/

#include "../../../core/utility/Utils.hpp"
#include "../../../core/utility/Logger.hpp"
#include "../../../core/data-model/DataSeries.hpp"
#include "../../../core/data-access/DataAccessor.hpp"
#include "../../../core/data-access/DataStorager.hpp"
#include "../../../core/utility/DataAccessorFactory.hpp"
#include "../../../core/utility/DataStoragerFactory.hpp"

#include "RunAlert.hpp"
#include "Alert.hpp"
#include "Report.hpp"
#include "ReportFactory.hpp"

#include <QObject>

#include <limits>

#include <terralib/memory/DataSet.h>
#include <terralib/memory/DataSetItem.h>
#include <terralib/datatype/SimpleProperty.h>
#include <terralib/datatype/DateTimeProperty.h>
#include <terralib/datatype/TimeInstantTZ.h>
#include <terralib/dataaccess/dataset/ForeignKey.h>

void terrama2::services::alert::core::runAlert(std::pair<AlertId, std::shared_ptr<te::dt::TimeInstantTZ> > alertInfo,
    std::shared_ptr< AlertLogger > logger,
    std::weak_ptr<DataManager> weakDataManager)
{
  auto dataManager = weakDataManager.lock();
  if(!dataManager.get())
  {
    TERRAMA2_LOG_ERROR() << QObject::tr("Unable to access DataManager");
    return;
  }

  try
  {
    auto alertId = alertInfo.first;
    RegisterId logId = 0;
    if(logger.get())
      logId = logger->start(alertId);

    TERRAMA2_LOG_DEBUG() << QObject::tr("Starting alert generation");

    //////////////////////////////////////////////////////////
    //  aquiring metadata
    auto lock = dataManager->getLock();

    auto alertPtr = dataManager->findAlert(alertId);

    // input data
    auto inputDataSeries = dataManager->findDataSeries(alertPtr->risk.dataSeriesId);
    auto inputDataProvider = dataManager->findDataProvider(inputDataSeries->dataProviderId);

    //temp struct for additional data
    struct TempAdditionalData
    {
      terrama2::core::DataSeriesPtr dataSeries;
      terrama2::core::DataProviderPtr dataProvider;

      AdditionalData additionalData;
      std::map<terrama2::core::DataSetPtr, terrama2::core::DataSetSeries> dataMap;
    };
    //retrieve additional data
    std::vector<TempAdditionalData> additionalDataVector;
    for(auto additionalData : alertPtr->additionalDataVector)
    {
      TempAdditionalData tempData;
      tempData.additionalData = additionalData;

      tempData.dataSeries = dataManager->findDataSeries(additionalData.id);
      tempData.dataProvider = dataManager->findDataProvider(tempData.dataSeries->dataProviderId);

      additionalDataVector.push_back(tempData);
    }

    // dataManager no longer in use
    lock.unlock();

    /////////////////////////////////////////////////////////////////////////
    // analysing data

    auto filter = alertPtr->filter;
    auto risk = alertPtr->risk;

    auto dataAccessor = terrama2::core::DataAccessorFactory::getInstance().make(inputDataProvider, inputDataSeries);
    auto dataMap = dataAccessor->getSeries(filter);
    if(dataMap.empty())
    {
      if(logger.get())
        logger->done(nullptr, logId);
      TERRAMA2_LOG_WARNING() << QObject::tr("No data to available.");
      return;
    }

    for(auto iter = additionalDataVector.begin(); iter != additionalDataVector.end();)
    {
      auto additionalData = *iter;

      auto dataAccessor = terrama2::core::DataAccessorFactory::getInstance().make(additionalData.dataProvider, additionalData.dataSeries);
      additionalData.dataMap = dataAccessor->getSeries(filter);
      if(additionalData.dataMap.empty())
      {
        TERRAMA2_LOG_WARNING() << QObject::tr("No data to available in dataseries %1.").arg(additionalData.additionalData.id);
        //erase returns the next valid position
        iter = additionalDataVector.erase(iter);
        continue;
      }

      ++iter;


    }


    for(const auto& data : dataMap)
    {
      auto dataset = data.first;
      auto dataSeries = data.second;

      const std::string dataSetAlertName = "alert_"+std::to_string(alertPtr->id)+"_"+std::to_string(dataset->id);
      auto alertDataSetType = std::make_shared<te::da::DataSetType>(dataSetAlertName);

      const std::string riskLevelProperty = "risk_level";
      te::dt::SimpleProperty* riskLevelProp = new te::dt::SimpleProperty(riskLevelProperty, te::dt::INT32_TYPE);
      alertDataSetType->add(riskLevelProp);

      const std::string riskNameProperty = "risk_name";
      te::dt::SimpleProperty* riskNameProp = new te::dt::SimpleProperty(riskNameProperty, te::dt::STRING_TYPE);
      alertDataSetType->add(riskNameProp);

      auto teDataset = dataSeries.syncDataSet->dataset();
      auto dataSetType = dataSeries.teDataSetType;

      auto idProperty = dataSetType->getProperty(getIdentifierPropertyName(dataset, inputDataSeries));
      auto fkProperty = idProperty->clone();
      fkProperty->setName(idProperty->getName()+"_fk");
      alertDataSetType->add(fkProperty);

      auto alertDataSet = std::make_shared<te::mem::DataSet>(alertDataSetType.get());



      auto pos = dataSetType->getPropertyPosition(risk.attribute);
      if(pos == std::numeric_limits<decltype(pos)>::max())
      {
        //TODO: warning
        continue;
      }

      teDataset->moveBeforeFirst();
      alertDataSet->moveBeforeFirst();

      std::function<std::tuple<int, std::string>(size_t pos)> getRisk = terrama2::services::alert::core::createGetRiskFunction(risk, teDataset);

      while(teDataset->moveNext())
      {
        alertDataSet->moveNext();

        te::mem::DataSetItem* item = new te::mem::DataSetItem(alertDataSet.get());
        //fk value
        item->setValue(fkProperty->getName(), teDataset->getValue(idProperty->getName())->clone());

        // risk level
        int riskLevel = 0;
        std::string riskName;
        std::tie(riskLevel, riskName) = getRisk(pos);
        item->setInt32(riskLevelProperty, riskLevel);
        item->setString(riskNameProperty, riskName);

        alertDataSet->add(item);
      }

      auto& factory = ReportFactory::getInstance();
      auto report = factory.make(alertPtr->reportMetadata.at(ReportTags::TYPE), alertPtr->reportMetadata);
      report->process(alertPtr, dataset, alertInfo.second, alertDataSet);
    }

    if(logger.get())
      logger->done(alertInfo.second, logId);
  }
  catch(const terrama2::Exception& e )
  {
    TERRAMA2_LOG_ERROR() << boost::get_error_info<terrama2::ErrorDescription>(e)->toStdString();
    throw;
  }
  catch(boost::exception& e)
  {
    TERRAMA2_LOG_ERROR() << boost::diagnostic_information(e);
  }
  catch(std::exception& e)
  {
    QString errMsg(e.what());
    TERRAMA2_LOG_ERROR() << errMsg;
  }
  catch(...)
  {
    TERRAMA2_LOG_ERROR() << QObject::tr("Unknown exception");
  }
}

std::function<std::tuple<int, std::string>(size_t pos)> terrama2::services::alert::core::createGetRiskFunction(terrama2::core::DataSeriesRisk risk, std::shared_ptr<te::da::DataSet> teDataSet)
{
  if(risk.riskType == terrama2::core::RiskType::NUMERIC)
  {
    return [risk, teDataSet](size_t pos)
    {
      const auto& value = teDataSet->getDouble(pos);
      return risk.riskLevel(value);
    };
  }
  else
  {
    return [risk, teDataSet](size_t pos)
    {
      const auto& value = teDataSet->getString(pos);
      return risk.riskLevel(value);
    };
  }
}

std::string terrama2::services::alert::core::getIdentifierPropertyName(terrama2::core::DataSetPtr dataSet, terrama2::core::DataSeriesPtr dataSeries)
{
  return getProperty(dataSet, dataSeries, "identifier");
}
