/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "BrokerBase.hpp"

#include "../common/configFileHelpers.hpp"
#include "../common/logging.hpp"
#include "AsyncTimeCoordinator.hpp"
#include "ForwardingTimeCoordinator.hpp"
#include "GlobalTimeCoordinator.hpp"
#include "LogManager.hpp"
#include "ProfilerBuffer.hpp"
#include "core-exceptions.hpp"
#include "flagOperations.hpp"
#include "gmlc/libguarded/guarded.hpp"
#include "gmlc/utilities/stringOps.h"
#include "gmlc/utilities/string_viewConversion.h"
#include "helics/common/JsonGeneration.hpp"
#include "helics/common/LogBuffer.hpp"
#include "helics/core/helicsCLI11JsonConfig.hpp"
#include "helicsCLI11.hpp"
#include "loggingHelper.hpp"

#include <fmt/format.h>

#ifndef HELICS_DISABLE_ASIO
#    include "gmlc/networking/AsioContextManager.h"

#    include <asio/steady_timer.hpp>
#else
#    if defined(_WIN32) || defined(WIN32)
#        include <windows.h>
#    else
#        include <unistd.h>
#    endif
#endif

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

static inline std::string genId()
{
    std::string newid = gmlc::utilities::randomString(24);

    newid[0] = '-';
    newid[6] = '-';
    newid[12] = '-';
    newid[18] = '-';

#ifdef _WIN32
    std::string pid_str = std::to_string(GetCurrentProcessId()) + newid;
#else
    std::string pid_str = std::to_string(getpid()) + newid;
#endif
    return pid_str;
}

