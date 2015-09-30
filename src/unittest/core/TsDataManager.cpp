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

#include <terrama2/core/ApplicationController.hpp>
#include <terrama2/core/DataManager.hpp>
#include <terrama2/core/DataProvider.hpp>
#include <terrama2/core/DataSet.hpp>
#include <terrama2/core/DataSetItem.hpp>
#include <terrama2/core/Filter.hpp>
#include <terrama2/core/Utils.hpp>
#include <terrama2/core/Exception.hpp>

//QT
#include <QtTest>
#include <QSignalSpy>

// STL
#include <memory>
#include <vector>



void TsDataManager::init()
{
  terrama2::core::DataManager::getInstance().unload();
  clearDatabase();
}

void TsDataManager::cleanup()
{
  terrama2::core::DataManager::getInstance().unload();
  clearDatabase();
}


void TsDataManager::clearDatabase()
{
  std::shared_ptr<te::da::DataSource> dataSource = terrama2::core::ApplicationController::getInstance().getDataSource();

  std::auto_ptr<te::da::DataSourceTransactor> transactor = dataSource->getTransactor();
  transactor->begin();

  std::string query = "DELETE FROM terrama2.dataset";
  transactor->execute(query);

  query = "DELETE FROM terrama2.data_provider";
  transactor->execute(query);

  transactor->commit();
}

terrama2::core::DataProviderPtr TsDataManager::createDataProvider()
{
  auto dataProvider = terrama2::core::DataProviderPtr(new terrama2::core::DataProvider("Server 1", terrama2::core::DataProvider::FTP_TYPE));
  dataProvider->setStatus(terrama2::core::DataProvider::ACTIVE);
  dataProvider->setDescription("This server...");
  dataProvider->setUri("localhost@...");

  return dataProvider;
}

terrama2::core::DataSetPtr TsDataManager::createDataSet()
{
  terrama2::core::DataProviderPtr dataProvider = createDataProvider();
  terrama2::core::DataManager::getInstance().add(dataProvider);

  // create a new dataset and save it to the database
  terrama2::core::DataSetPtr dataSet(new terrama2::core::DataSet(dataProvider, "Queimadas", terrama2::core::DataSet::OCCURENCE_TYPE));
  te::dt::TimeDuration dataFrequency(2,0,0);
  dataSet->setDataFrequency(dataFrequency);

  std::vector<terrama2::core::DataSet::CollectRule> collectRules;
  {
    terrama2::core::DataSet::CollectRule collectRule;
    collectRule.script = "... LUA SCRIPT 1...";
    collectRules.push_back(collectRule);
  }
  {
    terrama2::core::DataSet::CollectRule collectRule;
    collectRule.script = "... LUA SCRIPT 2...";
    collectRules.push_back(collectRule);
  }
  dataSet->setCollectRules(collectRules);

  std::map<std::string, std::string> metadata;
  metadata["key"] = "value";
  metadata["key1"] = "value1";
  metadata["key2"] = "value2";

  dataSet->setMetadata(metadata);


  // Creates a data list with two DataSetItem
  std::vector<terrama2::core::DataSetItemPtr> dataSetItemList;

  terrama2::core::DataSetItemPtr dataSetItem(new terrama2::core::DataSetItem(dataSet, terrama2::core::DataSetItem::PCD_INPE_TYPE));

  terrama2::core::FilterPtr filter(new terrama2::core::Filter(dataSetItem));
  filter->setByValueType(terrama2::core::Filter::GREATER_THAN_TYPE);
  filter->setByValue(100.);
  dataSetItem->setFilter(filter);
  dataSetItemList.push_back(dataSetItem);

  terrama2::core::DataSetItemPtr data2(new terrama2::core::DataSetItem(dataSet, terrama2::core::DataSetItem::FIRE_POINTS_TYPE));
  dataSetItemList.push_back(data2);
  dataSet->setDataSetItemList(dataSetItemList);

  return dataSet;
}

