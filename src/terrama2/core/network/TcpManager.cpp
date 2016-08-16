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
  \file terrama2/core/TcpManager.cpp
  \brief A class to receive data through a socket.
  \author Jano Simas
*/

#include "TcpManager.hpp"
#include "TcpSignals.hpp"
#include "../utility/Logger.hpp"
#include "../utility/ServiceManager.hpp"
#include "../data-model/DataManager.hpp"

// Qt
#include <QObject>
#include <QDataStream>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

class RaiiBlock
{
  public:
    RaiiBlock(uint32_t& block) : block_(block) {}
    ~RaiiBlock() {block_ = 0;}

    uint32_t& block_;
};

bool terrama2::core::TcpManager::updateListeningPort(uint32_t port) noexcept
{
  try
  {
    if(serverPort() == port)
      return true;

    if(isListening())
      close();

    return listen(serverAddress(), static_cast<quint16>(port));
  }
  catch(...)
  {
    // exception guard, slots should never emit exceptions.
    return false;
  }
}

terrama2::core::TcpManager::TcpManager(std::weak_ptr<terrama2::core::DataManager> dataManager,
                                       std::weak_ptr<terrama2::core::ProcessLogger> logger,
                                       QObject* parent)
  : QTcpServer(parent),
    blockSize_(0),
    dataManager_(dataManager),
    logger_(logger)
{
  QObject::connect(this, &terrama2::core::TcpManager::newConnection, this, &terrama2::core::TcpManager::receiveConnection);
  serviceManager_ = &terrama2::core::ServiceManager::getInstance();
}

void terrama2::core::TcpManager::updateService(const QByteArray& bytearray)
{
  TERRAMA2_LOG_DEBUG() << "JSon size: " << bytearray.size();
  TERRAMA2_LOG_DEBUG() << QString(bytearray);
  QJsonParseError error;
  QJsonDocument jsonDoc = QJsonDocument::fromJson(bytearray, &error);

  if(error.error != QJsonParseError::NoError)
    TERRAMA2_LOG_ERROR() << QObject::tr("Error receiving remote configuration.\nJson parse error: %1\n").arg(error.errorString());
  else
  {
    if(jsonDoc.isObject())
    {
      auto obj = jsonDoc.object();
      serviceManager_->updateService(obj);
    }
    else
      TERRAMA2_LOG_ERROR() << QObject::tr("Error receiving remote configuration.\nJson is not an object.\n");
  }
}

void terrama2::core::TcpManager::addData(const QByteArray& bytearray)
{
  TERRAMA2_LOG_DEBUG() << "JSon size: " << bytearray.size();
  TERRAMA2_LOG_DEBUG() << QString(bytearray);
  QJsonParseError error;
  QJsonDocument jsonDoc = QJsonDocument::fromJson(bytearray, &error);

  if(error.error != QJsonParseError::NoError)
    TERRAMA2_LOG_ERROR() << QObject::tr("Error receiving remote configuration.\nJson parse error: %1\n").arg(error.errorString());
  else
  {
    std::shared_ptr<terrama2::core::DataManager> dataManager = dataManager_.lock();
    if(jsonDoc.isObject())
    {
      auto obj = jsonDoc.object();
      dataManager->addJSon(obj);
    }
    else
      TERRAMA2_LOG_ERROR() << QObject::tr("Error receiving remote configuration.\nJson is not an object.\n");
  }
}

void terrama2::core::TcpManager::removeData(const QByteArray& bytearray)
{
  TERRAMA2_LOG_DEBUG() << "JSon size: " << bytearray.size();
  TERRAMA2_LOG_DEBUG() << QString(bytearray);
  QJsonParseError error;
  QJsonDocument jsonDoc = QJsonDocument::fromJson(bytearray, &error);

  if(error.error != QJsonParseError::NoError)
    TERRAMA2_LOG_ERROR() << QObject::tr("Error receiving remote configuration.\nJson parse error: %1\n").arg(error.errorString());
  else
  {
    std::shared_ptr<terrama2::core::DataManager> dataManager = dataManager_.lock();
    if(jsonDoc.isObject())
    {
      auto obj = jsonDoc.object();
      dataManager->removeJSon(obj);
    }
    else
      TERRAMA2_LOG_ERROR() << QObject::tr("Error receiving remote configuration.\nJson is not an object.\n");
  }
}

