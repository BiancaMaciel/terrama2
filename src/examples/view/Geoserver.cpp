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
  \file examples/view/GeoServer.cpp

  \brief

  \author Vinicius Campanha
 */

// Qt
#include <QTemporaryFile>

// TerraMA2
#include "Utils.hpp"

#include <terrama2/core/utility/Utils.hpp>
#include <terrama2/impl/Utils.hpp>
#include <terrama2/services/view/data-access/Geoserver.hpp>
#include <terrama2/services/view/core/JSonUtils.hpp>


int main(int argc, char** argv)
{
  terrama2::core::initializeTerraMA();
  terrama2::core::registerFactories();

  try
  {
    // Make sure to have a geoServer with the below configuration
    te::core::URI uri("http://admin:geoserver@localhost:8080/geoserver");
    terrama2::services::view::data_access::GeoServer geoserver(uri);

    geoserver.registerWorkspace("exampleworkspace");

    geoserver.uploadVectorFile("example", "/home/vinicius/MyDevel/terrama2/build-debug/data/shapefile/shapefile.zip");

    std::unique_ptr<te::se::Style> style(CreateFeatureTypeStyle(te::gm::PolygonType, "#00c290"));

//    geoserver.registerStyle("an_another_example", style);
  }
  catch(const std::exception& e)
  {
    std::cout << std::endl << "An exception has occurred in GeoServer example: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch(...)
  {
    std::cout << std::endl << "An unexpected exception has occurred in GeoServer example!" << std::endl;
    return EXIT_FAILURE;
  }

  terrama2::core::finalizeTerraMA();

  return EXIT_SUCCESS;
}
