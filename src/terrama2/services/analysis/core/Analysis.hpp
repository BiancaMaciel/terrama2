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
  \file terrama2/services/analysis/core/Analysis.hpp

  \brief Model for the analysis configuration.

  \author Paulo R. M. Oliveira
*/


#ifndef __TERRAMA2_ANALYSIS_CORE_ANALYSIS_HPP__
#define __TERRAMA2_ANALYSIS_CORE_ANALYSIS_HPP__

#include "../../../core/data-model/DataSeries.hpp"
#include "../../../core/data-model/Schedule.hpp"
#include "../Typedef.hpp"

// STL
#include <string>
#include <vector>

namespace terrama2
{
  namespace services
  {
    namespace analysis
    {
      namespace core
      {
        /*!
          \brief Defines the type of the analysis.
        */
        enum AnalysisType
        {
          PCD_TYPE = 1, //!< Analysis for DCP.
          MONITORED_OBJECT_TYPE = 2, //!< Analysis for monitored objects.
          GRID_TYPE = 3, //!< Analysis for grids.
        };

        /*!
          \brief Defines the language of the script.
        */
        enum ScriptLanguage
        {
          PYTHON = 1, //!< Scripts in Python.
          LUA = 2 //!< Scripts in LUA.
        };

        /*!
          \brief Defines the type of use of a DataSeries in the analysis.
          Analysis of type PCD_TYPE requires an DATASERIES_PCD_TYPE in the analysis DataSeries list.
          Analysis of type MONITORED_OBJECT_TYPE requires an DATASERIES_MONITORED_OBJECT_TYPE in the analysis DataSeries list.
          Analysis of type GRID_TYPE requires an DATASERIES_GRID_TYPE in the analysis DataSeries list.

          Any additional DataSeries that will be used in the analysis must exist in the DataSeries list with the type ADDITIONAL_DATA_TYPE.

        */
        enum AnalysisDataSeriesType
        {
          DATASERIES_MONITORED_OBJECT_TYPE = 1, //!< Identifies a DataSeries used as monitored object.
          DATASERIES_GRID_TYPE = 2, //!< Identifies a DataSeries used as grid.
          DATASERIES_PCD_TYPE = 3, //!< Identifies a DataSeries used as DCP.
          ADDITIONAL_DATA_TYPE = 4 //!< Identifies a DataSeries used as an additional data.
        };

        /*!
          \struct AnalysisDataSeries
          \brief Contains the configuration of an DataSeries used in an analysis.
        */
        struct AnalysisDataSeries
        {
          AnalysisDataSeriesId id = 0; //!< AnalysisDataSeries identifier.
          terrama2::core::DataSeriesPtr dataSeries; //!< Smart pointer to the DataSeries.
          DataSeriesId dataSeriesId; //!< Identifier of the DataSeries.
          AnalysisDataSeriesType type; //!< Type of use of the DataSeries in the analysis.
          std::map<uint64_t, std::string> alias; //!< Map containing the alias for the columns of a DataSeries.
          std::map<std::string, std::string> metadata; //!< Metadata of the AnalysisDataSeries.
        };

        /*!
          \struct Analysis
          \brief Model for the configuration of an analysis execution.
        */
        struct Analysis
        {
          AnalysisId id = 0; //!< Analysis identifier.
          ProjectId projectId = 0; //!< Project identifier.
					ScriptLanguage scriptLanguage; //!< Language of the script.
					std::string script; //!< Content of the script.
          AnalysisType type; //!< Type of the analysis.
					std::string name; //!< Name of the analysis.
					std::string description; //!< Short description of the purpose of the analysis.
          bool active = true; //!< Defines if the analysis is active, if true it will be executed according to the schedule.
          DataSetId outputDataset; //!< The dataset that stores the result of the analysis.
          std::map<std::string, std::string> metadata; //!< Metadata of the analysis.
          std::vector<AnalysisDataSeries> analysisDataSeriesList; //!< DataSeries that are used in this anlysis.
          terrama2::core::Schedule schedule; //!< Time schedule for the analysis execution.
        };

      } // end namespace core
    }   // end namespace analysis
  }     // end namespace services
}       // end namespace terrama2

#endif //__TERRAMA2_ANALYSIS_CORE_ANALYSIS_HPP__
