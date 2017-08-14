/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/config.h"
#include "BrokerFactory.h"
#include "helics/core/core-types.h"

#if HELICS_HAVE_ZEROMQ
#include "helics/core/zmq/ZmqBroker.h"
#endif

#if HELICS_HAVE_MPI
#include "helics/core/mpi/mpiBroker.h"
#endif

#include "helics/core/TestBroker.h"

#include <cassert>

namespace helics {

std::shared_ptr<CoreBroker> BrokerFactory::create(helics_core_type type, const std::string &initializationString) {

	std::shared_ptr<CoreBroker> broker;

	switch (type)
	{
	case HELICS_ZMQ:
	{
#if HELICS_HAVE_ZEROMQ
		broker = std::make_shared<ZmqBroker>();
		
#else
		assert(false);
#endif
		break;
	}
	case HELICS_MPI:
	{
#if HELICS_HAVE_MPI
		broker = std::make_shared<MpiBroker>();
#else
		assert(false);
#endif
		break;
	}
	case HELICS_TEST:
	{
		broker = std::make_shared<TestBroker>();
		break;
	}
	case HELICS_INTERPROCESS:
		break;
	default:
		assert(false);
	}
	broker->Initialize(initializationString);
	registerBroker(broker);
	broker->connect();
	return broker;
}

std::shared_ptr<CoreBroker> BrokerFactory::create(helics_core_type type, const std::string &broker_name, std::string &initializationString) {

	std::shared_ptr<CoreBroker> broker;

	switch (type)
	{
	case HELICS_ZMQ:
	{
#if HELICS_HAVE_ZEROMQ
		broker = std::make_shared<ZmqBroker>(broker_name);

#else
		assert(false);
#endif
		break;
	}
	case HELICS_MPI:
	{
#if HELICS_HAVE_MPI
		broker = std::make_shared<MpiBroker>(broker_name);
#else
		assert(false);
#endif
		break;
	}
	case HELICS_TEST:
	{
		broker = std::make_shared<TestBroker>(broker_name);
		break;
	}
	case HELICS_INTERPROCESS:
		break;
	default:
		assert(false);
	}
	broker->Initialize(initializationString);
	registerBroker(broker);
	return broker;
}


bool BrokerFactory::available(helics_core_type type) {

	bool available = false;

	switch (type)
	{
	case HELICS_ZMQ:
	{
#if HELICS_HAVE_ZEROMQ
		available = true;
#endif
		break;
	}
	case HELICS_MPI:
	{
#if HELICS_HAVE_MPI
		available = true;
#endif
		break;
	}
	case HELICS_TEST:
	{
		available = true;
		break;
	}
	case HELICS_INTERPROCESS:
	{
		available = false;
		break;
	}
	default:
		assert(false);
	}

	return available;
}


static std::map<std::string, std::shared_ptr<CoreBroker>> BrokerMap;

static std::mutex mapLock;  //!<lock for the broker and core maps


std::shared_ptr<CoreBroker> findBroker(const std::string &brokerName)
{
	std::lock_guard<std::mutex> lock(mapLock);
	auto fnd = BrokerMap.find(brokerName);
	if (fnd != BrokerMap.end())
	{
		return fnd->second;
	}
	return nullptr;
}

bool registerBroker(std::shared_ptr<CoreBroker> tbroker)
{
	std::lock_guard<std::mutex> lock(mapLock);
	auto res = BrokerMap.emplace(tbroker->getIdentifier(), std::move(tbroker));
	return res.second;
}

void unregisterBroker(const std::string &name)
{
	std::lock_guard<std::mutex> lock(mapLock);
	auto fnd = BrokerMap.find(name);
	if (fnd != BrokerMap.end())
	{
		BrokerMap.erase(fnd);
		return;
	}
	for (auto brk=BrokerMap.begin();brk!=BrokerMap.end();++brk)
	{
		if (brk->second->getIdentifier() == name)
		{
			BrokerMap.erase(brk);
			return;
		}
	}
}
} // namespace 

