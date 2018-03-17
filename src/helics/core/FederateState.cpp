/*

Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "FederateState.hpp"

#include "../flag-definitions.h"
#include "EndpointInfo.hpp"
#include "PublicationInfo.hpp"
#include "SubscriptionInfo.hpp"
#include "TimeCoordinator.hpp"

#include <algorithm>
#include <chrono>
#include <thread>

#include "helics/helics-config.h"
#include <boost/foreach.hpp>

#define LOG_ERROR(message) logMessage (0, "", message);
#define LOG_WARNING(message) logMessage (1, "", message);

#ifndef LOGGING_DISABLED
#define LOG_NORMAL(message)                                                                                       \
    if (logLevel >= 2)                                                                                            \
    {                                                                                                             \
        logMessage (2, "", message);                                                                              \
    }

#ifndef DEBUG_LOGGING_DISABLED
#define LOG_DEBUG(message)                                                                                        \
    if (logLevel >= 3)                                                                                            \
    {                                                                                                             \
        logMessage (3, "", message);                                                                              \
    }
#else
#define LOG_DEBUG(message)
#endif

#ifndef TRACE_LOGGING_DISABLED
#define LOG_TRACE(message)                                                                                        \
    if (logLevel >= 4)                                                                                            \
    {                                                                                                             \
        logMessage (4, "", message);                                                                              \
    }
#else
#define LOG_TRACE(message)
#endif
#else  // LOGGING_DISABLED
#define LOG_NORMAL(message)
#define LOG_DEBUG(message)
#define LOG_TRACE(message)
#endif  // LOGGING_DISABLED

namespace helics
{
FederateState::FederateState (const std::string &name_, const CoreFederateInfo &info_) : name (name_)
{
    state = HELICS_CREATED;
    timeCoord = std::make_unique<TimeCoordinator> (info_);
    timeCoord->setMessageSender([this](const ActionMessage &msg) { routeMessage(msg); });
    
    logLevel = info_.logLevel;
    only_update_on_change = info_.only_update_on_change;
    only_transmit_on_change = info_.only_transmit_on_change;
}

FederateState::~FederateState () = default;

// define the allowable state transitions for a federate
void FederateState::setState (helics_federate_state_type newState)
{
    if (state == newState)
    {
        return;
    }
    switch (newState)
    {
    case HELICS_ERROR:
    case HELICS_FINISHED:
    case HELICS_CREATED:
        state = newState;
        break;
    case HELICS_INITIALIZING:
    {
        auto reqState = HELICS_CREATED;
        state.compare_exchange_strong (reqState, newState);
        break;
    }
    case HELICS_EXECUTING:
    {
        auto reqState = HELICS_INITIALIZING;
        state.compare_exchange_strong (reqState, newState);
        break;
    }
    case HELICS_NONE:
    default:
        break;
    }
}

void FederateState::reset ()
{
    global_id = invalid_fed_id;
    local_id = invalid_fed_id;
    state = HELICS_CREATED;
    queue.clear ();
    delayQueue.clear ();
    // TODO:: this probably needs to do a lot more
}
/** reset the federate to the initializing state*/
void FederateState::reInit ()
{
    state = HELICS_INITIALIZING;
    queue.clear ();
    delayQueue.clear ();
    // TODO:: this needs to reset a bunch of stuff as well as check a few things
}
helics_federate_state_type FederateState::getState () const { return state; }

int32_t FederateState::getCurrentIteration () const { return timeCoord->getCurrentIteration (); }

CoreFederateInfo FederateState::getInfo () const
{
    // lock the mutex to ensure we have the latest values
    std::lock_guard<std::mutex> lock (_mutex);
    return timeCoord->getFedInfo ();
}

void FederateState::updateFederateInfo (const ActionMessage &cmd)
{
    if (cmd.action () != CMD_FED_CONFIGURE)
    {
        return;
    }
    if (state == HELICS_CREATED)
    {
        std::lock_guard<std::mutex> lock (_mutex);
        processConfigUpdate (cmd);
    }
    else
    {
        addAction (cmd);
    }
}

