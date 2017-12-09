/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "CoreBroker.h"
#include "../common/stringToCmdLine.h"
#include "BrokerFactory.h"

#include "argParser.h"
#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include "../common/logger.h"
#include "TimeCoordinator.h"
#include "loggingHelper.hpp"
#include <fstream>
#include <json/json.h>

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
        auto brkNum = static_cast<int32_t> (fedid - global_broker_id_shift);
        return (brkNum < static_cast<int32_t> (_brokers.size ())) ? brkNum : (-1);
    }

    auto fnd = broker_table.find (fedid);
    return (fnd != broker_table.end ()) ? fnd->second : (-1);
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

void CoreBroker::generateQueryResult (const ActionMessage &command)
{
    std::string repStr;
    bool listV = true;
    if (command.payload == "federates")
    {
        repStr.push_back ('[');
        for (const auto &fed : _federates)
        {
            repStr.append (fed.name);
            repStr.push_back (';');
        }
    }
    else if (command.payload == "publications")
    {
        repStr.push_back ('[');
        for (const auto &pub : publications)
        {
            repStr.append (pub.first);
            repStr.push_back (';');
        }
    }
    else if (command.payload == "endpoints")
    {
        repStr.push_back ('[');
        for (const auto &ept : endpoints)
        {
            repStr.append (ept.first);
            repStr.push_back (';');
        }
    }
    else if (command.payload == "brokers")
    {
        repStr.push_back ('[');
        for (const auto &brk : _brokers)
        {
            repStr.append (brk.name);
            repStr.push_back (';');
        }
    }
    else
    {
        repStr = "#invalid";
        listV = false;
    }
    if (listV)
    {
        if (repStr.size () > 1)
        {
            repStr.back () = ']';
        }
        else
        {
            repStr.push_back (']');
        }
    }

    ActionMessage queryResp (CMD_QUERY_REPLY);
    queryResp.dest_id = command.source_id;
    queryResp.source_id = global_broker_id;
    queryResp.index = command.index;

    queryResp.payload = repStr;
    transmit (getRoute (queryResp.dest_id), queryResp);
}

