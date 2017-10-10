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

void TestCore::InitializeFromArgs (int argc, char *argv[])
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
        CommonCore::InitializeFromArgs (argc, argv);
    }
}

bool TestCore::brokerConnect ()
{
    std::lock_guard<std::mutex> lock (routeMutex);
    if (!tbroker)
    {
        tbroker = findBroker (brokerName);
        if (!tbroker)
        {
            tbroker = BrokerFactory::create (core_type::TEST, brokerName, brokerInitString);
        }
    }
    if (tbroker)
    {
        tbroker->connect ();
    }
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
    // lock to ensure all the data is synchronized before deletion
    std::lock_guard<std::mutex> lock (routeMutex);
}

void TestCore::transmit (int route_id, const ActionMessage &cmd)
{
    auto lock = (brokerState == operating) ? std::unique_lock<std::mutex> (routeMutex, std::defer_lock) :
                                           std::unique_lock<std::mutex> (routeMutex);
    if (route_id == 0)
    {
        tbroker->addActionMessage (cmd);
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

    tbroker->addActionMessage (cmd);
}

void TestCore::addRoute (int route_id, const std::string &routeInfo)
{
    auto brk = findBroker (routeInfo);
    if (brk)
    {
        std::lock_guard<std::mutex> lock (routeMutex);
        brokerRoutes.emplace (route_id, std::move (brk));
        return;
    }
    auto tcore = CoreFactory::findCore (routeInfo);
    if (tcore)
    {
        std::lock_guard<std::mutex> lock (routeMutex);
        coreRoutes.emplace (route_id, std::move (tcore));
        return;
    }
    // the route will default to the central route
}

std::string TestCore::getAddress () const { return getIdentifier (); }

/*
void TestCore::computeDependencies()
{
    for (auto &fed : _federates)
    {
        fed->generateKnownDependencies();
    }
    //TODO:: work in the additional rules for endpoints to reduce dependencies
    for (auto &fed : _federates)
    {
        if (fed->hasEndpoints)
        {
            for (auto &fedD : _federates)
            {
                if (fedD->hasEndpoints)
                {
                    fed->addDependency(fedD->id);
                    fedD->addDependent(fed->id);
                }
            }
        }
    }
}

*/

}  // namespace helics