void TsDataManager::testLoad()
{
  QSignalSpy spy(&terrama2::core::DataManager::getInstance(), SIGNAL(dataManagerLoaded()));

  terrama2::core::DataSetPtr dataSet = createDataSet();
  terrama2::core::DataManager::getInstance().add(dataSet);

  terrama2::core::DataManager::getInstance().unload();

  terrama2::core::DataManager::getInstance().load();

  QVERIFY2(spy.count() == 1, "Expect an emitted signal");

  // Calling load again should have no effect
  terrama2::core::DataManager::getInstance().load();

  QVERIFY2(spy.count() == 1, "Should not emit a new signal, the data manager is already loaded");

  QVERIFY2(terrama2::core::DataManager::getInstance().providers().size() == 1, "List should have one provider!");
  QVERIFY2(terrama2::core::DataManager::getInstance().dataSets().size() == 1, "List should have one dataset!");
}

void TsDataManager::testUnload()
{
  QSignalSpy spy(&terrama2::core::DataManager::getInstance(), SIGNAL(dataManagerUnloaded()));

  terrama2::core::DataSetPtr dataSet = createDataSet();
  terrama2::core::DataManager::getInstance().add(dataSet);

  terrama2::core::DataManager::getInstance().unload();

  QVERIFY2(spy.count() == 1, "Expect an emitted signal");

  QVERIFY2(terrama2::core::DataManager::getInstance().providers().size() == 0, "List of providers should be empty after unload!");
  QVERIFY2(terrama2::core::DataManager::getInstance().dataSets().size() == 0, "List of datasets should be empty after unload!");
}



void TsDataManager::testAddDataProvider()
{
  qRegisterMetaType<terrama2::core::DataProviderPtr>("DataProviderPtr");
  QSignalSpy spy(&terrama2::core::DataManager::getInstance(), SIGNAL(dataProviderAdded(DataProviderPtr)));

  terrama2::core::DataProviderPtr dataProvider = createDataProvider();

  terrama2::core::DataManager::getInstance().add(dataProvider);

  QVERIFY2(spy.count() == 1, "Expect an emitted signal");

  QVERIFY2(dataProvider->id() != 0, "The id wasn't set in the provider after insert!");
}

void TsDataManager::testRemoveDataProvider()
{
  qRegisterMetaType<terrama2::core::DataProviderPtr>("DataProviderPtr");
  QSignalSpy spy(&terrama2::core::DataManager::getInstance(), SIGNAL(dataProviderRemoved(DataProviderPtr)));

  terrama2::core::DataProviderPtr dataProvider = createDataProvider();

  terrama2::core::DataManager::getInstance().add(dataProvider);

  QVERIFY2(dataProvider->id() != 0, "The id wasn't set in the provider after insert!");

  // Removes the data provider
  terrama2::core::DataManager::getInstance().removeDataProvider(dataProvider->id());

  QVERIFY2(spy.count() == 1, "Expect an emitted signal");

  // Lists all data providers
  auto vecDataProvider = terrama2::core::DataManager::getInstance().providers();

  QVERIFY2(vecDataProvider.empty(), "List should be empty after remove!");
}

void TsDataManager::testFindDataProvider()
{
  terrama2::core::DataProviderPtr dataProvider = createDataProvider();

  terrama2::core::DataManager::getInstance().add(dataProvider);

  // Find the same data provider by id
  terrama2::core::DataProviderPtr foundDataProvider = terrama2::core::DataManager::getInstance().findDataProvider(
          foundDataProvider->id());

  QVERIFY2(foundDataProvider.get(), "Could not recover the data provider by id!");

  QVERIFY2("This server..." == foundDataProvider->description(), "Wrong Description in recovered provider");
  QVERIFY2("Server 1" == foundDataProvider->name(), "Wrong name in recovered provider");
  QVERIFY2(terrama2::core::DataProvider::FTP_TYPE == foundDataProvider->kind(), "Wrong type in recovered provider");
  QVERIFY2(terrama2::core::DataProvider::ACTIVE == foundDataProvider->status(), "Wrong status in recovered provider");
  QVERIFY2("localhost@..." == foundDataProvider->uri(), "Wrong uri in recovered provider");

}

