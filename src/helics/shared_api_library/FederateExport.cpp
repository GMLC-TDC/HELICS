/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "../helics.hpp"
#include "helics.h"
#include "internal/api_objects.h"
#include <iostream>
#include <memory>
#include <mutex>
#include <vector>
#include <map>

/** this is a random identifier put in place when the federate gets created*/
static const int validationIdentifier = 0x2352188;

helics::Federate *getFed (helics_federate fed)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    if (fedObj->valid == validationIdentifier)
    {
        return fedObj->fedptr.get ();
    }
    return nullptr;
}

helics::ValueFederate *getValueFed (helics_value_federate fed)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    if (fedObj->valid == validationIdentifier)
    {
        if ((fedObj->type == helics::vtype::valueFed) || (fedObj->type == helics::vtype::combinFed))
        {
            return dynamic_cast<helics::ValueFederate *> (fedObj->fedptr.get ());
        }
    }
    return nullptr;
}

helics::MessageFederate *getMessageFed (helics_message_federate fed)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    if (fedObj->valid == validationIdentifier)
    {
        if ((fedObj->type == helics::vtype::messageFed) || (fedObj->type == helics::vtype::combinFed) ||
            (fedObj->type == helics::vtype::filterFed))
        {
            return dynamic_cast<helics::MessageFederate *> (fedObj->fedptr.get ());
        }
    }
    return nullptr;
}

std::shared_ptr<helics::Federate> getFedSharedPtr (helics_federate fed)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    if (fedObj->valid == validationIdentifier)
    {
        return fedObj->fedptr;
    }
    return nullptr;
}
std::shared_ptr<helics::ValueFederate> getValueFedSharedPtr (helics_value_federate fed)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    if (fedObj->valid == validationIdentifier)
    {
        if ((fedObj->type == helics::vtype::valueFed) || (fedObj->type == helics::vtype::combinFed))
        {
            return std::dynamic_pointer_cast<helics::ValueFederate> (fedObj->fedptr);
        }
    }
    return nullptr;
}
std::shared_ptr<helics::MessageFederate> getMessageFedSharedPtr (helics_message_federate fed)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    if (fedObj->valid == validationIdentifier)
    {
        if ((fedObj->type == helics::vtype::messageFed) || (fedObj->type == helics::vtype::combinFed) ||
            (fedObj->type == helics::vtype::filterFed))
        {
            return std::dynamic_pointer_cast<helics::MessageFederate> (fedObj->fedptr);
        }
    }
    return nullptr;
}

masterObjectHolder::masterObjectHolder () noexcept {}

masterObjectHolder::~masterObjectHolder ()
{
    deleteAll ();
    std::cout << "end of master Object Holder destructor" << std::endl;
}
int masterObjectHolder::addBroker (helics::BrokerObject *broker)
{
    std::lock_guard<std::mutex> lock (ObjectLock);
    auto index = static_cast<int> (brokers.size ());
    brokers.push_back (broker);
    return index;
}

int masterObjectHolder::addCore (helics::CoreObject *core)
{
    std::lock_guard<std::mutex> lock (ObjectLock);
    auto index = static_cast<int> (cores.size ());
    cores.push_back (core);
    return index;
}

int masterObjectHolder::addFed (helics::FedObject *fed)
{
    std::lock_guard<std::mutex> lock (ObjectLock);
    auto index = static_cast<int> (feds.size ());
    feds.push_back (fed);
    return index;
}

void masterObjectHolder::clearBroker (int index)
{
    std::lock_guard<std::mutex> lock (ObjectLock);
    if (index < static_cast<int> (brokers.size ()))
    {
        brokers[index] = nullptr;
    }
}

void masterObjectHolder::clearCore (int index)
{
    std::lock_guard<std::mutex> lock (ObjectLock);
    if (index < static_cast<int> (cores.size ()))
    {
        cores[index] = nullptr;
    }
}

void masterObjectHolder::clearFed (int index)
{
    std::lock_guard<std::mutex> lock (ObjectLock);
    if (index < static_cast<int> (feds.size ()))
    {
        feds[index] = nullptr;
    }
}

void masterObjectHolder::deleteAll ()
{
    std::lock_guard<std::mutex> lock (ObjectLock);
    for (auto obj : brokers)
    {
        delete obj;
    }
    for (auto obj : cores)
    {
        delete obj;
    }
    for (auto obj : feds)
    {
        delete obj;
    }
    brokers.clear ();
    feds.clear ();
    cores.clear ();
}

static masterObjectHolder mHolder;

masterObjectHolder *getMasterHolder () { return &mHolder; }

