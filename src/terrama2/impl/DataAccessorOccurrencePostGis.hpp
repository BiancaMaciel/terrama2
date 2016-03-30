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
  \file terrama2/core/data-access/DataAccessorOccurrencePostGis.hpp

  \brief

  \author Jano Simas
 */

#ifndef __TERRAMA2_CORE_DATA_ACCESS_DATA_ACCESSOR_OCCURRENCE_POSTGIS_HPP__
#define __TERRAMA2_CORE_DATA_ACCESS_DATA_ACCESSOR_OCCURRENCE_POSTGIS_HPP__

//TerraMA2
#include "DataAccessorPostGis.hpp"

#include "../core/shared.hpp"
#include "../core/data-access/DataAccessorOccurrence.hpp"

namespace terrama2
{
  namespace core
  {
    class DataAccessorOccurrencePostGis : public DataAccessorOccurrence, public DataAccessorPostGis
    {
    public:
      DataAccessorOccurrencePostGis(DataProviderPtr dataProvider, DataSeriesPtr dataSeries, const Filter& filter = Filter());
      virtual ~DataAccessorOccurrencePostGis() {}

    protected:
      virtual std::string getTableName(DataSetPtr dataSet) const override;
      virtual std::string getDateTimeColumnName(DataSetPtr dataSet) const override;
      virtual std::string getGeometryColumnName(DataSetPtr dataSet) const override;

      virtual std::string dataSourceType() const override;
    };
  }
}

#endif // __TERRAMA2_CORE_DATA_ACCESS_DATA_ACCESSOR_OCCURRENCE_POSTGIS_HPP__
