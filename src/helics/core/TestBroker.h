/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once
#include "CoreBroker.hpp"

namespace helics
{
class CommonCore;
namespace testcore
{
/** class implementing a basic broker that links to other brokers in process memory*/
class TestBroker : public CoreBroker
{
  public:
    /** default constructor*/
    TestBroker (bool isRoot_ = false) noexcept;
    TestBroker (const std::string &broker_name);
    /** construct with a pointer to a broker*/
    TestBroker (std::shared_ptr<TestBroker> nbroker);
    virtual ~TestBroker ();
    virtual void initializeFromArgs (int argc, const char *const *argv) override;

  protected:
    virtual void transmit (int32_t route, const ActionMessage &command) override;

    virtual void addRoute (int route_id, const std::string &routeInfo) override;

  public:
    virtual std::string getAddress () const override;
    /** static method to display the help message*/
    static void displayHelp (bool localOnly = false);

  protected:
    virtual bool brokerConnect () override;
    virtual void brokerDisconnect () override;
    virtual bool tryReconnect () override;

  private:
    std::string brokerName;  //!< the name of the higher level broker to connect to
    std::string brokerInitString;  //!< the initialization string for the higher level broker
    std::shared_ptr<CoreBroker> tbroker;  // the parent broker;
                                          // void computeDependencies();
    std::map<int32_t, std::shared_ptr<CoreBroker>> brokerRoutes;  //!< map of the routes to other brokers
    std::map<int32_t, std::shared_ptr<CommonCore>> coreRoutes;  //!< map of the routes to other cores
    mutable std::mutex routeMutex;  //!< mutex lock protecting the route maps
};
} // namespace testcore
} //namespace helics

