/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "CoreFactory.hpp"
#include "core-exceptions.hpp"
#include "core-types.hpp"
#include "helics/helics-config.h"
#if HELICS_HAVE_ZEROMQ
#include "zmq/ZmqCore.h"
#endif

#if HELICS_HAVE_MPI
#include "mpi/MpiCore.h"
#endif

#include "../common/delayedDestructor.hpp"
#include "../common/searchableObjectHolder.hpp"
#include "TestCore.h"
#include "ipc/IpcCore.h"
#include "udp/UdpCore.h"

#ifndef DISABLE_TCP_CORE
#include "tcp/TcpCore.h"
#endif

#include <cassert>

namespace helics
{
std::shared_ptr<Core> makeCore (core_type type, const std::string &name)
{
    std::shared_ptr<Core> core;
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
            core = std::make_shared<ZmqCore> ();
        }
        else
        {
            core = std::make_shared<ZmqCore> (name);
        }

#else
        throw (HelicsException ("ZMQ core is not available"));
#endif
        break;
    case core_type::MPI:
#if HELICS_HAVE_MPI
        if (name.empty ())
        {
            core = std::make_shared<MpiCore> ();
        }
        else
        {
            core = std::make_shared<MpiCore> (name);
        }
#else
        throw (HelicsException ("MPI core is not available"));
#endif
        break;
    case core_type::TEST:
        if (name.empty ())
        {
            core = std::make_shared<TestCore> ();
        }
        else
        {
            core = std::make_shared<TestCore> (name);
        }
        break;
    case core_type::INTERPROCESS:
    case core_type::IPC:
        if (name.empty ())
        {
            core = std::make_shared<IpcCore> ();
        }
        else
        {
            core = std::make_shared<IpcCore> (name);
        }
        break;
    case core_type::UDP:
        if (name.empty ())
        {
            core = std::make_shared<UdpCore> ();
        }
        else
        {
            core = std::make_shared<UdpCore> (name);
        }
        break;
    case core_type::TCP:
#ifndef DISABLE_TCP_CORE
        if (name.empty ())
        {
            core = std::make_shared<TcpCore> ();
        }
        else
        {
            core = std::make_shared<TcpCore> (name);
        }
#else
        throw (HelicsException ("TCP core is not available"));
#endif
        break;
    default:
        throw (HelicsException ("unrecognized core type"));
    }
    return core;
}

namespace CoreFactory
{
std::shared_ptr<Core> create (core_type type, const std::string &initializationString)
{
    auto core = makeCore (type, "");
    core->initialize (initializationString);
    registerCore (core);

    return core;
}

std::shared_ptr<Core> create (core_type type, const std::string &core_name, std::string &initializationString)
{
    auto core = makeCore (type, core_name);
    core->initialize (initializationString);
    registerCore (core);

    return core;
}

std::shared_ptr<Core> create (core_type type, int argc, const char *const *argv)
{
    auto core = makeCore (type, "");
    core->initializeFromArgs (argc, argv);
    registerCore (core);
    return core;
}

std::shared_ptr<Core> create (core_type type, const std::string &core_name, int argc, const char *const *argv)
{
    auto core = makeCore (type, core_name);
    core->initializeFromArgs (argc, argv);
    registerCore (core);

    return core;
}

std::shared_ptr<Core>
FindOrCreate (core_type type, const std::string &core_name, const std::string &initializationString)
{
    std::shared_ptr<Core> core = findCore (core_name);
    if (core)
    {
        return core;
    }
    core = makeCore (type, core_name);
    core->initialize (initializationString);

    bool success = registerCore (core);
    if (!success)
    {
        core = findCore (core_name);
        if (core)
        {
            return core;
        }
    }

    return core;
}

std::shared_ptr<Core>
FindOrCreate (core_type type, const std::string &core_name, int argc, const char *const *argv)
{
    std::shared_ptr<Core> core = findCore (core_name);
    if (core)
    {
        return core;
    }
    core = makeCore (type, core_name);

    core->initializeFromArgs (argc, argv);
    bool success = registerCore (core);
    if (!success)
    {
        core = findCore (core_name);
        if (core)
        {
            return core;
        }
    }

    return core;
}

/** lambda function to join cores before the destruction happens to avoid potential problematic calls in the
 * loops*/
static auto destroyerCallFirst = [](auto &core) {
    core->processDisconnect (true);
    core->joinAllThreads ();
};

/** so the problem this is addressing is that unregister can potentially cause a destructor to fire
that destructor can delete a thread variable, unfortunately it is possible that a thread stored in this variable
can do the unregister operation and destroy itself meaning it is unable to join and thus will call std::terminate
what we do is delay the destruction until it is called in a different thread which allows the destructor to fire if
need be
without issue*/
static DelayedDestructor<CommonCore>
  delayedDestroyer (destroyerCallFirst);  //!< the object handling the delayed destruction

static SearchableObjectHolder<CommonCore> searchableObjects;  //!< the object managing the searchable objects

//this will trip the line when it is destroyed at global destruction time
static tripwire::TripWireTrigger tripTrigger;

std::shared_ptr<Core> findCore (const std::string &name) { return searchableObjects.findObject (name); }



bool isJoinableCoreOfType (core_type type, const std::shared_ptr<CommonCore> &ptr)
{
    if (ptr->isOpenToNewFederates ())
    {
        switch (type)
        {
        case core_type::ZMQ:
#if HELICS_HAVE_ZEROMQ
            return (dynamic_cast<ZmqCore *> (ptr.get ()) != nullptr);
#else
            break;
#endif
        case core_type::MPI:
#if HELICS_HAVE_MPI
            return (dynamic_cast<MpiCore *> (ptr.get ()) != nullptr);
#else
            break;
#endif
        case core_type::TEST:
            return (dynamic_cast<TestCore *> (ptr.get ()) != nullptr);
        case core_type::INTERPROCESS:
        case core_type::IPC:
            return (dynamic_cast<IpcCore *> (ptr.get ()) != nullptr);
        case core_type::UDP:
            return (dynamic_cast<UdpCore *> (ptr.get ()) != nullptr);
        case core_type::TCP:
#ifndef DISABLE_TCP_CORE
            return (dynamic_cast<TcpCore *> (ptr.get ()) != nullptr);
#endif
        default:
            return true;
        }
    }
    return false;
}

std::shared_ptr<Core> findJoinableCoreOfType (core_type type)
{
    return searchableObjects.findObject ([type](auto &ptr) { return isJoinableCoreOfType (type, ptr); });
}

bool registerCore (std::shared_ptr<Core> core)
{
    bool res = false;
    auto tcore = std::dynamic_pointer_cast<CommonCore> (std::move (core));
    if (tcore)
    {
        res = searchableObjects.addObject (tcore->getIdentifier (), tcore);
    }
    cleanUpCores ();
    if (res)
    {
        delayedDestroyer.addObjectsToBeDestroyed (tcore);
    }
    return res;
}

size_t cleanUpCores () { return delayedDestroyer.destroyObjects (); }

size_t cleanUpCores (int delay) { return delayedDestroyer.destroyObjects (delay); }

void copyCoreIdentifier (const std::string &copyFromName, const std::string &copyToName)
{
    searchableObjects.copyObject (copyFromName, copyToName);
}

void unregisterCore (const std::string &name)
{
    if (!searchableObjects.removeObject (name))
    {
        searchableObjects.removeObject ([&name](auto &obj) { return (obj->getIdentifier () == name); });
    }
}

}  // namespace CoreFactory
}  // namespace helics
