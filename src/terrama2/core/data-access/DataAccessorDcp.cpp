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
  \file terrama2/core/data-access/DataAccessorDcp.cpp

  \brief

  \author Jano Simas
 */

//TerraMA2
#include "DataAccessorDcp.hpp"
#include "DataRetriever.hpp"
#include "../utility/Factory.hpp"
#include "../utility/Logger.hpp"
#include "../Exception.hpp"

//TerraLib
#include <terralib/dataaccess/datasource/DataSource.h>
#include <terralib/memory/DataSet.h>

//Qt
#include <QObject>

terrama2::core::DcpSeriesPtr terrama2::core::DataAccessorDcp::getDcpSeries(const Filter& filter)
{
  auto series = getSeries(filter);
  DcpSeriesPtr dcpSeries = std::make_shared<DcpSeries>();
  for(auto& serie : series)
  {
    try
    {
      DataSetDcpPtr dataset = std::dynamic_pointer_cast<const DataSetDcp>(serie.first);
      dcpSeries->addDcp(dataset, serie.second);
    }
    catch(const std::bad_cast& exp)
    {
      QString errMsg = QObject::tr("Bad Cast to DataSetDcp");
      TERRAMA2_LOG_ERROR() << errMsg;
      continue;
    }//bad cast
  }

  return dcpSeries;
}

void terrama2::core::DataAccessorDcp::addColumns(std::shared_ptr<te::da::DataSetTypeConverter> converter, const std::shared_ptr<te::da::DataSetType>& datasetType) const
{
  for(std::size_t i = 0, size = datasetType->size(); i < size; ++i)
  {
    te::dt::Property* p = datasetType->getProperty(i);

    converter->add(i,p->clone());
  }
}

te::dt::TimeInstantTZ terrama2::core::DataAccessorDcp::lastDateTime() const
{
  //TODO: implement lastDateTime
  assert(0);
}
