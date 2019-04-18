/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
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

#include "AsioContextManager.h"

#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <stdexcept>

/** a storage system for the available core objects allowing references by name to the core
 */
std::map<std::string, std::shared_ptr<AsioContextManager>> AsioContextManager::services;

/** we expect operations on core object that modify the map to be rare but we absolutely need them to be thread
safe so we are going to use a lock that is entirely controlled by this file*/
static std::mutex serviceLock;

std::shared_ptr<AsioContextManager> AsioContextManager::getServicePointer (const std::string &serviceName)
{
    std::shared_ptr<AsioContextManager> servicePtr;
    std::lock_guard<std::mutex> serveLock (serviceLock);  // just to ensure that nothing funny happens if you try
                                                          // to get a context while it is being constructed
    auto fnd = services.find (serviceName);
    if (fnd != services.end ())
    {
        servicePtr = fnd->second;
        return servicePtr;
    }

    servicePtr = std::shared_ptr<AsioContextManager> (new AsioContextManager (serviceName));
    services.emplace (serviceName, servicePtr);
    return servicePtr;
    // if it doesn't find it make a new one with the appropriate name
}

std::shared_ptr<AsioContextManager> AsioContextManager::getExistingServicePointer (const std::string &serviceName)
{
    std::lock_guard<std::mutex> serveLock (serviceLock);  // just to ensure that nothing funny happens if you try
                                                          // to get a context while it is being constructed
    auto fnd = services.find (serviceName);
    if (fnd != services.end ())
    {
        return fnd->second;
    }

    return nullptr;
}

asio::io_context &AsioContextManager::getService (const std::string &serviceName)
{
    return getServicePointer (serviceName)->getBaseService ();
}

asio::io_context &AsioContextManager::getExistingService (const std::string &serviceName)
{
    auto ptr = getExistingServicePointer (serviceName);
    if (ptr)
    {
        return ptr->getBaseService ();
    }
    throw (std::invalid_argument ("the service name specified was not available"));
}

void AsioContextManager::closeService (const std::string &serviceName)
{
    std::unique_lock<std::mutex> servelock (serviceLock);
    auto fnd = services.find (serviceName);
    //    std::cout << "closing service manager\n";
    if (fnd != services.end ())
    {
        auto ptr = fnd->second;
        services.erase (fnd);
        servelock.unlock ();
        if (ptr->running)
        {
            std::lock_guard<std::mutex> nullLock (ptr->runningLoopLock);
            ptr->nullwork.reset ();
            ptr->ictx->stop ();
            ptr->loopRet.get ();
        }
    }
}

void AsioContextManager::setServiceToLeakOnDelete (const std::string &serviceName)
{
    std::lock_guard<std::mutex> servelock (serviceLock);
    auto fnd = services.find (serviceName);
    if (fnd != services.end ())
    {
        fnd->second->leakOnDelete = true;
    }
}
AsioContextManager::~AsioContextManager ()
{
    //  std::cout << "deleting service manager\n";

    if (running)
    {
        try
        {
            std::lock_guard<std::mutex> nullLock (runningLoopLock);
            nullwork.reset ();
            ictx->stop ();
            loopRet.get ();
        }
        catch (...)
        {
        }
    }
    if (leakOnDelete)
    {
        // yes I am purposefully leaking this PHILIP TOP
        // this capability is needed for some operations on particular OS's with the shared library operations that
        // will crash if this is closed before the library closes which really only happens at program termination
        auto val = ictx.release ();
        (void)(val);
    }
}

AsioContextManager::AsioContextManager (const std::string &serviceName)
    : name (serviceName), ictx (std::make_unique<asio::io_context> ())
{
}

AsioContextManager::LoopHandle AsioContextManager::runServiceLoop (const std::string &serviceName)
{
    std::unique_lock<std::mutex> servelock (serviceLock);
    auto fnd = services.find (serviceName);
    if (fnd != services.end ())
    {
        auto ptr = fnd->second;
        servelock.unlock ();
        return ptr->startServiceLoop ();
    }
    throw (std::invalid_argument ("the service name specified was not available"));
}

