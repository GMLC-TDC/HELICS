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
#include <mutex>
#include <vector>

/** this is a random identifier put in place when the federate or core or broker gets created*/
static const int fedValidationIdentifier = 0x2352188;
static const char *invalidFedString = "federate object is not valid";

static const std::string nullstr;
static constexpr char nullcstr[] = "";

namespace helics
{
FedObject *getFedObject (helics_federate fed, helics_error *err)
{
    HELICS_ERROR_CHECK (err, nullptr);
    if (fed == nullptr)
    {
        if (err != nullptr)
        {
            err->error_code = helics_error_invalid_object;
            err->message = invalidFedString;
        }
        return nullptr;
    }
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    if (fedObj->valid == fedValidationIdentifier)
    {
        return fedObj;
    }
    if (err != nullptr)
    {
        err->error_code = helics_error_invalid_object;
        err->message = invalidFedString;
    }
    return nullptr;
}
}  // namespace helics

helics::Federate *getFed (helics_federate fed, helics_error *err)
{
    auto fedObj = helics::getFedObject (fed, err);
    return (fedObj == nullptr) ? nullptr : fedObj->fedptr.get ();
}

static const char *notValueFedString = "Federate must be a value federate";

helics::ValueFederate *getValueFed (helics_federate fed, helics_error *err)
{
    auto fedObj = helics::getFedObject (fed, err);
    if (fedObj == nullptr)
    {
        return nullptr;
    }
    if ((fedObj->type == helics::vtype::value_fed) || (fedObj->type == helics::vtype::combination_fed))
    {
        auto rval = dynamic_cast<helics::ValueFederate *> (fedObj->fedptr.get ());
        if (rval != nullptr)
        {
            return rval;
        }
    }
    if (err != nullptr)
    {
        err->error_code = helics_error_invalid_object;
        err->message = notValueFedString;
    }
    return nullptr;
}

static const char *notMessageFedString = "Federate must be a message federate";

helics::MessageFederate *getMessageFed (helics_federate fed, helics_error *err)
{
    auto fedObj = helics::getFedObject (fed, err);
    if (fedObj == nullptr)
    {
        return nullptr;
    }
    if ((fedObj->type == helics::vtype::message_fed) || (fedObj->type == helics::vtype::combination_fed))
    {
        auto rval = dynamic_cast<helics::MessageFederate *> (fedObj->fedptr.get ());
        if (rval != nullptr)
        {
            return rval;
        }
    }
    if (err != nullptr)
    {
        err->error_code = helics_error_invalid_object;
        err->message = notMessageFedString;
    }
    return nullptr;
}

std::shared_ptr<helics::Federate> getFedSharedPtr (helics_federate fed, helics_error *err)
{
    auto fedObj = helics::getFedObject (fed, err);
    if (fedObj == nullptr)
    {
        return nullptr;
    }
    return fedObj->fedptr;
}

std::shared_ptr<helics::ValueFederate> getValueFedSharedPtr (helics_federate fed, helics_error *err)
{
    auto fedObj = helics::getFedObject (fed, err);
    if (fedObj == nullptr)
    {
        return nullptr;
    }
    if ((fedObj->type == helics::vtype::value_fed) || (fedObj->type == helics::vtype::combination_fed))
    {
        auto rval = std::dynamic_pointer_cast<helics::ValueFederate> (fedObj->fedptr);
        if (rval)
        {
            return rval;
        }
    }
    if (err != nullptr)
    {
        err->error_code = helics_error_invalid_object;
        err->message = notValueFedString;
    }
    return nullptr;
}

std::shared_ptr<helics::MessageFederate> getMessageFedSharedPtr (helics_federate fed, helics_error *err)
{
    auto fedObj = helics::getFedObject (fed, err);
    if (fedObj == nullptr)
    {
        return nullptr;
    }
    if ((fedObj->type == helics::vtype::message_fed) || (fedObj->type == helics::vtype::combination_fed))
    {
        auto rval = std::dynamic_pointer_cast<helics::MessageFederate> (fedObj->fedptr);
        if (rval)
        {
            return rval;
        }
    }
    if (err != nullptr)
    {
        err->error_code = helics_error_invalid_object;
        err->message = notMessageFedString;
    }
    return nullptr;
}

