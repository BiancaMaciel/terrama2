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
  \file terrama2/services/analysis/core/occurrence/Operator.cpp

  \brief Contains occurrence analysis operators.

  \author Paulo R. M. Oliveira
*/



#include "Operator.hpp"

#include <QTextStream>

#include "../../../../core/utility/Logger.hpp"
#include "../../../../core/data-model/Filter.hpp"

// TerraLib
#include <terralib/dataaccess/utils/Utils.h>
#include <terralib/vp/BufferMemory.h>
#include <terralib/geometry/MultiPolygon.h>
#include <terralib/geometry/Utils.h>

double terrama2::services::analysis::core::occurrence::operatorImpl(StatisticOperation statisticOperation,
                                                                    const std::string& dataSeriesName,
                                                                    Buffer buffer,
                                                                    const std::string& dateFilter,
                                                                    Buffer aggregationBuffer,
                                                                    const std::string& attribute,
                                                                    StatisticOperation aggregationStatisticOperation,
                                                                    const std::string& restriction)
{
  OperatorCache cache;
  readInfoFromDict(cache);

  bool hasData = false;

  auto dataManagerPtr = Context::getInstance().getDataManager().lock();
  if(!dataManagerPtr)
  {
    QString msg(QObject::tr("Invalid data manager."));
    TERRAMA2_LOG_ERROR() << msg;
    return NAN;
  }

  Analysis analysis = Context::getInstance().getAnalysis(cache.analysisId);

  std::shared_ptr<ContextDataSeries> moDsContext = getMonitoredObjectContextDataSeries(analysis, dataManagerPtr);
  if(!moDsContext)
  {
    QString errMsg(QObject::tr("Analysis: %1 -> Could not recover monitored object dataset."));
    errMsg = errMsg.arg(cache.analysisId);
    TERRAMA2_LOG_ERROR() << errMsg;
    return NAN;
  }

  if(moDsContext->series.syncDataSet->size() == 0)
  {
    QString errMsg(QObject::tr("Analysis: %1 -> Could not recover monitored object dataset."));
    errMsg = errMsg.arg(cache.analysisId);
    TERRAMA2_LOG_ERROR() << errMsg;
    return NAN;
  }

  auto moEnvelope = moDsContext->series.syncDataSet->getExtent(moDsContext->geometryPos);
  auto firstOccurrence = moDsContext->series.syncDataSet->getGeometry(0, moDsContext->geometryPos);
  int srid = firstOccurrence->getSRID();

  std::shared_ptr<te::gm::Geometry> geomEnvelope(te::gm::GetGeomFromEnvelope(moEnvelope.get(), srid));

  auto moGeom = moDsContext->series.syncDataSet->getGeometry(cache.index, moDsContext->geometryPos);
  if(!moGeom.get())
  {
    QString errMsg(QObject::tr("Analysis: %1 -> Could not recover monitored object geometry."));
    errMsg = errMsg.arg(cache.analysisId);
    TERRAMA2_LOG_ERROR() << errMsg;
    return NAN;
  }


  // Save thread state before entering multi-thread zone

  Py_BEGIN_ALLOW_THREADS

    std::shared_ptr<ContextDataSeries> contextDataSeries;

    try
    {

      auto dataSeries = dataManagerPtr->findDataSeries(analysis.id, dataSeriesName);

      if(!dataSeries)
      {
        QString errMsg(QObject::tr("Analysis: %1 -> Could not find a data series with the given name: %2"));
        errMsg = errMsg.arg(analysis.id);
        errMsg = errMsg.arg(QString::fromStdString(dataSeriesName));
        TERRAMA2_LOG_ERROR() << errMsg;
        return NAN;
      }


      Context::getInstance().addDataSeries(analysis.hashCode(), dataSeries, geomEnvelope, dateFilter, true);

      auto datasets = dataSeries->datasetList;

      for(auto dataset : datasets)
      {

        contextDataSeries = Context::getInstance().getContextDataset(analysis.hashCode(), dataset->id, dateFilter);
        if(!contextDataSeries)
        {
          continue;
        }


        std::vector<uint64_t> indexes;
        uint32_t countValues = 0;
        terrama2::core::SynchronizedDataSetPtr syncDs = contextDataSeries->series.syncDataSet;

        if(syncDs->size() == 0)
        {
          continue;
        }
        else
        {
          auto geomResult = createBuffer(buffer, moGeom);

          // Converts the monitored object to the same srid of the occurrences
          auto firstOccurrence = syncDs->getGeometry(0, contextDataSeries->geometryPos);
          geomResult->transform(firstOccurrence->getSRID());

          // Searchs in the spatial index for the occurrences that intersects the monitored object box
          contextDataSeries->rtree.search(*geomResult->getMBR(), indexes);


          std::vector<double> values;

          int attributeType = 0;
          if(!attribute.empty())
          {
            auto property = contextDataSeries->series.teDataSetType->getProperty(attribute);

            // only operation COUNT can be done without attribute.
            if(!property && statisticOperation != COUNT)
            {
              QString errMsg(QObject::tr("Analysis: %1 -> Invalid attribute name"));
              errMsg = errMsg.arg(cache.analysisId);
              TERRAMA2_LOG_ERROR() << errMsg;
              return NAN;
            }
            attributeType = property->getType();
          }

          if(aggregationStatisticOperation != INVALID)
          {
            if(indexes.empty())
              continue;

            auto bufferDs = createAggregationBuffer(indexes, contextDataSeries, aggregationBuffer, aggregationStatisticOperation, attribute);

            if(!bufferDs)
            {
              continue;
            }

            for(unsigned int i = 0; i < bufferDs->size(); ++i)
            {
              bufferDs->move(i);
              auto occurrenceGeom = bufferDs->getGeometry(0);

              if(occurrenceGeom->intersects(geomResult.get()))
              {
                ++countValues;

                try
                {
                  if(!attribute.empty())
                  {
                    hasData = true;
                    cache.count++;
                    double value = bufferDs->getDouble(1);

                    values.push_back(value);
                    cache.sum += value;
                    if(value > cache.max)
                      cache.max = value;
                    if(value < cache.min)
                      cache.min = value;
                  }
                }
                catch(...)
                {
                  // In case the dataset doesn't have the specified attribute
                  continue;
                }
              }
            }
          }
          else
          {
            for(uint64_t i : indexes)
            {
              // Verifies if the occurrence intersects the monitored object
              auto occurrenceGeom = syncDs->getGeometry(i, contextDataSeries->geometryPos);

              if(occurrenceGeom->intersects(geomResult.get()))
              {
                ++countValues;

                try
                {
                  if(!attribute.empty() && !syncDs->isNull(i, attribute))
                  {
                    hasData = true;
                    cache.count++;
                    double value = terrama2::services::analysis::core::getValue(syncDs, attribute, i, attributeType);

                    values.push_back(value);
                    cache.sum += value;
                    if(value > cache.max)
                      cache.max = value;
                    if(value < cache.min)
                      cache.min = value;
                  }
                }
                catch(...)
                {
                  // In case the dataset doesn't have the specified attribute
                  continue;
                }
              }
            }
          }




          terrama2::services::analysis::core::calculateStatistics(values, cache);

          if(statisticOperation == COUNT)
            cache.count = countValues;
        }

      }
    }
    catch(terrama2::Exception e)
    {
      TERRAMA2_LOG_ERROR() << e.what();
      return NAN;
    }
    catch(std::exception e)
    {
      TERRAMA2_LOG_ERROR() << e.what();
      return NAN;
    }
    catch(...)
    {
      return NAN;
    }


    // All operations are done, acquires the GIL and set the return value
  Py_END_ALLOW_THREADS


  if(!hasData && statisticOperation != COUNT)
  {
    return NAN;
  }

  return terrama2::services::analysis::core::getOperationResult(cache, statisticOperation);
}

