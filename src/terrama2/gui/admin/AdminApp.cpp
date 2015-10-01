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
  \file terrama2/gui/admin/AdminApp.cpp

  \brief Main GUI for TerraMA2 Config application.

  \author Evandro Delatin
  \author Raphael Willian da Costa
*/

// TerraMA2
#include "AdminApp.hpp"
#include "ui_AdminAppForm.h"
#include "../Exception.hpp"
#include "../../core/Utils.hpp"
#include "AdminAppDBTab.hpp"
#include "AdminAppCollectTab.hpp"
#include "ServicesDialog.hpp"
#include "../../core/ApplicationController.hpp"

// Qt
#include <QIcon>
#include <QStringList>
#include <QToolBar>
#include <QTranslator>
#include <QFileDialog>
#include <QInputDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QList>

struct AdminApp::Impl
{
  Ui::AdminAppForm* ui_;
  
  Impl()
    : ui_(new Ui::AdminAppForm)
  {
  }

  ~Impl()
  {
    delete ui_;
  }
};

AdminApp::AdminApp(QWidget* parent)
  : QMainWindow(parent),
    pimpl_(new Impl)
{

// find icon theme library
  std::string icon_theme_path = terrama2::core::FindInTerraMA2Path("share/terrama2/icons");

  if(icon_theme_path.empty())
  {
    throw terrama2::InitializationError() << terrama2::ErrorDescription(tr("Could not find TerraMA2 icon library folder."));
  }
  
// load icon theme library
  QStringList ithemes = QIcon::themeSearchPaths();

  ithemes.push_back(icon_theme_path.c_str());

  QIcon::setThemeSearchPaths(ithemes);

  QIcon::setThemeName("terrama2");

// load idioms
  std::string idiom_path = terrama2::core::FindInTerraMA2Path("share/terrama2/translations/terrama2_admin_en_US.qm");
  
  if(idiom_path.empty())
  {
// LOG WARNING
  }
  else
  {
    QTranslator translator;
    
    translator.load(idiom_path.c_str());

    qApp->installTranslator(&translator);
  }

  pimpl_->ui_->setupUi(this);

  configManager_ = new ConfigManager(this);

// Init variable
  pimpl_->ui_->configListWidget->clear();
  nameConfig_ = "";
  CurrentConfigIndex_ = 0;
  ignoreChangeEvents_ = false;
  clearDataChanged();
  enableFields(false);

 // Init services for each tab
  QSharedPointer<AdminAppTab> dbTab(new AdminAppDBTab(this,pimpl_->ui_));
  QSharedPointer<AdminAppTab> collectTab(new AdminAppCollectTab(this,pimpl_->ui_));

  tabs_.append(dbTab);
  tabs_.append(collectTab);

// Connects settings menu actions
  connect(pimpl_->ui_->newAct,  SIGNAL(triggered()), SLOT(newRequested()));
  connect(pimpl_->ui_->openAct, SIGNAL(triggered()), SLOT(openRequested()));
  connect(pimpl_->ui_->saveAct, SIGNAL(triggered()), SLOT(saveRequested()));
  connect(pimpl_->ui_->renameAct, SIGNAL(triggered()), SLOT(renameRequested()));
  connect(pimpl_->ui_->exitAct, SIGNAL(triggered()), SLOT(close()));

// Connects settings menu services
  connect(pimpl_->ui_->consoleAct,  SIGNAL(triggered()), SLOT(showConsoles()));
  connect(pimpl_->ui_->servicesAct, SIGNAL(triggered()), SLOT(manageServices()));

// Connects actions related to the interface
  connect(pimpl_->ui_->saveBtn, SIGNAL(clicked()), SLOT(saveRequested()));
  connect(pimpl_->ui_->cancelBtn, SIGNAL(clicked()), SLOT(cancelRequested()));

  connect(pimpl_->ui_->dbCreateDatabaseBtn,  SIGNAL(clicked()), SLOT(dbCreateDatabaseRequested())); 
  connect(pimpl_->ui_->configListWidget, SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(itemClicked()));

// Prepare context menu to the list of settings
  pimpl_->ui_->configListWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
  pimpl_->ui_->configListWidget->insertAction(NULL, pimpl_->ui_->renameAct);
  pimpl_->ui_->configListWidget->insertAction(NULL, pimpl_->ui_->removeAct); 
}

// New file
void AdminApp::newRequested()
{
 nameConfig_ = tr("New Configuration");

 newData_ = true;

 pimpl_->ui_->configListWidget->setCurrentRow(pimpl_->ui_->configListWidget->count()+1);

 enableFields(true);
 clearFormData();
 setDataChanged();
 renameRequested();
}

