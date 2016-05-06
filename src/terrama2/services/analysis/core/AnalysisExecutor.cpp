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
  \file terrama2/services/analysis/core/AnalysisExecutor.hpp

  \brief Prepare context for an analysis execution.

  \author Paulo R. M. Oliveira
*/

#include "AnalysisExecutor.hpp"


// Python
#include <Python.h>

#include "PythonInterpreter.hpp"
#include "Context.hpp"
#include "DataManager.hpp"
#include "../../../core/data-access/SyncronizedDataSet.hpp"
#include "../../../core/utility/Logger.hpp"
#include "../../../core/utility/Utils.hpp"
#include "../../../core/utility/TimeUtils.hpp"
#include "../../../core/data-access/DataStorager.hpp"
#include "../../../core/data-model/DataProvider.hpp"
#include "../../../impl/DataStoragerPostGis.hpp"

// TerraLib
#include <terralib/datatype/SimpleProperty.h>
#include <terralib/datatype/StringProperty.h>
#include <terralib/datatype/DateTimeProperty.h>
#include <terralib/datatype/DateTimeInstant.h>
#include <terralib/datatype/TimeInstant.h>
#include <terralib/memory/DataSet.h>
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/dataaccess/datasource/DataSourceFactory.h>
#include <terralib/memory/DataSetItem.h>

//STL
#include <memory>


void terrama2::services::analysis::core::joinThread(std::thread& t)
{
  if(t.joinable())
    t.join();
}

void terrama2::services::analysis::core::joinAllThreads(std::vector<std::thread>& threads)
{
  std::for_each(threads.begin(), threads.end(), joinThread);
}

void terrama2::services::analysis::core::runAnalysis(DataManagerPtr dataManager, const Analysis& analysis)
{

  switch(analysis.type)
  {
    case MONITORED_OBJECT_TYPE:
    {
      runMonitoredObjectAnalysis(dataManager, analysis);
      break;
    }
    case PCD_TYPE:
    {
      runDCPAnalysis(dataManager, analysis);
      break;
    }
    default:
    {
      QString errMsg = QObject::tr("Not implemented yet.");
      TERRAMA2_LOG_ERROR() << errMsg;
      throw Exception()  << ErrorDescription(errMsg);
    }
  }
}

void terrama2::services::analysis::core::runMonitoredObjectAnalysis(DataManagerPtr dataManager, const Analysis& analysis)
{
  try
  {
    terrama2::services::analysis::core::Context::getInstance().loadMonitoredObject(analysis);

    int size = 0;
    for(auto analysisDataSeries : analysis.analysisDataSeriesList)
    {
      if(analysisDataSeries.type == DATASERIES_MONITORED_OBJECT_TYPE)
      {
        auto dataSeries = dataManager->findDataSeries(analysisDataSeries.dataSeriesId);
        auto datasets = dataSeries->datasetList;
        assert(datasets.size() == 1);
        auto dataset = datasets[0];

        auto contextDataset = terrama2::services::analysis::core::Context::getInstance().getContextDataset(analysis.id, dataset->id);
        if(!contextDataset->series.syncDataSet->dataset())
        {
          QString errMsg = QObject::tr("Could not recover monitored object dataset.");
          TERRAMA2_LOG_WARNING() << errMsg;
          throw terrama2::InvalidArgumentException() << ErrorDescription(errMsg);
        }
        size = contextDataset->series.syncDataSet->size();

        break;
      }
    }

    //check for the number o threads to create
    int threadNumber = std::thread::hardware_concurrency();

    PyThreadState * mainThreadState = NULL;
    // save a pointer to the main PyThreadState object
    mainThreadState = PyThreadState_Get();

    // get a reference to the PyInterpreterState
    PyInterpreterState * mainInterpreterState = mainThreadState->interp;



    // Calculates the number of geometries that each thread will contain.
    int packageSize = 1;
    if(size >= threadNumber)
    {
      packageSize = (int)(size / threadNumber);
    }

    // if it's different than 0, the last package will be bigger.
    int mod = size % threadNumber;

    int begin = 0;

    std::vector<std::thread> threads(threadNumber);
    std::vector<PyThreadState*> states;

    //Starts collection threads
    for (uint i = 0; i < threadNumber; ++i)
    {

      std::vector<uint64_t> indexes;
       // The last package takes the rest of the division.
      if(i == threadNumber - 1)
        packageSize += mod;

      for(unsigned int j = begin; j < begin + packageSize; ++j)
      {
        indexes.push_back(j);
      }


      // create a thread state object for this thread
      PyThreadState * myThreadState = PyThreadState_New(mainInterpreterState);
      states.push_back(myThreadState);
      threads[i] = std::thread(&terrama2::services::analysis::core::runScriptMonitoredObjectAnalysis, myThreadState, analysis.id, indexes);

      begin += packageSize;
    }

    joinAllThreads(threads);


    // grab the lock
    PyEval_AcquireLock();
    for(auto state : states)
    {
      // swap my thread state out of the interpreter
      PyThreadState_Swap(NULL);
      // clear out any cruft from thread state object
      PyThreadState_Clear(state);
      // delete my thread state object
      PyThreadState_Delete(state);
    }

    PyThreadState_Swap(mainThreadState);

    // release the lock
    PyEval_ReleaseLock();

    storeAnalysisResult(dataManager, analysis);


    terrama2::services::analysis::core::finalizeInterpreter();
  }
  catch(std::exception& e)
  {
    TERRAMA2_LOG_ERROR() << e.what();
  }
}


