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
  \file terrama2/services/analysis/core/PythonInterpreter.cpp

  \brief Manages the communication of Python and C.

  \author Paulo R. M. Oliveira
*/



#include "Operator.hpp"
#include "../Operator.hpp"
#include "../../Utils.hpp"
#include "../../ContextManager.hpp"


#include "../../Exception.hpp"
#include "../../../../../core/data-model/DataSetDcp.hpp"
#include "../../../../../core/data-model/Filter.hpp"
#include "../../../../../core/data-access/SynchronizedDataSet.hpp"
#include "../../../../../core/Shared.hpp"
#include "../../PythonUtils.hpp"


// TerraLib
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/vp/BufferMemory.h>
#include <terralib/geometry/MultiPolygon.h>


double terrama2::services::analysis::core::dcp::history::operatorImpl(StatisticOperation statisticOperation,
    const std::string& dataSeriesName,
    const std::string& attribute,
    const std::string& dateFilter,
    boost::python::list ids)
{
  OperatorCache cache;
  terrama2::services::analysis::core::python::readInfoFromDict(cache);
  auto context = ContextManager::getInstance().getMonitoredObjectContext(cache.analysisHashCode);
  // Inside Py_BEGIN_ALLOW_THREADS it's not allowed to return any value because it doesn' have the interpreter lock.
  // In case an exception is thrown, we need to set this boolean. Once the code left the lock is acquired we should return NAN.
  bool exceptionOccurred = false;


  try
  {
    // In case an error has already occurred, there is nothing to be done
    if(!context->getErrors().empty())
    {
      return NAN;
    }

    std::vector<DataSetId> vecDCPIds;
    terrama2::services::analysis::core::python::pythonToVector<DataSetId>(ids, vecDCPIds);

    if(vecDCPIds.empty())
    {
      return NAN;
    }

    bool hasData = false;

    AnalysisPtr analysis = context->getAnalysis();

    auto dataManagerPtr = context->getDataManager().lock();
    if(!dataManagerPtr)
    {
      QString errMsg(QObject::tr("Invalid data manager."));
      throw terrama2::core::InvalidDataManagerException() << terrama2::ErrorDescription(errMsg);
    }

    auto moDsContext = context->getMonitoredObjectContextDataSeries(dataManagerPtr);
    if(!moDsContext)
    {
      QString errMsg(QObject::tr("Could not recover monitored object dataset."));
      throw InvalidDataSeriesException() << terrama2::ErrorDescription(errMsg);
    }

    auto geom = moDsContext->series.syncDataSet->getGeometry(cache.index, moDsContext->geometryPos);
    if(!geom.get())
    {
      QString errMsg(QObject::tr("Could not recover monitored object geometry."));
      throw InvalidDataSeriesException() << terrama2::ErrorDescription(errMsg);
    }



    std::vector<double> values;

    // Frees the GIL, from now on can not use the interpreter
    Py_BEGIN_ALLOW_THREADS
    try
    {

      std::shared_ptr <ContextDataSeries> contextDataSeries;
      auto dataSeries = dataManagerPtr->findDataSeries(analysis->id, dataSeriesName);

      if(!dataSeries)
      {
        QString errMsg(QObject::tr("Could not find a data series with the given name: %1"));
        errMsg = errMsg.arg(QString::fromStdString(dataSeriesName));
        throw InvalidDataSeriesException() << terrama2::ErrorDescription(errMsg);
      }

      context->addDCPDataSeries(dataSeries, dateFilter, false);


      for(DataSetId dcpId : vecDCPIds)
      {
        for(auto dataset : dataSeries->datasetList)
        {
          if(dataset->id != dcpId)
            continue;

          contextDataSeries = context->getContextDataset(dataset->id, dateFilter);

          terrama2::core::DataSetDcpPtr dcpDataset = std::dynamic_pointer_cast<const terrama2::core::DataSetDcp>(
                  dataset);
          if(!dcpDataset)
          {
            QString errMsg(QObject::tr("Could not recover DCP dataset: %1.").arg(dataset->id));
            throw InvalidDataSetException() << terrama2::ErrorDescription(errMsg);
          }

          if(dcpDataset->position == nullptr)
          {
            QString errMsg(QObject::tr("DCP dataset does not have a valid position."));
            throw InvalidDataSetException() << terrama2::ErrorDescription(errMsg);
          }

          auto syncDs = contextDataSeries->series.syncDataSet;
          int attributeType = 0;

          if(!attribute.empty())
          {
            auto property = contextDataSeries->series.teDataSetType->getProperty(attribute);

            // only operation COUNT can be done without attribute.
            if(!property && statisticOperation != StatisticOperation::COUNT)
            {
              QString errMsg(QObject::tr("Invalid attribute name"));
              throw InvalidParameterException() << terrama2::ErrorDescription(errMsg);
            }
            attributeType = property->getType();
          }


          if(syncDs->size() == 0)
            continue;


          for(unsigned int i = 0; i < syncDs->size(); ++i)
          {
            try
            {
              if(!attribute.empty() && !syncDs->isNull(i, attribute))
              {
                hasData = true;
                double value = getValue(syncDs, attribute, i, attributeType);
                values.push_back(value);
              }
            }
            catch(...)
            {
              // In case the DCP doesn't have the specified column
              continue;
            }
          }


        }
      }

      calculateStatistics(values, cache);

    }
    catch(const terrama2::Exception& e)
    {
      context->addError(boost::get_error_info<terrama2::ErrorDescription>(e)->toStdString());
      exceptionOccurred = true;
    }
    catch(const std::exception& e)
    {
      context->addError(e.what());
      exceptionOccurred = true;
    }
    catch(...)
    {
      QString errMsg = QObject::tr("An unknown exception occurred.");
      context->addError(errMsg.toStdString());
      exceptionOccurred = true;
    }


    // All operations are done, acquires the GIL and set the return value
    Py_END_ALLOW_THREADS

    if(values.empty() && statisticOperation != StatisticOperation::COUNT)
      return NAN;

    if(exceptionOccurred)
      return NAN;

    if(!hasData && statisticOperation != StatisticOperation::COUNT)
    {
      return NAN;
    }

    double x = getOperationResult(cache, statisticOperation);
    return x;
  }
  catch(const terrama2::Exception& e)
  {
    context->addError(boost::get_error_info<terrama2::ErrorDescription>(e)->toStdString());
    return NAN;
  }
  catch(const std::exception& e)
  {
    context->addError(e.what());
    return NAN;
  }
  catch(...)
  {
    QString errMsg = QObject::tr("An unknown exception occurred.");
    context->addError(errMsg.toStdString());
    return NAN;
  }
}


