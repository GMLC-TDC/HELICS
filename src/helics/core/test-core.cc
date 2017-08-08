/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "test-core.h"
#include "TestBroker.h"

#include "ActionMessage.h"

namespace helics
{
using federate_id_t= Core::federate_id_t;
using Handle= Core::Handle;


TestCore::TestCore(std::shared_ptr<TestBroker> nbroker) : tbroker(std::move(nbroker))
{

}

void TestCore::initialize (const std::string &initializationString)
{
	CommonCore::initialize(initializationString);
	if (!tbroker)
	{
		tbroker = std::make_shared<TestBroker>();
		tbroker->Initialize(initializationString);
	}
	
}


TestCore::~TestCore() = default;

void TestCore::transmit(int route_id, const ActionMessage &cmd)
{
	if (route_id==0)
	{
		tbroker->addMessage(cmd);
		return;
	}
	if (_operating)
	{
		auto brkfnd = brokerRoutes.find(route_id);
		if (brkfnd != brokerRoutes.end())
		{
			brkfnd->second->addMessage(cmd);
			return;
		}
		auto crfnd = coreRoutes.find(route_id);
		if (crfnd != coreRoutes.end())
		{
			crfnd->second->addCommand(cmd);
			return;
		}
	}

	tbroker->addMessage(cmd);
	
}

void TestCore::addRoute(int route_id, const std::string & routeInfo)
{

	auto brk = findTestBroker(routeInfo);
	if (brk)
	{
		std::lock_guard<std::mutex> lock(routeMutex);
		brokerRoutes.emplace(route_id, std::move(brk));
		return;
	}
	auto tcore = findTestCore(routeInfo);
	if (tcore)
	{
		std::lock_guard<std::mutex> lock(routeMutex);
		coreRoutes.emplace(route_id, std::move(tcore));
		return;
	}
	//the route will default to the central route
}


static std::map<std::string, std::weak_ptr<TestCore>> TestCoreMap;

static std::map<std::string, std::weak_ptr<TestBroker>> TestBrokerMap;

static std::mutex mapLock;  //!<lock for the broker and core maps

std::shared_ptr<TestCore> findTestCore(const std::string &name)
{
	std::lock_guard<std::mutex> lock(mapLock);
	auto fnd = TestCoreMap.find(name);
	if (fnd != TestCoreMap.end())
	{
		return fnd->second.lock();
	}
	return nullptr;
}

std::shared_ptr<TestBroker> findTestBroker(const std::string &brokerName)
{
	std::lock_guard<std::mutex> lock(mapLock);
	auto fnd = TestBrokerMap.find(brokerName);
	if (fnd != TestBrokerMap.end())
	{
		return fnd->second.lock();
	}
	return nullptr;
}

void registerTestCore(std::shared_ptr<TestCore> tcore)
{
	std::lock_guard<std::mutex> lock(mapLock);
	TestCoreMap[tcore->getIdentifier()] = tcore;
}

void registerTestBroker(std::shared_ptr<TestBroker> tbroker)
{
	std::lock_guard<std::mutex> lock(mapLock);
	TestBrokerMap[tbroker->getIdentifier()] = tbroker;
}


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