int terrama2::services::analysis::core::occurrence::count(const std::string& dataSeriesName, Buffer buffer,
                                                          const std::string& dateFilter, const std::string& restriction)
{
  return (int) operatorImpl(COUNT, dataSeriesName, buffer, dateFilter, Buffer(), "",
                            INVALID, restriction);
}

double terrama2::services::analysis::core::occurrence::min(const std::string& dataSeriesName, terrama2::services::analysis::core::Buffer buffer,
                                                           const std::string& dateFilter, const std::string& attribute, const std::string& restriction)
{
  return operatorImpl(MIN, dataSeriesName, buffer, dateFilter, Buffer(), attribute,
                      INVALID, restriction);
}

double terrama2::services::analysis::core::occurrence::max(const std::string& dataSeriesName, Buffer buffer,
                                                           const std::string& dateFilter,
                                                           const std::string& attribute, const std::string& restriction)
{
  return operatorImpl(MAX, dataSeriesName, buffer, dateFilter, Buffer(), attribute,
                      INVALID, restriction);
}

double terrama2::services::analysis::core::occurrence::mean(const std::string& dataSeriesName, Buffer buffer,
                                                            const std::string& dateFilter,
                                                            const std::string& attribute,
                                                            const std::string& restriction)
{
  return operatorImpl(MEAN, dataSeriesName, buffer, dateFilter, Buffer(), attribute,
                      INVALID, restriction);
}

double terrama2::services::analysis::core::occurrence::median(const std::string& dataSeriesName, Buffer buffer,
                                                              const std::string& dateFilter,
                                                              const std::string& attribute,
                                                              const std::string& restriction)
{
  return operatorImpl(MEDIAN, dataSeriesName, buffer, dateFilter, Buffer(), attribute,
                      INVALID, restriction);
}

double terrama2::services::analysis::core::occurrence::standardDeviation(const std::string& dataSeriesName,
                                                                         Buffer buffer,
                                                                         const std::string& dateFilter,
                                                                         const std::string& attribute,
                                                                         const std::string& restriction)
{
  return operatorImpl(STANDARD_DEVIATION, dataSeriesName, buffer, dateFilter, Buffer(), attribute,
                      INVALID, restriction);
}

double terrama2::services::analysis::core::occurrence::sum(const std::string& dataSeriesName, Buffer buffer,
                                                           const std::string& dateFilter,
                                                           const std::string& attribute, const std::string& restriction)
{
  return operatorImpl(SUM, dataSeriesName, buffer, dateFilter, Buffer(), attribute,
                      INVALID, restriction);
}
