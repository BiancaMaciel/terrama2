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
  \file terrama2/ws/collector/client/Client.hpp

  \brief Client header of TerraMA2 Collector Web Service.

  \author Vinicius Campanha
 */

#ifndef __TERRAMA2_WS_COLLECTOR_CLIENT_HPP__
#define __TERRAMA2_WS_COLLECTOR_CLIENT_HPP__

// STL
#include <string>

// gSOAP
#include "soapWebProxy.h"
#include "Web.nsmap"

// TerraMA2
#include "../../../core/DataProvider.hpp"
#include "../../../core/DataSet.hpp"

namespace terrama2
{

  namespace ws
  {

    namespace collector
    {

      class Client
      {
        public:

          Client(std::string url);

          ~Client();

          int ping(std::string& answer);

          int addDataProvider(terrama2::core::DataProvider dataProvider);

          int addDataset(DataSet struct_dataSet);

          int updateDataProvider(DataProvider struct_dataProvider);

          int updateDataSet(DataSet struct_dataSet);

          int removeDataProvider(uint64_t id);

          int removeDataSet(uint64_t id);

          int findDataProvider(uint64_t id, DataProvider &struct_dataProvider);

          int findDataSet(uint64_t id,DataSet &struct_dataSet);

          int listDataProvider(std::vector< DataProvider > &dataProviderList);

          int listDataSet(std::vector< DataSet > &dataSetList);


        private:

          WebProxy* wsClient_;

    };

    }
  }

}

#endif // __TERRAMA2_WS_COLLECTOR_CLIENT_HPP__
