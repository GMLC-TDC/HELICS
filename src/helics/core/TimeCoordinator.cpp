/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "TimeCoordinator.h"

#include <algorithm>

namespace helics
{


TimeCoordinator::TimeCoordinator(const CoreFederateInfo &info_) :info(info_)
{
	if (info.timeDelta <= timeZero)
	{
		info.timeDelta = timeEpsilon;
	}
}

void TimeCoordinator::timeRequest(Time nextTime,Time newValueTime, Time newMessageTime)
{
	if (nextTime <= time_granted)
	{
		nextTime = time_granted + info.timeDelta;
	}
	time_requested = nextTime;
	
	time_value = newValueTime;
	time_message = newMessageTime;
	updateNextExecutionTime();
	updateNextPossibleEventTime();
	if (sendMessageFunction)
	{
		ActionMessage treq(CMD_TIME_REQUEST);
		treq.source_id = source_id;
		treq.actionTime = time_next;
		treq.info().Te = time_exec + info.lookAhead;
		treq.info().Tdemin = time_minDe;
		sendMessageFunction(treq);
	}
}

void TimeCoordinator::updateNextExecutionTime()
{
	time_exec = std::min(time_message, time_value) + info.impactWindow;
	time_exec = std::min(time_requested, time_exec);
	if (time_exec <= time_granted)
	{
		time_exec = (iterating) ? time_granted : (time_granted + info.timeDelta);
	}
	if (info.timeDelta > timeEpsilon)
	{
		auto blk = static_cast<int> (std::ceil((time_exec - time_granted) / info.timeDelta));
		time_exec = time_granted + blk * info.timeDelta;
	}
}


void TimeCoordinator::updateNextPossibleEventTime()
{
	if (!iterating)
	{
		time_next = time_granted + info.timeDelta + info.lookAhead;
		time_next = std::max(time_next, time_minDe + info.impactWindow + info.lookAhead);
	}
	else
	{
		time_next = time_granted + info.lookAhead;
	}
}
void TimeCoordinator::updateValueTime(Time valueUpdateTime)
{
	valueUpdateTime += info.impactWindow;
	if (valueUpdateTime < time_value)
	{
		if (valueUpdateTime <= time_granted)
		{
			if (iterating)
			{
				time_value = time_granted;
			}
			else
			{
				time_value = time_granted + info.timeDelta;
			}
		}
		else
		{
			time_value = valueUpdateTime;
		}
		
	}
}

void TimeCoordinator::updateMessageTime(Time messageUpdateTime)
{
	messageUpdateTime += info.impactWindow;
	if (messageUpdateTime < time_message)
	{
		if (messageUpdateTime <= time_granted)
		{
			if (iterating)
			{
				time_message = time_granted;
			}
			else
			{
				time_message = time_granted + info.timeDelta;
			}
		}
		else
		{
			time_message = messageUpdateTime;
		}
	}
}

bool TimeCoordinator::updateTimeFactors()
{
	Time minNext = Time::maxVal();
	Time minminDe = Time::maxVal();
	Time minDe = Time::maxVal();
	for (auto &dep : dependencies)
	{
		if (dep.Tnext < minNext)
		{
			minNext = dep.Tnext;
		}
		if (dep.Tdemin < minDe)
		{
			minminDe = dep.Tdemin;
		}
		if (dep.Te < minDe)
		{
			minDe = dep.Te;
		}
	}
	bool update = false;

	Time calc_next;
	if (!iterating)
	{
		calc_next = std::max(time_granted + info.timeDelta, info.impactWindow + minminDe) + info.lookAhead;
	}
	else
	{
		calc_next = time_granted + info.lookAhead;
	}

	if (calc_next > time_next)
	{
		update = true;
		time_next = calc_next;
	}
	time_minminDe = minminDe;
	if (minDe != time_minDe)
	{
		update = true;
		time_minDe = minDe;
	}
	time_allow = info.impactWindow + minNext;
	updateNextExecutionTime();
	return update;
}


convergence_state TimeCoordinator::checkTimeGrant()
{
	bool update = updateTimeFactors();
	if (!iterating)
	{
		if (time_allow >= time_exec)
		{
			time_granted = time_exec;
			return convergence_state::complete;
		}
	}
	else
	{
		if (time_allow > time_exec)
		{
			time_granted = time_exec;
			return convergence_state::complete;
		}
		else
		{
			// TODO:: something with the iteration conditions
		}
	}

	// if we haven't returned we need to update the time messages
	if ((update) && (sendMessageFunction))
	{
		ActionMessage upd(CMD_TIME_REQUEST);
		upd.source_id = source_id;
		upd.actionTime = time_next;
		upd.info().Te = time_exec;
		upd.info().Tdemin = time_minDe;
		sendMessageFunction(upd);
	}
	return convergence_state::continue_processing;
}

// an info sink no one cares about
static DependencyInfo dummyInfo;

static auto dependencyCompare = [](const auto &dep, auto &target) { return (dep.fedID < target); };

bool TimeCoordinator::isDependency(Core::federate_id_t ofed) const
{
	auto res = std::lower_bound(dependencies.begin(), dependencies.end(), ofed, dependencyCompare);
	if (res == dependencies.end())
	{
		return false;
	}
	return (res->fedID == ofed);
}

DependencyInfo *TimeCoordinator::getDependencyInfo(Core::federate_id_t ofed)
{
	auto res = std::lower_bound(dependencies.begin(), dependencies.end(), ofed, dependencyCompare);
	if ((res == dependencies.end()) || (res->fedID != ofed))
	{
		return nullptr;
	}

	return &(*res);
}

void TimeCoordinator::addDependency(Core::federate_id_t fedID)
{
	if (dependencies.empty())
	{
		dependencies.push_back(fedID);
	}
	auto dep = std::lower_bound(dependencies.begin(), dependencies.end(), fedID, dependencyCompare);
	if (dep == dependencies.end())
	{
		dependencies.emplace_back(fedID);
	}
	else
	{
		if (dep->fedID == fedID)
		{
			// the dependency is already present
			return;
		}
		dependencies.emplace(dep, fedID);
	}
}

void TimeCoordinator::addDependent(Core::federate_id_t fedID)
{
	if (dependents.empty())
	{
		dependents.push_back(fedID);
	}
	auto dep = std::lower_bound(dependents.begin(), dependents.end(), fedID);
	if (dep == dependents.end())
	{
		dependents.push_back(fedID);
	}
	else
	{
		if (*dep == fedID)
		{
			return;
		}
		dependents.insert(dep, fedID);
	}
}



convergence_state TimeCoordinator::checkExecEntry()
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
				time_granted = timeZero;
				return convergence_state::complete;
			}
			return convergence_state::nonconverged;
		}
		time_granted = timeZero;
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
		time_granted = timeZero;
		return convergence_state::complete;
	}
}


bool TimeCoordinator::processExecRequest(ActionMessage &cmd)
{
	auto ofed = getDependencyInfo(cmd.source_id);
	if (ofed == nullptr)
	{
		return false;
	}

	switch (cmd.action())
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

bool TimeCoordinator::processExternalTimeMessage(ActionMessage &cmd)
{
	auto ofed = getDependencyInfo(cmd.source_id);
	if (ofed == nullptr)
	{
		return false;
	}

	switch (cmd.action())
	{
	case CMD_TIME_REQUEST:
		ofed->grant = false;
		ofed->Tnext = cmd.actionTime;
		ofed->Te = cmd.info().Te;
		ofed->Tdemin = cmd.info().Tdemin;
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
} //namespace helics