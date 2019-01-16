/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
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

#pragma once

#include "BlockingQueue.hpp"
#include "TripWire.hpp"
#include <atomic>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <thread>

namespace helics
{
/** class to manage a single thread for all logging*/
class LoggingCore
{
  private:
    static std::atomic<bool> fastShutdown;  // set to true to enable a fast shutdown
    std::thread loggingThread;  //!< the thread object containing the thread running the actual Logger
    std::vector<std::function<void(std::string &&message)>> functions;  //!< container for the functions
    std::mutex functionLock;  //!< lock for updating the functions
    BlockingQueue<std::pair<int32_t, std::string>>
      loggingQueue;  //!< the actual queue containing the strings to log
    tripwire::TripWireDetector tripDetector;

  public:
    /** default constructor*/
    LoggingCore ();
    /** destructor*/
    ~LoggingCore ();
    /** add a message for the LoggingCore or just general console print
     */
    void addMessage (const std::string &message);
    /** move a message for the LoggingCore or just general console print
     */
    void addMessage (std::string &&message);
    /** add a message for a specific Logger
    @param index the index of the function callback to use
    @param message the message to send
    */
    void addMessage (int index, const std::string &message);
    /** add a message for a specific Logger
    @param index the index of the function callback to use
    @param message the message to send
    */
    void addMessage (int index, std::string &&message);
    /** add a file processing callback (not just files)
    @param newFunction the callback to call on receipt of a message
    */
    int addFileProcessor (std::function<void(std::string &&message)> newFunction);
    /** remove a function callback*/
    void haltOperations (int loggerIndex);
    /** update a callback for a particular instance*/
    void updateProcessingFunction (int index, std::function<void(std::string &&message)> newFunction);
    /** enable a fast shutdown in situations where a thread may be force-ably terminated*/
    static void setFastShutdown ();

  private: /** primary processing loop*/
    void processingLoop ();
};

/** class implementing a thread safe Logger
@details the Logger uses a queuing mechanism and condition variable to store messages to a queue and print/display
them in a single thread allowing for asynchronous logging
*/
class Logger
{
  private:
    std::atomic<bool> halted{true};  //!< indicator that the Logger was halted
    std::mutex fileLock;  //!< mutex to protect the file itself
    std::ofstream outFile;  //!< the stream to write the log messages
    std::shared_ptr<LoggingCore> logCore;  //!< pointer to the core operation
    int coreIndex = -1;  //!< index into the core
    std::atomic<int> consoleLevel{100};  //!< level below which we need to print to the console
    std::atomic<int> fileLevel{100};  //!< level below which we need to print to a file
  public:
    /** default constructor*/
    Logger ();
    /** construct and link to the specified logging Core*/
    explicit Logger (std::shared_ptr<LoggingCore> core);
    /**destructor*/
    ~Logger ();
    /** open a file to write the log messages
    @param file the name of the file to write messages to*/
    void openFile (const std::string &file);
    /** function to start the logging thread
    @param cLevel the console print level
    @param fLevel the file print level  messages coming in below these levels will be printed*/
    void startLogging (int cLevel, int fLevel);
    /** overload of @see startLogging with unspecified logging levels*/
    void startLogging () { startLogging (consoleLevel, fileLevel); }
    /** stop logging for a time messages received while halted are ignored*/
    void haltLogging ();
    /** log a message at a particular level
    @param level the level of the message
    @param logMessage the actual message to log
    */
    void log (int level, std::string logMessage);
    /** message to log without regard for levels*
    @param logMessage the message to log
    */
    void log (std::string logMessage) { log (-100000, std::move (logMessage)); }
    /** flush the log queue*/
    void flush ();
    /** check if the Logger is running*/
    bool isRunning () const;
    /** alter the printing levels
    @param cLevel the level to print to the console
    @param fLevel the level to print to the file if it is open*/
    void changeLevels (int cLevel, int fLevel);

  private:
    /** actual loop function to run the Logger*/
    void logFunction (std::string &&message);
};

/** logging class that handle the logs immediately with no threading or synchronization*/
class LoggerNoThread
{
  private:
    std::ofstream outFile;  //!< the file stream to write the log messages to
  public:
    int consoleLevel = 100;  //!< level below which we need to print to the console
    int fileLevel = 100;  //!< level below which we need to print to a file
  public:
    /** default constructor*/
    LoggerNoThread ();
    /**this does nothing with the argument since it is not threaded here to match the API of Logger*/
    explicit LoggerNoThread (const std::shared_ptr<LoggingCore> &core);
    /** open a file to write the log messages
    @param file the name of the file to write messages to*/
    void openFile (const std::string &file);
    /** function to start the logging thread
    @param cLevel the console print level
    @param fLevel the file print level  messages coming in below these levels will be printed*/
    void startLogging (int cLevel, int fLevel);
    /** overload of /ref startLogging with unspecified logging levels*/
    void startLogging () { startLogging (consoleLevel, fileLevel); }
    // NOTE:: the interface for log in the noThreadLogging is slightly different
    // due to the threaded Logger making use of move semantics which isn't that useful in the noThreadLogger
    /** log a message at a particular level
    @param level the level of the message
    @param logMessage the actual message to log
    */
    void log (int level, const std::string &logMessage);
    /** message to log without regard for levels*
    @param logMessage the message to log
    */
    void log (const std::string &logMessage) { log (-100000, logMessage); }
    /** check if the logging thread is running*/
    bool isRunning () const;
    /** flush the log queue*/
    void flush ();
    /** alter the printing levels
    @param cLevel the level to print to the console
    @param fLevel the level to print to the file if it is open*/
    void changeLevels (int cLevel, int fLevel);
};

/** class defining a singleton manager for all logging use*/
class LoggerManager
{
  private:
    static std::map<std::string, std::shared_ptr<LoggerManager>>
      loggers;  //!< container for pointers to all the available contexts
    std::string name;  //!< context name
    std::shared_ptr<LoggingCore> loggingControl;  //!< pointer to the actual Logger
    explicit LoggerManager (const std::string &loggerName);

  public:
    /** get a pointer to a logging manager so it cannot go out of scope*/
    static std::shared_ptr<LoggerManager> getLoggerManager (const std::string &loggerName = "");
    /** get a pointer to a logging core*/
    static std::shared_ptr<LoggingCore> getLoggerCore (const std::string &loggerName = "");
    /** close the named Logger
    @details prevents the Logger from being retrieved through this class
    but does not necessarily destroy the Logger*/
    static void closeLogger (const std::string &loggerName = "");
    /** sends a message to the default Logger*/
    static void logMessage (std::string message);

    /*destructor*/
    virtual ~LoggerManager ();
    /** get the name of the logger*/
    const std::string &getName () const { return name; }
};
}  // namespace helics
