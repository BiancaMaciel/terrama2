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
  \file unittest/core/TsUtility.cpp

  \brief Tests for Core Utility class

  \author Vinicius Campanha
*/

//TerraMA2
#include <terrama2/core/utility/Timer.hpp>
#include <terrama2/core/utility/TimeUtils.hpp>

#include "TsUtility.hpp"

void TsUtility::testTimerNoFrequencyException()
{
  try
  {
    terrama2::core::Schedule schedule;
    schedule.id = 0;
    schedule.frequency = 0;
    schedule.frequencyUnit = "second";

    terrama2::core::Timer timer(schedule, 0);

    QFAIL("Should not be here!");
  }
  catch(...)
  {

  }
}

void TsUtility::testTimerInvalidUnitException()
{
  try
  {
    terrama2::core::Schedule schedule;
    schedule.id = 0;
    schedule.frequency = 30;
    schedule.frequencyUnit = "invalid";

    terrama2::core::Timer timer(schedule, 0);

    QFAIL("Should not be here!");
  }
  catch(...)
  {

  }
}

void TsUtility::testTimer()
{
  try
  {
    terrama2::core::Schedule schedule;

    // Schedule a timer in seconds
    schedule.id = 0;
    schedule.frequency = 800;
    schedule.frequencyUnit = "second";

    terrama2::core::Timer timerSecond1(schedule, 0);

    schedule.frequencyUnit = "ss";
    terrama2::core::Timer timerSecond2(schedule, 0);

    schedule.frequencyUnit = "s";
    terrama2::core::Timer timerSecond3(schedule, 0);

    schedule.frequencyUnit = "sec";
    terrama2::core::Timer timerSecond4(schedule, 0);

    // Schedule a timer in minutes
    schedule.frequency = 35;
    schedule.frequencyUnit = "minute";

    terrama2::core::Timer timerMinute1(schedule, 0);

    schedule.frequencyUnit = "min";
    terrama2::core::Timer timerMinute2(schedule, 0);

    schedule.frequencyUnit = "minutes";
    terrama2::core::Timer timerMinute3(schedule, 0);

    // Schedule a timer in hours
    schedule.frequency = 2;
    schedule.frequencyUnit = "hour";

    terrama2::core::Timer timerHour1(schedule, 0);

    schedule.frequencyUnit = "hh";

    terrama2::core::Timer timerHour2(schedule, 0);

    schedule.frequencyUnit = "h";

    terrama2::core::Timer timerHour3(schedule, 0);

    // Schedule a timer in days
    schedule.frequency = 3;
    schedule.frequencyUnit = "day";

    terrama2::core::Timer timerDay1(schedule, 0);

    schedule.frequencyUnit = "d";
    terrama2::core::Timer timerDay2(schedule, 0);

    schedule.frequencyUnit = "dd";
    terrama2::core::Timer timerDay3(schedule, 0);

  }
  catch(...)
  {
    QFAIL("Should not be here!");
  }
}

void TsUtility::testTimeUtilsAddMonth()
{
  std::string dateTime = "2016-Apr-12 12:59:00.373302BRT+00";

  boost::posix_time::ptime boostDate(boost::posix_time::time_from_string(dateTime));

  boost::local_time::time_zone_ptr zone(new boost::local_time::posix_time_zone("+00"));
  boost::local_time::local_date_time date(boostDate.date(), boostDate.time_of_day(), zone, true);

  std::shared_ptr< te::dt::TimeInstantTZ > dt(new te::dt::TimeInstantTZ(date));

  terrama2::core::TimeUtils::addMonth(dt, 3);
  terrama2::core::TimeUtils::addMonth(dt, -1);

  std::string dateTime2 = "2016-Jun-12 12:59:00.373302BRT+00";

  boost::posix_time::ptime boostDate2(boost::posix_time::time_from_string(dateTime2));

  boost::local_time::time_zone_ptr zone2(new boost::local_time::posix_time_zone("+00"));
  boost::local_time::local_date_time date2(boostDate2.date(), boostDate2.time_of_day(), zone2, true);

  std::shared_ptr< te::dt::TimeInstantTZ > dt2(new te::dt::TimeInstantTZ(date2));

  if(*dt.get() != *dt2.get())
    QFAIL("Should not be here!");
}

void TsUtility::testTimeUtilsAddDay()
{
  std::string dateTime = "2016-Apr-12 12:59:00.373302BRT+00";

  boost::posix_time::ptime boostDate(boost::posix_time::time_from_string(dateTime));

  boost::local_time::time_zone_ptr zone(new boost::local_time::posix_time_zone("+00"));
  boost::local_time::local_date_time date(boostDate.date(), boostDate.time_of_day(), zone, true);

  std::shared_ptr< te::dt::TimeInstantTZ > dt(new te::dt::TimeInstantTZ(date));

  terrama2::core::TimeUtils::addDay(dt, 3);
  terrama2::core::TimeUtils::addDay(dt, -1);

  std::string dateTime2 = "2016-Apr-14 12:59:00.373302BRT+00";

  boost::posix_time::ptime boostDate2(boost::posix_time::time_from_string(dateTime2));

  boost::local_time::time_zone_ptr zone2(new boost::local_time::posix_time_zone("+00"));
  boost::local_time::local_date_time date2(boostDate2.date(), boostDate2.time_of_day(), zone2, true);

  std::shared_ptr< te::dt::TimeInstantTZ > dt2(new te::dt::TimeInstantTZ(date2));

  if(*dt.get() != *dt2.get())
    QFAIL("Should not be here!");
}

void TsUtility::testTimeUtilsAddYear()
{
  std::string dateTime = "2016-Apr-12 12:59:00.373302BRT+00";

  boost::posix_time::ptime boostDate(boost::posix_time::time_from_string(dateTime));

  boost::local_time::time_zone_ptr zone(new boost::local_time::posix_time_zone("+00"));
  boost::local_time::local_date_time date(boostDate.date(), boostDate.time_of_day(), zone, true);

  std::shared_ptr< te::dt::TimeInstantTZ > dt(new te::dt::TimeInstantTZ(date));

  terrama2::core::TimeUtils::addYear(dt, 3);
  terrama2::core::TimeUtils::addYear(dt, -1);

  std::string dateTime2 = "2018-Apr-12 12:59:00.373302BRT+00";

  boost::posix_time::ptime boostDate2(boost::posix_time::time_from_string(dateTime2));

  boost::local_time::time_zone_ptr zone2(new boost::local_time::posix_time_zone("+00"));
  boost::local_time::local_date_time date2(boostDate2.date(), boostDate2.time_of_day(), zone2, true);

  std::shared_ptr< te::dt::TimeInstantTZ > dt2(new te::dt::TimeInstantTZ(date2));

  if(*dt.get() != *dt2.get())
    QFAIL("Should not be here!");
}