void FederateState::createSubscription (Core::handle_id_t handle,
                                        const std::string &key,
                                        const std::string &type,
                                        const std::string &units,
                                        handle_check_mode check_mode)
{
    auto subHandle = subscriptions.lock();
	subHandle->insert(key, handle,handle, global_id, key, type, units,
		(check_mode == handle_check_mode::required));


    subHandle->back()->only_update_on_change = only_update_on_change;
}
void FederateState::createPublication (Core::handle_id_t handle,
                                       const std::string &key,
                                       const std::string &type,
                                       const std::string &units)
{

	publications.lock()->insert(key, handle, handle, global_id, key, type, units);
}

void FederateState::createEndpoint (Core::handle_id_t handle, const std::string &endpointName, const std::string &type)
{
    auto endHandle = endpoints.lock();
	endHandle->insert(endpointName, handle, handle, global_id, endpointName, type);
    hasEndpoints = true;
}

const SubscriptionInfo *FederateState::getSubscription (const std::string &subName) const
{
	return subscriptions.lock_shared()->find(subName);
}

SubscriptionInfo *FederateState::getSubscription(const std::string &subName)
{
	return subscriptions.lock()->find(subName);
}


const SubscriptionInfo *FederateState::getSubscription(Core::handle_id_t handle_) const
{
	return subscriptions.lock_shared()->find(handle_);
}

SubscriptionInfo *FederateState::getSubscription (Core::handle_id_t handle_)
{
	return subscriptions.lock()->find(handle_);
}

const PublicationInfo *FederateState::getPublication (const std::string &pubName) const
{
	return publications.lock_shared()->find(pubName);
}

const PublicationInfo *FederateState::getPublication (Core::handle_id_t handle_) const
{
	return publications.lock()->find(handle_);
}

PublicationInfo *FederateState::getPublication(const std::string &pubName)
{
	return publications.lock()->find(pubName);
}

PublicationInfo *FederateState::getPublication(Core::handle_id_t handle_)
{
	return publications.lock()->find(handle_);
}

const EndpointInfo *FederateState::getEndpoint (const std::string &endpointName) const
{
	return endpoints.lock_shared()->find(endpointName);
}

const EndpointInfo *FederateState::getEndpoint (Core::handle_id_t handle_) const
{
	return endpoints.lock_shared()->find(handle_);
}

EndpointInfo *FederateState::getEndpoint(const std::string &endpointName)
{
	return endpoints.lock()->find(endpointName);
}

EndpointInfo *FederateState::getEndpoint(Core::handle_id_t handle_)
{
	return endpoints.lock()->find(handle_);
}

bool FederateState::checkAndSetValue (Core::handle_id_t pub_id, const char *data, uint64_t len)
{
    if (!only_transmit_on_change)
    {
        return true;
    }
    // this function could be called externally in a multi-threaded context
    std::lock_guard<std::mutex> lock (_mutex);
    auto pub = getPublication (pub_id);
    return pub->CheckSetValue (data, len);
}

uint64_t FederateState::getQueueSize (Core::handle_id_t handle_) const
{
    auto epI = getEndpoint (handle_);
    if (epI != nullptr)
    {
        return epI->queueSize (time_granted);
    }
    return 0;
}

uint64_t FederateState::getQueueSize () const
{
    uint64_t cnt = 0;
    auto slock = endpoints.lock_shared();
    for (const auto &end_point :*slock)
    {
        cnt += end_point->queueSize (time_granted);
    }
    return cnt;
}

std::unique_ptr<Message> FederateState::receive (Core::handle_id_t handle_)
{
    auto epI = getEndpoint (handle_);
    if (epI != nullptr)
    {
        return epI->getMessage (time_granted);
    }
    return nullptr;
}

