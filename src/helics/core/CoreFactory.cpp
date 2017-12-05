/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "CoreFactory.h"
#include "core-exceptions.h"
#include "core-types.h"
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
#include <cassert>

namespace helics
{
std::string helicsTypeString (core_type type)
{
    switch (type)
    {
    case core_type::MPI:
        return "_mpi";
    case core_type::TEST:
        return "_test";
    case core_type::ZMQ:
        return "_zmq";
    case core_type::INTERPROCESS:
    case core_type::IPC:
        return "_ipc";
    case core_type::TCP:
        return "_tcp";
    case core_type::UDP:
        return "_udp";
    default:
        return "";
    }
}

core_type coreTypeFromString (const std::string &type)
{
    if ((type.empty ()) || (type == "default"))
    {
        return core_type::DEFAULT;
    }
    else if ((type.compare (0, 3, "mpi") == 0) || (type == "MPI"))
    {
        return core_type::MPI;
    }
    else if ((type == "0mq") || (type.compare (0, 3, "zmq") == 0) || (type == "zeromq") || (type == "ZMQ"))
    {
        return core_type::ZMQ;
    }
    else if ((type == "interprocess") || (type.compare (0, 3, "ipc") == 0))
    {
        return core_type::INTERPROCESS;
    }
    else if ((type.compare (0, 4, "test") == 0) || (type == "test1") || (type == "local"))
    {
        return core_type::TEST;
    }
    else if ((type.compare (0, 3, "tcp") == 0) || (type == "TCP"))
    {
        return core_type::TCP;
    }
    else if ((type.compare (0, 3, "udp") == 0) || (type == "UDP"))
    {
        return core_type::UDP;
    }
    throw (std::invalid_argument ("unrecognized core type"));
}

std::shared_ptr<Core> makeCore (core_type type, const std::string &name)
{
    std::shared_ptr<Core> core;

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
        throw (HelicsException ("TCP core is not available"));
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

bool isAvailable (core_type type)
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

std::shared_ptr<Core> findCore (const std::string &name) { return searchableObjects.findObject (name); }

bool isJoinableCoreOfType (core_type type, const std::shared_ptr<CommonCore> &ptr)
{
    if (ptr->isJoinable ())
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
            return (dynamic_cast<MPICore *> (ptr.get ()) != nullptr);
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
