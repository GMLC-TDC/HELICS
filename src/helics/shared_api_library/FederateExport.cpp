/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../core/core-exceptions.hpp"
#include "../core/coreTypeOperations.hpp"
#include "../helics.hpp"
#include "gmlc/concurrency/TripWire.hpp"
#include "helics.h"
#include "internal/api_objects.h"

#include <iostream>
#include <map>
#include <mutex>
#include <vector>

/** this is a random identifier put in place when the federate or core or broker gets created*/
static const int fedValidationIdentifier = 0x2352188;
static const char* invalidFedString = "federate object is not valid";

static constexpr char nullcstr[] = "";

namespace helics {
FedObject* getFedObject(helics_federate fed, helics_error* err) noexcept
{
    HELICS_ERROR_CHECK(err, nullptr);
    if (fed == nullptr) {
        assignError(err, helics_error_invalid_object, invalidFedString);
        return nullptr;
    }
    auto* fedObj = reinterpret_cast<helics::FedObject*>(fed);
    if (fedObj->valid == fedValidationIdentifier) {
        return fedObj;
    }
    assignError(err, helics_error_invalid_object, invalidFedString);
    return nullptr;
}
}  // namespace helics

helics::Federate* getFed(helics_federate fed, helics_error* err)
{
    auto* fedObj = helics::getFedObject(fed, err);
    return (fedObj == nullptr) ? nullptr : fedObj->fedptr.get();
}

static const char* notValueFedString = "Federate must be a value federate";

