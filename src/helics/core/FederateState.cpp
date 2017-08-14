/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "FederateState.h"

#include "EndpointInfo.h"
#include "FilterInfo.h"
#include "PublicationInfo.h"
#include "SubscriptionInfo.h"

#include <algorithm>
namespace helics
{
void FederateState::setState (helics_federate_state_type newState)
{
    if (newState == HELICS_INITIALIZING)
    {
        if (state != HELICS_CREATED)
        {
            return;
        }
    }
    state = newState;
}

helics_federate_state_type FederateState::getState () const { return state; }

static auto compareFunc = [](const auto &A, const auto &B) { return (A->id < B->id); };


void FederateState::createSubscription (Core::Handle handle,
                                        const std::string &key,
                                        const std::string &type,
                                        const std::string &units,
                                        bool required)
{
    auto sub = std::make_unique<SubscriptionInfo> (handle, global_id, key, type, units, required);

    std::lock_guard<std::mutex> lock (_mutex);
    subNames.emplace (key, sub.get ());

    // need to sort the vectors so the find works properly
    if (subscriptions.empty() || handle > subscriptions.back ()->id)
    {
        subscriptions.push_back (std::move (sub));
    }
    else
    {
        subscriptions.push_back (std::move (sub));
        std::sort (subscriptions.begin (), subscriptions.end (), compareFunc);
    }
}
void FederateState::createPublication (Core::Handle handle, const std::string &key, const std::string &type, const std::string &units)
{
    auto pub = std::make_unique<PublicationInfo> (handle, global_id, key, type, units);

    std::lock_guard<std::mutex> lock (_mutex);
    pubNames.emplace (key, pub.get ());

    // need to sort the vectors so the find works properly
    if (publications.empty() || handle > publications.back ()->id)
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
    if (endpoints.empty() || handle > endpoints.back ()->id)
    {
        endpoints.push_back (std::move (ep));
    }
    else
    {
        endpoints.push_back (std::move (ep));
        std::sort (endpoints.begin (), endpoints.end (), compareFunc);
    }
}
void FederateState::createFilter (Core::Handle handle,
                                  bool destFilter,
                                  const std::string &key,
                                  const std::string &target,
                                  const std::string &type)
{
    auto filt = std::make_unique<FilterInfo> (handle, global_id, key, target, type, destFilter);

    std::lock_guard<std::mutex> lock (_mutex);
    filterNames.emplace (key, filt.get ());

    if (filters.empty() || handle > filters.back ()->id)
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
        return epI->queueSize(time_granted);
    }
    auto fI = getFilter (handle_);
    if (fI != nullptr)
    {
        return fI->queueSize(time_granted);
    }
    return 0;
}



uint64_t FederateState::getQueueSize () const
{
    uint64_t cnt = 0;
    for (auto &end_point : endpoints)
    {
        cnt += end_point->queueSize(time_granted);
    }
    return cnt;
}

uint64_t FederateState::getFilterQueueSize() const
{
	uint64_t cnt = 0;
	for (auto &filt : filters)
	{
		cnt += filt->queueSize(time_granted);
	}
	return cnt;
}

std::unique_ptr<Message> FederateState::receive (Core::Handle handle_)
{
    auto epI = getEndpoint (handle_);
    if (epI != nullptr)
    {
		return epI->getMessage(time_granted);
    }
    auto fI = getFilter (handle_);
    if (fI != nullptr)
    {
		return fI->getMessage(time_granted);
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
		auto t = end_point->firstMessageTime();
		if (t < earliest_time)
		{
			t = earliest_time;
			endpointI = end_point.get();
		}
    }

    // Return the message found and remove from the queue
    if (earliest_time<=time_granted)
    {
        auto result = endpointI->getMessage(time_granted);
		id = endpointI->id;
		return result;
    }
	id = invalid_Handle;
    return nullptr;
}