void TsDataManager::testUpdateDataProvider()
{
  qRegisterMetaType<terrama2::core::DataProviderPtr>("DataProviderPtr");
  QSignalSpy spy(&terrama2::core::DataManager::getInstance(), SIGNAL(dataProviderUpdated(DataProviderPtr)));

  terrama2::core::DataProviderPtr dataProvider = createDataProvider();

  terrama2::core::DataManager::getInstance().add(dataProvider);

  dataProvider->setName("New server");
  dataProvider->setStatus(terrama2::core::DataProvider::INACTIVE);
  dataProvider->setDescription("New server is ...");
  dataProvider->setUri("myserver@...");

  terrama2::core::DataManager::getInstance().update(dataProvider);

  QVERIFY2(spy.count() == 1, "Expect an emitted signal");


  // Find the same data provider by id
  terrama2::core::DataProviderPtr foundDataProvider = terrama2::core::DataManager::getInstance().findDataProvider(
          foundDataProvider->id());

  QVERIFY2(foundDataProvider.get(), "Could not recover the data provider by id!");

  QVERIFY2(dataProvider->description() == foundDataProvider->description(), "Wrong Description after update");
  QVERIFY2(dataProvider->name() == foundDataProvider->name(), "Wrong name after update");
  QVERIFY2(dataProvider->kind() == foundDataProvider->kind(), "Wrong type after update");
  QVERIFY2(dataProvider->status() == foundDataProvider->status(), "Wrong status after update");
  QVERIFY2(dataProvider->uri() == foundDataProvider->uri(), "Wrong uri after update");

}


void TsDataManager::testUpdateDataProviderInvalidId()
{
  // Tries to update a data provider that doesn't have a valid ID
  try
  {
    terrama2::core::DataProviderPtr dataProvider = createDataProvider();
    terrama2::core::DataManager::getInstance().update(dataProvider);

    // An exception should be thrown, if not the test fails.
    QFAIL("InvalidDataProviderIdError not thrown");
  }
  catch (terrama2::core::InvalidDataProviderIdError /*ex*/)
  {
    // test ok
  }
}


void TsDataManager::testRemoveDataProviderInvalidId()
{
  try
  {
    terrama2::core::DataManager::getInstance().removeDataProvider(0);

    // An exception should be thrown, if not the test fails.
    QFAIL("InvalidDataProviderIdError not thrown");
  }
  catch (terrama2::core::InvalidDataProviderIdError /*ex*/)
  {
    // test ok
  }
}

void TsDataManager::testAddDataSet()
{
  qRegisterMetaType<terrama2::core::DataSetPtr>("DataSetPtr");
  QSignalSpy spy(&terrama2::core::DataManager::getInstance(), SIGNAL(dataSetAdded(DataSetPtr)));

  terrama2::core::DataSetPtr dataSet = createDataSet();

  terrama2::core::DataManager::getInstance().add(dataSet);

  QVERIFY2(spy.count() == 1, "Expect an emitted signal");

// assure we have a valid dataset identifier
  QVERIFY2(dataSet->id() > 0, "Id must be different than zero after save()!");

  // Test find dataset
  terrama2::core::DataSetPtr findDataSet = terrama2::core::DataManager::getInstance().findDataSet(dataSet->id());

}

void TsDataManager::testRemoveDataSet()
{
  qRegisterMetaType<terrama2::core::DataSetPtr>("DataSetPtr");
  QSignalSpy spy(&terrama2::core::DataManager::getInstance(), SIGNAL(dataSetRemoved(DataSetPtr)));

  terrama2::core::DataSetPtr dataSet = createDataSet();
  terrama2::core::DataManager::getInstance().add(dataSet);

  terrama2::core::DataManager::getInstance().removeDataSet(dataSet->id());

  QVERIFY2(spy.count() == 1, "Expect an emitted signal");

  auto findDataSet = terrama2::core::DataManager::getInstance().findDataSet(dataSet->id());

  QVERIFY2(findDataSet.get() == nullptr, "Find should return null after remove");

}

void TsDataManager::testRemoveDataSetInvalidId()
{
  qRegisterMetaType<terrama2::core::DataSetPtr>("DataSetPtr");
  QSignalSpy spy(&terrama2::core::DataManager::getInstance(), SIGNAL(dataSetRemoved(DataSetPtr)));

  try
  {
    terrama2::core::DataManager::getInstance().removeDataSet(0);
    // An exception should be thrown, if not the test fails.
    QFAIL("InvalidDataSetIdError not thrown");
  }
  catch (terrama2::core::InvalidDataSetIdError /*ex*/)
  {
    QVERIFY2(spy.count() == 0, "Should not emit a signal");

    // test ok
  }
}

