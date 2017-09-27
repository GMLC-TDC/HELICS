/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "BrokerFactory.h"
#include "helics/config.h"
#include "helics/core/core-types.h"

#if HELICS_HAVE_ZEROMQ
#include "helics/core/zmq/ZmqBroker.h"
#endif

#if HELICS_HAVE_MPI
#include "helics/core/mpi/mpiBroker.h"
#endif

#include "helics/core/ipc/IpcBroker.h"
#include "helics/core/TestBroker.h"

#include <cassert>

namespace helics
{

std::shared_ptr<CoreBroker> makeBroker(helics_core_type type, const std::string &name)
{
	std::shared_ptr<CoreBroker> broker;

	switch (type)
	{
	case HELICS_ZMQ:
	{
#if HELICS_HAVE_ZEROMQ
		if (name.empty())
		{
			broker = std::make_shared<ZmqBroker>();
		}
		else
		{
			broker = std::make_shared<ZmqBroker>(name);
		}

#else
		assert(false);
#endif
		break;
	}
	case HELICS_MPI:
	{
#if HELICS_HAVE_MPI
		if (name.empty())
		{
			broker = std::make_shared<MpiBroker>();
		}
		else
		{
			broker = std::make_shared<MpiBroker>(name);
		}
#else
		assert(false);
#endif
		break;
	}
	case HELICS_TEST:
	{
		if (name.empty())
		{
			broker = std::make_shared<TestBroker>();
		}
		else
		{
			broker = std::make_shared<TestBroker>(name);
		}
		break;
	}
	case HELICS_INTERPROCESS:
		if (name.empty())
		{
			broker = std::make_shared<IpcBroker>();
		}
		else
		{
			broker = std::make_shared<IpcBroker>(name);
		}
		break;
	default:
		assert(false);
	}
	return broker;
}

namespace BrokerFactory
{
	std::shared_ptr<CoreBroker> create(helics_core_type type, const std::string &initializationString)
	{
		auto broker = makeBroker(type, "");
		broker->Initialize(initializationString);
		registerBroker(broker);
		broker->connect();
		return broker;
	}

	std::shared_ptr<CoreBroker>
		create(helics_core_type type, const std::string &broker_name, std::string &initializationString)
	{
		auto broker = makeBroker(type, broker_name);
		broker->Initialize(initializationString);
		bool reg = registerBroker(broker);
		if (!reg)
		{

		}
		return broker;
	}

	std::shared_ptr<CoreBroker> create(helics_core_type type, int argc, char *argv[])
	{
		auto broker = makeBroker(type, "");
		broker->InitializeFromArgs(argc, argv);
		registerBroker(broker);
		broker->connect();
		return broker;
	}

	std::shared_ptr<CoreBroker>
		create(helics_core_type type, const std::string &broker_name, int argc, char *argv[])
	{
		auto broker = makeBroker(type, broker_name);
		broker->InitializeFromArgs(argc, argv);
		bool reg = registerBroker(broker);
		if (!reg)
		{

		}
		return broker;
	}


	bool available(helics_core_type type)
	{
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
			available = true;
			break;
		}
		default:
			assert(false);
		}

		return available;
	}

} //namespace BrokerFactory
static std::map<std::string, std::shared_ptr<CoreBroker>> BrokerMap;

static std::mutex mapLock;  //!<lock for the broker and core maps

/** so the problem this is addressing is that unregister can potentially cause a destructor to fire
that destructor can delete a thread variable, unfortunately it is possible that a thread stored in this variable
can do the unregister operation and destroy itself meaning it is unable to join and thus will call std::terminate
what we do is delay the destruction until it is called in a different thread which allows the destructor to fire if
need be without issue*/
std::vector<std::shared_ptr<CoreBroker>> delayedDestruction;

std::shared_ptr<CoreBroker> findBroker (const std::string &brokerName)
{
    std::lock_guard<std::mutex> lock (mapLock);
    auto fnd = BrokerMap.find (brokerName);
    if (fnd != BrokerMap.end ())
    {
        return fnd->second;
    }
    return nullptr;
}

bool registerBroker (std::shared_ptr<CoreBroker> tbroker)
{
    std::lock_guard<std::mutex> lock (mapLock);
    auto res = BrokerMap.emplace (tbroker->getIdentifier (), std::move (tbroker));

    /** clear out any previously unregistered brokers*/
    if (!delayedDestruction.empty ())
    {
        delayedDestruction.clear ();
    }
    return res.second;
}

void cleanUpBrokers ()
{
    std::lock_guard<std::mutex> lock (mapLock);
    delayedDestruction.clear ();
}

void copyBrokerIdentifier(const std::string &copyFromName, const std::string &copyToName)
{
	std::lock_guard<std::mutex> lock(mapLock);
	auto fnd = BrokerMap.find(copyFromName);
	if (fnd != BrokerMap.end())
	{
		auto newBrokerPtr = fnd->second;
		BrokerMap.emplace(copyToName, std::move(newBrokerPtr));
	}
}

void unregisterBroker (const std::string &name)
{
    std::lock_guard<std::mutex> lock (mapLock);
    auto fnd = BrokerMap.find (name);
    if (fnd != BrokerMap.end ())
    {
        delayedDestruction.push_back (std::move (fnd->second));
        BrokerMap.erase (fnd);
        return;
    }
    for (auto brk = BrokerMap.begin (); brk != BrokerMap.end (); ++brk)
    {
        if (brk->second->getIdentifier () == name)
        {
            delayedDestruction.push_back (std::move (brk->second));
            BrokerMap.erase (brk);
            return;
        }
    }
}
}  // namespace
