/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "BrokerFactory.hpp"
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
        type = core_type::UDP;
#endif
    }

    switch (type)
    {
    case core_type::ZMQ:
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
        throw (HelicsException ("ZMQ broker type is not available"));
#endif
        break;
    case core_type::MPI:
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
        throw (HelicsException ("mpi broker type is not available"));
#endif
        break;
    case core_type::TEST:
        if (name.empty ())
        {
            broker = std::make_shared<TestBroker> ();
        }
        else
        {
            broker = std::make_shared<TestBroker> (name);
        }
        break;
    case core_type::INTERPROCESS:
    case core_type::IPC:
        if (name.empty ())
        {
            broker = std::make_shared<IpcBroker> ();
        }
        else
        {
            broker = std::make_shared<IpcBroker> (name);
        }
        break;
    case core_type::UDP:
        if (name.empty ())
        {
            broker = std::make_shared<UdpBroker> ();
        }
        else
        {
            broker = std::make_shared<UdpBroker> (name);
        }
        break;
    case core_type::TCP:
#ifndef DISABLE_TCP_CORE
        if (name.empty ())
        {
            broker = std::make_shared<TcpBroker> ();
        }
        else
        {
            broker = std::make_shared<TcpBroker> (name);
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
    auto broker = makeBroker (type, "");
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
    return broker;
}

bool available (core_type type)
{
    bool available = false;

    switch (type)
    {
    case core_type::ZMQ:
#if HELICS_HAVE_ZEROMQ
        available = true;
#endif
        break;
    case core_type::MPI:
#if HELICS_HAVE_MPI
        available = true;
#endif
        break;
    case core_type::TEST:
        available = true;
        break;
    case core_type::INTERPROCESS:
    case core_type::IPC:
        available = true;
        break;
    case core_type::TCP:
        available = true;
        break;
    case core_type::UDP:
        available = true;
        break;
    default:
        break;
    }

    return available;
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

std::shared_ptr<Broker> findBroker (const std::string &brokerName)
{
    return searchableObjects.findObject (brokerName);
}

bool registerBroker (std::shared_ptr<Broker> broker)
{
    bool res = false;
    auto tbroker = std::dynamic_pointer_cast<CoreBroker> (std::move (broker));
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
        ZmqBroker::displayHelp (true);
#endif
        break;
    case core_type::MPI:
#if HELICS_HAVE_MPI
        MpiBroker::displayHelp (true);
#endif
        break;
    case core_type::TEST:
        TestBroker::displayHelp (true);
        break;
    case core_type::INTERPROCESS:
    case core_type::IPC:
        IpcBroker::displayHelp (true);
        break;
    case core_type::TCP:
        break;
    case core_type::UDP:
        UdpBroker::displayHelp (true);
        break;
    default:
#if HELICS_HAVE_ZEROMQ
        ZmqBroker::displayHelp (true);
#endif
#if HELICS_HAVE_MPI
        MpiBroker::displayHelp (true);
#endif
        IpcBroker::displayHelp (true);

        TestBroker::displayHelp (true);
        UdpBroker::displayHelp (true);
        break;
    }

    CoreBroker::displayHelp ();
}

}  // namespace BrokerFactory
}  // namespace helics
