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
  \file unittest/core/MockDataSource.hpp

  \brief Mock for TerraLib Data Source

  \author Vinicius Campanha
*/

// TerraLib
#include <terralib/dataaccess/datasource/DataSource.h>

// GMock
#include <gmock/gmock.h>

// TerraMA2
#include "MockDataSourceTransactor.hpp"


namespace te
{
  namespace da
  {
    class MockDataSource: public te::da::DataSource
    {
    public:

      MockDataSource(){ };

      virtual ~MockDataSource() = default;


      MOCK_METHOD0(close, void());

      MOCK_METHOD1(getTransactor, std::auto_ptr<DataSourceTransactor>());

      MOCK_METHOD1(dataSetExists, bool(const std::string& name));


      //MOCK_METHOD1(verifyURL,CURLcode(std::string url));
    };
  }
}
