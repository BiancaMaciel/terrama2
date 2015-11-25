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
  \file terrama2/gui/config/AdminApp.hpp

  \brief Main GUI for TerraMA2 Admin application.

  \author Evandro Delatin
  \author Raphael Willian da Costa
*/

#ifndef __TERRAMA2_GUI_ADMIN_ADMINAPP_HPP__
#define __TERRAMA2_GUI_ADMIN_ADMINAPP_HPP__

// TerraMA2
#include "ui_AdminAppForm.h"
#include "../core/ConfigManager.hpp"

// Boost
#include <boost/noncopyable.hpp>

// Qt
#include <QMainWindow>
#include <QList>
#include <QSharedPointer>

/*!
  \class AdminApp
 
  \brief Main dialog for TerraMA2 Administration module.
 */

class AdminAppTab;

class AdminApp : public QMainWindow, private boost::noncopyable
{
  Q_OBJECT

  public:

//! Default constructor.
    AdminApp(QWidget* parent = 0);

//! Destructor.
    ~AdminApp();

//! Fill fields
    void fillForm();

   // void save(const QString command = "", const QString parameters = "", const ConfigManager& newdata);
    void save();

    ConfigManager* getConfigManager();

signals:

protected:
    void closeEvent(QCloseEvent* close);


  private slots:

    void newRequested();
    void openRequested();
    void saveRequested();
    void saveAsRequested();
    void renameRequested();
    void removeRequested();
    void cancelRequested();

    void dbCreateDatabaseRequested();
    void dbCheckConnectionRequested();

    void manageServices();
    void showConsoles();

    void setDataChanged();
    void clearDataChanged();
    void clearFormData();
    void newFormData();
    void ondbTab();

    void itemClicked();
    void refresh();
  
  private:
  
    struct Impl;

    ConfigManager* configManager_;

    QString nameConfig_; //!< Current Configuration Name.
    bool newData_; //!< Current new configuration.    
    bool dataChanged_; //!< Indicates that the data has changed.
    int CurrentConfigIndex_; //!< Indice current configuration.

    QList<QSharedPointer<AdminAppTab>> tabs_; //!< List of TerraMA2 Administration Tabs

    Impl* pimpl_;  //!< Pimpl idiom.

    void enableFields(bool mode);
    bool searchDataList(int rowTotal, QString findName);
    bool validateDbData(QString& err);

};

#endif // __TERRAMA2_GUI_ADMIN_ADMINAPP_HPP__
