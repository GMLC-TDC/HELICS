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
#include "helicsCore.h"
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
FedObject* getFedObject(HelicsFederate fed, HelicsError* err) noexcept
{
    HELICS_ERROR_CHECK(err, nullptr);
    if (fed == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidFedString);
        return nullptr;
    }
    auto* fedObj = reinterpret_cast<helics::FedObject*>(fed);
    if (fedObj->valid == fedValidationIdentifier) {
        return fedObj;
    }
    assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidFedString);
    return nullptr;
}
}  // namespace helics

helics::Federate* getFed(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = helics::getFedObject(fed, err);
    return (fedObj == nullptr) ? nullptr : fedObj->fedptr.get();
}

static const char* notValueFedString = "Federate must be a value federate";

helics::ValueFederate* getValueFed(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = helics::getFedObject(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    if ((fedObj->type == helics::FederateType::VALUE) || (fedObj->type == helics::FederateType::COMBINATION)) {
        auto* rval = dynamic_cast<helics::ValueFederate*>(fedObj->fedptr.get());
        if (rval != nullptr) {
            return rval;
        }
    }
    assignError(err, HELICS_ERROR_INVALID_OBJECT, notValueFedString);
    return nullptr;
}

static const char* notMessageFedString = "Federate must be a message federate";

helics::MessageFederate* getMessageFed(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = helics::getFedObject(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    if ((fedObj->type == helics::FederateType::MESSAGE) || (fedObj->type == helics::FederateType::COMBINATION)) {
        auto* rval = dynamic_cast<helics::MessageFederate*>(fedObj->fedptr.get());
        if (rval != nullptr) {
            return rval;
        }
    }
    assignError(err, HELICS_ERROR_INVALID_OBJECT, notMessageFedString);
    return nullptr;
}

std::shared_ptr<helics::Federate> getFedSharedPtr(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = helics::getFedObject(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    return fedObj->fedptr;
}

std::shared_ptr<helics::ValueFederate> getValueFedSharedPtr(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = helics::getFedObject(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    if ((fedObj->type == helics::FederateType::VALUE) || (fedObj->type == helics::FederateType::COMBINATION)) {
        auto rval = std::dynamic_pointer_cast<helics::ValueFederate>(fedObj->fedptr);
        if (rval) {
            return rval;
        }
    }
    assignError(err, HELICS_ERROR_INVALID_OBJECT, notValueFedString);
    return nullptr;
}

std::shared_ptr<helics::MessageFederate> getMessageFedSharedPtr(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = helics::getFedObject(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    if ((fedObj->type == helics::FederateType::MESSAGE) || (fedObj->type == helics::FederateType::COMBINATION)) {
        auto rval = std::dynamic_pointer_cast<helics::MessageFederate>(fedObj->fedptr);
        if (rval) {
            return rval;
        }
    }
    assignError(err, HELICS_ERROR_INVALID_OBJECT, notMessageFedString);
    return nullptr;
}

// random integer for validation purposes of endpoints
static constexpr int FederateInfoValidationIdentifier = 0x6bfb'bce1;

HelicsFederateInfo helicsCreateFederateInfo()
{
    auto* fi = new helics::FederateInfo;
    fi->uniqueKey = FederateInfoValidationIdentifier;
    return reinterpret_cast<void*>(fi);
}

static const char* invalidFedInfoString = "helics Federate info object was not valid";

static helics::FederateInfo* getFedInfo(HelicsFederateInfo fi, HelicsError* err)
{
    if ((err != nullptr) && (err->error_code != 0)) {
        return nullptr;
    }
    if (fi == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidFedInfoString);
        return nullptr;
    }
    auto* ptr = reinterpret_cast<helics::FederateInfo*>(fi);
    if (ptr->uniqueKey != FederateInfoValidationIdentifier) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidFedInfoString);
        return nullptr;
    }
    return ptr;
}

HelicsFederateInfo helicsFederateInfoClone(HelicsFederateInfo fi, HelicsError* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return nullptr;
    }
    auto* fi_new = new helics::FederateInfo(*info);
    return reinterpret_cast<void*>(fi_new);
}

void helicsFederateInfoFree(HelicsFederateInfo fi)
{
    auto* info = getFedInfo(fi, nullptr);
    if (info == nullptr) {
        // fprintf(stderr, "The HelicsFederateInfo object is not valid\n");
        return;
    }
    info->uniqueKey = 0;
    delete info;
}

void helicsFederateInfoLoadFromArgs(HelicsFederateInfo fi, int argc, const char* const* argv, HelicsError* err)
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

void helicsFederateInfoSetCoreName(HelicsFederateInfo fi, const char* corename, HelicsError* err)
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

void helicsFederateInfoSetCoreInitString(HelicsFederateInfo fi, const char* coreinit, HelicsError* err)
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

void helicsFederateInfoSetBrokerInitString(HelicsFederateInfo fi, const char* brokerinit, HelicsError* err)
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

void helicsFederateInfoSetCoreType(HelicsFederateInfo fi, int coretype, HelicsError* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    info->coreType = static_cast<helics::CoreType>(coretype);
}

void helicsFederateInfoSetCoreTypeFromString(HelicsFederateInfo fi, const char* coretype, HelicsError* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    if (coretype == nullptr) {
        info->coreType = helics::CoreType::DEFAULT;
        return;
    }
    auto ctype = helics::core::coreTypeFromString(coretype);
    if (ctype == helics::CoreType::UNRECOGNIZED) {
        if (err != nullptr) {
            err->error_code = HELICS_ERROR_INVALID_ARGUMENT;
            err->message = getMasterHolder()->addErrorString(std::string(coretype) + " is not a valid core type");
            return;
        }
    }
    info->coreType = ctype;
}

void helicsFederateInfoSetBroker(HelicsFederateInfo fi, const char* broker, HelicsError* err)
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

void helicsFederateInfoSetBrokerKey(HelicsFederateInfo fi, const char* brokerkey, HelicsError* err)
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

void helicsFederateInfoSetBrokerPort(HelicsFederateInfo fi, int brokerPort, HelicsError* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    info->brokerPort = brokerPort;
}

void helicsFederateInfoSetLocalPort(HelicsFederateInfo fi, const char* localPort, HelicsError* err)
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

void helicsFederateInfoSetFlagOption(HelicsFederateInfo fi, int flag, HelicsBool value, HelicsError* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    info->setFlagOption(flag, (value != HELICS_FALSE));
}

void helicsFederateInfoSetTimeProperty(HelicsFederateInfo fi, int timeProperty, HelicsTime propertyValue, HelicsError* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    info->setProperty(timeProperty, propertyValue);
}

void helicsFederateInfoSetSeparator(HelicsFederateInfo fi, char separator, HelicsError* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    info->separator = separator;
}

void helicsFederateInfoSetIntegerProperty(HelicsFederateInfo fi, int integerProperty, int propertyValue, HelicsError* err)
{
    auto* info = getFedInfo(fi, err);
    if (info == nullptr) {
        return;
    }
    info->setProperty(integerProperty, propertyValue);
}

/* Creation and destruction of Federates */
HelicsFederate helicsCreateValueFederate(const char* fedName, HelicsFederateInfo fi, HelicsError* err)
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
    FedI->type = helics::FederateType::VALUE;
    FedI->valid = fedValidationIdentifier;
    auto* fed = reinterpret_cast<HelicsFederate>(FedI.get());
    getMasterHolder()->addFed(std::move(FedI));
    return (fed);
}

