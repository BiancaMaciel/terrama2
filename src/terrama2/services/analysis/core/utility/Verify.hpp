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
  \file terrama2/services/analysis/core/utility/Verify.hpp

  \brief Utility functions for easy consistency check

  \author Jano Simas
*/


#ifndef __TERRAMA2_SERVICES_ANALYSIS_CORE_VERIFY_HPP__
#define __TERRAMA2_SERVICES_ANALYSIS_CORE_VERIFY_HPP__

#include "../Shared.hpp"
#include "../Analysis.hpp"

namespace terrama2
{
  namespace core
  {
    namespace verify
    {
      void analysisType(const terrama2::services::analysis::core::AnalysisPtr analysis, terrama2::services::analysis::core::AnalysisType analysisType);
      void analysisGrid(const terrama2::services::analysis::core::AnalysisPtr analysis){analysisType(analysis, terrama2::services::analysis::core::AnalysisType::GRID_TYPE);}
      void analysisMonitoredObject(const terrama2::services::analysis::core::AnalysisPtr analysis){analysisType(analysis, terrama2::services::analysis::core::AnalysisType::MONITORED_OBJECT_TYPE);}
      void analysisDCP(const terrama2::services::analysis::core::AnalysisPtr analysis){analysisType(analysis, terrama2::services::analysis::core::AnalysisType::DCP_TYPE);}
    } /* verify */
  } /* core */
} /* terrama2 */

#endif //__TERRAMA2_SERVICES_ANALYSIS_CORE_VERIFY_HPP__
