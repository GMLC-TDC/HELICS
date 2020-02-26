/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "BrokerBase.hpp"

#include "../common/fmt_format.h"
#include "../common/logger.h"
#include "ForwardingTimeCoordinator.hpp"
#include "flagOperations.hpp"
#include "gmlc/libguarded/guarded.hpp"
#include "gmlc/utilities/stringOps.h"
#include "helicsCLI11.hpp"
#include "loggingHelper.hpp"
#ifndef HELICS_DISABLE_ASIO
#    include "../common/AsioContextManager.h"

#    include <asio/steady_timer.hpp>
#else
#    ifdef _WIN32
#        include <windows.h>
#    else
#    endif
#endif

#include <iostream>

static inline std::string genId()
{
    std::string nm = gmlc::utilities::randomString(24);

    nm[0] = '-';
    nm[6] = '-';
    nm[12] = '-';
    nm[18] = '-';

#ifdef _WIN32
    std::string pid_str = std::to_string(GetCurrentProcessId()) + nm;
#else
    std::string pid_str = std::to_string(getpid()) + nm;
#endif
    return pid_str;
}

namespace helics {
BrokerBase::BrokerBase(bool DisableQueue) noexcept: queueDisabled(DisableQueue) {}

BrokerBase::BrokerBase(const std::string& broker_name, bool DisableQueue):
    identifier(broker_name), queueDisabled(DisableQueue)
{
}

BrokerBase::~BrokerBase()
{
    if (loggingObj) {
        loggingObj->closeFile();
        loggingObj->haltLogging();
    }
    if (!queueDisabled) {
        try {
            joinAllThreads();
        }
        catch (...) {
            // no exceptions in the destructor
        }
    }
}
std::function<void(int, const std::string&, const std::string&)>
    BrokerBase::getLoggingCallback() const
{
    return [this](int level, const std::string& name, const std::string& message) {
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

static const std::map<std::string, int> log_level_map{{"none", helics_log_level_no_print},
                                                      {"no_print", helics_log_level_no_print},
                                                      {"error", helics_log_level_error},
                                                      {"warning", helics_log_level_warning},
                                                      {"summary", helics_log_level_summary},
                                                      {"connections", helics_log_level_connections},
                                                      /** connections+ interface definitions*/
                                                      {"interfaces", helics_log_level_interfaces},
                                                      /** interfaces + timing message*/
                                                      {"timing", helics_log_level_timing},
                                                      /** timing+ data transfer notices*/
                                                      {"data", helics_log_level_data},
                                                      /** all internal messages*/
                                                      {"trace", helics_log_level_trace}};

std::shared_ptr<helicsCLI11App> BrokerBase::generateBaseCLI()
{
    auto hApp = std::make_shared<helicsCLI11App>("Arguments applying to all Brokers and Cores");
    hApp->option_defaults()->ignore_underscore()->ignore_case();
    hApp->add_option(
            "--federates,-f,--minfederates,--minfed,-m",
            minFederateCount,
            "the minimum number of federates that will be connecting")
        ->ignore_underscore();
    hApp->add_option("--name,-n,--identifier", identifier, "the name of the broker/core");
    hApp->add_option(
            "--maxiter,--maxiterations",
            maxIterationCount,
            "the maximum number of iterations allowed")
        ->capture_default_str();
    hApp->add_option(
        "--minbrokers,--minbroker,--minbrokercount",
        minBrokerCount,
        "the minimum number of cores/brokers that need to be connected (ignored in cores)");
    hApp->add_option(
        "--key,--broker_key",
        brokerKey,
        "specify a key to use for all connections to/from a broker");
    hApp->add_flag(
        "--no_ping,--slow_responding",
        no_ping,
        "specify that a broker might be slow or unresponsive to ping requests from other brokers");
    hApp->add_flag(
        "--conservative_time_policy,--restrictive_time_policy",
        restrictive_time_policy,
        "specify that a broker should use a conservative time policy in the time coordinator");
    auto logging_group =
        hApp->add_option_group("logging", "Options related to file and message logging");
    logging_group->option_defaults()->ignore_underscore();
    logging_group->add_flag(
        "--force_logging_flush", forceLoggingFlush, "flush the log after every message");
    logging_group->add_option("--logfile", logFile, "the file to log the messages to")
        ->ignore_underscore();
    logging_group
        ->add_option_function<int>(
            "--loglevel,--log-level",
            [this](int val) { setLogLevel(val); },
            "the level which to log the higher this is set to the more gets logs(-1) for no logging")
        ->transform(
            CLI::CheckedTransformer(&log_level_map, CLI::ignore_case, CLI::ignore_underscore));

    logging_group
        ->add_option(
            "--fileloglevel", fileLogLevel, "the level at which messages get sent to the file")
        ->transform(
            CLI::CheckedTransformer(&log_level_map, CLI::ignore_case, CLI::ignore_underscore));
    logging_group
        ->add_option(
            "--consoleloglevel",
            consoleLogLevel,
            "the level at which messages get sent to the file")

        ->transform(
            CLI::CheckedTransformer(&log_level_map, CLI::ignore_case, CLI::ignore_underscore));
    logging_group->add_flag(
        "--dumplog",
        dumplog,
        "capture a record of all messages and dump a complete log to file or console on termination");

    auto timeout_group =
        hApp->add_option_group("timeouts", "Options related to network and process timeouts");
    timeout_group->add_option(
        "--tick",
        tickTimer,
        "heartbeat time in ms, if there is no broker communication for 2 ticks then "
        "secondary actions are taken  (can also be entered as a time like '10s' or '45ms')");
    timeout_group->add_flag(
        "--disable_timer,--no_tick",
        disable_timer,
        "if set to true the timeout timer is disabled, cannot be re-enabled later");
    timeout_group->add_option(
        "--timeout",
        timeout,
        "time to wait to establish a network or for a connection to communicate, default "
        "unit is in ms (can also be entered as "
        "a time like '10s' or '45ms') ");
    timeout_group->add_option(
        "--networktimeout",
        networkTimeout,
        "time to wait for a broker connection default unit is in ms(can also be entered as a time "
        "like '10s' or '45ms') ");
    timeout_group->add_option(
        "--errordelay,--errortimeout",
        errorDelay,
        "time to wait after an error state before terminating "
        "like '10s' or '45ms') ")->default_str(std::to_string(static_cast<double>(errorDelay)));

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

int BrokerBase::parseArgs(const std::string& initializationString)
{
    auto app = generateBaseCLI();
    auto sApp = generateCLI();
    app->add_subcommand(sApp);
    auto res = app->helics_parse(initializationString);
    return static_cast<int>(res);
}

void BrokerBase::configureBase()
{
    if (networkTimeout < timeZero) {
        networkTimeout = 4.0;
    }

    if (!noAutomaticID) {
        if (identifier.empty()) {
            identifier = genId();
        }
    }

    timeCoord = std::make_unique<ForwardingTimeCoordinator>();
    timeCoord->setMessageSender([this](const ActionMessage& msg) { addActionMessage(msg); });
    timeCoord->restrictive_time_policy = restrictive_time_policy;

    loggingObj = std::make_unique<Logger>();
    if (!logFile.empty()) {
        loggingObj->openFile(logFile);
    }
    loggingObj->startLogging(maxLogLevel, maxLogLevel);
    mainLoopIsRunning.store(true);
    queueProcessingThread = std::thread(&BrokerBase::queueProcessingLoop, this);
    brokerState = broker_state_t::configured;
}

bool BrokerBase::sendToLogger(
    global_federate_id federateID,
    int logLevel,
    const std::string& name,
    const std::string& message) const
{
    if ((federateID == parent_broker_id) || (federateID == global_id.load())) {
        if (logLevel > maxLogLevel) {
            // check the logging level
            return true;
        }
        if (loggerFunction) {
            loggerFunction(logLevel, fmt::format("{} ({})", name, federateID.baseValue()), message);
        } else if (loggingObj) {
            loggingObj->log(
                logLevel, fmt::format("{} ({})::{}", name, federateID.baseValue(), message));
            if (forceLoggingFlush) {
                loggingObj->flush();
            }
        }
        return true;
    }
    return false;
}

void BrokerBase::generateNewIdentifier()
{
    identifier = genId();
}

void BrokerBase::setErrorState(int eCode, const std::string& estring)
{
    lastErrorString = estring;
    errorCode = eCode;
    brokerState.store(broker_state_t::errored);
    sendToLogger(global_id.load(), helics_log_level_error, identifier, estring);
}

void BrokerBase::setLoggingFile(const std::string& lfile)
{
    if (loggingObj) {
        if (loggingObj->isRunning()) {
            loggingObj->haltLogging();
            logFile = lfile;
            loggingObj->openFile(logFile);
            loggingObj->startLogging();
        }
    } else {
        logFile = lfile;
    }
}

void BrokerBase::setLoggerFunction(
    std::function<void(int, const std::string&, const std::string&)> logFunction)
{
    loggerFunction = std::move(logFunction);
    if (loggerFunction) {
        if (loggingObj) {
            if (loggingObj->isRunning()) {
                loggingObj->haltLogging();
            }
        }
    } else if (!loggingObj->isRunning()) {
        loggingObj->startLogging();
    }
}

void BrokerBase::setLogLevel(int32_t level)
{
    setLogLevels(level, level);
}

/** set the logging levels
@param consoleLevel the logging level for the console display
@param fileLevel the logging level for the log file
*/
void BrokerBase::setLogLevels(int32_t consoleLevel, int32_t fileLevel)
{
    consoleLogLevel = consoleLevel;
    fileLogLevel = fileLevel;
    maxLogLevel = (std::max)(consoleLogLevel, fileLogLevel);
    if (loggingObj) {
        loggingObj->changeLevels(consoleLogLevel, fileLogLevel);
    }
}

void BrokerBase::addActionMessage(const ActionMessage& m)
{
    if (isPriorityCommand(m)) {
        actionQueue.pushPriority(m);
    } else {
        // just route to the general queue;
        actionQueue.push(m);
    }
}

void BrokerBase::addActionMessage(ActionMessage&& m)
{
    if (isPriorityCommand(m)) {
        actionQueue.emplacePriority(std::move(m));
    } else {
        // just route to the general queue;
        actionQueue.emplace(std::move(m));
    }
}
#ifndef HELICS_DISABLE_ASIO
using activeProtector = gmlc::libguarded::guarded<std::pair<bool, bool>>;

static bool haltTimer(activeProtector& active, asio::steady_timer& tickTimer)
{
    bool TimerRunning = true;
    {
        auto p = active.lock();
        if (p->second) {
            p->first = false;
            p.unlock();
            auto cancelled = tickTimer.cancel();
            if (cancelled == 0) {
                TimerRunning = false;
            }
        } else {
            TimerRunning = false;
        }
    }
    int ii = 0;
    while (TimerRunning) {
        if (ii % 4 != 3) {
            std::this_thread::yield();
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(40));
        }
        auto res = active.load();
        TimerRunning = res.second;
        ++ii;
        if (ii == 100) {
            // assume the timer was never started so just exit and hope it doesn't somehow get called later and generate a seg fault.
            return false;
        }
    }
    return true;
}

static void
    timerTickHandler(BrokerBase* bbase, activeProtector& active, const std::error_code& error)
{
    auto p = active.lock();
    if (p->first) {
        if (error != asio::error::operation_aborted) {
            try {
                bbase->addActionMessage(CMD_TICK);
            }
            catch (std::exception& e) {
                std::cerr << "exception caught from addActionMessage" << e.what() << std::endl;
            }
        } else {
            ActionMessage M(CMD_TICK);
            setActionFlag(M, error_flag);
            bbase->addActionMessage(M);
        }
    }
    p->second = false;
}

#endif

bool BrokerBase::tryReconnect()
{
    return false;
}

//#define DISABLE_TICK
void BrokerBase::queueProcessingLoop()
{
    if (haltOperations) {
        mainLoopIsRunning.store(false);
        return;
    }
    std::vector<ActionMessage> dumpMessages;
#ifndef HELICS_DISABLE_ASIO
    auto serv = AsioContextManager::getContextPointer();
    auto contextLoop = serv->startContextLoop();
    asio::steady_timer ticktimer(serv->getBaseContext());
    activeProtector active(true, false);

    auto timerCallback = [this, &active](const std::error_code& ec) {
        timerTickHandler(this, active, ec);
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
            LOG_WARNING(global_broker_id_local, identifier, "timer unable to cancel properly");
        }
        contextLoop = nullptr;
    };
#else
    auto timerStop = []() {};
#endif

    global_broker_id_local = global_id.load();
    int messagesSinceLastTick = 0;
    auto logDump = [&, this]() {
        if (dumplog) {
            for (auto& act : dumpMessages) {
                sendToLogger(
                    parent_broker_id,
                    -10,
                    identifier,
                    fmt::format(
                        "|| dl cmd:{} from {} to {}",
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
                if (messagesSinceLastTick == 0 || forwardTick) {
#ifndef DISABLE_TICK
                    processCommand(std::move(command));
#endif
                }
                messagesSinceLastTick = 0;
// reschedule the timer
#ifndef HELICS_DISABLE_ASIO
                if (tickTimer > timeZero && !disable_timer) {
                    ticktimer.expires_at(std::chrono::steady_clock::now() + tickTimer.to_ns());
                    active = std::make_pair(true, true);
                    ticktimer.async_wait(timerCallback);
                }
#endif
                break;
            case CMD_PING:
                // ping is processed normally but doesn't count as an actual message for timeout purposes unless it
                // comes from the parent
                if (command.source_id != parent_broker_id) {
                    ++messagesSinceLastTick;
                }
                processCommand(std::move(command));
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
                            LOG_TRACE(
                                global_broker_id_local,
                                identifier,
                                std::string("TI unprocessed command ") + prettyPrintString(*tcmd));
                        }
                        tcmd = actionQueue.try_pop();
                    }
                }
                return; // immediate return
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
                        LOG_TRACE(
                            global_broker_id_local,
                            identifier,
                            std::string("STOPPED unprocessed command ") + prettyPrintString(*tcmd));
                    }
                    tcmd = actionQueue.try_pop();
                }
                return;
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
        case CMD_PING:
            return command.action();
        case CMD_MULTI_MESSAGE:
            for (int ii = 0; ii < command.counter; ++ii) {
                ActionMessage NMess;
                NMess.from_string(command.getString(ii));
                auto V = commandProcessor(NMess);
                if (V != CMD_IGNORE) {
                    // overwrite the abort command but ignore ticks in a multi-message context they shouldn't be there
                    if (V != CMD_TICK) {
                        command = NMess;
                        return V;
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
const std::string& brokerStateName(BrokerBase::broker_state_t state)
{
    static const std::string createdString = "created";
    static const std::string configuringString = "configuring";
    static const std::string configuredString = "configured";
    static const std::string connectingString = "connecting";
    static const std::string connectedString = "connected";
    static const std::string initializingString = "initializing";
    static const std::string operatingString = "operating";
    static const std::string terminatingString = "terminating";
    static const std::string terminatedString = "terminated";
    static const std::string erroredString = "error";
    static const std::string otherString = "other";
    switch (state) {
        case BrokerBase::broker_state_t::created:
            return createdString;
        case BrokerBase::broker_state_t::configuring:
            return configuringString;
        case BrokerBase::broker_state_t::configured:
            return configuredString;
        case BrokerBase::broker_state_t::connecting:
            return connectingString;
        case BrokerBase::broker_state_t::connected:
            return connectedString;
        case BrokerBase::broker_state_t::initializing:
            return initializingString;
        case BrokerBase::broker_state_t::operating:
            return operatingString;
        case BrokerBase::broker_state_t::terminating:
            return terminatingString;
        case BrokerBase::broker_state_t::terminated:
            return terminatedString;
        case BrokerBase::broker_state_t::errored:
            return erroredString;
        default:
            return otherString;
    }
}
// LCOV_EXCL_STOP

} // namespace helics
