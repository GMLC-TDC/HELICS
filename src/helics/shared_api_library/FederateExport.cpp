/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "application_api/application_api.h"
#include "core/helics-time.h"
#include "helics.h"
#include "internal/api_objects.h"
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

helics::MessageFilterFederate *getFilterFed (helics_message_filter_federate fed)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    if (fedObj->valid == validationIdentifier)
    {
        if (fedObj->type == helics::vtype::filterFed)
        {
            return dynamic_cast<helics::MessageFilterFederate *> (fedObj->fedptr.get ());
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
std::shared_ptr<helics::MessageFilterFederate> getFilterFedSharedPtr (helics_message_filter_federate fed)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    if (fedObj->valid == validationIdentifier)
    {
        if (fedObj->type == helics::vtype::filterFed)
        {
            return std::dynamic_pointer_cast<helics::MessageFilterFederate> (fedObj->fedptr);
        }
    }
    return nullptr;
}

/* Creation and destruction of Federates */
helics_federate helicsCreateValueFederate (const helics_federate_info_t fi)
{
    if (fi == nullptr)
    {
        return nullptr;
    }
    auto *fed = new helics::FedObject;
    fed->fedptr = std::make_shared<helics::ValueFederate> (*reinterpret_cast<helics::FederateInfo *> (fi));
    fed->type = helics::vtype::valueFed;
    fed->valid = validationIdentifier;
    return reinterpret_cast<void *> (fed);
}

helics_federate helicsCreateValueFederateFromFile (const char *file)
{
    auto *fed = new helics::FedObject;
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
    fed->fedptr = std::make_shared<helics::MessageFederate> (*reinterpret_cast<helics::FederateInfo *> (fi));
    fed->type = helics::vtype::messageFed;
    fed->valid = validationIdentifier;
    return reinterpret_cast<void *> (fed);
}

helics_federate helicsCreateMessageFederateFromFile (const char *file)
{
    auto *fed = new helics::FedObject;
    fed->fedptr = std::make_shared<helics::MessageFederate> (file);
    fed->type = helics::vtype::messageFed;
    fed->valid = validationIdentifier;
    return reinterpret_cast<void *> (fed);
}

/* Creation and destruction of Federates */
helics_federate helicsCreateMessageFilterFederate (const helics_federate_info_t fi)
{
    auto *fed = new helics::FedObject;
    fed->fedptr = std::make_shared<helics::MessageFilterFederate> (*reinterpret_cast<helics::FederateInfo *> (fi));
    fed->type = helics::vtype::filterFed;
    fed->valid = validationIdentifier;
    return reinterpret_cast<void *> (fed);
}

helics_federate helicsCreateMessageFilterFederateFromFile (const char *file)
{
    auto *fed = new helics::FedObject;
    fed->fedptr = std::make_shared<helics::MessageFilterFederate> (file);
    fed->type = helics::vtype::filterFed;
    fed->valid = validationIdentifier;
    return reinterpret_cast<void *> (fed);
}

/* Creation and destruction of Federates */
helics_federate helicsCreateCombinationFederate (const helics_federate_info_t fi)
{
    auto *fed = new helics::FedObject;
    fed->fedptr = std::make_shared<helics::CombinationFederate> (*reinterpret_cast<helics::FederateInfo *> (fi));
    fed->type = helics::vtype::combinFed;
    fed->valid = validationIdentifier;
    return reinterpret_cast<void *> (fed);
}

helics_federate helicsCreateCombinationFederateFromFile (const char *file)
{
    auto *fed = new helics::FedObject;
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

static helics::convergence_state getConvergenceState (convergence_status conv)
{
    switch (conv)
    {
    case converged:
        return helics::convergence_state::complete;
    case nonconverged:
        return helics::convergence_state::nonconverged;
    case error:
    default:
        return helics::convergence_state::error;
    case halted:
        return helics::convergence_state::halted;
    }
}

static convergence_status getConvergenceStatus (helics::convergence_state convState)
{
    switch (convState)
    {
    case helics::convergence_state::complete:
        return converged;
    case helics::convergence_state::nonconverged:
    case helics::convergence_state::continue_processing:
        return nonconverged;
    case helics::convergence_state::error:
    default:
        return error;
    case helics::convergence_state::halted:
        return halted;
    }
}

helicsStatus helicsEnterExecutionModeIterative (helics_federate fed,
                                                convergence_status converged,
                                                convergence_status *outConverged)
{
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return helicsDiscard;
    }
    try
    {
        auto val = fedObj->enterExecutionState (getConvergenceState (converged));
        if (outConverged != nullptr)
        {
            *outConverged = getConvergenceStatus (val);
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
        return helicsDiscard;
    }
    auto tm = fedObj->requestTime (helics::Time (requestTime, timeUnits::ns));
    return tm.getBaseTimeCode ();
}

helics_iterative_time
helicsRequestTimeIterative (helics_federate fed, helics_time_t requestTime, convergence_status converged)
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
        auto val = fedObj->requestTimeIterative (helics::Time (requestTime, timeUnits::ns),
                                                 getConvergenceState (converged));
        itTime.time = val.stepTime.getBaseTimeCode ();
        itTime.status = getConvergenceStatus (val.state);
        return itTime;
    }
    catch (helics::InvalidStateTransition &)
    {
        return itTime;
    }
}

void helics_free_federate (helics_federate fed) { delete reinterpret_cast<helics::FedObject *> (fed); }