// Open file
void AdminApp::openRequested()
{
  QString filename = QFileDialog::getOpenFileName(this, tr("Choose file"), ".",
                                                        tr("TerraMA2 ( *.terrama2"));

  if (filename.isEmpty())
     return;

  configManager_->loadConfiguration(filename);


// fills fields
  fillForm();

  pimpl_->ui_->configListWidget->addItem(configManager_->getDatabase()->name_);

  enableFields(true);
}

// Search Data List
bool AdminApp::searchDataList(int row_total, QString find_name)
{
  bool find = true;

  if (row_total > 0)
  {
    for (int row = 0; row < row_total; row++)
      {
        QListWidgetItem *item = pimpl_->ui_->configListWidget->item(row);

        if (item->text() == find_name)
        {
          find = false;
          break;
        }
      }
  }
 return find;
}

// Rename file
void AdminApp::renameRequested()
{
  bool ok;
  QString newname = QInputDialog::getText(this, tr("Rename..."),
                                          tr ("New configuration name: "),
                                          QLineEdit::Normal, nameConfig_, &ok);

  if (!ok || newname.isEmpty())
  {
    clearDataChanged();
    enableFields(false);
    return;
  }

  enableFields(true);

  nameConfig_ = newname;

  ok = searchDataList(pimpl_->ui_->configListWidget->count(), newname);

  if (ok)
   {
    nameConfig_ = newname;
    pimpl_->ui_->configListWidget->addItem(nameConfig_);
    setDataChanged();
   }
   else
   {
    QMessageBox::information(this, tr("TerraMA2"), tr("Configuration Name Exists!"));
    clearDataChanged();
    enableFields(false);
   }
}

// Save file
void AdminApp::saveRequested()
{
  QJsonObject metadata;
  QString version = "4.0.0-alpha1";

  metadata["is_study"] = pimpl_->ui_->dbStudyChk->isChecked();
  metadata["name"] = nameConfig_;
  metadata["version"] = version;

  for(QSharedPointer<AdminAppTab> tab: tabs_)
  {
   QMap<QString, QJsonObject> tabJson = tab->toJson();
   metadata[tabJson.firstKey()] = tabJson.first();
  }

  QFile saveFile(nameConfig_+".terrama2");

  saveFile.open(QIODevice::WriteOnly);

  QJsonDocument jsondoc(metadata);

  saveFile.write(jsondoc.toJson());

  saveFile.close();

  QMessageBox::information(this, tr("TerraMA2"), tr("Project successfully saved!"));

  enableFields(false);

}

// Cancel
void AdminApp::cancelRequested()
{
 if (newData_)
 {
//  delete pimpl_->ui_->configListWidget->takeItem()
 }
 newData_= false;
 clearFormData();
 enableFields(false);
}

// Create Database
void AdminApp::dbCreateDatabaseRequested()
{

  configManager_->setDatabase(tabs_[0]->toJson().first());

  Database* database = configManager_->getDatabase();

  terrama2::core::ApplicationController::getInstance().createDatabase(database->dbName_.toStdString(),
                                                                      database->user_.toStdString(),
                                                                      database->password_.toStdString(),
                                                                      database->host_.toStdString(),
                                                                      database->port_ );

}

void AdminApp::manageServices()
{
 ServicesDialog dlg(this);
 dlg.exec();

}

void AdminApp::showConsoles()
{

}

//! Enable or Disable fields of form
void AdminApp::enableFields(bool mode)
 {
// action
  pimpl_->ui_->saveAct->setEnabled(mode);
  pimpl_->ui_->renameAct->setEnabled(mode);
  pimpl_->ui_->removeAct->setEnabled(mode);
  pimpl_->ui_->saveAsAct->setEnabled(mode);
  pimpl_->ui_->servicesAct->setEnabled(mode);
  pimpl_->ui_->consoleAct->setEnabled(mode);
  pimpl_->ui_->dbCreateDatabaseBtn->setEnabled(mode);
  pimpl_->ui_->dbCheckConnectionBtn->setEnabled(mode);
  pimpl_->ui_->saveBtn->setEnabled(mode);
  pimpl_->ui_->cancelBtn->setEnabled(mode);

// fields Database
  pimpl_->ui_->dbTypeCmb->setEnabled(mode);
  pimpl_->ui_->dbAddressLed->setEnabled(mode);
  pimpl_->ui_->dbPortLed->setEnabled(mode);
  pimpl_->ui_->dbUserLed->setEnabled(mode);
  pimpl_->ui_->dbPasswordLed->setEnabled(mode);
  pimpl_->ui_->dbDatabaseLed->setEnabled(mode);
  pimpl_->ui_->dbStudyChk->setEnabled(mode);

// fields Collect
  pimpl_->ui_->aqAddressLed->setEnabled(mode);
  pimpl_->ui_->aqPortLed->setEnabled(mode);
  pimpl_->ui_->aqLogFileLed->setEnabled(mode);
  pimpl_->ui_->aqTimeoutMinSpb->setEnabled(mode);
  pimpl_->ui_->aqTimeoutSecSpb->setEnabled(mode);
  pimpl_->ui_->aqDirNameLed->setEnabled(mode);

}