void TsDataManager::testFindDataSet()
{
  terrama2::core::DataSetPtr dataSet = createDataSet();
  terrama2::core::DataManager::getInstance().add(dataSet);

  auto foundDataSet = terrama2::core::DataManager::getInstance().findDataSet(dataSet->id());

  QCOMPARE(foundDataSet->kind(), dataSet->kind());
  QCOMPARE(foundDataSet->name(), dataSet->name());
  QCOMPARE(foundDataSet->dataFrequency(), dataSet->dataFrequency());



  QCOMPARE(foundDataSet->collectRules().size(), dataSet->collectRules().size());
  auto dsCollectRules = dataSet->collectRules();
  auto foundCollectRules = foundDataSet->collectRules();
  for(unsigned int i = 0; i < dsCollectRules.size(); ++i)
  {
    QCOMPARE(dsCollectRules[i].script, foundCollectRules[i].script);
  }

  QCOMPARE(foundDataSet->dataSetItemList().size(), dataSet->dataSetItemList().size());

}

void TsDataManager::testUpdateDataSet()
{
  qRegisterMetaType<terrama2::core::DataSetPtr>("DataSetPtr");
  QSignalSpy spy(&terrama2::core::DataManager::getInstance(), SIGNAL(dataSetUpdated(DataSetPtr)));

  terrama2::core::DataSetPtr dataSet = createDataSet();
  terrama2::core::DataManager::getInstance().add(dataSet);

  te::dt::TimeDuration schedule(12,0,0);
  dataSet->setSchedule(schedule);

  te::dt::TimeDuration scheduleTimeout(0,30,0);
  dataSet->setScheduleTimeout(scheduleTimeout);

  te::dt::TimeDuration scheduleRetry(0,5,0);
  dataSet->setScheduleRetry(scheduleRetry);

  dataSet->setStatus(terrama2::core::DataSet::ACTIVE);

  dataSet->setDescription("Description...");
  dataSet->setName("New queimadas");

  // Change the collect rule script
  std::vector<terrama2::core::DataSet::CollectRule>  collectRules = dataSet->collectRules();
  collectRules[0].script = "... LUA SCRIPT UPDATE 1...";
  dataSet->setCollectRules(collectRules);

  // Remove the dataset item PCD_INPE

  auto dataSetItemList = dataSet->dataSetItemList();
  dataSetItemList.erase(dataSetItemList.begin());

  // Updates the data from FIRE_POINTS_TYPE
  dataSetItemList[0]->setMask("Queimadas_*");

  // Add a new dataset item of type PCD_TOA5_TYPE
  terrama2::core::DataSetItemPtr dataSetItem(new terrama2::core::DataSetItem(dataSet, terrama2::core::DataSetItem::PCD_TOA5_TYPE));
  dataSetItemList.push_back(dataSetItem);
  dataSet->setDataSetItemList(dataSetItemList);

  terrama2::core::DataManager::getInstance().update(dataSet);

  QVERIFY2(spy.count() == 1, "Expect an emitted signal");

  auto foundDataSet = terrama2::core::DataManager::getInstance().findDataSet(dataSet->id());


  QVERIFY2(foundDataSet.get() != nullptr, "Find should return a valid dataset");

  QVERIFY2(dataSet->name() == foundDataSet->name(), "Name must be the same!");
  QVERIFY2(dataSet->kind() == foundDataSet->kind(), "Kind must be the same!");
  QVERIFY2(dataSet->status() == foundDataSet->status(), "Status must be the same!");
  QVERIFY2(dataSet->scheduleTimeout() == foundDataSet->scheduleTimeout(), "Schedule timeout must be the same!");
  QVERIFY2(dataSet->schedule() == foundDataSet->schedule(), "Schedule must be the same!");
  QVERIFY2(dataSet->scheduleRetry() == foundDataSet->scheduleRetry(), "Schedule retry must be the same!");
  QVERIFY2(dataSet->dataFrequency() == foundDataSet->dataFrequency(), "Data frequency must be the same!");

  QVERIFY2(collectRules[0].script == foundDataSet->collectRules()[0].script, "Collect rule script must be the same!");

  std::map<std::string, std::string> metadata = dataSet->metadata();
  QVERIFY2(metadata["key"] == foundDataSet->metadata()["key"], "Metadata key/value must be the same!");
  QVERIFY2(metadata["key1"] == foundDataSet->metadata()["key1"], "Metadata key1/value1 must be the same!");
  QVERIFY2(metadata["key2"] == foundDataSet->metadata()["key2"], "Metadata key2/value2 must be the same!");

  // Expected result is to remove the data PCD_INPE, update the FIRE_POINTS  and insert PCD_TOA5.
  QVERIFY2(foundDataSet->dataSetItemList().size() == 2, "dataSetItemList must have 2 itens!");
  QVERIFY2(foundDataSet->dataSetItemList()[0]->kind() == terrama2::core::DataSetItem::FIRE_POINTS_TYPE, "dataSetItemList[0] must be of the type FIRE_POINTS!");
  QVERIFY2(foundDataSet->dataSetItemList()[0]->mask() == "Queimadas_*", "Mask should be 'Queimadas_*'!");
  QVERIFY2(foundDataSet->dataSetItemList()[1]->kind() == terrama2::core::DataSetItem::PCD_TOA5_TYPE, "dataSetItemList[1] must be of the type PCD-TOA5!");


}

