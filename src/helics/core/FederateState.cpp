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
#include "queryHelpers.hpp"
#include <algorithm>
#include <chrono>
#include <thread>

#include "helics/helics-config.h"
#include <boost/foreach.hpp>
#include "MessageTimer.hpp"


static const std::string nullStr;
#define LOG_ERROR(message) logMessage (0, nullStr, message)
#define LOG_WARNING(message) logMessage (1, nullStr, message)

#ifndef LOGGING_DISABLED
#define LOG_NORMAL(message)                                                                                       \
    do                                                                                                            \
    {                                                                                                             \
        if (logLevel >= 2)                                                                                        \
        {                                                                                                         \
            logMessage (2, nullStr, message);                                                                     \
        }                                                                                                         \
    } while (false)

#ifndef DEBUG_LOGGING_DISABLED
#define LOG_DEBUG(message)                                                                                        \
    do                                                                                                            \
    {                                                                                                             \
        if (logLevel >= 3)                                                                                        \
        {                                                                                                         \
            logMessage (3, nullStr, message);                                                                     \
        }                                                                                                         \
    } while (false)
#else
#define LOG_DEBUG(message)
#endif

#ifndef TRACE_LOGGING_DISABLED
#define LOG_TRACE(message)                                                                                        \
    do                                                                                                            \
    {                                                                                                             \
        if (logLevel >= 4)                                                                                        \
        {                                                                                                         \
            logMessage (4, nullStr, message);                                                                     \
        }                                                                                                         \
    } while (false)
#else
#define LOG_TRACE(message) ((void)0)
#endif
#else  // LOGGING_DISABLED
#define LOG_NORMAL(message) ((void)0)
#define LOG_DEBUG(message) ((void)0)
#define LOG_TRACE(message) ((void)0)
#endif  // LOGGING_DISABLED

