/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "FederateState.h"

#include "EndpointInfo.h"
#include "FilterInfo.h"
#include "PublicationInfo.h"
#include "SubscriptionInfo.h"

#include <algorithm>
#include <chrono>
#include <thread>

namespace helics
{
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
    }
}

void FederateState::reset ()
{
    state = HELICS_CREATED;
    // TODO:: this probably needs to do a lot more
}
/** reset the federate to the initializing state*/
void FederateState::reInit ()
{
    state = HELICS_INITIALIZING;
    // TODO:: this needs to reset a bunch of stuff as well as check a few things
}
helics_federate_state_type FederateState::getState () const { return state; }

static auto compareFunc = [](const auto &A, const auto &B) { return (A->id < B->id); };

CoreFederateInfo FederateState::getInfo () const
{
    // lock the mutex to ensure we have the latest values
    std::lock_guard<std::mutex> lock (_mutex);
    return info;
}

void FederateState::UpdateFederateInfo (CoreFederateInfo &newInfo)
{
    std::lock_guard<std::mutex> lock (_mutex);
    info = newInfo;
}

void FederateState::createSubscription (Core::Handle handle,
                                        const std::string &key,
                                        const std::string &type,
                                        const std::string &units,
                                        handle_check_mode check_mode)
{
    auto sub = std::make_unique<SubscriptionInfo> (handle, global_id, key, type, units,
                                                   (check_mode == handle_check_mode::required));

    std::lock_guard<std::mutex> lock (_mutex);
    subNames.emplace (key, sub.get ());

    // need to sort the vectors so the find works properly
    if (subscriptions.empty () || handle > subscriptions.back ()->id)
    {
        subscriptions.push_back (std::move (sub));
    }
    else
    {
        subscriptions.push_back (std::move (sub));
        std::sort (subscriptions.begin (), subscriptions.end (), compareFunc);
    }
}
void FederateState::createPublication (Core::Handle handle,
                                       const std::string &key,
                                       const std::string &type,
                                       const std::string &units)
{
    auto pub = std::make_unique<PublicationInfo> (handle, global_id, key, type, units);

    std::lock_guard<std::mutex> lock (_mutex);
    pubNames.emplace (key, pub.get ());

    // need to sort the vectors so the find works properly
    if (publications.empty () || handle > publications.back ()->id)
    {
        publications.push_back (std::move (pub));
    }
    else
    {
        publications.push_back (std::move (pub));
        std::sort (publications.begin (), publications.end (), compareFunc);
    }
}

void FederateState::createEndpoint (Core::Handle handle, const std::string &key, const std::string &type)
{
    auto ep = std::make_unique<EndpointInfo> (handle, global_id, key, type);

    std::lock_guard<std::mutex> lock (_mutex);
    epNames.emplace (key, ep.get ());
    hasEndpoints = true;
    if (endpoints.empty () || handle > endpoints.back ()->id)
    {
        endpoints.push_back (std::move (ep));
    }
    else
    {
        endpoints.push_back (std::move (ep));
        std::sort (endpoints.begin (), endpoints.end (), compareFunc);
    }
}

void FederateState::createSourceFilter (Core::Handle handle,
                                        const std::string &key,
                                        const std::string &target,
                                        const std::string &type)
{
    auto filt = std::make_unique<FilterInfo> (handle, global_id, key, target, type, false);

    std::lock_guard<std::mutex> lock (_mutex);
    filterNames.emplace (key, filt.get ());

    if (filters.empty () || handle > filters.back ()->id)
    {
        filters.push_back (std::move (filt));
    }
    else
    {
        filters.push_back (std::move (filt));
        std::sort (filters.begin (), filters.end (), compareFunc);
    }
}

void FederateState::createDestFilter (Core::Handle handle,
                                      const std::string &key,
                                      const std::string &target,
                                      const std::string &type)
{
    auto filt = std::make_unique<FilterInfo> (handle, global_id, key, target, type, true);

    std::lock_guard<std::mutex> lock (_mutex);
    filterNames.emplace (key, filt.get ());

    if (filters.empty () || handle > filters.back ()->id)
    {
        filters.push_back (std::move (filt));
    }
    else
    {
        filters.push_back (std::move (filt));
        std::sort (filters.begin (), filters.end (), compareFunc);
    }
}

SubscriptionInfo *FederateState::getSubscription (const std::string &subName) const
{
    auto fnd = subNames.find (subName);
    if (fnd != subNames.end ())
    {
        return fnd->second;
    }
    return nullptr;
}


