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
  \file terrama2/core/data-model/SemanticsManager.hpp

  \brief

  \author Jano Simas
*/

#ifndef __TERRAMA2_CORE_DATA_MODEL_SEMANTICS_MANAGER_HPP__
#define __TERRAMA2_CORE_DATA_MODEL_SEMANTICS_MANAGER_HPP__

#include "../data-model/DataSeriesSemantics.hpp"

//STL
#include <string>

namespace terrama2
{
  namespace core
  {
    class SemanticsManager {
    public:
      SemanticsManager (){}
      ~SemanticsManager (){}

      /*!

        \exception TODO: raised when a semantics with same name is already present
      */
      DataSeriesSemantics  addSemantics(const std::string& name, const DataSeriesSemantics::MacroType& macroType, const DataFormat& format);

    private:

    };
  } /* core */
} /* terrama2 */

#endif //__TERRAMA2_CORE_DATA_MODEL_SEMANTICS_MANAGER_HPP__
