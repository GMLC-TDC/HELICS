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

#include "logger.h"

#include "loggerCore.hpp"

#include <iostream>
#include <string>
#include <utility>

namespace helics {
Logger::Logger(): logCore(LoggerManager::getLoggerCore())
{
    coreIndex = logCore->addFileProcessor(
        [this](std::string&& message) { logFunction(std::move(message)); });
}
Logger::Logger(std::shared_ptr<LoggingCore> core): logCore(std::move(core))
{
    coreIndex = logCore->addFileProcessor(
        [this](std::string&& message) { logFunction(std::move(message)); });
}

Logger::~Logger()
{
    logCore->haltOperations(coreIndex);
}
void Logger::openFile(const std::string& file)
{
    std::lock_guard<std::mutex> fLock(fileLock);
    if (outFile.is_open()) {
        outFile.close();
    }
    outFile.open(file.c_str());
    hasFile.store(outFile.is_open());
}

void Logger::closeFile()
{
    std::lock_guard<std::mutex> fLock(fileLock);
    if (outFile.is_open()) {
        outFile.close();
    }
    hasFile.store(false);
}

void Logger::startLogging(int cLevel, int fLevel)
{
    consoleLevel = cLevel;
    fileLevel = fLevel;
    halted.store(false);
}

void Logger::haltLogging()
{
    bool exp = false;
    if (halted.compare_exchange_strong(exp, true)) {
        logCore->addMessage(coreIndex, "!!>close");
    }
}
void Logger::changeLevels(int cLevel, int fLevel)
{
    consoleLevel = cLevel;
    fileLevel = fLevel;
}

void Logger::log(int level, std::string logMessage)
{
    if (!halted) {
        logMessage.push_back((level <= fileLevel) ? '^' : '~');
        logMessage.push_back((level <= consoleLevel) ? '$' : '-');

        logCore->addMessage(coreIndex, std::move(logMessage));
    }
}

void Logger::flush()
{
    logCore->addMessage(coreIndex, "!!>flush");
}
bool Logger::isRunning() const
{
    return (!halted);
}

void Logger::logFunction(std::string&& message)
{
    if (hasFile.load()) {
        std::lock_guard<std::mutex> fLock(fileLock);
        if (message.size() > 3) {
            if (message.compare(0, 3, "!!>") == 0) {
                if (message.compare(3, 5, "flush") == 0) {
                    if (outFile.is_open()) {
                        outFile.flush();
                    }
                }
                if (message.compare(3, 5, "close") == 0) {
                    if (outFile.is_open()) {
                        outFile.close();
                    }
                }
            }
        }

        if (outFile.is_open()) {
            outFile << message << '\n';
        }
    }
}

LoggerNoThread::LoggerNoThread() = default;

LoggerNoThread::LoggerNoThread(const std::shared_ptr<LoggingCore>& /*core*/) {}

void LoggerNoThread::openFile(const std::string& file)
{
    outFile.open(file.c_str());
}

void LoggerNoThread::closeFile()
{
    outFile.close();
}

void LoggerNoThread::startLogging(int cLevel, int fLevel)
{
    consoleLevel = cLevel;
    fileLevel = fLevel;
}

void LoggerNoThread::changeLevels(int cLevel, int fLevel)
{
    consoleLevel = cLevel;
    fileLevel = fLevel;
}

void LoggerNoThread::log(int level, const std::string& logMessage)
{
    if (level < consoleLevel) {
        std::cout << logMessage << '\n';
    }
    if (level < fileLevel) {
        if (outFile.is_open()) {
            outFile << logMessage << '\n';
        }
    }
}

void LoggerNoThread::flush()
{
    if (outFile.is_open()) {
        outFile.flush();
    }
    std::cout.flush();
}

bool LoggerNoThread::isRunning() const
{
    return true;
}

}  // namespace helics
