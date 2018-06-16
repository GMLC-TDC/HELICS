/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "../common/TripWire.hpp"
#include "../core/core-exceptions.hpp"
#include "../helics.hpp"
#include "helics.h"
#include "internal/api_objects.h"
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

/** this is a random identifier put in place when the federate or core or broker gets created*/
static const int fedValidationIdentifier = 0x2352188;

helics::Federate *getFed (helics_federate fed)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    if (fedObj->valid == fedValidationIdentifier)
    {
        return fedObj->fedptr.get ();
    }
    return nullptr;
}

helics::ValueFederate *getValueFed (helics_federate fed)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    if (fedObj->valid == fedValidationIdentifier)
    {
        if ((fedObj->type == helics::vtype::valueFed) || (fedObj->type == helics::vtype::combinationFed))
        {
            return dynamic_cast<helics::ValueFederate *> (fedObj->fedptr.get ());
        }
    }
    return nullptr;
}

helics::MessageFederate *getMessageFed (helics_federate fed)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    if (fedObj->valid == fedValidationIdentifier)
    {
        if ((fedObj->type == helics::vtype::messageFed) || (fedObj->type == helics::vtype::combinationFed))
        {
            return dynamic_cast<helics::MessageFederate *> (fedObj->fedptr.get ());
        }
    }
    return nullptr;
}

std::shared_ptr<helics::Federate> getFedSharedPtr (helics_federate fed)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    if (fedObj->valid == fedValidationIdentifier)
    {
        return fedObj->fedptr;
    }
    return nullptr;
}

std::shared_ptr<helics::ValueFederate> getValueFedSharedPtr (helics_federate fed)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    if (fedObj->valid == fedValidationIdentifier)
    {
        if ((fedObj->type == helics::vtype::valueFed) || (fedObj->type == helics::vtype::combinationFed))
        {
            return std::dynamic_pointer_cast<helics::ValueFederate> (fedObj->fedptr);
        }
    }
    return nullptr;
}

std::shared_ptr<helics::MessageFederate> getMessageFedSharedPtr (helics_federate fed)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    if (fedObj->valid == fedValidationIdentifier)
    {
        if ((fedObj->type == helics::vtype::messageFed) || (fedObj->type == helics::vtype::combinationFed))
        {
            return std::dynamic_pointer_cast<helics::MessageFederate> (fedObj->fedptr);
        }
    }
    return nullptr;
}

/* Creation and destruction of Federates */
helics_federate helicsCreateValueFederate (const helics_federate_info_t fi)
{
    auto FedI = std::make_unique<helics::FedObject> ();
    try
    {
        if (fi == nullptr)
        {
            FedI->fedptr = std::make_shared<helics::ValueFederate> (helics::FederateInfo ());
        }
        else
        {
            FedI->fedptr = std::make_shared<helics::ValueFederate> (*reinterpret_cast<helics::FederateInfo *> (fi));
        }
    }
    catch (const helics::RegistrationFailure &)
    {
        return nullptr;
    }
    FedI->type = helics::vtype::valueFed;
    FedI->valid = fedValidationIdentifier;
    auto fed = reinterpret_cast<helics_federate> (FedI.get ());
    getMasterHolder ()->addFed (std::move (FedI));
    return (fed);
}

helics_federate helicsCreateValueFederateFromJson (const char *json)
{
    auto FedI = std::make_unique<helics::FedObject> ();
    try
    {
        FedI->fedptr = std::make_shared<helics::ValueFederate> ((json != nullptr) ? std::string (json) : std::string ());
    }
    catch (const helics::RegistrationFailure &)
    {
        return nullptr;
    }
    FedI->type = helics::vtype::valueFed;
    FedI->valid = fedValidationIdentifier;
    auto fed = reinterpret_cast<helics_federate> (FedI.get ());
    getMasterHolder ()->addFed (std::move (FedI));
    return (fed);
}

/* Creation and destruction of Federates */
helics_federate helicsCreateMessageFederate (const helics_federate_info_t fi)
{
    auto FedI = std::make_unique<helics::FedObject> ();
    try
    {
        if (fi == nullptr)
        {
            FedI->fedptr = std::make_shared<helics::MessageFederate> (helics::FederateInfo ());
        }
        else
        {
            FedI->fedptr = std::make_shared<helics::MessageFederate> (*reinterpret_cast<helics::FederateInfo *> (fi));
        }
    }
    catch (const helics::RegistrationFailure &)
    {
        return nullptr;
    }
    FedI->type = helics::vtype::messageFed;
    FedI->valid = fedValidationIdentifier;
    auto fed = reinterpret_cast<helics_federate> (FedI.get ());
    getMasterHolder ()->addFed (std::move (FedI));
    return (fed);
}