namespace helics {

BrokerBase::BrokerBase(bool DisableQueue) noexcept:
    queueDisabled(DisableQueue), mLogManager(std::make_shared<LogManager>())
{
}

BrokerBase::BrokerBase(std::string_view broker_name, bool DisableQueue):
    identifier(broker_name), queueDisabled(DisableQueue),
    mLogManager(std::make_shared<LogManager>())
{
}

BrokerBase::~BrokerBase()
{
    if (!queueDisabled) {
        try {
            joinAllThreads();
        }
        catch (...) {
            ;  // no exceptions in the destructor
        }
    }
}
std::function<void(int, std::string_view, std::string_view)> BrokerBase::getLoggingCallback() const
{
    return [this](int level, std::string_view name, std::string_view message) {
        sendToLogger(global_id.load(), level, name, message);
    };
}

void BrokerBase::joinAllThreads()
{
    if ((!queueDisabled) && (queueProcessingThread.joinable())) {
        actionQueue.push(CMD_TERMINATE_IMMEDIATELY);
        queueProcessingThread.join();
    }
}

std::shared_ptr<helicsCLI11App> BrokerBase::generateCLI()
{
    auto hApp = std::make_shared<helicsCLI11App>("Core/Broker specific arguments");
    hApp->remove_helics_specifics();
    return hApp;
}

std::shared_ptr<helicsCLI11App> BrokerBase::generateBaseCLI()
{
    auto hApp = std::make_shared<helicsCLI11App>("Arguments applying to all Brokers and Cores");
    auto* fmtr = addJsonConfig(hApp.get());
    fmtr->maxLayers(0);
    fmtr->promoteSection("helics");
    hApp->option_defaults()->ignore_underscore()->ignore_case();
    hApp->add_option("--federates,-f",
                     minFederateCount,
                     "the minimum number of federates that will be connecting");
    hApp->add_option("--maxfederates",
                     maxFederateCount,
                     "the maximum number of federates that will be connecting");
    hApp->add_option("--name,-n,--identifier,--uuid", identifier, "the name of the broker/core");
    hApp->add_option("--max_iterations",
                     maxIterationCount,
                     "the maximum number of iterations allowed")
        ->capture_default_str();
    hApp->add_option(
        "--minbrokers,--minbroker,--minbrokercount",
        minBrokerCount,
        "the minimum number of cores/brokers that need to be connected (ignored in cores)");
    hApp->add_option(
        "--children,--subbrokers",
        minChildCount,
        "the minimum number of child objects that need to be connected before entering init mode");
    hApp->add_option("--maxbrokers",
                     maxBrokerCount,
                     "the maximum number of brokers that will be connecting (ignored in cores)");
    hApp->add_option("--brokerkey",
                     brokerKey,
                     "specify a key to use for all connections to/from a broker")
        ->envname("HELICS_BROKER_KEY");
    hApp->add_flag(
        "--slowresponding",
        no_ping,
        "specify that a broker might be slow or unresponsive to ping requests from other brokers");
    hApp->add_flag(
        "--restrictive_time_policy",
        restrictive_time_policy,
        "specify that a broker should use a conservative time policy in the time coordinator");
    hApp->add_flag(
        "--debugging",
        debugging,
        "specify that a broker/core should operate in user debugging mode equivalent to --slow_responding --disable_timer");
    hApp->add_flag(
        "--allow_remote_control,!--disable_remote_control",
        allowRemoteControl,
        "enable the broker to respond to certain remote commands that affect operations, such as disconnect");
    hApp->add_flag(
        "--globaltime",
        globalTime,
        "specify that the broker should use a globalTime coordinator to coordinate a master clock time with all federates");
    hApp->add_flag("--global_disconnect",
                   globalDisconnect,
                   "specify that all federates should delay disconnection until all are done");
    hApp->add_flag(
        "--asynctime",
        asyncTime,
        "specify that the federation should use the asynchronous time coordinator (only minimal time management is handled in HELICS and federates are allowed to operate independently)");
    hApp->add_option_function<std::string>(
            "--timing",
            [this](const std::string& arg) {
                if (arg == "async") {
                    asyncTime = true;
                } else if (arg == "global") {
                    globalTime = true;
                } else {
                    asyncTime = false;
                    globalTime = false;
                }
            },
            "specify the timing method to use in the broker")
        ->check(CLI::IsMember({"async", "global", "distributed", "default"}));
    hApp->add_flag("--observer",
                   observer,
                   "specify that the broker/core should be added as an observer only");
    hApp->add_flag("--dynamic",
                   dynamicFederation,
                   "specify that the broker/core should allow dynamic federates");
    hApp->add_flag(
        "--disable_dynamic_sources",
        disableDynamicSources,
        "specify that the data sources must be registered before entering Initializing mode");
    hApp->add_flag("--json",
                   useJsonSerialization,
                   "use the JSON serialization mode for communications");

    // add the profiling setup command
    auto* popt =
        hApp->add_option_function<std::string>(
                "--profiler",
                [this](const std::string& fileName) {
                    if (!fileName.empty()) {
                        if (fileName == "log" || fileName == "true") {
                            if (prBuff) {
                                prBuff.reset();
                            }
                        } else {
                            if (!prBuff) {
                                prBuff = std::make_shared<ProfilerBuffer>();
                            }
                            prBuff->setOutputFile(fileName, false);
                        }

                        enable_profiling = true;
                    } else {
                        enable_profiling = false;
                    }
                },
                "activate profiling and set the profiler data output file, set to empty string to disable profiling, set to \"log\" to route profile message to the logging system.")
            ->expected(0, 1)
            ->default_str("log");

    // add the profiling append file option
    hApp->add_option_function<std::string>(
            "--profiler_append",
            [this](const std::string& fileName) {
                if (!fileName.empty()) {
                    if (!prBuff) {
                        prBuff = std::make_shared<ProfilerBuffer>();
                    }
                    prBuff->setOutputFile(fileName, true);

                    enable_profiling = true;
                } else {
                    enable_profiling = false;
                }
            },
            "activate profiling and set the profiler data output file; new profiler output will be appended to the file")
        ->excludes(popt);

    hApp->add_flag("--terminate_on_error",
                   terminate_on_error,
                   "specify that a broker should cause the federation to terminate on an error");
    hApp->add_flag(
        "--error_on_unmatched",
        errorOnUnmatchedConnections,
        "set the broker to terminate the cosimulation if there are unmatched connections");
    mLogManager->addLoggingCLI(hApp);

    hApp->add_flag(
        "--dumplog",
        dumplog,
        "capture a record of all messages and dump a complete log to file or console on termination");

    auto* timeout_group =
        hApp->add_option_group("timeouts", "Options related to network and process timeouts");
    timeout_group
        ->add_option(
            "--tick",
            tickTimer,
            "heartbeat time in ms, if there is no broker communication for 2 ticks then "
            "secondary actions are taken (can also be entered as a time like '10s' or '45ms')")
        ->capture_default_str();
    timeout_group->add_flag("--disable_timer,--no_tick",
                            disable_timer,
                            "if set to true all timeouts are disabled, cannot be re-enabled later");
    timeout_group
        ->add_option(
            "--timeout",
            timeout,
            "time to wait to establish a network or for a connection to communicate, default "
            "unit is in ms (can also be entered as "
            "a time like '10s' or '45ms') ")
        ->capture_default_str();
    timeout_group
        ->add_option(
            "--networktimeout",
            networkTimeout,
            "time to wait for a broker connection, default unit is in ms (can also be entered as a time "
            "like '10s' or '45ms') ")
        ->capture_default_str();
    timeout_group->add_option(
        "--querytimeout",
        queryTimeout,
        "time to wait for a query to be answered; default unit is in ms  and default time is 15s (can also be entered as a time "
        "like '10s' or '45ms') ");
    timeout_group->add_option(
        "--granttimeout",
        grantTimeout,
        "time to wait for a time request to be granted before triggering diagnostic actions; default is in ms (can also be entered as a time "
        "like '10s' or '45ms')");
    timeout_group
        ->add_option(
            "--maxcosimduration",
            maxCoSimDuration,
            "the maximum time a broker/core should be active, the co-simulation will self terminate if it is still active after this duration, the time resolution is the tick timer (can also be entered as a time "
            "like '10s' or '45ms')")
        ->multi_option_policy(CLI::MultiOptionPolicy::TakeLast);
    timeout_group
        ->add_option("--errordelay,--errortimeout",
                     errorDelay,
                     "time to wait after an error state before terminating "
                     "like '10s' or '45ms') ")
        ->default_str(std::to_string(static_cast<double>(errorDelay)));

    return hApp;
}

int BrokerBase::parseArgs(int argc, char* argv[])
{
    auto app = generateBaseCLI();
    auto sApp = generateCLI();
    app->add_subcommand(sApp);
    auto res = app->helics_parse(argc, argv);
    return static_cast<int>(res);
}

int BrokerBase::parseArgs(std::vector<std::string> args)
{
    auto app = generateBaseCLI();
    auto sApp = generateCLI();
    app->add_subcommand(sApp);
    auto res = app->helics_parse(std::move(args));
    return static_cast<int>(res);
}

int BrokerBase::parseArgs(std::string_view initializationString)
{
    auto type = fileops::getConfigType(initializationString);

    switch (type) {
        case fileops::ConfigType::JSON_FILE:
            fileInUse = true;
            loadInfoFromJson(std::string(initializationString));
            configString = initializationString;
            return 0;
        case fileops::ConfigType::JSON_STRING:
            try {
                loadInfoFromJson(std::string(initializationString));
                configString = initializationString;
                return 0;
            }
            catch (const helics::InvalidParameter&) {
                if (fileops::looksLikeConfigToml(initializationString)) {
                    try {
                        loadInfoFromToml(std::string(initializationString));
                        configString = initializationString;
                        return 0;
                    }
                    catch (const helics::InvalidParameter&) {
                        if (fileops::looksLikeCommandLine(initializationString)) {
                            break;
                        }
                        throw;
                    }
                }
                throw;
            }
            break;
        case fileops::ConfigType::TOML_FILE:
            fileInUse = true;
            loadInfoFromToml(std::string(initializationString));
            configString = initializationString;
            return 0;
        case fileops::ConfigType::TOML_STRING:
            try {
                loadInfoFromToml(std::string(initializationString));
                configString = initializationString;
                return 0;
            }
            catch (const helics::InvalidParameter&) {
                if (fileops::looksLikeCommandLine(configString)) {
                    break;
                }
                throw;
            }
            break;
        case fileops::ConfigType::CMD_LINE:
        case fileops::ConfigType::NONE:
            // with NONE there are default command line and environment possibilities
            break;
    }

    auto app = generateBaseCLI();
    auto sApp = generateCLI();
    app->add_subcommand(sApp);
    auto res = app->helics_parse(std::string(initializationString));
    return static_cast<int>(res);
}

void BrokerBase::loadInfoFromJson(const std::string& jsonString, bool runArgParser)
{
    nlohmann::json doc;
    try {
        doc = fileops::loadJson(jsonString);
    }
    catch (const std::invalid_argument& iarg) {
        throw(helics::InvalidParameter(iarg.what()));
    }
    const bool hasHelicsSection = doc.contains("helics");
    bool hasHelicsSubSection{false};
    bool hasHelicsBrokerSubSection{false};
    if (hasHelicsSection) {
        hasHelicsSubSection = doc["helics"].contains("helics");
        hasHelicsBrokerSubSection = doc["helics"].contains("broker");
    }
    const bool hasBrokerSection = doc.contains("broker");

    if (runArgParser) {
        auto app = generateBaseCLI();
        auto sApp = generateCLI();
        app->add_subcommand(sApp);
        app->allow_extras();
        try {
            if (jsonString.find('{') != std::string::npos) {
                std::istringstream jstring(jsonString);
                app->parse_from_stream(jstring);
                if (hasHelicsSection) {
                    app->get_config_formatter_base()->section("helics");
                    std::istringstream jstringHelics(jsonString);
                    app->parse_from_stream(jstringHelics);
                    if (hasHelicsSubSection) {
                        app->get_config_formatter_base()->section("helics.helics");
                        std::istringstream jstringHelicsSub(jsonString);
                        app->parse_from_stream(jstringHelicsSub);
                    }
                    if (hasHelicsBrokerSubSection) {
                        app->get_config_formatter_base()->section("helics.broker");
                        std::istringstream jstringHelicsSub(jsonString);
                        app->parse_from_stream(jstringHelicsSub);
                    }
                }
                if (hasBrokerSection) {
                    app->get_config_formatter_base()->section("broker");
                    std::istringstream jstringBroker(jsonString);
                    app->parse_from_stream(jstringBroker);
                }
            } else {
                std::ifstream file(jsonString);
                app->parse_from_stream(file);
                if (hasHelicsSection) {
                    file.clear();
                    file.seekg(0);
                    app->get_config_formatter_base()->section("helics");
                    app->parse_from_stream(file);
                    if (hasHelicsSubSection) {
                        file.clear();
                        file.seekg(0);
                        app->get_config_formatter_base()->section("helics.helics");
                        app->parse_from_stream(file);
                    }
                    if (hasHelicsBrokerSubSection) {
                        file.clear();
                        file.seekg(0);
                        app->get_config_formatter_base()->section("helics.broker");
                        app->parse_from_stream(file);
                    }
                }
                if (hasBrokerSection) {
                    file.clear();
                    file.seekg(0);
                    app->get_config_formatter_base()->section("broker");
                    app->parse_from_stream(file);
                }
            }
        }
        catch (const CLI::Error& clierror) {
            throw(InvalidIdentifier(clierror.what()));
        }
    }
}

void BrokerBase::loadInfoFromToml(const std::string& tomlString, bool runArgParser)
{
    toml::value doc;
    try {
        doc = fileops::loadToml(tomlString);
    }
    catch (const std::invalid_argument& iarg) {
        throw(helics::InvalidParameter(iarg.what()));
    }

    if (runArgParser) {
        auto app = generateBaseCLI();
        auto sApp = generateCLI();
        app->add_subcommand(sApp);
        app->allow_extras();
        auto dptr = std::static_pointer_cast<HelicsConfigJSON>(app->get_config_formatter_base());
        if (dptr) {
            dptr->skipJson(true);
        }
        try {
            if (tomlString.find('=') != std::string::npos) {
                std::istringstream tstring(tomlString);
                app->parse_from_stream(tstring);
            } else {
                std::ifstream file(tomlString);
                app->parse_from_stream(file);
            }
        }
        catch (const CLI::Error& e) {
            throw(InvalidIdentifier(e.what()));
        }
    }
}

void BrokerBase::configureBase()
{
    if (debugging) {
        no_ping = true;
        disable_timer = true;
    }
    if (networkTimeout < timeZero) {
        networkTimeout = 4.0;
    }

    if (!noAutomaticID) {
        if (identifier.empty()) {
            identifier = genId();
        }
    }

    if (identifier.size() == 36) {
        if (identifier[8] == '-' && identifier[12] == '-' && identifier[16] == '-' &&
            identifier[20] == '-') {
            uuid_like = true;
        }
    }
    if (asyncTime) {
        timeCoord = std::make_unique<AsyncTimeCoordinator>();
        hasTimeDependency = true;
    } else if (globalTime) {
        timeCoord = std::make_unique<GlobalTimeCoordinator>();
        hasTimeDependency = true;
    } else {
        timeCoord = std::make_unique<ForwardingTimeCoordinator>();
    }

    timeCoord->setMessageSender([this](const ActionMessage& msg) { addActionMessage(msg); });
    timeCoord->setRestrictivePolicy(restrictive_time_policy);

    mLogManager->setTransmitCallback([this](ActionMessage&& message) {
        if (getBrokerState() < BrokerState::TERMINATING) {
            message.source_id = global_id.load();
            addActionMessage(std::move(message));
        }
    });
    mLogManager->initializeLogging(identifier);
    maxLogLevel.store(mLogManager->getMaxLevel());
    mainLoopIsRunning.store(true);
    queueProcessingThread = std::thread(&BrokerBase::queueProcessingLoop, this);
    brokerState = BrokerState::CONFIGURED;
}

bool BrokerBase::sendToLogger(GlobalFederateId federateID,
                              int logLevel,
                              std::string_view name,
                              std::string_view message,
                              bool fromRemote) const
{
    const bool noID = (federateID != global_id.load()) || (!name.empty() && name.back() == ']');

    std::string header;
    if (noID) {
        header = name;
    } else {
        std::string timeString;

        const Time currentTime = getSimulationTime();
        if (currentTime <= mInvalidSimulationTime || currentTime >= cHelicsBigNumber) {
            timeString.push_back('[');
            timeString.append(brokerStateName(getBrokerState()));
            timeString.push_back(']');
        } else {
            timeString = fmt::format("[t={}]", static_cast<double>(currentTime));
        }
        header = fmt::format("{} ({}){}", name, federateID.baseValue(), timeString);
    }
    return mLogManager->sendToLogger(logLevel, header, message, fromRemote);
}

void BrokerBase::generateNewIdentifier()
{
    identifier = genId();
    uuid_like = false;
}

void BrokerBase::saveProfilingData(std::string_view message)
{
    if (prBuff) {
        prBuff->addMessage(std::string(message));
    } else {
        sendToLogger(parent_broker_id, LogLevels::PROFILING, "[PROFILING]", message);
    }
}

void BrokerBase::writeProfilingData()
{
    if (prBuff) {
        try {
            prBuff->writeFile();
        }
        catch (const std::ios_base::failure&) {
            sendToLogger(parent_broker_id,
                         LogLevels::ERROR_LEVEL,
                         identifier,
                         "Unable to write profiling buffer data");
        }
    }
}

void BrokerBase::setErrorState(int eCode, std::string_view estring)
{
    lastErrorString.assign(estring.data(), estring.size());
    lastErrorCode.store(eCode);
    auto cBrokerState = brokerState.load();
    if (cBrokerState != BrokerState::ERRORED && cBrokerState != BrokerState::CONNECTED_ERROR) {
        if (cBrokerState > BrokerState::CONFIGURED && cBrokerState < BrokerState::TERMINATING) {
            brokerState.store(BrokerState::CONNECTED_ERROR);
        } else {
            brokerState.store(BrokerState::ERRORED);
        }
        if (errorDelay <= timeZero || eCode == HELICS_ERROR_TERMINATED ||
            eCode == HELICS_ERROR_USER_ABORT) {
            const ActionMessage halt(CMD_USER_DISCONNECT, global_id.load(), global_id.load());
            addActionMessage(halt);
        } else {
            errorTimeStart = std::chrono::steady_clock::now();
            const ActionMessage echeck(CMD_ERROR_CHECK, global_id.load(), global_id.load());
            addActionMessage(echeck);
        }
    }

    sendToLogger(global_id.load(), HELICS_LOG_LEVEL_ERROR, identifier, estring);
}

void BrokerBase::setLoggingFile(std::string_view lfile)
{
    mLogManager->setLoggingFile(lfile, identifier);
}

bool BrokerBase::getFlagValue(int32_t flag) const
{
    switch (flag) {
        case HELICS_FLAG_DUMPLOG:
            return dumplog;
        case HELICS_FLAG_FORCE_LOGGING_FLUSH:
            return mLogManager->forceLoggingFlush.load();
        default:
            return false;
    }
}

std::pair<bool, std::vector<std::string_view>>
    BrokerBase::processBaseCommands(ActionMessage& command)
{
    auto cmd = command.payload.to_string();
    auto commentLoc = cmd.find('#');
    if (commentLoc != std::string_view::npos) {
        cmd = cmd.substr(0, commentLoc - 1);
    }
    gmlc::utilities::string_viewOps::trimString(cmd);
    auto res = gmlc::utilities::string_viewOps::splitlineQuotes(
        cmd,
        " |",
        gmlc::utilities::string_viewOps::default_quote_chars,
        gmlc::utilities::string_viewOps::delimiter_compression::on);
    if (res.empty()) {
        return {true, {}};
    }
    if (res[0] == "ignore") {
    } else if (res[0] == "terminate") {
        if (allowRemoteControl) {
            LOG_SUMMARY(global_broker_id_local,
                        identifier,
                        " received terminate instruction via command instruction")
            const ActionMessage udisconnect(CMD_USER_DISCONNECT);
            addActionMessage(udisconnect);
        }
    } else if (res[0] == "echo") {
        LOG_SUMMARY(global_broker_id_local,
                    identifier,
                    " received echo command via command instruction")
        command.swapSourceDest();
        command.payload = "echo_reply";
        command.setString(targetStringLoc, command.getString(sourceStringLoc));
        command.setString(sourceStringLoc, identifier);
        addActionMessage(command);
    } else if (res[0] == "log") {
        LOG_SUMMARY(global_broker_id_local,
                    command.getString(sourceStringLoc),
                    command.payload.to_string().substr(4));
    } else if (res[0] == "logbuffer") {
        if (res.size() > 1) {
            if (res[1] == "stop") {
                mLogManager->getLogBuffer().enable(false);
            } else {
                mLogManager->getLogBuffer().resize(gmlc::utilities::numeric_conversion<std::size_t>(
                    res[1], LogBuffer::cDefaultBufferSize));
            }
        } else {
            mLogManager->getLogBuffer().enable(true);
        }
    } else if (res[0] == "remotelog") {
        if (res.size() > 1) {
            if (res[1] == "stop") {
                mLogManager->updateRemote(command.source_id, HELICS_LOG_LEVEL_NO_PRINT);
            } else {
                int newLogLevel = HELICS_LOG_LEVEL_NO_PRINT;
                if (isdigit(res[1][0]) != 0) {
                    newLogLevel =
                        gmlc::utilities::numeric_conversion<int>(res[1], HELICS_LOG_LEVEL_NO_PRINT);
                } else {
                    newLogLevel = logLevelFromString(res[1]);
                }
                mLogManager->updateRemote(command.source_id, newLogLevel);
            }
        } else {
            mLogManager->updateRemote(command.source_id, mLogManager->getConsoleLevel());
        }
        maxLogLevel.store(mLogManager->getMaxLevel());
    } else {
        return {false, res};
    }
    return {true, res};
}

void BrokerBase::addBaseInformation(nlohmann::json& base, bool hasParent) const
{
    nlohmann::json object;
    object["name"] = identifier;
    if (uuid_like) {
        object["uuid"] = identifier;
    }
    object["id"] = global_id.load().baseValue();
    if (hasParent) {
        object["parent"] = higher_broker_id.baseValue();
    } else {
        object["parent"] = 0;
    }
    base["attributes"] = object;
}

void BrokerBase::setLoggerFunction(
    std::function<void(int, std::string_view, std::string_view)> logFunction)
{
    mLogManager->setLoggerFunction(std::move(logFunction));
}

void BrokerBase::setLogLevel(int32_t level)
{
    setLogLevels(level, level);
}

void BrokerBase::logFlush()
{
    mLogManager->logFlush();
}
/** set the logging levels
@param consoleLevel the logging level for the console display
@param fileLevel the logging level for the log file
*/
void BrokerBase::setLogLevels(int32_t consoleLevel, int32_t fileLevel)
{
    mLogManager->setLogLevels(consoleLevel, fileLevel);
    maxLogLevel.store(mLogManager->getMaxLevel());
}

void BrokerBase::addActionMessage(const ActionMessage& message)
{
    if (isPriorityCommand(message)) {
        actionQueue.pushPriority(message);
    } else {
        // just route to the general queue;
        actionQueue.push(message);
    }
}

void BrokerBase::addActionMessage(ActionMessage&& message)
{
    if (isPriorityCommand(message)) {
        actionQueue.emplacePriority(std::move(message));
    } else {
        // just route to the general queue;
        actionQueue.emplace(std::move(message));
    }
}

void BrokerBase::addActionMessage(ActionMessage&& message) const
{
    // the queue is thread safe so can be run in a const situation without possibility of issues
    auto& lQueue = const_cast<decltype(actionQueue)&>(actionQueue);
    if (isPriorityCommand(message)) {
        lQueue.emplacePriority(std::move(message));
    } else {
        // just route to the general queue;
        lQueue.emplace(std::move(message));
    }
}

#ifndef HELICS_DISABLE_ASIO
using activeProtector = gmlc::libguarded::guarded<std::pair<bool, bool>>;

static bool haltTimer(activeProtector& active, asio::steady_timer& tickTimer)
{
    bool TimerRunning = true;
    {
        auto protector = active.lock();
        if (protector->second) {
            protector->first = false;
            protector.unlock();
            auto cancelled = tickTimer.cancel();
            if (cancelled == 0) {
                TimerRunning = false;
            }
        } else {
            TimerRunning = false;
        }
    }
    int timerLoopCount = 0;
    while (TimerRunning) {
        if (timerLoopCount % 4 != 3) {
            std::this_thread::yield();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(40));
        }
        auto res = active.load();
        TimerRunning = res.second;
        ++timerLoopCount;
        if (timerLoopCount == 100) {
            // assume the timer was never started so just exit and hope it doesn't somehow get
            // called later and generate a seg fault.
            return false;
        }
    }
    return true;
}

