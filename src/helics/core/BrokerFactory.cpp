/*

Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "BrokerFactory.hpp"
#include "../common/TripWire.hpp"
#include "../common/delayedDestructor.hpp"
#include "../common/searchableObjectHolder.hpp"
#include "core-exceptions.hpp"
#include "core-types.hpp"
#include "helics/helics-config.h"
#if HELICS_HAVE_ZEROMQ
#include "zmq/ZmqBroker.h"
#endif

#if HELICS_HAVE_MPI
#include "mpi/MpiBroker.h"
#endif

#include "TestBroker.h"
#include "ipc/IpcBroker.h"
#include "udp/UdpBroker.h"

#ifndef DISABLE_TCP_CORE
#include "tcp/TcpBroker.h"
#endif

#include <cassert>

namespace helics
{
std::shared_ptr<Broker> makeBroker (core_type type, const std::string &name)
{
    std::shared_ptr<Broker> broker;

    if (type == core_type::DEFAULT)
    {
#if HELICS_HAVE_ZEROMQ
        type = core_type::ZMQ;
#else
#ifndef DISABLE_TCP_CORE
        type = core_type::TCP;
#else
        type = core_type::UDP;
#endif
#endif
    }

    switch (type)
    {
    case core_type::ZMQ:
#if HELICS_HAVE_ZEROMQ
        if (name.empty ())
        {
            broker = std::make_shared<zeromq::ZmqBroker> ();
        }
        else
        {
            broker = std::make_shared<zeromq::ZmqBroker> (name);
        }

#else
        throw (HelicsException ("ZMQ broker type is not available"));
#endif
        break;
    case core_type::MPI:
#if HELICS_HAVE_MPI
        if (name.empty ())
        {
            broker = std::make_shared<mpi::MpiBroker> ();
        }
        else
        {
            broker = std::make_shared<mpi::MpiBroker> (name);
        }
#else
        throw (HelicsException ("mpi broker type is not available"));
#endif
        break;
    case core_type::TEST:
        if (name.empty ())
        {
            broker = std::make_shared<testcore::TestBroker> ();
        }
        else
        {
            broker = std::make_shared<testcore::TestBroker> (name);
        }
        break;
    case core_type::INTERPROCESS:
    case core_type::IPC:
        if (name.empty ())
        {
            broker = std::make_shared<ipc::IpcBroker> ();
        }
        else
        {
            broker = std::make_shared<ipc::IpcBroker> (name);
        }
        break;
    case core_type::UDP:
        if (name.empty ())
        {
            broker = std::make_shared<udp::UdpBroker> ();
        }
        else
        {
            broker = std::make_shared<udp::UdpBroker> (name);
        }
        break;
    case core_type::TCP:
#ifndef DISABLE_TCP_CORE
        if (name.empty ())
        {
            broker = std::make_shared<tcp::TcpBroker> ();
        }
        else
        {
            broker = std::make_shared<tcp::TcpBroker> (name);
        }
#else
        throw (HelicsException ("tcp broker type is not available"));
#endif
        break;
    default:
        throw (HelicsException ("unrecognized broker type"));
    }
    return broker;
}

namespace BrokerFactory
{
std::shared_ptr<Broker> create (core_type type, const std::string &initializationString)
{
    auto broker = makeBroker (type, std::string ());
    broker->initialize (initializationString);
    registerBroker (broker);
    broker->connect ();
    return broker;
}

std::shared_ptr<Broker>
create (core_type type, const std::string &broker_name, const std::string &initializationString)
{
    auto broker = makeBroker (type, broker_name);
    broker->initialize (initializationString);
    bool reg = registerBroker (broker);
    if (!reg)
    {
        // TODO:: do some automatic renaming?
    }
    broker->connect ();
    return broker;
}

std::shared_ptr<Broker> create (core_type type, int argc, const char *const *argv)
{
    auto broker = makeBroker (type, "");
    broker->initializeFromArgs (argc, argv);
    registerBroker (broker);
    broker->connect ();
    return broker;
}

std::shared_ptr<Broker> create (core_type type, const std::string &broker_name, int argc, const char *const *argv)
{
    auto broker = makeBroker (type, broker_name);
    broker->initializeFromArgs (argc, argv);
    bool reg = registerBroker (broker);
    if (!reg)
    {
    }
    broker->connect ();
    return broker;
}

/** lambda function to join cores before the destruction happens to avoid potential problematic calls in the
 * loops*/