std::unique_ptr<Message> FederateState::receiveAny (Core::handle_id_t &id)
{
    Time earliest_time = Time::maxVal ();
    EndpointInfo *endpointI = nullptr;
    auto slock = endpoints.lock_shared();
    // Find the end point with the earliest message time
    for (auto &end_point : *slock)
    {
        auto t = end_point->firstMessageTime ();
        if (t < earliest_time)
        {
            earliest_time = t;
            endpointI = end_point.get ();
        }
    }
    if (endpointI == nullptr)
    {
        return nullptr;
    }
    // Return the message found and remove from the queue
    if (earliest_time <= time_granted)
    {
        auto result = endpointI->getMessage (time_granted);
        id = endpointI->id;
        return result;
    }
    id = invalid_handle;
    return nullptr;
}

void FederateState::routeMessage(const ActionMessage &msg)
{
    if (msg.dest_id == global_id)
    {
        addAction(msg);
    }
    else if (parent_!=nullptr)
    {
        parent_->addActionMessage(msg);
    }
}

void FederateState::addAction (const ActionMessage &action)
{
    if (action.action () != CMD_IGNORE)
    {
        queue.push (action);
    }
}

void FederateState::addAction (ActionMessage &&action)
{
    if (action.action () != CMD_IGNORE)
    {
        queue.push (std::move (action));
    }
}

iteration_result FederateState::waitSetup ()
{
    bool expected = false;
    if (processing.compare_exchange_strong (expected, true))
    {  // only enter this loop once per federate
        auto ret = processQueue ();
        processing = false;
        return static_cast<iteration_result> (ret);
    }

    while (!processing.compare_exchange_weak (expected, true))
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (20));
    }
    iteration_result ret;
    switch (getState ())
    {
    case HELICS_ERROR:
        ret = iteration_result::error;
        break;
    case HELICS_FINISHED:
        ret = iteration_result::halted;
        break;
    default:
        ret = iteration_result::next_step;
        break;
    }

    processing = false;
    return ret;
}

iteration_result FederateState::enterInitializationState ()
{
    bool expected = false;
    if (processing.compare_exchange_strong (expected, true))
    {  // only enter this loop once per federate
        auto ret = processQueue ();
        processing = false;
        if (ret == iteration_state::next_step)
        {
            time_granted = initialTime;
        }
        return static_cast<iteration_result> (ret);
    }

    while (!processing.compare_exchange_weak (expected, true))
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (20));
    }
    iteration_result ret;
    switch (getState ())
    {
    case HELICS_ERROR:
        ret = iteration_result::error;
        break;
    case HELICS_FINISHED:
        ret = iteration_result::halted;
        break;
    case HELICS_CREATED:
        // not sure this can actually happen
        ret = iteration_result::iterating;
        break;
    default:  // everything >= HELICS_INITIALIZING
        ret = iteration_result::next_step;
        break;
    }
    processing = false;
    return ret;
}

iteration_result FederateState::enterExecutingState (iteration_request iterate)
{
    bool expected = false;
    if (processing.compare_exchange_strong (expected, true))
    {  // only enter this loop once per federate
        //timeCoord->enteringExecMode (iterate);
        ActionMessage exec(CMD_EXEC_REQUEST);
        exec.source_id = global_id;
        switch (iterate)
        {
        case iteration_request::force_iteration:
            setActionFlag(exec, iteration_requested_flag);
            setActionFlag(exec, required_flag);
            break;
        case iteration_request::iterate_if_needed:
            setActionFlag(exec, iteration_requested_flag);
            break;
        case iteration_request::no_iterations:
            break;
        }

        addAction(exec);
        ActionMessage execReq(CMD_EXEC_CHECK);
        queue.emplace(execReq);
        auto ret = processQueue ();
        if (ret == iteration_state::next_step)
        {
            time_granted = timeZero;
        }
        fillEventVector (time_granted);
        
        processing = false;
        return static_cast<iteration_result> (ret);
    }

    while (!processing.compare_exchange_weak (expected, true))
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (50));
    }
    iteration_result ret;
    switch (getState ())
    {
    case HELICS_ERROR:
        ret = iteration_result::error;
        break;
    case HELICS_FINISHED:
        ret = iteration_result::halted;
        break;
    case HELICS_CREATED:
    case HELICS_INITIALIZING:
    default:
        ret = iteration_result::iterating;
        break;
    case HELICS_EXECUTING:
        ret = iteration_result::next_step;
        break;
    }
    processing = false;
    return ret;
}

