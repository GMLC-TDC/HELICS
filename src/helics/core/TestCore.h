/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_TEST_CORE_
#define _HELICS_TEST_CORE_

#include "CommonCore.h"
//#include "test-broker.h"


namespace helics
{
class CoreBroker;
/** an object implementing a local core object that can communicate in process
 */
class TestCore : public CommonCore
{
  public:
    /** default constructor*/
    TestCore () = default;
	/**construct from a core name*/
	TestCore(const std::string &core_name);
    /** construct with a pointer to a broker*/
    TestCore (std::shared_ptr<CoreBroker> nbroker);
    /** destructor*/
    virtual ~TestCore ();  // the destructor is defined so the definition of TestBroker does not need to be
                           // available in the header
    virtual void InitializeFromArgs (int argc, char *argv[]) override;

  protected:
    virtual void transmit (int route_id, const ActionMessage &cmd) override;
    virtual void addRoute (int route_id, const std::string &routeInfo) override;
public:
	virtual std::string getAddress() const override;
private:
	//these should only be called by the CommonCore code
	virtual bool brokerConnect() override;
	virtual void brokerDisconnect() override;
private:
	  std::atomic<bool> initialized_{ false };  //!< atomic protecting local initialization
    std::shared_ptr<CoreBroker> tbroker;  //!<the parent broker;
	std::string brokerInitString;  //!< the initialization string to use for the Broker
	std::string brokerName;  //!< the name of the broker to connect to
    // void computeDependencies();
    std::map<int32_t, std::shared_ptr<CoreBroker>> brokerRoutes;  //!< map of the the different brokers
    std::map<int32_t, std::shared_ptr<CommonCore>> coreRoutes;  //!< map of the different cores that can be routed to
    mutable std::mutex routeMutex;  //!< mutex that protects the routing info
};


}  // namespace helics

#endif /* _HELICS_TEST_CORE_ */
