/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "BrokerBase.hpp"

#include "../common/AsioServiceManager.h"
#include "../common/argParser.h"
#include "../common/logger.h"

#include "../common/fmt_format.h"
#include "ForwardingTimeCoordinator.hpp"
#include "flagOperations.hpp"
#include "helics/helics-config.h"
#include "helicsVersion.hpp"
#include <iostream>
#include <libguarded/guarded.hpp>
#include <random>
#include <boost/asio/steady_timer.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

static constexpr auto chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

static inline std::string genId ()
{
    std::string nm = std::string (23, ' ');
    std::random_device rd;  // only used once to initialize (seed) engine
    std::mt19937 rng (rd ());  // random-number engine used (Mersenne-Twister in this case)
    std::uniform_int_distribution<int> uni (0, 61);  // guaranteed unbiased

    nm[5] = '-';
    nm[11] = '-';
    nm[17] = '-';

    for (int ii = 0; ii < 23; ii++)
    {
        if ((ii != 5) && (ii != 11) && (ii != 17))
        {
            nm[ii] = chars[uni (rng)];
        }
    }
#ifdef _WIN32
    std::string pid_str = std::to_string (GetCurrentProcessId ());
#else
    std::string pid_str = std::to_string (getpid ());
#endif
    return pid_str + "-" + nm;
}

