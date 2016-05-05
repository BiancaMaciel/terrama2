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
  \file terrama2/core/utility/ServiceManager.hpp
  \brief
  \author Jano Simas
*/


#ifndef __TERRAMA2_CORE_SERVICE_MANAGER_HPP__
#define __TERRAMA2_CORE_SERVICE_MANAGER_HPP__

//Qt
#include <QJsonObject>
#include <QObject>

// TerraLib
#include <terralib/common/Singleton.h>
#include <terralib/datatype/TimeInstantTZ.h>

namespace terrama2
{
  namespace core
  {
    class ServiceManager : public QObject, public te::common::Singleton<ServiceManager>
    {
        Q_OBJECT

      public:
        virtual bool serviceLoaded() const;

        void setInstanceName(const std::string& instanceName);
        virtual const std::string& instanceName() const;

        void setInstanceId(int instanceId);
        virtual int instanceId() const;

        void setServiceType(const std::string& serviceType);
        virtual const std::string& serviceType() const;

        void setListeningPort(int listeningPort);
        virtual int listeningPort() const;

        virtual const std::string& terrama2Version() const;
        virtual const std::shared_ptr< te::dt::TimeInstantTZ >& startTime() const;

        void updateService(const QJsonObject& obj);
        virtual QJsonObject status() const;

      signals:
        void listeningPortUpdated(int);

      protected:
        friend class te::common::Singleton<ServiceManager>;

        ServiceManager();

        virtual ~ServiceManager() = default;
        ServiceManager(const ServiceManager& other) = delete;
        ServiceManager(ServiceManager&& other) = delete;
        ServiceManager& operator=(const ServiceManager& other) = delete;
        ServiceManager& operator=(ServiceManager&& other) = delete;

        std::string instanceName_;
        int instanceId_ = 0;
        std::string serviceType_;
        int listeningPort_;
        const std::string terrama2Version_ = "TerraMA2-4-alpha2";//FIXME: use the global version
        std::shared_ptr< te::dt::TimeInstantTZ > startTime_;
        bool serviceLoaded_ = false;
    };
  }
}

#endif //__TERRAMA2_CORE_SERVICE_MANAGER_HPP__