/* Creation and destruction of Federates */
helics_federate helicsCreateValueFederate (const char *fedName, const helics_federate_info fi, helics_error *err)
{
    HELICS_ERROR_CHECK (err, nullptr);
    auto FedI = std::make_unique<helics::FedObject> ();
    try
    {
        if (fi == nullptr)
        {
            FedI->fedptr =
              std::make_shared<helics::ValueFederate> ((fedName != nullptr) ? std::string (fedName) : nullstr, helics::FederateInfo ());
        }
        else
        {
            FedI->fedptr = std::make_shared<helics::ValueFederate> ((fedName != nullptr) ? std::string (fedName) : nullstr,
                                                                    *reinterpret_cast<helics::FederateInfo *> (fi));
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return nullptr;
    }
    FedI->type = helics::vtype::value_fed;
    FedI->valid = fedValidationIdentifier;
    auto fed = reinterpret_cast<helics_federate> (FedI.get ());
    getMasterHolder ()->addFed (std::move (FedI));
    return (fed);
}

helics_federate helicsCreateValueFederateFromConfig (const char *configFile, helics_error *err)
{
    HELICS_ERROR_CHECK (err, nullptr);
    auto FedI = std::make_unique<helics::FedObject> ();
    try
    {
        FedI->fedptr = std::make_shared<helics::ValueFederate> ((configFile != nullptr) ? std::string (configFile) : nullstr);
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return nullptr;
    }
    FedI->type = helics::vtype::value_fed;
    FedI->valid = fedValidationIdentifier;
    auto fed = reinterpret_cast<helics_federate> (FedI.get ());
    getMasterHolder ()->addFed (std::move (FedI));
    return (fed);
}

/* Creation and destruction of Federates */
helics_federate helicsCreateMessageFederate (const char *fedName, const helics_federate_info fi, helics_error *err)
{
    HELICS_ERROR_CHECK (err, nullptr);
    auto FedI = std::make_unique<helics::FedObject> ();
    try
    {
        if (fi == nullptr)
        {
            FedI->fedptr =
              std::make_shared<helics::MessageFederate> ((fedName != nullptr) ? std::string (fedName) : nullstr, helics::FederateInfo ());
        }
        else
        {
            FedI->fedptr = std::make_shared<helics::MessageFederate> ((fedName != nullptr) ? std::string (fedName) : nullstr,
                                                                      *reinterpret_cast<helics::FederateInfo *> (fi));
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return nullptr;
    }
    FedI->type = helics::vtype::message_fed;
    FedI->valid = fedValidationIdentifier;
    auto fed = reinterpret_cast<helics_federate> (FedI.get ());
    getMasterHolder ()->addFed (std::move (FedI));
    return (fed);
}

helics_federate helicsCreateMessageFederateFromConfig (const char *configFile, helics_error *err)
{
    HELICS_ERROR_CHECK (err, nullptr);
    auto FedI = std::make_unique<helics::FedObject> ();

    try
    {
        FedI->fedptr = std::make_shared<helics::MessageFederate> ((configFile != nullptr) ? std::string (configFile) : nullstr);
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return nullptr;
    }
    FedI->type = helics::vtype::message_fed;
    FedI->valid = fedValidationIdentifier;
    auto fed = reinterpret_cast<helics_federate> (FedI.get ());
    getMasterHolder ()->addFed (std::move (FedI));
    return (fed);
}

/* Creation and destruction of Federates */
helics_federate helicsCreateCombinationFederate (const char *fedName, const helics_federate_info fi, helics_error *err)
{
    HELICS_ERROR_CHECK (err, nullptr);
    auto FedI = std::make_unique<helics::FedObject> ();
    try
    {
        if (fi == nullptr)
        {
            FedI->fedptr = std::make_shared<helics::CombinationFederate> ((fedName != nullptr) ? std::string (fedName) : nullstr,
                                                                          helics::FederateInfo ());
        }
        else
        {
            FedI->fedptr = std::make_shared<helics::CombinationFederate> ((fedName != nullptr) ? std::string (fedName) : nullstr,
                                                                          *reinterpret_cast<helics::FederateInfo *> (fi));
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return nullptr;
    }
    FedI->type = helics::vtype::combination_fed;
    FedI->valid = fedValidationIdentifier;
    auto fed = reinterpret_cast<helics_federate> (FedI.get ());
    getMasterHolder ()->addFed (std::move (FedI));
    return (fed);
}

helics_federate helicsCreateCombinationFederateFromConfig (const char *configFile, helics_error *err)
{
    HELICS_ERROR_CHECK (err, nullptr);
    auto FedI = std::make_unique<helics::FedObject> ();
    try
    {
        FedI->fedptr = std::make_shared<helics::CombinationFederate> ((configFile != nullptr) ? std::string (configFile) : nullstr);
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return nullptr;
    }

    FedI->type = helics::vtype::combination_fed;
    FedI->valid = fedValidationIdentifier;
    auto fed = reinterpret_cast<helics_federate> (FedI.get ());
    getMasterHolder ()->addFed (std::move (FedI));
    return (fed);
}

helics_federate helicsFederateClone (helics_federate fed, helics_error *err)
{
    auto *fedObj = helics::getFedObject (fed, err);
    if (fedObj == nullptr)
    {
        return nullptr;
    }
    auto fedClone = std::make_unique<helics::FedObject> ();
    fedClone->fedptr = fedObj->fedptr;

    fedClone->type = fedObj->type;
    fedClone->valid = fedObj->valid;
    auto fedB = reinterpret_cast<helics_federate> (fedClone.get ());
    getMasterHolder ()->addFed (std::move (fedClone));
    return (fedB);
}

helics_bool helicsFederateIsValid (helics_federate fed)
{
    auto fedObj = getFed (fed, nullptr);
    return (fedObj == nullptr) ? helics_false : helics_true;
}

helics_core helicsFederateGetCoreObject (helics_federate fed, helics_error *err)
{
    auto fedObj = getFed (fed, err);
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

static constexpr char invalidFile[] = "Invalid File specification";

void helicsFederateRegisterInterfaces (helics_federate fed, const char *file, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return;
    }
    if (file == nullptr)
    {
        if (err != nullptr)
        {
            err->error_code = helics_error_invalid_argument;
            err->message = invalidFile;
        }
        return;
    }
    try
    {
        fedObj->registerInterfaces (file);
    }
    catch (...)
    {
        return helicsErrorHandler (err);
    }
}

void helicsFederateFinalize (helics_federate fed, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return;
    }
    try
    {
        fedObj->finalize ();
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

/* initialization, execution, and time requests */
void helicsFederateEnterInitializingMode (helics_federate fed, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return;
    }
    try
    {
        fedObj->enterInitializingMode ();
    }
    catch (...)
    {
        return helicsErrorHandler (err);
    }
}

void helicsFederateEnterInitializingModeAsync (helics_federate fed, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return;
    }
    try
    {
        fedObj->enterInitializingModeAsync ();
    }
    catch (...)
    {
        return helicsErrorHandler (err);
    }
}

helics_bool helicsFederateIsAsyncOperationCompleted (helics_federate fed, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return helics_false;
    }
    return (fedObj->isAsyncOperationCompleted ()) ? helics_true : helics_false;
}

void helicsFederateEnterInitializingModeComplete (helics_federate fed, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return;
    }
    try
    {
        fedObj->enterInitializingModeComplete ();
    }
    catch (...)
    {
        return helicsErrorHandler (err);
    }
}

