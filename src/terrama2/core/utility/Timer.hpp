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
  \file terrama2/core/utility/Timer.hpp

  \brief

  \author Jano Simas
  \author Vinicius Campanha
*/

#ifndef __TERRAMA2_CORE_TIMER_HPP__
#define __TERRAMA2_CORE_TIMER_HPP__

// TerraMA2
#include "ProcessLogger.hpp"
#include "../Typedef.hpp"
#include "../data-model/Schedule.hpp"

//Qt
#include <QTimer>

namespace terrama2
{
  namespace core
  {
    class Timer : public QTimer
    {
      Q_OBJECT

    public:
      Timer(const Schedule& dataSchedule, uint64_t processId, std::shared_ptr< ProcessLogger > log);

      virtual ~Timer();
      Timer(const Timer& other) = delete;
      Timer(Timer&& other) = delete;
      Timer& operator=(const Timer& other) = delete;
      Timer& operator=(Timer&& other) = delete;

      uint64_t processId() const;

    signals:

      void timerSignal(uint64_t processId) const;

    private slots:

      //! Slot called when the timer times out, emits timerSignal.
      void timeoutSlot();
      void scheduleSlot() const;

    private:
      void prepareTimer(const terrama2::core::Schedule& dataSchedule);

      struct Impl;
      Impl* impl_;
    };
  }
}
#endif //__TERRAMA2_CORE_TIMER_HPP__