static auto destroyerCallFirst = [](auto &broker) {
    broker->processDisconnect (
      true);  // use true here as it is possible the searchableObjectHolder is deleted already
    broker->joinAllThreads ();
};
/** so the problem this is addressing is that unregister can potentially cause a destructor to fire
that destructor can delete a thread variable, unfortunately it is possible that a thread stored in this variable
can do the unregister operation and destroy itself meaning it is unable to join and thus will call std::terminate
what we do is delay the destruction until it is called in a different thread which allows the destructor to fire if
need be without issue*/

static DelayedDestructor<CoreBroker>
  delayedDestroyer (destroyerCallFirst);  //!< the object handling the delayed destruction

static SearchableObjectHolder<CoreBroker> searchableObjects;  //!< the object managing the searchable objects

// this will trip the line when it is destroyed at global destruction time
static tripwire::TripWireTrigger tripTrigger;

std::shared_ptr<Broker> findBroker (const std::string &brokerName)
{
    return searchableObjects.findObject (brokerName);
}

bool registerBroker (const std::shared_ptr<Broker> &broker)
{
    bool res = false;
    auto tbroker = std::dynamic_pointer_cast<CoreBroker> (broker);
    if (tbroker)
    {
        res = searchableObjects.addObject (tbroker->getIdentifier (), tbroker);
    }
    cleanUpBrokers ();
    if (res)
    {
        delayedDestroyer.addObjectsToBeDestroyed (tbroker);
    }

    return res;
}

size_t cleanUpBrokers () { return delayedDestroyer.destroyObjects (); }
size_t cleanUpBrokers (int delay) { return delayedDestroyer.destroyObjects (delay); }

void copyBrokerIdentifier (const std::string &copyFromName, const std::string &copyToName)
{
    searchableObjects.copyObject (copyFromName, copyToName);
}

void unregisterBroker (const std::string &name)
{
    if (!searchableObjects.removeObject (name))
    {
        searchableObjects.removeObject ([&name](auto &obj) { return (obj->getIdentifier () == name); });
    }
}

void displayHelp (core_type type)
{
    switch (type)
    {
    case core_type::ZMQ:
#if HELICS_HAVE_ZEROMQ
        zeromq::ZmqBroker::displayHelp (true);
#endif
        break;
    case core_type::MPI:
#if HELICS_HAVE_MPI
        mpi::MpiBroker::displayHelp (true);
#endif
        break;
    case core_type::TEST:
        testcore::TestBroker::displayHelp (true);
        break;
    case core_type::INTERPROCESS:
    case core_type::IPC:
        ipc::IpcBroker::displayHelp (true);
        break;
    case core_type::TCP:
#ifndef DISABLE_TCP_CORE
        tcp::TcpBroker::displayHelp (true);
#endif
        break;
    case core_type::UDP:
        udp::UdpBroker::displayHelp (true);
        break;
    default:
#if HELICS_HAVE_ZEROMQ
        zeromq::ZmqBroker::displayHelp (true);
#endif
#if HELICS_HAVE_MPI
        mpi::MpiBroker::displayHelp (true);
#endif
        ipc::IpcBroker::displayHelp (true);

        testcore::TestBroker::displayHelp (true);
#ifndef DISABLE_TCP_CORE
        tcp::TcpBroker::displayHelp (true);
#endif
        udp::UdpBroker::displayHelp (true);
        break;
    }

    CoreBroker::displayHelp ();
}

}  // namespace BrokerFactory
}  // namespace helics