void helicsFederateEnterExecutingMode (helics_federate fed, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return;
    }
    try
    {
        // printf("current state=%d\n", static_cast<int>(fedObj->getCurrentState()));
        fedObj->enterExecutingMode ();
    }
    catch (...)
    {
        return helicsErrorHandler (err);
    }
}

static helics::iteration_request getIterationRequest (helics_iteration_request iterate)
{
    switch (iterate)
    {
    case helics_iteration_request_no_iteration:
    default:
        return helics::iteration_request::no_iterations;
    case helics_iteration_request_force_iteration:
        return helics::iteration_request::force_iteration;

    case helics_iteration_request_iterate_if_needed:
        return helics::iteration_request::iterate_if_needed;
    }
}

static helics_iteration_result getIterationStatus (helics::iteration_result iterationState)
{
    switch (iterationState)
    {
    case helics::iteration_result::next_step:
        return helics_iteration_result_next_step;
    case helics::iteration_result::iterating:
        return helics_iteration_result_iterating;
    case helics::iteration_result::error:
    default:
        return helics_iteration_result_error;
    case helics::iteration_result::halted:
        return helics_iteration_result_halted;
    }
}

helics_iteration_result helicsFederateEnterExecutingModeIterative (helics_federate fed, helics_iteration_request iterate, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return helics_iteration_result_error;
    }
    try
    {
        auto val = fedObj->enterExecutingMode (getIterationRequest (iterate));
        return getIterationStatus (val);
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return helics_iteration_result_error;
    }
}

