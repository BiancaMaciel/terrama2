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
  \file terrama2/gui/config/SurfaceDialog.hpp

  \brief Definition of Class SurfaceDialog.hpp

  \author Raphael Willian da Costa  
  
*/

#ifndef __TERRAMA2_GUI_CONFIG_SURFACEDIALOG_HPP__
#define __TERRAMA2_GUI_CONFIG_SURFACEDIALOG_HPP__

// Boost
#include <boost/noncopyable.hpp>

// Forward declarations
namespace Ui
{
  class SurfaceDialogForm;
}
class QAction;


class SurfaceDialog : public QDialog, private boost::noncopyable
{
  Q_OBJECT
  
  public:
    SurfaceDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);

    ~SurfaceDialog();

  private slots:
    /*!
     * \brief It defines constants mask to format the mask field. Like %a %d
     */
    void onMenuMaskClicked(QAction*);

  private:
    struct Impl;

    Impl* pimpl_;
};

#endif // __TERRAMA2_GUI_CONFIG_SURFACEDIALOG_HPP__