void AdminApp::setDataChanged()
{
  if(ignoreChangeEvents_)
    return;

  dataChanged_ = true;
  pimpl_->ui_->saveBtn->setEnabled(true);
  pimpl_->ui_->cancelBtn->setEnabled(true);
  pimpl_->ui_->dbCreateDatabaseBtn->setEnabled(true);
  pimpl_->ui_->dbCheckConnectionBtn->setEnabled(true);
}

void AdminApp::clearDataChanged()
{
  dataChanged_ = false;
  pimpl_->ui_->saveBtn->setEnabled(false);
  pimpl_->ui_->cancelBtn->setEnabled(false);
  pimpl_->ui_->dbCreateDatabaseBtn->setEnabled(false);
  pimpl_->ui_->dbCheckConnectionBtn->setEnabled(false);
}

void AdminApp::clearFormData()
{
  ignoreChangeEvents_ = true;

// fields tab Database
  pimpl_->ui_->dbTypeCmb->setCurrentIndex(0);
  pimpl_->ui_->dbAddressLed->setText("");
  pimpl_->ui_->dbPortLed->setText("");
  pimpl_->ui_->dbUserLed->setText("");
  pimpl_->ui_->dbPasswordLed->setText("");
  pimpl_->ui_->dbDatabaseLed->setText("");
  pimpl_->ui_->dbStudyChk->setChecked(false);

// fields tab collect
  pimpl_->ui_->aqAddressLed->setText("");
  pimpl_->ui_->aqPortLed->setText("");
  pimpl_->ui_->aqLogFileLed->setText("");
  pimpl_->ui_->aqTimeoutMinSpb->setValue(3);
  pimpl_->ui_->aqTimeoutSecSpb->setValue(0);
  pimpl_->ui_->aqDirNameLed->setText("");
}

void AdminApp::itemClicked()
{
  if(ignoreChangeEvents_)
   return;

   QString selectedName = pimpl_->ui_->configListWidget->currentItem()->text();
   QJsonObject selectedMetadata = configManager_->getfiles().take(selectedName);

   configManager_->setDataForm(selectedMetadata);

   fillForm();
}

AdminApp::~AdminApp()
{
  delete pimpl_;
}

// fills fields
void AdminApp::fillForm()
{
// Database tab
  pimpl_->ui_->dbTypeCmb->setCurrentIndex(pimpl_->ui_->dbTypeCmb->findText(configManager_->getDatabase()->driver_));
  pimpl_->ui_->dbAddressLed->setText(configManager_->getDatabase()->host_);
  pimpl_->ui_->dbUserLed->setText(configManager_->getDatabase()->user_);
  pimpl_->ui_->dbDatabaseLed->setText(configManager_->getDatabase()->dbName_);
  pimpl_->ui_->dbPortLed->setText(QString::number(configManager_->getDatabase()->port_));
  pimpl_->ui_->dbPasswordLed->setText(configManager_->getDatabase()->password_);
  pimpl_->ui_->dbStudyChk->setChecked(configManager_->getDatabase()->study_.toLower() == "true");
 // pimpl_->ui_->configListWidget->addItem(configManager_->getDatabase()->name_);

// Collect tab
  pimpl_->ui_->aqAddressLed->setText(configManager_->getCollection()->address_);
  pimpl_->ui_->aqPortLed->setText(QString::number(configManager_->getCollection()->servicePort_));
  pimpl_->ui_->aqLogFileLed->setText(configManager_->getCollection()->logFile_);
  pimpl_->ui_->aqDirNameLed->setText(configManager_->getCollection()->dirPath_);
  pimpl_->ui_->aqTimeoutMinSpb->setValue(configManager_->getCollection()->timeout_ / 60);
  pimpl_->ui_->aqTimeoutSecSpb->setValue(configManager_->getCollection()->timeout_ % 60);

  ignoreChangeEvents_ = false;

  clearDataChanged();

}

