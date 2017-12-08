/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "TestBroker.h"
#include "BrokerFactory.h"
#include "CoreFactory.h"
#include "TestCore.h"

#include "argParser.h"
#include <fstream>

namespace helics
{
TestBroker::TestBroker (bool rootBroker) noexcept : CoreBroker (rootBroker) {}

TestBroker::TestBroker (const std::string &broker_name) : CoreBroker (broker_name) {}

TestBroker::TestBroker (std::shared_ptr<TestBroker> nbroker) : tbroker (std::move (nbroker)) {}

TestBroker::~TestBroker ()
{
    haltOperations = true;
    // lock to ensure all the data is synchronized before deletion
    std::unique_lock<std::mutex> lock (routeMutex);
    lock.unlock ();
    joinAllThreads ();
}
using namespace std::string_literals;
static const argDescriptors extraArgs{{"brokername"s, "string"s, "identifier for the broker-same as broker"s},
                                      {"brokerinit"s, "string"s, "the initialization string for the broker"s}};

void TestBroker::displayHelp (bool localOnly)
{
    std::cout << " Help for Test Broker: \n";
    namespace po = boost::program_options;
    po::variables_map vm;
    const char *const argV[] = {"", "--help"};
    argumentParser (2, argV, vm, extraArgs);
    if (!localOnly)
    {
        CoreBroker::displayHelp ();
    }
}

void TestBroker::initializeFromArgs (int argc, const char *const *argv)
{
    namespace po = boost::program_options;
    if (brokerState == broker_state_t::created)
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
        CoreBroker::initializeFromArgs (argc, argv);
    };
}

bool TestBroker::brokerConnect ()
{
    std::lock_guard<std::mutex> lock (routeMutex);
    if (!tbroker)
    {
        if (isRoot ())
        {
            return true;
        }
        if ((brokerName.empty ()) && (brokerInitString.empty ()))
        {
            setAsRoot ();
            return true;
        }

        auto broker = BrokerFactory::findBroker (brokerName);
        if (broker)
        {
            tbroker = std::static_pointer_cast<helics::CoreBroker> (broker);
        }
        else
        {
            tbroker = std::static_pointer_cast<helics::CoreBroker> (
              BrokerFactory::create (core_type::TEST, brokerName, brokerInitString));
        }
    }

    return static_cast<bool> (tbroker);
}

bool TestBroker::tryReconnect()
{
    
    if (isRoot())
    {
        return true;
    }
    auto broker = BrokerFactory::findBroker(brokerName);
    tbroker = std::dynamic_pointer_cast<CoreBroker> (broker);
    return static_cast<bool> (tbroker);
}

void TestBroker::brokerDisconnect ()
{
    brokerState = broker_state_t::terminating;
    std::lock_guard<std::mutex> lock (routeMutex);
    tbroker = nullptr;
}

void TestBroker::transmit (int32_t route_id, const ActionMessage &cmd)
{
    if (brokerState >= broker_state_t::terminating)
    {
        return;  // no message sent in terminating or higher state
    }
    // only activate the lock if we not in an operating state
    std::unique_lock<std::mutex> lock (routeMutex);

    if ((tbroker) && (route_id == 0))
    {
        tbroker->addActionMessage (cmd);
        return;
    }

    auto brkfnd = brokerRoutes.find (route_id);
    if (brkfnd != brokerRoutes.end ())
    {
        auto tmp = brkfnd->second;
        lock.unlock ();
        tmp->addActionMessage (cmd);
        return;
    }
    auto crfnd = coreRoutes.find (route_id);
    if (crfnd != coreRoutes.end ())
    {
        auto tmp = crfnd->second;
        lock.unlock ();
        tmp->addActionMessage (cmd);
        return;
    }

    if ((!isRoot ()) && (tbroker))
    {
        tbroker->addActionMessage (cmd);
    }
}

void TestBroker::addRoute (int route_id, const std::string &routeInfo)
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

std::string TestBroker::getAddress () const { return getIdentifier (); }

}  // namespace helics