namespace helics
{
FederateState::FederateState (const std::string &name_, const CoreFederateInfo &info_) : name (name_),logLevel(info_.logLevel)
{
    state = HELICS_CREATED;
    timeCoord = std::make_unique<TimeCoordinator> (info_);
    timeCoord->setMessageSender ([this](const ActionMessage &msg) { routeMessage (msg); });
    rt_lag = info_.rt_lag;
    rt_lead = info_.rt_lead;
    realtime = info_.realtime;
    interfaceInformation.setChangeUpdateFlag(info_.only_update_on_change);
    only_transmit_on_change = info_.only_transmit_on_change;
    
}

FederateState::~FederateState () = default;

// define the allowable state transitions for a federate
void FederateState::setState (federate_state_t newState)
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
    interfaceInformation.setGlobalId(invalid_fed_id);
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
federate_state_t FederateState::getState () const { return state; }

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


bool FederateState::checkAndSetValue (Core::handle_id_t pub_id, const char *data, uint64_t len)
{
    if (!only_transmit_on_change)
    {
        return true;
    }
    // this function could be called externally in a multi-threaded context
    std::lock_guard<std::mutex> lock (_mutex);
    auto pub = interfaceInformation.getPublication (pub_id);
    return pub->CheckSetValue (data, len);
}

uint64_t FederateState::getQueueSize (Core::handle_id_t handle_) const
{
    auto epI = interfaceInformation.getEndpoint (handle_);
    if (epI != nullptr)
    {
        return epI->queueSize (time_granted);
    }
    return 0;
}

uint64_t FederateState::getQueueSize () const
{
    uint64_t cnt = 0;
    for (const auto &end_point : interfaceInformation.getEndpoints() )
    {
        cnt += end_point->queueSize (time_granted);
    }
    return cnt;
}

std::unique_ptr<Message> FederateState::receive (Core::handle_id_t handle_)
{
    auto epI = interfaceInformation.getEndpoint (handle_);
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
    auto elock = interfaceInformation.getEndpoints();
    // Find the end point with the earliest message time
    for (auto &end_point : elock)
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

void FederateState::routeMessage (const ActionMessage &msg)
{
    if (parent_ != nullptr)
    {
        parent_->addActionMessage (msg);
    }
    else
    {
        queue.push (msg);
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
            allowed_send_time = initialTime;
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
        // timeCoord->enteringExecMode (iterate);
        ActionMessage exec (CMD_EXEC_REQUEST);
        exec.source_id = global_id;
        switch (iterate)
        {
        case iteration_request::force_iteration:
            setActionFlag (exec, iteration_requested_flag);
            setActionFlag (exec, required_flag);
            break;
        case iteration_request::iterate_if_needed:
            setActionFlag (exec, iteration_requested_flag);
            break;
        case iteration_request::no_iterations:
            break;
        }

        addAction (exec);
       
        auto ret = processQueue ();
        if (ret == iteration_state::next_step)
        {
            time_granted = timeZero;
            allowed_send_time = timeCoord->allowedSendTime ();
        }
        switch (iterate)
        {
        case iteration_request::force_iteration:
            fillEventVectorNextIteration (time_granted);
            break;
        case iteration_request::iterate_if_needed:
            if (ret == iteration_state::next_step)
            {
                fillEventVectorUpTo (time_granted);
            }
            else
            {
                fillEventVectorNextIteration (time_granted);
            }
            break;
        case iteration_request::no_iterations:
            fillEventVectorUpTo (time_granted);
            break;
        }

        processing = false;
        if ((realtime)&&(ret == iteration_state::next_step))
        {
            if (!mTimer)
            {
                mTimer = std::make_shared<MessageTimer>([this](ActionMessage &&mess) {return this->addAction(std::move(mess)); });
            }
            start_clock_time = std::chrono::steady_clock::now();
        }
        return static_cast<iteration_result> (ret);
    }
    //the following code is for situation which this has been called multiple times, which really shouldn't be done
    //but it isn't really an error so we need to deal with it.
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
        // timeCoord->timeRequest (nextTime, iterate, nextValueTime (), nextMessageTime ());

        ActionMessage treq (CMD_TIME_REQUEST);
        treq.source_id = global_id;
        treq.actionTime = nextTime;
        switch (iterate)
        {
        case iteration_request::force_iteration:
            setActionFlag (treq, iteration_requested_flag);
            setActionFlag (treq, required_flag);
            break;
        case iteration_request::iterate_if_needed:
            setActionFlag (treq, iteration_requested_flag);
            break;
        case iteration_request::no_iterations:
            break;
        }

        addAction (treq);
        LOG_TRACE (timeCoord->printTimeStatus ());
        // timeCoord->timeRequest (nextTime, iterate, nextValueTime (), nextMessageTime ());
        if ((realtime)&&(rt_lag<Time::maxVal()))
        {
            auto current_clock_time = std::chrono::steady_clock::now();
            auto timegap = current_clock_time - start_clock_time;
            auto current_lead = (nextTime + rt_lag).to_ns()-timegap;
            if (current_lead > std::chrono::milliseconds(0))
            {
                ActionMessage tforce(CMD_FORCE_TIME_GRANT);
                tforce.source_id = global_id;
                tforce.actionTime = nextTime;
                if (realTimeTimerIndex < 0)
                {
                    realTimeTimerIndex = mTimer->addTimer(current_clock_time+current_lead,std::move(tforce));
                }
                else
                {
                    mTimer->updateTimer(realTimeTimerIndex, current_clock_time + current_lead, std::move(tforce));
                }
            }
            else
            {
                ActionMessage tforce(CMD_FORCE_TIME_GRANT);
                tforce.source_id = global_id;
                tforce.actionTime = nextTime;
                addAction(tforce);
            }
        }
        auto ret = processQueue ();
        time_granted = timeCoord->getGrantedTime ();
        allowed_send_time = timeCoord->allowedSendTime ();
        iterating = (ret == iteration_state::iterating);

        iteration_time retTime = {time_granted, static_cast<iteration_result> (ret)};
        // now fill the event vector so external systems know what has been updated
        switch (iterate)
        {
        case iteration_request::force_iteration:
            fillEventVectorNextIteration (time_granted);
            break;
        case iteration_request::iterate_if_needed:
            if (time_granted < nextTime)
            {
                fillEventVectorNextIteration (time_granted);
            }
            else
            {
                fillEventVectorUpTo (time_granted);
            }
            break;
        case iteration_request::no_iterations:
            if (time_granted < nextTime)
            {
                fillEventVectorInclusive (time_granted);
            }
            else
            {
                fillEventVectorUpTo (time_granted);
            }

            break;
        }
        if (realtime)
        {
            if (rt_lag < Time::maxVal())
            {
                mTimer->cancelTimer(realTimeTimerIndex);
            }
            if (ret == iteration_state::next_step)
            {
                auto current_clock_time = std::chrono::steady_clock::now();
                auto timegap = current_clock_time - start_clock_time;
                if (time_granted - Time(timegap)>rt_lead)
                {
                    auto current_lead = (time_granted - rt_lead).to_ns() - timegap;
                    if (current_lead> std::chrono::milliseconds(5))
                    {
                        std::this_thread::sleep_for(current_lead);
                    }
                }
            }
       }
        
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

void FederateState::fillEventVectorUpTo (Time currentTime)
{
    events.clear ();
    for (auto &sub : interfaceInformation.getSubscriptions())
    {
        bool updated = sub->updateTimeUpTo (currentTime);
        if (updated)
        {
            events.push_back (sub->id);
        }
    }
}

void FederateState::fillEventVectorInclusive (Time currentTime)
{
    events.clear ();
    for (auto &sub : interfaceInformation.getSubscriptions())
    {
        bool updated = sub->updateTimeInclusive (currentTime);
        if (updated)
        {
            events.push_back (sub->id);
        }
    }
}

void FederateState::fillEventVectorNextIteration (Time currentTime)
{
    events.clear ();
    for (auto &sub : interfaceInformation.getSubscriptions())
    {
        bool updated = sub->updateTimeNextIteration (currentTime);
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
        allowed_send_time = timeCoord->allowedSendTime ();
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

iteration_state FederateState::processDelayQueue ()
{
    delayedFederates.clear ();
    auto ret_code = iteration_state::continue_processing;
    if (!delayQueue.empty ())
    {
        // copy the messages over since they could just be placed on the delay queue again
        decltype (delayQueue) tempQueue;
        std::swap (delayQueue, tempQueue);
        while ((ret_code == iteration_state::continue_processing) && (!tempQueue.empty ()))
        {
            auto &cmd = tempQueue.front ();

            if (messageShouldBeDelayed (cmd))
            {
                delayQueue.push_back (std::move (cmd));
            }
            else
            {
                ret_code = processActionMessage (cmd);
            }
            tempQueue.pop_front ();
        }
        if (ret_code != iteration_state::continue_processing)
        {
            while (!tempQueue.empty ())
            {
                auto &cmd = tempQueue.front ();
                delayQueue.push_back (std::move (cmd));
                tempQueue.pop_front ();
            }
        }
    }
    return ret_code;
}

void FederateState::addFederateToDelay (Core::federate_id_t id)
{
    if ((delayedFederates.empty ()) || (id > delayedFederates.back ()))
    {
        delayedFederates.push_back (id);
        return;
    }
    auto res = std::lower_bound (delayedFederates.begin (), delayedFederates.end (), id);
    if (res == delayedFederates.end ())
    {
        delayedFederates.push_back (id);
        return;
    }
    if (*res != id)
    {
        delayedFederates.insert (res, id);
    }
}

bool FederateState::messageShouldBeDelayed (const ActionMessage &cmd) const
{
    if (delayedFederates.empty ())
    {
        return false;
    }
    auto res = std::lower_bound (delayedFederates.begin (), delayedFederates.end (), cmd.source_id);
    return ((res != delayedFederates.end ()) && (*res == cmd.source_id));
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
    // process the delay Queue first
    auto ret_code = processDelayQueue ();

    while (ret_code == iteration_state::continue_processing)
    {
        auto cmd = queue.pop ();
        if (messageShouldBeDelayed (cmd))
        {
            delayQueue.push_back (cmd);
            continue;
        }
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
    case CMD_TIME_BLOCK:
    case CMD_TIME_UNBLOCK:
    {
        auto processed = timeCoord->processTimeMessage (cmd);
        if (processed == message_process_result::processed)
        {
            if (!timeGranted_mode)
            {
                if (state == HELICS_INITIALIZING)
                {
                    cmd.setAction(CMD_EXEC_CHECK);
                    return processActionMessage(cmd);
                }
                else if (state == HELICS_EXECUTING)
                {
                    cmd.setAction(CMD_TIME_CHECK);
                    return processActionMessage(cmd);
                }
            }
        }
        break;
    }
    case CMD_INIT_GRANT:
        if (state == HELICS_CREATED)
        {
            setState (HELICS_INITIALIZING);
            LOG_DEBUG ("Granting Initialization");
            timeGranted_mode = true;
            return iteration_state::next_step;
        }
        break;
    case CMD_EXEC_REQUEST:
        if ((cmd.source_id == global_id) && (cmd.dest_id == 0))
        {  // this sets up a time request
            iteration_request iterate = iteration_request::no_iterations;
            if (checkActionFlag (cmd, iteration_requested_flag))
            {
                iterate = (checkActionFlag (cmd, required_flag)) ? iteration_request::force_iteration :
                                                                   iteration_request::iterate_if_needed;
            }
            timeCoord->enteringExecMode (iterate);
            timeGranted_mode = false;
            auto ret = processDelayQueue();
            if (ret != iteration_state::continue_processing)
            {
                return ret;
            }
            cmd.setAction(CMD_EXEC_CHECK);
            return processActionMessage(cmd);
        }
        FALLTHROUGH
        /* FALLTHROUGH */
    case CMD_EXEC_GRANT:
        switch (timeCoord->processTimeMessage (cmd))
        {
        case message_process_result::delay_processing:
            addFederateToDelay (cmd.source_id);
            delayQueue.push_back (std::move (cmd));
            return iteration_state::continue_processing;
        case message_process_result::no_effect:
            return iteration_state::continue_processing;
        default:
            break;
        }
        FALLTHROUGH
        /* FALLTHROUGH */
    case CMD_EXEC_CHECK:  // just check the time for entry
    {
        if (state != HELICS_INITIALIZING)
        {
            break;
        }
        if (!timeGranted_mode)
        {
            auto grant = timeCoord->checkExecEntry ();
            switch (grant)
            {
            case iteration_state::iterating:
                timeGranted_mode = true;
                return grant;
            case iteration_state::next_step:
                setState (HELICS_EXECUTING);
                LOG_DEBUG ("Granting Execution");
                timeGranted_mode = true;
                return grant;
            case iteration_state::continue_processing:
                break;
            default:
                timeGranted_mode = true;
                return grant;
            }
        }
    }
    break;
    case CMD_TERMINATE_IMMEDIATELY:
        setState (HELICS_FINISHED);
        LOG_DEBUG ("Terminating");
        return iteration_state::halted;
    case CMD_STOP:
        setState (HELICS_FINISHED);
        LOG_DEBUG ("Terminating");
        return iteration_state::halted;
    case CMD_DISCONNECT:
        if (cmd.source_id == global_id)
        {
            setState (HELICS_FINISHED);
            timeCoord->disconnect ();
            cmd.dest_id = 0;
            if (parent_ != nullptr)
            {
                parent_->addActionMessage (cmd);
            }
            return iteration_state::halted;
        }
        else
        {
            switch (timeCoord->processTimeMessage (cmd))
            {
            case message_process_result::delay_processing:
                addFederateToDelay (cmd.source_id);
                delayQueue.push_back (std::move (cmd));
                return iteration_state::continue_processing;
            case message_process_result::no_effect:
                return iteration_state::continue_processing;
            default:
                break;
            }
            if (state != HELICS_EXECUTING)
            {
                break;
            }
            if (!timeGranted_mode)
            {
                auto ret = timeCoord->checkTimeGrant ();
                if (ret != iteration_state::continue_processing)
                {
                    time_granted = timeCoord->getGrantedTime ();
                    allowed_send_time = timeCoord->allowedSendTime ();
                    timeGranted_mode = true;
                    return ret;
                }
            }
        }
        break;
    case CMD_TIME_REQUEST:
        if ((cmd.source_id == global_id) && (cmd.dest_id == 0))
        {  // this sets up a time request
            iteration_request iterate = iteration_request::no_iterations;
            if (checkActionFlag (cmd, iteration_requested_flag))
            {
                iterate = (checkActionFlag (cmd, required_flag)) ? iteration_request::force_iteration :
                                                                   iteration_request::iterate_if_needed;
            }
            timeCoord->timeRequest (cmd.actionTime, iterate, nextValueTime (), nextMessageTime ());
            timeGranted_mode = false;
            auto ret=processDelayQueue ();
            if (ret != iteration_state::continue_processing)
            {
                return ret;
            }
            cmd.setAction(CMD_TIME_CHECK);
            return processActionMessage(cmd);
        }
        FALLTHROUGH
        /* FALLTHROUGH */
    case CMD_TIME_GRANT:
        switch (timeCoord->processTimeMessage (cmd))
        {
        case message_process_result::delay_processing:
            addFederateToDelay (cmd.source_id);
            delayQueue.push_back (std::move (cmd));
            return iteration_state::continue_processing;
        case message_process_result::no_effect:
            return iteration_state::continue_processing;
        default:
            break;
        }
        FALLTHROUGH
       /* FALLTHROUGH */
    case CMD_TIME_CHECK:
    {
        if (state != HELICS_EXECUTING)
        {
            break;
        }
        if (!timeGranted_mode)
        {
            auto ret = timeCoord->checkTimeGrant ();
            if (ret != iteration_state::continue_processing)
            {
                time_granted = timeCoord->getGrantedTime ();
                allowed_send_time = timeCoord->allowedSendTime ();
                LOG_DEBUG (std::string ("Granted Time=") + std::to_string (time_granted));
                timeGranted_mode = true;
                return ret;
            }
        }
    }
    break;
    case CMD_FORCE_TIME_GRANT:
    {
        if (cmd.actionTime < time_granted)
        {
            break;
        }
        timeCoord->processTimeMessage(cmd);
        time_granted = timeCoord->getGrantedTime();
        allowed_send_time = timeCoord->allowedSendTime();
        LOG_DEBUG(std::string("Granted Time=") + std::to_string(time_granted));
        timeGranted_mode = true;
        return iteration_state::next_step;
    }
    case CMD_SEND_MESSAGE:
    {
        auto epi = interfaceInformation.getEndpoint (cmd.dest_handle);
        if (epi != nullptr)
        {
            timeCoord->updateMessageTime (cmd.actionTime);
            epi->addMessage (createMessageFromCommand (std::move (cmd)));
            LOG_DEBUG ("receive_message " + prettyPrintString (cmd));
        }
    }
    break;
    case CMD_PUB:
    {
        auto subI = interfaceInformation.getSubscription (cmd.dest_handle);
        if (subI == nullptr)
        {
            break;
        }
        if (cmd.source_id == subI->target.first)
        {
            subI->addData (cmd.actionTime, cmd.counter,
                           std::make_shared<const data_block> (std::move (cmd.payload)));
            timeCoord->updateValueTime (cmd.actionTime);
            LOG_DEBUG ("receive publication " + prettyPrintString (cmd));
            LOG_TRACE (timeCoord->printTimeStatus ());
        }
    }
    break;
    case CMD_ERROR:
        setState (HELICS_ERROR);
        errorString = commandErrorString(cmd.index);
        return iteration_state::error;
    case CMD_REG_PUB:
    {
        auto subI = interfaceInformation.getSubscription (cmd.dest_handle);
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
        auto subI = interfaceInformation.getSubscription (cmd.dest_handle);
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
        auto pubI = interfaceInformation.getPublication (cmd.dest_handle);
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
                errorString = commandErrorString(cmd.index);
                return iteration_state::error;
            }
            global_id = cmd.dest_id;
            interfaceInformation.setGlobalId(cmd.dest_id);
            timeCoord->source_id = global_id;
            return iteration_state::next_step;
        }
        break;
    case CMD_FED_CONFIGURE:
        processConfigUpdate(cmd);
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
    case UPDATE_RTLAG:
        rt_lag = m.actionTime;
        break;
    case UPDATE_RTLEAD:
        rt_lead = m.actionTime;
        break;
    case UPDATE_FLAG:
        switch (m.dest_id)
        {
        case ONLY_TRANSMIT_ON_CHANGE_FLAG:
            only_transmit_on_change = checkActionFlag (m, indicator_flag);
            break;
        case ONLY_UPDATE_ON_CHANGE_FLAG:
            interfaceInformation.setChangeUpdateFlag(checkActionFlag (m, indicator_flag));
            break;
        case REALTIME_FLAG:
            realtime= checkActionFlag(m, indicator_flag);
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
    for (auto &sub : interfaceInformation.getSubscriptions())
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
    for (auto &ep : interfaceInformation.getEndpoints())
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
        loggerFunction (level,
                        (logMessageSource.empty ()) ? name + '(' + std::to_string (global_id) + ')' :
                                                      logMessageSource,
                        message);
    }
}

std::string FederateState::processQuery (const std::string &query) const
{
    if (query == "publications")
    {
        std::string ret;
        ret.push_back ('[');
        {
            for (auto &pub : interfaceInformation.getPublications())
            {
                ret.append (pub->key);
                ret.push_back (';');
    }
    if (query == "endpoints")
    {
        std::string ret;
        ret.push_back ('[');
        {
            for (auto &ept : interfaceInformation.getEndpoints())
            {
                ret.append (ept->key);
                ret.push_back (';');
    }
    if (queryCallback)
    {
        return queryCallback (query);
    }
    return "#invalid";
}
}  // namespace helics