void clearAllObjects () { mHolder.deleteAll (); }

/* Creation and destruction of Federates */
helics_federate helicsCreateValueFederate (const helics_federate_info_t fi)
{
    auto *fed = new helics::FedObject;
    fed->index = getMasterHolder ()->addFed (fed);
	if (fi == nullptr)
	{
		fed->fedptr = std::make_shared<helics::ValueFederate>(helics::FederateInfo());
	}
	else
	{
		fed->fedptr = std::make_shared<helics::ValueFederate>(*reinterpret_cast<helics::FederateInfo *> (fi));
	}
    fed->type = helics::vtype::valueFed;
    fed->valid = validationIdentifier;
    return reinterpret_cast<void *> (fed);
}

helics_federate helicsCreateValueFederateFromJson (const char *json)
{
    auto *fed = new helics::FedObject;
    fed->index = getMasterHolder ()->addFed (fed);
    fed->fedptr = std::make_shared<helics::ValueFederate> ((json != nullptr) ? std::string(json) : std::string());
    fed->type = helics::vtype::valueFed;
    fed->valid = validationIdentifier;
    return reinterpret_cast<void *> (fed);
}

/* Creation and destruction of Federates */
helics_federate helicsCreateMessageFederate (const helics_federate_info_t fi)
{
    
    auto *fed = new helics::FedObject;
    fed->index = getMasterHolder ()->addFed (fed);
	if (fi == nullptr)
	{
		fed->fedptr = std::make_shared<helics::MessageFederate>(helics::FederateInfo());
	}
	else
	{
		fed->fedptr = std::make_shared<helics::MessageFederate>(*reinterpret_cast<helics::FederateInfo *> (fi));
	}
    fed->type = helics::vtype::messageFed;
    fed->valid = validationIdentifier;
    return reinterpret_cast<void *> (fed);
}

helics_federate helicsCreateMessageFederateFromJson (const char *json)
{
    auto *fed = new helics::FedObject;
    fed->index = getMasterHolder ()->addFed (fed);
    fed->fedptr = std::make_shared<helics::MessageFederate> ((json!=nullptr)?std::string(json):std::string());
    fed->type = helics::vtype::messageFed;
    fed->valid = validationIdentifier;
    return reinterpret_cast<void *> (fed);
}

/* Creation and destruction of Federates */
helics_federate helicsCreateCombinationFederate (const helics_federate_info_t fi)
{
    auto *fed = new helics::FedObject;
    fed->index = getMasterHolder ()->addFed (fed);
	if (fi == nullptr)
	{
		fed->fedptr = std::make_shared<helics::CombinationFederate>(helics::FederateInfo());
	}
	else
	{
		fed->fedptr = std::make_shared<helics::CombinationFederate>(*reinterpret_cast<helics::FederateInfo *> (fi));
	}
    fed->type = helics::vtype::combinFed;
    fed->valid = validationIdentifier;
    return reinterpret_cast<void *> (fed);
}

helics_federate helicsCreateCombinationFederateFromJson(const char *json)
{
    auto *fed = new helics::FedObject;
    fed->index = getMasterHolder ()->addFed (fed);
    fed->fedptr = std::make_shared<helics::CombinationFederate> ((json != nullptr) ? std::string(json) : std::string());
    fed->type = helics::vtype::combinFed;
    fed->valid = validationIdentifier;
    return reinterpret_cast<void *> (fed);
}


helics_core helicsGetCoreObject(helics_federate fed)
{
    auto fedObj = getFed(fed);
    if (fedObj == nullptr)
    {
        return nullptr;
    }
    auto *core = new helics::CoreObject;
    core->index = getMasterHolder()->addCore(core);
    core->coreptr = fedObj->getCorePointer();
    return reinterpret_cast<helics_core> (core);
}

helicsStatus helicsFederateFinalize (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helicsDiscard;
    }
    fedObj->finalize ();

    return helicsOK;
}

/* initialization, execution, and time requests */
helicsStatus helicsEnterInitializationMode (helics_federate fed)
{
    auto fedObj = getFed (fed);
    try
    {
        fedObj->enterInitializationState ();
        return helicsOK;
    }
    catch (helics::InvalidStateTransition &)
    {
        return helicsError;
    }
}

helicsStatus helicsEnterInitializationModeAsync (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helicsDiscard;
    }
    try
    {
        fedObj->enterInitializationStateAsync ();
        return helicsOK;
    }
    catch (helics::InvalidStateTransition &)
    {
        return helicsError;
    }
}

int helicsisAsyncOperationCompleted (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return 0;
    }
    return (fedObj->isAsyncOperationCompleted ()) ? 1 : 0;
}

