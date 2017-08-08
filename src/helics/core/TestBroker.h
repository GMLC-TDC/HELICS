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
/** class implementing a basic broker that links to other brokers in process memory*/
class TestBroker : public CoreBroker
{
public:
	/** default constructor*/
	TestBroker() = default;

	virtual void transmit(int32_t route, const ActionMessage &command) override;

	virtual void addRoute(int route_id, const std::string &routeInfo) override;
private:
	std::shared_ptr<TestBroker> tbroker;  //the underlying broker;
										  //void computeDependencies();
	std::map<int32_t, std::shared_ptr<TestBroker>> brokerRoutes; //!< map of the routes to other brokers
	std::map < int32_t, std::shared_ptr<TestCore>>  coreRoutes; //!< map of the routes to other cores
	mutable std::mutex routeMutex;
};
}
#endif
