/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
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

#include "ZmqContextManager.h"

#include "cppzmq/zmq.hpp"

#include <iostream>
#include <map>
#include <mutex>
#include <utility>

/** a storage system for the available core objects allowing references by name to the core
 */
std::map<std::string, std::shared_ptr<ZmqContextManager>> ZmqContextManager::contexts;

/** we expect operations on core object that modify the map to be rare but we absolutely need them
to be thread safe so we are going to use a lock that is entirely controlled by this file*/
static std::mutex contextLock;

std::shared_ptr<ZmqContextManager>
    ZmqContextManager::getContextPointer(const std::string& contextName)
{
    std::lock_guard<std::mutex> conlock(
        contextLock);  // just to ensure that nothing funny happens if you try to
    // get a context while it is being constructed
    auto fnd = contexts.find(contextName);
    if (fnd != contexts.end()) {
        return fnd->second;
    }
    // std::cout << "creating context in " << std::this_thread::get_id() << std::endl;
    auto newContext = std::shared_ptr<ZmqContextManager>(new ZmqContextManager(contextName));
    contexts.emplace(contextName, newContext);
    return newContext;
    // if it doesn't make a new one with the appropriate name
}

zmq::context_t& ZmqContextManager::getContext(const std::string& contextName)
{
    return getContextPointer(contextName)->getBaseContext();
}

void ZmqContextManager::startContext(const std::string& contextName)
{
    std::lock_guard<std::mutex> conlock(contextLock);
    auto fnd = contexts.find(contextName);
    if (fnd == contexts.end()) {
        auto newContext = std::shared_ptr<ZmqContextManager>(new ZmqContextManager(contextName));
        contexts.emplace(contextName, std::move(newContext));
    }
}

void ZmqContextManager::closeContext(const std::string& contextName)
{
    std::lock_guard<std::mutex> conlock(contextLock);
    auto fnd = contexts.find(contextName);
    if (fnd != contexts.end()) {
        contexts.erase(fnd);
    }
}

bool ZmqContextManager::setContextToLeakOnDelete(const std::string& contextName)
{
    std::lock_guard<std::mutex> conlock(contextLock);
    auto fnd = contexts.find(contextName);
    if (fnd != contexts.end()) {
        fnd->second->leakOnDelete = true;
    }
    return false;
}

bool ZmqContextManager::setContextToNotLeakOnDelete(const std::string& contextName)
{
    std::lock_guard<std::mutex> conlock(contextLock);
    auto fnd = contexts.find(contextName);
    if (fnd != contexts.end()) {
        fnd->second->leakOnDelete = false;
    }
    return false;
}
ZmqContextManager::~ZmqContextManager()
{
    // std::cout << "destroying context in " << std::this_thread::get_id() << std::endl;

    if (leakOnDelete) {
        // yes I am purposefully leaking this( PHILIP TOP)
        // do the vagaries of library closeout this may end up being destroyed too soon and can
        // cause some extraneous errors to show up when closing programs, so this just lets the OS
        // process cleanup since the only issues occur on program termination, in other situations
        // closing does present an issue and since this particular object is mostly only closed on
        // termination this is the default.
        auto* val = zcontext.release();
        (void)(val);
    }
}

ZmqContextManager::ZmqContextManager(const std::string& contextName):
    name(contextName), zcontext(std::make_unique<zmq::context_t>(1, 4096))
{
}