HelicsFederate helicsCreateValueFederateFromConfig(const char* configFile, HelicsError* err)
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
    FedI->type = helics::FederateType::VALUE;
    FedI->valid = fedValidationIdentifier;
    auto* fed = reinterpret_cast<HelicsFederate>(FedI.get());
    getMasterHolder()->addFed(std::move(FedI));
    return (fed);
}

/* Creation and destruction of Federates */
HelicsFederate helicsCreateMessageFederate(const char* fedName, HelicsFederateInfo fi, HelicsError* err)
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
    FedI->type = helics::FederateType::MESSAGE;
    FedI->valid = fedValidationIdentifier;
    auto* fed = reinterpret_cast<HelicsFederate>(FedI.get());
    getMasterHolder()->addFed(std::move(FedI));
    return (fed);
}

HelicsFederate helicsCreateMessageFederateFromConfig(const char* configFile, HelicsError* err)
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
    FedI->type = helics::FederateType::MESSAGE;
    FedI->valid = fedValidationIdentifier;
    auto* fed = reinterpret_cast<HelicsFederate>(FedI.get());
    getMasterHolder()->addFed(std::move(FedI));
    return (fed);
}

/* Creation and destruction of Federates */
HelicsFederate helicsCreateCombinationFederate(const char* fedName, HelicsFederateInfo fi, HelicsError* err)
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
    FedI->type = helics::FederateType::COMBINATION;
    FedI->valid = fedValidationIdentifier;
    auto* fed = reinterpret_cast<HelicsFederate>(FedI.get());
    getMasterHolder()->addFed(std::move(FedI));
    return (fed);
}

