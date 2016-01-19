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
  \file terrama2/gui/core/Utils.cpp

  \brief The base gui module utils used

  \author Evandro Delatin
  \author Raphael Willian da Costa
*/

#ifndef __TERRAMA2_GUI_CORE_UTILS_HPP__
#define __TERRAMA2_GUI_CORE_UTILS_HPP__

// TerraMA2
#include "../../core/DataSet.hpp"

// STD
#include <string>
#include <map>

// Boost
#include <boost/noncopyable.hpp>


// Forward declaration
class QString;
class QMainWindow;
class QJsonObject;
class QMenu;
namespace terrama2
{
  namespace gui
  {
    namespace core
    {
      class ConfigManager;
    }
  }
}


namespace terrama2
{
  namespace gui
  {
    namespace core
    {
      /*!
        \brief Function to generate file destination of TerraMA2 File
        \param appFocus A pointer to QMainWindow for handling the ownership module
        \param destination A reference string containing where is to save file
        \param json A QJsonObject reference containing the metadata to be saved.
      */
      void saveTerraMA2File(QMainWindow* appFocus, const QString& destination, const QJsonObject& json);

      /*!
        \brief It makes dateset item storage metadata from configuration
        \param metadata DataSetItem metadata.
        \param uri An uri string.
        \param configuration A reference to terrama2 configuration class (ConfigManager)
        \return std::map<std::string, std::string> containing the storage metadata values from configuration
      */
      std::map<std::string, std::string> makeStorageMetadata(terrama2::core::DataSet::Kind kind, std::map<std::string, std::string> metadata, const QString uri,
                                                             const terrama2::gui::core::ConfigManager& configuration);

      /*!
        \brief It makes mask helpers for DataSet. It creates a menu bar with common items.
        \return A pointer to QMenuBar filled out.
      */
      QMenu* makeMaskHelpers();
    } // end core
  }   // end gui
}     // end terrama2

#endif // __TERRAMA2_GUI_CORE_UTILS_HPP__
