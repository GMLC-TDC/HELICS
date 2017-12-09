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
    if (fi == nullptr)
    {
        return nullptr;
    }
    auto *fed = new helics::FedObject;
    fed->index = getMasterHolder ()->addFed (fed);
    fed->fedptr = std::make_shared<helics::ValueFederate> (*reinterpret_cast<helics::FederateInfo *> (fi));
    fed->type = helics::vtype::valueFed;
    fed->valid = validationIdentifier;
    return reinterpret_cast<void *> (fed);
}

helics_federate helicsCreateValueFederateFromFile (const char *file)
{
    auto *fed = new helics::FedObject;
    fed->index = getMasterHolder ()->addFed (fed);
    fed->fedptr = std::make_shared<helics::ValueFederate> (file);
    fed->type = helics::vtype::valueFed;
    fed->valid = validationIdentifier;
    return reinterpret_cast<void *> (fed);
}

/* Creation and destruction of Federates */
helics_federate helicsCreateMessageFederate (const helics_federate_info_t fi)
{
    if (fi == nullptr)
    {
        return nullptr;
    }
    auto *fed = new helics::FedObject;
    fed->index = getMasterHolder ()->addFed (fed);
    fed->fedptr = std::make_shared<helics::MessageFederate> (*reinterpret_cast<helics::FederateInfo *> (fi));
    fed->type = helics::vtype::messageFed;
    fed->valid = validationIdentifier;
    return reinterpret_cast<void *> (fed);
}

helics_federate helicsCreateMessageFederateFromFile (const char *file)
{
    auto *fed = new helics::FedObject;
    fed->index = getMasterHolder ()->addFed (fed);
    fed->fedptr = std::make_shared<helics::MessageFederate> (file);
    fed->type = helics::vtype::messageFed;
    fed->valid = validationIdentifier;
    return reinterpret_cast<void *> (fed);
}

/* Creation and destruction of Federates */
helics_federate helicsCreateCombinationFederate (const helics_federate_info_t fi)
{
    auto *fed = new helics::FedObject;
    fed->index = getMasterHolder ()->addFed (fed);
    fed->fedptr = std::make_shared<helics::CombinationFederate> (*reinterpret_cast<helics::FederateInfo *> (fi));
    fed->type = helics::vtype::combinFed;
    fed->valid = validationIdentifier;
    return reinterpret_cast<void *> (fed);
}

helics_federate helicsCreateCombinationFederateFromFile (const char *file)
{
    auto *fed = new helics::FedObject;
    fed->index = getMasterHolder ()->addFed (fed);
    fed->fedptr = std::make_shared<helics::CombinationFederate> (file);
    fed->type = helics::vtype::combinFed;
    fed->valid = validationIdentifier;
    return reinterpret_cast<void *> (fed);
}

helicsStatus helicsFinalize (helics_federate fed)
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

int helicsAsyncOperationCompleted (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return 0;
    }
    return (fedObj->asyncOperationCompleted ()) ? 1 : 0;
}

helicsStatus helicsEnterInitializationModeFinalize (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helicsDiscard;
    }
    try
    {
        fedObj->enterInitializationStateFinalize ();
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
        return helicsDiscard;
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
        return error;
    case helics::iteration_result::halted:
        return halted;
    }
}

helicsStatus helicsEnterExecutionModeIterative (helics_federate fed, iteration_request iterate, iteration_status *outIterate)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helicsDiscard;
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
        return helicsDiscard;
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
        return helicsDiscard;
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

helicsStatus helicsEnterExecutionModeFinalize (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helicsDiscard;
    }
    try
    {
        fedObj->enterExecutionStateFinalize ();
        return helicsOK;
    }
    catch (helics::InvalidStateTransition &)
    {
        return helicsError;
    }
}
helicsStatus helicsEnterExecutionModeIterativeFinalize (helics_federate fed, iteration_status *outConverged)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helicsDiscard;
    }
    try
    {
        auto val = fedObj->enterExecutionStateFinalize ();
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
    itTime.status = error;
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
        return helicsDiscard;
    }
    fedObj->requestTimeAsync (requestTime);
    return helicsOK;
}

helicsStatus helicsRequestTimeIterativeAsync (helics_federate fed, helics_time_t requestTime, iteration_request iterate)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helicsDiscard;
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
helics_time_t helicsRequestTimeFinalize (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return (-1.0);
    }
    auto tm = fedObj->requestTimeFinalize ();
    return static_cast<double> (tm);
}
helics_iterative_time helicsRequestTimeIterativeFinalize (helics_federate fed)
{
    helics_iterative_time itTime;
    itTime.status = error;
    itTime.time = 0;

    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return itTime;
    }
    try
    {
        auto val = fedObj->requestTimeIterativeFinalize ();
        itTime.time = static_cast<double> (val.stepTime);
        itTime.status = getIterationStatus (val.state);
        return itTime;
    }
    catch (helics::InvalidStateTransition &)
    {
        return itTime;
    }
}