helicsStatus helicsEnterInitializationModeComplete (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helicsDiscard;
    }
    try
    {
        fedObj->enterInitializationStateComplete ();
        return helicsOK;
    }
    catch (helics::InvalidStateTransition &)
    {
        return helicsError;
    }
}

helicsStatus helicsEnterExecutionMode (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helicsInvalidObject;
    }
    try
    {
        fedObj->enterExecutionState ();
        return helicsOK;
    }
    catch (helics::InvalidStateTransition &)
    {
        return helicsError;
    }
}

static helics::iteration_request getIterationRequest (iteration_request iterate)
{
    switch (iterate)
    {
    case no_iteration:
    default:
        return helics::iteration_request::no_iterations;
    case force_iteration:
        return helics::iteration_request::force_iteration;

    case iterate_if_needed:
        return helics::iteration_request::iterate_if_needed;
    }
}

static iteration_status getIterationStatus (helics::iteration_result iterationState)
{
    switch (iterationState)
    {
    case helics::iteration_result::next_step:
        return next_step;
    case helics::iteration_result::iterating:
        return iterating;
    case helics::iteration_result::error:
    default:
        return iteration_error;
    case helics::iteration_result::halted:
        return iteration_halted;
    }
}

helicsStatus helicsEnterExecutionModeIterative (helics_federate fed, iteration_request iterate, iteration_status *outIterate)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helicsInvalidObject;
    }
    try
    {
        auto val = fedObj->enterExecutionState (getIterationRequest (iterate));
        if (outIterate != nullptr)
        {
            *outIterate = getIterationStatus (val);
        }
        return helicsOK;
    }
    catch (helics::InvalidStateTransition &)
    {
        return helicsError;
    }
}

helicsStatus helicsEnterExecutionModeAsync (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helicsInvalidObject;
    }
    try
    {
        fedObj->enterExecutionStateAsync ();
        return helicsOK;
    }
    catch (helics::InvalidStateTransition &)
    {
        return helicsError;
    }
}

helicsStatus helicsEnterExecutionModeIterativeAsync (helics_federate fed, iteration_request iterate)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helicsInvalidObject;
    }
    try
    {
        fedObj->enterExecutionStateAsync (getIterationRequest (iterate));
        return helicsOK;
    }
    catch (helics::InvalidStateTransition &)
    {
        return helicsError;
    }
}

helicsStatus helicsEnterExecutionModeComplete (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helicsInvalidObject;
    }
    try
    {
        fedObj->enterExecutionStateComplete ();
        return helicsOK;
    }
    catch (helics::InvalidStateTransition &)
    {
        return helicsError;
    }
}
helicsStatus helicsEnterExecutionModeIterativeComplete(helics_federate fed, iteration_status *outConverged)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helicsInvalidObject;
    }
    try
    {
        auto val = fedObj->enterExecutionStateComplete ();
        if (outConverged != nullptr)
        {
            *outConverged = getIterationStatus (val);
        }
        return helicsOK;
    }
    catch (helics::InvalidStateTransition &)
    {
        return helicsError;
    }
}

helics_time_t helicsRequestTime (helics_federate fed, helics_time_t requestTime)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return (-1.0);
    }
    auto tm = fedObj->requestTime (requestTime);
    return static_cast<double> (tm);
}

helics_iterative_time helicsRequestTimeIterative (helics_federate fed, helics_time_t requestTime, iteration_request iterate)
{
    helics_iterative_time itTime;
    itTime.status = iteration_error;
    itTime.time = 0;

    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return itTime;
    }
    try
    {
        auto val = fedObj->requestTimeIterative (helics::Time (requestTime, timeUnits::ns), getIterationRequest (iterate));
        itTime.time = val.stepTime.getBaseTimeCode ();
        itTime.status = getIterationStatus (val.state);
        return itTime;
    }
    catch (helics::InvalidStateTransition &)
    {
        return itTime;
    }
}

helicsStatus helicsRequestTimeAsync (helics_federate fed, helics_time_t requestTime)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helicsInvalidObject;
    }
    fedObj->requestTimeAsync (requestTime);
    return helicsOK;
}

helicsStatus helicsRequestTimeIterativeAsync (helics_federate fed, helics_time_t requestTime, iteration_request iterate)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helicsInvalidObject;
    }
    try
    {
        fedObj->requestTimeIterative (requestTime, getIterationRequest (iterate));
        return helicsOK;
    }
    catch (helics::InvalidStateTransition &)
    {
        return helicsError;
    }
}
helics_time_t helicsRequestTimeComplete (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return (-1.0);
    }
    auto tm = fedObj->requestTimeComplete ();
    return static_cast<double> (tm);
}

