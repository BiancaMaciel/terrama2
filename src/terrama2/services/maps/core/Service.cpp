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
  \file terrama2/services/maps/core/Service.hpp

  \brief Class for the maps configuration.

  \author Vinicius Campanha
*/

#include "Service.hpp"
#include "Maps.hpp"

#include "../../../core/Shared.hpp"

#include "../../../core/data-model/DataSeries.hpp"
#include "../../../core/data-model/DataSet.hpp"
#include "../../../core/data-model/Filter.hpp"

#include "../../../core/data-access/DataAccessor.hpp"
#include "../../../core/data-access/DataStorager.hpp"

#include "../../../core/utility/Timer.hpp"
#include "../../../core/utility/Logger.hpp"
#include "../../../core/utility/DataAccessorFactory.hpp"
#include "../../../core/utility/DataStoragerFactory.hpp"
#include "../../../core/utility/ServiceManager.hpp"

terrama2::services::maps::core::Service::Service(std::weak_ptr<terrama2::services::maps::core::DataManager> dataManager)
  : dataManager_(dataManager)
{
  connectDataManager();
}

bool terrama2::services::maps::core::Service::hasDataOnQueue() noexcept
{
  return !mapsQueue_.empty();
}

bool terrama2::services::maps::core::Service::processNextData()
{
  // check if there is data to collect
  if(mapsQueue_.empty())
    return false;

  // get first data
  const auto& mapId = mapsQueue_.front();

  // prepare task for collecting
  prepareTask(mapId);

  // remove from queue
  mapsQueue_.pop_front();

  // is there more data to process?
  return !mapsQueue_.empty();
}

void terrama2::services::maps::core::Service::prepareTask(MapsId mapId)
{
  try
  {
    taskQueue_.emplace(std::bind(&makeMap, mapId, logger_, dataManager_));
  }
  catch(std::exception& e)
  {
    TERRAMA2_LOG_ERROR() << e.what();
  }
}

void terrama2::services::maps::core::Service::addToQueue(MapsId mapId) noexcept
{
  try
  {
    std::lock_guard<std::mutex> lock(mutex_);
    TERRAMA2_LOG_DEBUG() << tr("Map added to queue.");

    auto datamanager = dataManager_.lock();
    auto map = datamanager->findMap(mapId);

    const auto& serviceManager = terrama2::core::ServiceManager::getInstance();
    auto serviceInstanceId = serviceManager.instanceId();

    // Check if this map should be executed in this instance
    if(map->serviceInstanceId != serviceInstanceId)
      return;

    mapsQueue_.push_back(mapId);
    mainLoopCondition_.notify_one();
  }
  catch(...)
  {
    // exception guard, slots should never emit exceptions.
    TERRAMA2_LOG_ERROR() << QObject::tr("Unknown exception...");
  }
}

void terrama2::services::maps::core::Service::makeMap(MapsId mapId, std::shared_ptr< terrama2::services::maps::core::MapsLogger > logger, std::weak_ptr<DataManager> weakDataManager)
{
  auto dataManager = weakDataManager.lock();
  if(!dataManager.get())
  {
    TERRAMA2_LOG_ERROR() << tr("Unable to access DataManager");
    return;
  }

  try
  {

  }
  catch(const terrama2::Exception&)
  {
    TERRAMA2_LOG_INFO() << tr("Collection for collector %1 finished with error(s).").arg(mapId);
  }
  catch(const boost::exception& e)
  {
    TERRAMA2_LOG_ERROR() << boost::get_error_info<terrama2::ErrorDescription>(e);
    TERRAMA2_LOG_INFO() << tr("Collection for collector %1 finished with error(s).").arg(mapId);
  }
  catch(const std::exception& e)
  {
    TERRAMA2_LOG_ERROR() << e.what();
    TERRAMA2_LOG_INFO() << tr("Collection for collector %1 finished with error(s).").arg(mapId);
  }
  catch(...)
  {
    TERRAMA2_LOG_ERROR() << tr("Unkown error.");
    TERRAMA2_LOG_INFO() << tr("Collection for collector %1 finished with error(s).").arg(mapId);
  }
}

void terrama2::services::maps::core::Service::connectDataManager()
{
  auto dataManager = dataManager_.lock();
  connect(dataManager.get(), &terrama2::services::maps::core::DataManager::mapAdded, this,
          &terrama2::services::maps::core::Service::addMap);
  connect(dataManager.get(), &terrama2::services::maps::core::DataManager::mapRemoved, this,
          &terrama2::services::maps::core::Service::removeMap);
  connect(dataManager.get(), &terrama2::services::maps::core::DataManager::mapUpdated, this,
          &terrama2::services::maps::core::Service::updateMap);
}

void terrama2::services::maps::core::Service::setLogger(std::shared_ptr<MapsLogger> logger) noexcept
{
  logger_ = logger;
}

void terrama2::services::maps::core::Service::addMap(MapsPtr map) noexcept
{
  try
  {
    const auto& serviceManager = terrama2::core::ServiceManager::getInstance();
    auto serviceInstanceId = serviceManager.instanceId();

    // Check if this map should be executed in this instance
    if(map->serviceInstanceId != serviceInstanceId)
      return;

    try
    {
      if(map->active)
      {
        std::lock_guard<std::mutex> lock(mutex_);

        std::shared_ptr<te::dt::TimeInstantTZ> lastProcess;
        if(logger_.get())
          lastProcess = logger_->getLastProcessTimestamp(map->id);

        terrama2::core::TimerPtr timer = createTimer(map->schedule, map->id, lastProcess);
        timers_.emplace(map->id, timer);
      }
    }
    catch(terrama2::core::InvalidFrequencyException&)
    {
      // invalid schedule, already logged
    }
    catch(const te::common::Exception& e)
    {
      TERRAMA2_LOG_ERROR() << e.what();
    }

    addToQueue(map->id);
  }
  catch(...)
  {
    // exception guard, slots should never emit exceptions.
    TERRAMA2_LOG_ERROR() << QObject::tr("Unknown exception...");
  }

}

void terrama2::services::maps::core::Service::removeMap(MapsId mapId) noexcept
{
  try
  {
    std::lock_guard<std::mutex> lock(mutex_);


    TERRAMA2_LOG_INFO() << tr("Removing map %1.").arg(mapId);

    auto it = timers_.find(mapId);
    if(it != timers_.end())
    {
      auto timer = timers_.at(mapId);
      timer->disconnect();
      timers_.erase(mapId);
    }

    // remove from queue
    mapsQueue_.erase(std::remove(mapsQueue_.begin(), mapsQueue_.end(), mapId), mapsQueue_.end());


    TERRAMA2_LOG_INFO() << tr("Map %1 removed successfully.").arg(mapId);
  }
  catch(std::exception& e)
  {
    TERRAMA2_LOG_ERROR() << e.what();
    TERRAMA2_LOG_INFO() << tr("Could not remove map: %1.").arg(mapId);
  }
  catch(boost::exception& e)
  {
    TERRAMA2_LOG_ERROR() << boost::get_error_info<terrama2::ErrorDescription>(e);
    TERRAMA2_LOG_INFO() << tr("Could not remove collector: %1.").arg(mapId);
  }
  catch(...)
  {
    TERRAMA2_LOG_ERROR() << tr("Unknown error");
    TERRAMA2_LOG_INFO() << tr("Could not remove collector: %1.").arg(mapId);
  }
}

void terrama2::services::maps::core::Service::updateMap(MapsPtr map) noexcept
{
  //TODO: adds to queue, is this expected? remove and then add?
  addMap(map);
}
