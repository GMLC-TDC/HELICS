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

#include "logger.h"
#include <iostream>

namespace helics
{
logger::~logger ()
{
    if (loggingThread.joinable ())
    {
        if (!halted)
        {
            loggingQueue.emplace ("!!>close");
        }
        loggingThread.join ();
    }
}
void logger::openFile (const std::string &file)
{
    if (loggingThread.joinable ())
    {
        loggingQueue.emplace ("!!>close");
        loggingThread.join ();
        outFile.open (file.c_str ());
        startLogging ();
    }
    else
    {
        outFile.open (file.c_str ());
    }
}

void logger::startLogging (int cLevel, int fLevel)
{
    if (!halted)
    {
        if (loggingThread.joinable ())
        {
            return;
        }
    }
    else
    {
        if (loggingThread.joinable ())
        {
            loggingThread.join ();
        }
    }

    consoleLevel = cLevel;
    fileLevel = fLevel;
    loggingThread = std::thread (&logger::loggerLoop, this);
}

void logger::haltLogging ()
{
    bool exp = false;
    if (halted.compare_exchange_strong (exp, true))
    {
        loggingQueue.emplace ("!!>close");
    }
}
void logger::changeLevels (int cLevel, int fLevel)
{
    consoleLevel = cLevel;
    fileLevel = fLevel;
}

void logger::log (int level, std::string logMessage)
{
    if (!halted)
    {
        logMessage.push_back ((level <= consoleLevel) ? 'c' : 'n');
        logMessage.push_back ((level <= fileLevel) ? 'f' : 'n');
        loggingQueue.emplace (std::move (logMessage));
    }
}

void logger::flush () { loggingQueue.emplace ("!!>flush"); }
bool logger::isRunning () const { return (!halted) && (loggingThread.joinable ()); }

void logger::loggerLoop ()
{
    halted = false;
    while (true)
    {
        auto msg = loggingQueue.pop ();

        if (msg.size () < 3)
        {
            continue;
        }
        if (msg.compare (0, 3, "!!>") == 0)
        {
            if (msg.compare (3, 5, "close") == 0)
            {
                break;  // break the loop
            }
            if (msg.compare (3, 5, "flush") == 0)
            {
                if (outFile.is_open ())
                {
                    outFile.flush ();
                }
                std::cout.flush ();
                continue;
            }
        }
        // if a the file should be written there will be a 'f' at the end
        auto f = msg.back ();
        msg.pop_back ();
        // if a the console should be written there will be a 'c' at the end
        auto c = msg.back ();
        msg.pop_back ();
        if (f == 'f')
        {
            if (outFile.is_open ())
            {
                outFile << msg << '\n';
            }
        }
        if (c == 'c')
        {
            std::cout << msg << '\n';
        }
    }
}

loggerNoThread::loggerNoThread () = default;

void loggerNoThread::openFile (const std::string &file) { outFile.open (file.c_str ()); }
void loggerNoThread::startLogging (int cLevel, int fLevel)
{
    consoleLevel = cLevel;
    fileLevel = fLevel;
}

void loggerNoThread::changeLevels (int cLevel, int fLevel)
{
    consoleLevel = cLevel;
    fileLevel = fLevel;
}

void loggerNoThread::log (int level, const std::string &logMessage)
{
    if (level < consoleLevel)
    {
        std::cout << logMessage << '\n';
    }
    if (level < fileLevel)
    {
        if (outFile.is_open ())
        {
            outFile << logMessage << '\n';
        }
    }
}

void loggerNoThread::flush ()
{
    if (outFile.is_open ())
    {
        outFile.flush ();
    }
    std::cout.flush ();
}

bool loggerNoThread::isRunning () const { return true; }

/** a storage system for the available logger objects allowing references by name to the core
 */
std::map<std::string, std::shared_ptr<loggerManager>> loggerManager::loggers;

/** we expect operations on core object that modify the map to be rare but we absolutely need them to be thread
safe so we are going to use a lock that is entirely controlled by this file*/
static std::mutex loggerLock;

std::shared_ptr<loggerManager> loggerManager::getLoggerManager (const std::string &loggerName)
{
    std::lock_guard<std::mutex> loglock (
      loggerLock);  // just to ensure that nothing funny happens if you try to get a context
                    // while it is being constructed
    auto fnd = loggers.find (loggerName);
    if (fnd != loggers.end ())
    {
        return fnd->second;
    }

    auto newLogger = std::shared_ptr<loggerManager> (new loggerManager (loggerName));
    loggers.emplace (loggerName, newLogger);
    return newLogger;
    // if it doesn't make a new one with the appropriate name
}

std::shared_ptr<logger> loggerManager::getLogger (const std::string &loggerName)
{
    return getLoggerManager (loggerName)->loggingControl;
}

void loggerManager::closeLogger (const std::string &loggerName)
{
    std::lock_guard<std::mutex> loglock (loggerLock);
    auto fnd = loggers.find (loggerName);
    if (fnd != loggers.end ())
    {
        loggers.erase (fnd);
    }
}

loggerManager::~loggerManager () {}

loggerManager::loggerManager (const std::string &loggerName) : name (loggerName)
{
    loggingControl = std::make_shared<logger> ();
}
}  // namespace helics