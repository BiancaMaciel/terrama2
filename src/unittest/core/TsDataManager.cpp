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
  \file unittest/core/DataProviderDAO.cpp

  \brief Test for DataProviderDAO functionalities

  \author Paulo R. M. Oliveira
*/

#include "TsDataManager.hpp"
#include "Utils.hpp"

// TerraMA2
#include <terrama2/core/ApplicationController.hpp>
#include <terrama2/core/DataManager.hpp>
#include <terrama2/core/DataProvider.hpp>
#include <terrama2/core/DataSet.hpp>
#include <terrama2/core/DataSetItem.hpp>
#include <terrama2/core/Filter.hpp>
#include <terrama2/core/Utils.hpp>
#include <terrama2/core/Exception.hpp>
#include <terrama2/Exception.hpp>

//QT
#include <QtTest>
#include <QSignalSpy>

// STL
#include <memory>
#include <vector>


using namespace terrama2::core;

void TsDataManager::init()
{
  DataManager::getInstance().unload();
  clearDatabase();
}

void TsDataManager::cleanup()
{
  DataManager::getInstance().unload();
  clearDatabase();
}


void TsDataManager::clearDatabase()
{
  std::shared_ptr<te::da::DataSource> dataSource = ApplicationController::getInstance().getDataSource();

  if(!dataSource.get())
  {
    QFAIL("Invalid database connection");
  }

  std::auto_ptr<te::da::DataSourceTransactor> transactor = dataSource->getTransactor();

  if(!transactor.get())
  {
    QFAIL("Invalid database transactor");
  }

  transactor->begin();

  std::string query = "DELETE FROM terrama2.dataset";
  transactor->execute(query);

  query = "DELETE FROM terrama2.data_provider";
  transactor->execute(query);

  transactor->commit();
}

DataProvider TsDataManager::createDataProvider()
{
  auto dataProvider = DataProvider();
  dataProvider.setName("Server 1");
  dataProvider.setKind(DataProvider::FTP_TYPE);
  dataProvider.setStatus(DataProvider::ACTIVE);
  dataProvider.setDescription("This server...");
  dataProvider.setUri("localhost@...");

  return dataProvider;
}

DataSet TsDataManager::createDataSet()
{
  DataProvider dataProvider = createDataProvider();
  DataManager::getInstance().add(dataProvider);

  // create a new dataset and save it to the database
  DataSet dataSet("Queimadas", DataSet::OCCURENCE_TYPE, 0, dataProvider.id());
  dataSet.setStatus(DataSet::Status::ACTIVE);

  te::dt::TimeDuration dataFrequency(2,0,0);
  dataSet.setDataFrequency(dataFrequency);

  std::vector<DataSet::CollectRule> collectRules;
  {
    DataSet::CollectRule collectRule = { 0, "... LUA SCRIPT 1...", 0 };
    collectRules.push_back(collectRule);
  }
  {
    DataSet::CollectRule collectRule = {0, "... LUA SCRIPT 2...", 0 };
    collectRules.push_back(collectRule);
  }
  dataSet.setCollectRules(collectRules);

  std::map<std::string, std::string> metadata;
  metadata["key"] = "value";
  metadata["key1"] = "value1";
  metadata["key2"] = "value2";

  dataSet.setMetadata(metadata);


  DataSetItem dataSetItem(DataSetItem::PCD_INPE_TYPE, 0, dataSet.id());

  Filter filter(dataSetItem.id());
  filter.setExpressionType(Filter::GREATER_THAN_TYPE);
  filter.setValue(std::move(std::unique_ptr<double>(new double(100.))));
  dataSetItem.setFilter(filter);

  dataSet.add(dataSetItem);


  DataSetItem dataSetItem2(DataSetItem::FIRE_POINTS_TYPE, 0, dataSet.id());

  std::map<std::string, std::string> storageMetadata;
  storageMetadata["key"] = "value";
  storageMetadata["key1"] = "value1";
  storageMetadata["key2"] = "value2";

  dataSetItem2.setStorageMetadata(storageMetadata);

  dataSet.add(dataSetItem2);

  return dataSet;
}

void TsDataManager::testLoad()
{
  try
  {
    QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataManagerLoaded()));

    DataSet dataSet = createDataSet();
    DataManager::getInstance().add(dataSet);

    DataManager::getInstance().unload();

    DataManager::getInstance().load();

    QCOMPARE(spy.count(), 1);

    // Calling load again should have no effect
    DataManager::getInstance().load();

    QCOMPARE(spy.count(), 1);

    QVERIFY2(DataManager::getInstance().providers().size() == 1, "List should have one provider!");
    QVERIFY2(DataManager::getInstance().dataSets().size() == 1, "List should have one dataset!");
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }


}

