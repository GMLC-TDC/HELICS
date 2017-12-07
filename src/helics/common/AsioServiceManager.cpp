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

#include <map>
#include <mutex>
#include <stdexcept>
#include <iostream>

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
            fnd->second->nullwork.reset ();
            fnd->second->iserv->stop ();
            fnd->second->loopRet.get();
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
    if (running)
    {
        nullwork.reset ();
        iserv->stop ();
        loopRet.get();
    }
    if (leakOnDelete)
    {
        // yes I am purposefully leaking this PHILIP TOP
        // this capability is needed for some operations on particular OS's with the shared library operations
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
        ++ptr->runCounter;
        if (!ptr->running)
        {
            ptr->nullwork = std::make_unique<boost::asio::io_service::work> (ptr->getBaseService ());
            ptr->running = true;
            ptr->loopRet = std::async (std::launch::async, [ptr]() { serviceRunLoop(ptr); });
        }
        else
        {
            if (ptr->getBaseService ().stopped ())
            {
                ptr->loopRet.get();
                ptr->nullwork = std::make_unique<boost::asio::io_service::work> (ptr->getBaseService ());
                ptr->running = true;
                ptr->loopRet = std::async(std::launch::async, [ptr]() { serviceRunLoop(ptr); });
            }
        }
        return;
    }
    throw (std::invalid_argument ("the service name specified was not available"));
}

void AsioServiceManager::haltServiceLoop (const std::string &serviceName)
{
    std::lock_guard<std::mutex> servelock (serviceLock);
    auto fnd = services.find (serviceName);
    if (fnd != services.end ())
    {
        auto ptr = fnd->second;
        if (ptr->running)
        {
            if (ptr->runCounter > 0)
            {
                --ptr->runCounter;
            }
            if (ptr->runCounter <= 0)
            {
                ptr->nullwork.reset ();
                ptr->iserv->stop();
                ptr->loopRet.get();
               
            }
        }
        return;
    }
    throw (std::invalid_argument ("the service name specified was not available"));
}

void serviceRunLoop(std::shared_ptr<AsioServiceManager> ptr)
{
    std::cout << "starting service loop" << std::endl;
    try
    {
        ptr->iserv->run();
    }
    catch (...)
    {
        std::cout << "caught error in service loop" << std::endl;
    }
    ptr->running = false;
    std::cout << "exiting service loop" << std::endl;
}