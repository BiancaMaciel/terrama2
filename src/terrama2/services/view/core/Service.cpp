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
  \file terrama2/services/view/core/Service.hpp

  \brief Class for the view configuration.

  \author Vinicius Campanha
*/

// TerraMA2
#include "Service.hpp"
#include "View.hpp"
#include "MemoryDataSetLayer.hpp"

#include "data-access/Geoserver.hpp"

#include "../../../core/Shared.hpp"
#include "../../../core/utility/TimeUtils.hpp"

#include "../../../core/data-model/DataSeries.hpp"
#include "../../../core/data-model/DataSet.hpp"
#include "../../../core/data-model/Filter.hpp"

#include "../../../core/data-access/DataAccessor.hpp"
#include "../../../core/data-access/DataStorager.hpp"

#include "../../../impl/DataAccessorFile.hpp"
#include "../../../impl/DataAccessorGeoTiff.hpp"
#include "../../../impl/DataAccessorPostGis.hpp"

#include "../../../core/utility/Timer.hpp"
#include "../../../core/utility/Logger.hpp"
#include "../../../core/utility/DataAccessorFactory.hpp"
#include "../../../core/utility/DataStoragerFactory.hpp"
#include "../../../core/utility/ServiceManager.hpp"

// Qt
#include <QUrl>
#include <QJsonArray>

terrama2::services::view::core::Service::Service(std::weak_ptr<terrama2::services::view::core::DataManager> dataManager)
  : dataManager_(dataManager)
{
  connectDataManager();
}

bool terrama2::services::view::core::Service::hasDataOnQueue() noexcept
{
  return !viewQueue_.empty();
}

bool terrama2::services::view::core::Service::processNextData()
{
  // check if there is View to build
  if(viewQueue_.empty())
    return false;

  // get first data
  const auto& viewId = viewQueue_.front();

  // prepare task for View building
  prepareTask(viewId);

  // remove from queue
  viewQueue_.pop_front();

  // is there more data to process?
  return !viewQueue_.empty();
}

void terrama2::services::view::core::Service::prepareTask(ViewId viewId)
{
  try
  {
    taskQueue_.emplace(std::bind(&Service::viewJob, this, viewId, logger_, dataManager_));
  }
  catch(std::exception& e)
  {
    TERRAMA2_LOG_ERROR() << e.what();
  }
}

void terrama2::services::view::core::Service::addToQueue(ViewId viewId) noexcept
{
  try
  {
    std::lock_guard<std::mutex> lock(mutex_);
    TERRAMA2_LOG_DEBUG() << tr("View added to queue.");

    auto datamanager = dataManager_.lock();
    auto view = datamanager->findView(viewId);

    const auto& serviceManager = terrama2::core::ServiceManager::getInstance();
    auto serviceInstanceId = serviceManager.instanceId();

    // Check if this view should be executed in this instance
    if(view->serviceInstanceId != serviceInstanceId)
      return;

    viewQueue_.push_back(viewId);
    mainLoopCondition_.notify_one();
  }
  catch(...)
  {
    // exception guard, slots should never emit exceptions.
    TERRAMA2_LOG_ERROR() << QObject::tr("Unknown exception...");
  }
}



void terrama2::services::view::core::Service::connectDataManager()
{
  auto dataManager = dataManager_.lock();
  connect(dataManager.get(), &terrama2::services::view::core::DataManager::viewAdded, this,
          &terrama2::services::view::core::Service::addView);
  connect(dataManager.get(), &terrama2::services::view::core::DataManager::viewRemoved, this,
          &terrama2::services::view::core::Service::removeView);
  connect(dataManager.get(), &terrama2::services::view::core::DataManager::viewUpdated, this,
          &terrama2::services::view::core::Service::updateView);
}

void terrama2::services::view::core::Service::setLogger(std::shared_ptr<ViewLogger> logger) noexcept
{
  logger_ = logger;
}

