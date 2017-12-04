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
logger::logger ()
{
    logCore = loggerManager::getLoggerCore ();
    coreIndex = logCore->addFileProcessor ([this](std::string &&message) { logFunction (std::move (message)); });
}
logger::logger (std::shared_ptr<loggingCore> core) : logCore (std::move (core))
{
    coreIndex = logCore->addFileProcessor ([this](std::string &&message) { logFunction (std::move (message)); });
}

logger::~logger () { logCore->haltOperations (coreIndex); }
void logger::openFile (const std::string &file)
{
    std::lock_guard<std::mutex> fLock (fileLock);
    if (outFile.is_open ())
    {
        outFile.close ();
    }
    outFile.open (file.c_str ());
}

void logger::startLogging (int cLevel, int fLevel)
{
    consoleLevel = cLevel;
    fileLevel = fLevel;
    halted.store (false);
}

void logger::haltLogging ()
{
    bool exp = false;
    if (halted.compare_exchange_strong (exp, true))
    {
        logCore->addMessage (coreIndex, "!!>close");
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
        logMessage.push_back ((level <= fileLevel) ? '^' : '~');
        logMessage.push_back ((level <= consoleLevel) ? '$' : '-');

        logCore->addMessage (coreIndex, std::move (logMessage));
    }
}

void logger::flush () { logCore->addMessage (coreIndex, "!!>flush"); }
bool logger::isRunning () const { return (!halted); }

void logger::logFunction (std::string &&message)
{
    std::lock_guard<std::mutex> fLock (fileLock);
    if (message.size () > 3)
    {
        if (message.compare (0, 3, "!!>") == 0)
        {
            if (message.compare (3, 5, "flush") == 0)
            {
                if (outFile.is_open ())
                {
                    outFile.flush ();
                }
            }
        }
    }

    if (outFile.is_open ())
    {
        outFile << message << '\n';
    }
}

loggerNoThread::loggerNoThread () = default;

loggerNoThread::loggerNoThread (std::shared_ptr<loggingCore> /*core*/) {}

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

loggingCore::loggingCore () { loggingThread = std::thread (&loggingCore::processingLoop, this); }

loggingCore::~loggingCore ()
{
    loggingQueue.emplace (-1, "!!>close");
    loggingThread.join ();
}

void loggingCore::addMessage (std::string &&message) { loggingQueue.emplace (-1, std::move (message)); }

void loggingCore::addMessage (const std::string &message) { loggingQueue.emplace (-1, message); }

void loggingCore::addMessage (int index, std::string &&message)
{
    loggingQueue.emplace (index, std::move (message));
}

void loggingCore::addMessage (int index, const std::string &message) { loggingQueue.emplace (index, message); }
int loggingCore::addFileProcessor (std::function<void(std::string &&message)> newFunction)
{
    std::lock_guard<std::mutex> fLock (functionLock);
    functions.push_back (std::move (newFunction));
    return static_cast<int> (functions.size ()) - 1;
}

void loggingCore::haltOperations (int loggerIndex)
{
    std::lock_guard<std::mutex> fLock (functionLock);
    if (loggerIndex < static_cast<int> (functions.size ()))
    {
        functions[loggerIndex] = nullptr;
    }
}

/** update a callback for a particular instance*/
void loggingCore::updateProcessingFunction (int index, std::function<void(std::string &&message)> newFunction)
{
    std::lock_guard<std::mutex> fLock (functionLock);
    if (index < static_cast<int> (functions.size ()))
    {
        functions[index] = std::move (newFunction);
    }
}

void loggingCore::processingLoop ()
{
    int index;
    std::string msg;
    while (true)
    {
        std::tie (index, msg) = loggingQueue.pop ();

        if (msg.size () > 3)
        {
            if (msg.compare (0, 3, "!!>") == 0)
            {
                if (msg.compare (3, 5, "flush") == 0)
                {  // any flush command we need flush the console, we may also need to flush a particular file
                    std::cout.flush ();
                    if (index == -1)
                    {
                        continue;
                    }
                }
                if (msg.compare (3, 5, "close") == 0)
                {
                    if (index == -1)
                    {
                        break;  // break the loop
                    }
                    else
                    {
                        msg.push_back ('$');
                    }
                }
            }
        }
        // if a the callback should be called there will be a '^' at the end
        bool nosymbol = true;
        auto f = msg.back ();
        if ((f == '^') || (f == '~'))
        {
            nosymbol = false;
            msg.pop_back ();
        }

        // if a the console should be written there will be a '$' at the end
        auto c = msg.back ();
        if ((c == '$') || (c == '-'))
        {
            nosymbol = false;
            msg.pop_back ();
        }
        // in case they were written out of order
        if ((f == '$') || (f == '-'))
        {
            f = msg.back ();
            if ((f == '^') || (f == '~'))
            {
                msg.pop_back ();
            }
        }
        if ((c == '$') || (nosymbol))
        {
            std::cout << msg << '\n';
        }
        if (index >= 0)
        {
            if ((f == '^') || (nosymbol))
            {
                std::lock_guard<std::mutex> fLock (functionLock);
                if (index < static_cast<int> (functions.size ()))
                {
                    if (functions[index])
                    {
                        functions[index](std::move (msg));
                    }
                }
            }
        }
    }
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

std::shared_ptr<loggingCore> loggerManager::getLoggerCore (const std::string &loggerName)
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

void loggerManager::logMessage (const std::string &message)
{
    std::lock_guard<std::mutex> loglock (loggerLock);
    auto fnd = loggers.find ("");
    if (fnd != loggers.end ())
    {
        if (fnd->second->loggingControl)
        {
            fnd->second->loggingControl->addMessage (message);
            return;
        }
    }
    // if there is no default logger just dump it to the console
    std::cout << message << std::endl;
}

loggerManager::~loggerManager () = default;

loggerManager::loggerManager (const std::string &loggerName) : name (loggerName)
{
    loggingControl = std::make_shared<loggingCore> ();
}
}  // namespace helics