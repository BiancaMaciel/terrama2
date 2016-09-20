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
    std::unique_ptr<te::se::Style> style(CreateFeatureTypeStyle(te::gm::PolygonType, "#00c290"));

    // Make sure to have a geoServer running on the port 8080
    terrama2::services::view::data_access::GeoServer geoserver("http://localhost", "8080", "admin", "geoserver");

    QTemporaryFile file;
    if(!file.open())
    {
      std::cout << std::endl << "Could not create XML file!" << std::endl;
      return EXIT_FAILURE;
    }

    terrama2::services::view::core::writeStyleGeoserverXML(style.get(), file.fileName().toStdString());

    QByteArray content = file.readAll();
    if(content.isEmpty())
    {
      std::cout << std::endl << "Could not create XML file!" << std::endl;
      return EXIT_FAILURE;

    }

    geoserver.registerStyle("aname", QString(content).toStdString());

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