void terrama2::services::analysis::core::runDCPAnalysis(DataManagerPtr dataManager, const Analysis& analysis)
{
  try
  {

    int size = 0;
    for(auto analysisDataSeries : analysis.analysisDataSeriesList)
    {
      if(analysisDataSeries.type == DATASERIES_PCD_TYPE)
      {
        auto dataSeriesPtr = dataManager->findDataSeries(analysisDataSeries.dataSeriesId);
        size =  dataSeriesPtr->datasetList.size();

        Context::getInstance().addDCP(analysis.id, dataSeriesPtr);
        break;
      }
    }

    // save a pointer to the main PyThreadState object
    PyThreadState* mainThreadState = PyThreadState_Get();

    // get a reference to the PyInterpreterState
    PyInterpreterState* mainInterpreterState = mainThreadState->interp;

    // create a thread state object for this thread
    PyThreadState* myThreadState = PyThreadState_New(mainInterpreterState);
    std::thread thread(&terrama2::services::analysis::core::runScriptDCPAnalysis, myThreadState, analysis.id);

    thread.join();


    // grab the lock
    PyEval_AcquireLock();

    // swap my thread state out of the interpreter
    PyThreadState_Swap(NULL);
    // clear out any cruft from thread state object
    PyThreadState_Clear(myThreadState);
    // delete my thread state object
    PyThreadState_Delete(myThreadState);

    PyThreadState_Swap(mainThreadState);

    // release the lock
    PyEval_ReleaseLock();

    terrama2::services::analysis::core::finalizeInterpreter();
  }
  catch(std::exception& e)
  {
    TERRAMA2_LOG_ERROR() << e.what();
  }
}

