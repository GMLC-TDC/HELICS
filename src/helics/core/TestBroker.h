/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef TEST_BROKER_H_
#define TEST_BROKER_H_
#pragma once

#include "core-broker.h"

namespace helics
{
class TestCore;

class TestBroker : public CoreBroker
{
public:
	TestBroker();

	virtual void transmit(int32_t route, const ActionMessage &command);

	virtual void addRoute(int route_id, const std::string &routeInfo);
private:
	std::shared_ptr<TestBroker> tbroker;  //the underlying broker;
										  //void computeDependencies();
	std::map<int32_t, std::shared_ptr<TestBroker>> brokerRoutes;
	std::map < int32_t, std::shared_ptr<TestCore>>  coreRoutes;
	mutable std::mutex routeMutex;
};
}
#endif
