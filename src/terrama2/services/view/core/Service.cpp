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
    taskQueue_.emplace(std::bind(&makeView, viewId, logger_, dataManager_));
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
