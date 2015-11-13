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
  \file terrama2/collector/Factory.cpp

  \brief Instantiate collectors .

  \author Jano Simas
*/

#include "../core/DataManager.hpp"
#include "../core/DataSetItem.hpp"

#include "StoragerPostgis.hpp"
#include "DataRetriever.hpp"
#include "ParserPostgis.hpp"
#include "ParserOGR.hpp"
#include "Exception.hpp"
#include "Factory.hpp"

//std
#include <memory>
#include <algorithm>

//Qt
#include <QUrl>


terrama2::collector::ParserPtr terrama2::collector::Factory::makeParser(const std::string& uri, const terrama2::core::DataSetItem& datasetItem)
{
  QUrl url(uri.c_str());
  if(url.scheme().toLower() == "postgis")
  {
    ParserPtr newParser = std::make_shared<ParserPostgis>();
    return newParser;
  }

  if(url.scheme().toLower() == "file")
  {
    switch (datasetItem.kind()) {
      case core::DataSetItem::PCD_INPE_TYPE:
      case core::DataSetItem::PCD_TOA5_TYPE:
      case core::DataSetItem::FIRE_POINTS_TYPE:
      case core::DataSetItem::UNKNOWN_TYPE:
      {
        ParserPtr newParser = std::make_shared<ParserOGR>();
        return newParser;
      }
      default:
        break;
    }
  }

  //FIXME: throw here
  return ParserPtr();
}

terrama2::collector::StoragerPtr terrama2::collector::Factory::makeStorager(const core::DataSetItem &datasetItem)
{
  std::map<std::string, std::string> storageMetadata = datasetItem.storageMetadata();

  if(storageMetadata.empty())
  {
    storageMetadata = { {"KIND", "POSTGIS"},
                        {"PG_HOST", "localhost"},
                        {"PG_PORT", "5432"},
                        {"PG_USER", "postgres"},
                        {"PG_PASSWORD", "postgres"},
                        {"PG_DB_NAME", "basedeteste"},
                        {"PG_CONNECT_TIMEOUT", "4"},
                        {"PG_CLIENT_ENCODING", "UTF-8"},
                        {"PG_SCHEME", "terrama2"},
                        {"PG_TABLENAME", "teste_ogr"} };
  }

  //Exceptions

  std::map<std::string, std::string>::const_iterator localFind = storageMetadata.find("KIND");

  if(localFind == storageMetadata.cend())
    return StoragerPtr();

  std::string storagerKind = localFind->second;

  //to lower case
  std::transform(storagerKind.begin(), storagerKind.end(), storagerKind.begin(), ::tolower);
  if(storagerKind == "postgis")
  {
    return std::make_shared<StoragerPostgis>(storageMetadata);
  }


  //FIXME: throw here
  return StoragerPtr();
}

terrama2::collector::DataRetrieverPtr terrama2::collector::Factory::makeRetriever(const terrama2::core::DataProvider& dataProvider)
{
  return std::make_shared<DataRetriever>(dataProvider);
}