AsioContextManager::LoopHandle AsioContextManager::startServiceLoop ()
{
    ++runCounter;  // atomic

    bool exp = false;
    if (running.compare_exchange_strong (exp, true))
    {
        auto ptr = shared_from_this ();
        std::packaged_task<void()> serviceTask ([ptr = std::move (ptr)]() { serviceProcessingLoop (ptr); });
        //   std::cout << "run Service loop " << runCounter << "\n";
        std::unique_lock<std::mutex> nullLock (runningLoopLock);

        nullwork = std::make_unique<asio::io_context::work> (getBaseService ());
        loopRet = serviceTask.get_future ();
        nullLock.unlock ();
        std::thread serviceThread (std::move (serviceTask));
        serviceThread.detach ();
        //  std::cout << "starting service loop thread " << runCounter << "\n";
    }
    else
    {
        std::unique_lock<std::mutex> nullLock (runningLoopLock);
        if (getBaseService ().stopped ())
        {
            // std::cout << "run Service loop already stopped" << runCounter << "\n";
            if (loopRet.valid ())
            {
                loopRet.get ();
            }
            nullLock.unlock ();
            exp = false;
            if (running.compare_exchange_strong (exp, true))
            {
                auto ptr = shared_from_this ();
                std::packaged_task<void()> serviceTask (
                  [ptr = std::move (ptr)]() { serviceProcessingLoop (ptr); });
                nullLock.lock ();
                nullwork = std::make_unique<asio::io_context::work> (getBaseService ());
                loopRet = serviceTask.get_future ();
                nullLock.unlock ();
                std::thread serviceThread (std::move (serviceTask));
                serviceThread.detach ();
            }
        }
    }
    return std::make_unique<Servicer> (shared_from_this ());
}

void AsioContextManager::haltServiceLoop ()
{
    if (running.load ())
    {
        // std::cout << "service loop halted "<<ptr->runCounter<<"\n";
        if (--runCounter <= 0)
        {
            std::lock_guard<std::mutex> nullLock (runningLoopLock);
            //    std::cout << "calling halt on service loop \n";

            if (runCounter <= 0)
            {
                if (nullwork)
                {
                    terminateLoop = true;
                    nullwork.reset ();
                    ictx->stop ();
                    int lcnt = 0;
                    while (loopRet.wait_for (std::chrono::milliseconds (0)) == std::future_status::timeout)
                    {
                        if (lcnt == 0)
                        {
                            std::this_thread::yield ();
                        }
                        else
                        {
                            std::this_thread::sleep_for (std::chrono::milliseconds (50));
                            ++lcnt;
                            ictx->stop ();
                        }
                    }
                    loopRet.get ();
                    ictx->reset ();  // prepare for future runs
                    terminateLoop = false;
                }
            }
        }
    }
    else
    {
        runCounter.store (0);
    }
}

void serviceProcessingLoop (std::shared_ptr<AsioContextManager> ptr)
{
    while ((ptr->runCounter > 0) && (!(ptr->terminateLoop)))
    {
        auto clk = std::chrono::steady_clock::now ();
        try
        {
            ptr->ictx->run ();
        }
        catch (const std::system_error &se)
        {
            auto nclk = std::chrono::steady_clock::now ();
            std::cerr << "asio system error in service loop " << se.what () << " ran for "
                      << (nclk - clk).count () / 1000000 << "ms" << std::endl;
        }
        catch (const std::exception &e)
        {
            auto nclk = std::chrono::steady_clock::now ();
            std::cerr << "std::exception in service loop " << e.what () << " ran for "
                      << (nclk - clk).count () / 1000000 << "ms" << std::endl;
        }
        catch (...)
        {
            std::cout << "caught other error in service loop" << std::endl;
        }
    }

    //   std::cout << "service loop stopped\n";
    ptr->running.store (false);
}