helics_federate helicsCreateMessageFederateFromJson (const char *json)
{
    auto FedI = std::make_unique<helics::FedObject> ();

    try
    {
        FedI->fedptr = std::make_shared<helics::MessageFederate> ((json != nullptr) ? std::string (json) : std::string ());
    }
    catch (const helics::RegistrationFailure &)
    {
        return nullptr;
    }
    FedI->type = helics::vtype::messageFed;
    FedI->valid = fedValidationIdentifier;
    auto fed = reinterpret_cast<helics_federate> (FedI.get ());
    getMasterHolder ()->addFed (std::move (FedI));
    return (fed);
}

/* Creation and destruction of Federates */
helics_federate helicsCreateCombinationFederate (const helics_federate_info_t fi)
{
    auto FedI = std::make_unique<helics::FedObject> ();
    try
    {
        if (fi == nullptr)
        {
            FedI->fedptr = std::make_shared<helics::CombinationFederate> (helics::FederateInfo ());
        }
        else
        {
            FedI->fedptr = std::make_shared<helics::CombinationFederate> (*reinterpret_cast<helics::FederateInfo *> (fi));
        }
    }
    catch (const helics::RegistrationFailure &)
    {
        return nullptr;
    }
    FedI->type = helics::vtype::combinationFed;
    FedI->valid = fedValidationIdentifier;
    auto fed = reinterpret_cast<helics_federate> (FedI.get ());
    getMasterHolder ()->addFed (std::move (FedI));
    return (fed);
}

helics_federate helicsCreateCombinationFederateFromJson (const char *json)
{
    auto FedI = std::make_unique<helics::FedObject> ();
    try
    {
        FedI->fedptr = std::make_shared<helics::CombinationFederate> ((json != nullptr) ? std::string (json) : std::string ());
    }
    catch (const helics::RegistrationFailure &)
    {
        return nullptr;
    }

    FedI->type = helics::vtype::combinationFed;
    FedI->valid = fedValidationIdentifier;
    auto fed = reinterpret_cast<helics_federate> (FedI.get ());
    getMasterHolder ()->addFed (std::move (FedI));
    return (fed);
}

helics_federate helicsFederateClone (helics_federate fed)
{
    if (fed == nullptr)
    {
        return nullptr;
    }
    auto *fedObj = reinterpret_cast<helics::FedObject *> (fed);
    auto fedClone = std::make_unique<helics::FedObject> ();
    fedClone->fedptr = fedObj->fedptr;

    fedClone->type = fedObj->type;
    fedClone->valid = fedObj->valid;
    auto fedB = reinterpret_cast<helics_federate> (fedClone.get ());
    getMasterHolder ()->addFed (std::move (fedClone));
    return (fedB);
}

helics_core helicsFederateGetCoreObject (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return nullptr;
    }
    auto core = std::make_unique<helics::CoreObject> ();
    core->valid = coreValidationIdentifier;
    core->coreptr = fedObj->getCorePointer ();
    auto retcore = reinterpret_cast<helics_core> (core.get ());
    getMasterHolder ()->addCore (std::move (core));
    return retcore;
}

