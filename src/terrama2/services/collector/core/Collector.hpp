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

#ifndef __TERRAMA2_SERVICES_COLLECTOR_CORE_COLLECTOR_HPP__
#define __TERRAMA2_SERVICES_COLLECTOR_CORE_COLLECTOR_HPP__

#include "../../../core/data-model/DataSeries.hpp"
#include "../../../core/data-model/Schedule.hpp"
#include "../../../core/data-model/Filter.hpp"
#include "../../../core/Typedef.hpp"
#include "Typedef.hpp"
#include "Shared.hpp"

// STL
#include <string>
#include <vector>

namespace terrama2
{
  namespace services
  {
    namespace collector
    {
      namespace core
      {
        struct Collector
        {
          CollectorId id = 0;
          ProjectId projectId = 0;
          ServiceInstanceId serviceInstanceId = 0;

          bool active = true;

          DataSeriesId inputDataSeries = 0;
          DataSeriesId outputDataSeries = 0;

          std::map<DataSetId, DataSetId> inputOutputMap;

          terrama2::core::Schedule schedule;
          terrama2::core::Filter filter;
          IntersectionPtr intersection;
        };

      } // end namespace core
    }   // end namespace collector
  }     // end namespace services
} // end namespace terrama2

#endif //__TERRAMA2_SERVICES_COLLECTOR_CORE_COLLECTOR_HPP__