void terrama2::services::view::core::Service::addView(ViewPtr view) noexcept
{
  try
  {
    const auto& serviceManager = terrama2::core::ServiceManager::getInstance();
    auto serviceInstanceId = serviceManager.instanceId();

    // Check if this view should be executed in this instance
    if(view->serviceInstanceId != serviceInstanceId)
      return;

    try
    {
      if(view->active)
      {
        std::lock_guard<std::mutex> lock(mutex_);

        std::shared_ptr<te::dt::TimeInstantTZ> lastProcess;
        if(logger_.get())
          lastProcess = logger_->getLastProcessTimestamp(view->id);

        terrama2::core::TimerPtr timer = createTimer(view->schedule, view->id, lastProcess);
        timers_.emplace(view->id, timer);
      }
    }
    catch(const terrama2::core::InvalidFrequencyException&)
    {
      // invalid schedule, already logged
    }
    catch(const te::common::Exception& e)
    {
      TERRAMA2_LOG_ERROR() << e.what();
    }

    addToQueue(view->id);
  }
  catch(...)
  {
    // exception guard, slots should never emit exceptions.
    TERRAMA2_LOG_ERROR() << QObject::tr("Unknown exception...");
  }

}

void terrama2::services::view::core::Service::removeView(ViewId viewId) noexcept
{
  try
  {
    std::lock_guard<std::mutex> lock(mutex_);


    TERRAMA2_LOG_INFO() << tr("Removing view %1.").arg(viewId);

    auto it = timers_.find(viewId);
    if(it != timers_.end())
    {
      auto timer = timers_.at(viewId);
      timer->disconnect();
      timers_.erase(viewId);
    }

    // remove from queue
    viewQueue_.erase(std::remove(viewQueue_.begin(), viewQueue_.end(), viewId), viewQueue_.end());


    TERRAMA2_LOG_INFO() << tr("View %1 removed successfully.").arg(viewId);
  }
  catch(std::exception& e)
  {
    TERRAMA2_LOG_ERROR() << e.what();
    TERRAMA2_LOG_INFO() << tr("Could not remove view: %1.").arg(viewId);
  }
  catch(boost::exception& e)
  {
    TERRAMA2_LOG_ERROR() << boost::get_error_info<terrama2::ErrorDescription>(e);
    TERRAMA2_LOG_INFO() << tr("Could not remove view: %1.").arg(viewId);
  }
  catch(...)
  {
    TERRAMA2_LOG_ERROR() << tr("Unknown error");
    TERRAMA2_LOG_INFO() << tr("Could not remove view: %1.").arg(viewId);
  }
}

void terrama2::services::view::core::Service::updateView(ViewPtr view) noexcept
{
  //TODO: adds to queue, is this expected? remove and then add?
  addView(view);
}

