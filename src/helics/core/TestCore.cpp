/*

Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "TestCore.h"
#include "../common/argParser.h"
#include "ActionMessage.hpp"
#include "BrokerFactory.hpp"
#include "CoreBroker.hpp"
#include "CoreFactory.hpp"
#include <fstream>

namespace helics
{
using federate_id_t = Core::federate_id_t;
using handle_id_t = Core::handle_id_t;

namespace testcore
{
TestCore::TestCore (const std::string &core_name) : CommonCore (core_name) {}

TestCore::TestCore (std::shared_ptr<CoreBroker> nbroker) : tbroker (std::move (nbroker)) {}
using namespace std::string_literals;
static const ArgDescriptors extraArgs{{"brokername"s, "identifier for the broker-same as broker"s},
                                      {"broker,b"s, "identifier for the broker"s},
                                      {"broker_address", "location of the broker i.e network address"},
                                      {"brokerinit"s, "the initialization string for the broker"s}};

void TestCore::initializeFromArgs (int argc, const char *const *argv)
{
    bool exp = false;
    if (initialized_.compare_exchange_strong (exp, true))
    {
        variable_map vm;
        argumentParser (argc, argv, vm, extraArgs);

        if (vm.count ("broker") > 0)
        {
            brokerName = vm["broker"].as<std::string> ();
        }
        else if (vm.count ("brokername") > 0)
        {
            brokerName = vm["brokername"].as<std::string> ();
        }
        else if (vm.count ("broker_address") > 0)
        {
            brokerName = vm["broker_address"].as<std::string> ();
        }
        if (vm.count ("brokerinit") > 0)
        {
            brokerInitString = vm["brokerinit"].as<std::string> ();
        }
    }
    if (brokerState == created)
    {
        CommonCore::initializeFromArgs (argc, argv);
    }
}

bool TestCore::brokerConnect ()
{
    std::lock_guard<std::mutex> lock (routeMutex);
    if (!tbroker)
    {
        auto broker = BrokerFactory::findBroker (brokerName);
        tbroker = std::dynamic_pointer_cast<CoreBroker> (broker);
        if (!tbroker)
        {
            tbroker = std::static_pointer_cast<CoreBroker> (
              BrokerFactory::create (core_type::TEST, brokerName, brokerInitString));
        }
        else
        {
            if (!tbroker->isOpenToNewFederates ())
            {
                tbroker = nullptr;
                broker = nullptr;
                BrokerFactory::cleanUpBrokers (200);
                broker = BrokerFactory::findBroker (brokerName);
                tbroker = std::dynamic_pointer_cast<CoreBroker> (broker);
                if (!tbroker)
                {
                    tbroker = std::static_pointer_cast<CoreBroker> (
                      BrokerFactory::create (core_type::TEST, brokerName, brokerInitString));
                }
                else
                {
                    if (!tbroker->isOpenToNewFederates ())
                    {
                        tbroker = nullptr;
                        broker = nullptr;
                    }
                }
            }
        }
    }
    if (tbroker)
    {
        tbroker->connect ();
    }
    return static_cast<bool> (tbroker);
}

bool TestCore::tryReconnect ()
{
    auto broker = BrokerFactory::findBroker (brokerName);
    tbroker = std::dynamic_pointer_cast<CoreBroker> (broker);
    return static_cast<bool> (tbroker);
}

void TestCore::brokerDisconnect ()
{
    brokerState = broker_state_t::terminating;
    std::lock_guard<std::mutex> lock (routeMutex);
    tbroker = nullptr;
}

TestCore::~TestCore ()
{
    haltOperations = true;
    joinAllThreads ();
    // lock to ensure all the data is synchronized before deletion
    std::lock_guard<std::mutex> lock (routeMutex);
}

void TestCore::transmit (int route_id, const ActionMessage &cmd)
{
    std::lock_guard<std::mutex> lock (routeMutex);
    if (route_id == 0)
    {
        if (tbroker)
        {
            tbroker->addActionMessage (cmd);
        }

        return;
    }

    auto brkfnd = brokerRoutes.find (route_id);
    if (brkfnd != brokerRoutes.end ())
    {
        brkfnd->second->addActionMessage (cmd);
        return;
    }
    auto crfnd = coreRoutes.find (route_id);
    if (crfnd != coreRoutes.end ())
    {
        crfnd->second->addActionMessage (cmd);
        return;
    }
    if (tbroker)
    {
        tbroker->addActionMessage (cmd);
    }
}

void TestCore::addRoute (int route_id, const std::string &routeInfo)
{
    auto brk = BrokerFactory::findBroker (routeInfo);
    if (brk)
    {
        auto cbrk = std::dynamic_pointer_cast<CoreBroker> (brk);
        if (cbrk)
        {
            std::lock_guard<std::mutex> lock (routeMutex);
            brokerRoutes.emplace (route_id, std::move (cbrk));
        }
        return;
    }
    auto core = CoreFactory::findCore (routeInfo);
    if (core)
    {
        auto tcore = std::dynamic_pointer_cast<CommonCore> (core);
        if (tcore)
        {
            std::lock_guard<std::mutex> lock (routeMutex);
            coreRoutes.emplace (route_id, std::move (tcore));
        }
        return;
    }
    // the route will default to the central route
}

std::string TestCore::getAddress () const { return getIdentifier (); }

}  // namespace testcore
}  // namespace helics