int32_t CoreBroker::fillMessageRouteInformation (ActionMessage &mess)
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
            SET_ACTION_FLAG (badInit, error_flag);
            badInit.source_id = global_broker_id;
            badInit.name = command.name;
            transmit (command.source_id, badInit);  // this isn't correct
            return;
        }
        _federates.emplace_back (command.name);
        _federates.back ().route_id = getRoute (command.source_id);

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
            SET_ACTION_FLAG (badInit, error_flag);
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
            brokerReply.source_id = global_broker_id;  // source is global root
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
            if (CHECK_ACTION_FLAG (command, error_flag))
            {
                // generate an error message
                return;
            }
            global_broker_id = command.dest_id;
            higher_broker_id = command.source_id;
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
            command.source_id = global_broker_id;  // we want the intermediate broker to change the source_id
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
    case CMD_TICK:
        if (!_isRoot)
        {
            if (waitingForServerPingReply)
            {
                // try to reset the connection to the broker
                // brokerReconnect()
            }
            else
            {
                // if (allFedWaiting())
                //{
                ActionMessage png (CMD_PING);
                png.source_id = global_broker_id;
                png.dest_id = higher_broker_id;
                transmit (0, png);
                waitingForServerPingReply = true;
                //}
            }
        }
        break;
    case CMD_PING:
        if (command.dest_id == global_broker_id)
        {
            ActionMessage pngrep (CMD_PING_REPLY);
            pngrep.dest_id = command.source_id;
            pngrep.source_id = global_broker_id;
            routeMessage (pngrep);
        }
        else
        {
            routeMessage (command);
        }
        break;
    case CMD_PING_REPLY:
        if (command.dest_id == global_broker_id)
        {
            waitingForServerPingReply = false;
        }
        else
        {
            routeMessage (command);
        }
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
                checkDependencies ();
                ActionMessage m (CMD_INIT_GRANT);
                brokerState = broker_state_t::operating;
                for (auto &brk : _brokers)
                {
                    transmit (brk.route_id, m);
                }
                timeCoord->enteringExecMode (iteration_request::no_iterations);
                auto res = timeCoord->checkExecEntry ();
                if (res == iteration_state::next_step)
                {
                    enteredExecutionMode = true;
                    timeCoord->timeRequest (Time::maxVal (), iteration_request::no_iterations, Time::maxVal (),
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
            timeCoord->enteringExecMode (iteration_request::no_iterations);
            auto res = timeCoord->checkExecEntry ();
            if (res == iteration_state::next_step)
            {
                enteredExecutionMode = true;
                timeCoord->timeRequest (Time::maxVal (), iteration_request::no_iterations, Time::maxVal (),
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
        FALLTHROUGH
    case CMD_DISCONNECT:
    {
        if ((command.dest_id == 0) || (command.dest_id == global_broker_id))
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
                if (res == iteration_state::next_step)
                {
                    enteredExecutionMode = true;
                    timeCoord->timeRequest (Time::maxVal (), iteration_request::no_iterations, Time::maxVal (),
                                            Time::maxVal ());
                }
            }
        }
        else if (command.source_id == global_broker_id)
        {
            for (auto dep : timeCoord->getDependents ())
            {
                routeMessage (command, dep);
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
                routeMessage (command, dep);
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
            routeMessage (command);
        }

        break;
    case CMD_SEND_MESSAGE:
    case CMD_SEND_FOR_FILTER:
    case CMD_SEND_FOR_FILTER_OPERATION:
    case CMD_SEND_FOR_FILTER_RETURN:
        if (command.dest_id == 0)
        {
            auto route = fillMessageRouteInformation (command);
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
                routeMessage (command);
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
                routeMessage (command);
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
                routeMessage (command);
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
                routeMessage (command);
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
                routeMessage (command);
                break;
            }
        }
        addSourceFilter (command);
        break;
    case CMD_ADD_DEPENDENCY:
    case CMD_REMOVE_DEPENDENCY:
    case CMD_ADD_DEPENDENT:
    case CMD_REMOVE_DEPENDENT:
    case CMD_ADD_INTERDEPENDENCY:
    case CMD_REMOVE_INTERDEPENDENCY:
        if (command.dest_id != global_broker_id)
        {
            routeMessage (command);
        }
        else
        {
            timeCoord->processDependencyUpdateMessage (command);
        }
        break;
    default:
        if (command.dest_id != global_broker_id)
        {
            routeMessage (command);
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
    _handles.back ().processed = CHECK_ACTION_FLAG (m, processingComplete);
    if (!CHECK_ACTION_FLAG (m, processingComplete))
    {
        bool proc = FindandNotifySubscriptionPublisher (_handles.back ());
        if (!_isRoot)
        {
            if (proc)
            {
                // just let any higher level brokers know we have found the publisher and let them know
                SET_ACTION_FLAG (m, processingComplete);
            }

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

    if (!_isRoot)
    {
        SET_ACTION_FLAG (m, processingComplete);
        transmit (0, m);
    }
    else
    {
        FindandNotifyEndpointFilters (_handles.back ());
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
        if (proc)
        {
            SET_ACTION_FLAG (m, processingComplete);
        }

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
        if (proc)
        {
            SET_ACTION_FLAG (m, processingComplete);
        }
        transmit (0, m);
    }
}

CoreBroker::CoreBroker (bool setAsRootBroker) noexcept : _isRoot (setAsRootBroker) {}

CoreBroker::CoreBroker (const std::string &broker_name) : BrokerBase (broker_name) {}

void CoreBroker::initialize (const std::string &initializationString)
{
    if (brokerState == broker_state_t::created)
    {
        stringToCmdLine cmdline (initializationString);
        initializeFromArgs (cmdline.getArgCount (), cmdline.getArgV ());
    }
}

void CoreBroker::initializeFromArgs (int argc, const char *const *argv)
{
    broker_state_t exp = broker_state_t::created;
    if (brokerState.compare_exchange_strong (exp, broker_state_t::initialized))
    {
        namespace po = boost::program_options;

        po::variables_map vm;
        argumentParser (argc, argv, vm, extraArgs);
        // Initialize the brokerBase component
        initializeFromCmdArgs (argc, argv);

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

void CoreBroker::processDisconnect (bool skipUnregister)
{
    LOG_NORMAL (0, getIdentifier (), "||disconnecting");
    if (brokerState > broker_state_t::initialized)
    {
        brokerState = broker_state_t::terminating;
        brokerDisconnect ();
    }
    brokerState = broker_state_t::terminated;

    if (!skipUnregister)
    {
        unregister ();
    }
}

void CoreBroker::unregister ()
{
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

void CoreBroker::disconnect () { processDisconnect (); }

void CoreBroker::routeMessage (ActionMessage &cmd, Core::federate_id_t dest)
{
    if ((dest == 0) || (dest == higher_broker_id))
    {
        cmd.dest_id = 0;
        transmit (0, cmd);
    }
    else
    {
        auto route = getRoute (dest);
        cmd.dest_id = dest;
        transmit (route, cmd);
    }
}

void CoreBroker::routeMessage (const ActionMessage &cmd)
{
    if ((cmd.dest_id == 0) || (cmd.dest_id == higher_broker_id))
    {
        transmit (0, cmd);
    }
    else
    {
        auto route = getRoute (cmd.dest_id);
        transmit (route, cmd);
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
        if (handleInfo.flag)
        {
            SET_ACTION_FLAG (m, indicator_flag);
        }
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
                    // TODO:: send warning/error to subscriptions
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
    if (query == "federates")
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
    if (query == "brokers")
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
    if (query == "federate_map")
    {
        return generateFederateMap ();
    }
    if (query == "dependency_graph")
    {
        // TOOD:  create this information
        return "#invalid";
    }
    if (query == "dependencies")
    {
        Json_helics::Value base;
        base["name"] = getIdentifier ();
        base["id"] = static_cast<int> (global_broker_id);
        if (!isRoot ())
        {
            base["parent"] = static_cast<int> (global_broker_id);
        }
        base["dependents"] = Json_helics::arrayValue;
        int index = 0;
        for (auto &dep : timeCoord->getDependents ())
        {
            base["dependents"][index] = dep;
            ++index;
        }
        base["dependencies"] = Json_helics::arrayValue;
        index = 0;
        for (auto &dep : timeCoord->getDependencies ())
        {
            base["dependencies"][index] = dep;
            ++index;
        }
        Json_helics::StreamWriterBuilder builder;
        builder["commentStyle"] = "None";
        builder["indentation"] = "   ";  // or whatever you like
        auto writer (builder.newStreamWriter ());
        std::stringstream sstr;
        writer->write (base, &sstr);
        return sstr.str ();
    }
    return "#invalid";
}

std::string CoreBroker::generateFederateMap () const
{
    Json_helics::Value base;
    base["name"] = getIdentifier ();
    base["id"] = static_cast<int> (global_broker_id);
    if (!isRoot ())
    {
        base["parent"] = static_cast<int> (global_broker_id);
    }
    base["brokers"] = Json_helics::arrayValue;
    //  int index = 0;
    //  for (auto &dep : timeCoord->getDependents())
    // {
    // base["brokers"][index] = dep;
    //     ++index;
    //  }
    base["cores"] = Json_helics::arrayValue;
    //  index = 0;
    //  for (auto &dep : timeCoord->getDependencies())
    //  {
    //  base["cores"][index] = dep;
    //      ++index;
    //  }
    Json_helics::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = "   ";  // or whatever you like
    auto writer (builder.newStreamWriter ());
    std::stringstream sstr;
    writer->write (base, &sstr);
    return sstr.str ();
}

void CoreBroker::processLocalQuery (const ActionMessage &m)
{
    ActionMessage queryRep (CMD_QUERY_REPLY);
    queryRep.source_id = global_broker_id;
    queryRep.index = m.index;
    queryRep.payload = generateQueryAnswer (m.payload);
    routeMessage (queryRep, m.source_id);
}

void CoreBroker::processQuery (const ActionMessage &m)
{
    if ((m.info ().target == getIdentifier ()) || (m.info ().target == "broker"))
    {
        generateQueryResult (m);
    }
    else if ((isRoot ()) && ((m.info ().target == "root") || (m.info ().target == "federation")))
    {
        generateQueryResult (m);
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

void CoreBroker::checkDependencies ()
{
    if (_isRoot)
    {
        if (timeCoord->getDependents ().size () == 1)
        {  // if there is just one dependency remove it
            ActionMessage rmdep (CMD_REMOVE_INTERDEPENDENCY);
            rmdep.source_id = global_broker_id;
            auto depid = timeCoord->getDependents ()[0];
            routeMessage (rmdep, depid);
            timeCoord->removeDependency (depid);
            timeCoord->removeDependent (depid);
        }
    }
    else
    {
        // if there is more than 2 dependents(higher broker + 2 or more other objects then we need to be a
        // timeCoordinator
        if (timeCoord->getDependents ().size () > 2)
        {
            return;
        }

        auto fedid = invalid_fed_id;
        int localcnt = 0;
        for (auto &dep : timeCoord->getDependents ())
        {
            if (dep == higher_broker_id)
            {
                ++localcnt;
                fedid = dep;
            }
        }
        if (localcnt != 1)
        {
            return;
        }
        // remove the core from the time dependency chain
        timeCoord->removeDependency (higher_broker_id);
        timeCoord->removeDependency (fedid);
        timeCoord->removeDependent (higher_broker_id);
        timeCoord->removeDependent (fedid);

        ActionMessage rmdep (CMD_REMOVE_INTERDEPENDENCY);

        rmdep.source_id = global_broker_id;
        routeMessage (rmdep, higher_broker_id);
        ActionMessage adddep (CMD_ADD_INTERDEPENDENCY);
        adddep.source_id = fedid;
        routeMessage (adddep, higher_broker_id);
        adddep.source_id = higher_broker_id;
        routeMessage (adddep, fedid);
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

}  // namespace helics
