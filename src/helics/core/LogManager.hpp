/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

/**
@file
helper class for managing logging information
*/

#include "../common/LogBuffer.hpp"
#include "../helics_enums.h"
#include "FederateIdExtra.hpp"

#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace spdlog {
class logger;
}

namespace helics {
class LogBuffer;
class helicsCLI11App;
class LogBuffer;
class ActionMessage;

class LogManager {
  private:
    std::string logIdentifier;
    std::atomic<int32_t> maxLogLevel{HELICS_LOG_LEVEL_WARNING};
    /// the logging level for console display
    int32_t consoleLogLevel{HELICS_LOG_LEVEL_WARNING};
    /// the logging level for logging to a file
    int32_t fileLogLevel{HELICS_LOG_LEVEL_WARNING};
    /// the logging level and targets for a remote logging message
    std::vector<std::pair<GlobalFederateId, std::int32_t>> remoteTargets;
    /// default logging object to use if the logging callback is not specified
    std::shared_ptr<spdlog::logger> consoleLogger;
    /// default logging object to use if the logging callback is not specified
    std::shared_ptr<spdlog::logger> fileLogger;
    std::atomic<bool> initialized{false};
    mutable LogBuffer mLogBuffer;  //!< object for buffering a set of log messages
    /** a logging function for logging or printing messages*/
    std::function<void(int, std::string_view, std::string_view)> loggerFunction;
    std::function<void(ActionMessage&& mm)> mTransmit;
    std::string logFile;  //!< the file to log messages to

  public:
    /// force the log to flush after every message
    std::atomic<bool> forceLoggingFlush{false};

    ~LogManager();

    void initializeLogging(const std::string& identifier);

    /** send a message to the logging system
@return true if the message was actually logged
*/
    bool sendToLogger(int logLevel,
                      std::string_view header,
                      std::string_view message,
                      bool disableRemote = false) const;
    /** add logging Command line options to a CLI App*/
    void addLoggingCLI(std::shared_ptr<helicsCLI11App>& app);

    /** set the logging level */
    void setLogLevel(int32_t level);
    /** set the logging levels
    @param consoleLevel the logging level for the console display
    @param fileLevel the logging level for the log file
    */
    void setLogLevels(int32_t consoleLevel, int32_t fileLevel);
    /** flush the loggers*/
    void logFlush();

    /** set the logging callback function
@param logFunction a function with a signature of void(int level, std::string_view identifier,
std::string_view message) the function takes a level indicating the logging level string with
the source name and a string with the message
*/
    void setLoggerFunction(
        std::function<void(int level, std::string_view identifier, std::string_view message)>
            logFunction);

    void setTransmitCallback(std::function<void(ActionMessage&& mm)> transmit)

    {
        mTransmit = std::move(transmit);
    }
    int getMaxLevel() const { return maxLogLevel.load(); }
    int getFileLevel() const { return fileLogLevel; }
    int getConsoleLevel() const { return consoleLogLevel; }
    void setLoggingFile(std::string_view lfile, const std::string& identifier);
    LogBuffer& getLogBuffer() { return mLogBuffer; }
    void updateRemote(GlobalFederateId destination, int level);

  private:
    void updateMaxLogLevel();
};

}  // namespace helics