SubscriptionInfo *FederateState::getSubscription (Core::Handle handle_) const
{
    static auto cmptr = [](const std::unique_ptr<SubscriptionInfo> &ptrA, Core::Handle handle) {
        return (ptrA->id < handle);
    };

    auto fnd = std::lower_bound (subscriptions.begin (), subscriptions.end (), handle_, cmptr);
    if (fnd->operator-> ()->id == handle_)
    {
        return fnd->get ();
    }
    return nullptr;
}

PublicationInfo *FederateState::getPublication (const std::string &pubName) const
{
    auto fnd = pubNames.find (pubName);
    if (fnd != pubNames.end ())
    {
        return fnd->second;
    }
    return nullptr;
}

PublicationInfo *FederateState::getPublication (Core::Handle handle_) const
{
    static auto cmptr = [](const std::unique_ptr<PublicationInfo> &ptrA, Core::Handle handle) {
        return (ptrA->id < handle);
    };

    auto fnd = std::lower_bound (publications.begin (), publications.end (), handle_, cmptr);
    if (fnd->operator-> ()->id == handle_)
    {
        return fnd->get ();
    }
    return nullptr;
}

EndpointInfo *FederateState::getEndpoint (const std::string &endpointName) const
{
    auto fnd = epNames.find (endpointName);
    if (fnd != epNames.end ())
    {
        return fnd->second;
    }
    return nullptr;
}

EndpointInfo *FederateState::getEndpoint (Core::Handle handle_) const
{
    static auto cmptr = [](const std::unique_ptr<EndpointInfo> &ptrA, Core::Handle handle) {
        return (ptrA->id < handle);
    };

    auto fnd = std::lower_bound (endpoints.begin (), endpoints.end (), handle_, cmptr);
    if (fnd->operator-> ()->id == handle_)
    {
        return fnd->get ();
    }
    return nullptr;
}


FilterInfo *FederateState::getFilter (const std::string &filterName) const
{
    auto fnd = filterNames.find (filterName);
    if (fnd != filterNames.end ())
    {
        return fnd->second;
    }
    return nullptr;
}


FilterInfo *FederateState::getFilter (Core::Handle handle_) const
{
    static auto cmptr = [](const std::unique_ptr<FilterInfo> &ptrA, Core::Handle handle) {
        return (ptrA->id < handle);
    };

    auto fnd = std::lower_bound (filters.begin (), filters.end (), handle_, cmptr);
    if (fnd->operator-> ()->id == handle_)
    {
        return fnd->get ();
    }
    return nullptr;
}


uint64_t FederateState::getQueueSize (Core::Handle handle_) const
{
    auto epI = getEndpoint (handle_);
    if (epI != nullptr)
    {
        return epI->queueSize (time_granted);
    }
    auto fI = getFilter (handle_);
    if (fI != nullptr)
    {
        return fI->queueSize (time_granted);
    }
    return 0;
}


uint64_t FederateState::getQueueSize () const
{
    uint64_t cnt = 0;
    for (auto &end_point : endpoints)
    {
        cnt += end_point->queueSize (time_granted);
    }
    return cnt;
}

uint64_t FederateState::getFilterQueueSize () const
{
    uint64_t cnt = 0;
    for (auto &filt : filters)
    {
        cnt += filt->queueSize (time_granted);
    }
    return cnt;
}


std::unique_ptr<Message> FederateState::receive (Core::Handle handle_)
{
    auto epI = getEndpoint (handle_);
    if (epI != nullptr)
    {
        return epI->getMessage (time_granted);
    }
    auto fI = getFilter (handle_);
    if (fI != nullptr)
    {
        return fI->getMessage (time_granted);
    }
    return nullptr;
}


std::unique_ptr<Message> FederateState::receiveAny (Core::Handle &id)
{
    Time earliest_time = Time::maxVal ();
    EndpointInfo *endpointI = nullptr;
    // Find the end point with the earliest message time
    for (auto &end_point : endpoints)
    {
        auto t = end_point->firstMessageTime ();
        if (t < earliest_time)
        {
            t = earliest_time;
            endpointI = end_point.get ();
        }
    }

    // Return the message found and remove from the queue
    if (earliest_time <= time_granted)
    {
        auto result = endpointI->getMessage (time_granted);
        id = endpointI->id;
        return result;
    }
    id = invalid_Handle;
    return nullptr;
}

