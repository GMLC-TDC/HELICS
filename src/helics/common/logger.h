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

#pragma once

#include <atomic>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

namespace helics {
class LoggingCore;

constexpr int always_log = -100000;  //!< level that will always log
constexpr int log_everything = 100;  //!< level that will log everything

/** class implementing a thread safe Logger
@details the Logger uses a queuing mechanism and condition variable to store messages to a queue and
print/display them in a single thread allowing for asynchronous logging
*/
class Logger {
  private:
    std::atomic<bool> halted{true};  //!< indicator that the Logger was halted
    std::mutex fileLock;  //!< mutex to protect the file itself
    std::atomic<bool> hasFile{false};  //!< flag indicating the logger has a file
    std::ofstream outFile;  //!< the stream to write the log messages
    std::shared_ptr<LoggingCore> logCore;  //!< pointer to the core operation
    int coreIndex = -1;  //!< index into the core
    std::atomic<int> consoleLevel{
        log_everything};  //!< level below which we need to print to the console
    std::atomic<int> fileLevel{log_everything};  //!< level below which we need to print to a file
  public:
    /** default constructor*/
    Logger();
    /** construct and link to the specified logging Core*/
    explicit Logger(std::shared_ptr<LoggingCore> core);
    /**destructor*/
    ~Logger();
    /** open a file to write the log messages
    @param file the name of the file to write messages to*/
    void openFile(const std::string& file);
    /** close the current file for logging
     */
    void closeFile();
    /** function to start the logging thread
    @param cLevel the console print level
    @param fLevel the file print level  messages coming in below these levels will be printed*/
    void startLogging(int cLevel, int fLevel);
    /** overload of @see startLogging with unspecified logging levels*/
    void startLogging() { startLogging(consoleLevel, fileLevel); }
    /** stop logging for a time, messages received while halted are ignored*/
    void haltLogging();
    /** log a message at a particular level
    @param level the level of the message
    @param logMessage the actual message to log
    */
    void log(int level, std::string logMessage);
    /** message to log without regard for levels*
    @param logMessage the message to log
    */
    void log(std::string logMessage) { log(always_log, std::move(logMessage)); }
    /** flush the log queue*/
    void flush();
    /** check if the Logger is running*/
    bool isRunning() const;
    /** alter the printing levels
    @param cLevel the level to print to the console
    @param fLevel the level to print to the file if it is open*/
    void changeLevels(int cLevel, int fLevel);

  private:
    /** actual loop function to run the Logger*/
    void logFunction(std::string&& message);
};

/** logging class that handle the logs immediately with no threading or synchronization*/
class LoggerNoThread {
  private:
    std::ofstream outFile;  //!< the file stream to write the log messages to
  public:
    int consoleLevel = log_everything;  //!< level below which we need to print to the console
    int fileLevel = log_everything;  //!< level below which we need to print to a file
  public:
    /** default constructor*/
    LoggerNoThread();
    /**this does nothing with the argument since it is not threaded here to match the API of
     * Logger*/
    explicit LoggerNoThread(const std::shared_ptr<LoggingCore>& core);
    /** open a file to write the log messages
    @param file the name of the file to write messages to*/
    void openFile(const std::string& file);
    /** close the file for logging*/
    void closeFile();
    /** function to start the logging thread
    @param cLevel the console print level
    @param fLevel the file print level  messages coming in below these levels will be printed*/
    void startLogging(int cLevel, int fLevel);
    /** overload of /ref startLogging with unspecified logging levels*/
    void startLogging() { startLogging(consoleLevel, fileLevel); }
    // NOTE:: the interface for log in the noThreadLogging is slightly different
    // due to the threaded Logger making use of move semantics which isn't that useful in the
    // noThreadLogger
    /** log a message at a particular level
    @param level the level of the message
    @param logMessage the actual message to log
    */
    void log(int level, const std::string& logMessage);
    /** message to log without regard for levels*
    @param logMessage the message to log
    */
    void log(const std::string& logMessage) { log(always_log, logMessage); }
    /** check if the logging thread is running*/
    bool isRunning() const;
    /** flush the log queue*/
    void flush();
    /** alter the printing levels
    @param cLevel the level to print to the console
    @param fLevel the level to print to the file if it is open*/
    void changeLevels(int cLevel, int fLevel);
};
}  // namespace helics