static void
    timerTickHandler(BrokerBase* bbase, activeProtector& active, const std::error_code& error)
{
    auto timeProtector = active.lock();
    if (timeProtector->first) {
        if (error != asio::error::operation_aborted) {
            try {
                bbase->addActionMessage(CMD_TICK);
            }
            catch (std::exception& e) {
                std::cerr << "exception caught from addActionMessage" << e.what() << '\n';
            }
        } else {
            ActionMessage tick(CMD_TICK);
            setActionFlag(tick, error_flag);
            bbase->addActionMessage(tick);
        }
    }
    timeProtector->second = false;
}

#endif

bool BrokerBase::tryReconnect()
{
    return false;
}

// #define DISABLE_TICK
void BrokerBase::queueProcessingLoop()
{
    if (haltOperations) {
        mainLoopIsRunning.store(false);
        return;
    }
    std::vector<ActionMessage> dumpMessages;
#ifndef HELICS_DISABLE_ASIO
    auto serv = gmlc::networking::AsioContextManager::getContextPointer();
    auto contextLoop = serv->startContextLoop();
    asio::steady_timer ticktimer(serv->getBaseContext());
    activeProtector active(true, false);

    auto timerCallback = [this, &active](const std::error_code& errorCode) {
        timerTickHandler(this, active, errorCode);
    };
    if (tickTimer > timeZero && !disable_timer) {
        if (tickTimer < Time(0.5)) {
            tickTimer = Time(0.5);
        }
        active = std::make_pair(true, true);
        ticktimer.expires_at(std::chrono::steady_clock::now() + tickTimer.to_ns());
        ticktimer.async_wait(timerCallback);
    }
    auto timerStop = [&, this]() {
        if (!haltTimer(active, ticktimer)) {
            sendToLogger(global_broker_id_local,
                         LogLevels::WARNING,
                         identifier,
                         "timer unable to cancel properly");
        }
        contextLoop = nullptr;
    };
    auto timeStart = std::chrono::steady_clock::now();
#else
    auto timerStop = []() {};
#endif

    global_broker_id_local = global_id.load();
    int messagesSinceLastTick = 0;
    auto logDump = [&, this]() {
        if (!dumpMessages.empty()) {
            for (auto& act : dumpMessages) {
                mLogManager->sendToLogger(HELICS_LOG_LEVEL_DUMPLOG,
                                          identifier,
                                          fmt::format("|| dl cmd:{} from {} to {}",
                                                      prettyPrintString(act),
                                                      act.source_id.baseValue(),
                                                      act.dest_id.baseValue()));
            }
        }
    };
    if (haltOperations) {
        timerStop();
        mainLoopIsRunning.store(false);
        return;
    }
    while (true) {
        auto command = actionQueue.pop();
        ++messageCounter;
        if (dumplog) {
            dumpMessages.push_back(command);
        }
        if (command.action() == CMD_IGNORE) {
            continue;
        }
        auto ret = commandProcessor(command);
        if (ret == CMD_IGNORE) {
            ++messagesSinceLastTick;
            continue;
        }
        switch (ret) {
            case CMD_TICK:
                if (checkActionFlag(command, error_flag)) {
#ifndef HELICS_DISABLE_ASIO
                    contextLoop = nullptr;
                    contextLoop = serv->startContextLoop();
#endif
                }
                // deal with error state timeout
                if (brokerState.load() == BrokerState::CONNECTED_ERROR) {
                    auto ctime = std::chrono::steady_clock::now();
                    auto timeDiff = ctime - errorTimeStart;
                    if (timeDiff >= errorDelay.to_ms()) {
                        command.setAction(CMD_USER_DISCONNECT);
                        addActionMessage(command);
                    } else {
#ifndef HELICS_DISABLE_ASIO
                        if (!disable_timer) {
                            ticktimer.expires_at(errorTimeStart + errorDelay.to_ns());
                            active = std::make_pair(true, true);
                            ticktimer.async_wait(timerCallback);
                        } else {
                            command.setAction(CMD_ERROR_CHECK);
                            addActionMessage(command);
                        }
#else
                        command.setAction(CMD_ERROR_CHECK);
                        addActionMessage(command);
#endif
                    }
                    break;
                }
#ifndef DISABLE_TICK
                if (messagesSinceLastTick == 0) {
                    command.messageID =
                        forwardingReasons | static_cast<uint32_t>(TickForwardingReasons::NO_COMMS);
                    processCommand(std::move(command));
                } else if (forwardTick) {
                    command.messageID = forwardingReasons;
                }
#endif
                messagesSinceLastTick = 0;
// reschedule the timer
#ifndef HELICS_DISABLE_ASIO
                {
                    auto currTime = std::chrono::steady_clock::now();
                    if (maxCoSimDuration > timeZero) {
                        if ((currTime - timeStart) > maxCoSimDuration.to_ms()) {
                            ActionMessage dDisable(CMD_TIMEOUT_DISCONNECT);
                            dDisable.source_id = global_broker_id_local;
                            dDisable.dest_id = global_broker_id_local;
                            addActionMessage(dDisable);
                            break;
                        }
                    }
                    if (tickTimer > timeZero && !disable_timer) {
                        ticktimer.expires_at(currTime + tickTimer.to_ns());
                        active = std::make_pair(true, true);
                        ticktimer.async_wait(timerCallback);
                    }
                }
#endif
                break;
            case CMD_ERROR_CHECK:
                if (brokerState.load() == BrokerState::CONNECTED_ERROR) {
                    auto ctime = std::chrono::steady_clock::now();
                    auto timeDiff = ctime - errorTimeStart;
                    if (timeDiff > errorDelay.to_ms()) {
                        command.setAction(CMD_USER_DISCONNECT);
                        addActionMessage(command);
                    } else {
#ifndef HELICS_DISABLE_ASIO
                        if (tickTimer > timeDiff * 2 || disable_timer) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(200));
                            addActionMessage(command);
                        }
#else
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                        addActionMessage(command);
#endif
                    }
                }
                break;
            case CMD_PING:
                // ping is processed normally but doesn't count as an actual message for timeout
                // purposes unless it comes from the parent
                if (command.source_id != parent_broker_id) {
                    ++messagesSinceLastTick;
                }
                processCommand(std::move(command));
                break;
            case CMD_BASE_CONFIGURE:
                baseConfigure(command);
                break;
            case CMD_IGNORE:
            default:
                break;
            case CMD_TERMINATE_IMMEDIATELY:
                timerStop();
                mainLoopIsRunning.store(false);
                logDump();
                {
                    auto tcmd = actionQueue.try_pop();
                    while (tcmd) {
                        if (!isDisconnectCommand(*tcmd)) {
                            LOG_TRACE(global_broker_id_local,
                                      identifier,
                                      std::string("TI unprocessed command ") +
                                          prettyPrintString(*tcmd));
                        }
                        tcmd = actionQueue.try_pop();
                    }
                }
                return;  // immediate return
            case CMD_STOP:
                timerStop();
                if (!haltOperations) {
                    processCommand(std::move(command));
                    mainLoopIsRunning.store(false);
                    logDump();
                    processDisconnect();
                }
                auto tcmd = actionQueue.try_pop();
                while (tcmd) {
                    if (!isDisconnectCommand(*tcmd)) {
                        LOG_TRACE(global_broker_id_local,
                                  identifier,
                                  std::string("STOPPED unprocessed command ") +
                                      prettyPrintString(*tcmd));
                    }
                    tcmd = actionQueue.try_pop();
                }
                return;
        }
    }
}

