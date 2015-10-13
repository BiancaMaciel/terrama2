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
  \file terrama2/core/DataProviderDAO.hpp

  \brief DataProvider DAO...

  \author Paulo R. M. Oliveira
*/

// TerraMA2
#include "DataProviderDAO.hpp"
#include "DataProvider.hpp"
#include "DataSetDAO.hpp"
#include "Exception.hpp"
#include "Utils.hpp"

// TerraLib
#include <terralib/dataaccess/dataset/DataSetType.h>
#include <terralib/dataaccess/datasource/DataSourceTransactor.h>
#include <terralib/memory/DataSet.h>
#include <terralib/memory/DataSetItem.h>

// Qt
#include <QObject>

//Boost
#include <boost/format.hpp>

static const std::string dataSetName = "terrama2.data_provider";

std::vector<terrama2::core::DataProviderPtr>
terrama2::core::DataProviderDAO::load(te::da::DataSourceTransactor& transactor)
{
  std::vector<terrama2::core::DataProviderPtr> vecProviders;

  try
  {
    std::auto_ptr<te::da::DataSet> dataSet = transactor.getDataSet(dataSetName);

    while(dataSet->moveNext())
    {
      terrama2::core::DataProvider::Kind kind = IntToDataProviderKind(dataSet->getInt32("kind"));
      std::string name = dataSet->getAsString("name");

      DataProviderPtr provider(new DataProvider(name, kind));

      provider->setId(dataSet->getInt32("id"));
      provider->setDescription(dataSet->getString("description"));
      provider->setUri(dataSet->getString("uri"));
      provider->setStatus(BoolToDataProviderStatus(dataSet->getBool("active")));

      DataSetDAO::load(*provider, transactor);

      vecProviders.push_back(provider);
    }
  }
  catch(const terrama2::Exception&)
  {
    throw;
  }
  catch(const std::exception& e)
  {
    throw DataAccessError() << ErrorDescription(e.what());
  }
  catch(...)
  {
    throw DataAccessError() << ErrorDescription(QObject::tr("Could not retrieve the data provider list."));
  }

  return std::move(vecProviders);
}

void terrama2::core::DataProviderDAO::save(DataProvider& dataProvider,
                                           te::da::DataSourceTransactor& transactor,
                                           const bool shallowSave)
{
  if(dataProvider->id() != 0)
  {
    throw InvalidParameterError() <<
          ErrorDescription(QObject::tr("Can not save a data provider with identifier different than 0."));
  }

  try
  {
    boost::format query("INSERT INTO terrama2.data_provider (name, description, kind, uri, active) VALUES('%1%', '%2%', %3%, '%4%', %5%)");

    query.bind_arg(1, dataProvider->name());
    query.bind_arg(2, dataProvider->description());
    query.bind_arg(3, (int)dataProvider->kind());
    query.bind_arg(4, dataProvider->uri());
    query.bind_arg(5, BoolToString(DataProviderStatusToBool(dataProvider->status())));

    transactor.execute(query.str());

    dataProvider->setId(transactor.getLastGeneratedId());

    if(!shallowSave)
    {
      // save all datasets in this provider, their id must be zero
      for(auto dataset: dataProvider.dataSets())
      {
        DataSetDAO::save(*dataset, transactor);
      }
    }

  }
  catch(const terrama2::Exception&)
  {
    throw;
  }
  catch(const std::exception& e)
  {
    throw DataAccessError() << ErrorDescription(e.what());
  }
  catch(...)
  {
    throw DataAccessError() << ErrorDescription(QObject::tr("Could not save the data provider."));
  }
}

void terrama2::core::DataProviderDAO::update(DataProvider& dataProvider,
                                             te::da::DataSourceTransactor& transactor,
                                             const bool shallowSave)
{
  if(dataProvider.id() == 0)
  {
    throw InvalidParameterError() <<
          ErrorDescription(QObject::tr("Can not update a data provider with identifier: 0."));
  }

  try
  {
    boost::format query("UPDATE terrama2.data_provider SET name = '%1%', description = '%2%', kind = %3%, uri = '%4%', active = %5% WHERE id = %6%");

    query.bind_arg(1, dataProvider.name());
    query.bind_arg(2, dataProvider.description());
    query.bind_arg(3, (int)dataProvider.kind());
    query.bind_arg(4, dataProvider.uri());
    query.bind_arg(5, BoolToString(DataProviderStatusToBool(dataProvider.status())));
    query.bind_arg(6, dataProvider.id());

    transactor.execute(query.str());

    if(!shallowSave)
    {
      for(auto dataset: dataProvider.dataSets())
      {
        DataSetDAO::update(*dataset, transactor);
      }
    }
  }
  catch(const terrama2::Exception&)
  {
    throw;
  }
  catch(const std::exception& e)
  {
    throw DataAccessError() << ErrorDescription(e.what());
  }
  catch(...)
  {
    throw DataAccessError() << ErrorDescription(QObject::tr("Could not update the data provider."));
  }
}

void terrama2::core::DataProviderDAO::remove(const uint64_t id, te::da::DataSourceTransactor& transactor)
{
  if(id == 0)
  {
    throw InvalidParameterError() <<
          ErrorDescription(QObject::tr("Can not remove a data provider with identifier: 0."));
  }

  try
  {
    boost::format query("DELETE FROM terrama2.data_provider WHERE id = %1%");
    query.bind_arg(1, id);

    transactor.execute(query.str());
  }
  catch(const terrama2::Exception&)
  {
    throw;
  }
  catch(const std::exception& e)
  {
    throw DataAccessError() << ErrorDescription(e.what());
  }
  catch(...)
  {
    throw DataAccessError() << ErrorDescription(QObject::tr("Could not remove the data provider."));
  }
}


std::unique_ptr<terrama2::core::DataProvider>
terrama2::core::DataProviderDAO::load(const uint64_t id, te::da::DataSourceTransactor& transactor)
{
  if(id == 0)
    throw InvalidParameterError() << ErrorDescription(QObject::tr("Can not load a data provider with identifier: 0."));

  try
  {
    boost::format query("SELECT * FROM terrama2.data_provider WHERE id = %1%");
    query.bind_arg(1, id);

    std::auto_ptr<te::da::DataSet> dataSet = transactor.query(query.str());

    if(dataSet->moveNext())
    {
      terrama2::core::DataProvider::Kind kind = IntToDataProviderKind(dataSet->getInt32("kind"));
      std::string name = dataSet->getAsString("name");

      std::unique_ptr<DataProvider> provider(new DataProvider(name, kind));
      provider->setId(dataSet->getInt32("id"));
      provider->setDescription(dataSet->getString("description"));
      provider->setUri(dataSet->getString("uri"));
      provider->setStatus(BoolToDataProviderStatus(dataSet->getBool("active")));

      DataSetDAO::load(provider, transactor);

      return provider;
    }
  }
  catch(const terrama2::Exception&)
  {
    throw;
  }
  catch(const std::exception& e)
  {
    throw DataAccessError() << ErrorDescription(e.what());
  }
  catch(...)
  {
    throw DataAccessError() << ErrorDescription(QObject::tr("Could not remove the data provider."));
  }

  return std::unique_ptr<DataProvider>(nullptr);


}