void helicsFederateEnterExecutingModeAsync (helics_federate fed, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return;
    }
    try
    {
        fedObj->enterExecutingModeAsync ();
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsFederateEnterExecutingModeIterativeAsync (helics_federate fed, helics_iteration_request iterate, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return;
    }
    try
    {
        fedObj->enterExecutingModeAsync (getIterationRequest (iterate));
    }
    catch (...)
    {
        return helicsErrorHandler (err);
    }
}

void helicsFederateEnterExecutingModeComplete (helics_federate fed, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return;
    }
    try
    {
        fedObj->enterExecutingModeComplete ();
    }
    catch (...)
    {
        return helicsErrorHandler (err);
    }
}
helics_iteration_result helicsFederateEnterExecutingModeIterativeComplete (helics_federate fed, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return helics_iteration_result_error;
    }
    try
    {
        auto val = fedObj->enterExecutingModeComplete ();
        return getIterationStatus (val);
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return helics_iteration_result_error;
    }
}

helics_time helicsFederateRequestTime (helics_federate fed, helics_time requestTime, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return helics_time_invalid;
    }
    try
    {
        return static_cast<double> (fedObj->requestTime (requestTime));
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return helics_time_invalid;
    }
}

helics_time helicsFederateRequestNextStep (helics_federate fed, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return helics_time_invalid;
    }
    try
    {
        return static_cast<helics_time> (fedObj->requestNextStep ());
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return helics_time_invalid;
    }
}

helics_time helicsFederateRequestTimeIterative (helics_federate fed,
                                                helics_time requestTime,
                                                helics_iteration_request iterate,
                                                helics_iteration_result *outIterate,
                                                helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        if (outIterate != nullptr)
        {
            *outIterate = helics_iteration_result_error;
        }
        return helics_time_invalid;
    }
    try
    {
        auto val = fedObj->requestTimeIterative (requestTime, getIterationRequest (iterate));
        if (outIterate != nullptr)
        {
            *outIterate = getIterationStatus (val.state);
        }
        return static_cast<double> (val.grantedTime);
    }
    catch (...)
    {
        helicsErrorHandler (err);
        if (outIterate != nullptr)
        {
            *outIterate = helics_iteration_result_error;
        }
        return helics_time_invalid;
    }
}

void helicsFederateRequestTimeAsync (helics_federate fed, helics_time requestTime, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return;
    }
    try
    {
        fedObj->requestTimeAsync (requestTime);
    }
    catch (...)
    {
        return helicsErrorHandler (err);
    }
}

void helicsFederateRequestTimeIterativeAsync (helics_federate fed,
                                              helics_time requestTime,
                                              helics_iteration_request iterate,
                                              helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return;
    }
    try
    {
        fedObj->requestTimeIterative (requestTime, getIterationRequest (iterate));
    }
    catch (...)
    {
        return helicsErrorHandler (err);
    }
}

