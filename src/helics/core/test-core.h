/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_TEST_CORE_
#define _HELICS_TEST_CORE_

#include "core-common.h"
//#include "test-broker.h"


namespace helics {

class TestBroker;
/** an object implementing a local core object that can communicate in process
*/
class TestCore : public CommonCore {

public:
	/** default constructor*/
  TestCore()=default;
  //* construct with a pointer to a broker*
  TestCore(std::shared_ptr<TestBroker> nbroker);
  /** destructor*/
  virtual ~TestCore();  //the destructor is defined so the definition of TestBroker does not need to be available in the header
  virtual void initialize (const std::string &initializationString) override;

protected:
	virtual void transmit(int route_id, const ActionMessage &cmd) override;
	virtual void addRoute(int route_id, const std::string &routeInfo) override;
private:
	std::shared_ptr<TestBroker> tbroker;  //the underlying broker;
	//void computeDependencies();
	std::map<int32_t, std::shared_ptr<TestBroker>> brokerRoutes; //!< map of the the different brokers
	std::map < int32_t, std::shared_ptr<TestCore>>  coreRoutes;	//!< map of the different cores that can be routed to
	mutable std::mutex routeMutex;   //!< mutex that protects the routing info
};

/** container for builing a bunch of test cores and brokers*/
/** locate a TestCore by name
@param name the name of the core to find
@return a shared_ptr to the testCore*/
std::shared_ptr<TestCore> findTestCore(const std::string &name);
/** locate a TestBroker by name
@param name the name of the broker
@return a shared_ptr to the testBroker*/
std::shared_ptr<TestBroker> findTestBroker(const std::string &brokerName);

/** register a testCore so it can be found by others
@param tcore a pointer to a testCore object that should be found globally*/
void registerTestCore(std::shared_ptr<TestCore> tcore);
/** register a testBroker so it can be found by others
@param tbroker a pointer to a testBroker object that should be able to be found globally*/
void registerTestBroker(std::shared_ptr<TestBroker> tbroker);

} // namespace helics
 
#endif /* _HELICS_TEST_CORE_ */
