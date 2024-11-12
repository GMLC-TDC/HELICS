/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../application_api/FederateInfo.hpp"
#include "../core/CoreTypes.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/coreTypeOperations.hpp"
#include "../helics.hpp"
#include "api-data.h"
#include "gmlc/concurrency/TripWire.hpp"
#include "helics/helics_enums.h"
#include "helicsCallbacks.h"
#include "helicsCore.h"
#include "internal/api_objects.h"

#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

/** this is a random identifier put in place when the federate or core or broker gets created*/
static constexpr int fedValidationIdentifier = 0x235'2188;
static constexpr int fedPreservationIdentifier = 0x235'2185;
static constexpr const char* invalidFedString = "federate object is not valid";

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

static constexpr const char* notValueFedString = "Federate must be a value federate";

helics::ValueFederate* getValueFed(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = helics::getFedObject(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    if ((fedObj->type == helics::FederateType::VALUE) || (fedObj->type == helics::FederateType::COMBINATION) ||
        (fedObj->type == helics::FederateType::CALLBACK)) {
        auto* rval = dynamic_cast<helics::ValueFederate*>(fedObj->fedptr.get());
        if (rval != nullptr) {
            return rval;
        }
    }
    assignError(err, HELICS_ERROR_INVALID_OBJECT, notValueFedString);
    return nullptr;
}

static constexpr const char* notMessageFedString = "Federate must be a message federate";

helics::MessageFederate* getMessageFed(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = helics::getFedObject(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    if ((fedObj->type == helics::FederateType::MESSAGE) || (fedObj->type == helics::FederateType::COMBINATION) ||
        (fedObj->type == helics::FederateType::CALLBACK)) {
        auto* rval = dynamic_cast<helics::MessageFederate*>(fedObj->fedptr.get());
        if (rval != nullptr) {
            return rval;
        }
    }
    assignError(err, HELICS_ERROR_INVALID_OBJECT, notMessageFedString);
    return nullptr;
}

static constexpr const char* notCallbackFedString = "Federate must be a callback federate";

helics::CallbackFederate* getCallbackFed(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = helics::getFedObject(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    if (fedObj->type == helics::FederateType::CALLBACK) {
        auto* rval = dynamic_cast<helics::CallbackFederate*>(fedObj->fedptr.get());
        if (rval != nullptr) {
            return rval;
        }
    }
    assignError(err, HELICS_ERROR_INVALID_OBJECT, notCallbackFedString);
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
    if ((fedObj->type == helics::FederateType::VALUE) || (fedObj->type == helics::FederateType::COMBINATION) ||
        (fedObj->type == helics::FederateType::CALLBACK)) {
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
    if ((fedObj->type == helics::FederateType::MESSAGE) || (fedObj->type == helics::FederateType::COMBINATION) ||
        (fedObj->type == helics::FederateType::CALLBACK)) {
        auto rval = std::dynamic_pointer_cast<helics::MessageFederate>(fedObj->fedptr);
        if (rval) {
            return rval;
        }
    }
    assignError(err, HELICS_ERROR_INVALID_OBJECT, notMessageFedString);
    return nullptr;
}

std::shared_ptr<helics::CallbackFederate> getCallbackFedSharedPtr(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = helics::getFedObject(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    if (fedObj->type == helics::FederateType::CALLBACK) {
        auto rval = std::dynamic_pointer_cast<helics::CallbackFederate>(fedObj->fedptr);
        if (rval) {
            return rval;
        }
    }
    assignError(err, HELICS_ERROR_INVALID_OBJECT, notCallbackFedString);
    return nullptr;
}

// random integer for validation purposes of endpoints
static constexpr int FederateInfoValidationIdentifier = 0x6bfb'bce1;

HelicsFederateInfo helicsCreateFederateInfo()
{
    auto* fedInfo = new helics::FederateInfo;
    fedInfo->uniqueKey = FederateInfoValidationIdentifier;
    return reinterpret_cast<void*>(fedInfo);
}

static constexpr const char* invalidFedInfoString = "helics Federate info object was not valid";

helics::FederateInfo* getFedInfo(HelicsFederateInfo fedInfo, HelicsError* err)
{
    if ((err != nullptr) && (err->error_code != 0)) {
        return nullptr;
    }
    if (fedInfo == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidFedInfoString);
        return nullptr;
    }
    auto* ptr = reinterpret_cast<helics::FederateInfo*>(fedInfo);
    if (ptr->uniqueKey != FederateInfoValidationIdentifier) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidFedInfoString);
        return nullptr;
    }
    return ptr;
}

HelicsFederateInfo helicsFederateInfoClone(HelicsFederateInfo fedInfo, HelicsError* err)
{
    auto* info = getFedInfo(fedInfo, err);
    if (info == nullptr) {
        return nullptr;
    }
    auto* fi_new = new helics::FederateInfo(*info);
    return reinterpret_cast<void*>(fi_new);
}

void helicsFederateInfoFree(HelicsFederateInfo fedInfo)
{
    auto* info = getFedInfo(fedInfo, nullptr);
    if (info == nullptr) {
        // fprintf(stderr, "The HelicsFederateInfo object is not valid\n");
        return;
    }
    info->uniqueKey = 0;
    delete info;
}

void helicsFederateInfoLoadFromArgs(HelicsFederateInfo fedInfo, int argc, const char* const* argv, HelicsError* err)
{
    auto* info = getFedInfo(fedInfo, err);
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
        helicsErrorHandler(err);
    }
}

void helicsFederateInfoLoadFromString(HelicsFederateInfo fedInfo, const char* args, HelicsError* err)
{
    auto* info = getFedInfo(fedInfo, err);
    if (info == nullptr) {
        return;
    }
    try {
        info->loadInfoFromArgs(args);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsFederateInfoSetCoreName(HelicsFederateInfo fedInfo, const char* corename, HelicsError* err)
{
    auto* info = getFedInfo(fedInfo, err);
    if (info == nullptr) {
        return;
    }
    try {
        info->coreName = AS_STRING(corename);
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateInfoSetCoreInitString(HelicsFederateInfo fedInfo, const char* coreinit, HelicsError* err)
{
    auto* info = getFedInfo(fedInfo, err);
    if (info == nullptr) {
        return;
    }
    try {
        info->coreInitString = AS_STRING(coreinit);
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateInfoSetBrokerInitString(HelicsFederateInfo fedInfo, const char* brokerinit, HelicsError* err)
{
    auto* info = getFedInfo(fedInfo, err);
    if (info == nullptr) {
        return;
    }
    try {
        info->brokerInitString = AS_STRING(brokerinit);
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateInfoSetCoreType(HelicsFederateInfo fedInfo, int coretype, HelicsError* err)
{
    auto* info = getFedInfo(fedInfo, err);
    if (info == nullptr) {
        return;
    }
    info->coreType = static_cast<helics::CoreType>(coretype);
}

void helicsFederateInfoSetCoreTypeFromString(HelicsFederateInfo fedInfo, const char* coretype, HelicsError* err)
{
    auto* info = getFedInfo(fedInfo, err);
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

void helicsFederateInfoSetBroker(HelicsFederateInfo fedInfo, const char* broker, HelicsError* err)
{
    auto* info = getFedInfo(fedInfo, err);
    if (info == nullptr) {
        return;
    }
    try {
        info->broker = AS_STRING(broker);
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateInfoSetBrokerKey(HelicsFederateInfo fedInfo, const char* brokerkey, HelicsError* err)
{
    auto* info = getFedInfo(fedInfo, err);
    if (info == nullptr) {
        return;
    }
    try {
        info->key = AS_STRING(brokerkey);
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateInfoSetBrokerPort(HelicsFederateInfo fedInfo, int brokerPort, HelicsError* err)
{
    auto* info = getFedInfo(fedInfo, err);
    if (info == nullptr) {
        return;
    }
    info->brokerPort = brokerPort;
}

void helicsFederateInfoSetLocalPort(HelicsFederateInfo fedInfo, const char* localPort, HelicsError* err)
{
    auto* info = getFedInfo(fedInfo, err);
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

int helicsGetDataType(const char* val)
{
    if (val == nullptr) {
        return -1;
    }
    return static_cast<int>(helics::getTypeFromString(val));
}

void helicsFederateInfoSetFlagOption(HelicsFederateInfo fedInfo, int flag, HelicsBool value, HelicsError* err)
{
    auto* info = getFedInfo(fedInfo, err);
    if (info == nullptr) {
        return;
    }
    switch (flag) {
        case HELICS_FLAG_OBSERVER:
            info->observer = (value != HELICS_FALSE);
            break;
        case HELICS_FLAG_DEBUGGING:
            info->debugging = (value != HELICS_FALSE);
            break;
        case HELICS_FLAG_USE_JSON_SERIALIZATION:
            info->useJsonSerialization = (value != HELICS_FALSE);
            break;
        default:
            break;
    }

    info->setFlagOption(flag, (value != HELICS_FALSE));
}

void helicsFederateInfoSetTimeProperty(HelicsFederateInfo fedInfo, int timeProperty, HelicsTime propertyValue, HelicsError* err)
{
    auto* info = getFedInfo(fedInfo, err);
    if (info == nullptr) {
        return;
    }
    info->setProperty(timeProperty, propertyValue);
}

void helicsFederateInfoSetSeparator(HelicsFederateInfo fedInfo, char separator, HelicsError* err)
{
    auto* info = getFedInfo(fedInfo, err);
    if (info == nullptr) {
        return;
    }
    info->separator = separator;
}

void helicsFederateInfoSetIntegerProperty(HelicsFederateInfo fedInfo, int integerProperty, int propertyValue, HelicsError* err)
{
    auto* info = getFedInfo(fedInfo, err);
    if (info == nullptr) {
        return;
    }
    info->setProperty(integerProperty, propertyValue);
}

HelicsFederate generateNewHelicsFederateObject(std::shared_ptr<helics::Federate> fed, helics::FederateType type)
{
    auto fedI = std::make_unique<helics::FedObject>();
    fedI->fedptr = std::move(fed);
    fedI->type = type;
    fedI->valid = fedValidationIdentifier;
    auto* hfed = reinterpret_cast<HelicsFederate>(fedI.get());
    getMasterHolder()->addFed(std::move(fedI));
    return (hfed);
}

/* Creation and destruction of Federates */
HelicsFederate helicsCreateValueFederate(const char* fedName, HelicsFederateInfo fedInfo, HelicsError* err)
{
    HELICS_ERROR_CHECK(err, nullptr);

    std::shared_ptr<helics::Federate> fed;
    try {
        if (fedInfo == nullptr) {
            fed = std::make_shared<helics::ValueFederate>(AS_STRING(fedName), helics::FederateInfo());
        } else {
            auto* info = getFedInfo(fedInfo, err);
            if (info == nullptr) {
                return nullptr;
            }
            fed = std::make_shared<helics::ValueFederate>(AS_STRING(fedName), *info);
        }
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    return generateNewHelicsFederateObject(std::move(fed), helics::FederateType::VALUE);
}

HelicsFederate helicsCreateValueFederateFromConfig(const char* configFile, HelicsError* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    CHECK_NULL_STRING(configFile, nullptr);
    std::shared_ptr<helics::Federate> fed;
    try {
        fed = std::make_shared<helics::ValueFederate>(AS_STRING(configFile));
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    return generateNewHelicsFederateObject(std::move(fed), helics::FederateType::VALUE);
}

/* Creation and destruction of Federates */
HelicsFederate helicsCreateMessageFederate(const char* fedName, HelicsFederateInfo fedInfo, HelicsError* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    std::shared_ptr<helics::Federate> fed;
    try {
        if (fedInfo == nullptr) {
            fed = std::make_shared<helics::MessageFederate>(AS_STRING(fedName), helics::FederateInfo());
        } else {
            auto* info = getFedInfo(fedInfo, err);
            if (info == nullptr) {
                return nullptr;
            }
            fed = std::make_shared<helics::MessageFederate>(AS_STRING(fedName), *info);
        }
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    return generateNewHelicsFederateObject(std::move(fed), helics::FederateType::MESSAGE);
}

HelicsFederate helicsCreateMessageFederateFromConfig(const char* configFile, HelicsError* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    CHECK_NULL_STRING(configFile, nullptr);
    std::shared_ptr<helics::Federate> fed;

    try {
        fed = std::make_shared<helics::MessageFederate>(AS_STRING(configFile));
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    return generateNewHelicsFederateObject(std::move(fed), helics::FederateType::MESSAGE);
}

/* Creation and destruction of Federates */
HelicsFederate helicsCreateCombinationFederate(const char* fedName, HelicsFederateInfo fedInfo, HelicsError* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    std::shared_ptr<helics::Federate> fed;
    try {
        if (fedInfo == nullptr) {
            fed = std::make_shared<helics::CombinationFederate>(AS_STRING(fedName), helics::FederateInfo());
        } else {
            auto* info = getFedInfo(fedInfo, err);
            if (info == nullptr) {
                return nullptr;
            }
            fed = std::make_shared<helics::CombinationFederate>(AS_STRING(fedName), *info);
        }
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    return generateNewHelicsFederateObject(std::move(fed), helics::FederateType::COMBINATION);
}

HelicsFederate helicsCreateCombinationFederateFromConfig(const char* configFile, HelicsError* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    CHECK_NULL_STRING(configFile, nullptr);
    std::shared_ptr<helics::Federate> fed;
    try {
        fed = std::make_shared<helics::CombinationFederate>(AS_STRING(configFile));
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }

    return generateNewHelicsFederateObject(std::move(fed), helics::FederateType::COMBINATION);
}

/* Creation and destruction of Federates */
HelicsFederate helicsCreateCallbackFederate(const char* fedName, HelicsFederateInfo fedInfo, HelicsError* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    std::shared_ptr<helics::Federate> fed;
    try {
        if (fedInfo == nullptr) {
            fed = std::make_shared<helics::CallbackFederate>(AS_STRING(fedName), helics::FederateInfo());
        } else {
            auto* info = getFedInfo(fedInfo, err);
            if (info == nullptr) {
                return nullptr;
            }
            fed = std::make_shared<helics::CallbackFederate>(AS_STRING(fedName), *info);
        }
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    return generateNewHelicsFederateObject(std::move(fed), helics::FederateType::CALLBACK);
}

HelicsFederate helicsCreateCallbackFederateFromConfig(const char* configFile, HelicsError* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    std::shared_ptr<helics::Federate> fed;
    try {
        fed = std::make_shared<helics::CallbackFederate>(AS_STRING(configFile));
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }

    return generateNewHelicsFederateObject(std::move(fed), helics::FederateType::CALLBACK);
}

HelicsFederate helicsFederateClone(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = helics::getFedObject(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    return generateNewHelicsFederateObject(fedObj->fedptr, fedObj->type);
}

HelicsFederate helicsGetFederateByName(const char* fedName, HelicsError* err)
{
    if ((err != nullptr) && (err->error_code != 0)) {
        return nullptr;
    }
    if (fedName == nullptr) {
        if (err != nullptr) {
            err->error_code = HELICS_ERROR_INVALID_ARGUMENT;
            err->message = getMasterHolder()->addErrorString("fedName is empty");
        }
        return nullptr;
    }
    auto mob = getMasterHolder();
    auto* fed = mob->findFed(fedName);
    if (fed == nullptr) {
        if (err != nullptr) {
            err->error_code = HELICS_ERROR_INVALID_ARGUMENT;
            err->message = getMasterHolder()->addErrorString(std::string(fedName) + " is not an active federate identifier");
        }
        return nullptr;
    }
    return generateNewHelicsFederateObject(fed->fedptr, fed->type);
}

void helicsFederateProtect(const char* fedName, HelicsError* err)
{
    HelicsFederate newFed = helicsGetFederateByName(fedName, err);
    auto* fedObj = helics::getFedObject(newFed, err);
    if (fedObj == nullptr) {
        return;
    }
    fedObj->valid = fedPreservationIdentifier;
}

static constexpr const char* unrecognizedFederate = "Federate was not found";
void helicsFederateUnProtect(const char* fedName, HelicsError* err)
{
    const bool result = getMasterHolder()->removeFed(fedName, fedPreservationIdentifier);
    if (!result) {
        if (!(getMasterHolder()->findFed(fedName) != nullptr)) {
            if (err != nullptr) {
                if (err->error_code == 0) {
                    err->error_code = HELICS_ERROR_INVALID_OBJECT;
                    err->message = unrecognizedFederate;
                }
            }
        }
    }
}

HelicsBool helicsFederateIsProtected(const char* fedName, HelicsError* err)
{
    auto* fed = getMasterHolder()->findFed(fedName, fedPreservationIdentifier);
    if (fed != nullptr) {
        return HELICS_TRUE;
    }
    if (!(getMasterHolder()->findFed(fedName) != nullptr)) {
        if (err != nullptr) {
            if (err->error_code == 0) {
                err->error_code = HELICS_ERROR_INVALID_OBJECT;
                err->message = unrecognizedFederate;
            }
        }
    }
    return HELICS_FALSE;
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
        helicsErrorHandler(err);
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
        helicsErrorHandler(err);
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
        helicsErrorHandler(err);
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
        helicsErrorHandler(err);
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
        helicsErrorHandler(err);
    }
}

HelicsBool helicsFederateIsAsyncOperationCompleted(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return HELICS_FALSE;
    }
    try {
        return (fedObj->isAsyncOperationCompleted()) ? HELICS_TRUE : HELICS_FALSE;
    }
    catch (...) {
        helicsErrorHandler(err);
        return HELICS_FALSE;
    }
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
        helicsErrorHandler(err);
    }
}

void helicsFederateEnterInitializingModeIterative(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->enterInitializingModeIterative();
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsFederateEnterInitializingModeIterativeAsync(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->enterInitializingModeIterativeAsync();
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsFederateEnterInitializingModeIterativeComplete(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->enterInitializingModeIterativeComplete();
    }
    catch (...) {
        helicsErrorHandler(err);
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
        helicsErrorHandler(err);
    }
}

namespace {
helics::IterationRequest getIterationRequest(HelicsIterationRequest iterate)
{
    switch (iterate) {
        case HELICS_ITERATION_REQUEST_NO_ITERATION:
        default:
            return helics::IterationRequest::NO_ITERATIONS;
        case HELICS_ITERATION_REQUEST_FORCE_ITERATION:
            return helics::IterationRequest::FORCE_ITERATION;

        case HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED:
            return helics::IterationRequest::ITERATE_IF_NEEDED;
        case HELICS_ITERATION_REQUEST_HALT_OPERATIONS:
            return helics::IterationRequest::HALT_OPERATIONS;  // LCOV_EXCL_LINE
        case HELICS_ITERATION_REQUEST_ERROR:
            return helics::IterationRequest::ERROR_CONDITION;  // LCOV_EXCL_LINE
    }
}

HelicsIterationResult getIterationStatus(helics::IterationResult iterationState)
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
}  // namespace
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
        helicsErrorHandler(err);
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
        helicsErrorHandler(err);
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
        helicsErrorHandler(err);
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
        helicsErrorHandler(err);
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

void helicsFederateProcessCommunications(HelicsFederate fed, HelicsTime period, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    try {
        fedObj->processCommunication(helics::Time(period).to_ms());
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

namespace {
HelicsFederateState stateConversion(helics::Federate::Modes mode)
{
    return static_cast<HelicsFederateState>(static_cast<int32_t>(static_cast<std::underlying_type<helics::Federate::Modes>::type>(mode)));
}
}  // namespace

HelicsFederateState helicsFederateGetState(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return HELICS_STATE_UNKNOWN;
    }

    auto fedMode = fedObj->getCurrentMode();
    return stateConversion(fedMode);
}

void helicsFederateSetTimeRequestEntryCallback(
    HelicsFederate fed,
    void (*requestTimeEntry)(HelicsTime currentTime, HelicsTime requestTime, HelicsBool iterating, void* userdata),
    void* userdata,
    HelicsError* err)
{
    auto* fedptr = getFed(fed, err);
    if (fedptr == nullptr) {
        return;
    }

    try {
        if (requestTimeEntry == nullptr) {
            fedptr->setTimeRequestEntryCallback({});
        } else {
            fedptr->setTimeRequestEntryCallback(
                [requestTimeEntry, userdata](helics::Time currentTime, helics::Time requestTime, bool iterating) {
                    requestTimeEntry(currentTime, requestTime, (iterating) ? HELICS_TRUE : HELICS_FALSE, userdata);
                });
        }
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateSetStateChangeCallback(HelicsFederate fed,
                                          void (*stateChange)(HelicsFederateState newState, HelicsFederateState oldState, void* userdata),
                                          void* userdata,
                                          HelicsError* err)
{
    auto* fedptr = getFed(fed, err);
    if (fedptr == nullptr) {
        return;
    }

    try {
        if (stateChange == nullptr) {
            fedptr->setModeUpdateCallback({});
        } else {
            fedptr->setModeUpdateCallback([stateChange, userdata](helics::Federate::Modes newMode, helics::Federate::Modes oldMode) {
                stateChange(stateConversion(newMode), stateConversion(oldMode), userdata);
            });
        }
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateInitializingEntryCallback(HelicsFederate fed,
                                             void (*initializingEntry)(HelicsBool iterating, void* userdata),
                                             void* userdata,
                                             HelicsError* err)
{
    auto* fedptr = getFed(fed, err);
    if (fedptr == nullptr) {
        return;
    }

    try {
        if (initializingEntry == nullptr) {
            fedptr->setInitializingEntryCallback({});
        } else {
            fedptr->setInitializingEntryCallback(
                [initializingEntry, userdata](bool iterating) { initializingEntry(iterating ? HELICS_TRUE : HELICS_FALSE, userdata); });
        }
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateExecutingEntryCallback(HelicsFederate fed, void (*executingEntry)(void* userdata), void* userdata, HelicsError* err)
{
    auto* fedptr = getFed(fed, err);
    if (fedptr == nullptr) {
        return;
    }

    try {
        if (executingEntry == nullptr) {
            fedptr->setExecutingEntryCallback({});
        } else {
            fedptr->setExecutingEntryCallback([executingEntry, userdata]() { executingEntry(userdata); });
        }
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateCosimulationTerminationCallback(HelicsFederate fed,
                                                   void (*cosimTermination)(void* userdata),
                                                   void* userdata,
                                                   HelicsError* err)
{
    auto* fedptr = getFed(fed, err);
    if (fedptr == nullptr) {
        return;
    }

    try {
        if (cosimTermination == nullptr) {
            fedptr->setCosimulationTerminatedCallback({});
        } else {
            fedptr->setCosimulationTerminatedCallback([cosimTermination, userdata]() { cosimTermination(userdata); });
        }
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateErrorHandlerCallback(HelicsFederate fed,
                                        void (*errorHandler)(int errorCode, const char* errorString, void* userdata),
                                        void* userdata,
                                        HelicsError* err)
{
    auto* fedptr = getFed(fed, err);
    if (fedptr == nullptr) {
        return;
    }

    try {
        if (errorHandler == nullptr) {
            fedptr->setErrorHandlerCallback({});
        } else {
            fedptr->setErrorHandlerCallback([errorHandler, userdata](int errorCode, std::string_view errorMessage) {
                // string is to ensure we have a null terminator
                const std::string eMessage(errorMessage);
                errorHandler(errorCode, eMessage.c_str(), userdata);
            });
        }
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsFederateSetTimeRequestReturnCallback(HelicsFederate fed,
                                                void (*requestTimeReturn)(HelicsTime newTime, HelicsBool iterating, void* userdata),
                                                void* userdata,
                                                HelicsError* err)
{
    auto* fedptr = getFed(fed, err);
    if (fedptr == nullptr) {
        return;
    }

    try {
        if (requestTimeReturn == nullptr) {
            fedptr->setTimeRequestReturnCallback({});
        } else {
            fedptr->setTimeRequestReturnCallback([requestTimeReturn, userdata](helics::Time newTime, bool iterating) {
                requestTimeReturn(newTime, (iterating) ? HELICS_TRUE : HELICS_FALSE, userdata);
            });
        }
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsCallbackFederateNextTimeCallback(HelicsFederate fed,
                                            HelicsTime (*timeUpdate)(HelicsTime time, void* userdata),
                                            void* userdata,
                                            HelicsError* err)
{
    auto* fedptr = getCallbackFed(fed, err);
    if (fedptr == nullptr) {
        return;
    }

    try {
        if (timeUpdate == nullptr) {
            fedptr->clearNextTimeCallback();
        } else {
            fedptr->setNextTimeCallback([timeUpdate, userdata](helics::Time newTime) { return timeUpdate(newTime, userdata); });
        }
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsCallbackFederateNextTimeIterativeCallback(
    HelicsFederate fed,
    HelicsTime (*timeUpdate)(HelicsTime time, HelicsIterationResult, HelicsIterationRequest* iteration, void* userdata),
    void* userdata,
    HelicsError* err)
{
    auto* fedptr = getCallbackFed(fed, err);
    if (fedptr == nullptr) {
        return;
    }

    try {
        if (timeUpdate == nullptr) {
            fedptr->clearNextTimeCallback();
        } else {
            fedptr->setNextTimeIterativeCallback([timeUpdate, userdata](helics::iteration_time time) {
                HelicsIterationRequest request{HELICS_ITERATION_REQUEST_ERROR};
                const helics::Time newTime = timeUpdate(time.grantedTime, getIterationStatus(time.state), &request, userdata);
                return std::make_pair(newTime, getIterationRequest(request));
            });
        }
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}

void helicsCallbackFederateInitializeCallback(HelicsFederate fed,
                                              HelicsIterationRequest (*initialize)(void* userdata),
                                              void* userdata,
                                              HelicsError* err)
{
    auto* fedptr = getCallbackFed(fed, err);
    if (fedptr == nullptr) {
        return;
    }

    try {
        if (initialize == nullptr) {
            fedptr->setInitializeCallback({});
        } else {
            fedptr->setInitializeCallback([initialize, userdata]() { return getIterationRequest(initialize(userdata)); });
        }
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
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
        helicsErrorHandler(err);
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
        helicsErrorHandler(err);
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
        helicsErrorHandler(err);
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
        auto timeprop = fedObj->getTimeProperty(timeProperty);

        return (timeprop < helics::Time::maxVal()) ? static_cast<double>(timeprop) : HELICS_TIME_MAXTIME;
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
        const bool res = fedObj->getFlagOption(flag);
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
        return HELICS_INVALID_OPTION_INDEX;
    }
    try {
        return fedObj->getIntegerProperty(intProperty);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return HELICS_INVALID_OPTION_INDEX;
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
    auto time = fedObj->getCurrentTime();
    return (time < helics::Time::maxVal()) ? static_cast<double>(time) : HELICS_TIME_MAXTIME;
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

static constexpr char invalidInterfaceName[] = "Interface name cannot be empty";
static constexpr char invalidAliasName[] = "Alias cannot be empty";

void helicsFederateAddAlias(HelicsFederate fed, const char* interfaceName, const char* alias, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    if (interfaceName == nullptr || interfaceName[0] == '\0') {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidInterfaceName);
        return;
    }
    if (alias == nullptr || alias[0] == '\0') {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidAliasName);
        return;
    }
    try {
        fedObj->addAlias(interfaceName, alias);
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
    const auto& corePtr = fedObj->getCorePointer();

    try {
        if (corePtr) {
            corePtr->setLogFile(AS_STRING(logFile));
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
    helicsFederateLogLevelMessage(fed, HELICS_LOG_LEVEL_DEBUG, logmessage, err);
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
        return gHelicsEmptyStr.c_str();
    }
    auto res = fedObj->fedptr->getCommand();
    if (res.first.empty()) {
        return gHelicsEmptyStr.c_str();
    }
    fedObj->commandBuffer = std::move(res);
    return fedObj->commandBuffer.first.c_str();
}

const char* helicsFederateGetCommandSource(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = helics::getFedObject(fed, err);

    if (fedObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }
    return fedObj->commandBuffer.second.c_str();
}

const char* helicsFederateWaitCommand(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = helics::getFedObject(fed, err);

    if (fedObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }
    auto res = fedObj->fedptr->waitCommand();
    if (res.first.empty()) {
        return gHelicsEmptyStr.c_str();
    }
    fedObj->commandBuffer = std::move(res);
    return fedObj->commandBuffer.first.c_str();
}
