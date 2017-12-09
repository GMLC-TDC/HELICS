/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "TestCore.h"
#include "ActionMessage.h"
#include "BrokerFactory.h"
#include "CoreBroker.h"
#include "CoreFactory.h"
#include "argParser.h"
#include <fstream>

namespace helics
{
using federate_id_t = Core::federate_id_t;
using Handle = Core::Handle;

TestCore::TestCore (const std::string &core_name) : CommonCore (core_name) {}

TestCore::TestCore (std::shared_ptr<CoreBroker> nbroker) : tbroker (std::move (nbroker)) {}
using namespace std::string_literals;
static const argDescriptors extraArgs{{"brokername"s, "string"s, "identifier for the broker-same as broker"s},
                                      {"brokerinit"s, "string"s, "the initialization string for the broker"s}};

void TestCore::initializeFromArgs (int argc, const char *const *argv)
{
    namespace po = boost::program_options;
    bool exp = false;
    if (initialized_.compare_exchange_strong (exp, true))
    {
        po::variables_map vm;

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

}  // namespace helics
