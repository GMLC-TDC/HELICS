/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "TestBroker.h"
#include "TestCore.h"
#include "BrokerFactory.h"
#include "CoreFactory.h"

#include "argParser.h"
#include <fstream>

namespace helics
{

TestBroker::TestBroker(bool rootBroker) noexcept:CoreBroker(rootBroker)
{}

TestBroker::TestBroker(const std::string &broker_name) :CoreBroker(broker_name)
{}

TestBroker::TestBroker (std::shared_ptr<TestBroker> nbroker) : tbroker (std::move (nbroker)) {}

using namespace std::string_literals;
static const argDescriptors extraArgs
{
	{ "brokername"s, "string"s, "identifier for the broker-same as broker"s },
	{ "brokerinit"s, "string"s, "the initialization string for the broker"s }
};


void TestBroker::InitializeFromArgs (int argc, char *argv[])
{
    namespace po = boost::program_options;
    if (brokerState==broker_state_t::created)
    {
        po::variables_map vm;
        argumentParser (argc, argv, vm,extraArgs);

        if (vm.count ("broker") > 0)
        {
            brokerName = vm["broker"].as<std::string> ();
        }
		else if (vm.count("brokername") > 0)
		{
			brokerName = vm["brokername"].as<std::string>();
		}
        if (vm.count ("brokerinit") > 0)
        {
			brokerInitString=vm["brokerinit"].as<std::string> ();
        }
        CoreBroker::InitializeFromArgs (argc, argv);
       
		
    };
}


bool TestBroker::brokerConnect()
{
	if (!tbroker)
	{
		if (isRoot())
		{
			return true;
		}
		if ((brokerName.empty()) && (brokerInitString.empty()))
		{
			setAsRoot();
			return true;
		}
		else
		{
			tbroker = findBroker(brokerName);
			if (!tbroker)
			{
				tbroker = BrokerFactory::create(helics_core_type::HELICS_TEST, brokerName, brokerInitString);
			}
		}
		
	}

	return static_cast<bool>(tbroker);
}

void TestBroker::brokerDisconnect()
{
	
		tbroker = nullptr;
}

void TestBroker::transmit (int32_t route_id, const ActionMessage &cmd)
{
    if ((tbroker) && (route_id == 0))
    {
		
        tbroker->addCommand (cmd);
        return;
    }
    // only activate the lock if we not in an operating state
    auto lock = (_operating) ? std::unique_lock<std::mutex> (routeMutex, std::defer_lock) :
                               std::unique_lock<std::mutex> (routeMutex);

    auto brkfnd = brokerRoutes.find (route_id);
    if (brkfnd != brokerRoutes.end ())
    {
        brkfnd->second->addCommand (cmd);
        return;
    }
    auto crfnd = coreRoutes.find (route_id);
    if (crfnd != coreRoutes.end ())
    {
        crfnd->second->addCommand (cmd);
        return;
    }

    if (!isRoot())
    {
        tbroker->addCommand (cmd);
    }
}

void TestBroker::addRoute (int route_id, const std::string &routeInfo)
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


std::string TestBroker::getAddress() const
{
	return getIdentifier();
}

}  // namespace helics