void TsDataManager::testUpdateDataSetInvalidId()
{
  qRegisterMetaType<terrama2::core::DataSetPtr>("DataSetPtr");
  QSignalSpy spy(&terrama2::core::DataManager::getInstance(), SIGNAL(dataSetUpdated(DataSetPtr)));

  // Tries to update a dataset that doesn't have a valid ID
  try
  {
    terrama2::core::DataSetPtr dataSet = createDataSet();
    terrama2::core::DataManager::getInstance().update(dataSet);

    // An exception should be thrown, if not the test fails.
    QFAIL("InvalidDataSetIdError not thrown");
  }
  catch (terrama2::core::InvalidDataSetIdError /*ex*/)
  {
    QVERIFY2(spy.count() == 0, "Should not emit a signal");

    // test ok
  }
}


void TsDataManager::testRemoveDataSetInUse()
{
  //TODO: Try to remove a dataset in use by an analysis, expected an exception: DataSetInUseError
}

void TsDataManager::testAddDataProviderWithId()
{
  qRegisterMetaType<terrama2::core::DataProviderPtr>("DataProviderPtr");
  QSignalSpy spy(&terrama2::core::DataManager::getInstance(), SIGNAL(dataProviderAdded(DataProviderPtr)));

  // Tries to add a data provider with an Id different than 0
  try
  {
    auto dataProvider = terrama2::core::DataProviderPtr(new terrama2::core::DataProvider("Server 1", terrama2::core::DataProvider::FTP_TYPE, 1));
    terrama2::core::DataManager::getInstance().add(dataProvider);

    // An exception should be thrown, if not the test fails.
    QFAIL("InvalidDataProviderIdError not thrown");
  }
  catch (terrama2::core::InvalidDataProviderIdError /*ex*/)
  {
    // test ok
  }
}

void TsDataManager::testAddDataSetWihId()
{
  qRegisterMetaType<terrama2::core::DataSetPtr>("DataSetPtr");
  QSignalSpy spy(&terrama2::core::DataManager::getInstance(), SIGNAL(dataSetAdded(DataSetPtr)));

  // Tries to update a dataset that doesn't have a valid ID
  try
  {
    terrama2::core::DataProviderPtr dataProvider = createDataProvider();
    terrama2::core::DataManager::getInstance().add(dataProvider);

    // create a new dataset and save it to the database
    terrama2::core::DataSetPtr dataSet(new terrama2::core::DataSet(dataProvider, "Queimadas", terrama2::core::DataSet::OCCURENCE_TYPE, 1));

    terrama2::core::DataManager::getInstance().add(dataSet);

    // An exception should be thrown, if not the test fails.
    QFAIL("InvalidDataSetIdError not thrown");
  }
  catch (terrama2::core::InvalidDataSetIdError /*ex*/)
  {
    QVERIFY2(spy.count() == 0, "Should not emit a signal");

    // test ok
  }
}