void TsDataManager::testUnload()
{
  try
  {
    QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataManagerUnloaded()));

    DataSet dataSet = createDataSet();
    DataManager::getInstance().add(dataSet);

    DataManager::getInstance().unload();

    QVERIFY2(spy.count() == 1, "Expect an emitted signal");

    QVERIFY2(DataManager::getInstance().providers().size() == 0, "List of providers should be empty after unload!");
    QVERIFY2(DataManager::getInstance().dataSets().size() == 0, "List of datasets should be empty after unload!");
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
}



void TsDataManager::testAddDataProvider()
{
  try
  {
    qRegisterMetaType<DataProvider>("DataProvider");
    QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataProviderAdded(DataProvider)));

    DataProvider dataProvider = createDataProvider();

    DataManager::getInstance().add(dataProvider);

    QVERIFY2(spy.count() == 1, "Expect an emitted signal");

    QVERIFY2(dataProvider.id() != 0, "The id wasn't set in the provider after insert!");
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }

}

void TsDataManager::testRemoveDataProvider()
{
  try
  {
    qRegisterMetaType<DataProvider>("DataProvider");
    QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataProviderRemoved(DataProvider)));

    DataProvider dataProvider = createDataProvider();

    DataManager::getInstance().add(dataProvider);

    QVERIFY2(dataProvider.id() != 0, "The id wasn't set in the provider after insert!");

    // Removes the data provider
    DataManager::getInstance().removeDataProvider(dataProvider.id());

    QVERIFY2(spy.count() == 1, "Expect an emitted signal");

    // Lists all data providers
    auto vecDataProvider = DataManager::getInstance().providers();

    QVERIFY2(vecDataProvider.empty(), "List should be empty after remove!");
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }


}

void TsDataManager::testFindDataProvider()
{
  try
  {
    DataProvider dataProvider = createDataProvider();

    DataManager::getInstance().add(dataProvider);

    // Find the same data provider by id
    DataProvider foundDataProvider = DataManager::getInstance().findDataProvider(dataProvider.id());

    QVERIFY2(foundDataProvider.id() == dataProvider.id(), "Could not recover the data provider by id!");

    QVERIFY2("This server..." == foundDataProvider.description(), "Wrong Description in recovered provider");
    QVERIFY2("Server 1" == foundDataProvider.name(), "Wrong name in recovered provider");
    QVERIFY2(DataProvider::FTP_TYPE == foundDataProvider.kind(), "Wrong type in recovered provider");
    QVERIFY2(DataProvider::ACTIVE == foundDataProvider.status(), "Wrong status in recovered provider");
    QVERIFY2("localhost@..." == foundDataProvider.uri(), "Wrong uri in recovered provider");
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
}


void TsDataManager::testFindDataProviderByName()
{
  try
  {
    DataProvider dataProvider = createDataProvider();

    DataManager::getInstance().add(dataProvider);

    // Find the same data provider by name
    DataProvider foundDataProvider = DataManager::getInstance().findDataProvider(dataProvider.name());

    QVERIFY2(foundDataProvider.id() == dataProvider.id(), "Could not recover the data provider by id!");

    QVERIFY2("This server..." == foundDataProvider.description(), "Wrong Description in recovered provider");
    QVERIFY2("Server 1" == foundDataProvider.name(), "Wrong name in recovered provider");
    QVERIFY2(DataProvider::FTP_TYPE == foundDataProvider.kind(), "Wrong type in recovered provider");
    QVERIFY2(DataProvider::ACTIVE == foundDataProvider.status(), "Wrong status in recovered provider");
    QVERIFY2("localhost@..." == foundDataProvider.uri(), "Wrong uri in recovered provider");
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
}

void TsDataManager::testUpdateDataProvider()
{
  try
  {
    qRegisterMetaType<DataProvider>("DataProvider");
    QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataProviderUpdated(DataProvider)));

    DataProvider dataProvider = createDataProvider();

    DataManager::getInstance().add(dataProvider);

    dataProvider.setName("New server");
    dataProvider.setStatus(DataProvider::INACTIVE);
    dataProvider.setDescription("New server is ...");
    dataProvider.setUri("myserver@...");

    DataManager::getInstance().update(dataProvider);

    QVERIFY2(spy.count() == 1, "Expect an emitted signal");


    // Find the same data provider by id
    DataProvider foundDataProvider = DataManager::getInstance().findDataProvider(dataProvider.id());

    QVERIFY2(foundDataProvider.id() == dataProvider.id(), "Could not recover the data provider by id!");

    QVERIFY2(dataProvider.description() == foundDataProvider.description(), "Wrong Description after update");
    QVERIFY2(dataProvider.name() == foundDataProvider.name(), "Wrong name after update");
    QVERIFY2(dataProvider.kind() == foundDataProvider.kind(), "Wrong type after update");
    QVERIFY2(dataProvider.status() == foundDataProvider.status(), "Wrong status after update");
    QVERIFY2(dataProvider.uri() == foundDataProvider.uri(), "Wrong uri after update");
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }

}


