/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "TestBroker.h"
#include "BrokerFactory.hpp"
#include "CoreFactory.hpp"
#include "TestCore.h"

#include "../common/argParser.h"
#include <fstream>

namespace helics
{
namespace testcore
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
static const ArgDescriptors extraArgs{{"brokername"s, "identifier for the broker-same as broker"s},
                                      {"broker,b"s, "identifier for the broker"s},
                                      {"autobroker"s, ArgDescriptor::arg_type_t::flag_type,
                                       "automatically generate a broker"},
                                      {"broker_address", "location of the broker i.e network address"},
                                      {"brokerinit"s, "the initialization string for the broker"s}};

void TestBroker::displayHelp (bool localOnly)
{
    std::cout << " Help for Test Broker: \n";
    variable_map vm;
    const char *const argV[] = {"", "--help"};
    argumentParser (2, argV, vm, extraArgs);
    if (!localOnly)
    {
        CoreBroker::displayHelp ();
    }
}

void TestBroker::initializeFromArgs (int argc, const char *const *argv)
{
    if (brokerState == broker_state_t::created)
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
		if (vm.count("autobroker") > 0)
		{
            autoBroker = true;
		}
        CoreBroker::initializeFromArgs (argc, argv);
    };
}

bool TestBroker::brokerConnect ()
{
    std::lock_guard<std::mutex> lock (routeMutex);
    while (!tbroker)
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
        std::chrono::milliseconds totalSleep (0);
        auto broker = BrokerFactory::findBroker (brokerName);
        tbroker = std::dynamic_pointer_cast<CoreBroker> (broker);
        if (!tbroker)
        {
            if (autoBroker)
            {
                tbroker = std::static_pointer_cast<CoreBroker> (
                  BrokerFactory::create (core_type::TEST, brokerName, brokerInitString));
                tbroker->connect ();
            }
            else
            {
                if (totalSleep > std::chrono::milliseconds (timeout))
                {
                    return false;
                }
                std::this_thread::sleep_for (std::chrono::milliseconds (200));
                totalSleep += std::chrono::milliseconds (200);
            }
        }
        else
        {
            if (!tbroker->isOpenToNewFederates ())
            {
                std::cerr << "broker is not open to new federates " << brokerName << std::endl;
                tbroker = nullptr;
                broker = nullptr;
                BrokerFactory::cleanUpBrokers (std::chrono::milliseconds(200));
                totalSleep += std::chrono::milliseconds (200);
                if (totalSleep > std::chrono::milliseconds (timeout))
                {
                    return false;
                }
            }
        }
    }

    return static_cast<bool> (tbroker);
}

bool TestBroker::tryReconnect ()
{
    if (isRoot ())
    {
        return true;
    }
    auto broker = BrokerFactory::findBroker (brokerName);
    std::lock_guard<std::mutex> lock(routeMutex);
    tbroker = std::dynamic_pointer_cast<CoreBroker> (broker);
    return static_cast<bool> (tbroker);
}

void TestBroker::brokerDisconnect ()
{
    brokerState = broker_state_t::terminating;
    std::lock_guard<std::mutex> lock (routeMutex);
    tbroker = nullptr;
    brokerRoutes.clear ();
    coreRoutes.clear ();
}

void TestBroker::transmit (route_id_t route_id, const ActionMessage &cmd)
{
    if (brokerState >= broker_state_t::terminating)
    {
        return;  // no message sent in terminating or higher state
    }
    std::unique_lock<std::mutex> lock (routeMutex);

    if ((tbroker) && (route_id == parent_route_id))
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

void TestBroker::transmit (route_id_t route_id, ActionMessage &&cmd)
{
    if (brokerState >= broker_state_t::terminating)
    {
        return;  // no message sent in terminating or higher state
    }
    std::unique_lock<std::mutex> lock (routeMutex);

    if ((tbroker) && (route_id == parent_route_id))
    {
        tbroker->addActionMessage (std::move(cmd));
        return;
    }

    auto brkfnd = brokerRoutes.find (route_id);
    if (brkfnd != brokerRoutes.end ())
    {
        auto tmp = brkfnd->second;
        lock.unlock ();
        tmp->addActionMessage (std::move(cmd));
        return;
    }
    auto crfnd = coreRoutes.find (route_id);
    if (crfnd != coreRoutes.end ())
    {
        auto tmp = crfnd->second;
        lock.unlock ();
        tmp->addActionMessage (std::move(cmd));
        return;
    }

    if ((!isRoot ()) && (tbroker))
    {
        tbroker->addActionMessage (std::move(cmd));
    }
}

void TestBroker::addRoute (route_id_t route_id, const std::string &routeInfo)
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

std::string TestBroker::generateLocalAddressString () const { return getIdentifier (); }

}  // namespace testcore
}  // namespace helics
