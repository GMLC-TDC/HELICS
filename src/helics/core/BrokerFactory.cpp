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

#include "helics/core/TestBroker.h"
#include "helics/core/ipc/IpcBroker.h"

#include <cassert>

namespace helics
{
std::shared_ptr<CoreBroker> makeBroker (core_type type, const std::string &name)
{
    std::shared_ptr<CoreBroker> broker;

    switch (type)
    {
    case core_type::ZMQ:
    {
#if HELICS_HAVE_ZEROMQ
        if (name.empty ())
        {
            broker = std::make_shared<ZmqBroker> ();
        }
        else
        {
            broker = std::make_shared<ZmqBroker> (name);
        }

#else
        assert (false);
#endif
        break;
    }
    case core_type::MPI:
    {
#if HELICS_HAVE_MPI
        if (name.empty ())
        {
            broker = std::make_shared<MpiBroker> ();
        }
        else
        {
            broker = std::make_shared<MpiBroker> (name);
        }
#else
        assert (false);
#endif
        break;
    }
    case core_type::TEST:
    {
        if (name.empty ())
        {
            broker = std::make_shared<TestBroker> ();
        }
        else
        {
            broker = std::make_shared<TestBroker> (name);
        }
        break;
    }
    case core_type::INTERPROCESS:
        if (name.empty ())
        {
            broker = std::make_shared<IpcBroker> ();
        }
        else
        {
            broker = std::make_shared<IpcBroker> (name);
        }
        break;
    default:
        assert (false);
    }
    return broker;
}

namespace BrokerFactory
{
std::shared_ptr<CoreBroker> create (core_type type, const std::string &initializationString)
{
    auto broker = makeBroker (type, "");
    broker->Initialize (initializationString);
    registerBroker (broker);
    broker->connect ();
    return broker;
}

std::shared_ptr<CoreBroker>
create (core_type type, const std::string &broker_name, const std::string &initializationString)
{
    auto broker = makeBroker (type, broker_name);
    broker->Initialize (initializationString);
    bool reg = registerBroker (broker);
    if (!reg)
    {
		//TODO:: do some automatic renaming?  
    }
    broker->connect ();
    return broker;
}

std::shared_ptr<CoreBroker> create (core_type type, int argc, char *argv[])
{
    auto broker = makeBroker (type, "");
    broker->InitializeFromArgs (argc, argv);
    registerBroker (broker);
    broker->connect ();
    return broker;
}

std::shared_ptr<CoreBroker> create (core_type type, const std::string &broker_name, int argc, char *argv[])
{
    auto broker = makeBroker (type, broker_name);
    broker->InitializeFromArgs (argc, argv);
    bool reg = registerBroker (broker);
    if (!reg)
    {
    }
    return broker;
}

bool available (core_type type)
{
    bool available = false;

    switch (type)
    {
    case core_type::ZMQ:
    {
#if HELICS_HAVE_ZEROMQ
        available = true;
#endif
        break;
    }
    case core_type::MPI:
    {
#if HELICS_HAVE_MPI
        available = true;
#endif
        break;
    }
    case core_type::TEST:
    {
        available = true;
        break;
    }
    case core_type::INTERPROCESS:
    case core_type::IPC:
    {
        available = true;
        break;
    }
    case core_type::TCP:
    case core_type::UDP:
        available = false;
        break;
    default:
        assert (false);
    }

    return available;
}

}  // namespace BrokerFactory
static std::map<std::string, std::shared_ptr<CoreBroker>> BrokerMap;

static std::mutex mapLock;  //!< lock for the broker and core maps

/** so the problem this is addressing is that unregister can potentially cause a destructor to fire
that destructor can delete a thread variable, unfortunately it is possible that a thread stored in this variable
can do the unregister operation and destroy itself meaning it is unable to join and thus will call std::terminate
what we do is delay the destruction until it is called in a different thread which allows the destructor to fire if
need be without issue*/

std::vector<std::shared_ptr<CoreBroker>> delayedDestruction;
//TODO:: this needs to be wrapped in a thread safe class to ensure it gets cleaned up properly on destruction and prevent thread sanitizer triggers

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
	cleanUpBrokers();
    std::lock_guard<std::mutex> lock (mapLock);
    auto res = BrokerMap.emplace (tbroker->getIdentifier (), std::move (tbroker));
    return res.second;
}

void cleanUpBrokers ()
{
    std::unique_lock<std::mutex> lock (mapLock);
	if (!delayedDestruction.empty())
	{
		auto tempbuffer = std::move(delayedDestruction);
		lock.unlock();
		tempbuffer.clear();
		//we don't want to actually do the destruction with the lock engaged since that could be a lengthy operation so we use a temporary buffer
	}
}

void copyBrokerIdentifier (const std::string &copyFromName, const std::string &copyToName)
{
    std::lock_guard<std::mutex> lock (mapLock);
    auto fnd = BrokerMap.find (copyFromName);
    if (fnd != BrokerMap.end ())
    {
        auto newBrokerPtr = fnd->second;
        BrokerMap.emplace (copyToName, std::move (newBrokerPtr));
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
}  // namespace helics
