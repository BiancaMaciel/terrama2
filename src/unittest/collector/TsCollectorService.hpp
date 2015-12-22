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
  \file unittest/collector/TsCollectorService.cpp

  \brief Test Collector service...

  \author Paulo R. M. Oliveira
*/

#ifndef __TERRAMA2_UNITTEST_COLLECTOR_COLLECTORSERVICE_HPP__
#define __TERRAMA2_UNITTEST_COLLECTOR_COLLECTORSERVICE_HPP__

//QT
#include <QtTest>

//TODO: add and remove methods
class TsCollectorService: public QObject
{
  Q_OBJECT

protected:

private slots:
    void initTestCase(); // Run before all tests

    void cleanupTestCase(); // Run after all tests


    void init(); //run before each test
    void cleanup(); //run before each test

    //******Test functions********


    void TestStartServerTwice();

    void TestStopServer();


    //******End of Test functions****

};
#endif// __TERRAMA2_UNITTEST_COLLECTOR_COLLECTORSERVICE_HPP__