void BrokerBase::setTickForwarding(TickForwardingReasons reason, bool value)
{
    if (value) {
        forwardingReasons |= static_cast<std::uint32_t>(reason);
    } else {
        forwardingReasons &= ~static_cast<std::uint32_t>(reason);
    }
    forwardTick = (forwardingReasons != 0);
}

bool BrokerBase::setBrokerState(BrokerState newState)
{
    auto currentState = brokerState.load();
    switch (currentState) {
        case BrokerState::ERRORED:
            return (newState == BrokerState::ERRORED);
        case BrokerState::CONNECTED_ERROR:
            if (newState == BrokerState::TERMINATING) {
                newState = BrokerState::TERMINATING_ERROR;
            } else if (newState == BrokerState::TERMINATED || newState == BrokerState::ERRORED) {
                newState = BrokerState::ERRORED;
            } else {
                return (newState == BrokerState::CONNECTED_ERROR);
            }
            break;
        case BrokerState::TERMINATING_ERROR:
            if (newState == BrokerState::TERMINATED || newState == BrokerState::ERRORED) {
                newState = BrokerState::ERRORED;
            } else {
                return (newState == BrokerState::TERMINATING_ERROR);
            }
            break;
        default:
            if (newState == BrokerState::ERRORED) {
                if (currentState > BrokerState::CONNECTING &&
                    currentState < BrokerState::TERMINATING) {
                    newState = BrokerState::CONNECTED_ERROR;
                }
            }
            break;
    }

    brokerState.store(newState);
    return true;
}

