/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "CommonCore.hpp"
//#include "test-broker.h"


namespace helics
{
class CoreBroker;
namespace testcore
{
/** an object implementing a local core object that can communicate in process
 */
class TestCore final : public CommonCore
{
public:
    /** default constructor*/
    TestCore() = default;
    /**construct from a core name*/
    TestCore(const std::string &core_name);
    /** construct with a pointer to a broker*/
    TestCore(std::shared_ptr<CoreBroker> nbroker);
    /** destructor*/
    virtual ~TestCore();  // the destructor is defined so the definition of TestBroker does not need to be
                           // available in the header
    virtual void initializeFromArgs(int argc, const char * const *argv) override;

protected:
    virtual void transmit(route_id_t route_id, const ActionMessage &cmd) override;
  virtual void transmit (route_id_t route_id, ActionMessage &&cmd) override;
    virtual void addRoute(route_id_t route_id, const std::string &routeInfo) override;
public:
    virtual std::string generateLocalAddressString () const override;

  private:
    //these should only be called by the CommonCore code
    virtual bool brokerConnect() override;
    virtual void brokerDisconnect() override;
    virtual bool tryReconnect() override;
private:
    std::atomic<bool> initialized_{ false };  //!< atomic protecting local initialization
	bool autoBroker = false;
    std::shared_ptr<CoreBroker> tbroker;  //!<the parent broker;
    std::string brokerInitString;  //!< the initialization string to use for the Broker
    std::string brokerName;  //!< the name of the broker to connect to
    // void computeDependencies();
    std::map<route_id_t, std::shared_ptr<CoreBroker>> brokerRoutes;  //!< map of the different brokers
    std::map<route_id_t, std::shared_ptr<CommonCore>> coreRoutes;  //!< map of the different cores that can be routed to
    mutable std::mutex routeMutex;  //!< mutex that protects the routing info
};

} // namespace testcore
}  // namespace helics