HelicsFederate helicsCreateCombinationFederateFromConfig(const char* configFile, HelicsError* err)
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

    FedI->type = helics::FederateType::COMBINATION;
    FedI->valid = fedValidationIdentifier;
    auto* fed = reinterpret_cast<HelicsFederate>(FedI.get());
    getMasterHolder()->addFed(std::move(FedI));
    return (fed);
}

HelicsFederate helicsFederateClone(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = helics::getFedObject(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    auto fedClone = std::make_unique<helics::FedObject>();
    fedClone->fedptr = fedObj->fedptr;

    fedClone->type = fedObj->type;
    fedClone->valid = fedObj->valid;
    auto* fedB = reinterpret_cast<HelicsFederate>(fedClone.get());
    getMasterHolder()->addFed(std::move(fedClone));
    return (fedB);
}

HelicsBool helicsFederateIsValid(HelicsFederate fed)
{
    auto* fedObj = getFed(fed, nullptr);
    return (fedObj == nullptr) ? HELICS_FALSE : HELICS_TRUE;
}

HelicsCore helicsFederateGetCore(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    auto core = std::make_unique<helics::CoreObject>();
    core->valid = gCoreValidationIdentifier;
    core->coreptr = fedObj->getCorePointer();
    auto* retcore = reinterpret_cast<HelicsCore>(core.get());
    getMasterHolder()->addCore(std::move(core));
    return retcore;
}

static constexpr char invalidFile[] = "Invalid File specification";

void helicsFederateRegisterInterfaces(HelicsFederate fed, const char* file, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    if (file == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidFile);
        return;
    }
    try {
        fedObj->registerInterfaces(file);
    }
    catch (...) {
        return helicsErrorHandler(err);
    }
}

void helicsFederateGlobalError(HelicsFederate fed, int errorCode, const char* errorString, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->globalError(errorCode, AS_STRING(errorString));
    }
    // LCOV_EXCL_START
    catch (...) {
        return helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsFederateLocalError(HelicsFederate fed, int errorCode, const char* errorString, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->localError(errorCode, AS_STRING(errorString));
    }
    // LCOV_EXCL_START
    catch (...) {
        return helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsFederateFinalize(HelicsFederate fed, HelicsError* err)
{
    helicsFederateDisconnect(fed, err);
}

void helicsFederateFinalizeAsync(HelicsFederate fed, HelicsError* err)
{
    helicsFederateDisconnectAsync(fed, err);
}

void helicsFederateFinalizeComplete(HelicsFederate fed, HelicsError* err)
{
    helicsFederateDisconnectComplete(fed, err);
}

void helicsFederateDisconnect(HelicsFederate fed, HelicsError* err)
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

void helicsFederateDisconnectAsync(HelicsFederate fed, HelicsError* err)
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

void helicsFederateDisconnectComplete(HelicsFederate fed, HelicsError* err)
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
void helicsFederateEnterInitializingMode(HelicsFederate fed, HelicsError* err)
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

void helicsFederateEnterInitializingModeAsync(HelicsFederate fed, HelicsError* err)
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

HelicsBool helicsFederateIsAsyncOperationCompleted(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return HELICS_FALSE;
    }
    return (fedObj->isAsyncOperationCompleted()) ? HELICS_TRUE : HELICS_FALSE;
}

void helicsFederateEnterInitializingModeComplete(HelicsFederate fed, HelicsError* err)
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

void helicsFederateEnterExecutingMode(HelicsFederate fed, HelicsError* err)
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

static helics::IterationRequest getIterationRequest(HelicsIterationRequest iterate)
{
    switch (iterate) {
        case HELICS_ITERATION_REQUEST_NO_ITERATION:
        default:
            return helics::IterationRequest::NO_ITERATIONS;
        case HELICS_ITERATION_REQUEST_FORCE_ITERATION:
            return helics::IterationRequest::FORCE_ITERATION;

        case HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED:
            return helics::IterationRequest::ITERATE_IF_NEEDED;
    }
}

static HelicsIterationResult getIterationStatus(helics::IterationResult iterationState)
{
    switch (iterationState) {
        case helics::IterationResult::NEXT_STEP:
            return HELICS_ITERATION_RESULT_NEXT_STEP;
        case helics::IterationResult::ITERATING:
            return HELICS_ITERATION_RESULT_ITERATING;
        case helics::IterationResult::ERROR_RESULT:
        default:
            // most cases of this return error directly without going through this function
            return HELICS_ITERATION_RESULT_ERROR;  // LCOV_EXCL_LINE
        case helics::IterationResult::HALTED:
            return HELICS_ITERATION_RESULT_HALTED;
    }
}

HelicsIterationResult helicsFederateEnterExecutingModeIterative(HelicsFederate fed, HelicsIterationRequest iterate, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return HELICS_ITERATION_RESULT_ERROR;
    }
    try {
        auto val = fedObj->enterExecutingMode(getIterationRequest(iterate));
        return getIterationStatus(val);
    }
    catch (...) {
        helicsErrorHandler(err);
        return HELICS_ITERATION_RESULT_ERROR;
    }
}

