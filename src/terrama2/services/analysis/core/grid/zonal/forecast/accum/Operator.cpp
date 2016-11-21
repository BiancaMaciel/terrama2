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
  \file terrama2/services/analysis/core/grid/zonal/Operator.cpp

  \brief Contains grid zonal analysis operators.

  \author Jano Simas
*/



#include "Operator.hpp"
#include "../prec/Operator.hpp"
#include "../../Operator.hpp"
#include "../../../../utility/Utils.hpp"
#include "../../../../utility/Verify.hpp"
#include "../../../../python/PythonInterpreter.hpp"
#include "../../../../ContextManager.hpp"
#include "../../../../MonitoredObjectContext.hpp"

#include "../../../../../../../core/data-model/Filter.hpp"
#include "../../../../../../../core/utility/TimeUtils.hpp"
#include "../../../../../../../core/utility/Logger.hpp"

// TerraLib
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/vp/BufferMemory.h>
#include <terralib/geometry/MultiPolygon.h>
#include <terralib/geometry/Utils.h>
#include <terralib/raster/PositionIterator.h>

double terrama2::services::analysis::core::grid::zonal::forecast::accum::getAbsTimeFromString(const std::string& timeStr)
{
  auto begin = timeStr.find_first_of("0123456789");
  auto end = timeStr.find_last_of("0123456789");

  if(begin == std::string::npos || end == std::string::npos)
    return NAN;

  auto time = timeStr.substr(begin, end-begin);
  return std::stod(time);
}

