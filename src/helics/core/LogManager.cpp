/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "LogManager.hpp"

#include "../common/LogBuffer.hpp"
#include "../common/logging.hpp"
#include "helics/core/helicsCLI11JsonConfig.hpp"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#if !defined(WIN32) && !defined(__MINGW32__) && !defined(CYGWIN) && !defined(_WIN32)
#    include "spdlog/sinks/syslog_sink.h"
#endif

#include "ActionMessage.hpp"

#include <iostream>
#include <string>
#include <utility>

namespace helics {
LogManager::~LogManager()
{
    consoleLogger.reset();
    if (fileLogger) {
        spdlog::drop(logIdentifier);
    }
}
void LogManager::initializeLogging(const std::string& identifier)
{
    bool expected{false};
    if (initialized.compare_exchange_strong(expected, true)) {
        logIdentifier = identifier;
        try {
            consoleLogger = spdlog::get("console");
            if (!consoleLogger) {
                try {
                    consoleLogger = spdlog::stdout_color_mt("console");
                    consoleLogger->flush_on(spdlog::level::info);
                    consoleLogger->set_level(spdlog::level::trace);
                }
                catch (const spdlog::spdlog_ex&) {
                    consoleLogger = spdlog::get("console");
                }
            }
            if (logFile == "syslog") {
#if !defined(WIN32) && !defined(__MINGW32__) && !defined(CYGWIN) && !defined(_WIN32)
                fileLogger = spdlog::syslog_logger_mt("syslog", identifier);
#endif
            } else if (!logFile.empty()) {
                fileLogger = spdlog::basic_logger_mt(identifier, logFile);
            }
            if (fileLogger) {
                fileLogger->flush_on(spdlog::level::info);
                fileLogger->set_level(spdlog::level::trace);
            }
        }
        catch (const spdlog::spdlog_ex& ex) {
            std::cerr << "Log init failed in " << identifier << " : " << ex.what() << std::endl;
        }
    }
}

static spdlog::level::level_enum getSpdLogLevel(int helicsLogLevel)
{
    if (helicsLogLevel >= LogLevels::TRACE || helicsLogLevel == LogLevels::DUMPLOG) {
        return spdlog::level::trace;
    }
    if (helicsLogLevel >= LogLevels::TIMING) {
        return spdlog::level::debug;
    }
    if (helicsLogLevel >= LogLevels::SUMMARY) {
        return spdlog::level::info;
    }
    if (helicsLogLevel >= LogLevels::WARNING) {
        return spdlog::level::warn;
    }
    if (helicsLogLevel == LogLevels::PROFILING) {
        return spdlog::level::info;
    }
    if (helicsLogLevel >= LogLevels::ERROR_LEVEL) {
        return spdlog::level::err;
    }
    return spdlog::level::critical;
}

bool LogManager::sendToLogger(int logLevel,
                              std::string_view header,
                              std::string_view message,
                              bool fromRemote) const
{
    bool alwaysLog{fromRemote};
    if (logLevel > LogLevels::FED - 100) {
        logLevel -= static_cast<int>(LogLevels::FED);
        alwaysLog = true;
    }

    if (logLevel > maxLogLevel && !alwaysLog) {
        // check the logging level
        return true;
    }

    mLogBuffer.push(logLevel, header, message);
    if (!fromRemote) {
        for (const auto& rl : remoteTargets) {
            if (rl.second >= logLevel && rl.first.isValid()) {
                if (mTransmit) {
                    ActionMessage remMess(CMD_REMOTE_LOG);
                    remMess.dest_id = rl.first;
                    remMess.setString(0, header);
                    remMess.payload = message;
                    mTransmit(std::move(remMess));
                }
            }
        }
    }

    if (loggerFunction) {
        if (consoleLogLevel >= logLevel || fileLogLevel >= logLevel || alwaysLog) {
            loggerFunction(logLevel, header, message);
        }
    } else if (initialized.load()) {
        if (consoleLogLevel >= logLevel || alwaysLog) {
            if (logLevel == HELICS_LOG_LEVEL_DUMPLOG) {  // dumplog
                consoleLogger->log(spdlog::level::trace, "{}", message);
            } else {
                consoleLogger->log(getSpdLogLevel(logLevel), "{}::{}", header, message);
            }

            if (forceLoggingFlush) {
                consoleLogger->flush();
            }
        }
        if (fileLogger && (fileLogLevel >= logLevel || alwaysLog)) {
            if (logLevel == HELICS_LOG_LEVEL_DUMPLOG) {  // dumplog
                fileLogger->log(spdlog::level::trace, "{}", message);
            } else {
                fileLogger->log(getSpdLogLevel(logLevel), "{}::{}", header, message);
            }

            if (forceLoggingFlush) {
                fileLogger->flush();
            }
        }
        return true;
    }
    return false;
}

void LogManager::addLoggingCLI(std::shared_ptr<helicsCLI11App>& app)
{
    auto* logging_group =
        app->add_option_group("logging", "Options related to file and message logging");
    logging_group->add_flag_function(
        "--force_logging_flush",
        [this](int64_t val) {
            if (val > 0) {
                forceLoggingFlush = true;
            }
        },
        "flush the log after every message");
    logging_group->add_option("--logfile", logFile, "the file to log the messages to");
    logging_group
        ->add_option_function<int>(
            "--loglevel",
            [this](int val) { setLogLevel(val); },
            "the level at which to log, the higher this is set the more gets logged; set to \"no_print\" for no logging")
        ->envname("HELICS_BROKER_LOG_LEVEL")

        ->transform(
            CLI::CheckedTransformer(&gLogLevelMap, CLI::ignore_case, CLI::ignore_underscore))
        ->transform(CLI::IsMember(&gLogLevelMap, CLI::ignore_case, CLI::ignore_underscore));

    logging_group
        ->add_option("--fileloglevel",
                     fileLogLevel,
                     "the level at which messages get sent to the file")
        ->transform(
            CLI::CheckedTransformer(&gLogLevelMap, CLI::ignore_case, CLI::ignore_underscore))
        ->transform(CLI::IsMember(&gLogLevelMap, CLI::ignore_case, CLI::ignore_underscore));
    logging_group
        ->add_option("--consoleloglevel",
                     consoleLogLevel,
                     "the level at which messages get sent to the file")

        ->transform(
            CLI::CheckedTransformer(&gLogLevelMap, CLI::ignore_case, CLI::ignore_underscore))
        ->transform(CLI::IsMember(&gLogLevelMap, CLI::ignore_case, CLI::ignore_underscore));
    logging_group
        ->add_flag_function(
            fmt::format("--logbuffer{{{}}}", LogBuffer::cDefaultBufferSize),
            [this](std::int64_t val) { mLogBuffer.resize(val); },
            "optionally specify the size of the circular buffer for storing log messages for later retrieval ")
        ->expected(0, 1)
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);
    logging_group->callback([this]() { updateMaxLogLevel(); });
}

