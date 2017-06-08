/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "TestBroker.h"
#include "test-core.h"

namespace helics
{
TestBroker::TestBroker()
{

}

void TestBroker::transmit(int32_t route_id, const ActionMessage &cmd)
{
	if ((!_isRoot)&&(route_id == 0))
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

	if (!_isRoot)
	{
		tbroker->addMessage(cmd);
	}
	

}

void TestBroker::addRoute(int route_id, const std::string & routeInfo)
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


}// namespace helics