iteration_time FederateState::requestTime (Time nextTime, iteration_request iterate)
{
    bool expected = false;
    if (processing.compare_exchange_strong (expected, true))
    {  // only enter this loop once per federate
        events.clear ();  // clear the event queue
        LOG_TRACE (timeCoord->printTimeStatus ());
        //timeCoord->timeRequest (nextTime, iterate, nextValueTime (), nextMessageTime ());

        ActionMessage treq(CMD_TIME_REQUEST);
        treq.source_id = global_id;
        treq.actionTime = nextTime;
        switch (iterate)
        {
        case iteration_request::force_iteration:
            setActionFlag(treq, iteration_requested_flag);
            setActionFlag(treq, required_flag);
            break;
        case iteration_request::iterate_if_needed:
            setActionFlag(treq, iteration_requested_flag);
            break;
        case iteration_request::no_iterations:
            break;
        }

        addAction(treq);
        LOG_TRACE(timeCoord->printTimeStatus());
        //timeCoord->timeRequest (nextTime, iterate, nextValueTime (), nextMessageTime ());
        queue.push(CMD_TIME_CHECK);

        auto ret = processQueue ();
        time_granted = timeCoord->getGrantedTime ();
        allowed_send_time = timeCoord->allowedSendTime();
        iterating = (ret == iteration_state::iterating);

        iteration_time retTime = {time_granted, static_cast<iteration_result> (ret)};
        // now fill the event vector so external systems know what has been updated
        fillEventVector (time_granted);
        processing = false;
        return retTime;
    }
    // this would not be good practice to get into this part of the function
    // but the area must protect itself and should return something sensible
    while (!processing.compare_exchange_weak (expected, true))
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (20));
    }
    iteration_result ret = iterating ? iteration_result::iterating : iteration_result::next_step;
    if (state == HELICS_FINISHED)
    {
        ret = iteration_result::halted;
    }
    else if (state == HELICS_ERROR)
    {
        ret = iteration_result::error;
    }
    iteration_time retTime = {time_granted, ret};
    processing = false;
    return retTime;
}

void FederateState::fillEventVector (Time currentTime)
{
    events.clear ();
    auto slock = subscriptions.lock_shared();
    for (auto &sub : *slock)
    {
        bool updated = sub->updateTime (currentTime);
        if (updated)
        {
            events.push_back (sub->id);
        }
    }
}

iteration_result FederateState::genericUnspecifiedQueueProcess ()
{
    bool expected = false;
    if (processing.compare_exchange_strong (expected, true))
    {  // only 1 thread can enter this loop once per federate
        auto ret = processQueue ();
        time_granted = timeCoord->getGrantedTime ();
        processing = false;
        return static_cast<iteration_result> (ret);
    }

    while (!processing.compare_exchange_weak (expected, true))
    {
        std::this_thread::sleep_for (std::chrono::milliseconds (20));
    }
    processing = false;
    return iteration_result::next_step;
}

const std::vector<Core::handle_id_t> emptyHandles;

const std::vector<Core::handle_id_t> &FederateState::getEvents () const
{
    if (!processing)
    {  //!< if we are processing this vector is in an undefined state
        return events;
    }
    return emptyHandles;
}