double terrama2::services::analysis::core::dcp::history::sum(const std::string& dataSeriesName,
    const std::string& attribute,
    const std::string& dateFilter,
    boost::python::list ids)
{
  return operatorImpl(StatisticOperation::SUM, dataSeriesName, attribute, dateFilter, ids);
}

double terrama2::services::analysis::core::dcp::history::mean(const std::string& dataSeriesName,
    const std::string& attribute,
    const std::string& dateFilter,
    boost::python::list ids)
{
  return operatorImpl(StatisticOperation::MEAN, dataSeriesName, attribute, dateFilter, ids);
}

double terrama2::services::analysis::core::dcp::history::min(const std::string& dataSeriesName,
   const std::string& attribute,
   const std::string& dateFilter,
   boost::python::list ids)
{
  return operatorImpl(StatisticOperation::MIN, dataSeriesName, attribute, dateFilter, ids);
}

double terrama2::services::analysis::core::dcp::history::max(const std::string& dataSeriesName,
   const std::string& attribute,
   const std::string& dateFilter,
   boost::python::list ids)
{
  return operatorImpl(StatisticOperation::MAX, dataSeriesName, attribute, dateFilter, ids);
}

double terrama2::services::analysis::core::dcp::history::median(const std::string& dataSeriesName,
    const std::string& attribute,
    const std::string& dateFilter,
    boost::python::list ids)
{
  return operatorImpl(StatisticOperation::MEDIAN, dataSeriesName, attribute, dateFilter, ids);
}

double terrama2::services::analysis::core::dcp::history::standardDeviation(const std::string& dataSeriesName,
   const std::string& attribute,
   const std::string& dateFilter,
   boost::python::list ids)
{
  return operatorImpl(StatisticOperation::STANDARD_DEVIATION, dataSeriesName, attribute, dateFilter, ids);
}

double terrama2::services::analysis::core::dcp::history::variance(const std::string& dataSeriesName,
    const std::string& attribute,
    const std::string& dateFilter,
    boost::python::list ids)
{
  return operatorImpl(StatisticOperation::VARIANCE, dataSeriesName, attribute, dateFilter, ids);
}
