#pragma once

#include "dji_status.hpp" 
#include <dji_vehicle.hpp>
#include <dji_linux_helpers.hpp>
#include <iostream>

#include "LinuxChrono.h"

class DataContainer
{
public:
  DataContainer();
  ~DataContainer();

  DJI::OSDK::Telemetry::RCFullRawData RC;
  DJI::OSDK::Telemetry::Battery       Bat;
  DJI::OSDK::Telemetry::GlobalPosition GPS;

private:
};

DataContainer::DataContainer()
{
}

DataContainer::~DataContainer()
{
}

class Data
{
public:
  Data(DataContainer* dataContainer_,pthread_mutex_t* mutex_);
  ~Data();
  DJI::OSDK::Telemetry::RCFullRawData RC()
  {
	  pthread_mutex_lock(mutex);
      auto res = dataContainer->RC;
      pthread_mutex_unlock(mutex);
      return res;
      }

      DJI::OSDK::Telemetry::Battery Bat()
      {
        pthread_mutex_lock(mutex);
        auto res = dataContainer->Bat;
        pthread_mutex_unlock(mutex);
        return res;
      }

      DJI::OSDK::Telemetry::GlobalPosition GPS()
      {
        pthread_mutex_lock(mutex);
        auto res = dataContainer->GPS;
        pthread_mutex_unlock(mutex);
        return res;
      }

private:
  DataContainer *dataContainer;
  pthread_mutex_t *mutex;
};

Data::Data(DataContainer* dataContainer_,pthread_mutex_t* mutex_)
{
  dataContainer=dataContainer_;
  mutex        = mutex_;
}

Data::~Data()
{
}

class LinuxA3
{

public:
  LinuxA3(Vehicle* v_);
  ~LinuxA3();
  Data data;

private:
  pthread_mutex_t              mutex;
  common_things::Alarm         alarm;
  void*          upd(int);
  DataContainer dataContainer;
  Vehicle* v;
};

LinuxA3::LinuxA3(Vehicle* v_)
  : v(v_),data(&dataContainer,&mutex)
{
  pthread_mutex_init(&mutex, NULL);

  ACK::ErrorCode subscribeStatus;
  subscribeStatus = v->subscribe->verify(5000);

  int                             pkgIndex        = 0;
  int                             freq            = 50;
  DJI::OSDK::Telemetry::TopicName topicList50Hz[] = {
    DJI::OSDK::Telemetry::TOPIC_RC_FULL_RAW_DATA,
    DJI::OSDK::Telemetry::TOPIC_BATTERY_INFO
  };

  int  numTopic        = sizeof(topicList50Hz) / sizeof(topicList50Hz[0]);
  bool enableTimestamp = false;

  bool pkgStatus = v->subscribe->initPackageFromTopicList(
    pkgIndex, numTopic, topicList50Hz, enableTimestamp, freq);

  subscribeStatus = v->subscribe->startPackage(pkgIndex, 5000);
  auto f          = std::bind(&LinuxA3::upd, this, std::placeholders::_1);
  alarm.start(f, 1000 * 100, 1);
}

void*
LinuxA3::upd(int)
{
  auto RC =
    v->subscribe->getValue<DJI::OSDK::Telemetry::TOPIC_RC_FULL_RAW_DATA>();
  auto Bat =
    v->subscribe->getValue<DJI::OSDK::Telemetry::TOPIC_BATTERY_INFO>();
  auto GPS = v->broadcast->getGlobalPosition();
  pthread_mutex_lock(&mutex);
  dataContainer.RC = RC;
  dataContainer.Bat = Bat;
  dataContainer.GPS = GPS;
  pthread_mutex_unlock(&mutex);
}

LinuxA3::~LinuxA3()
{
}