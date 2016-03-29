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
  \file terrama2/core/data-access/DataAccessorDcpPostGIS.hpp

  \brief

  \author Jano Simas
 */

#include "DataAccessorDcpPostGIS.hpp"
#include "../core/utility/Raii.hpp"

//TerraLib
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/dataaccess/datasource/DataSourceFactory.h>
#include <terralib/dataaccess/datasource/DataSourceTransactor.h>

//QT
#include <QUrl>
#include <QObject>

terrama2::core::DataAccessorDcpPostGIS::DataAccessorDcpPostGIS(DataProviderPtr dataProvider, DataSeriesPtr dataSeries, const Filter& filter)
 : DataAccessor(dataProvider, dataSeries, filter),
   DataAccessorDcp(dataProvider, dataSeries, filter),
   DataAccessorPostGis(dataProvider, dataSeries, filter)
{
  if(dataSeries->semantics.name != "PCD-postgis")
  {
    QString errMsg = QObject::tr("Wrong DataSeries semantics.");
    TERRAMA2_LOG_ERROR() << errMsg;
    throw WrongDataSeriesSemanticsException()  << ErrorDescription(errMsg);;
  }
}

std::string terrama2::core::DataAccessorDcpPostGIS::getTableName(DataSetPtr dataSet) const
{
  try
  {
    return dataSet->format.at("table_name");
  }
  catch (...)
  {
    QString errMsg = QObject::tr("Undefined table name in dataset: %1.").arg(dataSet->id);
    TERRAMA2_LOG_ERROR() << errMsg;
    throw UndefinedTagException() << ErrorDescription(errMsg);
  }
}

std::string terrama2::core::DataAccessorDcpPostGIS::getDateTimeColumnName(DataSetPtr dataSet) const
{
  try
  {
    return dataSet->format.at("date_time_column");
  }
  catch (...)
  {
    QString errMsg = QObject::tr("Undefined table name in dataset: %1.").arg(dataSet->id);
    TERRAMA2_LOG_ERROR() << errMsg;
    throw UndefinedTagException() << ErrorDescription(errMsg);
  }
}

std::string terrama2::core::DataAccessorDcpPostGIS::getGeometryColumnName(DataSetPtr dataSet) const
{
  try
  {
    return dataSet->format.at("geometry_column");
  }
  catch (...)
  {
    QString errMsg = QObject::tr("Undefined table name in dataset: %1.").arg(dataSet->id);
    TERRAMA2_LOG_ERROR() << errMsg;
    throw UndefinedTagException() << ErrorDescription(errMsg);
  }
}

std::string terrama2::core::DataAccessorDcpPostGIS::dataSourceType() const
{
  return "POSTGIS";
}
