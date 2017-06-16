/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/core/core-factory.h"
#include "helics/core/core.h"
#include "helics/core/core-types.h"
#include "helics/common/barrier.hpp"

#include <atomic>
#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <mutex>
#include <thread>

#define SUCCESS false
#define FAILURE true

#define USE_GLOG 0
#if defined(HELICS_HAVE_GLOG) && USE_GLOG
#include <glog/logging.h>
#define ENDL ""
#else
static std::ostream& log(std::string str, bool condition) {
  static std::mutex mutex;
  static std::ostringstream NullStream;
  std::unique_lock<std::mutex> lock(mutex);
  if (!condition) {
    std::cout << str << std::endl;
    assert(condition);
    return NullStream;
  }
  else {
    return std::cout;
  }
}
#define LOG(LEVEL) std::cout
#define ENDL std::endl
#define CHECK(A) log(#A, A)
#define CHECK_EQ(A,B) log(#A " == " #B, A == B)
#define CHECK_STREQ(A,B) log(#A " == " #B, std::string(A) == std::string(B))
#endif
using helics::CoreFederateInfo;
using helics::Core;
using helics::CoreFactory;

static helics::Barrier barrier(4);

void simA (Core * core, const char *NAME)
{
  Core::federate_id_t id = core->registerFederate(
      NAME,
      CoreFederateInfo()
  );
  LOG(INFO) << NAME << ": " << "id " << id << ENDL;
  CHECK_STREQ(NAME, core->getFederateName(id));
  CHECK_EQ(id, core->getFederateId(NAME));

  // simple barrier
  barrier.Wait();
  LOG(INFO) << NAME << ": exiting barrier" << ENDL;

  core->setTimeDelta(id, 1);
  Core::Handle sub1 = core->registerSubscription(id, NAME, "type", "units", true);
  CHECK_EQ(sub1, core->getSubscription(id, NAME));

  Core::Handle pub1 = core->registerPublication(id, NAME, "type", "units");

  Core::Handle end1 = core->registerEndpoint(id, NAME, "type");

  core->enterInitializingState(id);
  LOG(INFO) << NAME << ": " << "entered initializing state" << ENDL;

  core->enterExecutingState(id);
  LOG(INFO) << NAME << ": " << "entered executing state" << ENDL;

  // time loop
  helics::data_t *data;
  const Core::Handle *events;
  uint64_t events_size = 0;

  core->timeRequest(id, 50);
  std::string str1 = "hello world";
  core->setValue(pub1, str1.data(), str1.size());
  //data = core->getValue(pub1); // this would assert
  data = core->getValue(sub1); // should be empty so far
  events = core->getValueUpdates(id, &events_size);
  assert(0 == events_size);
  LOG(INFO) << "GET VALUE size " << data->len << ENDL;
  core->dereference(data);
  core->timeRequest(id, 100);
  events = core->getValueUpdates(id, &events_size);
  assert(1 == events_size);
  data = core->getValue(sub1);
  
  LOG(INFO) << "GET VALUE size " << data->len << " and handle " << *events << ENDL;
  std::string str2(data->data, data->len);
  
  CHECK_EQ(str1, str2);
  assert(data->len == str1.size());
  core->dereference(data);
  core->setValue(pub1, "hello\n\0helloAgain", 17);
  core->timeRequest(id, 150);
  events = core->getValueUpdates(id, &events_size);
  LOG(INFO) << "events_size=" << events_size << ENDL;
  assert(1 == events_size);
  data = core->getValue(sub1);
  LOG(INFO) << "GET VALUE size " << data->len << " and handle " << *events << ENDL;
  assert(data->len == 17);
  core->dereference(data);

  core->timeRequest(id, 200);
  core->send(end1, "simA1", "test123", 8);
  core->timeRequest(id, 250);
  LOG(INFO) << NAME << " RECEIVECOUNT " << core->receiveCount(end1) << ENDL;
  while (core->receiveCount(end1) > 0) {
	  auto end1_recv = core->receive(end1);
	  LOG(INFO) << NAME << " RECEIVE " << end1_recv->data << ENDL;
  }

  core->finalize(id);
}


void simB (Core * core, const char *NAME)
{
  Core::federate_id_t id = core->registerFederate(
      NAME,
      CoreFederateInfo()
  );
  LOG(INFO) << NAME << ": " << "id " << id << ENDL;
  CHECK_STREQ(NAME, core->getFederateName(id));
  CHECK_EQ(id, core->getFederateId(NAME));

  // simple barrier
  barrier.Wait();
  LOG(INFO) << NAME << ": exiting barrier" << ENDL;

  core->setTimeDelta(id, 1);
  Core::Handle sub1 = core->registerSubscription(id, NAME, "type", "units", true);
  CHECK_EQ(sub1, core->getSubscription(id, NAME));

  core->enterInitializingState(id);
  LOG(INFO) << NAME << ": " << "entered initializing state" << ENDL;

  core->enterExecutingState(id);
  LOG(INFO) << NAME << ": " << "entered executing state" << ENDL;

  // time loop
  
  core->requestTimeIterative(id, 100, false);
  core->requestTimeIterative(id, 100, true);
  core->requestTimeIterative(id, 105, false);
  core->requestTimeIterative(id, 105, true);
  core->finalize(id);

}


int main (int argc, char **argv)
{
#if defined(HELICS_HAVE_GLOG) && USE_GLOG
  google::InitGoogleLogging(argv[0]);
#endif

  const char *initializationString;
  helics_core_type type;
  if (argc == 1) {
    initializationString = "4";
    type = HELICS_TEST;
    LOG(INFO) << "Single-node Core Factory Test" << ENDL;
  }
  else {
    initializationString = "1";
    type = HELICS_ZMQ;
    LOG(INFO) << "ZeroMQ Core Factory Test" << ENDL;
  }

  Core* core = CoreFactory::create (
      type, initializationString);

  if (core) {
    LOG(INFO) << "Created Test Core" << ENDL;
  }
  else {
    LOG(INFO) << "Failed to create Test Core" << ENDL;
    return FAILURE;
  }

  CHECK(core->isInitialized());

  //core->setFederationSize(4);

  std::thread simA1_thread(simA, core, "simA1");
  std::thread simA2_thread(simA, core, "simA2");
  std::thread simB1_thread(simB, core, "simB1");
  std::thread simB2_thread(simB, core, "simB2");

  simA1_thread.join();
  simA2_thread.join();
  simB1_thread.join();
  simB2_thread.join();

  delete core;

  LOG(INFO) << "Deleted core" << ENDL;

  return SUCCESS;
}

