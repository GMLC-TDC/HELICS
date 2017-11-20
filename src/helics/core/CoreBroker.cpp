/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "CoreBroker.h"
#include "BrokerFactory.h"
#include "common/stringToCmdLine.h"

#include "helics/core/argParser.h"
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "TimeCoordinator.h"
#include "helics/common/logger.h"
#include "loggingHelper.hpp"
#include <fstream>

namespace helics
{
using namespace std::string_literals;

static const argDescriptors extraArgs{
  {"root"s, ""s, "specify whether the broker is a root"s},
};

bool matchingTypes (const std::string &type1, const std::string &type2);

void CoreBroker::displayHelp ()
{
    std::cout << "Broker Specific options:\n";
    namespace po = boost::program_options;
    po::variables_map vm;
    const char *const argV[] = {"", "--help"};
    argumentParser (2, argV, vm, extraArgs);
    BrokerBase::displayHelp ();
}

CoreBroker::~CoreBroker ()
{
    std::lock_guard<std::mutex> lock (mutex_);
    // make sure everything is synchronized
}

void CoreBroker::setIdentifier (const std::string &name)
{
    if (brokerState <= broker_state_t::connecting)  // can't be changed after initialization
    {
        std::lock_guard<std::mutex> lock (mutex_);
        identifier = name;
    }
}
int32_t CoreBroker::getRoute (Core::federate_id_t fedid) const
{
    // only activate the lock if we not in an operating state
    auto fnd = routing_table.find (fedid);
    return (fnd != routing_table.end ()) ? fnd->second : 0;  // zero is the default route
}

int32_t CoreBroker::getRouteNoLock (Core::federate_id_t fedid) const
{
    auto fnd = routing_table.find (fedid);
    return (fnd != routing_table.end ()) ? fnd->second : 0;  // zero is the default route
}

int32_t CoreBroker::getFedByName (const std::string &fedName) const
{
    auto fnd = fedNames.find (fedName);
    return (fnd != fedNames.end ()) ? fnd->second : -1;
}

int32_t CoreBroker::getBrokerByName (const std::string &brokerName) const
{
    auto fnd = brokerNames.find (brokerName);
    return (fnd != brokerNames.end ()) ? fnd->second : -1;
}

int32_t CoreBroker::getBrokerById (Core::federate_id_t fedid) const
{
    if (_isRoot)
    {
        return static_cast<int32_t> (fedid - global_broker_id_shift);
    }

    auto fnd = broker_table.find (fedid);
    return (fnd != broker_table.end ()) ? fnd->second : -1;
}

int32_t CoreBroker::getFedById (Core::federate_id_t fedid) const
{
    if (_isRoot)
    {
        return static_cast<int32_t> (fedid - global_federate_id_shift);
    }

    auto fnd = federate_table.find (fedid);
    return (fnd != federate_table.end ()) ? fnd->second : -1;
}

int32_t CoreBroker::FillRouteInformation (ActionMessage &mess)
{
    auto &endpointName = mess.info ().target;
    auto fnd = endpoints.find (endpointName);
    if (fnd != endpoints.end ())
    {
        auto hinfo = _handles[fnd->second];
        mess.dest_id = hinfo.fed_id;
        mess.dest_handle = hinfo.id;
        return getRoute (hinfo.fed_id);
    }
    auto fnd2 = knownExternalEndpoints.find (endpointName);
    if (fnd2 != knownExternalEndpoints.end ())
    {
        return fnd2->second;
    }
    return 0;
}
void CoreBroker::processPriorityCommand (ActionMessage &&command)
{
    // deal with a few types of message immediately
    LOG_TRACE (
      0, getIdentifier (),
      (boost::format ("|| priority_cmd:%s from %d") % prettyPrintString (command) % command.source_id).str ());
    switch (command.action ())
    {
    case CMD_REG_FED:
    {
        if (brokerState != operating)
        {
            if (allInitReady ())
            {
                ActionMessage noInit (CMD_INIT_NOT_READY);
                noInit.source_id = global_broker_id;
                transmit (0, noInit);
            }
        }
        else  // we are initialized already
        {
            ActionMessage badInit (CMD_FED_ACK);
            badInit.error = true;
            badInit.source_id = global_broker_id;
            badInit.name = command.name;
            transmit (command.source_id, badInit);  // this isn't correct
            return;
        }
        _federates.emplace_back (command.name);
        _federates.back ().route_id = getRouteNoLock (command.source_id);

        fedNames.emplace (command.name, static_cast<int32_t> (_federates.size () - 1));
        if (!_isRoot)
        {
            if (_gateway)
            {
                ActionMessage mcopy (CMD_REG_FED);
                mcopy.name = command.name;
                mcopy.info ().target = getAddress ();
                if (global_broker_id != 0)
                {
                    mcopy.source_id = global_broker_id;
                    transmit (0, mcopy);
                }
                else
                {
                    // delay the response if we are not fully registered yet
                    delayTransmitQueue.push (mcopy);
                }
            }
            else
            {
                transmit (0, command);
            }
        }
        else
        {
            _federates.back ().global_id =
              static_cast<Core::federate_id_t> (_federates.size ()) - 1 + global_federate_id_shift;
            auto route_id = _federates.back ().route_id;
            auto global_id = _federates.back ().global_id;
            routing_table.emplace (global_id, route_id);
            // don't bother with the federate_table
            // transmit the response
            ActionMessage fedReply (CMD_FED_ACK);
            fedReply.source_id = 0;
            fedReply.dest_id = global_id;
            fedReply.name = command.name;
            transmit (route_id, fedReply);
        }
    }
    break;
    case CMD_REG_BROKER:
    {
        if (brokerState != operating)
        {
            if (allInitReady ())
            {
                // send an init not ready as we were ready now we are not
            }
        }
        else  // we are initialized already
        {
            ActionMessage badInit (CMD_BROKER_ACK);
            badInit.error = true;
            badInit.source_id = global_broker_id;
            badInit.name = command.name;
            transmit (command.source_id, badInit);
            return;
        }
        _brokers.emplace_back (command.name);
        _brokers.back ().route_id = static_cast<decltype (_brokers.back ().route_id)> (_brokers.size ());
        brokerNames.emplace (command.name, static_cast<int32_t> (_brokers.size () - 1));
        addRoute (_brokers.back ().route_id, command.info ().target);
        if (!_isRoot)
        {
            if (_gateway)
            {
                auto mcopy = command;
                mcopy.source_id = global_broker_id;
                transmit (0, mcopy);
            }
            else
            {
                transmit (0, command);
            }
        }
        else
        {
            _brokers.back ().global_id =
              static_cast<Core::federate_id_t> (_brokers.size ()) - 1 + global_broker_id_shift;
            auto global_id = _brokers.back ().global_id;
            auto route_id = _brokers.back ().route_id;
            routing_table.emplace (global_id, route_id);
            // don't bother with the broker_table for root broker

            // sending the response message
            ActionMessage brokerReply (CMD_BROKER_ACK);
            brokerReply.source_id = 0;  // source is global root
            brokerReply.dest_id = global_id;  // the new id
            brokerReply.name = command.name;  // the identifier of the broker
            transmit (route_id, brokerReply);
        }
    }
    break;
    case CMD_FED_ACK:
    {  // we can't be root if we got one of these
        auto fed_num = getFedByName (command.name);
        if (fed_num >= 0)
        {
            _federates[fed_num].global_id = command.dest_id;
            auto route = _federates[fed_num].route_id;
            federate_table.emplace (command.dest_id, fed_num);
            transmit (route, command);
        }
        else
        {
            // this means we haven't seen this federate before for some reason
            _federates.emplace_back (command.name);
            _federates.back ().route_id = getRoute (command.source_id);
            _federates.back ().global_id = command.dest_id;
            fedNames.emplace (command.name, static_cast<int32_t> (_federates.size () - 1));
            federate_table.emplace (command.dest_id, static_cast<int32_t> (_federates.size () - 1));
            // it also means we don't forward it
        }
    }
    break;
    case CMD_BROKER_ACK:
    {  // we can't be root if we got one of these
        if (command.name == identifier)
        {
            if (command.error)
            {
                // generate an error message
                return;
            }
            global_broker_id = command.dest_id;
            timeCoord->source_id = global_broker_id;
            transmitDelayedMessages ();

            return;
        }
        auto broker_num = getBrokerByName (command.name);
        if (broker_num >= 0)
        {
            _brokers[broker_num].global_id = command.dest_id;
            auto route = _brokers[broker_num].route_id;
            broker_table.emplace (command.dest_id, broker_num);
            transmit (route, command);
        }
        else
        {
            _brokers.emplace_back (command.name);
            _brokers.back ().route_id = getRoute (command.source_id);
            _brokers.back ().global_id = command.dest_id;
            brokerNames.emplace (command.name, static_cast<int32_t> (_brokers.size () - 1));
            broker_table.emplace (command.dest_id, static_cast<int32_t> (_brokers.size () - 1));
        }
    }
    break;
    case CMD_PRIORITY_DISCONNECT:
    {
        auto brkNum = getBrokerById (command.source_id);
        if (brkNum >= 0)
        {
            _brokers[brkNum]._disconnected = true;
        }
        if (allDisconnected ())
        {
            if (!_isRoot)
            {
                ActionMessage dis (CMD_PRIORITY_DISCONNECT);
                dis.source_id = global_broker_id;
                transmit (0, dis);
            }
            addActionMessage (CMD_STOP);
        }
    }
    break;
    case CMD_REG_ROUTE:
        break;
    case CMD_QUERY:
        processQuery (command);
        break;
    case CMD_QUERY_REPLY:
        transmit (getRoute (command.dest_id), command);
        break;
    default:
        // must not have been a priority command
        break;
    }
}

void CoreBroker::transmitDelayedMessages ()
{
    auto msg = delayTransmitQueue.pop ();
    while (msg)
    {
        msg->source_id = global_broker_id;
        transmit (0, *msg);
        msg = delayTransmitQueue.pop ();
    }
}

void CoreBroker::processCommand (ActionMessage &&command)
{
    LOG_TRACE (0, getIdentifier (),
               (boost::format ("|| cmd:%s from %d") % prettyPrintString (command) % command.source_id).str ());
    switch (command.action ())
    {
    case CMD_IGNORE:
    case CMD_PROTOCOL:
        break;
    case CMD_INIT:
    {
        auto brkNum = getBrokerById (command.source_id);
        if (brkNum >= 0)
        {
            std::lock_guard<std::mutex> lock (mutex_);
            _brokers[brkNum]._initRequested = true;
        }
        if (allInitReady ())
        {
            if (_isRoot)
            {
                checkSubscriptions ();
                checkEndpoints ();
                checkFilters ();
                // computeDependencies();
                ActionMessage m (CMD_INIT_GRANT);
                brokerState = broker_state_t::operating;
                for (auto &brk : _brokers)
                {
                    transmit (brk.route_id, m);
                }
                timeCoord->enteringExecMode (convergence_state::complete);
                auto res = timeCoord->checkExecEntry ();
                if (res == convergence_state::complete)
                {
                    enteredExecutionMode = true;
                    timeCoord->timeRequest (Time::maxVal (), convergence_state::complete, Time::maxVal (),
                                            Time::maxVal ());
                }
            }
            else
            {
                command.source_id = global_broker_id;
                transmit (0, command);
            }
        }
    }
    break;
    case CMD_INIT_NOT_READY:
    {
        auto brkNum = getBrokerById (command.source_id);
        if (allInitReady ())
        {
            transmit (0, command);
        }
        if (brkNum >= 0)
        {
            _brokers[brkNum]._initRequested = false;
        }
    }
    break;
    case CMD_INIT_GRANT:
        brokerState = broker_state_t::operating;
        for (auto &brk : _brokers)
        {
            transmit (brk.route_id, command);
        }
        {
            timeCoord->enteringExecMode (convergence_state::complete);
            auto res = timeCoord->checkExecEntry ();
            if (res == convergence_state::complete)
            {
                enteredExecutionMode = true;
                timeCoord->timeRequest (Time::maxVal (), convergence_state::complete, Time::maxVal (),
                                        Time::maxVal ());
            }
        }
        break;
    case CMD_DISCONNECT_NAME:
        if (command.dest_id == 0)
        {
            auto brkNum = getBrokerByName (command.payload);
            if (brkNum >= 0)
            {
                _brokers[brkNum]._disconnected = true;
            }
        }
        // FALLTHROUGH
    case CMD_DISCONNECT:
    {
        if (command.dest_id == 0)
        {
            auto brkNum = getBrokerById (command.source_id);
            if (brkNum >= 0)
            {
                _brokers[brkNum]._disconnected = true;
            }
            if (allDisconnected ())
            {
                if (!_isRoot)
                {
                    ActionMessage dis (CMD_DISCONNECT);
                    dis.source_id = global_broker_id;
                    transmit (0, dis);
                }
                addActionMessage (CMD_STOP);
            }
        }
        else
        {
            transmit (getRoute (command.dest_id), command);
        }
    }
    break;
    case CMD_STOP:
        if ((!allDisconnected ()) && (!_isRoot))
        {  // only send a disconnect message if we haven't done so already
            ActionMessage m (CMD_DISCONNECT);
            m.source_id = global_broker_id;
            transmit (0, m);
        }
        break;
    case CMD_EXEC_REQUEST:
    case CMD_EXEC_GRANT:
        if (command.dest_id == global_broker_id)
        {
            timeCoord->processTimeMessage (command);
            if (!enteredExecutionMode)
            {
                auto res = timeCoord->checkExecEntry ();
                if (res == convergence_state::complete)
                {
                    enteredExecutionMode = true;
                    timeCoord->timeRequest (Time::maxVal (), convergence_state::complete, Time::maxVal (),
                                            Time::maxVal ());
                }
            }
        }
        else if (command.source_id == global_broker_id)
        {
            for (auto dep : timeCoord->getDependents ())
            {
                command.dest_id = dep;
                transmit (getRoute (dep), command);
            }
        }
        else
        {
            transmit (getRoute (command.dest_id), command);
        }

        break;
    case CMD_TIME_REQUEST:
    case CMD_TIME_GRANT:
        if (command.source_id == global_broker_id)
        {
            for (auto dep : timeCoord->getDependents ())
            {
                command.dest_id = dep;
                transmit (getRoute (dep), command);
            }
        }
        else if (command.dest_id == global_broker_id)
        {
            if (timeCoord->processTimeMessage (command))
            {
                timeCoord->checkTimeGrant ();
            }
        }
        else
        {
            transmit (getRoute (command.dest_id), command);
        }

        break;
    case CMD_SEND_MESSAGE:
    case CMD_SEND_FOR_FILTER:
        if (command.dest_id == 0)
        {
            auto route = FillRouteInformation (command);
            transmit (route, command);
        }
        else
        {
            transmit (getRoute (command.dest_id), command);
        }

        break;

    case CMD_PUB:
        transmit (getRoute (command.dest_id), command);
        break;

    case CMD_LOG:
        if (_isRoot)
        {
            // do some logging
        }
        else
        {
            transmit (0, command);
        }
        break;
    case CMD_ERROR:
        if (_isRoot)
        {
            // do some logging
        }
        else
        {
            transmit (0, command);
        }
        break;
    case CMD_REG_PUB:
        if (!_isRoot)
        {
            if (command.dest_id != 0)
            {
                auto rt = getRoute (command.dest_id);
                transmit (rt, command);
                break;
            }
        }
        addPublication (command);
        break;
    case CMD_REG_SUB:
        if (!_isRoot)
        {
            if (command.dest_id != 0)
            {
                auto rt = getRoute (command.dest_id);
                transmit (rt, command);
                break;
            }
        }
        addSubscription (command);
        break;
    case CMD_REG_END:
        if (!_isRoot)
        {
            if (command.dest_id != 0)
            {
                auto rt = getRoute (command.dest_id);
                transmit (rt, command);
                break;
            }
        }
        addEndpoint (command);
        break;
    case CMD_REG_DST_FILTER:
        if (!_isRoot)
        {
            if (command.dest_id != 0)
            {
                auto rt = getRoute (command.dest_id);
                transmit (rt, command);
                break;
            }
        }
        addDestFilter (command);
        break;
    case CMD_REG_SRC_FILTER:
        if (!_isRoot)
        {
            if (command.dest_id != 0)
            {
                auto rt = getRoute (command.dest_id);
                transmit (rt, command);
                break;
            }
        }
        addSourceFilter (command);
        break;
    case CMD_SRC_FILTER_HAS_OPERATOR:
        if (command.dest_id != 0)
        {
            auto rt = getRoute (command.dest_id);
            transmit (rt, command);
            break;
        }
        {
            if ((!updateSourceFilterOperator (command)) && (!_isRoot))
            {
                transmit (0, command);
            }
        }

        break;
    case CMD_ADD_DEPENDENCY:
        if (command.dest_id != global_broker_id)
        {
            auto rt = getRoute (command.dest_id);
            transmit (rt, command);
        }
        else
        {
            timeCoord->addDependency (command.source_id);
        }
        break;
    case CMD_ADD_DEPENDENT:
        if (command.dest_id != global_broker_id)
        {
            auto rt = getRoute (command.dest_id);
            transmit (rt, command);
        }
        else
        {
            timeCoord->addDependent (command.source_id);
        }
        break;
    case CMD_REMOVE_DEPENDENT:
        if (command.dest_id != global_broker_id)
        {
            auto rt = getRoute (command.dest_id);
            transmit (rt, command);
        }
        else
        {
            timeCoord->removeDependent (command.source_id);
        }
        break;
    case CMD_REMOVE_DEPENDENCY:
        if (command.dest_id != global_broker_id)
        {
            auto rt = getRoute (command.dest_id);
            transmit (rt, command);
        }
        else
        {
            timeCoord->removeDependency (command.source_id);
        }
        break;
    default:
        // check again if it is a priority command and if so process it in that function
        if (command.dest_id != global_broker_id)
        {
            auto rt = getRoute (command.dest_id);
            transmit (rt, command);
        }
    }
}

void CoreBroker::addLocalInfo (BasicHandleInfo &handleInfo, const ActionMessage &m)
{
    auto res = global_id_translation.find (m.source_id);
    if (res != global_id_translation.end ())
    {
        handleInfo.local_fed_id = res->second;
    }
}

void CoreBroker::addPublication (ActionMessage &m)
{
    _handles.emplace_back (m.source_handle, m.source_id, HANDLE_PUB, m.name, m.info ().type, m.info ().units);

    publications.emplace (m.name, static_cast<int32_t> (_handles.size () - 1));
    handle_table.emplace (makeGlobalHandleIdentifier (m.source_id, m.source_handle),
                          static_cast<int32_t> (_handles.size () - 1));
    addLocalInfo (_handles.back (), m);
    if (!_isRoot)
    {
        transmit (0, m);
    }
    else
    {
        FindandNotifyPublicationSubscribers (_handles.back ());
    }
}
void CoreBroker::addSubscription (ActionMessage &m)
{
    _handles.emplace_back (m.source_handle, m.source_id, HANDLE_SUB, m.name, m.info ().type, m.info ().units);
    handle_table.emplace (makeGlobalHandleIdentifier (m.source_id, m.source_handle),
                          static_cast<int32_t> (_handles.size () - 1));
    addLocalInfo (_handles.back (), m);
    subscriptions.emplace (m.name, static_cast<int32_t> (_handles.size () - 1));
    _handles.back ().processed = m.processingComplete;
    if (!m.processingComplete)
    {
        bool proc = FindandNotifySubscriptionPublisher (_handles.back ());
        if (!_isRoot)
        {
            // just let any higher level brokers know we have found the publisher and let them know
            m.processingComplete = proc;
            transmit (0, m);
        }
    }
}

void CoreBroker::addEndpoint (ActionMessage &m)
{
    _handles.emplace_back (m.source_handle, m.source_id, HANDLE_END, m.name, m.info ().type, m.info ().units);
    endpoints.emplace (m.name, static_cast<int32_t> (_handles.size () - 1));
    handle_table.emplace (makeGlobalHandleIdentifier (m.source_id, m.source_handle),
                          static_cast<int32_t> (_handles.size () - 1));
    addLocalInfo (_handles.back (), m);

    bool addDep = (!m.processingComplete);
    if (!_isRoot)
    {
        m.processingComplete = true;
        transmit (0, m);
    }
    else
    {
        FindandNotifyEndpointFilters (_handles.back ());
    }

    if (addDep)
    {
        bool added = timeCoord->addDependency (m.source_id);
        if (added)
        {
            ActionMessage add (CMD_ADD_DEPENDENCY);
            add.source_id = global_broker_id;
            add.dest_id = m.source_id;
            auto rt = getRoute (m.source_id);
            transmit (rt, add);
            add.setAction (CMD_ADD_DEPENDENT);
            transmit (rt, add);
            timeCoord->addDependent (m.source_id);
        }
    }
}
void CoreBroker::addSourceFilter (ActionMessage &m)
{
    _handles.emplace_back (m.source_handle, m.source_id, HANDLE_SOURCE_FILTER, m.name, m.info ().target,
                           m.info ().type, m.info ().type_out);
    addLocalInfo (_handles.back (), m);
    handle_table.emplace (makeGlobalHandleIdentifier (m.source_id, m.source_handle),
                          static_cast<int32_t> (_handles.size () - 1));
    bool proc = FindandNotifyFilterEndpoint (_handles.back ());
    if (!_isRoot)
    {
        m.processingComplete = proc;
        transmit (0, m);
    }
}

bool CoreBroker::updateSourceFilterOperator (ActionMessage &m)
{
    auto hndl_fnd = handle_table.find (makeGlobalHandleIdentifier (m.source_id, m.source_handle));
    if (hndl_fnd != handle_table.end ())
    {
        _handles[hndl_fnd->second].flag = true;

        auto endHandle = endpoints.find (_handles[hndl_fnd->second].target);
        if (endHandle != endpoints.end ())
        {
            auto &endInfo = _handles[endHandle->second];

            m.dest_id = endInfo.fed_id;
            m.dest_handle = endInfo.id;

            transmit (getRoute (m.dest_id), m);
            return true;
        }
    }
    return false;
}

void CoreBroker::addDestFilter (ActionMessage &m)
{
    _handles.emplace_back (m.source_handle, m.source_id, HANDLE_DEST_FILTER, m.name, m.info ().target,
                           m.info ().type, m.info ().type_out);
    addLocalInfo (_handles.back (), m);
    handle_table.emplace (makeGlobalHandleIdentifier (m.source_id, m.source_handle),
                          static_cast<int32_t> (_handles.size () - 1));
    bool proc = FindandNotifyFilterEndpoint (_handles.back ());
    if (!_isRoot)
    {
        m.processingComplete = proc;
        transmit (0, m);
    }
}

CoreBroker::CoreBroker (bool setAsRootBroker) noexcept : _isRoot (setAsRootBroker) {}

CoreBroker::CoreBroker (const std::string &broker_name) : BrokerBase (broker_name) {}

void CoreBroker::Initialize (const std::string &initializationString)
{
    if (brokerState == broker_state_t::created)
    {
        stringToCmdLine cmdline (initializationString);
        InitializeFromArgs (cmdline.getArgCount (), cmdline.getArgV ());
    }
}

void CoreBroker::InitializeFromArgs (int argc, const char *const *argv)
{
    broker_state_t exp = broker_state_t::created;
    if (brokerState.compare_exchange_strong (exp, broker_state_t::initialized))
    {
        namespace po = boost::program_options;

        po::variables_map vm;
        argumentParser (argc, argv, vm, extraArgs);
        BrokerBase::InitializeFromArgs (argc, argv);

        if (vm.count ("root") > 0)
        {
            setAsRoot ();
        }
    }
}

void CoreBroker::setAsRoot ()
{
    if (brokerState < broker_state_t::connected)
    {
        _isRoot = true;
        global_broker_id = 1;
    }
}

bool CoreBroker::connect ()
{
    if (brokerState < broker_state_t::connected)
    {
        broker_state_t exp = broker_state_t::initialized;
        if (brokerState.compare_exchange_strong (exp, broker_state_t::connecting))
        {
            LOG_NORMAL (0, getIdentifier (), "connecting");
            auto res = brokerConnect ();
            if (res)
            {
                LOG_NORMAL (0, getIdentifier (), (boost::format ("||connected on %s") % getAddress ()).str ());
                if (!_isRoot)
                {
                    ActionMessage m (CMD_REG_BROKER);
                    m.name = getIdentifier ();
                    m.info ().target = getAddress ();
                    transmit (0, m);
                }
                else
                {
                    timeCoord->source_id = global_broker_id;
                }
                brokerState = broker_state_t::connected;
            }
            else
            {
                brokerState = broker_state_t::initialized;
            }
            return res;
        }
        if (brokerState == broker_state_t::connecting)
        {
            while (brokerState == broker_state_t::connecting)
            {
                std::this_thread::sleep_for (std::chrono::milliseconds (20));
            }
        }
    }
    return isConnected ();
}

bool CoreBroker::isConnected () const { return ((brokerState == operating) || (brokerState == connected)); }

void CoreBroker::processDisconnect () { disconnect (); }

void CoreBroker::disconnect (bool skipUnregister)
{
    if (brokerState == broker_state_t::terminated)
    {
        return;
    }
    LOG_NORMAL (0, getIdentifier (), "||disconnecting");
    if (brokerState > broker_state_t::initialized)
    {
        brokerState = broker_state_t::terminating;
        brokerDisconnect ();
    }
    brokerState = broker_state_t::terminated;
    if (skipUnregister)
    {
        return;
    }
    /*We need to ensure that the destructor is not called immediately upon calling unregister
    otherwise this would be a mess and probably cause segmentation faults so we capture it in a local variable
    that will be destroyed on function exit
    */
    auto keepBrokerAlive = BrokerFactory::findBroker (identifier);
    if (keepBrokerAlive)
    {
        BrokerFactory::unregisterBroker (identifier);
    }
    if (!previous_local_broker_identifier.empty ())
    {
        auto keepBrokerAlive2 = BrokerFactory::findBroker (previous_local_broker_identifier);
        if (keepBrokerAlive2)
        {
            BrokerFactory::unregisterBroker (previous_local_broker_identifier);
        }
    }
}

bool CoreBroker::FindandNotifySubscriptionPublisher (BasicHandleInfo &handleInfo)
{
    if (!handleInfo.processed)
    {
        auto pubHandle = publications.find (handleInfo.key);
        if (pubHandle != publications.end ())
        {
            auto &pubInfo = _handles[pubHandle->second];
            if (!matchingTypes (pubInfo.type, handleInfo.type))
            {
                // LOG(WARN) << "sub " << hndl->key << " does not match types" << hndl->type << " " <<
                // pubInfo->type << ENDL;
            }
            // notify the subscription about its publisher
            ActionMessage m (CMD_NOTIFY_PUB);
            m.source_id = pubInfo.fed_id;
            m.source_handle = pubInfo.id;
            m.dest_id = handleInfo.fed_id;
            m.dest_handle = handleInfo.id;
            m.payload = pubInfo.type;
            transmit (getRoute (m.dest_id), m);

            // notify the publisher about its subscription
            m.setAction (CMD_NOTIFY_SUB);
            m.source_id = handleInfo.fed_id;
            m.source_handle = handleInfo.id;
            m.dest_id = pubInfo.fed_id;
            m.dest_handle = pubInfo.id;

            transmit (getRoute (m.dest_id), m);

            handleInfo.processed = true;
        }
    }
    return handleInfo.processed;
}

void CoreBroker::FindandNotifyPublicationSubscribers (BasicHandleInfo &handleInfo)
{
    auto subHandles = subscriptions.equal_range (handleInfo.key);
    for (auto sub = subHandles.first; sub != subHandles.second; ++sub)
    {
        auto &subInfo = _handles[sub->second];
        if (subInfo.processed)
        {
            continue;
        }
        if (!matchingTypes (subInfo.type, handleInfo.type))
        {
            LOG_WARNING (global_broker_id, handleInfo.key,
                         std::string ("types do not match ") + handleInfo.type + " vs " + subInfo.type);
        }
        // notify the publication about its subscriber
        ActionMessage m (CMD_NOTIFY_SUB);
        m.source_id = subInfo.fed_id;
        m.source_handle = subInfo.id;
        m.dest_id = handleInfo.fed_id;
        m.dest_handle = handleInfo.id;

        transmit (getRoute (m.dest_id), m);

        // notify the subscriber about its publisher
        m.setAction (CMD_NOTIFY_PUB);
        m.source_id = handleInfo.fed_id;
        m.source_handle = handleInfo.id;
        m.dest_id = subInfo.fed_id;
        m.dest_handle = subInfo.id;
        m.payload = handleInfo.type;
        transmit (getRoute (m.dest_id), m);
        subInfo.processed = true;
    }
}

bool CoreBroker::FindandNotifyFilterEndpoint (BasicHandleInfo &handleInfo)
{
    if (!handleInfo.processed)
    {
        auto endHandle = endpoints.find (handleInfo.target);
        if (endHandle != endpoints.end ())
        {
            auto &endInfo = _handles[endHandle->second];
            if (!matchingTypes (endInfo.type, handleInfo.type))
            {
                // LOG(WARN) << "sub " << hndl->key << " does not match types" << hndl->type << " " <<
                // pubInfo->type << ENDL;
            }
            // notify the filter about its endpoint
            ActionMessage m (CMD_NOTIFY_END);
            m.source_id = endInfo.fed_id;
            m.source_handle = endInfo.id;
            m.dest_id = handleInfo.fed_id;
            m.dest_handle = handleInfo.id;

            transmit (getRoute (m.dest_id), m);

            // notify the endpoint about its filter
            m.setAction ((handleInfo.what == HANDLE_SOURCE_FILTER) ? CMD_NOTIFY_SRC_FILTER :
                                                                     CMD_NOTIFY_DST_FILTER);
            m.source_id = handleInfo.fed_id;
            m.source_handle = handleInfo.id;
            m.dest_id = endInfo.fed_id;
            m.dest_handle = endInfo.id;

            transmit (getRoute (m.dest_id), m);

            handleInfo.processed = true;
        }
    }
    return handleInfo.processed;
}

void CoreBroker::FindandNotifyEndpointFilters (BasicHandleInfo &handleInfo)
{
    auto filtHandles = filters.equal_range (handleInfo.target);
    for (auto filt = filtHandles.first; filt != filtHandles.second; ++filt)
    {
        auto &filtInfo = _handles[filt->second];
        if (filtInfo.processed)
        {
            continue;
        }
        if (!matchingTypes (filtInfo.type, handleInfo.type))
        {
            // LOG(WARN) << "sub " << hndl->key << " does not match types" << hndl->type << " " <<
            // pubInfo->type << ENDL;
        }
        // notify the endpoint about a filter
        ActionMessage m ((handleInfo.what == HANDLE_SOURCE_FILTER) ? CMD_NOTIFY_SRC_FILTER :
                                                                     CMD_NOTIFY_DST_FILTER);
        m.source_id = filtInfo.fed_id;
        m.source_handle = filtInfo.id;
        m.dest_id = handleInfo.fed_id;
        m.dest_handle = handleInfo.id;
        m.flag = handleInfo.flag;
        transmit (getRoute (m.dest_id), m);

        // notify the publisher about its subscription
        m.setAction (CMD_NOTIFY_END);
        m.source_id = handleInfo.fed_id;
        m.source_handle = handleInfo.id;
        m.dest_id = filtInfo.fed_id;
        m.dest_handle = filtInfo.id;

        transmit (getRoute (m.dest_id), m);
        filtInfo.processed = true;
    }
}

void CoreBroker::checkSubscriptions ()
{
    // pub/sub checks
    // LOG(INFO) << "performing pub/sub check" << ENDL;
    for (auto &hndl : _handles)
    {
        if (hndl.what == HANDLE_SUB)
        {
            if (!hndl.processed)
            {
                auto fnd = FindandNotifySubscriptionPublisher (hndl);
                if ((!fnd) && (hndl.flag))
                {
                    // LOG(WARN) << "sub " << hndl->key << " has no corresponding pub" << ENDL;
                }
            }
        }
    }
}

std::string CoreBroker::generateQueryAnswer (const std::string &query) const
{
    if (query == "isinit")
    {
        return (brokerState >= broker_state_t::operating) ? "true" : "false";
    }
    else if (query == "federates")
    {
        std::string ret;
        ret.push_back ('[');
        for (auto &fed : _federates)
        {
            ret.append (fed.name);
            ret.push_back (';');
        }
        if (ret.size () > 1)
        {
            ret.back () = ']';
        }
        else
        {
            ret.push_back (']');
        }

        return ret;
    }
    else if (query == "brokers")
    {
        std::string ret;
        ret.push_back ('[');
        for (auto &brk : _brokers)
        {
            ret.append (brk.name);
            ret.push_back (';');
        }
        if (ret.size () > 1)
        {
            ret.back () = ']';
        }
        else
        {
            ret.push_back (']');
        }
        return ret;
    }
    else
    {
        return "#invalid";
    }
}

void CoreBroker::processLocalQuery (const ActionMessage &m)
{
    ActionMessage queryRep (CMD_QUERY_REPLY);
    queryRep.dest_id = m.source_id;
    queryRep.source_id = global_broker_id;
    queryRep.index = m.index;
    queryRep.payload = generateQueryAnswer (m.payload);
    transmit (getRoute (m.source_id), queryRep);
}

void CoreBroker::processQuery (const ActionMessage &m)
{
    if ((m.info ().target == getIdentifier ()) || (m.info ().target == "broker"))
    {
    }
    else if ((isRoot ()) && ((m.info ().target == "root") || (m.info ().target == "federation")))
    {
    }
    else
    {
        int32_t route = 0;
        auto res = getFedByName (m.info ().target);
        if (res != -1)
        {
            auto &fed = _federates[res];
            route = fed.route_id;
        }
        else
        {
            res = getBrokerByName (m.info ().target);
            if (res != -1)
            {
                auto &brk = _brokers[res];
                route = brk.route_id;
            }
        }

        transmit (route, m);
    }
}

void CoreBroker::checkEndpoints () {}

void CoreBroker::checkFilters ()
{
    // LOG(INFO) << "performing filter check" << ENDL;
    for (auto &hndl : _handles)
    {
        if ((hndl.what == HANDLE_DEST_FILTER) || (hndl.what == HANDLE_SOURCE_FILTER))
        {
            auto fnd = FindandNotifyFilterEndpoint (hndl);
            if (!fnd)
            {
                // LOG(WARN) << "sub " << hndl->key << " has no corresponding pub" << ENDL;
            }
        }
    }
}

bool CoreBroker::allInitReady () const
{
    // the federate count must be greater than the min size
    if (static_cast<decltype (_min_federates)> (_federates.size ()) < _min_federates)
    {
        return false;
    }
    if (static_cast<decltype (_min_brokers)> (_brokers.size ()) < _min_brokers)
    {
        return false;
    }

    return std::all_of (_brokers.begin (), _brokers.end (), [](auto &brk) { return brk._initRequested; });
}

bool CoreBroker::allDisconnected () const
{
    return std::all_of (_brokers.begin (), _brokers.end (), [](auto &brk) { return brk._disconnected; });
}

bool matchingTypes (const std::string &type1, const std::string &type2)
{
    if (type1 == type2)
    {
        return true;
    }
    if ((type1.empty ()) || (type2.empty ()))
    {
        return true;
    }
    if ((type1.compare(0,3,"def")==0) || (type2.compare(0, 3, "def") == 0))
    {
        return true;
    }
    if ((type1 == "block") || (type2 == "block"))
    {
        return true;
    }
    if ((type1 == "string") || (type2 == "string"))
    {
        return true;
    }
    if ((type1 == "data") || (type2 == "data"))
    {
        return true;
    }
    return false;
}
}  // namespace helics