void TsDataManager::testUpdateDataProviderShallow()
{
  try
  {
    
    DataProvider dataProvider = createDataProvider();

    DataSet dataSet("Queimadas", DataSet::OCCURENCE_TYPE, 0, dataProvider.id());
    dataSet.setStatus(DataSet::Status::ACTIVE);
    dataProvider.add(dataSet);

    DataSet dataSet2("Queimadas2", DataSet::OCCURENCE_TYPE, 0, dataProvider.id());
    dataSet.setStatus(DataSet::Status::ACTIVE);
    dataProvider.add(dataSet2);

    DataManager::getInstance().add(dataProvider);

    // Remove the first dataset
    dataProvider.removeDataSet(dataProvider.datasets()[0].id());

    // Update the name of the second
    auto& ds2 = dataProvider.datasets()[0];
    ds2.setName("New Queimadas");

    // Add a new dataset
    DataSet dataSet3("Queimadas3", DataSet::OCCURENCE_TYPE, 0, dataProvider.id());
    dataSet.setStatus(DataSet::Status::ACTIVE);
    dataProvider.add(dataSet3);

    qRegisterMetaType<DataProvider>("DataProvider");
    QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataProviderUpdated(DataProvider)));

    dataProvider.setName("New server");
    dataProvider.setStatus(DataProvider::INACTIVE);
    dataProvider.setDescription("New server is ...");
    dataProvider.setUri("myserver@...");
    
    DataManager::getInstance().update(dataProvider);
    
    QVERIFY2(spy.count() == 1, "Expect an emitted signal");
    
    
    // Find the same data provider by id
    DataProvider foundDataProvider = DataManager::getInstance().findDataProvider(dataProvider.id());
    
    QVERIFY2(foundDataProvider.id() == dataProvider.id(), "Could not recover the data provider by id!");
    
    QVERIFY2(dataProvider.description() == foundDataProvider.description(), "Wrong Description after update");
    QVERIFY2(dataProvider.name() == foundDataProvider.name(), "Wrong name after update");
    QVERIFY2(dataProvider.kind() == foundDataProvider.kind(), "Wrong type after update");
    QVERIFY2(dataProvider.status() == foundDataProvider.status(), "Wrong status after update");
    QVERIFY2(dataProvider.uri() == foundDataProvider.uri(), "Wrong uri after update");

    QVERIFY2(dataProvider.datasets().size() == 2, "Wrong number of datasets after update");
    QCOMPARE("New Queimadas", dataProvider.datasets()[0].name().c_str());
    QCOMPARE("Queimadas3", dataProvider.datasets()[1].name().c_str());
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }

}



void TsDataManager::testUpdateDataProviderInvalidId()
{
  // Tries to update a data provider that doesn't have a valid ID
  try
  {
    DataProvider dataProvider = createDataProvider();
    DataManager::getInstance().update(dataProvider);

    // An exception should be thrown, if not the test fails.
    QFAIL("terrama2::InvalidArgumentError not thrown");
  }
  catch (terrama2::InvalidArgumentError /*ex*/)
  {
    // test ok
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }

}


void TsDataManager::testRemoveDataProviderInvalidId()
{
  try
  {
    DataManager::getInstance().removeDataProvider(0);

    // An exception should be thrown, if not the test fails.
    QFAIL("terrama2::InvalidArgumentError not thrown");
  }
  catch (terrama2::InvalidArgumentError /*ex*/)
  {
    // test ok
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }

}

void TsDataManager::testAddDataSet()
{
  try
  {

    qRegisterMetaType<DataProvider>("DataProvider");
    QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataProviderUpdated(DataProvider)));

    qRegisterMetaType<DataSet>("DataSet");
    QSignalSpy spy2(&DataManager::getInstance(), SIGNAL(dataSetAdded(DataSet)));

    DataSet dataSet = createDataSet();

    DataManager::getInstance().add(dataSet);

    QVERIFY2(spy.count() == 1, "Provider signal emitted");
    QVERIFY2(spy2.count() == 1, "DataSet signal emitted!");

