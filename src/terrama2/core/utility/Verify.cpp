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
  \file terrama2/core/utility/Verify.cpp

  \brief Utility functions for easy consistency check

  \author Jano Simas
*/

#include "Verify.hpp"

#include "../Exception.hpp"

#include <QObject>
#include <QString>


//TODO: improve message to help identify where the error accurred

void terrama2::core::verify::srid(int srid_)
{
  if(srid_ <= 0 || srid_ > 998999)
    throw VerifyException() << terrama2::ErrorDescription(QObject::tr("Invalid SRID."));
}

void terrama2::core::verify::date(const std::shared_ptr<te::dt::TimeInstantTZ>& date)
{
  verify::date(date->getTimeInstantTZ());
}

void terrama2::core::verify::date(const boost::local_time::local_date_time& date)
{
  if(date.is_special())
    throw VerifyException() << terrama2::ErrorDescription(QObject::tr("Invalid Date/Time."));

  if(!date.zone())
    throw VerifyException() << terrama2::ErrorDescription(QObject::tr("Invalid Timezone."));
}