QJsonObject terrama2::core::TcpManager::logToJson(const terrama2::core::ProcessLogger::Log& log)
{
  QJsonObject obj;
  obj.insert("process_id", static_cast<int>(log.processId));
  obj.insert("status", static_cast<int>(log.status));
  obj.insert("start_timestamp", log.start_timestamp.get() ? QString::fromStdString(log.start_timestamp->toString()) : "");
  obj.insert("data_timestamp", log.data_timestamp.get() ? QString::fromStdString(log.data_timestamp->toString()) : "");
  obj.insert("last_process_timestamp", log.last_process_timestamp.get() ? QString::fromStdString(log.last_process_timestamp->toString()) : "");
  obj.insert("data", QString::fromStdString(log.data));

  QJsonArray msgArray;
  for(const auto& msg  : log.messages)
  {
    QJsonObject msgObj;
    msgObj.insert("type", static_cast<int>(msg.type));
    msgObj.insert("description", QString::fromStdString(msg.description));
    msgObj.insert("timestamp", msg.timestamp.get() ? QString::fromStdString(msg.timestamp->toString()) : "");

    msgArray.append(msgObj);
  }
  obj.insert("messages", msgArray);

  return obj;
}

bool terrama2::core::TcpManager::sendLog(const QByteArray& bytearray, QTcpSocket* tcpSocket)
{
  QJsonParseError error;
  QJsonDocument jsonDoc = QJsonDocument::fromJson(bytearray, &error);

  if(error.error != QJsonParseError::NoError)
  {
    TERRAMA2_LOG_ERROR() << QObject::tr("Error receiving remote configuration.\nJson parse error: %1\n").arg(error.errorString());
    return false;
  }
  else
  {
    auto jsonObject = jsonDoc.object();
    auto idsArray = jsonObject.value("process_ids").toArray();

    uint32_t begin = static_cast<uint32_t>(jsonObject.value("begin").toInt());
    uint32_t end = static_cast<uint32_t>(jsonObject.value("end").toInt());

    QJsonArray logList;
    for(const auto& value : idsArray)
    {
      auto processId = static_cast<ProcessId>(value.toInt());

      QJsonArray processLogList;
      auto logger = logger_.lock();
      auto logs = logger->getLogs(processId, begin, end);
      for(const auto& log : logs)
      {
        processLogList.append(logToJson(log));
      }

      QJsonObject obj;
      obj.insert("process_id",  static_cast<int>(processId));
      obj.insert("log", processLogList);

      logList.push_back(obj);
    }

    QJsonDocument doc(logList);

    QByteArray logArray;
    QDataStream out(&logArray, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_2);

    out << static_cast<uint32_t>(0);
    out << static_cast<uint32_t>(TcpSignal::LOG_SIGNAL);

    out << doc.toJson(QJsonDocument::Compact);
    logArray.remove(8, 4);//Remove QByteArray header
    out.device()->seek(0);
    out << static_cast<uint32_t>(logArray.size() - sizeof(uint32_t));

    // wait while sending message
    qint64 written = tcpSocket->write(logArray);
    if(written == -1 || !tcpSocket->waitForBytesWritten(30000))
    {
      // couldn't write to socket
      return false;
    }
    else
      return true;
  }
}

void terrama2::core::TcpManager::sendTerminateSignal(QTcpSocket* tcpSocket)
{
  TERRAMA2_LOG_DEBUG() << "sending TERMINATE_SERVICE_SIGNAL";
  QByteArray bytearray;
  QDataStream out(&bytearray, QIODevice::WriteOnly);

  out << static_cast<uint32_t>(0);
  out << static_cast<uint32_t>(TcpSignal::TERMINATE_SERVICE_SIGNAL);
  out.device()->seek(0);
  out << static_cast<uint32_t>(bytearray.size() - sizeof(uint32_t));

  // wait while sending message
  qint64 written = tcpSocket->write(bytearray);
  if(written == -1 || !tcpSocket->waitForBytesWritten(30000))
    TERRAMA2_LOG_WARNING()
        << QObject::tr("Unable to establish connection with server.");

  return;
}