// assure we have a valid dataset identifier
    QVERIFY2(dataSet.id() > 0, "Id must be different than zero after save()!");

    // Test find dataset
    DataSet findDataSet = DataManager::getInstance().findDataSet(dataSet.id());
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }



}

void TsDataManager::testRemoveDataSet()
{
  try
  {
    qRegisterMetaType<DataProvider>("DataProvider");
    QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataProviderUpdated(DataProvider)));

    DataSet dataSet = createDataSet();
    DataManager::getInstance().add(dataSet);


    QSignalSpy spyDataSet(&DataManager::getInstance(), SIGNAL(dataSetRemoved(DataSet)));

    DataManager::getInstance().removeDataSet(dataSet.id());

    QVERIFY2(spyDataSet.count() == 1, "Expect an emitted signal for a removed dataset");

    auto foundDataSet = DataManager::getInstance().findDataSet(dataSet.id());

    QVERIFY2(spy.count() == 1, "Provider signal emitted");

    // An exception should be thrown, if not the test fails.
    QFAIL("terrama2::InvalidArgumentError not thrown");
  }
  catch (terrama2::InvalidArgumentError /*ex*/)
  {
    // test ok
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
}

void TsDataManager::testRemoveDataSetInvalidId()
{

  qRegisterMetaType<DataSet>("DataSet");
  QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataSetRemoved(DataSet)));

  try
  {
    DataManager::getInstance().removeDataSet(0);
    // An exception should be thrown, if not the test fails.
    QFAIL("terrama2::InvalidArgumentError not thrown");
  }
  catch (terrama2::InvalidArgumentError /*ex*/)
  {
    QVERIFY2(spy.count() == 0, "Should not emit a signal");

    // test ok
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
}

void TsDataManager::testFindDataSet()
{
  try
  {
    DataSet dataSet = createDataSet();
    DataManager::getInstance().add(dataSet);

    auto foundDataSet = DataManager::getInstance().findDataSet(dataSet.id());

    QCOMPARE(foundDataSet.kind(), dataSet.kind());
    QCOMPARE(foundDataSet.name(), dataSet.name());
    QCOMPARE(foundDataSet.dataFrequency(), dataSet.dataFrequency());
    QCOMPARE(foundDataSet.status(), dataSet.status());



    QCOMPARE(foundDataSet.collectRules().size(), dataSet.collectRules().size());
    auto dsCollectRules = dataSet.collectRules();
    auto foundCollectRules = foundDataSet.collectRules();
    for(unsigned int i = 0; i < dsCollectRules.size(); ++i)
    {
      QCOMPARE(dsCollectRules[i].script, foundCollectRules[i].script);
    }

    QCOMPARE(foundDataSet.dataSetItems().size(), dataSet.dataSetItems().size());
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }



}


void TsDataManager::testFindDataSetByName()
{
  try
  {
    DataSet dataSet = createDataSet();
    DataManager::getInstance().add(dataSet);

    auto foundDataSet = DataManager::getInstance().findDataSet(dataSet.name());

    QCOMPARE(foundDataSet.kind(), dataSet.kind());
    QCOMPARE(foundDataSet.name(), dataSet.name());
    QCOMPARE(foundDataSet.dataFrequency(), dataSet.dataFrequency());



    QCOMPARE(foundDataSet.collectRules().size(), dataSet.collectRules().size());
    auto dsCollectRules = dataSet.collectRules();
    auto foundCollectRules = foundDataSet.collectRules();
    for(unsigned int i = 0; i < dsCollectRules.size(); ++i)
    {
      QCOMPARE(dsCollectRules[i].script, foundCollectRules[i].script);
    }

    QCOMPARE(foundDataSet.dataSetItems().size(), dataSet.dataSetItems().size());
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }

}