bool BrokerBase::transitionBrokerState(BrokerState expectedState, BrokerState newState)
{
    return brokerState.compare_exchange_strong(expectedState, newState);
}

void BrokerBase::baseConfigure(ActionMessage& command)
{
    if (command.action() == CMD_BASE_CONFIGURE) {
        switch (command.messageID) {
            case HELICS_FLAG_DUMPLOG:
                dumplog = checkActionFlag(command, indicator_flag);
                break;
            case HELICS_FLAG_FORCE_LOGGING_FLUSH:
                mLogManager->forceLoggingFlush = checkActionFlag(command, indicator_flag);
                break;
            default:
                break;
        }
    }
}

action_message_def::action_t BrokerBase::commandProcessor(ActionMessage& command)
{
    switch (command.action()) {
        case CMD_IGNORE:
            break;
        case CMD_TERMINATE_IMMEDIATELY:
        case CMD_STOP:
        case CMD_TICK:
        case CMD_BASE_CONFIGURE:
        case CMD_PING:
        case CMD_ERROR_CHECK:
            return command.action();
        case CMD_MULTI_MESSAGE:
            for (int ii = 0; ii < command.counter; ++ii) {
                ActionMessage NMess;
                NMess.from_string(command.getString(ii));
                auto commandAction = commandProcessor(NMess);
                if (commandAction != CMD_IGNORE) {
                    // overwrite the abort command but ignore ticks in a multi-message context
                    // they shouldn't be there
                    if (commandAction != CMD_TICK) {
                        command = NMess;
                        return commandAction;
                    }
                }
            }
            break;
        default:
            if (!haltOperations) {
                if (isPriorityCommand(command)) {
                    processPriorityCommand(std::move(command));
                } else {
                    processCommand(std::move(command));
                }
            }
    }
    return CMD_IGNORE;
}

