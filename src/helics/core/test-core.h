/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_TEST_CORE_
#define _HELICS_TEST_CORE_

#include "helics/config.h"
#include "helics-time.h"
#include "helics/common/blocking_queue.h"
#include "helics/core/core.h"
#include "core-common.h"
//#include "test-broker.h"

#include <cstdint>
#include <mutex> 
#include <thread> 
#include <utility> 
#include <vector> 

namespace helics {

class TestBroker;

class TestCore : public CommonCore {

public:
  TestCore() {};
  TestCore(std::shared_ptr<TestBroker> nbroker);
  virtual ~TestCore();
  virtual void initialize (const std::string &initializationString) override;

protected:
	virtual void transmit(int route_id, ActionMessage &cmd) override;
	virtual void addRoute(int route_id, const std::string &routeInfo) override;
private:
	std::shared_ptr<TestBroker> tbroker;  //the underlying broker;
	//void computeDependencies();
	std::map<int32_t, std::shared_ptr<TestBroker>> brokerRoutes;
	std::map < int32_t, std::shared_ptr<TestCore>>  coreRoutes;
	mutable std::mutex routeMutex;
};

/** container for builing a bunch of test cores and brokers*/
std::shared_ptr<TestCore> findTestCore(const std::string &name);

std::shared_ptr<TestBroker> findTestBroker(const std::string &brokerName);

void registerTestCore(std::shared_ptr<TestCore> tcore);

void registerTestBroker(std::shared_ptr<TestBroker> tbroker);

} // namespace helics
 
#endif /* _HELICS_TEST_CORE_ */
