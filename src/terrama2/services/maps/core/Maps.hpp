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
  \file terrama2/services/maps/core/Maps.hpp

  \brief Model class for the maps configuration.

  \author Vinicius Campanha
*/

#ifndef __TERRAMA2_SERVICES_MAPS_CORE_MAPS_HPP__
#define __TERRAMA2_SERVICES_MAPS_CORE_MAPS_HPP__

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
    namespace maps
    {
      namespace core
      {
        /*!
          \brief The Maps groups the information to draw maps.
        */
        struct Maps
        {
          MapsId id = 0;//!< Map unique identification.
          ProjectId projectId = 0;//!< Identification of the project owner of the map.
          ServiceInstanceId serviceInstanceId = 0;//!< Map service instace where the map should be executed.

          bool active = true;//!< Flag if the map is active.

          DataSeriesId inputDataSeries = 0;//!< DataSeries source of the data.
          DataSeriesId outputDataSeries = 0;//!< DataSeries detiny os the data.

          std::map<DataSetId, DataSetId> inputOutputMap;//!< Map of source DataSet to destiny DataSet.
          terrama2::core::Schedule schedule;//!< terrama2::core::Schedule of execution of the map.
          terrama2::core::Filter filter;//!< Information on how input data should be filtered before storage.
        };

      } // end namespace core
    }   // end namespace maps
  }     // end namespace services
} // end namespace terrama2

#endif //__TERRAMA2_SERVICES_MAPS_CORE_MAPS_HPP__
