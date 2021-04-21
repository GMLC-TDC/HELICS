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

#include "AsioContextManager.h"

#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <stdexcept>
#include <utility>

/** a storage system for the available core objects allowing references by name to the core
 */
std::map<std::string, std::shared_ptr<AsioContextManager>> AsioContextManager::contexts;

/** we expect operations on core object that modify the map to be rare but we absolutely need them
to be thread safe so we are going to use a lock that is entirely controlled by this file*/
static std::mutex contextLock;

std::shared_ptr<AsioContextManager>
    AsioContextManager::getContextPointer(const std::string& contextName)
{
    std::shared_ptr<AsioContextManager> contextPtr;
    std::lock_guard<std::mutex> ctxlock(
        contextLock);  // just to ensure that nothing funny happens if you try
    // to get a context while it is being constructed
    auto fnd = contexts.find(contextName);
    if (fnd != contexts.end()) {
        contextPtr = fnd->second;
        return contextPtr;
    }

    contextPtr = std::shared_ptr<AsioContextManager>(new AsioContextManager(contextName));
    contexts.emplace(contextName, contextPtr);
    return contextPtr;
    // if it doesn't find it make a new one with the appropriate name
}

std::shared_ptr<AsioContextManager>
    AsioContextManager::getExistingContextPointer(const std::string& contextName)
{
    std::lock_guard<std::mutex> ctxlock(
        contextLock);  // just to ensure that nothing funny happens if you try
    // to get a context while it is being constructed
    auto fnd = contexts.find(contextName);
    if (fnd != contexts.end()) {
        return fnd->second;
    }

    return nullptr;
}

asio::io_context& AsioContextManager::getContext(const std::string& contextName)
{
    return getContextPointer(contextName)->getBaseContext();
}

asio::io_context& AsioContextManager::getExistingContext(const std::string& contextName)
{
    auto ptr = getExistingContextPointer(contextName);
    if (ptr) {
        return ptr->getBaseContext();
    }
    throw(std::invalid_argument("the context name specified was not available"));
}

void AsioContextManager::closeContext(const std::string& contextName)
{
    std::unique_lock<std::mutex> ctxlock(contextLock);
    auto fnd = contexts.find(contextName);
    //    std::cout << "closing context manager\n";
    if (fnd != contexts.end()) {
        auto ptr = fnd->second;
        contexts.erase(fnd);
        ctxlock.unlock();
        if (ptr->isRunning()) {
            std::lock_guard<std::mutex> nullLock(ptr->runningLoopLock);
            ptr->nullwork.reset();
            ptr->ictx->stop();
            ptr->loopRet.get();
        }
    }
}

void AsioContextManager::setContextToLeakOnDelete(const std::string& contextName)
{
    std::lock_guard<std::mutex> ctxlock(contextLock);
    auto fnd = contexts.find(contextName);
    if (fnd != contexts.end()) {
        fnd->second->leakOnDelete = true;
    }
}
AsioContextManager::~AsioContextManager()
{
    //  std::cout << "deleting context manager\n";

    if (isRunning()) {
        try {
            std::lock_guard<std::mutex> nullLock(runningLoopLock);
            nullwork.reset();
            ictx->stop();
            loopRet.get();
        }
        catch (...) {
        }
    }
    if (leakOnDelete) {
        // yes I am purposefully leaking this PHILIP TOP
        // this capability is needed for some operations on particular OS's with the shared library
        // operations that will crash if this is closed before the library closes which really only
        // happens at program termination
        auto val = ictx.release();
        (void)(val);
    }
}

AsioContextManager::AsioContextManager(const std::string& contextName):
    name(contextName), ictx(std::make_unique<asio::io_context>())
{
}

AsioContextManager::LoopHandle AsioContextManager::runContextLoop(const std::string& contextName)
{
    std::unique_lock<std::mutex> ctxlock(contextLock);
    auto fnd = contexts.find(contextName);
    if (fnd != contexts.end()) {
        auto ptr = fnd->second;
        ctxlock.unlock();
        return ptr->startContextLoop();
    }
    throw(std::invalid_argument("the context name specified was not available"));
}

