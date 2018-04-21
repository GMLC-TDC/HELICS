/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
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

#include <iostream>
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
    std::unique_lock<std::mutex> servelock (serviceLock);
    auto fnd = services.find (serviceName);
    //    std::cout << "closing service manager\n";
    if (fnd != services.end ())
    {
        auto ptr = fnd->second;
        services.erase(fnd);
        servelock.unlock();
        if (ptr->running)
        {
            std::lock_guard<std::mutex> nullLock(ptr->runningLoopLock);
            ptr->nullwork.reset ();
            ptr->iserv->stop ();
            ptr->loopRet.get ();
        }

        
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
    //  std::cout << "deleting service manager\n";
    if (running)
    {
        std::lock_guard<std::mutex> nullLock(runningLoopLock);
        nullwork.reset ();
        iserv->stop ();
        loopRet.get ();
    }
    if (leakOnDelete)
    {
        // yes I am purposefully leaking this PHILIP TOP
        // this capability is needed for some operations on particular OS's with the shared library operations that
        // will crash if this is closed before the library closes
        auto val = iserv.release ();
        (void)(val);
    }
}

AsioServiceManager::AsioServiceManager (const std::string &serviceName)
    : name (serviceName), iserv (std::make_unique<boost::asio::io_service> ())
{
}

AsioServiceManager::LoopHandle AsioServiceManager::runServiceLoop (const std::string &serviceName)
{
    std::unique_lock<std::mutex> servelock (serviceLock);
    auto fnd = services.find (serviceName);
    if (fnd != services.end ())
    {
        auto ptr = fnd->second;
        servelock.unlock();
        ++ptr->runCounter;
        std::lock_guard<std::mutex> nullLock(ptr->runningLoopLock);
        if (!ptr->running)
        {
            // std::cout << "run Service loop " << ptr->runCounter << "\n";
            ptr->nullwork = std::make_unique<boost::asio::io_service::work> (ptr->getBaseService ());
            ptr->running = true;
            ptr->loopRet = std::async (std::launch::async, [ptr]() { serviceProcessingLoop(ptr); });
        }
        else
        {
            if (ptr->getBaseService ().stopped ())
            {
                // std::cout << "run Service loop already stopped" << ptr->runCounter << "\n";
                if (ptr->loopRet.valid ())
                {
                    ptr->loopRet.get ();
                }
                ptr->nullwork = std::make_unique<boost::asio::io_service::work> (ptr->getBaseService ());
                ptr->running = true;
                ptr->loopRet = std::async (std::launch::async, [ptr]() { serviceProcessingLoop(ptr); });
            }
        }
        return std::make_unique<servicer> (ptr);
    }
    throw (std::invalid_argument ("the service name specified was not available"));
}

void AsioServiceManager::haltServiceLoop ()
{
    if (running)
    {
        // std::cout << "service loop halted "<<ptr->runCounter<<"\n";
        if (runCounter > 0)
        {
            --runCounter;
        }
        if (runCounter <= 0)
        {
            std::lock_guard<std::mutex> nullLock(runningLoopLock);
            //    std::cout << "calling halt on service loop \n";
            if (nullwork)
            {
                nullwork.reset ();
                iserv->stop ();
                loopRet.get ();
                iserv->reset ();  // prepare for future runs
            }
        }
    }
    else
    {
        runCounter = 0;
    }
}

void serviceProcessingLoop (std::shared_ptr<AsioServiceManager> ptr)
{
    try
    {
        ptr->iserv->run ();
    }
    catch (const std::exception &e)
    {
        std::cerr << "std::exception in service loop " << e.what () << std::endl;
    }
    catch (...)
    {
        std::cout << "caught other error in service loop" << std::endl;
    }
    // std::cout << "service loop stopped\n";
    ptr->running.store (false);
}