static const std::map<helics::Federate::op_states, federate_state> stateEnumConversions
{
    {helics::Federate::op_states::error, federate_state::helics_error},
    { helics::Federate::op_states::startup, federate_state::helics_error },
    { helics::Federate::op_states::execution, federate_state::helics_error },
    { helics::Federate::op_states::finalize, federate_state::helics_error },
    { helics::Federate::op_states::pending_exec, federate_state::helics_error },
    { helics::Federate::op_states::pending_init, federate_state::helics_error },
    { helics::Federate::op_states::pending_iterative_time, federate_state::helics_pending_iterative_time },
    { helics::Federate::op_states::pending_time, federate_state::helics_pending_time },
    { helics::Federate::op_states::initialization,federate_state::helics_initialization}
};

helicsStatus helicsFederateGetState(helics_federate fed, federate_state *state)
{
    auto fedObj = getFed(fed);
    if ((fedObj == nullptr)||(state==nullptr))
    {
        return helicsInvalidObject;
    }
    auto FedState = fedObj->getCurrentState();
    *state = stateEnumConversions.at(FedState);
    return helicsOK;
}


helicsStatus helicsFederateGetName(helics_federate fed, char *str, int maxlen)
{
    auto fedObj = getFed(fed);
    if (fedObj == nullptr)
    {
        return helicsInvalidObject;
    }
    auto &ident = fedObj->getName();
    if (static_cast<int> (ident.size()) > maxlen)
    {
        strncpy(str, ident.c_str(), maxlen);
        str[maxlen - 1] = 0;
    }
    else
    {
        strcpy(str, ident.c_str());
    }
    return helicsOK;
}

 helicsStatus helicsFederateSetTimeDelta(helics_federate fed, helics_time_t time)
 {
     auto fedObj = getFed(fed);
     if (fedObj == nullptr)
     {
         return helicsInvalidObject;
     }
     fedObj->setTimeDelta(time);
     return helicsOK;
 }

helicsStatus helicsFederateSetLookAhead(helics_federate fed, helics_time_t lookAhead)
{
    auto fedObj = getFed(fed);
    if (fedObj == nullptr)
    {
        return helicsInvalidObject;
    }
    fedObj->setLookAhead(lookAhead);
    return helicsOK;
}

 helicsStatus helicsFederateSetImpactWindow(helics_federate fed, helics_time_t window)
 {
     auto fedObj = getFed(fed);
     if (fedObj == nullptr)
     {
         return helicsInvalidObject;
     }
     fedObj->setImpactWindow(window);
     return helicsOK;
 }
 helicsStatus helicsFederateSetPeriod(helics_federate fed, helics_time_t period, helics_time_t offset)
 {
     auto fedObj = getFed(fed);
     if (fedObj == nullptr)
     {
         return helicsInvalidObject;
     }
     fedObj->setPeriod(period, offset);
     return helicsOK;
 }
 helicsStatus helicsFederateSetFlag(helics_federate fed, int flag, int flagValue)
 {
     auto fedObj = getFed(fed);
     if (fedObj == nullptr)
     {
         return helicsInvalidObject;
     }
     fedObj->setFlag(flag, (flagValue != 0));
     return helicsOK;
 }

helicsStatus helicsFederateSetLoggingLevel(helics_federate fed, int loggingLevel)
{
    auto fedObj = getFed(fed);
    if (fedObj == nullptr)
    {
        return helicsInvalidObject;
    }
    fedObj->setLoggingLevel(loggingLevel);
    return helicsOK;
}

/** get the current time of the federate
@param fed the federate object to query
@param[out] time storage location for the time variable
@return helicsStatus object indicating success or error
*/
helicsStatus helicsFederateGetCurrentTime(helics_federate fed, helics_time_t *time)
{
    auto fedObj = getFed(fed);
    if (fedObj == nullptr)
    {
        return helicsInvalidObject;
    }
    *time = static_cast<double>(fedObj->getCurrentTime());
    return helicsOK;
}

helics_iterative_time helicsRequestTimeIterativeComplete (helics_federate fed)
{
    helics_iterative_time itTime;
    itTime.status = iteration_error;
    itTime.time = 0;

    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return itTime;
    }
    try
    {
        auto val = fedObj->requestTimeIterativeComplete ();
        itTime.time = static_cast<double> (val.stepTime);
        itTime.status = getIterationStatus (val.state);
        return itTime;
    }
    catch (helics::InvalidStateTransition &)
    {
        return itTime;
    }
}