// LCOV_EXCL_START
const std::string& brokerStateName(BrokerBase::BrokerState state)
{
    static const std::string createdString = "created";
    static const std::string configuringString = "configuring";
    static const std::string configuredString = "configured";
    static const std::string connectingString = "connecting";
    static const std::string connectedString = "connected";
    static const std::string initializingString = "initializing";
    static const std::string operatingString = "operating";
    static const std::string terminatingString = "terminating";
    static const std::string terminatingErrorString = "terminating_error";
    static const std::string terminatedString = "terminated";
    static const std::string erroredString = "error";
    static const std::string connectedErrorString = "connected_error";
    static const std::string otherString = "other";
    switch (state) {
        case BrokerBase::BrokerState::CREATED:
            return createdString;
        case BrokerBase::BrokerState::CONFIGURING:
            return configuringString;
        case BrokerBase::BrokerState::CONFIGURED:
            return configuredString;
        case BrokerBase::BrokerState::CONNECTING:
            return connectingString;
        case BrokerBase::BrokerState::CONNECTED:
            return connectedString;
        case BrokerBase::BrokerState::INITIALIZING:
            return initializingString;
        case BrokerBase::BrokerState::OPERATING:
            return operatingString;
        case BrokerBase::BrokerState::TERMINATING:
            return terminatingString;
        case BrokerBase::BrokerState::TERMINATING_ERROR:
            return terminatingErrorString;
        case BrokerBase::BrokerState::TERMINATED:
            return terminatedString;
        case BrokerBase::BrokerState::ERRORED:
            return erroredString;
        case BrokerBase::BrokerState::CONNECTED_ERROR:
            return connectedErrorString;
        default:
            return otherString;
    }
}
// LCOV_EXCL_STOP

}  // namespace helics
