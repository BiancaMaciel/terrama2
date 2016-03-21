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
  \file terrama2/core/data-access/DataAccessor.hpp

  \brief

  \author Jano Simas
 */

#ifndef __TERRAMA2_CORE_DATA_ACCESS_DATA_ACCESSOR_HPP__
#define __TERRAMA2_CORE_DATA_ACCESS_DATA_ACCESSOR_HPP__

//TerraMA2
#include "../../Config.hpp"

#include "../shared.hpp"

#include "DataRetriever.hpp"
#include "../data-model/DataSeriesSemantic.hpp"
#include "../data-model/DataProvider.hpp"
#include "../data-model/Filter.hpp"

//TerraLib
#include <terralib/dataaccess/dataset/DataSetTypeConverter.h>
#include <terralib/datatype/TimeInstantTZ.h>
#include <terralib/memory/DataSet.h>

namespace te {
  namespace da {
    class DataSet;
  } /* da */
} /* te */

namespace terrama2
{
  namespace core
  {
    /*!
    \class DataAccessor
    \brief Base class to access data from a DataSeries.

    Derived classes as responsible for the whole data access process,
    from downloading, when necessary, to instantiating the proper data access class.

    Derived classes should also be able read the format data from the dataset format map.

    */
    class DataAccessor
    {
    public:
      //! Returns the last Data date found on last access.
      virtual te::dt::TimeInstantTZ lastDateTime() const = 0;
      DataSeriesSemantic semantic() const { return dataSeries_.semantic; }

      //! Utility function for converting string to double in the te::da::DataSet contruction.
      te::dt::AbstractData* stringToDouble(te::da::DataSet* dataset, const std::vector<std::size_t>& indexes, int /*dstType*/) const;
    protected:

      /*!
        \brief TODO: doc DataAccessor

        \param filter If defined creates a cache for the filtered data.
      */
      DataAccessor(DataProvider dataProvider, DataSeries dataSeries, Filter filter = Filter())
        : dataProvider_(dataProvider),
          dataSeries_(dataSeries),
          filter_(filter) {}

      virtual ~DataAccessor() {}

      /*!
         \brief Prefix especification for drivers.

         Some drivers may need especification to access a datasource,
         GDal, for example, need 'CSV:' befor the uri for csv files with txt extension.

       */
      virtual std::string typePrefix() const { return ""; }

      /*!
         \brief Driver that must be used to access a DataSource
       */
      virtual std::string dataSourceTye() const = 0;

      /*!
         \brief Creates a converter to make the necessary convertions of data type.

         This converter will convert strings to Data/Time formats or double,
         create coordinate point classe from lat long string and other cases

         \param dataSet Raw dataset
         \param datasetType DataSetType from dataSet
         \return Converter
       */
      virtual std::shared_ptr<te::da::DataSetTypeConverter> getConverter( const DataSet& dataSet, const std::shared_ptr<te::da::DataSetType>& datasetType) const;

      /*!
         \brief Add original attributes to the converter without convertion
       */
      virtual void addColumns(std::shared_ptr<te::da::DataSetTypeConverter> converter, const std::shared_ptr<te::da::DataSetType>& datasetType) const { }
      /*!
         \brief Add addapted attributes to the converter
       */
      virtual void adapt(const DataSet& dataSet, std::shared_ptr<te::da::DataSetTypeConverter> converter) const { }

      /*!
         \brief Retrieve data from server.

         Retrieved data is subjetc to filter.

       */
      virtual std::string retrieveData(const DataRetrieverPtr dataRetriever, const DataSet& dataSet, const Filter& filter) const = 0;

      /*!
         \brief Get a memory dataset do core::DataSet.
         \param uri Uri to the dataset storage
         \param filter Filter applyed to the dataset
         \return Filtered dataset
       */
      virtual std::shared_ptr<te::mem::DataSet> getDataSet(const std::string& uri, const Filter& filter, const DataSet& dataSet) const = 0;

      virtual std::map<std::shared_ptr<DataSet>, std::shared_ptr<te::mem::DataSet>> getSeries(const Filter& filter) const;


      DataProvider dataProvider_;
      DataSeries dataSeries_;
      Filter filter_;
    };
  }
}

#endif // __TERRAMA2_CORE_DATA_ACCESS_DATA_ACCESSOR_HPP__