void TsDataManager::testUpdateDataSet()
{
  try
  {
    qRegisterMetaType<DataSet>("DataSet");
    QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataSetUpdated(DataSet)));

    DataSet dataSet = createDataSet();
    DataManager::getInstance().add(dataSet);

    te::dt::TimeDuration schedule(12,0,0);
    dataSet.setSchedule(schedule);

    te::dt::TimeDuration scheduleTimeout(0,30,0);
    dataSet.setScheduleTimeout(scheduleTimeout);

    te::dt::TimeDuration scheduleRetry(0,5,0);
    dataSet.setScheduleRetry(scheduleRetry);

    dataSet.setStatus(DataSet::ACTIVE);

    dataSet.setDescription("Description...");
    dataSet.setName("New queimadas");

    // Change the collect rule script
    std::vector<DataSet::CollectRule>  collectRules = dataSet.collectRules();
    collectRules[0].script = "... LUA SCRIPT UPDATE 1...";
    dataSet.setCollectRules(collectRules);

    // Remove the dataset item PCD_INPE

    auto& dataSetItems = dataSet.dataSetItems();
    dataSet.removeDataSetItem(dataSetItems[0].id());

    // Updates the data from FIRE_POINTS_TYPE
    auto& dsItem = dataSetItems[0];
    dsItem.setMask("Queimadas_*");

    // Add a new dataset item of type PCD_TOA5_TYPE
    DataSetItem dataSetItem(DataSetItem::PCD_TOA5_TYPE, 0, dataSet.id());
    dataSet.add(dataSetItem);

    DataManager::getInstance().update(dataSet);

    QVERIFY2(spy.count() == 1, "Expect an emitted signal");

    auto foundDataSet = DataManager::getInstance().findDataSet(dataSet.id());


    QVERIFY2(foundDataSet.id() == dataSet.id(), "Find should return a valid dataset");

    QVERIFY2(dataSet.name() == foundDataSet.name(), "Name must be the same!");
    QVERIFY2(dataSet.kind() == foundDataSet.kind(), "Kind must be the same!");
    QVERIFY2(dataSet.status() == foundDataSet.status(), "Status must be the same!");
    QVERIFY2(dataSet.scheduleTimeout() == foundDataSet.scheduleTimeout(), "Schedule timeout must be the same!");
    QVERIFY2(dataSet.schedule() == foundDataSet.schedule(), "Schedule must be the same!");
    QVERIFY2(dataSet.scheduleRetry() == foundDataSet.scheduleRetry(), "Schedule retry must be the same!");
    QVERIFY2(dataSet.dataFrequency() == foundDataSet.dataFrequency(), "Data frequency must be the same!");

    QVERIFY2(collectRules[0].script == foundDataSet.collectRules()[0].script, "Collect rule script must be the same!");

    std::map<std::string, std::string> metadata = dataSet.metadata();
    std::map<std::string, std::string> metadataFound = foundDataSet.metadata();

    QVERIFY2(metadata["key"] == metadataFound["key"], "Metadata key/value must be the same!");
    QVERIFY2(metadata["key1"] == metadataFound["key1"], "Metadata key1/value1 must be the same!");
    QVERIFY2(metadata["key2"] == metadataFound["key2"], "Metadata key2/value2 must be the same!");

    // Expected result is to remove the data PCD_INPE, update the FIRE_POINTS  and insert PCD_TOA5.

    QVERIFY2(foundDataSet.dataSetItems().size() == 2, "dataSetItems must have 2 itens!");

    auto dsItem0 = foundDataSet.dataSetItems()[0];
    auto dsItem1 = foundDataSet.dataSetItems()[1];

    QVERIFY2(dsItem0.kind() == DataSetItem::FIRE_POINTS_TYPE, "dataSetItems[0] must be of the type FIRE_POINTS!");
    QVERIFY2(dsItem0.mask() == "Queimadas_*", "Mask should be 'Queimadas_*'!");
    QVERIFY2(dsItem1.kind() == DataSetItem::PCD_TOA5_TYPE, "dataSetItems[1] must be of the type PCD-TOA5!");

    std::map<std::string, std::string> storageMetadata =  dsItem0.storageMetadata();
    QVERIFY2("value" == storageMetadata["key"], "Metadata key/value must be the same!");
    QVERIFY2("value1" == storageMetadata["key1"], "Metadata key1/value1 must be the same!");
    QVERIFY2("value2" == storageMetadata["key2"], "Metadata key2/value2 must be the same!");
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
}

void TsDataManager::testUpdateDataSetInvalidId()
{
  try
  {
    qRegisterMetaType<DataSet>("DataSet");
    QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataSetUpdated(DataSet)));

    // Tries to update a dataset that doesn't have a valid ID
    try
    {
      DataSet dataSet = createDataSet();
      DataManager::getInstance().update(dataSet);

      // An exception should be thrown, if not the test fails.
      QFAIL("terrama2::InvalidArgumentError not thrown");
    }
    catch (terrama2::InvalidArgumentError /*ex*/)
    {
      QVERIFY2(spy.count() == 0, "Should not emit a signal");

      // test ok
    }
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }

}


