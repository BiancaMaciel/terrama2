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
  \file terrama2/core/data-access/GridSeries.cpp

  \brief

  \author Jano Simas
  \author Vinicius Campanha
 */

//TerraMA2
#include "GridSeries.hpp"
#include "../../Config.hpp"
#include "../Shared.hpp"
#include "../data-model/DataSetGrid.hpp"
#include "../utility/Logger.hpp"

//STL
#include <vector>

//TerraLib
#include <terralib/dataaccess/utils/Utils.h>

//Qt
#include <QString>
#include <QObject>


void terrama2::core::GridSeries::addGridSeries(std::map<DataSetPtr, Series > seriesMap)
{
  dataSeriesMap_ = seriesMap;
  for(const auto& item : seriesMap)
  {
    try
    {
      DataSetGridPtr dataSet = std::dynamic_pointer_cast<const DataSetGrid>(item.first);

      auto teDataSet = item.second.teDataSet;
      while(teDataSet->moveNext())
      {
        std::size_t rpos = te::da::GetFirstPropertyPos(teDataSet.get(), te::dt::RASTER_TYPE);
        std::shared_ptr<te::rst::Raster> raster(teDataSet->getRaster(rpos));
        rasterMap_.emplace(dataSet, raster);
      }
    }
    catch(const std::bad_cast& exp)
    {
      QString errMsg = QObject::tr("Bad Cast to DataSetGrid");
      TERRAMA2_LOG_ERROR() << errMsg;
      continue;
    }//bad cast
  }
}

const std::map<terrama2::core::DataSetGridPtr, std::shared_ptr<te::rst::Raster> >& terrama2::core::GridSeries::gridList()
{
  return rasterMap_;
}