helics::ValueFederate* getValueFed(helics_federate fed, helics_error* err)
{
    auto* fedObj = helics::getFedObject(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    if ((fedObj->type == helics::vtype::value_fed) || (fedObj->type == helics::vtype::combination_fed)) {
        auto* rval = dynamic_cast<helics::ValueFederate*>(fedObj->fedptr.get());
        if (rval != nullptr) {
            return rval;
        }
    }
    assignError(err, helics_error_invalid_object, notValueFedString);
    return nullptr;
}

static const char* notMessageFedString = "Federate must be a message federate";

helics::MessageFederate* getMessageFed(helics_federate fed, helics_error* err)
{
    auto* fedObj = helics::getFedObject(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    if ((fedObj->type == helics::vtype::message_fed) || (fedObj->type == helics::vtype::combination_fed)) {
        auto* rval = dynamic_cast<helics::MessageFederate*>(fedObj->fedptr.get());
        if (rval != nullptr) {
            return rval;
        }
    }
    assignError(err, helics_error_invalid_object, notMessageFedString);
    return nullptr;
}

std::shared_ptr<helics::Federate> getFedSharedPtr(helics_federate fed, helics_error* err)
{
    auto* fedObj = helics::getFedObject(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    return fedObj->fedptr;
}

std::shared_ptr<helics::ValueFederate> getValueFedSharedPtr(helics_federate fed, helics_error* err)
{
    auto* fedObj = helics::getFedObject(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    if ((fedObj->type == helics::vtype::value_fed) || (fedObj->type == helics::vtype::combination_fed)) {
        auto rval = std::dynamic_pointer_cast<helics::ValueFederate>(fedObj->fedptr);
        if (rval) {
            return rval;
        }
    }
    assignError(err, helics_error_invalid_object, notValueFedString);
    return nullptr;
}

std::shared_ptr<helics::MessageFederate> getMessageFedSharedPtr(helics_federate fed, helics_error* err)
{
    auto* fedObj = helics::getFedObject(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    if ((fedObj->type == helics::vtype::message_fed) || (fedObj->type == helics::vtype::combination_fed)) {
        auto rval = std::dynamic_pointer_cast<helics::MessageFederate>(fedObj->fedptr);
        if (rval) {
            return rval;
        }
    }
    assignError(err, helics_error_invalid_object, notMessageFedString);
    return nullptr;
}

// random integer for validation purposes of endpoints
static constexpr int FederateInfoValidationIdentifier = 0x6bfb'bce1;

helics_federate_info helicsCreateFederateInfo()
{
    auto* fi = new helics::FederateInfo;
    fi->uniqueKey = FederateInfoValidationIdentifier;
    return reinterpret_cast<void*>(fi);
}

static const char* invalidFedInfoString = "helics Federate info object was not valid";

static helics::FederateInfo* getFedInfo(helics_federate_info fi, helics_error* err)
{
    if ((err != nullptr) && (err->error_code != 0)) {
        return nullptr;
    }
    if (fi == nullptr) {
        assignError(err, helics_error_invalid_object, invalidFedInfoString);
        return nullptr;
    }
    auto* ptr = reinterpret_cast<helics::FederateInfo*>(fi);
    if (ptr->uniqueKey != FederateInfoValidationIdentifier) {
        assignError(err, helics_error_invalid_object, invalidFedInfoString);
        return nullptr;
    }
    return ptr;
}

helics_federate_info helicsFederateInfoClone(helics_federate_info fi, helics_error* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return nullptr;
    }
    auto* fi_new = new helics::FederateInfo(*info);
    return reinterpret_cast<void*>(fi_new);
}

void helicsFederateInfoFree(helics_federate_info fi)
{
    auto* info = getFedInfo(fi, nullptr);
    if (info == nullptr) {
        // fprintf(stderr, "The helics_federate_info object is not valid\n");
        return;
    }
    info->uniqueKey = 0;
    delete info;
}

void helicsFederateInfoLoadFromArgs(helics_federate_info fi, int argc, const char* const* argv, helics_error* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    try {
        std::vector<std::string> args;
        args.reserve(static_cast<size_t>(argc) - 1);
        for (int ii = argc - 1; ii > 0; --ii) {
            args.emplace_back(argv[ii]);
        }
        info->loadInfoFromArgs(args);
    }
    catch (...) {
        return helicsErrorHandler(err);
    }
}

void helicsFederateInfoSetCoreName(helics_federate_info fi, const char* corename, helics_error* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    try {
        info->coreName = AS_STRING(corename);
    }
    catch (...) {  // LCOV_EXCL_LINE
        return helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateInfoSetCoreInitString(helics_federate_info fi, const char* coreinit, helics_error* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    try {
        info->coreInitString = AS_STRING(coreinit);
    }
    catch (...) {  // LCOV_EXCL_LINE
        return helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateInfoSetBrokerInitString(helics_federate_info fi, const char* brokerinit, helics_error* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    try {
        info->brokerInitString = AS_STRING(brokerinit);
    }
    catch (...) {  // LCOV_EXCL_LINE
        return helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateInfoSetCoreType(helics_federate_info fi, int coretype, helics_error* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    info->coreType = static_cast<helics::core_type>(coretype);
}

void helicsFederateInfoSetCoreTypeFromString(helics_federate_info fi, const char* coretype, helics_error* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    if (coretype == nullptr) {
        info->coreType = helics::core_type::DEFAULT;
        return;
    }
    auto ctype = helics::core::coreTypeFromString(coretype);
    if (ctype == helics::core_type::UNRECOGNIZED) {
        if (err != nullptr) {
            err->error_code = helics_error_invalid_argument;
            err->message = getMasterHolder()->addErrorString(std::string(coretype) + " is not a valid core type");
            return;
        }
    }
    info->coreType = ctype;
}

void helicsFederateInfoSetBroker(helics_federate_info fi, const char* broker, helics_error* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    try {
        info->broker = AS_STRING(broker);
    }
    catch (...) {  // LCOV_EXCL_LINE
        return helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateInfoSetBrokerKey(helics_federate_info fi, const char* brokerkey, helics_error* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    try {
        info->key = AS_STRING(brokerkey);
    }
    catch (...) {  // LCOV_EXCL_LINE
        return helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateInfoSetBrokerPort(helics_federate_info fi, int brokerPort, helics_error* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    info->brokerPort = brokerPort;
}

void helicsFederateInfoSetLocalPort(helics_federate_info fi, const char* localPort, helics_error* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    info->localport = AS_STRING(localPort);
}

int helicsGetPropertyIndex(const char* val)
{
    if (val == nullptr) {
        return -1;
    }
    return helics::getPropertyIndex(val);
}

int helicsGetFlagIndex(const char* val)
{
    if (val == nullptr) {
        return -1;
    }
    return helics::getFlagIndex(val);
}

int helicsGetOptionIndex(const char* val)
{
    if (val == nullptr) {
        return -1;
    }
    return helics::getOptionIndex(val);
}

int helicsGetOptionValue(const char* val)
{
    if (val == nullptr) {
        return -1;
    }
    return helics::getOptionValue(val);
}

void helicsFederateInfoSetFlagOption(helics_federate_info fi, int flag, helics_bool value, helics_error* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    info->setFlagOption(flag, (value != helics_false));
}

void helicsFederateInfoSetTimeProperty(helics_federate_info fi, int timeProperty, helics_time propertyValue, helics_error* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    info->setProperty(timeProperty, propertyValue);
}

void helicsFederateInfoSetSeparator(helics_federate_info fi, char separator, helics_error* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    info->separator = separator;
}

void helicsFederateInfoSetIntegerProperty(helics_federate_info fi, int integerProperty, int propertyValue, helics_error* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    info->setProperty(integerProperty, propertyValue);
}

/* Creation and destruction of Federates */
helics_federate helicsCreateValueFederate(const char* fedName, helics_federate_info fi, helics_error* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    auto FedI = std::make_unique<helics::FedObject>();

    try {
        if (fi == nullptr) {
            FedI->fedptr = std::make_shared<helics::ValueFederate>(AS_STRING(fedName), helics::FederateInfo());
        } else {
            auto* info = getFedInfo(fi, err);
            if (info == nullptr) {
                return nullptr;
            }
            FedI->fedptr = std::make_shared<helics::ValueFederate>(AS_STRING(fedName), *info);
        }
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    FedI->type = helics::vtype::value_fed;
    FedI->valid = fedValidationIdentifier;
    auto* fed = reinterpret_cast<helics_federate>(FedI.get());
    getMasterHolder()->addFed(std::move(FedI));
    return (fed);
}

helics_federate helicsCreateValueFederateFromConfig(const char* configFile, helics_error* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    auto FedI = std::make_unique<helics::FedObject>();
    try {
        FedI->fedptr = std::make_shared<helics::ValueFederate>(AS_STRING(configFile));
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    FedI->type = helics::vtype::value_fed;
    FedI->valid = fedValidationIdentifier;
    auto* fed = reinterpret_cast<helics_federate>(FedI.get());
    getMasterHolder()->addFed(std::move(FedI));
    return (fed);
}

/* Creation and destruction of Federates */
helics_federate helicsCreateMessageFederate(const char* fedName, helics_federate_info fi, helics_error* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    auto FedI = std::make_unique<helics::FedObject>();
    try {
        if (fi == nullptr) {
            FedI->fedptr = std::make_shared<helics::MessageFederate>(AS_STRING(fedName), helics::FederateInfo());
        } else {
            auto* info = getFedInfo(fi, err);
            if (info == nullptr) {
                return nullptr;
            }
            FedI->fedptr = std::make_shared<helics::MessageFederate>(AS_STRING(fedName), *info);
        }
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    FedI->type = helics::vtype::message_fed;
    FedI->valid = fedValidationIdentifier;
    auto* fed = reinterpret_cast<helics_federate>(FedI.get());
    getMasterHolder()->addFed(std::move(FedI));
    return (fed);
}

helics_federate helicsCreateMessageFederateFromConfig(const char* configFile, helics_error* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    auto FedI = std::make_unique<helics::FedObject>();

    try {
        FedI->fedptr = std::make_shared<helics::MessageFederate>(AS_STRING(configFile));
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    FedI->type = helics::vtype::message_fed;
    FedI->valid = fedValidationIdentifier;
    auto* fed = reinterpret_cast<helics_federate>(FedI.get());
    getMasterHolder()->addFed(std::move(FedI));
    return (fed);
}

/* Creation and destruction of Federates */
helics_federate helicsCreateCombinationFederate(const char* fedName, helics_federate_info fi, helics_error* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    auto FedI = std::make_unique<helics::FedObject>();
    try {
        if (fi == nullptr) {
            FedI->fedptr = std::make_shared<helics::CombinationFederate>(AS_STRING(fedName), helics::FederateInfo());
        } else {
            auto* info = getFedInfo(fi, err);
            if (info == nullptr) {
                return nullptr;
            }
            FedI->fedptr = std::make_shared<helics::CombinationFederate>(AS_STRING(fedName), *info);
        }
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    FedI->type = helics::vtype::combination_fed;
    FedI->valid = fedValidationIdentifier;
    auto* fed = reinterpret_cast<helics_federate>(FedI.get());
    getMasterHolder()->addFed(std::move(FedI));
    return (fed);
}

helics_federate helicsCreateCombinationFederateFromConfig(const char* configFile, helics_error* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    auto FedI = std::make_unique<helics::FedObject>();
    try {
        FedI->fedptr = std::make_shared<helics::CombinationFederate>(AS_STRING(configFile));
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }

    FedI->type = helics::vtype::combination_fed;
    FedI->valid = fedValidationIdentifier;
    auto* fed = reinterpret_cast<helics_federate>(FedI.get());
    getMasterHolder()->addFed(std::move(FedI));
    return (fed);
}

helics_federate helicsFederateClone(helics_federate fed, helics_error* err)
{
    auto* fedObj = helics::getFedObject(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    auto fedClone = std::make_unique<helics::FedObject>();
    fedClone->fedptr = fedObj->fedptr;

    fedClone->type = fedObj->type;
    fedClone->valid = fedObj->valid;
    auto* fedB = reinterpret_cast<helics_federate>(fedClone.get());
    getMasterHolder()->addFed(std::move(fedClone));
    return (fedB);
}

helics_bool helicsFederateIsValid(helics_federate fed)
{
    auto* fedObj = getFed(fed, nullptr);
    return (fedObj == nullptr) ? helics_false : helics_true;
}

helics_core helicsFederateGetCoreObject(helics_federate fed, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    auto core = std::make_unique<helics::CoreObject>();
    core->valid = coreValidationIdentifier;
    core->coreptr = fedObj->getCorePointer();
    auto* retcore = reinterpret_cast<helics_core>(core.get());
    getMasterHolder()->addCore(std::move(core));
    return retcore;
}

static constexpr char invalidFile[] = "Invalid File specification";

void helicsFederateRegisterInterfaces(helics_federate fed, const char* file, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    if (file == nullptr) {
        assignError(err, helics_error_invalid_argument, invalidFile);
        return;
    }
    try {
        fedObj->registerInterfaces(file);
    }
    catch (...) {
        return helicsErrorHandler(err);
    }
}

void helicsFederateGlobalError(helics_federate fed, int errorCode, const char* errorString)
{
    auto* fedObj = getFed(fed, nullptr);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->globalError(errorCode, AS_STRING(errorString));
    }
    // LCOV_EXCL_START
    catch (...) {
    }
    // LCOV_EXCL_STOP
}

void helicsFederateLocalError(helics_federate fed, int errorCode, const char* errorString)
{
    auto* fedObj = getFed(fed, nullptr);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->localError(errorCode, AS_STRING(errorString));
    }
    // LCOV_EXCL_START
    catch (...) {
    }
    // LCOV_EXCL_STOP
}

void helicsFederateFinalize(helics_federate fed, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->finalize();
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsFederateFinalizeAsync(helics_federate fed, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->finalizeAsync();
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsFederateFinalizeComplete(helics_federate fed, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->finalizeComplete();
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

/* initialization, execution, and time requests */
void helicsFederateEnterInitializingMode(helics_federate fed, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->enterInitializingMode();
    }
    catch (...) {
        return helicsErrorHandler(err);
    }
}

void helicsFederateEnterInitializingModeAsync(helics_federate fed, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->enterInitializingModeAsync();
    }
    catch (...) {
        return helicsErrorHandler(err);
    }
}

helics_bool helicsFederateIsAsyncOperationCompleted(helics_federate fed, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return helics_false;
    }
    return (fedObj->isAsyncOperationCompleted()) ? helics_true : helics_false;
}

void helicsFederateEnterInitializingModeComplete(helics_federate fed, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->enterInitializingModeComplete();
    }
    catch (...) {
        return helicsErrorHandler(err);
    }
}

void helicsFederateEnterExecutingMode(helics_federate fed, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        // printf("current state=%d\n", static_cast<int>(fedObj->getCurrentState()));
        fedObj->enterExecutingMode();
    }
    catch (...) {
        return helicsErrorHandler(err);
    }
}

static helics::iteration_request getIterationRequest(helics_iteration_request iterate)
{
    switch (iterate) {
        case helics_iteration_request_no_iteration:
        default:
            return helics::iteration_request::no_iterations;
        case helics_iteration_request_force_iteration:
            return helics::iteration_request::force_iteration;

        case helics_iteration_request_iterate_if_needed:
            return helics::iteration_request::iterate_if_needed;
    }
}

static helics_iteration_result getIterationStatus(helics::iteration_result iterationState)
{
    switch (iterationState) {
        case helics::iteration_result::next_step:
            return helics_iteration_result_next_step;
        case helics::iteration_result::iterating:
            return helics_iteration_result_iterating;
        case helics::iteration_result::error:
        default:
            // most cases of this return error directly without going through this function
            return helics_iteration_result_error;  // LCOV_EXCL_LINE
        case helics::iteration_result::halted:
            return helics_iteration_result_halted;
    }
}

helics_iteration_result helicsFederateEnterExecutingModeIterative(helics_federate fed, helics_iteration_request iterate, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return helics_iteration_result_error;
    }
    try {
        auto val = fedObj->enterExecutingMode(getIterationRequest(iterate));
        return getIterationStatus(val);
    }
    catch (...) {
        helicsErrorHandler(err);
        return helics_iteration_result_error;
    }
}

void helicsFederateEnterExecutingModeAsync(helics_federate fed, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->enterExecutingModeAsync();
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsFederateEnterExecutingModeIterativeAsync(helics_federate fed, helics_iteration_request iterate, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->enterExecutingModeAsync(getIterationRequest(iterate));
    }
    catch (...) {
        return helicsErrorHandler(err);
    }
}

void helicsFederateEnterExecutingModeComplete(helics_federate fed, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->enterExecutingModeComplete();
    }
    catch (...) {
        return helicsErrorHandler(err);
    }
}
helics_iteration_result helicsFederateEnterExecutingModeIterativeComplete(helics_federate fed, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return helics_iteration_result_error;
    }
    try {
        auto val = fedObj->enterExecutingModeComplete();
        return getIterationStatus(val);
    }
    catch (...) {
        helicsErrorHandler(err);
        return helics_iteration_result_error;
    }
}

helics_time helicsFederateRequestTime(helics_federate fed, helics_time requestTime, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return helics_time_invalid;
    }
    try {
        auto timeret = fedObj->requestTime(requestTime);
        return (timeret < helics::Time::maxVal()) ? static_cast<double>(timeret) : helics_time_maxtime;
    }
    catch (...) {
        helicsErrorHandler(err);
        return helics_time_invalid;
    }
}

helics_time helicsFederateRequestTimeAdvance(helics_federate fed, helics_time timeDelta, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return helics_time_invalid;
    }
    try {
        auto timeret = fedObj->requestTimeAdvance(timeDelta);
        return (timeret < helics::Time::maxVal()) ? static_cast<double>(timeret) : helics_time_maxtime;
    }
    catch (...) {
        helicsErrorHandler(err);
        return helics_time_invalid;
    }
}

helics_time helicsFederateRequestNextStep(helics_federate fed, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return helics_time_invalid;
    }
    try {
        auto timeret = fedObj->requestNextStep();
        return (timeret < helics::Time::maxVal()) ? static_cast<double>(timeret) : helics_time_maxtime;
    }
    catch (...) {
        helicsErrorHandler(err);
        return helics_time_invalid;
    }
}

helics_time helicsFederateRequestTimeIterative(helics_federate fed,
                                               helics_time requestTime,
                                               helics_iteration_request iterate,
                                               helics_iteration_result* outIteration,
                                               helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        if (outIteration != nullptr) {
            *outIteration = helics_iteration_result_error;
        }
        return helics_time_invalid;
    }
    try {
        auto val = fedObj->requestTimeIterative(requestTime, getIterationRequest(iterate));
        if (outIteration != nullptr) {
            *outIteration = getIterationStatus(val.state);
        }
        return (val.grantedTime < helics::Time::maxVal()) ? static_cast<double>(val.grantedTime) : helics_time_maxtime;
    }
    catch (...) {
        helicsErrorHandler(err);
        if (outIteration != nullptr) {
            *outIteration = helics_iteration_result_error;
        }
        return helics_time_invalid;
    }
}

void helicsFederateRequestTimeAsync(helics_federate fed, helics_time requestTime, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->requestTimeAsync(requestTime);
    }
    catch (...) {
        return helicsErrorHandler(err);
    }
}

helics_time helicsFederateRequestTimeComplete(helics_federate fed, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return helics_time_invalid;
    }
    try {
        auto timeret = fedObj->requestTimeComplete();
        return (timeret < helics::Time::maxVal()) ? static_cast<double>(timeret) : helics_time_maxtime;
    }
    catch (...) {
        helicsErrorHandler(err);
        return helics_time_invalid;
    }
}

void helicsFederateRequestTimeIterativeAsync(helics_federate fed,
                                             helics_time requestTime,
                                             helics_iteration_request iterate,
                                             helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->requestTimeIterativeAsync(requestTime, getIterationRequest(iterate));
    }
    catch (...) {
        return helicsErrorHandler(err);
    }
}

helics_time helicsFederateRequestTimeIterativeComplete(helics_federate fed, helics_iteration_result* outIteration, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        if (outIteration != nullptr) {
            *outIteration = helics_iteration_result_error;
        }
        return helics_time_invalid;
    }
    try {
        auto val = fedObj->requestTimeIterativeComplete();
        if (outIteration != nullptr) {
            *outIteration = getIterationStatus(val.state);
        }
        return (val.grantedTime < helics::Time::maxVal()) ? static_cast<double>(val.grantedTime) : helics_time_maxtime;
    }
    catch (...) {
        helicsErrorHandler(err);
        if (outIteration != nullptr) {
            *outIteration = helics_iteration_result_error;
        }
        return helics_time_invalid;
    }
}

static const std::map<helics::Federate::modes, helics_federate_state> modeEnumConversions{
    {helics::Federate::modes::error, helics_federate_state::helics_state_error},
    {helics::Federate::modes::startup, helics_federate_state::helics_state_startup},
    {helics::Federate::modes::executing, helics_federate_state::helics_state_execution},
    {helics::Federate::modes::finalize, helics_federate_state::helics_state_finalize},
    {helics::Federate::modes::pending_exec, helics_federate_state::helics_state_pending_exec},
    {helics::Federate::modes::pending_init, helics_federate_state::helics_state_pending_init},
    {helics::Federate::modes::pending_iterative_time, helics_federate_state::helics_state_pending_iterative_time},
    {helics::Federate::modes::pending_time, helics_federate_state::helics_state_pending_time},
    {helics::Federate::modes::initializing, helics_federate_state::helics_state_initialization},
    {helics::Federate::modes::pending_finalize, helics_federate_state::helics_state_pending_finalize},
    {helics::Federate::modes::finished, helics_federate_state::helics_state_finished}};

helics_federate_state helicsFederateGetState(helics_federate fed, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return helics_state_error;
    }
    try {
        auto FedMode = fedObj->getCurrentMode();
        return modeEnumConversions.at(FedMode);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return helics_state_error;
    }
    // LCOV_EXCL_STOP
}

const char* helicsFederateGetName(helics_federate fed)
{
    auto* fedObj = getFed(fed, nullptr);
    if (fedObj == nullptr) {
        return nullcstr;
    }
    const auto& ident = fedObj->getName();
    return ident.c_str();
}

void helicsFederateSetTimeProperty(helics_federate fed, int timeProperty, helics_time time, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->setProperty(timeProperty, time);
    }
    catch (...) {
        return helicsErrorHandler(err);
    }
}

void helicsFederateSetFlagOption(helics_federate fed, int flag, helics_bool flagValue, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->setFlagOption(flag, (flagValue != helics_false));
    }
    // LCOV_EXCL_START
    catch (...) {
        return helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsFederateSetIntegerProperty(helics_federate fed, int intProperty, int propVal, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->setProperty(intProperty, propVal);
    }
    // LCOV_EXCL_START
    catch (...) {
        return helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

helics_time helicsFederateGetTimeProperty(helics_federate fed, int timeProperty, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return helics_time_invalid;
    }
    try {
        auto T = fedObj->getTimeProperty(timeProperty);

        return (T < helics::Time::maxVal()) ? static_cast<double>(T) : helics_time_maxtime;
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return helics_time_invalid;
    }
    // LCOV_EXCL_STOP
}

helics_bool helicsFederateGetFlagOption(helics_federate fed, int flag, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return helics_false;
    }
    try {
        bool res = fedObj->getFlagOption(flag);
        return (res) ? helics_true : helics_false;
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return helics_false;
    }
    // LCOV_EXCL_STOP
}

int helicsFederateGetIntegerProperty(helics_federate fed, int intProperty, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return -101;
    }
    try {
        return fedObj->getIntegerProperty(intProperty);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return -101;
    }
    // LCOV_EXCL_STOP
}

void helicsFederateSetSeparator(helics_federate fed, char separator, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    fedObj->setSeparator(separator);
}

helics_time helicsFederateGetCurrentTime(helics_federate fed, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return helics_time_invalid;
    }
    auto T = fedObj->getCurrentTime();
    return (T < helics::Time::maxVal()) ? static_cast<double>(T) : helics_time_maxtime;
}

static constexpr char invalidGlobalString[] = "Global name cannot be null";
void helicsFederateSetGlobal(helics_federate fed, const char* valueName, const char* value, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    if (valueName == nullptr) {
        assignError(err, helics_error_invalid_argument, invalidGlobalString);
        return;
    }
    try {
        fedObj->setGlobal(valueName, AS_STRING(value));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

static constexpr char invalidFedNameString[] = "Federate name for dependency cannot be null";
void helicsFederateAddDependency(helics_federate fed, const char* fedName, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    if (fedName == nullptr) {
        assignError(err, helics_error_invalid_argument, invalidFedNameString);
        return;
    }
    try {
        fedObj->addDependency(fedName);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

static constexpr char invalidFederateCore[] = "Federate core is not connected";
void helicsFederateSetLogFile(helics_federate fed, const char* logFile, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    auto cr = fedObj->getCorePointer();

    try {
        if (cr) {
            cr->setLogFile(AS_STRING(logFile));
            // LCOV_EXCL_START
        } else {  // this can theoretically happen but it we be pretty odd
            assignError(err, helics_error_invalid_function_call, invalidFederateCore);
            return;
            // LCOV_EXCL_STOP
        }
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsFederateLogErrorMessage(helics_federate fed, const char* logmessage, helics_error* err)
{
    helicsFederateLogLevelMessage(fed, helics_log_level_error, logmessage, err);
}

void helicsFederateLogWarningMessage(helics_federate fed, const char* logmessage, helics_error* err)
{
    helicsFederateLogLevelMessage(fed, helics_log_level_warning, logmessage, err);
}

void helicsFederateLogInfoMessage(helics_federate fed, const char* logmessage, helics_error* err)
{
    helicsFederateLogLevelMessage(fed, helics_log_level_summary, logmessage, err);
}

void helicsFederateLogDebugMessage(helics_federate fed, const char* logmessage, helics_error* err)
{
    helicsFederateLogLevelMessage(fed, helics_log_level_data, logmessage, err);
}

void helicsFederateLogLevelMessage(helics_federate fed, int loglevel, const char* logmessage, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    fedObj->logMessage(loglevel, AS_STRING(logmessage));
}