void TsDataManager::testRemoveDataSetInUse()
{
  //TODO: Try to remove a dataset in use by an analysis, expected an exception: DataSetInUseError
}

void TsDataManager::testAddDataProviderWithId()
{
  try
  {
    qRegisterMetaType<DataProvider>("DataProvider");
    QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataProviderAdded(DataProvider)));

    // Tries to add a data provider with an Id different than 0
    try
    {
      auto dataProvider = DataProvider("Provider", DataProvider::FTP_TYPE, 1);
      DataManager::getInstance().add(dataProvider);

      // An exception should be thrown, if not the test fails.
      QFAIL("terrama2::InvalidArgumentError not thrown");
    }
    catch (terrama2::InvalidArgumentError /*ex*/)
    {
      // test ok
    }
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }

}

void TsDataManager::testAddDataSetWihId()
{
  try
  {
    qRegisterMetaType<DataSet>("DataSet");
    QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataSetAdded(DataSet)));

    // Tries to update a dataset that doesn't have a valid ID
    try
    {
      DataProvider dataProvider = createDataProvider();
      DataManager::getInstance().add(dataProvider);

      // create a new dataset and save it to the database
      DataSet dataSet("Queimadas", DataSet::OCCURENCE_TYPE, 1, dataProvider.id());

      DataManager::getInstance().add(dataSet);

      // An exception should be thrown, if not the test fails.
      QFAIL("terrama2::InvalidArgumentError not thrown");
    }
    catch (terrama2::InvalidArgumentError /*ex*/)
    {
      QVERIFY2(spy.count() == 0, "Should not emit a signal");

      // test ok
    }
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }


}

void TsDataManager::testAddDataProviderShallow()
{
  try
  {
    qRegisterMetaType<DataProvider>("DataProvider");
    QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataProviderAdded(DataProvider)));

    qRegisterMetaType<DataSet>("DataSet");
    QSignalSpy spy2(&DataManager::getInstance(), SIGNAL(dataSetAdded(DataSet)));

    auto dataProvider = createDataProvider();

    DataSet dataSet("Queimadas", DataSet::OCCURENCE_TYPE, 0, dataProvider.id());
    dataProvider.add(dataSet);

    DataManager::getInstance().add(dataProvider, false);

    QVERIFY2(DataManager::getInstance().dataSets().size() == 1,  "The dataset was not added to the data manager!");
    QVERIFY2(dataProvider.datasets().size() != 0, "The dataset was not persisted!");

    for(auto ds: dataProvider.datasets())
    {
      QVERIFY2(ds.id() != 0, "DataSet id wasn't set in the provider after insert!");
    }


    QVERIFY2(spy.count() == 1, "Expect an emitted signal for an added provider");

    QVERIFY2(spy2.count() == 1, "Expect an emitted signal for an added dataset");

    QVERIFY2(dataProvider.id() != 0, "The id wasn't set in the provider after insert!");
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }


}

void TsDataManager::testAddNullDataProvider()
{
  try
  {
    qRegisterMetaType<DataProvider>("DataProvider");
    QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataProviderAdded(DataProvider)));

    // Tries to update a data provider that doesn't have a valid ID
    try
    {
      DataProvider dataProvider;
      DataManager::getInstance().add(dataProvider);

      // An exception should be thrown, if not the test fails.
      QFAIL("terrama2::InvalidArgumentError not thrown");
    }
    catch (terrama2::InvalidArgumentError /*ex*/)
    {
      QVERIFY2(spy.count() == 0, "Should not emit a signal");
      // test ok
    }
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
}


void TsDataManager::testAddNullDataSet()
{
  try
  {
    qRegisterMetaType<DataSet>("DataSet");
    QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataSetAdded(DataSet)));

    // Tries to update a dataset that doesn't have a valid ID
    try
    {
      DataSet dataSet;
      DataManager::getInstance().add(dataSet);

      // An exception should be thrown, if not the test fails.
      QFAIL("terrama2::InvalidArgumentError not thrown");
    }
    catch (terrama2::InvalidArgumentError /*ex*/)
    {
      QVERIFY2(spy.count() == 0, "Should not emit a signal");
      // test ok
    }
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }



}


void TsDataManager::testUpdateNullDataProvider()
{
  try
  {
    qRegisterMetaType<DataProvider>("DataProvider");
    QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataProviderAdded(DataProvider)));

    // Tries to update a data provider that doesn't have a valid ID
    try
    {
      DataProvider dataProvider;
      DataManager::getInstance().update(dataProvider);

      // An exception should be thrown, if not the test fails.
      QFAIL("terrama2::InvalidArgumentError not thrown");
    }
    catch (terrama2::InvalidArgumentError /*ex*/)
    {
      QVERIFY2(spy.count() == 0, "Should not emit a signal");
      // test ok
    }
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }


}

