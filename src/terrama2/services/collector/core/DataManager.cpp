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
  \file terrama2/services/collector/core/Collector.hpp

  \brief Model class for the collector configuration.

  \author Jano Simas
*/

#include "DataManager.hpp"
#include "Collector.hpp"
#include "Exception.hpp"
#include "../../../core/Exception.hpp"

//STL
#include <mutex>

struct terrama2::services::collector::core::DataManager::CImpl
{
  std::map<CollectorId, CollectorPtr> collectors;
};

terrama2::services::collector::core::CollectorPtr
terrama2::services::collector::core::DataManager::findCollector(CollectorId id) const
{
  return pcimpl_->collectors.at(id);
}


void terrama2::services::collector::core::DataManager::add(terrama2::services::collector::core::CollectorPtr collector)
{
  // Inside a block so the lock is released before emitting the signal
  {
    std::lock_guard<std::recursive_mutex> lock(pimpl_->mtx);

    if(collector->id == terrama2::core::InvalidId())
      throw terrama2::InvalidArgumentException() <<
            ErrorDescription(QObject::tr("Can not add a data provider with an invalid id."));

    pcimpl_->collectorr[collector->id] = collector;
  }

  emit collectorAdded(collector);
}

void terrama2::services::collector::core::DataManager::update(terrama2::services::collector::core::CollectorPtr collector)
{
  {
    std::lock_guard<std::recursive_mutex> lock(pimpl_->mtx);
    blockSignals(true);
    removeCollector(collector->id);
    add(collector);
    blockSignals(false);
  }

  emit collectorUpdated(collector);
}

void terrama2::services::collector::core::DataManager::removeCollector(CollectorId collectorId)
{
  {
    std::lock_guard<std::recursive_mutex> lock(pimpl_->mtx);
    auto itPr = pcimpl_->collectors.find(collectorId);
    if(itPr == pcimpl_->collectors.end())
    {
      throw terrama2::InvalidArgumentException() <<
            ErrorDescription(QObject::tr("DataProvider not registered."));
    }

    pcimpl_->collectors.erase(itPr);
  }

  emit collectorRemoved(collectorId);
}
