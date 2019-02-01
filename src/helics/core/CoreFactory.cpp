/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
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

#ifndef DISABLE_TEST_CORE
#include "test/TestCore.h"
#endif

#ifndef DISABLE_IPC_CORE
#include "ipc/IpcCore.h"
#endif

#ifndef DISABLE_UDP_CORE
#include "udp/UdpCore.h"
#endif

#ifndef DISABLE_TCP_CORE
#include "tcp/TcpCore.h"
#endif

#include <cassert>
#include <cstring>

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
#ifndef DISABLE_TCP_CORE
        type = core_type::TCP;
#else
#ifndef DISABLE_UDP_CORE
        type = core_type::UDP;
#else
#ifdef HELICS_HAVE_MPI
        type = core_type::MPI;
#else
#ifndef DISABLE_UDP_CORE
        type = core_type::UDP;
#else
#ifndef DISABLE_IPC_CORE
        type = core_type::IPC;
#else
#ifndef DISABLE_TEST_CORE
        type = core_type::TEST;
#else
        type = core_type::UNRECOGNIZED;
#endif  // DISABLE_TEST_CORE
#endif  // DISABLE_IPC_CORE
#endif  // DISABLE_UDP_CORE
#endif  // HELICS_HAVE_MPI
#endif  // DISABLE_UDP_CORE
#endif  // DISABLE_TCP_CORE
#endif  // HELICS_HAVE_ZEROMQ
    }

    switch (type)
    {
    case core_type::ZMQ:
#if HELICS_HAVE_ZEROMQ
        if (name.empty ())
        {
            core = std::make_shared<zeromq::ZmqCore> ();
        }
        else
        {
            core = std::make_shared<zeromq::ZmqCore> (name);
        }

#else
        throw (HelicsException ("ZMQ core is not available"));
#endif
        break;
    case core_type::ZMQ_SS:
#if HELICS_HAVE_ZEROMQ
        if (name.empty ())
        {
            core = std::make_shared<zeromq::ZmqCoreSS> ();
        }
        else
        {
            core = std::make_shared<zeromq::ZmqCoreSS> (name);
        }

#else
        throw (HelicsException ("ZMQ core is not available"));
#endif
        break;
    case core_type::MPI:
#if HELICS_HAVE_MPI
        if (name.empty ())
        {
            core = std::make_shared<mpi::MpiCore> ();
        }
        else
        {
            core = std::make_shared<mpi::MpiCore> (name);
        }
#else
        throw (HelicsException ("MPI core is not available"));
#endif
        break;
    case core_type::TEST:
#ifndef DISABLE_TEST_CORE
        if (name.empty ())
        {
            core = std::make_shared<testcore::TestCore> ();
        }
        else
        {
            core = std::make_shared<testcore::TestCore> (name);
        }
        break;
#else
        throw (HelicsException ("TEST core is not available"));
#endif
    case core_type::INTERPROCESS:
    case core_type::IPC:
#ifndef DISABLE_IPC_CORE
        if (name.empty ())
        {
            core = std::make_shared<ipc::IpcCore> ();
        }
        else
        {
            core = std::make_shared<ipc::IpcCore> (name);
        }
        break;
#else
        throw (HelicsException ("IPC core is not available"));
#endif
    case core_type::UDP:
#ifndef DISABLE_UDP_CORE
        if (name.empty ())
        {
            core = std::make_shared<udp::UdpCore> ();
        }
        else
        {
            core = std::make_shared<udp::UdpCore> (name);
        }
        break;
#else
        throw (HelicsException ("UDP core is not available"));
#endif
    case core_type::TCP:
#ifndef DISABLE_TCP_CORE
        if (name.empty ())
        {
            core = std::make_shared<tcp::TcpCore> ();
        }
        else
        {
            core = std::make_shared<tcp::TcpCore> (name);
        }
#else
        throw (HelicsException ("TCP core is not available"));
#endif
        break;
    case core_type::TCP_SS:
#ifndef DISABLE_TCP_CORE
        if (name.empty ())
        {
            core = std::make_shared<tcp::TcpCoreSS> ();
        }
        else
        {
            core = std::make_shared<tcp::TcpCoreSS> (name);
        }
#else
        throw (HelicsException ("TCP single socket core is not available"));
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
    auto core = makeCore (type, std::string ());
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

std::shared_ptr<Core> create (int argc, const char *const *argv)
{
    core_type type = core_type::DEFAULT;
    for (int ii = 1; ii < argc; ++ii)
    {
        if (strncmp ("coretype", argv[ii], 8) == 0)
        {
            if (strlen (argv[ii]) > 9)
            {
                type = coreTypeFromString (&(argv[ii][9]));
            }
            else
            {
                type = coreTypeFromString (argv[ii + 1]);
            }
            break;
        }
    }
    return create (type, argc, argv);
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

// this will trip the line when it is destroyed at global destruction time
static tripwire::TripWireTrigger tripTrigger;

std::shared_ptr<Core> findCore (const std::string &name) { return searchableObjects.findObject (name); }

static bool isJoinableCoreOfType (core_type type, const std::shared_ptr<CommonCore> &ptr)
{
    if (ptr->isOpenToNewFederates ())
    {
        switch (type)
        {
        case core_type::ZMQ:
#if HELICS_HAVE_ZEROMQ
            return (dynamic_cast<zeromq::ZmqCore *> (ptr.get ()) != nullptr);
#else
            break;
#endif
        case core_type::MPI:
#if HELICS_HAVE_MPI
            return (dynamic_cast<mpi::MpiCore *> (ptr.get ()) != nullptr);
#else
            break;
#endif
        case core_type::TEST:
#ifndef DISABLE_TEST_CORE
            return (dynamic_cast<testcore::TestCore *> (ptr.get ()) != nullptr);
#else
            break;
#endif
        case core_type::INTERPROCESS:
        case core_type::IPC:
#ifndef DISABLE_IPC_CORE
            return (dynamic_cast<ipc::IpcCore *> (ptr.get ()) != nullptr);
#else
            break;
#endif
        case core_type::UDP:
#ifndef DISABLE_UDP_CORE
            return (dynamic_cast<udp::UdpCore *> (ptr.get ()) != nullptr);
#else
            break;
#endif
        case core_type::TCP:
#ifndef DISABLE_TCP_CORE
            return (dynamic_cast<tcp::TcpCore *> (ptr.get ()) != nullptr);
#else
            break;
#endif
        case core_type::TCP_SS:
#ifndef DISABLE_TCP_CORE
            return (dynamic_cast<tcp::TcpCoreSS *> (ptr.get ()) != nullptr);
#else
            break;
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

bool registerCore (const std::shared_ptr<Core> &core)
{
    bool res = false;
    auto tcore = std::dynamic_pointer_cast<CommonCore> (core);
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

size_t cleanUpCores (std::chrono::milliseconds delay) { return delayedDestroyer.destroyObjects (delay); }

bool copyCoreIdentifier (const std::string &copyFromName, const std::string &copyToName)
{
    return searchableObjects.copyObject (copyFromName, copyToName);
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
