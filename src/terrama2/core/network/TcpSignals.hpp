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
  \file terrama2/core/TcpSignals.hpp

  \brief Signals for TCP messages.

  \author Jano Simas
*/

namespace terrama2 {
  namespace core {
    //! Namespace for signals for TCP communication.
    namespace TcpSignals{
      /*!
        \enum TcpSignal
        \brief  Signals for TCP communication.
        \see https://trac.dpi.inpe.br/terrama2/wiki/programmersguide/tcp
      */
      enum TcpSignal
      {
        TERMINATE_SERVICE_SIGNAL = 0,//!< Stop service signal.
        STATUS_SIGNAL = 1,//!< Check if the connection is alive and get service information
        ADD_DATA_SIGNAL = 2,//!< Data signal, followed by data.
        START_PROCESS_SIGNAL = 3,//!< Start signal, queue the collection, analysis, ...
        LOG_SIGNAL = 4,//!< Log signal, send error message to node.js server
        REMOVE_DATA_SIGNAL = 5, //!< Remove data signal
        PROCESS_FINISHED_SIGNAL = 6 //!< Signal emited when a process has finished
      };
    }
  }
}
