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
  \file terrama2/core/data-access/DataAccessorFile.hpp

  \brief

  \author Jano Simas
 */

#ifndef __TERRAMA2_CORE_DATA_ACCESS_DATA_ACCESSOR_FILE_HPP__
#define __TERRAMA2_CORE_DATA_ACCESS_DATA_ACCESSOR_FILE_HPP__

//TerraMA2
#include "../core/shared.hpp"
#include "../core/data-access/DataAccessor.hpp"
#include "../core/data-model/DataSet.hpp"
#include "../core/data-model/Filter.hpp"

namespace terrama2
{
  namespace core
  {
    /*!
      \class DataAccessorFile

      Base class for DataAccessor classes that access a file.

    */
    class DataAccessorFile : public virtual DataAccessor
    {
    public:
      // Doc in base class
      virtual std::string retrieveData(const DataRetrieverPtr dataRetriever, const DataSet& dataset, const Filter& filter) const override;
      // Doc in base class
      virtual std::shared_ptr<te::mem::DataSet> getDataSet(const std::string& uri, const Filter& filter, const DataSet& dataSet) const override;
      //! Recover file mask
      virtual std::string getMask(const DataSet& dataset) const;
    };
  }
}

#endif // __TERRAMA2_CORE_DATA_ACCESS_DATA_ACCESSOR_FILE_HPP__