iteration_state FederateState::processQueue ()
{
    if (state == HELICS_FINISHED)
    {
        return iteration_state::halted;
    }
    if (state == HELICS_ERROR)
    {
        return iteration_state::error;
    }
    auto ret_code = iteration_state::continue_processing;

    // process the delay Queue first
    if (!delayQueue.empty ())
    {
        // copy the messages over since they could just be placed on the delay queue again
        decltype (delayQueue) tempQueue;
        std::swap (delayQueue, tempQueue);
        while ((ret_code == iteration_state::continue_processing) && (!tempQueue.empty ()))
        {
            auto cmd = tempQueue.front ();
            tempQueue.pop_front ();
            ret_code = processActionMessage (cmd);
        }
        if (ret_code != iteration_state::continue_processing)
        {
            if (!tempQueue.empty ())
            {
                while (!tempQueue.empty ())
                {
                    auto cmd = tempQueue.back ();
                    tempQueue.pop_back ();
                    delayQueue.push_front (cmd);
                }
            }
            return ret_code;
        }
    }

    while (ret_code == iteration_state::continue_processing)
    {
        auto cmd = queue.pop ();
        //    messLog.push_back(cmd);
        ret_code = processActionMessage (cmd);
    }
    return ret_code;
}

iteration_state FederateState::processActionMessage (ActionMessage &cmd)
{
    LOG_TRACE ("processing cmd " + prettyPrintString (cmd));
    switch (cmd.action ())
    {
    case CMD_IGNORE:
    default:
        break;
    case CMD_INIT_GRANT:
        if (state == HELICS_CREATED)
        {
            setState (HELICS_INITIALIZING);
            LOG_DEBUG("Granting Initialization");
            return iteration_state::next_step;
        }
        break;
    case CMD_EXEC_REQUEST:
        if ((cmd.source_id == global_id) && (cmd.dest_id == 0))
        { //this sets up a time request
            iteration_request iterate = iteration_request::no_iterations;
            if (checkActionFlag(cmd, iteration_requested_flag))
            {
                iterate = (checkActionFlag(cmd, required_flag)) ? iteration_request::force_iteration : iteration_request::iterate_if_needed;
            }
            timeCoord->enteringExecMode(iterate);
            break;

        }
        FALLTHROUGH
    case CMD_EXEC_GRANT:
        if (!timeCoord->processTimeMessage (cmd))
        {
            break;
        }
        FALLTHROUGH
    case CMD_EXEC_CHECK:  // just check the time for entry
    {
        if (state != HELICS_INITIALIZING)
        {
            break;
        }
        auto grant = timeCoord->checkExecEntry ();
        switch (grant)
        {
        case iteration_state::iterating:

            return grant;
        case iteration_state::next_step:
            setState (HELICS_EXECUTING);
            LOG_DEBUG("Granting Execution");
            return grant;
        case iteration_state::continue_processing:
            break;
        default:
            return grant;
        }
    }
    break;
    case CMD_TERMINATE_IMMEDIATELY:
        setState (HELICS_FINISHED);
        LOG_DEBUG("Terminating");
        return iteration_state::halted;
    case CMD_STOP:
        setState (HELICS_FINISHED);
        LOG_DEBUG("Terminating");
        return iteration_state::halted;
    case CMD_DISCONNECT:
        if (cmd.source_id == global_id)
        {
            setState (HELICS_FINISHED);
            timeCoord->disconnect();
            return iteration_state::halted;
        }
        else
        {
            if (timeCoord->processTimeMessage (cmd))
            {
                if (state != HELICS_EXECUTING)
                {
                    break;
                }
                auto ret = timeCoord->checkTimeGrant ();
                if (ret != iteration_state::continue_processing)
                {
                    time_granted = timeCoord->getGrantedTime ();
                    return ret;
                }
            }
        }
        break;
    case CMD_TIME_REQUEST:
        if ((cmd.source_id == global_id) && (cmd.dest_id == 0))
        { //this sets up a time request
            iteration_request iterate = iteration_request::no_iterations;
            if (checkActionFlag(cmd, iteration_requested_flag))
            {
                iterate = (checkActionFlag(cmd, required_flag)) ? iteration_request::force_iteration : iteration_request::iterate_if_needed;
            }
            timeCoord->timeRequest(cmd.actionTime, iterate, nextValueTime(), nextMessageTime());
            break;
        }
        FALLTHROUGH
    case CMD_TIME_GRANT:
        if (!timeCoord->processTimeMessage (cmd))
        {
            break;
        }
        FALLTHROUGH
    case CMD_TIME_CHECK:
    {
        if (state != HELICS_EXECUTING)
        {
            break;
        }
        auto ret = timeCoord->checkTimeGrant ();
        if (ret != iteration_state::continue_processing)
        {
            time_granted = timeCoord->getGrantedTime ();
            LOG_DEBUG(std::string("Granted Time=") + std::to_string(time_granted));
            return ret;
        }
    }
    break;
    case CMD_SEND_MESSAGE:
    {
        auto epi = getEndpoint (cmd.dest_handle);
        if (epi != nullptr)
        {
            timeCoord->updateMessageTime (cmd.actionTime);
            epi->addMessage (createMessageFromCommand (std::move (cmd)));
            LOG_DEBUG("receive_message " + prettyPrintString(cmd));
        }
    }
    break;
    case CMD_PUB:
    {
        auto subI = getSubscription (cmd.dest_handle);
        if (subI == nullptr)
        {
            break;
        }
        if (cmd.source_id == subI->target.first)
        {
            subI->addData (cmd.actionTime, std::make_shared<const data_block> (std::move (cmd.payload)));
            timeCoord->updateValueTime (cmd.actionTime);
            LOG_DEBUG("receive publication " + prettyPrintString(cmd));
            LOG_TRACE (timeCoord->printTimeStatus ());
        }
    }
    break;
    case CMD_ERROR:
        setState (HELICS_ERROR);
        return iteration_state::error;
    case CMD_REG_PUB:
    {
        auto subI = getSubscription (cmd.dest_handle);
        if (subI != nullptr)
        {
            subI->target = {cmd.source_id, cmd.source_handle};
            subI->pubType = cmd.info ().type;
            addDependency (cmd.source_id);
        }
    }
    break;
    case CMD_NOTIFY_PUB:
    {
        auto subI = getSubscription (cmd.dest_handle);
        if (subI != nullptr)
        {
            subI->target = {cmd.source_id, cmd.source_handle};
            subI->pubType = cmd.payload;
            addDependency (cmd.source_id);
        }
    }
    break;
    case CMD_REG_SUB:
    case CMD_NOTIFY_SUB:
    {
        auto pubI = getPublication (cmd.dest_handle);
        if (pubI != nullptr)
        {
            pubI->subscribers.emplace_back (cmd.source_id, cmd.source_handle);
            addDependent (cmd.source_id);
        }
    }
    break;
    case CMD_ADD_DEPENDENCY:
    case CMD_REMOVE_DEPENDENCY:
    case CMD_ADD_DEPENDENT:
    case CMD_REMOVE_DEPENDENT:
    case CMD_ADD_INTERDEPENDENCY:
    case CMD_REMOVE_INTERDEPENDENCY:
        if (cmd.dest_id == global_id)
        {
            timeCoord->processDependencyUpdateMessage (cmd);
        }

        break;
    case CMD_REG_DST_FILTER:
    case CMD_NOTIFY_DST_FILTER:
    {
        auto endI = getEndpoint (cmd.dest_handle);
        if (endI != nullptr)
        {
            addDependency (cmd.source_id);
            // todo probably need to do something more here
        }
        break;
    }
    case CMD_REG_SRC_FILTER:
    case CMD_NOTIFY_SRC_FILTER:
    {
        auto endI = getEndpoint (cmd.dest_handle);
        if (endI != nullptr)
        {
            endI->hasFilter = true;
            addDependent (cmd.source_id);
            // todo probably need to do something more here
        }
    }
    break;
    case CMD_FED_ACK:
        if (state != HELICS_CREATED)
        {
            break;
        }
        if (cmd.name == name)
        {
            if (checkActionFlag (cmd, error_flag))
            {
                setState (HELICS_ERROR);
                return iteration_state::error;
            }
            global_id = cmd.dest_id;
            timeCoord->source_id = global_id;
            return iteration_state::next_step;
        }
        break;
    }
    return iteration_state::continue_processing;
}