helics_time helicsFederateRequestTimeComplete (helics_federate fed, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return helics_time_invalid;
    }
    try
    {
        return static_cast<double> (fedObj->requestTimeComplete ());
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return helics_time_invalid;
    }
}

static const std::map<helics::Federate::states, helics_federate_state> stateEnumConversions{
  {helics::Federate::states::error, helics_federate_state::helics_state_error},
  {helics::Federate::states::startup, helics_federate_state::helics_state_startup},
  {helics::Federate::states::execution, helics_federate_state::helics_state_execution},
  {helics::Federate::states::finalize, helics_federate_state::helics_state_finalize},
  {helics::Federate::states::pending_exec, helics_federate_state::helics_state_pending_exec},
  {helics::Federate::states::pending_init, helics_federate_state::helics_state_pending_init},
  {helics::Federate::states::pending_iterative_time, helics_federate_state::helics_state_pending_iterative_time},
  {helics::Federate::states::pending_time, helics_federate_state::helics_state_pending_time},
  {helics::Federate::states::initialization, helics_federate_state::helics_state_initialization}};

helics_federate_state helicsFederateGetState (helics_federate fed, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return helics_state_error;
    }
    try
    {
        auto FedState = fedObj->getCurrentState ();
        return stateEnumConversions.at (FedState);
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return helics_state_error;
    }
}

const char *helicsFederateGetName (helics_federate fed)
{
    auto fedObj = getFed (fed, nullptr);
    if (fedObj == nullptr)
    {
        return nullcstr;
    }
    auto &ident = fedObj->getName ();
    return ident.c_str ();
}

void helicsFederateSetTimeProperty (helics_federate fed, int timeProperty, helics_time time, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return;
    }
    try
    {
        fedObj->setTimeProperty (timeProperty, time);
    }
    catch (...)
    {
        return helicsErrorHandler (err);
    }
}

void helicsFederateSetFlagOption (helics_federate fed, int flag, helics_bool flagValue, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return;
    }
    try
    {
        fedObj->setFlagOption (flag, (flagValue != helics_false));
    }
    catch (...)
    {
        return helicsErrorHandler (err);
    }
}

void helicsFederateSetIntegerProperty (helics_federate fed, int intProperty, int propVal, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return;
    }
    try
    {
        fedObj->setIntegerProperty (intProperty, propVal);
    }
    catch (...)
    {
        return helicsErrorHandler (err);
    }
}

helics_time helicsFederateGetTimeProperty (helics_federate fed, int timeProperty, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return helics_time_invalid;
    }
    try
    {
        auto T = fedObj->getTimeProperty (timeProperty);
        return static_cast<double> (T);
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return helics_time_invalid;
    }
}

helics_bool helicsFederateGetFlagOption (helics_federate fed, int flag, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return helics_false;
    }
    try
    {
        bool res = fedObj->getFlagOption (flag);
        return (res) ? helics_true : helics_false;
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return helics_false;
    }
}

int helicsFederateGetIntegerProperty (helics_federate fed, int intProperty, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return -101;
    }
    try
    {
        return fedObj->getIntegerProperty (intProperty);
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return -101;
    }
}

void helicsFederateSetSeparator (helics_federate fed, char separator, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return;
    }
    try
    {
        fedObj->setSeparator (separator);
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

helics_time helicsFederateGetCurrentTime (helics_federate fed, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return helics_time_invalid;
    }
    try
    {
        return static_cast<double> (fedObj->getCurrentTime ());
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return helics_time_invalid;
    }
}

helics_time helicsFederateRequestTimeIterativeComplete (helics_federate fed, helics_iteration_result *outIteration, helics_error *err)
{
    auto fedObj = getFed (fed, err);
    if (fedObj == nullptr)
    {
        return helics_time_invalid;
    }
    try
    {
        auto val = fedObj->requestTimeIterativeComplete ();
        if (outIteration != nullptr)
        {
            *outIteration = getIterationStatus (val.state);
        }
        return static_cast<double> (val.grantedTime);
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return helics_time_invalid;
    }
}
