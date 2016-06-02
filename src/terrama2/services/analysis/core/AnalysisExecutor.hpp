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
  \file terrama2/services/analysis/core/AnalysisExecutor.hpp

  \brief Prepare context for an analysis execution.

  \author Paulo R. M. Oliveira
*/


#ifndef __TERRAMA2_ANALYSIS_CORE_ANALYSIS_EXECUTOR_HPP__
#define __TERRAMA2_ANALYSIS_CORE_ANALYSIS_EXECUTOR_HPP__

// TerraMA2
#include "../Shared.hpp"

// STL
#include <thread>
#include <vector>

namespace terrama2
{
  namespace services
  {
    namespace analysis
    {
      namespace core
      {
        // Forward declaration
        struct Analysis;

        /*!
          \brief Join a given thread.
          \param t The thread to join.
        */
        void joinThread(std::thread& t);

        /*!
          \brief Join all created threads for an anlysis execution.
          \param threads The threads to join.
        */
        void joinAllThreads(std::vector<std::thread>& threads);

        /*!
          \brief Starts the process of an analysis execution.
          \param dataManager A smart pointer to the data manager.
          \param analysis The analysis to be executed.
        */
        void runAnalysis(DataManagerPtr dataManager, const Analysis& analysis, unsigned int threadNumber);

        /*!
          \brief Prepare the context for a monitored object analysis.
          \param dataManager A smart pointer to the data manager.
          \param analysis The analysis to be executed
        */
        void runMonitoredObjectAnalysis(DataManagerPtr dataManager, const Analysis& analysis, unsigned int threadNumber);

        /*!
          \brief Prepare the context for a DCP analysis.
          \param dataManager A smart pointer to the data manager.
          \param analysis The analysis to be executed
        */
        void runDCPAnalysis(DataManagerPtr dataManager, const Analysis& analysis, unsigned int threadNumber);

        /*!
          \brief Reads the analysis result from context and stores it to the configured output dataset.
          \param dataManager A smart pointer to the data manager.
          \param analysis The analysis to be executed
        */
        void storeAnalysisResult(DataManagerPtr dataManager, const Analysis& analysis);

      } // end namespace core
    }   // end namespace analysis
  }     // end namespace services
}       // end namespace terrama2

#endif //__TERRAMA2_ANALYSIS_CORE_ANALYSIS_EXECUTOR_HPP__