std::unique_ptr<Message> FederateState::receiveForFilter(Core::Handle &id)
{
	Time earliest_time = Time::maxVal();
	FilterInfo *filterI = nullptr;
	// Find the end point with the earliest message time
	for (auto &filt : filters)
	{
		auto t = filt->firstMessageTime();
		if (t < earliest_time)
		{
			t = earliest_time;
			filterI = filt.get();
		}
	}

	// Return the message found and remove from the queue
	if (earliest_time <= time_granted)
	{
		auto result = filterI->getMessage(time_granted);
		id = filterI->id;
		return result;
	}
	id = invalid_Handle;
	return nullptr;
}


bool FederateState::processQueue ()
{
    auto cmd = queue.pop ();
    while (1)
    {
        switch (cmd.action())
        {
        case CMD_IGNORE:
        default:
            break;
        case CMD_INIT_GRANT:
            setState (HELICS_INITIALIZING);
            return true;
        case CMD_EXEC_REQUEST:
        case CMD_EXEC_GRANT:
		{
			auto grant = processExecRequest(cmd);
			if (grant == 1)
			{
				return false;
			}
			else if (grant == 2)
			{
				//TODO: send a time granted message
				//ActionMessage grant(CMD_EXEC_GRANT);
				//grant.source_id = global_id;
				
				setState(HELICS_EXECUTING);
				return true;
			}
		}
            break;
		case CMD_EXEC_CHECK:  //just check the time for entry
		{
			auto grant = checkExecEntry();
			if (grant == 1)
			{
				return false;
			}
			else if (grant == 2)
			{
				//TODO: send a time granted message
				setState(HELICS_EXECUTING);
				return true;
			}
		}
		break;
        case CMD_STOP:
        case CMD_DISCONNECT:
			if (cmd.dest_id == global_id)
			{
				setState(HELICS_FINISHED);
				return false;
			}
			break;
        case CMD_TIME_REQUEST:
        case CMD_TIME_GRANT:
		{
			auto update = processExternalTimeMessage(cmd);
			if (update == 1)
			{
				//TODO: send an update externally
			}
			else if (update == 2)
			{
				//TODO: send a time granted message
				return true;
			}
		}
            break;
		case CMD_TIME_CHECK:
		{
			auto update = updateTimeFactors();
			if (update == 1)
			{
				//TODO: send an update externally
			}
			else if (update == 2)
			{
				//TODO: send a time granted message
				return true;
			}
		}
		break;
        case CMD_SEND_MESSAGE:
        {
            auto epi = getEndpoint (cmd.dest_handle);
            epi->addMessage (createMessage (cmd));
        }
        break;
        case CMD_SEND_FOR_FILTER:
		{
			//this should only be non time_agnostic filters
			auto fI = getFilter(cmd.dest_handle);
			fI->addMessage(createMessage(cmd));
		}
            break;
        case CMD_PUB:
		{
			auto subI = getSubscription(cmd.dest_handle);
			if (cmd.source_id == subI->target.first)
			{
				subI->updateData(cmd.actionTime, std::make_shared<const data_block>(cmd.payload));
			}
		}
            break;
        case CMD_ERROR:
			setState(HELICS_ERROR);
			return false;
        case CMD_REG_PUB:
		{
			auto subI = getSubscription(cmd.dest_handle);
			subI->target = { cmd.source_id,cmd.source_handle };
		}
            break;
        case CMD_REG_SUB:
		{
			auto pubI = getPublication(cmd.dest_handle);
			pubI->subscribers.emplace_back(cmd.source_id, cmd.source_handle);
		}
            break;
        case CMD_REG_END:
            break;
        case CMD_REG_DST:
            break;
        case CMD_REG_SRC:
		{
			auto endI = getEndpoint(cmd.dest_handle);
			endI->hasFilter = true;
			//todo probably need to do something more here
		}
            break;
        case CMD_FED_ACK:
            if (cmd.name == name)
            {
                if (cmd.error)
                {
                    setState (HELICS_ERROR);
                    return false;
                }
                global_id = cmd.dest_id;
				return true;
            }
            break;
        }
        cmd = queue.pop ();
    }
}



