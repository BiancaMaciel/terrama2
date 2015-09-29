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
  \file terrama2/gui/config/ConfigApp.hpp

  \brief Class responsible for handling WeatherTab actions

  \author Raphael Willian da Costa
*/


#ifndef __TERRAMA2_GUI_CONFIG_CONFIGAPPWEATHERTAB_HPP__
#define __TERRAMA2_GUI_CONFIG_CONFIGAPPWEATHERTAB_HPP__

// TerraMA2
#include "ConfigAppTab.hpp"

// QT
#include <QString>

class ConfigAppWeatherTab : public ConfigAppTab
{
  Q_OBJECT

  public:
    ConfigAppWeatherTab(ConfigApp* app, Ui::ConfigAppForm* ui);
    ~ConfigAppWeatherTab();

    void load();
    bool dataChanged();
    bool validate();
    void save();
    void saveServer();
    void saveGridDataSeries();
    void discardChanges(bool restore_data);

  private:
    void isValidConnection();
    void showDataSeries(bool state);
    void hidePanels(QWidget* except);
  signals:
    void serverChanged();

    //! It could be used in server operations when it was inserted or server was cancelled
    void serverDone();

  private slots:
    //! It used when user click add server btn
    void onEnteredWeatherTab();

    //! Activated when user starting editing an input
    void onWeatherTabEdited();

    //! Slot for handling importServer btn
    void onImportServer();

    //! Slot for handling if it is valid connection. TODO: ftp
    void onCheckConnection();

    //! Triggered when click datagridbtn to show datagrid modal
    void onDataGridBtnClicked();

    //! Triggered when click datapointbtn to show datapoint modal
    void onInsertPointBtnClicked();

    //! Triggered when click datapointdiffbtn to show datapointdiff modal
    void onInsertPointDiffBtnClicked();

    //! Triggered when click serverDeleteBtn to remove data provider
    void onDeleteServerClicked();

    //! Triggered when click in weatherTree to display metadata from DB
    void onWeatherDataTreeClicked(QTreeWidgetItem*);

  private:
    bool serverTabChanged_; //!< Defines if the user is inserting a server
    bool dataGridSeriesChanged_; //!< Defines if the user is inserting datagrid series
    bool dataPointSeriesChanged_; //!< Defines if the user is inserting data point series
    bool dataPointDiffSeriesChanged_; //!< Defines if the user is inserting data point series

};

#endif