std::unique_ptr<Message> FederateState::receiveForFilter (Core::Handle &id)
{
    Time earliest_time = Time::maxVal ();
    FilterInfo *filterI = nullptr;
    // Find the end point with the earliest message time
    for (auto &filt : filters)
    {
        auto t = filt->firstMessageTime ();
        if (t < earliest_time)
        {
            t = earliest_time;
            filterI = filt.get ();
        }
    }

    // Return the message found and remove from the queue
    if (earliest_time <= time_granted)
    {
        auto result = filterI->getMessage (time_granted);
        id = filterI->id;
        return result;
    }
    id = invalid_Handle;
    return nullptr;
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

convergence_state FederateState::waitSetup ()
{
    bool expected = false;
    if (processing.compare_exchange_strong (expected, true))
    {  // only enter this loop once per federate
        auto ret = processQueue ();
        processing = false;
        return ret;
    }
    else
    {
        while (!processing.compare_exchange_weak (expected, true))
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (20));
        }
        convergence_state ret;
        switch (getState ())
        {
        case HELICS_ERROR:
            ret = convergence_state::error;
            break;
        case HELICS_FINISHED:
            ret = convergence_state::halted;
            break;
        default:
            ret = convergence_state::complete;
            break;
        }

        processing = false;
        return ret;
    }
}
/** process until the init state has been entered or there is a failure*/
convergence_state FederateState::enterInitState ()
{
    bool expected = false;
    if (processing.compare_exchange_strong (expected, true))
    {  // only enter this loop once per federate
        auto ret = processQueue ();
        processing = false;
        return ret;
    }
    else
    {
        while (!processing.compare_exchange_weak (expected, true))
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (20));
        }
        convergence_state ret;
        switch (getState ())
        {
        case HELICS_ERROR:
            ret = convergence_state::error;
            break;
        case HELICS_FINISHED:
            ret = convergence_state::halted;
            break;
        case HELICS_CREATED:
            // not sure this can actually happen
            ret = convergence_state::nonconverged;
            break;
        default:  // everything >= HELICS_INITIALIZING
            ret = convergence_state::complete;
            break;
        }
        processing = false;
        return ret;
    }
}

convergence_state FederateState::enterExecutingState (convergence_state converged)
{
    bool expected = false;
    if (processing.compare_exchange_strong (expected, true))
    {  // only enter this loop once per federate
        if (parent_ != nullptr)
        {
            ActionMessage execreq (CMD_EXEC_REQUEST);
            execreq.source_id = global_id;
            execreq.iterationComplete = (converged == convergence_state::complete);
            parent_->addCommand (execreq);
        }
        auto ret = processQueue ();
        if (parent_ != nullptr)
        {
            ActionMessage execgrant (CMD_EXEC_GRANT);
            execgrant.source_id = global_id;
            execgrant.iterationComplete = (ret == convergence_state::complete);
            parent_->addCommand (execgrant);
        }
        time_granted = timeZero;
        processing = false;
        return ret;
    }
    else
    {
        while (!processing.compare_exchange_weak (expected, true))
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (20));
        }
        convergence_state ret;
        switch (getState ())
        {
        case HELICS_ERROR:
            ret = convergence_state::error;
            break;
        case HELICS_FINISHED:
            ret = convergence_state::halted;
            break;
        case HELICS_CREATED:
        case HELICS_INITIALIZING:
        default:
            ret = convergence_state::nonconverged;
            break;
        case HELICS_EXECUTING:
            ret = convergence_state::complete;
            break;
        }
        processing = false;
        return ret;
    }
}

iterationTime FederateState::requestTime (Time nextTime, convergence_state converged)
{
    bool expected = false;
    if (processing.compare_exchange_strong (expected, true))
    {  // only enter this loop once per federate
        iterating = (converged != convergence_state::complete);
        time_requested = nextTime;
        updateNextExecutionTime ();
        updateNextPossibleEventTime (converged);
        if (parent_ != nullptr)
        {
            ActionMessage treq (CMD_TIME_REQUEST);
            treq.source_id = global_id;
            treq.actionTime = time_next;
            treq.info ().Te = time_exec + info.lookAhead;
            treq.info ().Tdemin = time_minDe;
            parent_->addCommand (treq);
        }


        // push a message to check whether time can be granted
        queue.push (CMD_TIME_CHECK);
        auto ret = processQueue ();

        iterating = (ret == convergence_state::nonconverged);
        iterationTime retTime = {time_granted, ret};
        if (parent_ != nullptr)
        {
            ActionMessage treq (CMD_TIME_GRANT);
            treq.source_id = global_id;
            treq.actionTime = time_granted;
            parent_->addCommand (treq);
        }
        processing = false;
        return retTime;
    }
    else
    {
        // this would not be good practice to get into this part of the function
        // but the area must protect itself
        while (!processing.compare_exchange_weak (expected, true))
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (20));
        }
        iterationTime retTime = {time_granted,
                                 iterating ? convergence_state::nonconverged : convergence_state::complete};
        processing = false;
        return retTime;
    }
}