void terrama2::core::TcpManager::readReadySlot(QTcpSocket* tcpSocket) noexcept
{
  try
  {
    {
      QDataStream in(tcpSocket);
      TERRAMA2_LOG_DEBUG() << "bytes available: " << tcpSocket->bytesAvailable();

      if(blockSize_ == 0)
      {
        if(tcpSocket->bytesAvailable() < static_cast<int>(sizeof(uint32_t)))
        {
          TERRAMA2_LOG_ERROR() << QObject::tr("Error receiving remote configuration.\nInvalid message size.");
          return;
        }


        in >> blockSize_;
        TERRAMA2_LOG_DEBUG() << "message size: " << blockSize_;
      }

      if(tcpSocket->bytesAvailable() < blockSize_)
      {
        //The message isn't complete, wait for next readReady signal
        return;
      }
      //Raii block
      RaiiBlock block(blockSize_);
      Q_UNUSED(block)

      int sigInt = -1;
      in >> sigInt;

      //read signal
      TcpSignal signal = static_cast<TcpSignal>(sigInt);
      //update left blockSize
      blockSize_-=sizeof(TcpSignal);

      if(signal != TcpSignal::UPDATE_SERVICE_SIGNAL && !serviceManager_->serviceLoaded())
      {
        // wait for TcpSignals::UPDATE_SERVICE_SIGNAL
        return;
      }

      switch(signal)
      {
        case TcpSignal::UPDATE_SERVICE_SIGNAL:
        {
          QByteArray bytearray = tcpSocket->read(blockSize_);

          updateService(bytearray);
          break;
        }
        case TcpSignal::TERMINATE_SERVICE_SIGNAL:
        {
          TERRAMA2_LOG_DEBUG() << "TERMINATE_SERVICE_SIGNAL";

          serviceManager_->setShuttingDownProcessInitiated();

          emit stopSignal();

          sendTerminateSignal(tcpSocket);

          emit closeApp();

          break;
        }
        case TcpSignal::ADD_DATA_SIGNAL:
        {
          TERRAMA2_LOG_DEBUG() << "ADD_DATA_SIGNAL";
          QByteArray bytearray = tcpSocket->read(blockSize_);

          addData(bytearray);
          break;
        }
        case TcpSignal::REMOVE_DATA_SIGNAL:
        {
          TERRAMA2_LOG_DEBUG() << "REMOVE_DATA_SIGNAL";
          QByteArray bytearray = tcpSocket->read(blockSize_);

          removeData(bytearray);
          break;
        }
        case TcpSignal::START_PROCESS_SIGNAL:
        {
          TERRAMA2_LOG_DEBUG() << "START_PROCESS_SIGNAL";
          uint32_t dataId;
          in >> dataId;

          emit startProcess(dataId);

          break;
        }
        case TcpSignal::STATUS_SIGNAL:
        {
          TERRAMA2_LOG_DEBUG() << "STATUS_SIGNAL";
          QByteArray bytearray;
          QDataStream out(&bytearray, QIODevice::WriteOnly);

          auto jsonObj = ServiceManager::getInstance().status();
          QJsonDocument doc(jsonObj);

          out << static_cast<uint32_t>(0);
          out << static_cast<uint32_t>(TcpSignal::STATUS_SIGNAL);
          out << doc.toJson(QJsonDocument::Compact);
          bytearray.remove(8, 4);//Remove QByteArray header
          out.device()->seek(0);
          out << static_cast<uint32_t>(bytearray.size() - sizeof(uint32_t));

          // wait while sending message
          qint64 written = tcpSocket->write(bytearray);
          if(written == -1 || !tcpSocket->waitForBytesWritten(30000))
            TERRAMA2_LOG_WARNING() << QObject::tr("Unable to establish connection with server.");

          break;
        }
        case TcpSignal::LOG_SIGNAL:
        {
          TERRAMA2_LOG_DEBUG() << "LOG_SIGNAL";
          QByteArray bytearray = tcpSocket->read(blockSize_);

          sendLog(bytearray, tcpSocket);
          break;
        }
        default:
          TERRAMA2_LOG_ERROR() << QObject::tr("Error\n Unknown signal received.");
          break;
      }
    }//end of Raii block

    if(tcpSocket && !tcpSocket->atEnd())
      readReadySlot(tcpSocket);
  }
  catch(...)
  {
    // exception guard, slots should never emit exceptions.
    TERRAMA2_LOG_ERROR() << QObject::tr("Unknown exception...");
  }
}

void terrama2::core::TcpManager::receiveConnection() noexcept
{
  try
  {
    TERRAMA2_LOG_INFO() << QObject::tr("Receiving new configuration...");

    QTcpSocket* tcpSocket(nextPendingConnection());
    if(!tcpSocket)
      return;

    connect(tcpSocket, &QTcpSocket::readyRead, [this, tcpSocket]() { readReadySlot(tcpSocket);});
  }
  catch(...)
  {
    // exception guard, slots should never emit exceptions.
    TERRAMA2_LOG_ERROR() << QObject::tr("Unknown exception...");
  }

  return;
}
