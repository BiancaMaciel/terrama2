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
  \file src/terrama2/services/alert/core/JSonUtils.cpp

  \brief

  \author Jano Simas
*/

#include "JSonUtils.hpp"
#include "../../../core/utility/JSonUtils.hpp"
#include "../../../core/utility/Logger.hpp"

// Qt
#include <QJsonDocument>
#include <QJsonArray>
#include <QObject>

terrama2::services::alert::core::AlertPtr terrama2::services::alert::core::fromAlertJson(QJsonObject json)
{
  if(json["class"].toString() != "Alert")
  {
    QString errMsg = QObject::tr("Invalid Alert JSON object.");
    TERRAMA2_LOG_ERROR() << errMsg;
    throw terrama2::core::JSonParserException() << ErrorDescription(errMsg);
  }

  if(!(json.contains("id")
       && json.contains("project_id")
       && json.contains("active")
       && json.contains("name")
       && json.contains("description")
       && json.contains("service_instance_id")
       && json.contains("risk")
       && json.contains("schedule")
       && json.contains("filter")
       && json.contains("additional_data")))
  {
    QString errMsg = QObject::tr("Invalid Alert JSON object.");
    TERRAMA2_LOG_ERROR() << errMsg;
    throw terrama2::core::JSonParserException() << ErrorDescription(errMsg);
  }


  terrama2::services::alert::core::Alert* alert = new terrama2::services::alert::core::Alert();
  terrama2::services::alert::core::AlertPtr alertPtr(alert);

  alert->id = static_cast<uint32_t>(json["id"].toInt());
  alert->projectId = static_cast<uint32_t>(json["project_id"].toInt());
  alert->active = json["active"].toBool();
  alert->name = json["name"].toString().toStdString();
  alert->description = json["description"].toString().toStdString();
  alert->serviceInstanceId = static_cast<uint32_t>(json["service_instance_id"].toInt());
 
  alert->risk = terrama2::core::fromDataSeriesRiskJson(json["risk"].toObject());
  alert->schedule = terrama2::core::fromScheduleJson(json["schedule"].toObject());
  alert->filter = terrama2::core::fromFilterJson(json["filter"].toObject());

  auto addDataArray = json["additional_data"].toArray();
  for(const auto& value : addDataArray)
  {
    auto obj = value.toObject();
    auto id = static_cast<uint32_t>(obj["dataseries_id"].toInt());

    std::vector<std::string> attributes;
    auto attributesArray = obj["attributes"].toArray();
    for(const auto& tempAttribute : attributesArray)
      attributes.push_back(tempAttribute.toString().toStdString());

    alert->additionalDataMap.emplace(id, attributes);
  }

  return nullptr;
}

QJsonObject terrama2::services::alert::core::toJson(AlertPtr alert)
{
  QJsonObject obj;
  //TODO: alert to json

  return obj;
}
