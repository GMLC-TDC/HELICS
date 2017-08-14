/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/config.h"
#include "CoreFactory.h"
#include "core-types.h"

#if HELICS_HAVE_ZEROMQ
#include "zmq/zmq-core.h"
#endif

#if HELICS_HAVE_MPI
#include "mpi/mpi-core.h"
#endif

#include "TestCore.h"

#include <cassert>

namespace helics {

std::shared_ptr<Core> CoreFactory::create(helics_core_type type, const std::string &initializationString) {

  std::shared_ptr<Core> core;

  switch(type)
    {
    case HELICS_ZMQ:
      {
#if HELICS_HAVE_ZEROMQ
        core = std::make_shared<ZeroMQCore> ();
#else
        assert (false);
#endif
        break;
      }
    case HELICS_MPI:
      {
#if HELICS_HAVE_MPI
        core = std::make_shared<MpiCore> ();
#else
        assert (false);
#endif
        break;
      }
    case HELICS_TEST:
      {
        core = std::make_shared<TestCore> ();
        break;
      }
	case HELICS_INTERPROCESS:
		break;
    default:
      assert (false);
    }
  core->initialize(initializationString);
  auto ccore = std::dynamic_pointer_cast<CommonCore>(core);
  if (ccore)
  {
	  registerCore(ccore);
  }
  return core;
}

std::shared_ptr<Core> CoreFactory::create(helics_core_type type,const std::string &core_name, const std::string &initializationString) {

	std::shared_ptr<Core> core;

	switch (type)
	{
	case HELICS_ZMQ:
	{
#if HELICS_HAVE_ZEROMQ
		core = std::make_shared<ZeroMQCore>(core_name);
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
		break;
	default:
		assert(false);
	}
	core->initialize(initializationString);
	auto ccore = std::dynamic_pointer_cast<CommonCore>(core);
	if (ccore)
	{
		registerCore(ccore);
	}
	return core;
}


bool CoreFactory::available (helics_core_type type) {

  bool available = false;

  switch(type)
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
		available = false;
		break;
	}
    default:
      assert (false);
    }

  return available;
}



static std::map<std::string, std::shared_ptr<CommonCore>> CoreMap;

static std::mutex mapLock;  //!<lock for the broker and core maps

/** so the problem this is addressing is that unregister can potentially cause a destructor to fire
that destructor can delete a thread variable, unfortunately it is possible that a thread stored in this variable
can do the unregister operation and destroy itself meaning it is unable to join and thus will call std::terminate
what we do is delay the destruction until it is called in a different thread which allows the destructor to fire if need be
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

bool registerCore(std::shared_ptr<CommonCore> tcore)
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
} // namespace 

