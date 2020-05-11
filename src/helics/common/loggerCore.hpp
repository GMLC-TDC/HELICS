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

#include "gmlc/concurrency/TripWire.hpp"
#include "gmlc/containers/BlockingQueue.hpp"

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace helics {
/** class to manage a single thread for all logging*/
class LoggingCore {
  private:
    static std::atomic<bool> fastShutdown;  // set to true to enable a fast shutdown
    std::thread
        loggingThread;  //!< the thread object containing the thread running the actual Logger
    std::vector<std::function<void(std::string&& message)>>
        functions;  //!< container for the functions
    std::mutex functionLock;  //!< lock for updating the functions
    gmlc::containers::BlockingQueue<std::pair<int32_t, std::string>>
        loggingQueue;  //!< the actual queue containing the strings to log
    gmlc::concurrency::TripWireDetector tripDetector;

  public:
    /** default constructor*/
    LoggingCore();
    /** destructor*/
    ~LoggingCore();
    /** add a message for the LoggingCore or just general console print
     */
    void addMessage(const std::string& message);
    /** move a message for the LoggingCore or just general console print
     */
    void addMessage(std::string&& message);
    /** add a message for a specific Logger
    @param index the index of the function callback to use
    @param message the message to send
    */
    void addMessage(int index, const std::string& message);
    /** add a message for a specific Logger
    @param index the index of the function callback to use
    @param message the message to send
    */
    void addMessage(int index, std::string&& message);
    /** add a file processing callback (not just files)
    @param newFunction the callback to call on receipt of a message
    */
    int addFileProcessor(std::function<void(std::string&& message)> newFunction);
    /** remove a function callback*/
    void haltOperations(int loggerIndex);
    /** update a callback for a particular instance*/
    void updateProcessingFunction(int index,
                                  std::function<void(std::string&& message)> newFunction);
    /** enable a fast shutdown in situations where a thread may be force-ably terminated*/
    static void setFastShutdown();

  private: /** primary processing loop*/
    void processingLoop();
};

/** class defining a singleton manager for all logging use*/
class LoggerManager {
  private:
    static std::map<std::string, std::shared_ptr<LoggerManager>>
        loggers;  //!< container for pointers to all the available contexts
    std::string name;  //!< context name
    std::shared_ptr<LoggingCore> loggingControl;  //!< pointer to the actual Logger
    explicit LoggerManager(const std::string& loggerName);

  public:
    /** get a pointer to a logging manager so it cannot go out of scope*/
    static std::shared_ptr<LoggerManager> getLoggerManager(const std::string& loggerName = "");
    /** get a pointer to a logging core*/
    static std::shared_ptr<LoggingCore> getLoggerCore(const std::string& loggerName = "");
    /** close the named Logger
    @details prevents the Logger from being retrieved through this class
    but does not necessarily destroy the Logger*/
    static void closeLogger(const std::string& loggerName = "");
    /** sends a message to the default Logger*/
    static void logMessage(std::string message);

    /*destructor*/
    virtual ~LoggerManager();
    /** get the name of the logger*/
    const std::string& getName() const { return name; }
};
}  // namespace helics
