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
  \file terrama2/gui/admin/AdminAppTab.cpp

  \brief 

  \author Evandro Delatin
  \author Raphael Willian da Costa
  
*/

// TerraMA2
#include "AdminApp.hpp"
#include "AdminAppDBTab.hpp"  
#include "ui_AdminAppForm.h"
#include "Exception.hpp"
#include <QJsonObject>
#include <QString>

AdminAppDBTab::AdminAppDBTab(AdminApp* app, Ui::AdminAppForm* ui)
 :AdminAppTab(app, ui)
{

} 

AdminAppDBTab::~AdminAppDBTab()
{
  
}

QMap<QString, QJsonObject> AdminAppDBTab::toJson()
{

 QJsonObject database;

 database["name"] = ui_->dbDatabaseLed->text();
 database["driver"] = ui_->dbTypeCmb->currentText();
 database["host"] = ui_->dbAddressLed->text();
 database["port"] = ui_->dbPortLed->text();
 database["user"] = ui_->dbUserLed->text();
 database["password"] = ui_->dbPasswordLed->text();
 QMap<QString, QJsonObject> json;
 json.insert(QString("database"), database);

 return json;
}

bool AdminAppDBTab::load()
{
    return false;
}

bool AdminAppDBTab::validate()
{
    return false;
}