helics_status helicsFederateFinalize (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        fedObj->finalize ();
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

/* initialization, execution, and time requests */
helics_status helicsFederateEnterInitializationMode (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        fedObj->enterInitializationState ();
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFederateEnterInitializationModeAsync (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        fedObj->enterInitializationStateAsync ();
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

int helicsFederateIsAsyncOperationCompleted (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return 0;
    }
    return (fedObj->isAsyncOperationCompleted ()) ? 1 : 0;
}

helics_status helicsFederateEnterInitializationModeComplete (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        fedObj->enterInitializationStateComplete ();
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFederateEnterExecutionMode (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        // printf("current state=%d\n", static_cast<int>(fedObj->getCurrentState()));
        fedObj->enterExecutionState ();
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

static helics::iteration_request getIterationRequest (helics_iteration_request iterate)
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

static helics_iteration_status getIterationStatus (helics::iteration_result iterationState)
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

helics_status
helicsFederateEnterExecutionModeIterative (helics_federate fed, helics_iteration_request iterate, helics_iteration_status *outIterate)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        auto val = fedObj->enterExecutionState (getIterationRequest (iterate));
        if (outIterate != nullptr)
        {
            *outIterate = getIterationStatus (val);
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFederateEnterExecutionModeAsync (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        fedObj->enterExecutionStateAsync ();
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFederateEnterExecutionModeIterativeAsync (helics_federate fed, helics_iteration_request iterate)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        fedObj->enterExecutionStateAsync (getIterationRequest (iterate));
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFederateEnterExecutionModeComplete (helics_federate fed)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        fedObj->enterExecutionStateComplete ();
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}
helics_status helicsFederateEnterExecutionModeIterativeComplete (helics_federate fed, helics_iteration_status *outConverged)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        auto val = fedObj->enterExecutionStateComplete ();
        if (outConverged != nullptr)
        {
            *outConverged = getIterationStatus (val);
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFederateRequestTime (helics_federate fed, helics_time_t requestTime, helics_time_t *timeOut)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        *timeOut = static_cast<double> (fedObj->requestTime (requestTime));
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFederateRequestTimeIterative (helics_federate fed,
                                                  helics_time_t requestTime,
                                                  helics_iteration_request iterate,
                                                  helics_time_t *timeOut,
                                                  helics_iteration_status *outIteration)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        auto val = fedObj->requestTimeIterative (requestTime, getIterationRequest (iterate));
        if (val.state == helics::iteration_result::error)
        {
            return helics_error;
        }
        *outIteration = getIterationStatus (val.state);
        *timeOut = static_cast<double> (val.grantedTime);
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFederateRequestTimeAsync (helics_federate fed, helics_time_t requestTime)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        fedObj->requestTimeAsync (requestTime);
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFederateRequestTimeIterativeAsync (helics_federate fed, helics_time_t requestTime, helics_iteration_request iterate)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        fedObj->requestTimeIterative (requestTime, getIterationRequest (iterate));
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}
helics_status helicsFederateRequestTimeComplete (helics_federate fed, helics_time_t *timeOut)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        *timeOut = fedObj->requestTimeComplete ();
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

static const std::map<helics::Federate::op_states, federate_state> stateEnumConversions{
  {helics::Federate::op_states::error, federate_state::helics_error_state},
  {helics::Federate::op_states::startup, federate_state::helics_startup_state},
  {helics::Federate::op_states::execution, federate_state::helics_execution_state},
  {helics::Federate::op_states::finalize, federate_state::helics_finalize_state},
  {helics::Federate::op_states::pending_exec, federate_state::helics_pending_exec_state},
  {helics::Federate::op_states::pending_init, federate_state::helics_pending_init_state},
  {helics::Federate::op_states::pending_iterative_time, federate_state::helics_pending_iterative_time_state},
  {helics::Federate::op_states::pending_time, federate_state::helics_pending_time_state},
  {helics::Federate::op_states::initialization, federate_state::helics_initialization_state}};

helics_status helicsFederateGetState (helics_federate fed, federate_state *state)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    if (state == nullptr)
    {
        return helics_discard;
    }
    try
    {
        auto FedState = fedObj->getCurrentState ();
        *state = stateEnumConversions.at (FedState);
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFederateGetName (helics_federate fed, char *outputString, int maxlen)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    if (outputString == nullptr)
    {
        return helics_discard;
    }
    try
    {
        auto &ident = fedObj->getName ();
        if (static_cast<int> (ident.size ()) > maxlen)
        {
            strncpy (outputString, ident.c_str (), maxlen);
            outputString[maxlen - 1] = 0;
        }
        else
        {
            strcpy (outputString, ident.c_str ());
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFederateSetTimeDelta (helics_federate fed, helics_time_t time)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        fedObj->setTimeDelta (time);
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFederateSetOutputDelay (helics_federate fed, helics_time_t outputDelay)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        fedObj->setOutputDelay (outputDelay);
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFederateSetInputDelay (helics_federate fed, helics_time_t inputDelay)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        fedObj->setInputDelay (inputDelay);
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}
helics_status helicsFederateSetPeriod (helics_federate fed, helics_time_t period, helics_time_t offset)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        fedObj->setPeriod (period, offset);
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}
helics_status helicsFederateSetFlag (helics_federate fed, int flag, helics_bool_t flagValue)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        fedObj->setFlag (flag, (flagValue != helics_false));
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFederateSetLoggingLevel (helics_federate fed, int loggingLevel)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        fedObj->setLoggingLevel (loggingLevel);
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFederateSetMaxIterations (helics_federate fed, int maxIterations)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        fedObj->setMaxIterations (maxIterations);
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFederateSetSeparator(helics_federate fed, char separator)
{
    auto fedObj = getFed(fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    fedObj->setSeparator(separator);
    return helics_ok;
}


helics_status helicsFederateGetCurrentTime (helics_federate fed, helics_time_t *time)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        *time = static_cast<double> (fedObj->getCurrentTime ());
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status
helicsFederateRequestTimeIterativeComplete (helics_federate fed, helics_time_t *timeOut, helics_iteration_status *outIteration)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        auto val = fedObj->requestTimeIterativeComplete ();
        if (val.state == helics::iteration_result::error)
        {
            return helics_error;
        }
        *outIteration = getIterationStatus (val.state);
        *timeOut = static_cast<double> (val.grantedTime);
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}
