/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was modified by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute;
the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence
Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/
/*
 * LLNS Copyright Start
 * Copyright (c) 2017, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#include "zmqContextManager.h"

#include "cppzmq/zmq.hpp"
#include <map>
#include <mutex>

/** a storage system for the available core objects allowing references by name to the core
 */
std::map<std::string, std::shared_ptr<zmqContextManager>> zmqContextManager::contexts;

/** we expect operations on core object that modify the map to be rare but we absolutely need them to be thread
safe so we are going to use a lock that is entirely controlled by this file*/
static std::mutex contextLock;

std::shared_ptr<zmqContextManager> zmqContextManager::getContextPointer (const std::string &contextName)
{
    std::lock_guard<std::mutex> conlock (
      contextLock);  // just to ensure that nothing funny happens if you try to get a context
                     // while it is being constructed
    auto fnd = contexts.find (contextName);
    if (fnd != contexts.end ())
    {
        return fnd->second;
    }
    else
    {  // can't use make_shared since it is a private constructor
        auto newContext = std::shared_ptr<zmqContextManager> (new zmqContextManager (contextName));
        contexts.emplace (contextName, newContext);
        return newContext;
    }
    // if it doesn't make a new one with the appropriate name
}

zmq::context_t &zmqContextManager::getContext (const std::string &contextName)
{
    return getContextPointer (contextName)->getBaseContext ();
}

void zmqContextManager::closeContext (const std::string &contextName)
{
    std::lock_guard<std::mutex> conlock (contextLock);
    auto fnd = contexts.find (contextName);
    if (fnd != contexts.end ())
    {
        contexts.erase (fnd);
    }
}

void zmqContextManager::setContextToLeakOnDelete(const std::string &contextName)
{
	std::lock_guard<std::mutex> conlock(contextLock);
	auto fnd = contexts.find(contextName);
	if (fnd != contexts.end())
	{
		fnd->second->leakOnDelete = true;
	}
 }
zmqContextManager::~zmqContextManager()
{
	if (leakOnDelete)
	{
		//yes I am purposefully leaking this PHILIP TOP
		auto val = zcontext.release();
		(void)(val);
	}
}

zmqContextManager::zmqContextManager (const std::string &contextName) : name (contextName)
{
    zcontext = std::make_unique<zmq::context_t> ();
}