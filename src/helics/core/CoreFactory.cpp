/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "CoreFactory.h"
#include "core-types.h"
#include "helics/config.h"

#if HELICS_HAVE_ZEROMQ
#include "zmq/ZmqCore.h"
#endif

#if HELICS_HAVE_MPI
#include "mpi/mpi-core.h"
#endif

#include "TestCore.h"
#include "ipc/IpcCore.h"

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
    {
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
        assert (false);
#endif
        break;
    }
    case core_type::MPI:
    {
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
        assert (false);
#endif
        break;
    }
    case core_type::TEST:
    {
        if (name.empty ())
        {
            core = std::make_shared<TestCore> ();
        }
        else
        {
            core = std::make_shared<TestCore> (name);
        }
        break;
    }
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
    default:
        assert (false);
    }
    return core;
}

namespace CoreFactory
{
std::shared_ptr<Core> create (core_type type, const std::string &initializationString)
{
    auto core = makeCore (type, "");
    core->initialize (initializationString);
    auto ccore = std::dynamic_pointer_cast<CommonCore> (core);
    if (ccore)
    {
        registerCommonCore (ccore);
    }
    return core;
}

std::shared_ptr<Core> create (core_type type, const std::string &core_name, std::string &initializationString)
{
    auto core = makeCore (type, core_name);
    core->initialize (initializationString);
    auto ccore = std::dynamic_pointer_cast<CommonCore> (core);
    if (ccore)
    {
        registerCommonCore (ccore);
    }
    return core;
}

std::shared_ptr<Core> create (core_type type, int argc, char *argv[])
{
    auto core = makeCore (type, "");

    auto ccore = std::dynamic_pointer_cast<CommonCore> (core);
    if (ccore)
    {
        ccore->InitializeFromArgs (argc, argv);
        registerCommonCore (ccore);
    }
    return core;
}

std::shared_ptr<Core> create (core_type type, const std::string &core_name, int argc, char *argv[])
{
    auto core = makeCore (type, core_name);

    auto ccore = std::dynamic_pointer_cast<CommonCore> (core);
    if (ccore)
    {
        ccore->InitializeFromArgs (argc, argv);
        registerCommonCore (ccore);
    }
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
    auto ccore = std::dynamic_pointer_cast<CommonCore> (core);
    if (ccore)
    {
        bool success = registerCommonCore (ccore);
        if (!success)
        {
            core = findCore (core_name);
            if (core)
            {
                return core;
            }
        }
    }
    return core;
}

std::shared_ptr<Core> FindOrCreate (core_type type, const std::string &core_name, int argc, char *argv[])
{
    std::shared_ptr<Core> core = findCore (core_name);
    if (core)
    {
        return core;
    }
    core = makeCore (type, core_name);

    auto ccore = std::dynamic_pointer_cast<CommonCore> (core);
    if (ccore)
    {
        ccore->InitializeFromArgs (argc, argv);
        bool success = registerCommonCore (ccore);
        if (!success)
        {
            core = findCore (core_name);
            if (core)
            {
                return core;
            }
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
	case core_type::UDP:
        break;
    default:
        assert (false);
    }

    return available;
}

static std::map<std::string, std::shared_ptr<CommonCore>> CoreMap;

static std::mutex mapLock;  //!< lock for the broker and core maps

/** so the problem this is addressing is that unregister can potentially cause a destructor to fire
that destructor can delete a thread variable, unfortunately it is possible that a thread stored in this variable
can do the unregister operation and destroy itself meaning it is unable to join and thus will call std::terminate
what we do is delay the destruction until it is called in a different thread which allows the destructor to fire if
need be
without issue*/
static std::vector<std::shared_ptr<CommonCore>> delayedDestruction;

std::shared_ptr<CommonCore> findCore (const std::string &name)
{
    std::lock_guard<std::mutex> lock (mapLock);
    auto fnd = CoreMap.find (name);
    if (fnd != CoreMap.end ())
    {
        return fnd->second;
    }
    return nullptr;
}

std::shared_ptr<Core> findJoinableCoreOfType (core_type type)
{
    std::lock_guard<std::mutex> lock (mapLock);
    for (auto &cmap : CoreMap)
    {
        if (cmap.second->isJoinable ())
        {
            switch (type)
            {
            case core_type::ZMQ:
            {
#if HELICS_HAVE_ZEROMQ
                if (dynamic_cast<ZmqCore *> (cmap.second.get ()) != nullptr)
                {
                    return cmap.second;
                }
#endif
                break;
            }
            case core_type::MPI:
            {
#if HELICS_HAVE_MPI
                if (dynamic_cast<MPICore *> (cmap.second.get ()) != nullptr)
                {
                    return cmap.second;
                }
#endif
                break;
            }
            case core_type::TEST:
            {
                if (dynamic_cast<TestCore *> (cmap.second.get ()) != nullptr)
                {
                    return cmap.second;
                }
                break;
            }
            case core_type::INTERPROCESS:
            {
                if (dynamic_cast<IpcCore *> (cmap.second.get ()) != nullptr)
                {
                    return cmap.second;
                }
                break;
            }
            default:
                return cmap.second;
            }
        }
    }
    return nullptr;
}

bool registerCommonCore (std::shared_ptr<CommonCore> tcore)
{
    std::unique_lock<std::mutex> lock (mapLock);
    if (!delayedDestruction.empty ())
    {
		auto tempBuffer = delayedDestruction;
        delayedDestruction.clear ();
		lock.unlock();
		//don't let the destructors get called with the lock engaged
    }
    auto res = CoreMap.emplace (tcore->getIdentifier (), std::move (tcore));
    return res.second;
}

void cleanUpCores ()
{
    std::unique_lock<std::mutex> lock (mapLock);
	if (!delayedDestruction.empty())
	{
		auto tempBuffer = delayedDestruction;
		delayedDestruction.clear();
		lock.unlock();
		//don't let the destructors get called with the lock engaged
	}
}

void copyCoreIdentifier (const std::string &copyFromName, const std::string &copyToName)
{
    std::lock_guard<std::mutex> lock (mapLock);
    auto fnd = CoreMap.find (copyFromName);
    if (fnd != CoreMap.end ())
    {
        auto newCorePtr = fnd->second;
        CoreMap.emplace (copyToName, std::move (newCorePtr));
    }
}

void unregisterCore (const std::string &name)
{
    std::lock_guard<std::mutex> lock (mapLock);
    auto fnd = CoreMap.find (name);
    if (fnd != CoreMap.end ())
    {
        delayedDestruction.push_back (std::move (fnd->second));
        CoreMap.erase (fnd);
        return;
    }
    for (auto core = CoreMap.begin (); core != CoreMap.end (); ++core)
    {
        if (core->second->getIdentifier () == name)
        {
            delayedDestruction.push_back (std::move (core->second));
            CoreMap.erase (core);
            return;
        }
    }
}

}  // namespace CoreFactory
}  // namespace helics