void helicsFederateEnterExecutingModeAsync(HelicsFederate fed, HelicsError* err)
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

void helicsFederateEnterExecutingModeIterativeAsync(HelicsFederate fed, HelicsIterationRequest iterate, HelicsError* err)
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

void helicsFederateEnterExecutingModeComplete(HelicsFederate fed, HelicsError* err)
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
HelicsIterationResult helicsFederateEnterExecutingModeIterativeComplete(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return HELICS_ITERATION_RESULT_ERROR;
    }
    try {
        auto val = fedObj->enterExecutingModeComplete();
        return getIterationStatus(val);
    }
    catch (...) {
        helicsErrorHandler(err);
        return HELICS_ITERATION_RESULT_ERROR;
    }
}

HelicsTime helicsFederateRequestTime(HelicsFederate fed, HelicsTime requestTime, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return HELICS_TIME_INVALID;
    }
    try {
        auto timeret = fedObj->requestTime(requestTime);
        return (timeret < helics::Time::maxVal()) ? static_cast<double>(timeret) : HELICS_TIME_MAXTIME;
    }
    catch (...) {
        helicsErrorHandler(err);
        return HELICS_TIME_INVALID;
    }
}

HelicsTime helicsFederateRequestTimeAdvance(HelicsFederate fed, HelicsTime timeDelta, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return HELICS_TIME_INVALID;
    }
    try {
        auto timeret = fedObj->requestTimeAdvance(timeDelta);
        return (timeret < helics::Time::maxVal()) ? static_cast<double>(timeret) : HELICS_TIME_MAXTIME;
    }
    catch (...) {
        helicsErrorHandler(err);
        return HELICS_TIME_INVALID;
    }
}

HelicsTime helicsFederateRequestNextStep(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return HELICS_TIME_INVALID;
    }
    try {
        auto timeret = fedObj->requestNextStep();
        return (timeret < helics::Time::maxVal()) ? static_cast<double>(timeret) : HELICS_TIME_MAXTIME;
    }
    catch (...) {
        helicsErrorHandler(err);
        return HELICS_TIME_INVALID;
    }
}

HelicsTime helicsFederateRequestTimeIterative(HelicsFederate fed,
                                              HelicsTime requestTime,
                                              HelicsIterationRequest iterate,
                                              HelicsIterationResult* outIteration,
                                              HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        if (outIteration != nullptr) {
            *outIteration = HELICS_ITERATION_RESULT_ERROR;
        }
        return HELICS_TIME_INVALID;
    }
    try {
        auto val = fedObj->requestTimeIterative(requestTime, getIterationRequest(iterate));
        if (outIteration != nullptr) {
            *outIteration = getIterationStatus(val.state);
        }
        return (val.grantedTime < helics::Time::maxVal()) ? static_cast<double>(val.grantedTime) : HELICS_TIME_MAXTIME;
    }
    catch (...) {
        helicsErrorHandler(err);
        if (outIteration != nullptr) {
            *outIteration = HELICS_ITERATION_RESULT_ERROR;
        }
        return HELICS_TIME_INVALID;
    }
}

void helicsFederateRequestTimeAsync(HelicsFederate fed, HelicsTime requestTime, HelicsError* err)
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

