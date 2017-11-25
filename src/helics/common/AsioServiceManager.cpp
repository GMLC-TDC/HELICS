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

#include "AsioServiceManager.h"

#include <boost/asio/io_service.hpp>

#include <map>
#include <mutex>
#include <stdexcept>

/** a storage system for the available core objects allowing references by name to the core
 */
std::map<std::string, std::shared_ptr<AsioServiceManager>> AsioServiceManager::services;

/** we expect operations on core object that modify the map to be rare but we absolutely need them to be thread
safe so we are going to use a lock that is entirely controlled by this file*/
static std::mutex serviceLock;

std::shared_ptr<AsioServiceManager> AsioServiceManager::getServicePointer (const std::string &serviceName)
{
    std::lock_guard<std::mutex> serveLock (
      serviceLock);  // just to ensure that nothing funny happens if you try to get a context
                     // while it is being constructed
    auto fnd = services.find (serviceName);
    if (fnd != services.end ())
    {
        return fnd->second;
    }

    auto newService = std::shared_ptr<AsioServiceManager> (new AsioServiceManager (serviceName));
    services.emplace (serviceName, newService);
    return newService;
    // if it doesn't find it make a new one with the appropriate name
}

std::shared_ptr<AsioServiceManager> AsioServiceManager::getExistingServicePointer (const std::string &serviceName)
{
    std::lock_guard<std::mutex> serveLock (
      serviceLock);  // just to ensure that nothing funny happens if you try to get a context
                     // while it is being constructed
    auto fnd = services.find (serviceName);
    if (fnd != services.end ())
    {
        return fnd->second;
    }

    return nullptr;
}

boost::asio::io_service &AsioServiceManager::getService (const std::string &serviceName)
{
    return getServicePointer (serviceName)->getBaseService ();
}

boost::asio::io_service &AsioServiceManager::getExistingService (const std::string &serviceName)
{
    auto ptr = getExistingServicePointer (serviceName);
    if (ptr)
    {
        return ptr->getBaseService ();
    }
    throw (std::invalid_argument ("the service name specified was not available"));
}

void AsioServiceManager::closeService (const std::string &serviceName)
{
    std::lock_guard<std::mutex> servelock (serviceLock);
    auto fnd = services.find (serviceName);
    if (fnd != services.end ())
    {
        if (fnd->second->running)
        {
            fnd->second->iserv->stop ();
            fnd->second->serviceThread.join ();
        }

        services.erase (fnd);
    }
}

void AsioServiceManager::setServiceToLeakOnDelete (const std::string &serviceName)
{
    std::lock_guard<std::mutex> servelock (serviceLock);
    auto fnd = services.find (serviceName);
    if (fnd != services.end ())
    {
        fnd->second->leakOnDelete = true;
    }
}
AsioServiceManager::~AsioServiceManager ()
{
    if (leakOnDelete)
    {
        // yes I am purposefully leaking this PHILIP TOP
        auto val = iserv.release ();
        (void)(val);
    }
}

AsioServiceManager::AsioServiceManager (const std::string &serviceName) : name (serviceName)
{
    iserv = std::make_unique<boost::asio::io_service> ();
}

void AsioServiceManager::runServiceLoop (const std::string &serviceName)
{
    std::lock_guard<std::mutex> servelock (serviceLock);
    auto fnd = services.find (serviceName);
    if (fnd != services.end ())
    {
        auto ptr = fnd->second;
        if (!ptr->running)
        {
            ptr->running = true;
            ptr->serviceThread = std::thread ([ptr]() { ptr->getBaseService ().run (); });
        }
    }
    throw (std::invalid_argument ("the service name specified was not available"));
}