/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
/*
 * LLNS Copyright Start
 * Copyright (c) 2014-2018, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#include "loggerCore.hpp"

#include <iostream>
#include <string>
#include <tuple>
#include <utility>

DECLARE_TRIPLINE()

namespace helics {
std::atomic<bool> LoggingCore::fastShutdown{false};

LoggingCore::LoggingCore()
{
    loggingThread = std::thread(&LoggingCore::processingLoop, this);
}

LoggingCore::~LoggingCore()
{
    try {
        if (fastShutdown) {
            if (!tripDetector.isTripped()) {
                loggingQueue.emplace(-1, "!!>close");
            }
        } else {
            loggingQueue.emplace(-1, "!!>close");
        }

        loggingThread.join();
    }
    catch (...) {
    }
}

void LoggingCore::addMessage(std::string&& message)
{
    loggingQueue.emplace(-1, std::move(message));
}

void LoggingCore::addMessage(const std::string& message)
{
    loggingQueue.emplace(-1, message);
}

void LoggingCore::addMessage(int index, std::string&& message)
{
    loggingQueue.emplace(index, std::move(message));
}

void LoggingCore::addMessage(int index, const std::string& message)
{
    loggingQueue.emplace(index, message);
}
int LoggingCore::addFileProcessor(std::function<void(std::string&& message)> newFunction)
{
    std::lock_guard<std::mutex> fLock(functionLock);
    for (int ii = 0; ii < static_cast<int>(functions.size()); ++ii) {
        if (functions[ii]) {
            continue;
        }
        functions[ii] = std::move(newFunction);
        return ii;
    }
    functions.push_back(std::move(newFunction));
    return static_cast<int>(functions.size()) - 1;
}

void LoggingCore::setFastShutdown()
{
    fastShutdown.store(true);
}

void LoggingCore::haltOperations(int loggerIndex)
{
    std::lock_guard<std::mutex> fLock(functionLock);
    if (loggerIndex < static_cast<int>(functions.size())) {
        functions[loggerIndex] = nullptr;
    }
}

/** update a callback for a particular instance*/
void LoggingCore::updateProcessingFunction(int index,
                                           std::function<void(std::string&& message)> newFunction)
{
    std::lock_guard<std::mutex> fLock(functionLock);
    if (index < static_cast<int>(functions.size())) {
        functions[index] = std::move(newFunction);
    }
}

void LoggingCore::processingLoop()
{
    int index;
    std::string msg;
    while (true) {
        std::tie(index, msg) = loggingQueue.pop();

        if (msg.size() > 3) {
            if (msg.compare(0, 3, "!!>") == 0) {
                if (msg.compare(3, 5, "flush") ==
                    0) {  // any flush command we need flush the console, we may also need to flush
                          // a particular file
                    std::cout.flush();
                    if (index == -1) {
                        continue;
                    }
                    msg.push_back('^');
                }
                if (msg.compare(3, 5, "close") == 0) {
                    if (index == -1) {
                        break;  // break the loop
                    }
                    msg.push_back('^');
                }
            }
        }
        // if a the callback should be called there will be a '^' at the end
        bool nosymbol = true;
        auto f = msg.back();
        if ((f == '^') || (f == '~')) {
            nosymbol = false;
            msg.pop_back();
        }

        // if a the console should be written there will be a '$' at the end
        auto c = msg.back();
        if ((c == '$') || (c == '-')) {
            nosymbol = false;
            msg.pop_back();
        }
        // in case they were written out of order
        if ((f == '$') || (f == '-')) {
            f = msg.back();
            if ((f == '^') || (f == '~')) {
                msg.pop_back();
            }
        }
        if ((c == '$') || (nosymbol)) {
            std::cout << msg << '\n';
        }
        if (index >= 0) {
            if ((f == '^') || (nosymbol)) {
                std::lock_guard<std::mutex> fLock(functionLock);
                if (index < static_cast<int>(functions.size())) {
                    if (functions[index]) {
                        functions[index](std::move(msg));
                    }
                }
            }
        }
    }
}

/** a storage system for the available Logger objects allowing references by name to the core
 */
std::map<std::string, std::shared_ptr<LoggerManager>> LoggerManager::loggers;

/** we expect operations on core object that modify the map to be rare but we absolutely need them
to be thread safe so we are going to use a lock that is entirely controlled by this file*/
static std::mutex loggerLock;

std::shared_ptr<LoggerManager> LoggerManager::getLoggerManager(const std::string& loggerName)
{
    std::lock_guard<std::mutex> loglock(
        loggerLock);  // just to ensure that nothing funny happens if you try to
    // get a context while it is being constructed
    auto fnd = loggers.find(loggerName);
    if (fnd != loggers.end()) {
        return fnd->second;
    }

    auto newLogger = std::shared_ptr<LoggerManager>(new LoggerManager(loggerName));
    loggers.emplace(loggerName, newLogger);
    return newLogger;
    // if it doesn't make a new one with the appropriate name
}

std::shared_ptr<LoggingCore> LoggerManager::getLoggerCore(const std::string& loggerName)
{
    return getLoggerManager(loggerName)->loggingControl;
}

void LoggerManager::closeLogger(const std::string& loggerName)
{
    std::lock_guard<std::mutex> loglock(loggerLock);
    auto fnd = loggers.find(loggerName);
    if (fnd != loggers.end()) {
        loggers.erase(fnd);
    }
}

void LoggerManager::logMessage(std::string message)
{
    std::lock_guard<std::mutex> loglock(loggerLock);
    auto fnd = loggers.find(std::string());
    if (fnd != loggers.end()) {
        if (fnd->second->loggingControl) {
            fnd->second->loggingControl->addMessage(std::move(message));
            return;
        }
    }
    // if there is no default Logger just dump it to the console
    std::cout << message << std::endl;
}

LoggerManager::~LoggerManager() = default;

LoggerManager::LoggerManager(const std::string& loggerName):
    name(loggerName), loggingControl(std::make_shared<LoggingCore>())
{
}
}  // namespace helics
