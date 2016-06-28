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
  \file terrama2/core/data-model/DataSeriesRisk.hpp

  \brief Models the information of a DataSeries.

  \author Jano Simas
*/

#include "DataSeriesRisk.hpp"

uint32_t terrama2::core::DataSeriesRisk::riskLevel(const std::string& value) const
{
  auto pos = std::find_if(std::begin(riskLevels), std::end(riskLevels), [value](const RiskLevel& risk) { return risk.textValue == value;});
  if(pos != std::end(riskLevels))
    return (*pos).level;
  else
  {
    throw; //TODO: review risk exception
  }
}

uint32_t terrama2::core::DataSeriesRisk::riskLevel(double value) const
{
  auto pos = std::find_if(std::begin(riskLevels), std::end(riskLevels), [value](const RiskLevel& risk)
                                                                                {
                                                                                  bool graterThanLower = risk.hasLowerBound ? risk.lowerBound <= value : true;
                                                                                  bool lesserThanUpper = risk.hasUpperBound ? risk.upperBound > value : true;
                                                                                  return graterThanLower && lesserThanUpper;
                                                                                });
  if(pos != std::end(riskLevels))
    return (*pos).level;
  else
  {
    throw; //TODO: review risk exception
  }
}
