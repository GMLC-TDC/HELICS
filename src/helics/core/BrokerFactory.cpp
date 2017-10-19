/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "BrokerFactory.h"
#include "helics/common/delayedDestructor.hpp"
#include "helics/common/searchableObjectHolder.hpp"
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
        // TODO:: do some automatic renaming?
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

/** so the problem this is addressing is that unregister can potentially cause a destructor to fire
that destructor can delete a thread variable, unfortunately it is possible that a thread stored in this variable
can do the unregister operation and destroy itself meaning it is unable to join and thus will call std::terminate
what we do is delay the destruction until it is called in a different thread which allows the destructor to fire if
need be without issue*/

static DelayedDestructor<CoreBroker> delayedDestroyer;  //!< the object handling the delayed destruction

static SearchableObjectHolder<CoreBroker> searchableObjects; //!< the object managing the searchable objects

std::shared_ptr<CoreBroker> findBroker (const std::string &brokerName)
{
	return searchableObjects.findObject(brokerName);
}

bool registerBroker (std::shared_ptr<CoreBroker> tbroker)
{
    cleanUpBrokers ();
    delayedDestroyer.addObjectsToBeDestroyed (tbroker);
	return searchableObjects.addObject(tbroker->getIdentifier(),tbroker);
}

size_t cleanUpBrokers () { return delayedDestroyer.destroyObjects (); }

void copyBrokerIdentifier (const std::string &copyFromName, const std::string &copyToName)
{
	searchableObjects.copyObject(copyFromName, copyToName);
}

void unregisterBroker (const std::string &name)
{
	if (!searchableObjects.removeObject(name))
	{
		searchableObjects.removeObject([&name](auto &obj) {return (obj->getIdentifier() == name); });
	}
	
}

}  // namespace BrokerFactory
}  // namespace helics