AsioContextManager::LoopHandle AsioContextManager::startContextLoop()
{
    ++runCounter;  // atomic

    loop_mode exp = loop_mode::stopped;
    if (running.compare_exchange_strong(exp, loop_mode::starting)) {
        auto ptr = shared_from_this();
        std::packaged_task<void()> contextTask(
            [ptr = std::move(ptr)]() { contextProcessingLoop(ptr); });
        //   std::cout << "run Context loop " << runCounter << "\n";
        std::unique_lock<std::mutex> nullLock(runningLoopLock);

        nullwork = std::make_unique<asio::io_context::work>(getBaseContext());
        loopRet = contextTask.get_future();
        nullLock.unlock();
        std::thread contextThread(std::move(contextTask));
        contextThread.detach();
        //  std::cout << "starting context loop thread " << runCounter << "\n";
    } else {
        std::unique_lock<std::mutex> nullLock(runningLoopLock);
        if (getBaseContext().stopped()) {
            // std::cout << "run Context loop already stopped" << runCounter << "\n";
            if (loopRet.valid()) {
                loopRet.get();
            }
            nullLock.unlock();
            exp = loop_mode::stopped;
            if (running.compare_exchange_strong(exp, loop_mode::starting)) {
                auto ptr = shared_from_this();
                std::packaged_task<void()> contextTask(
                    [ptr = std::move(ptr)]() { contextProcessingLoop(ptr); });
                nullLock.lock();
                nullwork = std::make_unique<asio::io_context::work>(getBaseContext());
                loopRet = contextTask.get_future();
                nullLock.unlock();
                std::thread contextThread(std::move(contextTask));
                contextThread.detach();
            }
        }
    }
    return std::make_unique<Servicer>(shared_from_this());
}

void AsioContextManager::haltContextLoop()
{
    if (isRunning()) {
        // std::cout << "context loop halted "<<ptr->runCounter<<"\n";
        if (--runCounter <= 0) {
            std::lock_guard<std::mutex> nullLock(runningLoopLock);
            //    std::cout << "calling halt on context loop \n";

            if (runCounter <= 0) {
                if (nullwork) {
                    terminateLoop = true;
                    nullwork.reset();
                    ictx->stop();
                    int lcnt = 0;
                    while (loopRet.wait_for(std::chrono::milliseconds(0)) ==
                           std::future_status::timeout) {
                        if (lcnt == 0) {
                            std::this_thread::yield();
                        } else {
                            std::this_thread::sleep_for(std::chrono::milliseconds(50));
                            ++lcnt;
                            ictx->stop();
                        }
                    }
                    loopRet.get();
                    ictx->reset();  // prepare for future runs
                    terminateLoop = false;
                }
            }
        }
    } else {
        runCounter.store(0);
    }
}

void contextProcessingLoop(std::shared_ptr<AsioContextManager> ptr)
{
    while ((ptr->runCounter > 0) && (!(ptr->terminateLoop))) {
        auto clk = std::chrono::steady_clock::now();
        try {
            ptr->running.store(AsioContextManager::loop_mode::running);
            ptr->ictx->run();
        }
        catch (const std::system_error& se) {
            auto nclk = std::chrono::steady_clock::now();
            std::cerr << "asio system error in context loop " << se.what() << " ran for "
                      << (nclk - clk).count() / 1000000 << "ms" << std::endl;
        }
        catch (const std::exception& e) {
            auto nclk = std::chrono::steady_clock::now();
            std::cerr << "std::exception in context loop " << e.what() << " ran for "
                      << (nclk - clk).count() / 1000000 << "ms" << std::endl;
        }
        catch (...) {
            std::cout << "caught other error in context loop" << std::endl;
        }
    }

    //   std::cout << "context loop stopped\n";
    ptr->running.store(AsioContextManager::loop_mode::stopped);
}