void terrama2::services::view::core::Service::viewJob(ViewId viewId,
                                                      std::shared_ptr< terrama2::services::view::core::ViewLogger > logger,
                                                      std::weak_ptr<DataManager> weakDataManager)
{
  auto dataManager = weakDataManager.lock();
  if(!dataManager.get())
  {
    TERRAMA2_LOG_ERROR() << QObject::tr("Unable to access DataManager");
    return;
  }

  if(!logger.get())
  {
    QString errMsg = QObject::tr("Unable to access Logger class in view %1").arg(viewId);
    TERRAMA2_LOG_ERROR() << errMsg;
    return;
  }

  QJsonObject jsonAnswer;

  try
  {
    RegisterId logId = 0;

    TERRAMA2_LOG_DEBUG() << QObject::tr("Starting view %1 generation.").arg(viewId);

    logId = logger->start(viewId);

    /////////////////////////////////////////////////////////////////////////
    //  aquiring metadata

    auto lock = dataManager->getLock();

    auto viewPtr = dataManager->findView(viewId);

    std::unordered_map< terrama2::core::DataSeriesPtr, terrama2::core::DataProviderPtr > dataSeriesProviders;
    for(auto dataSeriesId : viewPtr->dataSeriesList)
    {
      terrama2::core::DataSeriesPtr inputDataSeries = dataManager->findDataSeries(dataSeriesId);
      terrama2::core::DataProviderPtr inputDataProvider = dataManager->findDataProvider(inputDataSeries->dataProviderId);

      dataSeriesProviders.emplace(inputDataSeries, inputDataProvider);
    }

    lock.unlock();

    /////////////////////////////////////////////////////////////////////////

    for(auto dataSeriesProvider : dataSeriesProviders)
    {
      terrama2::core::DataSeriesPtr inputDataSeries = dataSeriesProvider.first;
      terrama2::core::DataProviderPtr inputDataProvider = dataSeriesProvider.second;

      DataProviderType dataProviderType = inputDataProvider->dataProviderType;

      if(dataProviderType != "POSTGIS" && dataProviderType != "FILE")
      {
        TERRAMA2_LOG_ERROR() << QObject::tr("Data provider not supported: %1.").arg(dataProviderType.c_str());
      }

      DataFormat dataFormat = inputDataSeries->semantics.dataFormat;

      // Check if the view can be done by the maps server
      bool mapsServerGeneration = false;

      if(!viewPtr->maps_server_uri.uri().empty())
      {
        if(dataFormat != "OGR" && dataFormat != "POSTGIS" && dataFormat != "GEOTIFF")
        {
          TERRAMA2_LOG_WARNING() << QObject::tr("Data format not supported in the maps server: %1.").arg(dataFormat.c_str());
        }
        else
        {
          mapsServerGeneration = true;
        }
      }

      if(mapsServerGeneration)
      {
        QFileInfoList fileInfoList;
        QJsonArray layersArray;

        da::GeoServer geoserver(viewPtr->maps_server_uri);
        geoserver.registerWorkspace();

        terrama2::core::DataAccessorPtr dataAccessor =
            terrama2::core::DataAccessorFactory::getInstance().make(inputDataProvider, inputDataSeries);

        terrama2::core::Filter filter;

        auto it = viewPtr->filtersPerDataSeries.find(inputDataSeries->id);

        if(it != viewPtr->filtersPerDataSeries.end())
        {
          filter = terrama2::core::Filter(it->second);
        }

        auto remover = std::make_shared<terrama2::core::FileRemover>();

        const std::vector< terrama2::core::DataSetPtr > datasets = inputDataSeries->datasetList;

        if(!datasets.empty())
        {
          if(dataProviderType == "FILE")
          {
            // Get the list of layers to register
            if(dataFormat == "OGR")
            {
              auto files = dataSeriesFileList<std::shared_ptr<terrama2::core::DataAccessorFile>>(datasets,
                                                                                                 inputDataProvider,
                                                                                                 filter,
                                                                                                 remover,
                                                                                                 std::dynamic_pointer_cast<terrama2::core::DataAccessorFile>(dataAccessor));
              fileInfoList.append(files);
            }
            else if(dataFormat == "GEOTIFF")
            {
              auto files = dataSeriesFileList<std::shared_ptr<terrama2::core::DataAccessorGeoTiff>>(datasets,
                                                                                                    inputDataProvider,
                                                                                                    filter,
                                                                                                    remover,
                                                                                                    std::dynamic_pointer_cast<terrama2::core::DataAccessorGeoTiff>(dataAccessor));
              fileInfoList.append(files);
            }

            for(auto& fileInfo : fileInfoList)
            {
              if(dataFormat == "OGR")
              {
                geoserver.registerVectorFile(viewPtr->viewName + std::to_string(inputDataSeries->id) + "datastore",
                                             fileInfo.absoluteFilePath().toStdString(),
                                             fileInfo.completeSuffix().toStdString());
              }
              else if(dataFormat == "GEOTIFF")
              {
                geoserver.registerCoverageFile(viewPtr->viewName + std::to_string(inputDataSeries->id) + "coveragestore",
                                               fileInfo.absoluteFilePath().toStdString(),
                                               "geotiff");
              }

              QJsonObject layer;
              layer.insert("layer", fileInfo.baseName());
              layersArray.push_back(layer);
            }
          }
          else if(dataProviderType == "POSTGIS")
          {
            std::shared_ptr< terrama2::core::DataAccessorPostGis > dataAccessorPostGis =
                  std::dynamic_pointer_cast<terrama2::core::DataAccessorPostGis>(dataAccessor);

            std::vector<std::string> tablesNames;

            for(auto& dataset : datasets)
            {
              std::string tableName = dataAccessorPostGis->getDataSetTableName(dataset);

              tablesNames.push_back(tableName);
            }

            QUrl url(inputDataProvider->uri.c_str());

            std::map<std::string, std::string> connInfo
            {
              {"PG_HOST", url.host().toStdString()},
              {"PG_PORT", std::to_string(url.port())},
              {"PG_USER", url.userName().toStdString()},
              {"PG_PASSWORD", url.password().toStdString()},
              {"PG_DB_NAME", url.path().section("/", 1, 1).toStdString()},
              {"PG_CONNECT_TIMEOUT", "4"},
              {"PG_CLIENT_ENCODING", "UTF-8"}
            };

            for(auto& name : tablesNames)
            {
              geoserver.registerPostgisTable(viewPtr->viewName + std::to_string(inputDataSeries->id) + "postgisstore",
                                             connInfo,
                                             name);

              QJsonObject layer;
              layer.insert("layer", QString::fromStdString(name));
              layersArray.push_back(layer);
            }
          }
        }
        else
        {
          logger->info("No data to register.", logId);
          TERRAMA2_LOG_WARNING() << tr("No data to register in maps server.");
        }

        std::string styleName = "";
        auto itStyle = viewPtr->stylesPerDataSeries.find(inputDataSeries->id);

        if(itStyle != viewPtr->stylesPerDataSeries.end())
        {
          styleName = viewPtr->viewName + "style" + std::to_string(inputDataSeries->id);
          geoserver.registerStyle(styleName, itStyle->second);
        }

        jsonAnswer.insert("class", QString("RegisteredViews"));
        jsonAnswer.insert("process_id",static_cast<int32_t>(viewPtr->id));
        jsonAnswer.insert("maps_server_uri", QString::fromStdString(geoserver.uri().uri()));
        jsonAnswer.insert("workspace", QString::fromStdString(geoserver.workspace()));
        jsonAnswer.insert("style", QString::fromStdString(styleName));
        jsonAnswer.insert("layers_list", layersArray);
      }

      if(!viewPtr->imageName.empty())
      {
        // TODO: create VIEW with TerraLib
      }

    }

    TERRAMA2_LOG_INFO() << tr("View %1 generated successfully.").arg(viewId);

    logger->done(terrama2::core::TimeUtils::nowUTC(), logId);

    emit processFinishedSignal(jsonAnswer);
  }
  catch(const terrama2::Exception& e)
  {
    TERRAMA2_LOG_ERROR() << boost::get_error_info<terrama2::ErrorDescription>(e)->toStdString() << std::endl;
    TERRAMA2_LOG_INFO() << QObject::tr("Build of view %1 finished with error(s).").arg(viewId);
  }
  catch(const boost::exception& e)
  {
    TERRAMA2_LOG_ERROR() << boost::get_error_info<terrama2::ErrorDescription>(e);
    TERRAMA2_LOG_INFO() << QObject::tr("Build of view %1 finished with error(s).").arg(viewId);
  }
  catch(const std::exception& e)
  {
    TERRAMA2_LOG_ERROR() << e.what();
    TERRAMA2_LOG_INFO() << QObject::tr("Build of view %1 finished with error(s).").arg(viewId);
  }
  catch(...)
  {
    TERRAMA2_LOG_ERROR() << QObject::tr("Unkown error.");
    TERRAMA2_LOG_INFO() << QObject::tr("Build of view %1 finished with error(s).").arg(viewId);
  }
}


