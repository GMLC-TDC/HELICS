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
std::string helicsTypeString (helics_core_type type)
{
    switch (type)
    {
    case HELICS_MPI:
        return "_mpi";
    case HELICS_TEST:
        return "_test";
    case HELICS_ZMQ:
        return "_zmq";
    case HELICS_INTERPROCESS:
        return "_ipc";
    default:
        return "";
    }
}

helics_core_type coreTypeFromString (const std::string &type)
{
    if ((type.empty ()) || (type == "default"))
    {
        return HELICS_DEFAULT;
    }
    else if ((type == "mpi") || (type == "MPI"))
    {
        return HELICS_MPI;
    }
    else if ((type == "0mq") || (type == "zmq") || (type == "zeromq") || (type == "ZMQ"))
    {
        return HELICS_ZMQ;
    }
    else if ((type == "interprocess") || (type == "ipc"))
    {
        return HELICS_INTERPROCESS;
    }
    else if ((type == "test") || (type == "test1") || (type == "local"))
    {
        return HELICS_TEST;
    }
    throw (std::invalid_argument ("unrecognized core type"));
}

namespace CoreFactory
{
	std::shared_ptr<Core> create(helics_core_type type, const std::string &initializationString)
	{
		std::shared_ptr<Core> core;
		if (type == HELICS_DEFAULT)
		{  // deal with the default type
			if (isAvailable(HELICS_ZMQ))
			{
				type = HELICS_ZMQ;
			}
			else if (isAvailable(HELICS_INTERPROCESS))
			{
				type = HELICS_INTERPROCESS;
			}
			else
			{
				type = HELICS_TEST;
			}
		}
		switch (type)
		{
		case HELICS_ZMQ:
		{
#if HELICS_HAVE_ZEROMQ
			core = std::make_shared<ZmqCore>();
#else
			assert(false);
#endif
			break;
		}
		case HELICS_MPI:
		{
#if HELICS_HAVE_MPI
			core = std::make_shared<MpiCore>();
#else
			assert(false);
#endif
			break;
		}
		case HELICS_TEST:
		{
			core = std::make_shared<TestCore>();
			break;
		}
		case HELICS_INTERPROCESS:
			core = std::make_shared<IpcCore>();
			break;
		default:
			assert(false);
		}
		core->initialize(initializationString);
		auto ccore = std::dynamic_pointer_cast<CommonCore> (core);
		if (ccore)
		{
			registerCommonCore(ccore);
		}
		return core;
	}

	std::shared_ptr<Core> FindOrCreate(helics_core_type type,
		const std::string &core_name,
		const std::string &initializationString)
	{
		std::shared_ptr<Core> core = findCore(core_name);
		if (core)
		{
			return core;
		}
		if (type == HELICS_DEFAULT)
		{  // deal with the default type
			if (isAvailable(HELICS_ZMQ))
			{
				type = HELICS_ZMQ;
			}
			else if (isAvailable(HELICS_INTERPROCESS))
			{
				type = HELICS_INTERPROCESS;
			}
			else
			{
				type = HELICS_TEST;
			}
		}
		switch (type)
		{
		case HELICS_ZMQ:
		{
#if HELICS_HAVE_ZEROMQ
			core = std::make_shared<ZmqCore>(core_name);
#else
			assert(false);
#endif
			break;
		}
		case HELICS_MPI:
		{
#if HELICS_HAVE_MPI
			core = std::make_shared<MpiCore>(core_name);
#else
			assert(false);
#endif
			break;
		}
		case HELICS_TEST:
		{
			core = std::make_shared<TestCore>(core_name);
			break;
		}
		case HELICS_INTERPROCESS:
			core = std::make_shared<IpcCore>(core_name);
			break;
		default:
			assert(false);
		}
		core->initialize(initializationString);
		auto ccore = std::dynamic_pointer_cast<CommonCore> (core);
		if (ccore)
		{
			bool success = registerCommonCore(ccore);
			if (!success)
			{
				core = findCore(core_name);
				if (core)
				{
					return core;
				}
			}
		}
		return core;
	}

	bool isAvailable(helics_core_type type)
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

	static std::map<std::string, std::shared_ptr<CommonCore>> CoreMap;

	static std::mutex mapLock;  //!<lock for the broker and core maps

	/** so the problem this is addressing is that unregister can potentially cause a destructor to fire
	that destructor can delete a thread variable, unfortunately it is possible that a thread stored in this variable
	can do the unregister operation and destroy itself meaning it is unable to join and thus will call std::terminate
	what we do is delay the destruction until it is called in a different thread which allows the destructor to fire if
	need be
	without issue*/
	static std::vector<std::shared_ptr<CommonCore>> delayedDestruction;

	std::shared_ptr<CommonCore> findCore(const std::string &name)
	{
		std::lock_guard<std::mutex> lock(mapLock);
		auto fnd = CoreMap.find(name);
		if (fnd != CoreMap.end())
		{
			return fnd->second;
		}
		return nullptr;
	}

	std::shared_ptr<Core> findJoinableCoreOfType(helics_core_type type)
	{
		std::lock_guard<std::mutex> lock(mapLock);
		for (auto &cmap : CoreMap)
		{
			if (cmap.second->isJoinable())
			{
				switch (type)
				{
				case HELICS_ZMQ:
				{
#if HELICS_HAVE_ZEROMQ
					if (dynamic_cast<ZmqCore *> (cmap.second.get()) != nullptr)
					{
						return cmap.second;
					}
#endif
					break;
				}
				case HELICS_MPI:
				{
#if HELICS_HAVE_MPI
					if (dynamic_cast<MPICore *> (cmap.second.get()) != nullptr)
					{
						return cmap.second;
					}
#endif
					break;
				}
				case HELICS_TEST:
				{
					if (dynamic_cast<TestCore *> (cmap.second.get()) != nullptr)
					{
						return cmap.second;
					}
					break;
				}
				case HELICS_INTERPROCESS:
				{
					if (dynamic_cast<IpcCore *> (cmap.second.get()) != nullptr)
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

	bool registerCommonCore(std::shared_ptr<CommonCore> tcore)
	{
		std::lock_guard<std::mutex> lock(mapLock);
		if (!delayedDestruction.empty())
		{
			delayedDestruction.clear();
		}
		auto res = CoreMap.emplace(tcore->getIdentifier(), std::move(tcore));
		return res.second;
	}

	void cleanUpCores()
	{
		std::lock_guard<std::mutex> lock(mapLock);
		delayedDestruction.clear();
	}

	void copyCoreIdentifier(const std::string &copyFromName, const std::string &copyToName)
	{
		std::lock_guard<std::mutex> lock(mapLock);
		auto fnd = CoreMap.find(copyFromName);
		if (fnd != CoreMap.end())
		{
			auto newCorePtr = fnd->second;
			CoreMap.emplace(copyToName, std::move(newCorePtr));
		}
	}

	void unregisterCore(const std::string &name)
	{
		std::lock_guard<std::mutex> lock(mapLock);
		auto fnd = CoreMap.find(name);
		if (fnd != CoreMap.end())
		{
			delayedDestruction.push_back(std::move(fnd->second));
			CoreMap.erase(fnd);
			return;
		}
		for (auto core = CoreMap.begin(); core != CoreMap.end(); ++core)
		{
			if (core->second->getIdentifier() == name)
			{
				delayedDestruction.push_back(std::move(core->second));
				CoreMap.erase(core);
				return;
			}
		}
	}

} // namespace CoreFactory
}  // namespace