void LogManager::setLoggerFunction(
    std::function<void(int, std::string_view, std::string_view)> logFunction)
{
    loggerFunction = std::move(logFunction);
}

void LogManager::setLogLevel(int32_t level)
{
    setLogLevels(level, level);
}

void LogManager::logFlush()
{
    if (consoleLogger) {
        consoleLogger->flush();
    }
    if (fileLogger) {
        fileLogger->flush();
    }
}

void LogManager::setLogLevels(int32_t consoleLevel, int32_t fileLevel)
{
    consoleLogLevel = consoleLevel;
    fileLogLevel = fileLevel;
    updateMaxLogLevel();
}

void LogManager::updateMaxLogLevel()
{
    int maxLevel = (std::max)(consoleLogLevel, fileLogLevel);
    for (const auto& rl : remoteTargets) {
        if (rl.second > maxLevel) {
            maxLevel = rl.second;
        }
    }
    maxLogLevel.store(maxLevel);
}

void LogManager::setLoggingFile(std::string_view lfile, const std::string& identifier)
{
    if (logFile.empty() || lfile != logFile) {
        logFile = lfile;
        if (!logFile.empty()) {
            fileLogger = spdlog::basic_logger_mt(identifier, logFile);
        } else {
            if (fileLogger) {
                spdlog::drop(logIdentifier);
                fileLogger.reset();
            }
        }
    }
    logIdentifier = identifier;
}

void LogManager::updateRemote(GlobalFederateId destination, int level)
{
    for (auto& rl : remoteTargets) {
        if (rl.first == destination) {
            rl.second = level;
            return;
        }
    }
    remoteTargets.emplace_back(destination, level);
    updateMaxLogLevel();
}
}  // namespace helics