void TsDataManager::testUpdateNullDataSet()
{
  try
  {
    qRegisterMetaType<DataSet>("DataSet");
    QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataSetAdded(DataSet)));

    // Tries to update a dataset that doesn't have a valid ID
    try
    {
      DataSet dataSet;
      DataManager::getInstance().update(dataSet);

      // An exception should be thrown, if not the test fails.
      QFAIL("terrama2::InvalidArgumentError not thrown");
    }
    catch (terrama2::InvalidArgumentError /*ex*/)
    {
      QVERIFY2(spy.count() == 0, "Should not emit a signal");
      // test ok
    }

  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }

}

void TsDataManager::testFindNonExistentDataSet()
{
  try
  {
    DataProvider foundDataProvider = DataManager::getInstance().findDataProvider(999);

    QVERIFY2(foundDataProvider.id()== 0, "Should return an invalid provider");
    // An exception should be thrown, if not the test fails.
    QFAIL("terrama2::InvalidArgumentError not thrown");
  }
  catch (terrama2::InvalidArgumentError /*ex*/)
  {
    // test ok
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }

}

void TsDataManager::testUpdateNonexistentDataProvider()
{

  qRegisterMetaType<DataProvider>("DataProvider");
  QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataProviderAdded(DataProvider)));

  // Tries to update a data provider that doesn't have a valid ID
  try
  {
    DataProvider dataProvider("Server 1", DataProvider::FTP_TYPE, 10);

    DataManager::getInstance().update(dataProvider);

    // An exception should be thrown, if not the test fails.
    QFAIL("terrama2::InvalidArgumentError not thrown");
  }
  catch (terrama2::InvalidArgumentError /*ex*/)
  {
    QVERIFY2(spy.count() == 0, "Should not emit a signal");
    // test ok
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }

}

void TsDataManager::testFindNonExistentDataProvider()
{

  try
  {
    auto dataSet = DataManager::getInstance().findDataSet(999);

    QVERIFY2(dataSet.id() == 0, "Should return an invalid dataset");

    // An exception should be thrown, if not the test fails.
    QFAIL("terrama2::InvalidArgumentError not thrown");
  }
  catch (terrama2::InvalidArgumentError /*ex*/)
  {
    // test ok
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }

}

void TsDataManager::testRemoveNonExistentDataSet()
{
  qRegisterMetaType<DataSet>("DataSet");
  QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataSetRemoved(DataSet)));

  // Tries to remove an nonexistent dataset
  try
  {
    DataManager::getInstance().removeDataSet(1);

    // An exception should be thrown, if not the test fails.
    QFAIL("terrama2::InvalidArgumentError not thrown");
  }
  catch (terrama2::InvalidArgumentError /*ex*/)
  {
    QVERIFY2(spy.count() == 0, "Should not emit a signal");

    // test ok
  }

  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }

}

void TsDataManager::testRemoveNonExistentDataProvider()
{
  qRegisterMetaType<DataProvider>("DataProvider");
  QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataProviderAdded(DataProvider)));

  // Tries to update a data provider that doesn't have a valid ID
  try
  {
    // Removes the data provider
    DataManager::getInstance().removeDataProvider(1);

    // An exception should be thrown, if not the test fails.
    QFAIL("terrama2::InvalidArgumentError not thrown");
  }
  catch (terrama2::InvalidArgumentError /*ex*/)
  {
    QVERIFY2(spy.count() == 0, "Should not emit a signal");
    // test ok

  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }

}

void TsDataManager::testAddDataSetWithNullProvider()
{
  qRegisterMetaType<DataSet>("DataSet");
  QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataSetAdded(DataSet)));

  // Tries to add an dataset with an invalid data provider
  try
  {
    DataSet dataSet("Queimadas", DataSet::OCCURENCE_TYPE);

    DataManager::getInstance().add(dataSet);

    // An exception should be thrown, if not the test fails.
    QFAIL("terrama2::InvalidArgumentError not thrown");
  }
  catch (terrama2::InvalidArgumentError /*ex*/)
  {
    QVERIFY2(spy.count() == 0, "Should not emit a signal");
    // test ok
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }


}