HelicsTime helicsFederateRequestTimeComplete(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return HELICS_TIME_INVALID;
    }
    try {
        auto timeret = fedObj->requestTimeComplete();
        return (timeret < helics::Time::maxVal()) ? static_cast<double>(timeret) : HELICS_TIME_MAXTIME;
    }
    catch (...) {
        helicsErrorHandler(err);
        return HELICS_TIME_INVALID;
    }
}

void helicsFederateRequestTimeIterativeAsync(HelicsFederate fed, HelicsTime requestTime, HelicsIterationRequest iterate, HelicsError* err)
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

HelicsTime helicsFederateRequestTimeIterativeComplete(HelicsFederate fed, HelicsIterationResult* outIteration, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        if (outIteration != nullptr) {
            *outIteration = HELICS_ITERATION_RESULT_ERROR;
        }
        return HELICS_TIME_INVALID;
    }
    try {
        auto val = fedObj->requestTimeIterativeComplete();
        if (outIteration != nullptr) {
            *outIteration = getIterationStatus(val.state);
        }
        return (val.grantedTime < helics::Time::maxVal()) ? static_cast<double>(val.grantedTime) : HELICS_TIME_MAXTIME;
    }
    catch (...) {
        helicsErrorHandler(err);
        if (outIteration != nullptr) {
            *outIteration = HELICS_ITERATION_RESULT_ERROR;
        }
        return HELICS_TIME_INVALID;
    }
}

static const std::map<helics::Federate::Modes, HelicsFederateState> modeEnumConversions{
    {helics::Federate::Modes::ERROR_STATE, HelicsFederateState::HELICS_STATE_ERROR},
    {helics::Federate::Modes::STARTUP, HelicsFederateState::HELICS_STATE_STARTUP},
    {helics::Federate::Modes::EXECUTING, HelicsFederateState::HELICS_STATE_EXECUTION},
    {helics::Federate::Modes::FINALIZE, HelicsFederateState::HELICS_STATE_FINALIZE},
    {helics::Federate::Modes::PENDING_EXEC, HelicsFederateState::HELICS_STATE_PENDING_EXEC},
    {helics::Federate::Modes::PENDING_INIT, HelicsFederateState::HELICS_STATE_PENDING_INIT},
    {helics::Federate::Modes::PENDING_ITERATIVE_TIME, HelicsFederateState::HELICS_STATE_PENDING_ITERATIVE_TIME},
    {helics::Federate::Modes::PENDING_TIME, HelicsFederateState::HELICS_STATE_PENDING_TIME},
    {helics::Federate::Modes::INITIALIZING, HelicsFederateState::HELICS_STATE_INITIALIZATION},
    {helics::Federate::Modes::PENDING_FINALIZE, HelicsFederateState::HELICS_STATE_PENDING_FINALIZE},
    {helics::Federate::Modes::FINISHED, HelicsFederateState::HELICS_STATE_FINISHED}};

HelicsFederateState helicsFederateGetState(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return HELICS_STATE_ERROR;
    }
    try {
        auto FedMode = fedObj->getCurrentMode();
        return modeEnumConversions.at(FedMode);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return HELICS_STATE_ERROR;
    }
    // LCOV_EXCL_STOP
}

const char* helicsFederateGetName(HelicsFederate fed)
{
    auto* fedObj = getFed(fed, nullptr);
    if (fedObj == nullptr) {
        return nullcstr;
    }
    const auto& ident = fedObj->getName();
    return ident.c_str();
}

void helicsFederateSetTimeProperty(HelicsFederate fed, int timeProperty, HelicsTime time, HelicsError* err)
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

void helicsFederateSetFlagOption(HelicsFederate fed, int flag, HelicsBool flagValue, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->setFlagOption(flag, (flagValue != HELICS_FALSE));
    }
    // LCOV_EXCL_START
    catch (...) {
        return helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsFederateSetIntegerProperty(HelicsFederate fed, int intProperty, int propVal, HelicsError* err)
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

HelicsTime helicsFederateGetTimeProperty(HelicsFederate fed, int timeProperty, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return HELICS_TIME_INVALID;
    }
    try {
        auto T = fedObj->getTimeProperty(timeProperty);

        return (T < helics::Time::maxVal()) ? static_cast<double>(T) : HELICS_TIME_MAXTIME;
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return HELICS_TIME_INVALID;
    }
    // LCOV_EXCL_STOP
}