void terrama2::services::analysis::core::storeAnalysisResult(DataManagerPtr dataManager, const Analysis& analysis)
{
  auto resultMap = Context::getInstance().analysisResult(analysis.id);

  if(resultMap.empty())
  {
    QString errMsg = QObject::tr("Analysis %1 returned an empty result.").arg(analysis.id);
    TERRAMA2_LOG_WARNING() << errMsg;
    return;
  }

  auto attributes = Context::getInstance().getAttributes(analysis.id);

  auto dataSeries = dataManager->findDataSeries(analysis.outputDataSeriesId);

  if(!dataSeries)
  {
    QString errMsg = QObject::tr("Could not find the output data series.");
    TERRAMA2_LOG_ERROR() << errMsg;
    throw terrama2::InvalidArgumentException() << ErrorDescription(errMsg);
  }

  auto dataProvider = dataManager->findDataProvider(dataSeries->dataProviderId);
  if(!dataProvider)
  {
    QString errMsg = QObject::tr("Could not find the output data provider.");
    TERRAMA2_LOG_ERROR() << errMsg;
    throw terrama2::InvalidArgumentException() << ErrorDescription(errMsg);
  }

  if(analysis.type == MONITORED_OBJECT_TYPE || analysis.type == PCD_TYPE)
  {
    assert(dataSeries->datasetList.size() == 1);

    std::string datasetName;
    std::unique_ptr<terrama2::core::DataStorager> storager;
    if(dataProvider->dataProviderType == "POSTGIS")
    {
      auto datasSet = dataSeries->datasetList[0];
      datasetName = datasSet->format.at("table_name");
      storager.reset(new terrama2::core::DataStoragerPostGis(dataProvider));
    }
    else
    {
      //TODO Paulo: Implement storager file
      throw terrama2::Exception() << ErrorDescription("NOT IMPLEMENTED YET");
    }

    te::da::DataSetType* dt = new te::da::DataSetType(datasetName);

    // first property is the geomId
    te::dt::StringProperty* geomIdProp = new te::dt::StringProperty("geom_id", te::dt::VAR_STRING, 255, true);
    dt->add(geomIdProp);

    //define a primary key
    std::string namepk = datasetName+ "_pk";
    te::da::PrimaryKey* pk = new te::da::PrimaryKey(namepk, dt);
    pk->add(geomIdProp);

    //second property: analysis execution date
    te::dt::DateTimeProperty* dateProp = new te::dt::DateTimeProperty( "execution_date", te::dt::TIME_INSTANT_TZ, true);
    dt->add(dateProp);


    //create index on date column
    te::da::Index* indexDate = new te::da::Index(datasetName+ "_idx", te::da::B_TREE_TYPE, dt);
    indexDate->add(dateProp);

    for(std::string attribute : attributes)
    {
      te::dt::SimpleProperty* prop = new te::dt::SimpleProperty(attribute, te::dt::DOUBLE_TYPE, false);
      dt->add(prop);
    }

    auto date = terrama2::core::TimeUtils::nowUTC();

    // Creates memory dataset and add the items.
    std::shared_ptr<te::mem::DataSet> ds = std::make_shared<te::mem::DataSet>(dt);
    for(auto it = resultMap.begin(); it != resultMap.end(); ++it)
    {
      te::mem::DataSetItem* dsItem = new te::mem::DataSetItem(ds.get());
      dsItem->setString("geom_id", it->first);
      dsItem->setDateTime("execution_date",  dynamic_cast<te::dt::DateTimeInstant*>(date.get()->clone()));
      for(auto itAttribute = it->second.begin(); itAttribute != it->second.end(); ++itAttribute)
      {
        dsItem->setDouble(itAttribute->first, itAttribute->second);
      }

      ds->add(dsItem);
    }


    assert(dataSeries->datasetList.size() == 1);
    auto dataset = dataSeries->datasetList[0];


    if(!storager)
    {
      QString errMsg = QObject::tr("Could not find storager support for the output data provider.");
      TERRAMA2_LOG_ERROR() << errMsg;
      throw terrama2::InvalidArgumentException() << ErrorDescription(errMsg);
    }

    std::shared_ptr<terrama2::core::SyncronizedDataSet> syncDataSet = std::make_shared<terrama2::core::SyncronizedDataSet>(ds);

    terrama2::core::Series series;
    series.teDataSetType.reset(dt);
    series.syncDataSet.swap(syncDataSet);

    try
    {
      storager->store(series, dataset);
    }
    catch(terrama2::Exception /*e*/)
    {
      QString errMsg = QObject::tr("Could not store the result of the analysis: %1.").arg(analysis.id);
      TERRAMA2_LOG_ERROR() << errMsg;
    }


    QString errMsg = QObject::tr("Analysis %1 finished successfully.").arg(analysis.id);
    TERRAMA2_LOG_INFO() << errMsg;

  }
}