convergence_state FederateState::genericUnspecifiedQueueProcess ()
{
    bool expected = false;
    if (processing.compare_exchange_strong (expected, true))
    {  // only 1 thread can enter this loop once per federate
        auto ret = processQueue ();
        processing = false;
        return ret;
    }
    else
    {
        while (!processing.compare_exchange_weak (expected, true))
        {
            std::this_thread::sleep_for (std::chrono::milliseconds (20));
        }
        processing = false;
        return convergence_state::complete;
    }
}

convergence_state FederateState::processQueue ()
{
	if (state == HELICS_FINISHED)
	{
		return convergence_state::halted;
	}
	if (state == HELICS_ERROR)
	{
		return convergence_state::error;
	}
    auto cmd = queue.pop ();
    while (1)
    {
        switch (cmd.action ())
        {
        case CMD_IGNORE:
        default:
            break;
        case CMD_INIT_GRANT:
			if (state == HELICS_CREATED)
			{
				setState(HELICS_INITIALIZING);
				return convergence_state::complete;
			}
			break;
        case CMD_EXEC_REQUEST:
        case CMD_EXEC_GRANT:
            if (!processExecRequest (cmd))
            {
                break;
            }
        // FALLTHROUGH
        case CMD_EXEC_CHECK:  // just check the time for entry
        {
            if (state != HELICS_INITIALIZING)
            {
                break;
            }
            auto grant = checkExecEntry ();
            switch (grant)
            {
            case convergence_state::nonconverged:

                return grant;
            case convergence_state::complete:
                setState (HELICS_EXECUTING);
                // TODO: send a time granted message
                // ActionMessage grant(CMD_EXEC_GRANT);
                // grant.source_id = global_id;
                return grant;
            case convergence_state::continue_processing:
                break;
            default:
                return grant;
            }
        }
        break;
        case CMD_STOP:
        case CMD_DISCONNECT:
            if (cmd.dest_id == global_id)
            {
                setState (HELICS_FINISHED);
                return convergence_state::halted;
            }
            break;
        case CMD_TIME_REQUEST:
        case CMD_TIME_GRANT:
            if (!processExternalTimeMessage (cmd))
            {
                break;
            }
        // FALLTHROUGH
        case CMD_TIME_CHECK:
        {
			if (state != HELICS_EXECUTING)
			{
				break;
			}
            auto update = updateTimeFactors ();
            switch (update)
            {
            case convergence_state::nonconverged:
                break;
            case convergence_state::complete:
				time_granted = time_exec;
                return update;
            case convergence_state::continue_processing:
				//generate and send an updated time request message
				if (parent_ != nullptr)
				{
					ActionMessage upd(CMD_TIME_REQUEST);
					upd.source_id = global_id;
					upd.actionTime = time_next;
					upd.info().Te = time_exec;
					upd.info().Tdemin = time_minDe;
					parent_->addCommand(upd);
				}
                break;
            default:
                return update;
            }
        }
        break;
        case CMD_SEND_MESSAGE:
        {
            auto epi = getEndpoint (cmd.dest_handle);
            if (epi != nullptr)
            {
                updateMessageTime (cmd);
                epi->addMessage (createMessage (std::move (cmd)));
            }
        }
        break;
        case CMD_SEND_FOR_FILTER:
        {
            // this should only be non time_agnostic filters
            auto fI = getFilter (cmd.dest_handle);
            if (fI != nullptr)
            {
                updateMessageTime (cmd);
                fI->addMessage (createMessage (std::move (cmd)));
            }
        }
        break;
        case CMD_PUB:
        {
            auto subI = getSubscription (cmd.dest_handle);
            if (cmd.source_id == subI->target.first)
            {
                subI->updateData (cmd.actionTime, std::make_shared<const data_block> (std::move (cmd.payload)));
                updateValueTime (cmd);
            }
        }
        break;
        case CMD_ERROR:
            setState (HELICS_ERROR);
            return convergence_state::error;
        case CMD_REG_PUB:
        case CMD_NOTIFY_PUB:
        {
            auto subI = getSubscription (cmd.dest_handle);
            if (subI != nullptr)
            {
                subI->target = {cmd.source_id, cmd.source_handle};
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
        case CMD_REG_END:
        case CMD_NOTIFY_END:
        {
            auto filtI = getFilter (cmd.dest_handle);
            if (filtI != nullptr)
            {
                filtI->target = {cmd.source_id, cmd.source_handle};
                addDependency (cmd.source_id);
            }
        }
        break;
        case CMD_REG_DST_FILTER:
        case CMD_NOTIFY_DST_FILTER:
            break;
        case CMD_REG_SRC_FILTER:
        case CMD_NOTIFY_SRC_FILTER:
        {
            auto endI = getEndpoint (cmd.dest_handle);
            if (endI != nullptr)
            {
                endI->hasFilter = true;
                // TODO: this should be conditional on whether it is a operational filter
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
                if (cmd.error)
                {
                    setState (HELICS_ERROR);
                    return convergence_state::error;
                }
                global_id = cmd.dest_id;
                return convergence_state::complete;
            }
            break;
        }
        cmd = queue.pop ();
    }
}


// an info sink no one cares about
static DependencyInfo dummyInfo;

static auto dependencyCompare = [](const auto &dep, auto &target) { return (dep.fedID < target); };

bool FederateState::isDependency (Core::federate_id_t ofed) const
{
    auto res = std::lower_bound (dependencies.begin (), dependencies.end (), ofed, dependencyCompare);
    if (res == dependencies.end ())
    {
        return false;
    }
    return (res->fedID == ofed);
}

DependencyInfo *FederateState::getDependencyInfo (Core::federate_id_t ofed)
{
    auto res = std::lower_bound (dependencies.begin (), dependencies.end (), ofed, dependencyCompare);
    if ((res == dependencies.end ()) || (res->fedID != ofed))
    {
        return nullptr;
    }

    return &(*res);
}


bool FederateState::processExecRequest (ActionMessage &cmd)
{
    auto ofed = getDependencyInfo (cmd.source_id);
    if (ofed == nullptr)
    {
        return false;
    }

    switch (cmd.action ())
    {
    case CMD_EXEC_REQUEST:
        ofed->exec_requested = true;
        ofed->exec_converged = cmd.iterationComplete;
        break;
    case CMD_EXEC_GRANT:
        ofed->exec_requested = false;
        ofed->exec_converged = cmd.iterationComplete;
        break;
    }
    return true;
}

convergence_state FederateState::checkExecEntry ()
{
    if (iterating)
    {
        for (auto &dep : dependencies)
        {
            if (!dep.exec_requested)
            {
                return convergence_state::continue_processing;
            }
            if (!dep.exec_requested)
            {
                return convergence_state::continue_processing;
            }
        }
        if (time_value == timeZero)
        {
            if (iteration > info.max_iterations)
            {
                return convergence_state::complete;
            }
            return convergence_state::nonconverged;
        }
        return convergence_state::complete;  // todo add a check for updates and iteration limit
    }
    else
    {
        for (auto &dep : dependencies)
        {
            if (!dep.exec_requested)
            {
                return convergence_state::continue_processing;
            }
            if (!dep.exec_converged)
            {
                return convergence_state::continue_processing;
            }
        }
        return convergence_state::complete;
    }
}

bool FederateState::processExternalTimeMessage (ActionMessage &cmd)
{
    auto ofed = getDependencyInfo (cmd.source_id);
    if (ofed == nullptr)
    {
        return false;
    }

    switch (cmd.action ())
    {
    case CMD_TIME_REQUEST:
        ofed->grant = false;
        ofed->Tnext = cmd.actionTime;
        ofed->Te = cmd.info ().Te;
        ofed->Tdemin = cmd.info ().Tdemin;
        break;
    case CMD_TIME_GRANT:
        ofed->grant = true;
        ofed->Tnext = cmd.actionTime;
        ofed->Te = cmd.actionTime;
        ofed->Tdemin = cmd.actionTime;
        break;
    }
    return true;
}

void FederateState::updateNextExecutionTime ()
{
    time_exec = std::min (time_message, time_value) + info.impactWindow;
    time_exec = std::min (time_requested, time_exec);
    if (info.timeDelta > Time::epsilon ())
    {
        auto blk = static_cast<int> (std::ceil ((time_exec - time_granted) / info.timeDelta));
        time_exec = time_granted + blk * info.timeDelta;
    }
}


void FederateState::updateNextPossibleEventTime (convergence_state converged)
{
    if (converged == convergence_state::complete)
    {
        time_next = time_granted + info.timeDelta + info.lookAhead;
        time_next = std::max (time_next, time_minDe + info.impactWindow + info.lookAhead);
    }
    else
    {
        time_next = time_granted + info.lookAhead;
    }
}
void FederateState::updateValueTime (const ActionMessage &cmd)
{
    if (cmd.actionTime < time_value)
    {
        if (cmd.actionTime < time_granted)
        {
            // if this condition is true then the value update is ignored for timing purposes
            return;
        }
        time_value = cmd.actionTime;
    }
}

void FederateState::updateMessageTime (const ActionMessage &cmd)
{
    if (cmd.actionTime < time_message)
    {
        if (cmd.actionTime < time_granted)
        {
            // if this condition is true then the value update is ignored for timing purposes
            return;
        }
        time_message = cmd.actionTime;
    }
}

convergence_state FederateState::updateTimeFactors ()
{
    Time minNext = Time::maxVal ();
    Time minDe = Time::maxVal ();
    Time minTe = Time::maxVal ();
    for (auto &dep : dependencies)
    {
        if (dep.Tnext < minNext)
        {
            minNext = dep.Tnext;
        }
        if (dep.Tdemin < minDe)
        {
            minDe = dep.Tdemin;
        }
        if (dep.Te < minTe)
        {
            minTe = dep.Te;
        }
    }
    bool update = false;


    if (minNext > time_next)
    {
        update = true;
        time_next = minNext;
    }
    if (minDe > time_minDe)
    {
        update = true;
        time_minDe = minDe;
    }
    if (minTe > time_minTe)
    {
        update = true;
        time_minTe = minTe;
    }
    Time Tallow (info.impactWindow + minNext);
    if (time_exec <= Tallow)
    {
        return convergence_state::complete;  // we can grant the time request
    }

    return (update) ? convergence_state::nonconverged : convergence_state::continue_processing;
}

void FederateState::generateKnownDependencies ()
{
    std::vector<Core::federate_id_t> dependenciesList;
    std::vector<Core::federate_id_t> dependentList;
    // start with the subscriptions
    for (auto &sub : subscriptions)
    {
        if (sub->has_target)
        {
            dependenciesList.push_back (sub->target.first);
        }
    }
    // publications for dependent operations
    for (auto &pub : publications)
    {
        if (!pub->subscribers.empty ())
        {
            for (auto &pubTarget : pub->subscribers)
            {
                dependentList.push_back (pubTarget.first);
            }
        }
    }
    // filters for dependencies
    for (auto &filt : filters)
    {
        if (filt->filterOp == nullptr)
        {
            dependenciesList.push_back (filt->target.first);
        }
    }
    auto last1 = std::unique (dependenciesList.begin (), dependenciesList.end ());
    dependenciesList.erase (last1, dependenciesList.end ());
    auto last2 = std::unique (dependentList.begin (), dependentList.end ());
    dependenciesList.erase (last2, dependentList.end ());

    dependencies.resize (dependenciesList.size ());
    for (size_t ii = 0; ii < dependenciesList.size (); ++ii)
    {
        dependencies[ii].fedID = dependenciesList[ii];
    }

    dependents = dependentList;
}


void FederateState::addDependency (Core::federate_id_t fedID)
{
    if (dependencies.empty ())
    {
        dependencies.push_back (fedID);
    }
    auto dep = std::lower_bound (dependencies.begin (), dependencies.end (), fedID, dependencyCompare);
    if (dep == dependencies.end ())
    {
        dependencies.emplace_back (fedID);
    }
    else
    {
        if (dep->fedID == fedID)
        {
            // the dependency is already present
            return;
        }
        dependencies.emplace (dep, fedID);
    }
}

void FederateState::addDependent (Core::federate_id_t fedID)
{
    if (dependents.empty ())
    {
        dependents.push_back (fedID);
    }
    auto dep = std::lower_bound (dependents.begin (), dependents.end (), fedID);
    if (dep == dependents.end ())
    {
        dependents.push_back (fedID);
    }
    else
    {
        if (*dep == fedID)
        {
            return;
        }
        dependents.insert (dep, fedID);
    }
}


void FederateState::setCoreObject (CommonCore *parent)
{
    std::lock_guard<std::mutex> lock (_mutex);
    parent_ = parent;
}
}