HelicsBool helicsFederateGetFlagOption(HelicsFederate fed, int flag, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return HELICS_FALSE;
    }
    try {
        bool res = fedObj->getFlagOption(flag);
        return (res) ? HELICS_TRUE : HELICS_FALSE;
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return HELICS_FALSE;
    }
    // LCOV_EXCL_STOP
}

int helicsFederateGetIntegerProperty(HelicsFederate fed, int intProperty, HelicsError* err)
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

void helicsFederateSetSeparator(HelicsFederate fed, char separator, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    fedObj->setSeparator(separator);
}

HelicsTime helicsFederateGetCurrentTime(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return HELICS_TIME_INVALID;
    }
    auto T = fedObj->getCurrentTime();
    return (T < helics::Time::maxVal()) ? static_cast<double>(T) : HELICS_TIME_MAXTIME;
}

static constexpr char invalidGlobalString[] = "Global name cannot be null";
void helicsFederateSetGlobal(HelicsFederate fed, const char* valueName, const char* value, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    if (valueName == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidGlobalString);
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

static constexpr char invalidTagString[] = "Tag name cannot be null";
void helicsFederateSetTag(HelicsFederate fed, const char* tagName, const char* value, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    if (tagName == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidTagString);
        return;
    }
    try {
        fedObj->setTag(tagName, AS_STRING(value));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

const char* helicsFederateGetTag(HelicsFederate fed, const char* tagName, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return nullcstr;
    }
    if (tagName == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidTagString);
        return nullcstr;
    }
    try {
        const auto& str = fedObj->getTag(tagName);
        return str.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return nullcstr;
    }
    // LCOV_EXCL_STOP
}

static constexpr char invalidFedNameString[] = "Federate name for dependency cannot be null";
void helicsFederateAddDependency(HelicsFederate fed, const char* fedName, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    if (fedName == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidFedNameString);
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
void helicsFederateSetLogFile(HelicsFederate fed, const char* logFile, HelicsError* err)
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
        } else {  // this can theoretically happen but it would be pretty odd
            assignError(err, HELICS_ERROR_INVALID_FUNCTION_CALL, invalidFederateCore);
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

void helicsFederateLogErrorMessage(HelicsFederate fed, const char* logmessage, HelicsError* err)
{
    helicsFederateLogLevelMessage(fed, HELICS_LOG_LEVEL_ERROR, logmessage, err);
}

void helicsFederateLogWarningMessage(HelicsFederate fed, const char* logmessage, HelicsError* err)
{
    helicsFederateLogLevelMessage(fed, HELICS_LOG_LEVEL_WARNING, logmessage, err);
}

void helicsFederateLogInfoMessage(HelicsFederate fed, const char* logmessage, HelicsError* err)
{
    helicsFederateLogLevelMessage(fed, HELICS_LOG_LEVEL_SUMMARY, logmessage, err);
}

void helicsFederateLogDebugMessage(HelicsFederate fed, const char* logmessage, HelicsError* err)
{
    helicsFederateLogLevelMessage(fed, HELICS_LOG_LEVEL_DATA, logmessage, err);
}

void helicsFederateLogLevelMessage(HelicsFederate fed, int loglevel, const char* logmessage, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    fedObj->logMessage(loglevel, AS_STRING(logmessage));
}

void helicsFederateSendCommand(HelicsFederate fed, const char* target, const char* command, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    fedObj->sendCommand(AS_STRING(target), AS_STRING(command));
}

const char* helicsFederateGetCommand(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = helics::getFedObject(fed, err);

    if (fedObj == nullptr) {
        return gEmptyStr.c_str();
    }
    auto res = fedObj->fedptr->getCommand();
    if (res.first.empty()) {
        return gEmptyStr.c_str();
    }
    fedObj->commandBuffer = std::move(res);
    return fedObj->commandBuffer.first.c_str();
}

const char* helicsFederateGetCommandSource(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = helics::getFedObject(fed, err);

    if (fedObj == nullptr) {
        return gEmptyStr.c_str();
    }
    return fedObj->commandBuffer.second.c_str();
}

const char* helicsFederateWaitCommand(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = helics::getFedObject(fed, err);

    if (fedObj == nullptr) {
        return gEmptyStr.c_str();
    }
    auto res = fedObj->fedptr->waitCommand();
    if (res.first.empty()) {
        return gEmptyStr.c_str();
    }
    fedObj->commandBuffer = std::move(res);
    return fedObj->commandBuffer.first.c_str();
}