void TsDataManager::testAddDataSetWithNonexistentProvider()
{

  qRegisterMetaType<DataProvider>("DataProvider");
  QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataSetAdded(DataSet)));

  // Tries to add an dataset with an invalid data provider
  try
  {
    DataProvider nonExistentProvider("Server 1", DataProvider::FTP_TYPE, 1);
    DataSet dataSet("Queimadas", DataSet::OCCURENCE_TYPE, 0, nonExistentProvider.id());

    DataManager::getInstance().add(dataSet);

    // An exception should be thrown, if not the test fails.
    QFAIL("terrama2::InvalidArgumentError not thrown");
  }
  catch (terrama2::InvalidArgumentError /*ex*/)
  {
    QVERIFY2(spy.count() == 0, "Should not emit a signal");
    // test ok
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
}

void TsDataManager::testRemoveDataProviderWithDataSet()
{
  try
  {
    qRegisterMetaType<DataProvider>("DataProvider");
    QSignalSpy spyDataProvider(&DataManager::getInstance(), SIGNAL(dataProviderRemoved(DataProvider)));


    auto dataProvider = createDataProvider();
    auto dataSets = dataProvider.datasets();
    DataSet dataSet("Queimadas", DataSet::OCCURENCE_TYPE, 0, dataProvider.id());

    dataProvider.add(dataSet);

    DataManager::getInstance().add(dataProvider);


    // Removes the data provider
    DataManager::getInstance().removeDataProvider(dataProvider.id());

    QVERIFY2(spyDataProvider.count() == 1, "Expect an emitted signal for a removed data provider");

    // Lists all data providers
    auto vecDataProvider = DataManager::getInstance().providers();
    QVERIFY2(vecDataProvider.empty(), "List of providers should be empty after remove!");

    auto vecDataSets = DataManager::getInstance().dataSets();
    QVERIFY2(vecDataSets.empty(), "List of datasets should be empty after remove!");
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }

}

void TsDataManager::testUpdateDataSetWithNullProvider()
{
  qRegisterMetaType<DataSet>("DataSet");
  QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataSetUpdated(DataSet)));

  // Tries to add an dataset with an invalid data provider
  try
  {

    DataSet dataSet("Queimadas", DataSet::OCCURENCE_TYPE);

    DataManager::getInstance().update(dataSet);

    // An exception should be thrown, if not the test fails.
    QFAIL("terrama2::InvalidArgumentError not thrown");
  }
  catch (terrama2::InvalidArgumentError /*ex*/)
  {
    QVERIFY2(spy.count() == 0, "Should not emit a signal");
    // test ok
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
}

void TsDataManager::testUpdateDataSetWithNonexistentProvider()
{

  DataSet dataSet = createDataSet();
  DataManager::getInstance().add(dataSet);

  qRegisterMetaType<DataSet>("DataSet");
  QSignalSpy spy(&DataManager::getInstance(), SIGNAL(dataSetUpdated(DataSet)));

  // Tries to add an dataset with an invalid data provider
  try
  {
    // Nonexistent data provider
    DataProvider dataProvider("Server 1", DataProvider::FTP_TYPE, 10);

    DataSet dataSet("Queimadas", DataSet::OCCURENCE_TYPE, 1, dataProvider.id());

    DataManager::getInstance().update(dataSet);

    // An exception should be thrown, if not the test fails.
    QFAIL("terrama2::InvalidArgumentError not thrown");
  }
  catch (terrama2::InvalidArgumentError /*ex*/)
  {
    QVERIFY2(spy.count() == 0, "Should not emit a signal");
    // test ok
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
}

void TsDataManager::testDataProviderValidName()
{
  try
  {
    auto dataProvider = createDataProvider();
    DataManager::getInstance().add(dataProvider);

    QVERIFY2(DataManager::getInstance().isDataProviderNameValid("Server 1") == false, "Should not be valid");
    QVERIFY2(DataManager::getInstance().isDataProviderNameValid("Server 2") == true, "Should be valid");
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }


}

void TsDataManager::testDatasetValidName()
{
  try
  {
    auto dataset = createDataSet();
    DataManager::getInstance().add(dataset);

    QVERIFY2(DataManager::getInstance().isDatasetNameValid("Queimadas") == false, "Should not be valid");
    QVERIFY2(DataManager::getInstance().isDatasetNameValid("Queimadas 1") == true, "Should be valid");
  }
  catch(boost::exception& e)
  {
    qDebug() << boost::get_error_info< terrama2::ErrorDescription >(e)->toStdString().c_str();
    QFAIL(NO_EXCEPTION_EXPECTED);
  }
  catch(...)
  {
    QFAIL(NO_EXCEPTION_EXPECTED);
  }


}