void FederateState::processConfigUpdate (const ActionMessage &m)
{
    timeCoord->processConfigUpdateMessage (m, (getState () == HELICS_CREATED));
    switch (m.index)
    {
    case UPDATE_LOG_LEVEL:
        logLevel = static_cast<int> (m.dest_id);
        break;
    case UPDATE_FLAG:
        switch (m.dest_id)
        {
        case ONLY_TRANSMIT_ON_CHANGE_FLAG:
            only_transmit_on_change = checkActionFlag (m, indicator_flag);
            break;
        case ONLY_UPDATE_ON_CHANGE_FLAG:
            only_update_on_change = checkActionFlag (m, indicator_flag);
            break;
        default:
            break;
        }
    default:
        break;
    }
}
const std::vector<Core::federate_id_t> &FederateState::getDependents () const
{
    return timeCoord->getDependents ();
}

void FederateState::addDependency (Core::federate_id_t fedToDependOn) { timeCoord->addDependency (fedToDependOn); }

void FederateState::addDependent (Core::federate_id_t fedThatDependsOnThis)
{
    timeCoord->addDependent (fedThatDependsOnThis);
}

Time FederateState::nextValueTime () const
{
    auto firstValueTime = Time::maxVal ();
    auto slock = subscriptions.lock_shared();
    for (auto &sub : *slock)
    {
        auto nvt = sub->nextValueTime ();
        if (nvt >= time_granted)
        {
            if (nvt < firstValueTime)
            {
                firstValueTime = nvt;
            }
        }
    }
    return firstValueTime;
}