void TsDataManager::testAddDataProviderWithDataSet()
{
  qRegisterMetaType<terrama2::core::DataProviderPtr>("DataProviderPtr");
  QSignalSpy spy(&terrama2::core::DataManager::getInstance(), SIGNAL(dataProviderAdded(DataProviderPtr)));

  auto dataProvider = createDataProvider();
  auto dataSets = dataProvider->dataSets();
  terrama2::core::DataSetPtr dataSet(new terrama2::core::DataSet(dataProvider, "Queimadas", terrama2::core::DataSet::OCCURENCE_TYPE));
  dataSets.push_back(dataSet);
  dataProvider->setDataSets(dataSets);

  terrama2::core::DataManager::getInstance().add(dataProvider);

  QVERIFY2(dataProvider->dataSets().size() != 0, "The dataset was not persisted!");

  foreach(auto ds, dataProvider->dataSets())
  {
    QVERIFY2(ds->id() != 0, "DataSet id wasn't set in the provider after insert!");
  }


  QVERIFY2(spy.count() == 1, "Expect an emitted signal");

  QVERIFY2(dataProvider->id() != 0, "The id wasn't set in the provider after insert!");
}

void TsDataManager::testAddNullDataProvider()
{
  qRegisterMetaType<terrama2::core::DataProviderPtr>("DataProviderPtr");
  QSignalSpy spy(&terrama2::core::DataManager::getInstance(), SIGNAL(dataProviderAdded(DataProviderPtr)));

  // Tries to update a data provider that doesn't have a valid ID
  try
  {
    terrama2::core::DataProviderPtr dataProvider;
    terrama2::core::DataManager::getInstance().add(dataProvider);

    // An exception should be thrown, if not the test fails.
    QFAIL("InvalidDataProviderError not thrown");
  }
  catch (terrama2::core::InvalidDataProviderError /*ex*/)
  {
    QVERIFY2(spy.count() == 0, "Should not emit a signal");
    // test ok
  }
}


void TsDataManager::testAddNullDataSet()
{
  qRegisterMetaType<terrama2::core::DataSetPtr>("DataSetPtr");
  QSignalSpy spy(&terrama2::core::DataManager::getInstance(), SIGNAL(dataSetAdded(DataSetPtr)));

  // Tries to update a dataset that doesn't have a valid ID
  try
  {
    terrama2::core::DataSetPtr dataSet;
    terrama2::core::DataManager::getInstance().add(dataSet);

    // An exception should be thrown, if not the test fails.
    QFAIL("InvalidDataSetError not thrown");
  }
  catch (terrama2::core::InvalidDataSetError /*ex*/)
  {
    QVERIFY2(spy.count() == 0, "Should not emit a signal");
    // test ok
  }

}


void TsDataManager::testUpdateNullDataProvider()
{
  qRegisterMetaType<terrama2::core::DataProviderPtr>("DataProviderPtr");
  QSignalSpy spy(&terrama2::core::DataManager::getInstance(), SIGNAL(dataProviderAdded(DataProviderPtr)));

  // Tries to update a data provider that doesn't have a valid ID
  try
  {
    terrama2::core::DataProviderPtr dataProvider;
    terrama2::core::DataManager::getInstance().update(dataProvider);

    // An exception should be thrown, if not the test fails.
    QFAIL("InvalidDataProviderError not thrown");
  }
  catch (terrama2::core::InvalidDataProviderError /*ex*/)
  {
    QVERIFY2(spy.count() == 0, "Should not emit a signal");
    // test ok
  }
}

void TsDataManager::testUpdateNullDataSet()
{
  qRegisterMetaType<terrama2::core::DataSetPtr>("DataSetPtr");
  QSignalSpy spy(&terrama2::core::DataManager::getInstance(), SIGNAL(dataSetAdded(DataSetPtr)));

  // Tries to update a dataset that doesn't have a valid ID
  try
  {
    terrama2::core::DataSetPtr dataSet;
    terrama2::core::DataManager::getInstance().update(dataSet);

    // An exception should be thrown, if not the test fails.
    QFAIL("InvalidDataSetError not thrown");
  }
  catch (terrama2::core::InvalidDataSetError /*ex*/)
  {
    QVERIFY2(spy.count() == 0, "Should not emit a signal");
    // test ok
  }

}