namespace helics
{
BrokerBase::BrokerBase (bool DisableQueue) noexcept : queueDisabled (DisableQueue) {}

BrokerBase::BrokerBase (const std::string &broker_name, bool DisableQueue)
    : identifier (broker_name), queueDisabled (DisableQueue)
{
}

BrokerBase::~BrokerBase ()
{
    if (!queueDisabled)
    {
        joinAllThreads ();
    }
}

void BrokerBase::joinAllThreads ()
{
    if ((!queueDisabled) && (queueProcessingThread.joinable ()))
    {
        actionQueue.push (CMD_TERMINATE_IMMEDIATELY);
        queueProcessingThread.join ();
    }
}

static const ArgDescriptors extraArgs{
  {"name,n", "name of the broker/core"},
  {"federates", ArgDescriptor::arg_type_t::int_type, "the minimum number of federates that will be connecting"},
  {"minfed", ArgDescriptor::arg_type_t::int_type, "the minimum number of federates that will be connecting"},
  {"maxiter", ArgDescriptor::arg_type_t::int_type, "maximum number of iterations"},
  {"logfile", "the file to log message to"},
  {"loglevel", ArgDescriptor::arg_type_t::int_type,
   "the level which to log the higher this is set to the more gets logs (-1) for no logging"},
  {"log_level", ArgDescriptor::arg_type_t::int_type,
   "the level which to log the higher this is set to the more gets logs (-1) for no logging"},
  {"fileloglevel", ArgDescriptor::arg_type_t::int_type, "the level at which messages get sent to the file"},
  {"consoleloglevel", ArgDescriptor::arg_type_t::int_type, "the level at which message get sent to the console"},
  {"minbrokers", ArgDescriptor::arg_type_t::int_type,
   "the minimum number of core/brokers that need to be connected (ignored in cores)"},
  {"identifier", "name of the core/broker"},
  {"tick", "number of milliseconds per tick counter if there is no broker communication for 2 ticks then "
           "secondary actions are taken  (can also be entered as a time like '10s' or '45ms')"},
  {"dumplog", ArgDescriptor::arg_type_t::flag_type,
   "capture a record of all messages and dump a complete log to file or console on termination"},
  {"networktimeout",
   "milliseconds to wait to establish a network (can also be entered as a time like '500ms' or '2s') "},
  {"timeout",
   "milliseconds to wait for a broker connection (can also be entered as a time like '10s' or '45ms') "}};

void BrokerBase::displayHelp ()
{
    std::cout << " Global options for all Brokers:\n";
    variable_map vm;
    const char *const argV[] = {"", "-?"};
    argumentParser (2, argV, vm, extraArgs);
}

void BrokerBase::initializeFromCmdArgs (int argc, const char *const *argv)
{
    variable_map vm;
    argumentParser (argc, argv, vm, extraArgs, "min");
    if (vm.count ("min") > 0)
    {
        try
        {
            minFederateCount = std::stod (vm["min"].as<std::string> ());
        }
        catch (const std::invalid_argument &ia)
        {
            std::cerr << vm["min"].as<std::string> () << " is not a valid minimum federate count\n";
        }
    }
    if (vm.count ("minfed") > 0)
    {
        minFederateCount = vm["minfed"].as<int> ();
    }
    if (vm.count ("federates") > 0)
    {
        minFederateCount = vm["federates"].as<int> ();
    }
    if (vm.count ("minbrokers") > 0)
    {
        minBrokerCount = vm["minbrokers"].as<int> ();
    }
    if (vm.count ("maxiter") > 0)
    {
        maxIterationCount = vm["maxiter"].as<int> ();
    }

    if (vm.count ("name") > 0)
    {
        identifier = vm["name"].as<std::string> ();
    }

    if (vm.count ("dumplog") > 0)
    {
        dumplog = true;
    }
    if (vm.count ("identifier") > 0)
    {
        identifier = vm["identifier"].as<std::string> ();
    }
    if (vm.count ("loglevel") > 0)
    {
        maxLogLevel = vm["loglevel"].as<int> ();
    }
    if (vm.count ("log_level") > 0)
    {
        maxLogLevel = vm["log_level"].as<int> ();
    }
    if (vm.count ("logfile") > 0)
    {
        logFile = vm["logfile"].as<std::string> ();
    }
    if (vm.count ("networktimeout") > 0)
    {
        auto network_to = loadTimeFromString (vm["timeout"].as<std::string> (), timeUnits::ms);
        networkTimeout = network_to.toCount (timeUnits::ms);
    }
    if (vm.count ("timeout") > 0)
    {
        auto time_out = loadTimeFromString (vm["timeout"].as<std::string> (), timeUnits::ms);
        timeout = time_out.toCount (timeUnits::ms);
        if (networkTimeout < 0)
        {
            networkTimeout = timeout;
        }
    }
    if (networkTimeout < 0)
    {
        networkTimeout = 4000;
    }
    if (vm.count ("tick") > 0)
    {
        auto time_tick = loadTimeFromString (vm["tick"].as<std::string> (), timeUnits::ms);
        tickTimer = time_tick.toCount (timeUnits::ms);
    }
    if (!noAutomaticID)
    {
        if (identifier.empty ())
        {
            identifier = genId ();
        }
    }

    timeCoord = std::make_unique<ForwardingTimeCoordinator> ();
    timeCoord->setMessageSender ([this](const ActionMessage &msg) { addActionMessage (msg); });

    loggingObj = std::make_unique<Logger> ();
    if (!logFile.empty ())
    {
        loggingObj->openFile (logFile);
    }
    loggingObj->startLogging (maxLogLevel, maxLogLevel);
    queueProcessingThread = std::thread (&BrokerBase::queueProcessingLoop, this);
}

bool BrokerBase::sendToLogger (global_federate_id_t federateID,
                               int logLevel,
                               const std::string &name,
                               const std::string &message) const
{
    if ((federateID == parent_broker_id) || (federateID == global_broker_id.load ()))
    {
        if (logLevel > maxLogLevel)
        {
            // check the logging level
            return true;
        }
        if (loggerFunction)
        {
            loggerFunction (logLevel, fmt::format ("{} ({})", name, federateID.baseValue ()), message);
        }
        else if (loggingObj)
        {
            loggingObj->log (logLevel, fmt::format ("{} ({})::{}", name, federateID.baseValue (), message));
        }
        return true;
    }
    return false;
}

void BrokerBase::generateNewIdentifier () { identifier = genId (); }

void BrokerBase::setLoggerFunction (std::function<void(int, const std::string &, const std::string &)> logFunction)
{
    loggerFunction = std::move (logFunction);
    if (loggerFunction)
    {
        if (loggingObj)
        {
            if (loggingObj->isRunning ())
            {
                loggingObj->haltLogging ();
            }
        }
    }
    else if (!loggingObj->isRunning ())
    {
        loggingObj->startLogging ();
    }
}

void BrokerBase::setLogLevel (int32_t level) { setLogLevels (level, level); }

/** set the logging levels
@param consoleLevel the logging level for the console display
@param fileLevel the logging level for the log file
*/
void BrokerBase::setLogLevels (int32_t consoleLevel, int32_t fileLevel)
{
    consoleLogLevel = consoleLevel;
    fileLogLevel = fileLevel;
    maxLogLevel = std::max (consoleLogLevel, fileLogLevel);
    if (loggingObj)
    {
        loggingObj->changeLevels (consoleLogLevel, fileLogLevel);
    }
}

void BrokerBase::addActionMessage (const ActionMessage &m)
{
    if (isPriorityCommand (m))
    {
        actionQueue.pushPriority (m);
    }
    else
    {
        // just route to the general queue;
        actionQueue.push (m);
    }
}

void BrokerBase::addActionMessage (ActionMessage &&m)
{
    if (isPriorityCommand (m))
    {
        actionQueue.emplacePriority (std::move (m));
    }
    else
    {
        // just route to the general queue;
        actionQueue.emplace (std::move (m));
    }
}

using activeProtector = libguarded::guarded<std::pair<bool, bool>>;

static void haltTimer (activeProtector &active, boost::asio::steady_timer &tickTimer)
{
    bool TimerRunning = true;
    {
        auto p = active.lock ();
        if (p->second)
        {
            p->first = false;
            tickTimer.cancel ();
        }
        else
        {
            TimerRunning = false;
        }
    }
    while (TimerRunning)
    {
        std::this_thread::yield ();
        auto res = active.load ();
        TimerRunning = res.second;
    }
}

static void timerTickHandler (BrokerBase *bbase, activeProtector &active, const boost::system::error_code &error)
{
    auto p = active.lock ();
    if (p->first)
    {
        if (error != boost::asio::error::operation_aborted)
        {
            try
            {
                bbase->addActionMessage (CMD_TICK);
            }
            catch (std::exception &e)
            {
                std::cout << "exception caught from addActionMessage" << std::endl;
            }
        }
        else
        {
            ActionMessage M (CMD_TICK);
            setActionFlag (M, error_flag);
            bbase->addActionMessage (M);
        }
    }
    p->second = false;
}

bool BrokerBase::tryReconnect () { return false; }

//#define DISABLE_TICK
void BrokerBase::queueProcessingLoop ()
{
    std::vector<ActionMessage> dumpMessages;
    mainLoopIsRunning.store (true);
    auto serv = AsioServiceManager::getServicePointer ();
    auto serviceLoop = AsioServiceManager::runServiceLoop ();
    boost::asio::steady_timer ticktimer (serv->getBaseService ());
    activeProtector active (true, false);

    auto timerCallback = [this, &active](const boost::system::error_code &ec) {
        timerTickHandler (this, active, ec);
    };
    ticktimer.expires_at (std::chrono::steady_clock::now () + std::chrono::milliseconds (tickTimer));
    active = std::make_pair (true, true);
    ticktimer.async_wait (timerCallback);
    global_broker_id_local = global_broker_id.load ();
    int messagesSinceLastTick = 0;
    auto logDump = [&, this]() {
        if (dumplog)
        {
            for (auto &act : dumpMessages)
            {
                sendToLogger (parent_broker_id, -10, identifier,
                              fmt::format ("|| dl cmd:{} from {} to {}", prettyPrintString (act),
                                           act.source_id.baseValue (), act.dest_id.baseValue ()));
            }
        }
    };

    while (true)
    {
        auto command = actionQueue.pop ();
        if (dumplog)
        {
            dumpMessages.push_back (command);
        }
        auto ret = commandProcessor (command);
		if (ret == CMD_IGNORE)
		{
            continue;
		}
        switch (ret)
        {
        case CMD_TICK:
            if (checkActionFlag (command, error_flag))
            {
                serviceLoop = nullptr;
                serviceLoop = AsioServiceManager::runServiceLoop ();
            }
            if (messagesSinceLastTick == 0)
            {
#ifndef DISABLE_TICK
                //   std::cout << "sending tick " << std::endl;
                processCommand (std::move (command));
#endif
            }
            messagesSinceLastTick = 0;
            // reschedule the timer
            ticktimer.expires_at (std::chrono::steady_clock::now () + std::chrono::milliseconds (tickTimer));
            active = std::make_pair (true, true);
            ticktimer.async_wait (timerCallback);
            break;
        case CMD_IGNORE:
        default:
            break;
        case CMD_TERMINATE_IMMEDIATELY:
            haltTimer (active, ticktimer);
            serviceLoop = nullptr;
            mainLoopIsRunning.store (false);
            logDump ();
            return;  // immediate return
        case CMD_STOP:
            haltTimer (active, ticktimer);
            serviceLoop = nullptr;
            if (!haltOperations)
            {
                processCommand (std::move (command));
                mainLoopIsRunning.store (false);
                logDump ();
                processDisconnect ();
            }
            return;
        
        }
    }
}

action_message_def::action_t BrokerBase::commandProcessor (ActionMessage &command)
{
    switch (command.action ())
    {
    case CMD_IGNORE:
        break;
    case CMD_TERMINATE_IMMEDIATELY:
    case CMD_STOP:
    case CMD_TICK:
        return command.action ();
    case CMD_MULTI_MESSAGE:
        for (int ii = 0; ii < command.counter; ++ii)
        {
            ActionMessage NMess;
            NMess.from_string (command.getString (ii));
            auto V = commandProcessor (NMess);
			if (V != CMD_IGNORE)
			{
				//overwrite the abort command but ignore ticks in a multimessage context they shouldn't be there
				if (V != CMD_TICK)
				{
                    command = NMess;
                    return V;
				}
			}
        }
        break;
    default:
        if (!haltOperations)
        {
            if (isPriorityCommand (command))
            {
                processPriorityCommand (std::move (command));
            }
            else
            {
                processCommand (std::move (command));
            }
        }
    }
    return CMD_IGNORE;
}

}  // namespace helics