/** find the next Message Event*/
Time FederateState::nextMessageTime () const
{
    auto firstMessageTime = Time::maxVal ();
    auto slock = endpoints.lock_shared();
    for (auto &ep : *slock)
    {
        auto messageTime = ep->firstMessageTime ();
        if (messageTime >= time_granted)
        {
            if (messageTime < firstMessageTime)
            {
                firstMessageTime = messageTime;
            }
        }
    }
    return firstMessageTime;
}

void FederateState::setCoreObject (CommonCore *parent)
{
    std::lock_guard<std::mutex> lock (_mutex);
    parent_ = parent;
}

void FederateState::logMessage (int level, const std::string &logMessageSource, const std::string &message) const
{
    if ((loggerFunction) && (level <= logLevel))
    {
        loggerFunction (level, (logMessageSource.empty ()) ? name+"("+std::to_string(global_id)+")" : logMessageSource, message);
    }
}

std::string FederateState::processQuery (const std::string &query) const
{
    if (query == "publications")
    {
        std::string ret;
        ret.push_back ('[');
        {
            auto pubHandle = publications.lock_shared();
            for (auto &pub : *pubHandle)
            {
            ret.append (pub->key);
            ret.push_back (';');
        }
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
    if (query == "endpoints")
    {
        std::string ret;
        ret.push_back ('[');
        {
            auto endHandle = endpoints.lock_shared();
            for (auto &ept : *endHandle)
            {
            ret.append (ept->key);
            ret.push_back (';');
        }
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
    if (queryCallback)
    {
        return queryCallback (query);
    }
    return "#invalid";
}
}  // namespace helics