template< typename Accessor >
QFileInfoList terrama2::services::view::core::Service::dataSeriesFileList(const std::vector<terrama2::core::DataSetPtr> datasets,
                                                                          const terrama2::core::DataProviderPtr inputDataProvider,
                                                                          const terrama2::core::Filter filter,
                                                                          const std::shared_ptr<terrama2::core::FileRemover> remover,
                                                                          const Accessor dataAccessor)
{
  QFileInfoList fileInfoList;

  for(auto& dataset : datasets)
  {
    // TODO: mask in folder
    QUrl url;
    try
    {
      url = QUrl(QString::fromStdString(inputDataProvider->uri+"/"+dataAccessor->getFolder(dataset)));
    }
    catch(const terrama2::core::UndefinedTagException& /*e*/)
    {
      url = QUrl(QString::fromStdString(inputDataProvider->uri));
    }

    //get timezone of the dataset
    std::string timezone;
    try
    {
      timezone = dataAccessor->getTimeZone(dataset);
    }
    catch(const terrama2::core::UndefinedTagException& /*e*/)
    {
      //if timezone is not defined
      timezone = "UTC+00";
    }

    QFileInfoList tempFileInfoList = dataAccessor->getDataFileInfoList(url.toString().toStdString(),
                                                                       dataAccessor->getMask(dataset),
                                                                       timezone,
                                                                       filter,
                                                                       remover);

    if(tempFileInfoList.empty())
    {
      TERRAMA2_LOG_WARNING() << tr("No data in folder: %1").arg(url.toString());
      continue;
    }

    fileInfoList.append(tempFileInfoList);
  }

  return fileInfoList;
}