double terrama2::services::analysis::core::grid::zonal::forecast::accum::operatorImpl(terrama2::services::analysis::core::StatisticOperation statisticOperation,
    const std::string& dataSeriesName, const std::string& dateDiscardBefore, const std::string& dateDiscardAfter, const size_t band, terrama2::services::analysis::core::Buffer buffer)
{
  OperatorCache cache;
  terrama2::services::analysis::core::python::readInfoFromDict(cache);
  // After the operator lock is released it's not allowed to return any value because it doesn' have the interpreter lock.
  // In case an exception is thrown, we need to set this boolean. Once the code left the lock is acquired we should return NAN.
  bool exceptionOccurred = false;

  auto& contextManager = ContextManager::getInstance();
  auto analysis = cache.analysisPtr;

  try
  {
    terrama2::core::verify::analysisMonitoredObject(analysis);
  }
  catch(const terrama2::core::VerifyException&)
  {
    contextManager.addError(cache.analysisHashCode, QObject::tr("Use of invalid operator for analysis %1.").arg(analysis->id).toStdString());
    return NAN;
  }

  terrama2::services::analysis::core::MonitoredObjectContextPtr context;
  try
  {
    context = ContextManager::getInstance().getMonitoredObjectContext(cache.analysisHashCode);
  }
  catch(const terrama2::Exception& e)
  {
    TERRAMA2_LOG_ERROR() << boost::get_error_info<terrama2::ErrorDescription>(e)->toStdString();
    return NAN;
  }


  try
  {
    // In case an error has already occurred, there is nothing to be done
    if(!context->getErrors().empty())
      return NAN;

    bool hasData = false;

    auto dataManagerPtr = context->getDataManager().lock();
    if(!dataManagerPtr)
    {
      QString errMsg(QObject::tr("Invalid data manager."));
      throw terrama2::core::InvalidDataManagerException() << terrama2::ErrorDescription(errMsg);
    }

    std::shared_ptr<ContextDataSeries> moDsContext = context->getMonitoredObjectContextDataSeries(dataManagerPtr);
    if(!moDsContext)
    {
      QString errMsg(QObject::tr("Could not recover monitored object data series."));
      throw InvalidDataSeriesException() << terrama2::ErrorDescription(errMsg);
    }

    if(moDsContext->series.syncDataSet->size() == 0)
    {
      QString errMsg(QObject::tr("Could not recover monitored object data series."));
      throw InvalidDataSeriesException() << terrama2::ErrorDescription(errMsg);
    }

    auto moGeom = moDsContext->series.syncDataSet->getGeometry(cache.index, moDsContext->geometryPos);
    if(!moGeom.get())
    {
      QString errMsg(QObject::tr("Could not recover monitored object geometry."));
      throw InvalidDataSetException() << terrama2::ErrorDescription(errMsg);
    }
    auto geomResult = createBuffer(buffer, moGeom);

    auto dataSeries = context->findDataSeries(dataSeriesName);
    if(!dataSeries)
    {
      QString errMsg(QObject::tr("Could not find a data series with the given name: %1"));
      errMsg = errMsg.arg(QString::fromStdString(dataSeriesName));
      throw InvalidDataSeriesException() << terrama2::ErrorDescription(errMsg);
    }

    /////////////////////////////////////////////////////////////////
    //map of sum of values for each pixel
    std::unordered_map<std::pair<int, int>, std::pair<double, int>, boost::hash<std::pair<int, int>>  > valuesMap;

    auto datasets = dataSeries->datasetList;
    for(const auto& dataset : datasets)
    {
      auto rasterList = context->getRasterList(dataSeries, dataset->id, dateDiscardBefore, dateDiscardAfter);
      //sanity check, if no date range only the last raster should be returned
      if(dateDiscardBefore.empty() && rasterList.size() > 1)
      {
        QString errMsg(QObject::tr("Invalid list of raster for dataset: %1").arg(dataset->id));
        throw terrama2::InvalidArgumentException() << terrama2::ErrorDescription(errMsg);
      }

      if(rasterList.empty())
      {
        QString errMsg(QObject::tr("Invalid raster for dataset: %1").arg(dataset->id));
        throw terrama2::InvalidArgumentException() << terrama2::ErrorDescription(errMsg);
      }

      auto firstRaster = rasterList.front();

      //no intersection between the raster and the object geometry
      if(!firstRaster->getExtent()->intersects(*geomResult->getMBR()))
        continue;

      geomResult->transform(firstRaster->getSRID());
      prec::appendValues(rasterList, band, geomResult.get() , valuesMap);

      if(!valuesMap.empty())
      {
        hasData = true;
        break;
      }
    }

    if(exceptionOccurred)
      return NAN;

    if(!hasData && statisticOperation != StatisticOperation::COUNT)
    {
      return NAN;
    }
    std::vector<double> values;
    values.reserve(valuesMap.size());

    for(const auto& pair : valuesMap)
      values.push_back(pair.second.first);

    terrama2::services::analysis::core::calculateStatistics(values, cache);
    return terrama2::services::analysis::core::getOperationResult(cache, statisticOperation);
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

double terrama2::services::analysis::core::grid::zonal::forecast::accum::count(const std::string& dataSeriesName, const std::string& dateDiscardBefore, terrama2::services::analysis::core::Buffer buffer)
{
  return operatorImpl(StatisticOperation::COUNT, dataSeriesName, dateDiscardBefore, "", 0, buffer);
}


double terrama2::services::analysis::core::grid::zonal::forecast::accum::min(const std::string& dataSeriesName, const std::string& dateDiscardBefore, const size_t band, terrama2::services::analysis::core::Buffer buffer)
{
  return operatorImpl(StatisticOperation::MIN, dataSeriesName, dateDiscardBefore, "", band, buffer);
}

double terrama2::services::analysis::core::grid::zonal::forecast::accum::max(const std::string& dataSeriesName, const std::string& dateDiscardBefore, const size_t band, terrama2::services::analysis::core::Buffer buffer)
{
  return operatorImpl(StatisticOperation::MAX, dataSeriesName, dateDiscardBefore, "", band, buffer);
}

double terrama2::services::analysis::core::grid::zonal::forecast::accum::mean(const std::string& dataSeriesName, const std::string& dateDiscardBefore, const size_t band, terrama2::services::analysis::core::Buffer buffer)
{
  return operatorImpl(StatisticOperation::MEAN, dataSeriesName, dateDiscardBefore, "", band, buffer);
}

double terrama2::services::analysis::core::grid::zonal::forecast::accum::median(const std::string& dataSeriesName, const std::string& dateDiscardBefore, const size_t band, terrama2::services::analysis::core::Buffer buffer)
{
  return operatorImpl(StatisticOperation::MEDIAN, dataSeriesName, dateDiscardBefore, "", band, buffer);
}

double terrama2::services::analysis::core::grid::zonal::forecast::accum::standardDeviation(const std::string& dataSeriesName, const std::string& dateDiscardBefore, const size_t band, terrama2::services::analysis::core::Buffer buffer)
{
  return operatorImpl(StatisticOperation::STANDARD_DEVIATION, dataSeriesName, dateDiscardBefore, "", band, buffer);
}

double terrama2::services::analysis::core::grid::zonal::forecast::accum::variance(const std::string& dataSeriesName, const std::string& dateDiscardBefore, const size_t band, terrama2::services::analysis::core::Buffer buffer)
{
  return operatorImpl(StatisticOperation::VARIANCE, dataSeriesName, dateDiscardBefore, "", band, buffer);
}

double terrama2::services::analysis::core::grid::zonal::forecast::accum::sum(const std::string& dataSeriesName, const std::string& dateDiscardBefore, const size_t band, terrama2::services::analysis::core::Buffer buffer)
{
  return operatorImpl(StatisticOperation::SUM, dataSeriesName, dateDiscardBefore, "", band, buffer);
}
