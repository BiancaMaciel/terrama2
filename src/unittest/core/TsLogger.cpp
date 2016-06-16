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
  \file unittest/core/TsLogger.cpp

  \brief Tests for Core Logger class

  \author Vinicius Campanha
*/

//TerraMA2
#include <terrama2/core/Typedef.hpp>
#include <terrama2/core/utility/TimeUtils.hpp>
#include <terrama2/Exception.hpp>

#include "TsLogger.hpp"
#include "TestLogger.hpp"



void TsLogger::testProcessLogger()
{
  try
  {
  TestLogger log;

  RegisterId registerID = log.start(1);

  log.logValue("tag1", "value1", registerID);
  log.logValue("tag2", "value2", registerID);
  log.logValue("tag1", "value3", registerID);
  log.logValue("tag2", "value4", registerID);
  log.error("Unit Test Error", registerID);
  log.error("Unit Test second Error", registerID);
  log.info("Unit Test Info", registerID);
  log.info("Unit Test seconde Info", registerID);

  std::shared_ptr< te::dt::TimeInstantTZ > dataTime = terrama2::core::TimeUtils::nowUTC();

  log.done(dataTime, registerID);
  }
  catch(terrama2::Exception& e)
  {
    QFAIL(e.what());
  }
}