// an info sink no one cares about
static DependencyInfo dummyInfo;

static auto dependencyCompare = [](const auto &dep, auto &target) { return (dep.fedID < target); };

DependencyInfo &FederateState::getDependencyInfo (Core::federate_id_t ofed)
{
    auto res = std::lower_bound (dependencies.begin (), dependencies.end (), ofed, dependencyCompare);
	return *res;
}


iterationTime  FederateState::requestTime(Time nextTime, convergence_state converged)
{
	iterating = (converged!=convergence_state::complete);
	time_requested = nextTime;
	time_event = time_requested; //TODO: this is not correct yet but will pass the next case
	//*push a message to check whether time can be granted
	queue.push(CMD_TIME_CHECK);
	bool ret = processQueue();
	return{ time_event,ret?convergence_state::complete:convergence_state::nonconverged };
}

int FederateState::processExecRequest(ActionMessage &cmd)
{
	auto &ofed = getDependencyInfo(cmd.source_id);
	if (ofed.fedID == cmd.source_id)
	{
		switch (cmd.action())
		{
		case CMD_EXEC_REQUEST:
			ofed.exec_requested = true;
			ofed.converged = cmd.iterationComplete;
			break;
		case CMD_EXEC_GRANT:
			ofed.exec_requested = false;
			ofed.converged = cmd.iterationComplete;
			break;
		}
		return checkExecEntry();
	}
	return 0;
}

int FederateState::checkExecEntry()
{
	if (iterating)
	{
		for (auto &dep : dependencies)
		{
			if (!dep.exec_requested)
			{
				return 0;
			}
			if (!dep.exec_requested)
			{
				return 0;
			}
		}
		return 1;  //todo add a check for updates and iteration limit
	}
	else
	{
		for (auto &dep : dependencies)
		{
			if (!dep.exec_requested)
			{
				return 0;
			}
			if (!dep.converged)
			{
				return 0;
			}
		}
		return 2;
	}
	
}

int FederateState::processExternalTimeMessage (ActionMessage &cmd)
{
    auto &ofed = getDependencyInfo (cmd.source_id);
    if (ofed.fedID == cmd.source_id)
    {
        switch (cmd.action())
        {
        case CMD_TIME_REQUEST:
            ofed.grant = false;
            ofed.Tnext = cmd.actionTime;
            ofed.Te = cmd.info ().Te;
            ofed.Tdemin = cmd.info ().Tdemin;
            break;
        case CMD_TIME_GRANT:
            ofed.grant = true;
            ofed.Tnext = cmd.actionTime;
            ofed.Te = cmd.actionTime;
            ofed.Tdemin = cmd.actionTime;
            break;
        }
        return updateTimeFactors ();
    }
	return 0;
}


int FederateState::updateTimeFactors () 
{
	Time minNext = Time::maxVal();
	Time minDe = Time::maxVal();
	Time minTe = Time::maxVal();
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
	Time Tallow(std::max(time_next, time_minDe));
	if (time_event <= Tallow)
	{
		return 2;  //we can grant the time request
	}
	return (update) ? 1 : 0;
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
    auto dep = std::lower_bound (dependencies.begin (), dependencies.end (), fedID, dependencyCompare);
    if (dep->fedID == fedID)
    {
        return;
    }
    else
    {
        dependencies.emplace (dep, fedID);
    }
}

void FederateState::addDependent (Core::federate_id_t fedID)
{
    auto dep = std::lower_bound (dependents.begin (), dependents.end (), fedID);
    if (*dep == fedID)
    {
        return;
    }
    else
    {
        dependents.insert (dep, fedID);
    }
}


void FederateState::setCoreObject (CommonCore *parent)
{
    std::lock_guard<std::mutex> lock (_mutex);
    parent_ = parent;
}
}