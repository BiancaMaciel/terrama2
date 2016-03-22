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
  \file terrama2/core/data-access/DataAccessor.cpp

  \brief

  \author Jano Simas
 */

#include "DataAccessor.hpp"
#include "../utility/Logger.hpp"
#include "../utility/Factory.hpp"

//terralib
#include <terralib/dataaccess/dataset/DataSet.h>
#include <terralib/dataaccess/dataset/DataSetType.h>
#include <terralib/datatype/SimpleData.h>
#include <terralib/datatype/Property.h>

//QT
#include <QObject>

//STL
#include <algorithm>

te::dt::AbstractData* terrama2::core::DataAccessor::stringToDouble(te::da::DataSet* dataset, const std::vector<std::size_t>& indexes, int /*dstType*/) const
{
 assert(indexes.size() == 1);

 try
 {
   std::string strValue = dataset->getAsString(indexes[0]);

   if(strValue.empty())
   {
     return nullptr;
   }
   else
   {
     double value = 0;
     std::istringstream stream(strValue);//create stream
     stream >> value;

     te::dt::SimpleData<double>* data = new te::dt::SimpleData<double>(value);

     return data;
   }
 }
 catch(std::exception& e)
 {
   TERRAMA2_LOG_ERROR() << e.what();
 }
 catch(...)
 {
   TERRAMA2_LOG_ERROR() << "Unknown error";
 }

 return nullptr;
}

std::shared_ptr<te::da::DataSetTypeConverter> terrama2::core::DataAccessor::getConverter( const DataSet& dataset, const std::shared_ptr<te::da::DataSetType>& datasetType) const
{
  std::shared_ptr<te::da::DataSetTypeConverter> converter(new te::da::DataSetTypeConverter(datasetType.get()));

  addColumns(converter, datasetType);

  adapt(dataset, converter);
  std::string id = "FID";
  const std::vector< te::dt::Property * > & propertyList = converter->getResult()->getProperties();
  auto it = std::find_if(propertyList.cbegin(), propertyList.cend(), [id](te::dt::Property *property){ return property->getName() == id; });
  if(it != propertyList.cend())
    converter->remove(id);

  return converter;
}


std::map<std::shared_ptr<terrama2::core::DataSet>, std::shared_ptr<te::mem::DataSet> > terrama2::core::DataAccessor::getSeries(const Filter& filter) const
{

  //if data provider is not active, nothing to do
  if(!dataProvider_.active)
  {
    QString errMsg = QObject::tr("Disabled data provider (Should not arrive here!)");

    throw DisabledDataProviderException() << ErrorDescription(errMsg);
    TERRAMA2_LOG_ERROR() << errMsg.toStdString();
  }

  std::map<std::shared_ptr<DataSet>, std::shared_ptr<te::mem::DataSet> > series;

  try
  {
    DataRetrieverPtr dataRetriever = Factory::MakeRetriever(dataProvider_);
    for(const auto& dataset : dataSeries_.datasetList)
    {
      //if the dataset is not active, continue to next.
      if(!dataset->active)
        continue;

      // if this data retriever is a remote server that allows to retrieve data to a file,
      // download the file to a tmeporary location
      // if not, just get the DataProvider uri
      std::string uri;
      if(dataRetriever->isRetrivable())
        uri = retrieveData(dataRetriever, *dataset, filter);
      else
        uri = dataProvider_.uri;

      //TODO: Set last date collected in filter
      std::shared_ptr<te::mem::DataSet> memDataSet = getDataSet(uri, filter, *dataset);

      series.emplace(dataset, memDataSet);
    }//for each dataset
  }
  catch(...)
  {
    //TODO: catch cannot open DataProvider, log here
    assert(0);
  }

  return series;
}
