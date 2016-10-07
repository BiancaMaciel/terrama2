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
  \file src/terrama2/services/view/core/JSonUtils.cpp

  \brief Methods to convertion between a View and JSon object

  \author Vinicius Campanha
*/

// TerraMA2
#include "JSonUtils.hpp"
#include "se/Serialization.hpp"
#include "../../../core/Exception.hpp"
#include "../../../core/utility/JSonUtils.hpp"
#include "../../../core/utility/Logger.hpp"

// Terralib
#include <terralib/se/Style.h>

// Qt
#include <QJsonDocument>
#include <QJsonArray>
#include <QObject>
#include <QTemporaryFile>


QJsonObject terrama2::services::view::core::toJson(ViewPtr view)
{
  QJsonObject obj;
  obj.insert("class", QString("View"));
  obj.insert("name", QString(view->viewName.c_str()));
  obj.insert("id", static_cast<int32_t>(view->id));
  obj.insert("project_id", static_cast<int32_t>(view->projectId));
  obj.insert("service_instance_id", static_cast<int32_t>(view->serviceInstanceId));
  obj.insert("active", view->active);
  obj.insert("imageName", QString(view->imageName.c_str()));
  obj.insert("imageType", static_cast<int32_t>(view->imageType));
  obj.insert("imageResolutionWidth", static_cast<int32_t>(view->imageResolutionWidth));
  obj.insert("imageResolutionHeight", static_cast<int32_t>(view->imageResolutionHeight));
  obj.insert("srid", static_cast<int32_t>(view->srid));
  obj.insert("maps_server_uri", QString(view->maps_server_uri.uri().c_str()));
  obj.insert("schedule", terrama2::core::toJson(view->schedule));

  DataSeriesId dataSeriesID = view->dataSeriesList.at(0);

  obj.insert("dataseries_id", static_cast<int32_t>(dataSeriesID));

  // Style serialization
  {
    for(auto& it : view->stylesPerDataSeries)
    {
      QTemporaryFile file;
      if(!file.open())
        throw Exception() << ErrorDescription("Could not create XML file!");

      se::Serialization::writeStyleGeoserverXML(it.second.get(), file.fileName().toStdString());

      QByteArray content = file.readAll();
      if(content.isEmpty())
      {
        QString errMsg = QObject::tr("Could not create XML file!");
        TERRAMA2_LOG_ERROR() << errMsg;
        throw Exception() << ErrorDescription(errMsg);
      }

      obj.insert("style", QString(content));
    }
  }

  return obj;
}


terrama2::services::view::core::ViewPtr terrama2::services::view::core::fromViewJson(QJsonObject json)
{
  if(json["class"].toString() != "View")
  {
    QString errMsg = QObject::tr("Invalid View JSON object.");
    TERRAMA2_LOG_ERROR() << errMsg;
    throw terrama2::core::JSonParserException() << ErrorDescription(errMsg);
  }

  if(!json.contains("name")
     || !json.contains("id")
     || !json.contains("project_id")
     || !json.contains("service_instance_id")
     || !json.contains("active")
     || !json.contains("dataseries_id")
     || !json.contains("style")
     || !json.contains("maps_server_uri")
     || !json.contains("schedule"))
  {
    QString errMsg = QObject::tr("Invalid View JSON object.");
    TERRAMA2_LOG_ERROR() << errMsg;
    throw terrama2::core::JSonParserException() << ErrorDescription(errMsg);
  }

  terrama2::services::view::core::View* view = new terrama2::services::view::core::View();
  terrama2::services::view::core::ViewPtr viewPtr(view);

  view->viewName = json["name"].toString().toStdString();
  view->id = static_cast<uint32_t>(json["id"].toInt());
  view->projectId = static_cast<uint32_t>(json["project_id"].toInt());
  view->serviceInstanceId = static_cast<uint32_t>(json["service_instance_id"].toInt());
  view->active = json["active"].toBool();

  uint32_t dataseriesID = static_cast<uint32_t>(json["dataseries_id"].toInt());

  view->dataSeriesList.push_back(dataseriesID);

  if(!json["style"].toString().isEmpty())
  {
    QTemporaryFile file;

    if(!file.open())
    {
      QString errMsg = QObject::tr("Could not load the XML file!");
      TERRAMA2_LOG_ERROR() << errMsg;
      throw Exception() << ErrorDescription(errMsg);
    }

    file.write(json["style"].toString().toUtf8());
    file.flush();

    std::unique_ptr<te::se::Style> style = se::Serialization::readStyleXML(file.fileName().toStdString());

    view->stylesPerDataSeries.emplace(dataseriesID,
                                      std::unique_ptr<te::se::Style>(style.release()));
  }

  view->maps_server_uri = te::core::URI(json["maps_server_uri"].toString().toStdString());
  view->schedule = terrama2::core::fromScheduleJson(json["schedule"].toObject());


  view->imageName = json["imageName"].toString().toStdString();
  view->imageType = te::map::ImageType(json["imageType"].toInt());
  view->imageResolutionWidth = static_cast<uint32_t>(json["imageResolutionWidth"].toInt());
  view->imageResolutionHeight = static_cast<uint32_t>(json["imageResolutionHeight"].toInt());
  view->srid = static_cast<uint32_t>(json["srid"].toInt());

  return viewPtr;
}
