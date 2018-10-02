/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "CoreBroker.hpp"
#include "../common/stringToCmdLine.h"
#include "BrokerFactory.hpp"

#include "../common/argParser.h"
#include "../common/fmt_format.h"
#include <boost/filesystem.hpp>

#include "../common/JsonProcessingFunctions.hpp"
#include "../common/logger.h"
#include "ForwardingTimeCoordinator.hpp"
#include "loggingHelper.hpp"
#include "queryHelpers.hpp"
#include <fstream>

namespace helics
{
using namespace std::string_literals;

static const ArgDescriptors extraArgs{
  {"root"s, ArgDescriptor::arg_type_t::flag_type, "specify whether the broker is a root"s},
};

void CoreBroker::displayHelp ()
{
    std::cout << "Broker Specific options:\n";
    variable_map vm;
    const char *const argV[] = {"", "--help"};
    argumentParser (2, argV, vm, extraArgs);
    BrokerBase::displayHelp ();
}

CoreBroker::~CoreBroker ()
{
    std::lock_guard<std::mutex> lock (name_mutex_);
    // make sure everything is synchronized
}

void CoreBroker::setIdentifier (const std::string &name)
{
    if (brokerState <= broker_state_t::connecting)  // can't be changed after initialization
    {
        std::lock_guard<std::mutex> lock (name_mutex_);
        identifier = name;
    }
}
int32_t CoreBroker::getRoute (Core::federate_id_t fedid) const
{
    if ((fedid == 0) || (fedid == higher_broker_id))
    {
        return 0;
    }
    auto fnd = routing_table.find (fedid);
    return (fnd != routing_table.end ()) ? fnd->second : 0;  // zero is the default route
}

BasicBrokerInfo *CoreBroker::getBrokerById (Core::federate_id_t fedid)
{
    if (_isRoot)
    {
        auto brkNum = static_cast<int32_t> (fedid - global_broker_id_shift);
        return (isValidIndex (brkNum, _brokers) ? &_brokers[brkNum] : nullptr);
    }

    auto fnd = _brokers.find (fedid);
    return (fnd != _brokers.end ()) ? &(*fnd) : nullptr;
}

const BasicBrokerInfo *CoreBroker::getBrokerById (Core::federate_id_t fedid) const
{
    if (_isRoot)
    {
        auto brkNum = static_cast<int32_t> (fedid - global_broker_id_shift);
        return (brkNum < static_cast<int32_t> (_brokers.size ())) ? &_brokers[brkNum] : nullptr;
    }

    auto fnd = _brokers.find (fedid);
    return (fnd != _brokers.end ()) ? &(*fnd) : nullptr;
}

void CoreBroker::setLoggingCallback (
  const std::function<void(int, const std::string &, const std::string &)> &logFunction)
{
    // TODO:: this is not thread safe if done after startup
    BrokerBase::setLoggerFunction (logFunction);
}

int32_t CoreBroker::fillMessageRouteInformation (ActionMessage &mess)
{
    auto &endpointName = mess.info ().target;
    auto handle = handles.getEndpoint (endpointName);
    if (handle != nullptr)
    {
        mess.dest_id = handle->fed_id;
        mess.dest_handle = handle->handle;
        return getRoute (handle->fed_id);
    }
    auto fnd2 = knownExternalEndpoints.find (endpointName);
    if (fnd2 != knownExternalEndpoints.end ())
    {
        return fnd2->second;
    }
    return 0;
}

bool CoreBroker::isOpenToNewFederates () const { return ((brokerState != created) && (brokerState < operating)); }

void CoreBroker::processPriorityCommand (ActionMessage &&command)
{
    // deal with a few types of message immediately
    LOG_TRACE (global_broker_id, getIdentifier (),
               fmt::format ("|| priority_cmd:{} from {}", prettyPrintString (command), command.source_id));
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
            setActionFlag (badInit, error_flag);
            badInit.source_id = global_broker_id;
            badInit.index = 5;
            badInit.name = command.name;
            transmit (getRoute (command.source_id), badInit);
            return;
        }
        if (_federates.find (command.name) != _federates.end ())
        {
            ActionMessage badName (CMD_FED_ACK);
            setActionFlag (badName, error_flag);
            badName.source_id = global_broker_id;
            badName.index = 6;
            badName.name = command.name;
            transmit (getRoute (command.source_id), badName);
            return;
        }
        _federates.insert (command.name, static_cast<Core::federate_id_t> (_federates.size ()), command.name);
        _federates.back ().route_id = getRoute (command.source_id);
        if (!_isRoot)
        {
            if (global_broker_id != 0)
            {
                command.source_id = global_broker_id;
                transmit (0, command);
            }
            else
            {
                // delay the response if we are not fully registered yet
                delayTransmitQueue.push (command);
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
            LOG_CONNECTIONS (global_broker_id, getIdentifier (),
                             fmt::format ("registering federate {}({}) on route {}", command.name,global_id,route_id));
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
            setActionFlag (badInit, error_flag);
            badInit.source_id = global_broker_id;
            badInit.name = command.name;
            transmit (command.source_id, badInit);
            return;
        }
        _brokers.insert (command.name, static_cast<Core::federate_id_t> (_brokers.size ()), command.name);
        if (command.source_id == 0)
        {
            _brokers.back ().route_id = static_cast<decltype (_brokers.back ().route_id)> (_brokers.size ());
            addRoute (_brokers.back ().route_id, command.info ().target);
        }
        else
        {
            _brokers.back ().route_id = getRoute (command.source_id);
            _brokers.back ()._nonLocal = true;
        }
        _brokers.back ()._core = checkActionFlag (command, core_flag);
        if (!_isRoot)
        {
            if (global_broker_id != 0)
            {
                command.source_id = global_broker_id;
                transmit (0, command);
            }
            else
            {
                // delay the response if we are not fully registered yet
                delayTransmitQueue.push (command);
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
            LOG_CONNECTIONS (global_broker_id, getIdentifier (),
                             fmt::format ("registering broker {}({}) on route {}", command.name, global_id,
                                          route_id));
        }
    }
    break;
    case CMD_FED_ACK:
    {  // we can't be root if we got one of these
        auto fed = _federates.find (command.name);
        if (fed != _federates.end ())
        {
            fed->global_id = command.dest_id;
            auto route = fed->route_id;
            _federates.addSearchTerm (command.dest_id, fed->name);
            transmit (route, command);
            routing_table.emplace (fed->global_id, route);
        }
        else
        {
            // this means we haven't seen this federate before for some reason
            _federates.insert (command.name, command.dest_id, command.name);
            _federates.back ().route_id = getRoute (command.source_id);
            _federates.back ().global_id = command.dest_id;
            routing_table.emplace (fed->global_id, _federates.back ().route_id);
            // it also means we don't forward it
        }
    }
    break;
    case CMD_BROKER_ACK:
    {  // we can't be root if we got one of these
        if (command.name == identifier)
        {
            if (checkActionFlag (command, error_flag))
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
        auto broker = _brokers.find (command.name);
        if (broker != _brokers.end ())
        {
            broker->global_id = command.dest_id;
            auto route = broker->route_id;
            _brokers.addSearchTerm (command.dest_id, broker->name);
            routing_table.emplace (broker->global_id, route);
            command.source_id = global_broker_id;  // we want the intermediate broker to change the source_id
            transmit (route, command);
        }
        else
        {
            _brokers.insert (command.name, command.dest_id, command.name);
            _brokers.back ().route_id = getRoute (command.source_id);
            _brokers.back ().global_id = command.dest_id;
            routing_table.emplace (broker->global_id, _brokers.back ().route_id);
        }
    }
    break;
    case CMD_PRIORITY_DISCONNECT:
    {
        auto brk = getBrokerById (command.source_id);
        if (brk != nullptr)
        {
            brk->_disconnected = true;
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
    case CMD_BROKER_QUERY:
        if (command.dest_id == global_broker_id)
        {
            processLocalQuery (command);
        }
        else if ((isRoot ()) && (command.dest_id == 0))
        {
            processLocalQuery (command);
        }
        else
        {
            routeMessage (command);
        }
        break;
    case CMD_QUERY:
        processQuery (command);
        break;
    case CMD_QUERY_REPLY:
        if (command.dest_id == global_broker_id)
        {
            processQueryResponse (command);
        }
        else
        {
            transmit (getRoute (command.dest_id), command);
        }
        break;
    default:
        // must not have been a priority command
        break;
    }
}

std::string CoreBroker::generateFederationSummary () const 
{
    int pubs = 0;
    int epts = 0;
    int subs = 0;
    int filt = 0;
	for (auto &hand : handles)
	{
		switch (hand.handle_type)
		{
        case handle_type_t::publication:
            ++pubs;
            break;
        case handle_type_t::subscription:
            ++subs;
            break;
        case handle_type_t::endpoint:
            ++epts;
            break;
        default:
            ++filt;
            break;
		}
	}
    std::string output = fmt::format ("Federation Summary> \n\t{} federates\n\t {}brokers/cores\n\t{} "
                                      "publications\n\t{} inputs\n\t{} endpoints\n\t{} filters\n<<<<<<<<<",
                                      _federates.size (), _brokers.size (), pubs, subs, epts, filt);
    return output;
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

void CoreBroker::sendDisconnect ()
{
    ActionMessage bye (CMD_DISCONNECT);
    bye.source_id = global_broker_id;
    for (auto &brk : _brokers)
    {
        if (!brk._nonLocal)
        {
            routeMessage (bye, brk.global_id);
            brk._disconnected = true;
        }
        if (hasTimeDependency)
        {
            timeCoord->removeDependency (brk.global_id);
            timeCoord->removeDependent (brk.global_id);
        }
    }
    if (hasTimeDependency)
    {
        timeCoord->disconnect ();
    }
}

void CoreBroker::sendErrorToImmediateBrokers (int error_code)
{
    ActionMessage errorCom (CMD_ERROR);
    errorCom.counter = static_cast<int16_t> (error_code);
    for (auto &brk : _brokers)
    {
        if (!brk._nonLocal)
        {
            routeMessage (errorCom, brk.global_id);
        }
    }
}

void CoreBroker::processCommand (ActionMessage &&command)
{
    LOG_TRACE (global_broker_id, getIdentifier (),
               fmt::format ("|| cmd:{} from {} to ", prettyPrintString (command), command.source_id,
                            command.dest_id));
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
                LOG_ERROR (global_broker_id, getIdentifier (), "lost connection with server");
                sendErrorToImmediateBrokers (-5);
                disconnect ();
                brokerState = broker_state_t::errored;
                addActionMessage (CMD_STOP);
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
        auto brk = getBrokerById (command.source_id);
        if (brk != nullptr)
        {
            brk->_initRequested = true;
        }
        if (allInitReady ())
        {
            if (_isRoot)
            {
                LOG_TIMING (global_broker_id, "root", "entering initialization mode");
                LOG_SUMMARY (global_broker_id, "root", generateFederationSummary ());
                executeInitializationOperations ();
            }
            else
            {
                LOG_TIMING (global_broker_id, getIdentifier(), "entering initialization mode");
                checkDependencies ();
                command.source_id = global_broker_id;
                transmit (0, command);
            }
        }
    }
    break;
    case CMD_INIT_NOT_READY:
    {
        if (allInitReady ())
        {
            transmit (0, command);
        }
        auto brk = getBrokerById (command.source_id);
        if (brk != nullptr)
        {
            brk->_initRequested = false;
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
            timeCoord->enteringExecMode ();
            auto res = timeCoord->checkExecEntry ();
            if (res == message_processing_result::next_step)
            {
                enteredExecutionMode = true;
            }
        }
        break;
    case CMD_SEARCH_DEPENDENCY:
    {
        auto fed = _federates.find (command.name);
        if (fed != _federates.end ())
        {
            if (fed->global_id != invalid_fed_id)
            {
                ActionMessage dep (CMD_ADD_DEPENDENCY, fed->global_id, command.source_id);
                routeMessage (dep);
                dep = ActionMessage (CMD_ADD_DEPENDENT, command.source_id, fed->global_id);
                routeMessage (dep);
                break;
            }
        }
        if (isRoot ())
        {
            delayedDependencies.emplace_back (command.name, command.source_id);
        }
        else
        {
            routeMessage (command);
        }
        break;
    }
    case CMD_DISCONNECT_NAME:
        if (command.dest_id == 0)
        {
            auto brk = _brokers.find (command.payload);
            if (brk != _brokers.end ())
            {
                command.source_id = brk->global_id;
            }
        }
        FALLTHROUGH
        /* FALLTHROUGH */
    case CMD_DISCONNECT:
    {
        if ((command.dest_id == 0) || (command.dest_id == global_broker_id))
        {
            if (!isRoot ())
            {
                if (command.source_id == higher_broker_id)
                {
                    sendDisconnect ();
                    addActionMessage (CMD_STOP);
                    return;
                }
            }

            auto brk = getBrokerById (command.source_id);
            if (brk != nullptr)
            {
                brk->_disconnected = true;
            }
            if (hasTimeDependency)
            {
                if (!enteredExecutionMode)
                {
                    timeCoord->processTimeMessage (command);
                    auto res = timeCoord->checkExecEntry ();
                    if (res == message_processing_result::next_step)
                    {
                        enteredExecutionMode = true;
                    }
                }
                else
                {
                    if (timeCoord->processTimeMessage (command))
                    {
                        timeCoord->updateTimeFactors ();
                    }
                }
            }

            if (allDisconnected ())
            {
                timeCoord->disconnect ();
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
    case CMD_USER_DISCONNECT:
        sendDisconnect ();
        addActionMessage (CMD_STOP);
        break;
    case CMD_STOP:
        if (!allDisconnected ())
        {  // only send a disconnect message if we haven't done so already
            timeCoord->disconnect ();
            if (!_isRoot)
            {
                ActionMessage m (CMD_DISCONNECT);
                m.source_id = global_broker_id;
                transmit (0, m);
            }
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
                if (res == message_processing_result::next_step)
                {
                    enteredExecutionMode = true;
                    LOG_TIMING (global_broker_id, getIdentifier (), "entering Exec Mode");
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
            LOG_TIMING(global_broker_id, getIdentifier (),
                       fmt::format ("time request update {}", prettyPrintString (command)));
            for (auto dep : timeCoord->getDependents ())
            {
                routeMessage (command, dep);
            }
        }
        else if (command.dest_id == global_broker_id)
        {
            if (timeCoord->processTimeMessage (command))
            {
                timeCoord->updateTimeFactors ();
            }
        }
        else
        {
            routeMessage (command);
        }

        break;
    case CMD_SEND_MESSAGE:
    case CMD_SEND_FOR_FILTER:
    case CMD_SEND_FOR_FILTER_AND_RETURN:
    case CMD_FILTER_RESULT:
    case CMD_NULL_MESSAGE:
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
        if ((!_isRoot) && (command.dest_id != 0))
        {
            routeMessage (command);
            break;
        }
        addPublication (command);
        break;
    case CMD_REG_SUB:
        if ((!_isRoot) && (command.dest_id != 0))
        {
            routeMessage (command);
            break;
        }
        addSubscription (command);
        break;
    case CMD_REG_END:
        if ((!_isRoot) && (command.dest_id != 0))
        {
            routeMessage (command);
            break;
        }
        addEndpoint (command);
        break;
    case CMD_REG_DST_FILTER:
        if ((!_isRoot) && (command.dest_id != 0))
        {
            routeMessage (command);
            break;
        }
        addDestFilter (command);
        break;
    case CMD_REG_SRC_FILTER:
        if ((!_isRoot) && (command.dest_id != 0))
        {
            routeMessage (command);
            break;
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
            if (!hasTimeDependency)
            {
                hasTimeDependency = true;
            }
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
    handleInfo.flags = m.flags;
}

void CoreBroker::addPublication (ActionMessage &m)
{
    // detect duplicate publications
    if (handles.getPublication (m.name) != nullptr)
    {
        ActionMessage eret (CMD_ERROR, global_broker_id, m.source_id);
        eret.dest_handle = m.source_handle;
        eret.counter = ERROR_CODE_REGISTRATION_FAILURE;
        eret.payload = "Duplicate publication names (" + m.name + ")";
        routeMessage (eret);
        return;
    }
    auto &pub = handles.addHandle (m.source_id, m.source_handle, handle_type_t::publication, m.name,
                                   m.info ().type, m.info ().units);

    addLocalInfo (pub, m);
    if (!_isRoot)
    {
        transmit (0, m);
    }
    else
    {
        FindandNotifyPublicationSubscribers (pub);
    }
}
void CoreBroker::addSubscription (ActionMessage &m)
{
    auto &sub = handles.addHandle (m.source_id, m.source_handle, handle_type_t::subscription, m.name,
                                   m.info ().type, m.info ().units);

    addLocalInfo (sub, m);
    if (!checkActionFlag (m, processing_complete_flag))
    {
        bool proc = FindandNotifySubscriptionPublisher (sub);
        if (!_isRoot)
        {
            if (proc)
            {
                // just let any higher level brokers know we have found the publisher and let them know
                setActionFlag (m, processing_complete_flag);
            }

            transmit (0, m);
        }
    }
}

void CoreBroker::addEndpoint (ActionMessage &m)
{
    // detect duplicate endpoints
    if (handles.getEndpoint (m.name) != nullptr)
    {
        ActionMessage eret (CMD_ERROR, global_broker_id, m.source_id);
        eret.dest_handle = m.source_handle;
        eret.counter = ERROR_CODE_REGISTRATION_FAILURE;
        eret.payload = "Duplicate endpoint names (" + m.name + ")";
        routeMessage (eret);
        return;
    }
    auto &ept = handles.addHandle (m.source_id, m.source_handle, handle_type_t::endpoint, m.name, m.info ().type,
                                   m.info ().units);

    addLocalInfo (ept, m);

    if (!_isRoot)
    {
        setActionFlag (m, processing_complete_flag);
        transmit (0, m);
        if (!hasTimeDependency)
        {
            if (timeCoord->addDependency (higher_broker_id))
            {
                hasTimeDependency = true;
                ActionMessage add (CMD_ADD_INTERDEPENDENCY, global_broker_id, higher_broker_id);
                transmit (getRoute (higher_broker_id), add);

                timeCoord->addDependent (higher_broker_id);
            }
        }
    }
    else
    {
        FindandNotifyEndpointFilters (ept);
    }
}
void CoreBroker::addSourceFilter (ActionMessage &m)
{
    auto &filt = handles.addHandle (m.source_id, m.source_handle, handle_type_t::source_filter, m.name,
                                    m.info ().target, m.info ().type, m.info ().type_out);
    addLocalInfo (filt, m);

    bool proc = FindandNotifyFilterEndpoint (filt);
    if (!_isRoot)
    {
        if (proc)
        {
            setActionFlag (m, processing_complete_flag);
        }

        transmit (0, m);
        if (!hasFilters)
        {
            hasFilters = true;
            if (timeCoord->addDependent (higher_broker_id))
            {
                ActionMessage add (CMD_ADD_DEPENDENCY, global_broker_id, higher_broker_id);
                transmit (getRoute (higher_broker_id), add);
            }
        }
    }
}

/*
bool CoreBroker::updateSourceFilterOperator (ActionMessage &m)
{
    auto filter = handles.findHandle (m.source_id, m.source_handle);
    if (filter != nullptr)
    {
        filter->flag = true;

        auto endHandle = handles.getEndpoint (filter->target);
        if (endHandle != nullptr)
        {
            m.dest_id = endHandle->fed_id;
            m.dest_handle = endHandle->handle;

            transmit (getRoute (m.dest_id), m);
            return true;
        }
    }
    return false;
}
*/
void CoreBroker::addDestFilter (ActionMessage &m)
{
    auto &filt = handles.addHandle (m.source_id, m.source_handle, handle_type_t::destination_filter, m.name,
                                    m.info ().target, m.info ().type, m.info ().type_out);
    addLocalInfo (filt, m);

    bool proc = FindandNotifyFilterEndpoint (filt);
    if (!_isRoot)
    {
        if (proc)
        {
            setActionFlag (m, processing_complete_flag);
        }
        transmit (0, m);
        if (!hasFilters)
        {
            hasFilters = true;
            if (timeCoord->addDependent (higher_broker_id))
            {
                hasTimeDependency = true;
                ActionMessage add (CMD_ADD_DEPENDENCY, global_broker_id, higher_broker_id);
                transmit (getRoute (higher_broker_id), add);
            }
        }
    }
}

CoreBroker::CoreBroker (bool setAsRootBroker) noexcept : _isRoot (setAsRootBroker) {}

CoreBroker::CoreBroker (const std::string &broker_name) : BrokerBase (broker_name) {}

void CoreBroker::initialize (const std::string &initializationString)
{
    if (brokerState == broker_state_t::created)
    {
        StringToCmdLine cmdline (initializationString);
        initializeFromArgs (cmdline.getArgCount (), cmdline.getArgV ());
    }
}

void CoreBroker::initializeFromArgs (int argc, const char *const *argv)
{
    broker_state_t exp = broker_state_t::created;
    if (brokerState.compare_exchange_strong (exp, broker_state_t::initialized))
    {
        variable_map vm;
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
            LOG_CONNECTIONS (0, getIdentifier (), "connecting");
            auto res = brokerConnect ();
            if (res)
            {
                LOG_CONNECTIONS (0, getIdentifier (), fmt::format ("||connected on {}", getAddress ()));
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
                disconnection.activate ();
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

void CoreBroker::waitForDisconnect (int msToWait) const
{
    if (msToWait <= 0)
    {
        disconnection.wait ();
    }
    else
    {
        disconnection.wait_for (std::chrono::milliseconds (msToWait));
    }
}

void CoreBroker::processDisconnect (bool skipUnregister)
{
    LOG_CONNECTIONS (0, getIdentifier (), "||disconnecting");
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
    disconnection.trigger ();
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

void CoreBroker::disconnect ()
{
    ActionMessage udisconnect (CMD_USER_DISCONNECT);
    addActionMessage (udisconnect);
    waitForDisconnect ();
}

void CoreBroker::routeMessage (ActionMessage &cmd, Core::federate_id_t dest)
{
    cmd.dest_id = dest;
    if ((dest == 0) || (dest == higher_broker_id))
    {
        transmit (0, cmd);
    }
    else
    {
        auto route = getRoute (dest);
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

void CoreBroker::executeInitializationOperations ()
{
    checkSubscriptions ();
    checkEndpoints ();
    checkFilters ();
    checkDependencies ();
    ActionMessage m (CMD_INIT_GRANT);
    brokerState = broker_state_t::operating;
    for (auto &broker : _brokers)
    {
        if (!broker._nonLocal)
        {
            transmit (broker.route_id, m);
        }
    }
    timeCoord->enteringExecMode ();
    auto res = timeCoord->checkExecEntry ();
    if (res == message_processing_result::next_step)
    {
        enteredExecutionMode = true;
    }
}

bool CoreBroker::FindandNotifySubscriptionPublisher (BasicHandleInfo &handleInfo)
{
    if (!checkActionFlag (handleInfo, processing_complete_flag))
    {
        auto pubHandle = handles.getPublication (handleInfo.key);
        if (pubHandle != nullptr)
        {
            if (!matchingTypes (pubHandle->type, handleInfo.type))
            {
                // LOG(WARN) << "sub " << hndl->key << " does not match types" << hndl->type << " " <<
                // pubInfo->type << ENDL;
            }
            // notify the subscription about its publisher
            ActionMessage m (CMD_NOTIFY_PUB);
            m.source_id = pubHandle->fed_id;
            m.source_handle = pubHandle->handle;
            m.dest_id = handleInfo.fed_id;
            m.dest_handle = handleInfo.handle;
            m.payload = pubHandle->type;
            transmit (getRoute (m.dest_id), m);

            // notify the publisher about its subscription
            m.setAction (CMD_NOTIFY_SUB);
            m.source_id = handleInfo.fed_id;
            m.source_handle = handleInfo.handle;
            m.dest_id = pubHandle->fed_id;
            m.dest_handle = pubHandle->handle;

            transmit (getRoute (m.dest_id), m);
            setActionFlag (handleInfo, processing_complete_flag);
        }
    }
    return checkActionFlag (handleInfo, processing_complete_flag);
}

void CoreBroker::FindandNotifyPublicationSubscribers (BasicHandleInfo &handleInfo)
{
    auto subHandles = handles.getSubscribers (handleInfo.key);
    for (auto sub = subHandles.first; sub != subHandles.second; ++sub)
    {
        auto &subInfo = handles[sub->second];
        if (checkActionFlag (subInfo, processing_complete_flag))
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
        m.source_handle = subInfo.handle;
        m.dest_id = handleInfo.fed_id;
        m.dest_handle = handleInfo.handle;

        transmit (getRoute (m.dest_id), m);

        // notify the subscriber about its publisher
        m.setAction (CMD_NOTIFY_PUB);
        m.source_id = handleInfo.fed_id;
        m.source_handle = handleInfo.handle;
        m.dest_id = subInfo.fed_id;
        m.dest_handle = subInfo.handle;
        m.payload = handleInfo.type;
        transmit (getRoute (m.dest_id), m);
        setActionFlag (subInfo, processing_complete_flag);
    }
}

bool CoreBroker::FindandNotifyFilterEndpoint (BasicHandleInfo &handleInfo)
{
    if (!checkActionFlag (handleInfo, processing_complete_flag))
    {
        auto endHandle = handles.getEndpoint (handleInfo.target);
        if (endHandle != nullptr)
        {
            if (!matchingTypes (endHandle->type, handleInfo.type))
            {
                // LOG(WARN) << "sub " << hndl->key << " does not match types" << hndl->type << " " <<
                // pubInfo->type << ENDL;
            }
            // notify the filter about its endpoint
            ActionMessage m (CMD_NOTIFY_END);
            m.source_id = endHandle->fed_id;
            m.source_handle = endHandle->handle;
            m.dest_id = handleInfo.fed_id;
            m.dest_handle = handleInfo.handle;

            transmit (getRoute (m.dest_id), m);

            // notify the endpoint about its filter
            m.setAction ((handleInfo.handle_type == handle_type_t::source_filter) ? CMD_NOTIFY_SRC_FILTER :
                                                                                    CMD_NOTIFY_DST_FILTER);
            m.source_id = handleInfo.fed_id;
            m.source_handle = handleInfo.handle;
            m.flags = handleInfo.flags;
            m.dest_id = endHandle->fed_id;
            m.dest_handle = endHandle->handle;
            transmit (getRoute (m.dest_id), m);
            setActionFlag (handleInfo, processing_complete_flag);
        }
    }
    return checkActionFlag (handleInfo, processing_complete_flag);
}

void CoreBroker::FindandNotifyEndpointFilters (BasicHandleInfo &handleInfo)
{
    auto filtHandles = handles.getFilters (handleInfo.target);
    for (auto filt = filtHandles.first; filt != filtHandles.second; ++filt)
    {
        auto &filtInfo = handles[filt->second];
        if (checkActionFlag (filtInfo, processing_complete_flag))
        {
            continue;
        }
        if (!matchingTypes (filtInfo.type, handleInfo.type))
        {
            ActionMessage mismatch (CMD_WARNING, global_broker_id, filtInfo.fed_id);
            mismatch.dest_handle = filtInfo.handle;
            mismatch.payload = fmt::format ("filter type mismatch for {}: {} does not match {} for endpoint {}",
                                            filtInfo.key, filtInfo.type, handleInfo.type, handleInfo.key);
            transmit (getRoute (filtInfo.fed_id), mismatch);
        }
        // notify the endpoint about a filter
        ActionMessage m ((handleInfo.handle_type == handle_type_t::source_filter) ? CMD_NOTIFY_SRC_FILTER :
                                                                                    CMD_NOTIFY_DST_FILTER);
        m.source_id = filtInfo.fed_id;
        m.source_handle = filtInfo.handle;
        m.dest_id = handleInfo.fed_id;
        m.dest_handle = handleInfo.handle;
        m.flags = handleInfo.flags;
        transmit (getRoute (m.dest_id), m);

        // notify the publisher about its subscription
        m.setAction (CMD_NOTIFY_END);
        m.source_id = handleInfo.fed_id;
        m.source_handle = handleInfo.handle;
        m.dest_id = filtInfo.fed_id;
        m.dest_handle = filtInfo.handle;

        transmit (getRoute (m.dest_id), m);
        setActionFlag (filtInfo, processing_complete_flag);
    }
}

void CoreBroker::checkSubscriptions ()
{
    // pub/sub checks
    // LOG(INFO) << "performing pub/sub check" << ENDL;
    for (auto &hndl : handles)
    {
        if (hndl.handle_type == handle_type_t::subscription)
        {
            if (!checkActionFlag (hndl, processing_complete_flag))
            {
                auto fnd = FindandNotifySubscriptionPublisher (hndl);
                if ((!fnd) && (checkActionFlag (hndl, required_flag)))
                {
                    auto str = fmt::format ("subscription {} has no corresponding publication", hndl.key);
                    LOG_WARNING (global_broker_id, getIdentifier (), str);
                    ActionMessage missing (CMD_ERROR, global_broker_id, hndl.fed_id);
                    missing.counter = ERROR_CODE_REGISTRATION_FAILURE;
                    missing.dest_handle = hndl.handle;
                    missing.payload = std::move (str);
                    transmit (getRoute (hndl.fed_id), missing);
                }
            }
        }
    }
}
// public query function
std::string CoreBroker::query (const std::string &target, const std::string &queryStr)
{
    if ((target == "broker") || (target == getIdentifier ()))
    {
        ActionMessage querycmd (CMD_BROKER_QUERY);
        querycmd.source_id = global_broker_id;
        querycmd.dest_id = global_broker_id;
        auto index = ++queryCounter;
        querycmd.index = index;
        querycmd.payload = queryStr;
        auto fut = ActiveQueries.getFuture (index);
        addActionMessage (std::move (querycmd));
        auto ret = fut.get ();
        ActiveQueries.finishedWithValue (index);
        return ret;
    }
    else if (target == "parent")
    {
        if (isRoot ())
        {
            return "#invalid";
        }
        ActionMessage querycmd (CMD_BROKER_QUERY);
        querycmd.source_id = global_broker_id;
        querycmd.dest_id = higher_broker_id;
        querycmd.index = ++queryCounter;
        querycmd.payload = queryStr;
        auto fut = ActiveQueries.getFuture (querycmd.index);
        addActionMessage (querycmd);
        auto ret = fut.get ();
        ActiveQueries.finishedWithValue (querycmd.index);
        return ret;
    }
    else if ((target == "root") || (target == "rootbroker"))
    {
        ActionMessage querycmd (CMD_BROKER_QUERY);
        querycmd.source_id = global_broker_id;
        querycmd.dest_id = 0;
        auto index = ++queryCounter;
        querycmd.index = index;
        querycmd.payload = queryStr;
        auto fut = ActiveQueries.getFuture (querycmd.index);
        if (global_broker_id == invalid_fed_id)
        {
            delayTransmitQueue.push (std::move (querycmd));
        }
        else
        {
            transmit (0, querycmd);
        }
        auto ret = fut.get ();
        ActiveQueries.finishedWithValue (index);
        return ret;
    }
    else
    {
        ActionMessage querycmd (CMD_QUERY);
        querycmd.source_id = global_broker_id;
        auto index = ++queryCounter;
        querycmd.index = index;
        querycmd.payload = queryStr;
        querycmd.info ().target = target;
        auto fut = ActiveQueries.getFuture (querycmd.index);
        if (global_broker_id == invalid_fed_id)
        {
            delayTransmitQueue.push (std::move (querycmd));
        }
        else
        {
            transmit (0, querycmd);
        }

        auto ret = fut.get ();
        ActiveQueries.finishedWithValue (index);
        return ret;
    }
    //  return "#invalid";
}

std::string CoreBroker::generateQueryAnswer (const std::string &request)
{
    if (request == "isinit")
    {
        return (brokerState >= broker_state_t::operating) ? std::string ("true") : std::string ("false");
    }
    if (request == "isconnected")
    {
        return (isConnected ()) ? std::string ("true") : std::string ("false");
    }
    if (request == "name")
    {
        return getIdentifier ();
    }
    if (request == "address")
    {
        return getAddress ();
    }
    if (request == "counts")
    {
        std::string cnts = "{\"brokers\":";
        cnts += std::to_string (_brokers.size ());
        cnts += ",\n";
        cnts += "\"federates\":";
        cnts += std::to_string (_federates.size ());
        cnts += ",\n";
        cnts += "\"handles\":";
        cnts += std::to_string (handles.size ());
        cnts += '}';
        return cnts;
    }
	if (request == "summary")
	{
        return generateFederationSummary ();
	}
    if (request == "federates")
    {
        return generateStringVector (_federates, [](auto &fed) { return fed.name; });
    }
    if (request == "brokers")
    {
        return generateStringVector (_brokers, [](auto &brk) { return brk.name; });
    }
    if (request == "publications")
    {
        return generateStringVector_if (handles, [](auto &handle) { return handle.key; },
                                        [](auto &handle) {
                                            return (handle.handle_type == handle_type_t::publication);
                                        });
    }
    if (request == "endpoints")
    {
        return generateStringVector_if (handles, [](auto &handle) { return handle.key; },
                                        [](auto &handle) {
                                            return (handle.handle_type == handle_type_t::endpoint);
                                        });
    }
    if (request == "federate_map")
    {
        if (fedMap.isCompleted ())
        {
            return fedMap.generate ();
        }
        if (fedMap.isActive ())
        {
            return "#wait";
        }
        initializeFederateMap ();
        if (fedMap.isCompleted ())
        {
            return fedMap.generate ();
        }
        return "#wait";
    }
    if (request == "dependency_graph")
    {
        if (depMap.isCompleted ())
        {
            return depMap.generate ();
        }
        if (depMap.isActive ())
        {
            return "#wait";
        }
        initializeDependencyGraph ();
        if (depMap.isCompleted ())
        {
            return depMap.generate ();
        }
        return "#wait";
    }
    if (request == "dependson")
    {
        return generateStringVector (timeCoord->getDependencies (),
                                     [](const auto &dep) { return std::to_string (dep); });
    }
    if (request == "dependents")
    {
        return generateStringVector (timeCoord->getDependents (),
                                     [](const auto &dep) { return std::to_string (dep); });
    }
    if (request == "dependencies")
    {
        Json_helics::Value base;
        base["name"] = getIdentifier ();
        base["id"] = static_cast<int> (global_broker_id);
        if (!isRoot ())
        {
            base["parent"] = static_cast<int> (higher_broker_id);
        }
        base["dependents"] = Json_helics::arrayValue;
        for (auto &dep : timeCoord->getDependents ())
        {
            base["dependents"].append (dep);
        }
        base["dependencies"] = Json_helics::arrayValue;
        for (auto &dep : timeCoord->getDependencies ())
        {
            base["dependencies"].append (dep);
        }
        return generateJsonString (base);
    }
    return "#invalid";
}

void CoreBroker::initializeFederateMap ()
{
    Json_helics::Value &base = fedMap.getJValue ();
    base["name"] = getIdentifier ();
    base["id"] = static_cast<int> (global_broker_id);
    if (!isRoot ())
    {
        base["parent"] = static_cast<int> (higher_broker_id);
    }
    base["brokers"] = Json_helics::arrayValue;
    ActionMessage queryReq (CMD_BROKER_QUERY);
    queryReq.payload = "federate_map";
    queryReq.source_id = global_broker_id;
    queryReq.counter = 2;  // indicating which processing to use
    bool hasCores = false;
    for (auto &broker : _brokers)
    {
        if (!broker._nonLocal)
        {
            int index;
            if (broker._core)
            {
                if (!hasCores)
                {
                    hasCores = true;
                    base["cores"] = Json_helics::arrayValue;
                }
                index = fedMap.generatePlaceHolder ("cores");
            }
            else
            {
                index = fedMap.generatePlaceHolder ("brokers");
            }
            queryReq.index = index;
            queryReq.dest_id = broker.global_id;
            transmit (broker.route_id, queryReq);
        }
    }
}

void CoreBroker::initializeDependencyGraph ()
{
    Json_helics::Value &base = depMap.getJValue ();
    base["name"] = getIdentifier ();
    base["id"] = static_cast<int> (global_broker_id);
    if (!isRoot ())
    {
        base["parent"] = static_cast<int> (higher_broker_id);
    }
    base["brokers"] = Json_helics::arrayValue;
    ActionMessage queryReq (CMD_BROKER_QUERY);
    queryReq.payload = "dependency_graph";
    queryReq.source_id = global_broker_id;
    queryReq.counter = 4;  // indicating which processing to use
    bool hasCores = false;
    for (auto &broker : _brokers)
    {
        int index;
        if (broker._core)
        {
            if (!hasCores)
            {
                hasCores = true;
                base["cores"] = Json_helics::arrayValue;
            }
            index = depMap.generatePlaceHolder ("cores");
        }
        else
        {
            index = depMap.generatePlaceHolder ("brokers");
        }
        queryReq.index = index;
        queryReq.dest_id = broker.global_id;
        transmit (broker.route_id, queryReq);
    }

    base["dependents"] = Json_helics::arrayValue;
    for (auto &dep : timeCoord->getDependents ())
    {
        base["dependents"].append (dep);
    }
    base["dependencies"] = Json_helics::arrayValue;
    for (auto &dep : timeCoord->getDependencies ())
    {
        base["dependencies"].append (dep);
    }
}

void CoreBroker::processLocalQuery (const ActionMessage &m)
{
    ActionMessage queryRep (CMD_QUERY_REPLY);
    queryRep.source_id = global_broker_id;
    queryRep.dest_id = m.source_id;
    queryRep.index = m.index;
    queryRep.payload = generateQueryAnswer (m.payload);
    queryRep.counter = m.counter;
    if (queryRep.payload == "#wait")
    {
        if (m.payload == "dependency_graph")
        {
            depMapRequestors.push_back (queryRep);
        }
        else if (m.payload == "federate_map")
        {
            fedMapRequestors.push_back (queryRep);
        }
    }
    else if (queryRep.dest_id == global_broker_id)
    {
        ActiveQueries.setDelayedValue (m.index, queryRep.payload);
    }
    else
    {
        routeMessage (queryRep, m.source_id);
    }
}

void CoreBroker::processQuery (const ActionMessage &m)
{
    if ((m.info ().target == getIdentifier ()) || (m.info ().target == "broker"))
    {
        processLocalQuery (m);
    }
    else if ((isRoot ()) && ((m.info ().target == "root") || (m.info ().target == "federation")))
    {
        processLocalQuery (m);
    }
    else
    {
        int32_t route = 0;
        auto fed = _federates.find (m.info ().target);
        if (fed != _federates.end ())
        {
            route = fed->route_id;
        }
        else
        {
            auto broker = _brokers.find (m.info ().target);
            if (broker != _brokers.end ())
            {
                route = broker->route_id;
            }
        }
        if ((route == 0) && (isRoot ()))
        {
            ActionMessage queryResp (CMD_QUERY_REPLY);
            queryResp.dest_id = m.source_id;
            queryResp.source_id = global_broker_id;
            queryResp.index = m.index;

            queryResp.payload = "#invalid";
            transmit (getRoute (queryResp.dest_id), queryResp);
        }
        else
        {
            transmit (route, m);
        }
    }
}

void CoreBroker::processQueryResponse (const ActionMessage &m)
{
    switch (m.counter)
    {
    case 0:
    default:
        ActiveQueries.setDelayedValue (m.index, m.payload);
        break;
    case 2:
        if (fedMap.addComponent (m.payload, m.index))
        {
            if (fedMapRequestors.size () == 1)
            {
                if (fedMapRequestors.front ().dest_id == global_broker_id)
                {
                    ActiveQueries.setDelayedValue (fedMapRequestors.front ().index, fedMap.generate ());
                }
                else
                {
                    fedMapRequestors.front ().payload = fedMap.generate ();
                    routeMessage (fedMapRequestors.front ());
                }
            }
            else
            {
                auto str = fedMap.generate ();
                for (auto &resp : fedMapRequestors)
                {
                    if (resp.dest_id == global_broker_id)
                    {
                        ActiveQueries.setDelayedValue (resp.index, str);
                    }
                    else
                    {
                        resp.payload = str;
                        routeMessage (resp);
                    }
                }
            }
            fedMapRequestors.clear ();
        }
        break;
    case 4:
        if (depMap.addComponent (m.payload, m.index))
        {
            if (depMapRequestors.size () == 1)
            {
                if (depMapRequestors.front ().dest_id == global_broker_id)
                {
                    ActiveQueries.setDelayedValue (depMapRequestors.front ().index, depMap.generate ());
                }
                else
                {
                    depMapRequestors.front ().payload = depMap.generate ();
                    routeMessage (depMapRequestors.front ());
                }
            }
            else
            {
                auto str = depMap.generate ();
                for (auto &resp : depMapRequestors)
                {
                    if (resp.dest_id == global_broker_id)
                    {
                        ActiveQueries.setDelayedValue (resp.index, str);
                    }
                    else
                    {
                        resp.payload = str;
                        routeMessage (resp);
                    }
                }
            }
            depMapRequestors.clear ();
        }
        break;
    }
}
void CoreBroker::checkEndpoints () {}

void CoreBroker::checkFilters ()
{
    // LOG(INFO) << "performing filter check" << ENDL;
    for (auto &hndl : handles)
    {
        if ((hndl.handle_type == handle_type_t::destination_filter) ||
            (hndl.handle_type == handle_type_t::source_filter))
        {
            auto fnd = FindandNotifyFilterEndpoint (hndl);
            if ((!fnd) && (checkActionFlag (hndl, required_flag)))
            {
                auto str = fmt::format ("Filter {} has no corresponding Endpoint {}", hndl.key, hndl.target);
                LOG_WARNING (global_broker_id, getIdentifier (), str);
                ActionMessage missing (CMD_ERROR, global_broker_id, hndl.fed_id);
                missing.counter = ERROR_CODE_REGISTRATION_FAILURE;
                missing.dest_handle = hndl.handle;
                missing.payload = std::move (str);
                transmit (getRoute (hndl.fed_id), missing);
            }
        }
    }
}

void CoreBroker::checkDependencies ()
{
    if (_isRoot)
    {
        for (const auto &newdep : delayedDependencies)
        {
            auto depfed = _federates.find (newdep.first);
            if (depfed != _federates.end ())
            {
                ActionMessage addDep (CMD_ADD_DEPENDENCY, newdep.second, depfed->global_id);
                routeMessage (addDep);
                addDep = ActionMessage (CMD_ADD_DEPENDENT, depfed->global_id, newdep.second);
                routeMessage (addDep);
            }
            else
            {
                ActionMessage logWarning (CMD_LOG, 0, newdep.second);
                logWarning.index = warning;
                logWarning.payload = "unable to locate " + newdep.first + " to establish dependency";
                routeMessage (logWarning);
            }
        }

        if (timeCoord->getDependents ().size () == 1)
        {  // if there is just one dependency remove it
            auto depid = timeCoord->getDependents ()[0];
            auto dependencies = timeCoord->getDependencies ();
            if (dependencies.size () == 1)
            {
                if (dependencies.front () != depid)
                {
                    ActionMessage adddep (CMD_ADD_DEPENDENT);
                    adddep.source_id = depid;
                    ActionMessage rmdep (CMD_REMOVE_DEPENDENT);
                    rmdep.source_id = global_broker_id;
                    routeMessage (adddep, dependencies.front ());
                    routeMessage (rmdep, dependencies.front ());

                    adddep.setAction (CMD_ADD_DEPENDENCY);
                    adddep.source_id = dependencies.front ();
                    rmdep.setAction (CMD_REMOVE_DEPENDENCY);
                    routeMessage (adddep, depid);
                    routeMessage (rmdep, depid);

                    timeCoord->removeDependency (dependencies.front ());
                    timeCoord->removeDependent (depid);
                }
                else
                {
                    ActionMessage rmdep (CMD_REMOVE_INTERDEPENDENCY);
                    rmdep.source_id = global_broker_id;

                    routeMessage (rmdep, depid);
                    timeCoord->removeDependency (depid);
                    timeCoord->removeDependent (depid);
                }
            }
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
            if (dep != higher_broker_id)
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
        routeMessage (rmdep, fedid);

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
    if (static_cast<decltype (minFederateCount)> (_federates.size ()) < minFederateCount)
    {
        return false;
    }
    if (static_cast<decltype (minBrokerCount)> (_brokers.size ()) < minBrokerCount)
    {
        return false;
    }

    return std::all_of (_brokers.begin (), _brokers.end (),
                        [](const auto &brk) { return ((brk._nonLocal) || (brk._initRequested)); });
}

bool CoreBroker::allDisconnected () const
{
    return std::all_of (_brokers.begin (), _brokers.end (),
                        [](const auto &brk) { return ((brk._nonLocal) || (brk._disconnected)); });
}

}  // namespace helics
