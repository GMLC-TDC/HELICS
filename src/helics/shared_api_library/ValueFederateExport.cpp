/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "../core/core-exceptions.hpp"
#include "../helics.hpp"
#include "ValueFederate.h"
#include "helicsData.h"
#include "internal/api_objects.h"

#include <algorithm>
#include <cstring>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace {
/** random integer for validation purposes of inputs */
constexpr int InputValidationIdentifier = 0x3456'E052;

/** random integer for validation purposes of publications */
constexpr int PublicationValidationIdentifier = 0x97B1'00A5;

constexpr char invalidInputString[] = "The given input object does not point to a valid object";

constexpr char invalidPublicationString[] = "The given publication object does not point to a valid object";

helics::InputObject* verifyInput(HelicsInput inp, HelicsError* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    if (inp == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidInputString);
        return nullptr;
    }
    auto* inpObj = reinterpret_cast<helics::InputObject*>(inp);
    if (inpObj->valid != InputValidationIdentifier) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidInputString);
        return nullptr;
    }
    return inpObj;
}

helics::PublicationObject* verifyPublication(HelicsPublication pub, HelicsError* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    if (pub == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidPublicationString);
        return nullptr;
    }
    auto* pubObj = reinterpret_cast<helics::PublicationObject*>(pub);
    if (pubObj->valid != PublicationValidationIdentifier) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidPublicationString);
        return nullptr;
    }
    return pubObj;
}

auto inputSearch = [](const helics::InterfaceHandle& hnd, const auto& testInput) { return hnd < testInput->inputPtr->getHandle(); };

inline HelicsInput addInput(HelicsFederate fed, std::unique_ptr<helics::InputObject> inp)
{
    auto* fedObj = reinterpret_cast<helics::FedObject*>(fed);
    inp->valid = InputValidationIdentifier;
    HelicsInput hinp = inp.get();
    if (fedObj->inputs.empty() || inp->inputPtr->getHandle() > fedObj->inputs.back()->inputPtr->getHandle()) {
        fedObj->inputs.push_back(std::move(inp));
    } else {
        auto ind = std::upper_bound(fedObj->inputs.begin(), fedObj->inputs.end(), inp->inputPtr->getHandle(), inputSearch);
        fedObj->inputs.insert(ind, std::move(inp));
    }
    return hinp;
}

HelicsInput findOrCreateInput(HelicsFederate fed, helics::Input& input)
{
    auto* fedObj = reinterpret_cast<helics::FedObject*>(fed);
    const auto handle = input.getHandle();
    auto ind = std::upper_bound(fedObj->inputs.begin(), fedObj->inputs.end(), handle, inputSearch);
    if (ind != fedObj->inputs.end() && (*ind)->inputPtr->getHandle() == handle) {
        HelicsInput hinp = ind->get();
        return hinp;
    }
    auto inp = std::make_unique<helics::InputObject>();
    inp->inputPtr = &input;
    inp->fedptr = getValueFedSharedPtr(fed, nullptr);
    return addInput(fed, std::move(inp));
}

auto pubSearch = [](const helics::InterfaceHandle& hnd, const auto& testPub) { return hnd < testPub->pubPtr->getHandle(); };

HelicsPublication addPublication(HelicsFederate fed, std::unique_ptr<helics::PublicationObject> pub)
{
    auto* fedObj = reinterpret_cast<helics::FedObject*>(fed);
    pub->valid = PublicationValidationIdentifier;
    HelicsPublication hpub = pub.get();
    if (fedObj->pubs.empty() || pub->pubPtr->getHandle() > fedObj->pubs.back()->pubPtr->getHandle()) {
        fedObj->pubs.push_back(std::move(pub));
    } else {
        auto ind = std::upper_bound(fedObj->pubs.begin(), fedObj->pubs.end(), pub->pubPtr->getHandle(), pubSearch);
        fedObj->pubs.insert(ind, std::move(pub));
    }

    return hpub;
}

HelicsPublication findOrCreatePublication(HelicsFederate fed, helics::Publication& pub)
{
    auto* fedObj = reinterpret_cast<helics::FedObject*>(fed);
    const auto handle = pub.getHandle();
    auto ind = std::upper_bound(fedObj->pubs.begin(), fedObj->pubs.end(), handle, pubSearch);
    if (ind != fedObj->pubs.end() && (*ind)->pubPtr->getHandle() == handle) {
        HelicsPublication hpub = ind->get();
        return hpub;
    }
    auto pubObj = std::make_unique<helics::PublicationObject>();
    pubObj->pubPtr = &pub;
    pubObj->fedptr = getValueFedSharedPtr(fed, nullptr);
    return addPublication(fed, std::move(pubObj));
}
}  // namespace
/* input/pub registration */
HelicsInput helicsFederateRegisterSubscription(HelicsFederate fed, const char* key, const char* units, HelicsError* err)
{
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto sub = std::make_unique<helics::InputObject>();
        sub->inputPtr = &fedObj->registerSubscription(AS_STRING_VIEW(key), AS_STRING_VIEW(units));
        sub->fedptr = std::move(fedObj);
        return addInput(fed, std::move(sub));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
    // LCOV_EXCL_STOP
}

HelicsPublication
    helicsFederateRegisterTypePublication(HelicsFederate fed, const char* key, const char* type, const char* units, HelicsError* err)
{
    // now generate a generic input
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto pub = std::make_unique<helics::PublicationObject>();
        pub->pubPtr = &fedObj->registerPublication(AS_STRING_VIEW(key), AS_STRING_VIEW(type), AS_STRING_VIEW(units));
        pub->fedptr = std::move(fedObj);
        return addPublication(fed, std::move(pub));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}
static constexpr char unknownTypeCode[] = "unrecognized type code";
HelicsPublication
    helicsFederateRegisterPublication(HelicsFederate fed, const char* key, HelicsDataTypes type, const char* units, HelicsError* err)
{
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    if ((type < HELICS_DATA_TYPE_STRING) || (type > HELICS_DATA_TYPE_CHAR)) {
        if (type == HELICS_DATA_TYPE_RAW) {
            return helicsFederateRegisterTypePublication(fed, key, "raw", units, err);
        }
        if (type != HELICS_DATA_TYPE_JSON) {
            assignError(err, HELICS_ERROR_INVALID_ARGUMENT, unknownTypeCode);
            return nullptr;
        }
    }
    try {
        auto pub = std::make_unique<helics::PublicationObject>();
        pub->pubPtr = &(fedObj->registerPublication(AS_STRING_VIEW(key),
                                                    helics::typeNameStringRef(static_cast<helics::DataType>(type)),
                                                    AS_STRING_VIEW(units)));
        pub->fedptr = std::move(fedObj);
        return addPublication(fed, std::move(pub));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

HelicsPublication
    helicsFederateRegisterGlobalTypePublication(HelicsFederate fed, const char* key, const char* type, const char* units, HelicsError* err)
{
    // now generate a generic input
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto pub = std::make_unique<helics::PublicationObject>();
        pub->pubPtr = &fedObj->registerGlobalPublication(AS_STRING_VIEW(key), AS_STRING_VIEW(type), AS_STRING_VIEW(units));
        pub->fedptr = std::move(fedObj);
        return addPublication(fed, std::move(pub));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

HelicsPublication
    helicsFederateRegisterGlobalPublication(HelicsFederate fed, const char* key, HelicsDataTypes type, const char* units, HelicsError* err)
{
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    if ((type < 0) || (type > HELICS_DATA_TYPE_TIME)) {
        if (type == HELICS_DATA_TYPE_RAW) {
            return helicsFederateRegisterGlobalTypePublication(fed, key, "raw", units, err);
        }
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, unknownTypeCode);
        return nullptr;
    }

    try {
        auto pub = std::make_unique<helics::PublicationObject>();
        pub->pubPtr = &(fedObj->registerGlobalPublication(AS_STRING_VIEW(key),
                                                          helics::typeNameStringRef(static_cast<helics::DataType>(type)),
                                                          AS_STRING_VIEW(units)));
        pub->fedptr = std::move(fedObj);
        return addPublication(fed, std::move(pub));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

HelicsInput helicsFederateRegisterTypeInput(HelicsFederate fed, const char* key, const char* type, const char* units, HelicsError* err)
{
    // now generate a generic input
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto inp = std::make_unique<helics::InputObject>();
        inp->inputPtr = &(fedObj->registerInput(AS_STRING_VIEW(key), AS_STRING_VIEW(type), AS_STRING_VIEW(units)));
        inp->fedptr = std::move(fedObj);
        return addInput(fed, std::move(inp));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

HelicsInput helicsFederateRegisterInput(HelicsFederate fed, const char* key, HelicsDataTypes type, const char* units, HelicsError* err)
{
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    if ((type < HELICS_DATA_TYPE_STRING) || (type > HELICS_DATA_TYPE_CHAR)) {
        if (type == HELICS_DATA_TYPE_RAW) {
            return helicsFederateRegisterTypeInput(fed, key, "raw", units, err);
        }
        if (type != HELICS_DATA_TYPE_ANY && type != HELICS_DATA_TYPE_JSON) {
            assignError(err, HELICS_ERROR_INVALID_ARGUMENT, unknownTypeCode);
            return nullptr;
        }
    }

    try {
        auto inp = std::make_unique<helics::InputObject>();
        inp->inputPtr = &(fedObj->registerInput(AS_STRING_VIEW(key),
                                                helics::typeNameStringRef(static_cast<helics::DataType>(type)),
                                                AS_STRING_VIEW(units)));
        inp->fedptr = std::move(fedObj);
        return addInput(fed, std::move(inp));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

HelicsInput
    helicsFederateRegisterGlobalTypeInput(HelicsFederate fed, const char* key, const char* type, const char* units, HelicsError* err)
{
    // now generate a generic input
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto inp = std::make_unique<helics::InputObject>();
        inp->inputPtr = &(fedObj->registerGlobalInput(AS_STRING_VIEW(key), AS_STRING_VIEW(type), AS_STRING_VIEW(units)));
        inp->fedptr = std::move(fedObj);
        return addInput(fed, std::move(inp));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

HelicsInput
    helicsFederateRegisterGlobalInput(HelicsFederate fed, const char* key, HelicsDataTypes type, const char* units, HelicsError* err)
{
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    if ((type < HELICS_DATA_TYPE_STRING) || (type > HELICS_DATA_TYPE_CHAR)) {
        if (type == HELICS_DATA_TYPE_RAW) {
            return helicsFederateRegisterGlobalTypeInput(fed, key, "raw", units, err);
        }
        if (type != HELICS_DATA_TYPE_ANY && type != HELICS_DATA_TYPE_JSON) {
            assignError(err, HELICS_ERROR_INVALID_ARGUMENT, unknownTypeCode);
            return nullptr;
        }
    }

    try {
        auto inp = std::make_unique<helics::InputObject>();
        inp->inputPtr = &(fedObj->registerGlobalInput(AS_STRING_VIEW(key),
                                                      helics::typeNameStringRef(static_cast<helics::DataType>(type)),
                                                      AS_STRING_VIEW(units)));
        inp->fedptr = std::move(fedObj);
        return addInput(fed, std::move(inp));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

void helicsFederateRegisterFromPublicationJSON(HelicsFederate fed, const char* json, HelicsError* err)
{
    if (json == nullptr) {  // this isn't an error, just doesn't do anything
        return;
    }
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return;
    }
    try {
        fedObj->registerFromPublicationJSON(json);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsFederatePublishJSON(HelicsFederate fed, const char* json, HelicsError* err)
{
    if (json == nullptr) {  // this isn't an error just doesn't do anything
        return;
    }
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return;
    }
    try {
        fedObj->publishJSON(json);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

static constexpr char invalidPubName[] = "the specified publication name is a not a valid publication name";
static constexpr char invalidPubIndex[] = "the specified publication index is not valid";

HelicsPublication helicsFederateGetPublication(HelicsFederate fed, const char* key, HelicsError* err)
{
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto& pub = fedObj->getPublication(key);
        if (!pub.isValid()) {
            assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidPubName);
            return nullptr;
        }
        return findOrCreatePublication(fed, pub);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    // LCOV_EXCL_STOP
}

HelicsPublication helicsFederateGetPublicationByIndex(HelicsFederate fed, int index, HelicsError* err)
{
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto& pub = fedObj->getPublication(index);
        if (!pub.isValid()) {
            assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidPubIndex);
            return nullptr;
        }
        return findOrCreatePublication(fed, pub);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    // LCOV_EXCL_STOP
}

static constexpr char invalidInputName[] = "the specified input name is a not a recognized input";
static constexpr char invalidInputIndex[] = "the specified input index is not valid";

HelicsInput helicsFederateGetInput(HelicsFederate fed, const char* key, HelicsError* err)
{
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    CHECK_NULL_STRING(key, nullptr);
    try {
        auto& input = fedObj->getInput(key);
        if (!input.isValid()) {
            assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidInputName);
            return nullptr;
        }
        return findOrCreateInput(fed, input);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    // LCOV_EXCL_STOP
}

HelicsInput helicsFederateGetInputByIndex(HelicsFederate fed, int index, HelicsError* err)
{
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto& input = fedObj->getInput(index);
        if (!input.isValid()) {
            assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidInputIndex);
            return nullptr;
        }
        return findOrCreateInput(fed, input);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    // LCOV_EXCL_STOP
}

static constexpr char invalidTargetKey[] = "the specified input target is a not a recognized";

HelicsInput helicsFederateGetSubscription(HelicsFederate fed, const char* target, HelicsError* err)
{
    return helicsFederateGetInputByTarget(fed, target, err);
}

HelicsInput helicsFederateGetInputByTarget(HelicsFederate fed, const char* target, HelicsError* err)
{
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    CHECK_NULL_STRING(target, nullptr);
    try {
        auto& input = fedObj->getInputByTarget(target);
        if (!input.isValid()) {
            assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidTargetKey);
            return nullptr;
        }
        return findOrCreateInput(fed, input);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    // LCOV_EXCL_STOP
}

void helicsFederateClearUpdates(HelicsFederate fed)
{
    auto fedObj = getValueFedSharedPtr(fed, nullptr);
    if (!fedObj) {
        return;
    }
    try {
        fedObj->clearUpdates();
    }
    // LCOV_EXCL_START
    catch (...) {
        ;
    }
    // LCOV_EXCL_STOP
}

/* getting and publishing values */
void helicsPublicationPublishBytes(HelicsPublication pub, const void* data, int datalen, HelicsError* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    try {
        pubObj->fedptr->publishBytes(*pubObj->pubPtr, reinterpret_cast<const char*>(data), datalen);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsPublicationPublishString(HelicsPublication pub, const char* str, HelicsError* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    try {
        pubObj->pubPtr->publish(AS_STRING_VIEW(str));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsPublicationPublishInteger(HelicsPublication pub, int64_t val, HelicsError* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    try {
        pubObj->pubPtr->publish(val);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsPublicationPublishBoolean(HelicsPublication pub, HelicsBool val, HelicsError* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    try {
        pubObj->pubPtr->publish((val != HELICS_FALSE));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsPublicationPublishDouble(HelicsPublication pub, double val, HelicsError* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    try {
        pubObj->pubPtr->publish(val);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsPublicationPublishTime(HelicsPublication pub, HelicsTime val, HelicsError* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    try {
        const helics::Time tval(val);
        pubObj->pubPtr->publish(tval);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsPublicationPublishChar(HelicsPublication pub, char val, HelicsError* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    try {
        pubObj->pubPtr->publish(val);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsPublicationPublishComplex(HelicsPublication pub, double real, double imag, HelicsError* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    try {
        pubObj->pubPtr->publish(std::complex<double>(real, imag));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsPublicationPublishVector(HelicsPublication pub, const double* vectorInput, int vectorLength, HelicsError* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    try {
        if ((vectorInput == nullptr) || (vectorLength <= 0)) {
            pubObj->pubPtr->publish(std::vector<double>());
        } else {
            pubObj->pubPtr->publish(vectorInput, vectorLength);
        }
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsPublicationPublishComplexVector(HelicsPublication pub, const double* vectorInput, int vectorLength, HelicsError* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    try {
        if ((vectorInput == nullptr) || (vectorLength <= 0)) {
            pubObj->pubPtr->publish(std::vector<double>());
        } else {
            pubObj->pubPtr->publishComplex(vectorInput, vectorLength);
        }
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsPublicationPublishNamedPoint(HelicsPublication pub, const char* str, double val, HelicsError* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    try {
        if (str == nullptr) {
            pubObj->pubPtr->publish(std::string(), val);
        } else {
            pubObj->pubPtr->publish(str, val);
        }
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsPublicationPublishDataBuffer(HelicsPublication pub, HelicsDataBuffer buffer, HelicsError* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    try {
        auto* buff = getBuffer(buffer);
        if (buff == nullptr) {
            pubObj->pubPtr->publish("");
            return;
        }
        helics::defV pubVal;
        helics::valueExtract(helics::data_view(*buff), helics::DataType::HELICS_UNKNOWN, pubVal);
        pubObj->pubPtr->publish(pubVal);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsPublicationAddTarget(HelicsPublication pub, const char* target, HelicsError* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    CHECK_NULL_STRING(target, void());

    pubObj->pubPtr->addTarget(target);
}

HelicsBool helicsPublicationIsValid(HelicsPublication pub)
{
    auto* pubObj = verifyPublication(pub, nullptr);
    if (pubObj == nullptr) {
        return HELICS_FALSE;
    }
    return (pubObj->pubPtr->isValid()) ? HELICS_TRUE : HELICS_FALSE;
}

HelicsBool helicsInputIsValid(HelicsInput ipt)
{
    auto* inpObj = verifyInput(ipt, nullptr);
    if (inpObj == nullptr) {
        return HELICS_FALSE;
    }
    return (inpObj->inputPtr->isValid()) ? HELICS_TRUE : HELICS_FALSE;
}

void helicsInputAddTarget(HelicsInput ipt, const char* target, HelicsError* err)
{
    auto* inpObj = verifyInput(ipt, err);
    if (inpObj == nullptr) {
        return;
    }
    CHECK_NULL_STRING(target, void());
    inpObj->inputPtr->addTarget(target);
}

int helicsInputGetByteCount(HelicsInput inp)
{
    auto* inpObj = verifyInput(inp, nullptr);
    if (inpObj == nullptr) {
        return (0);
    }
    return static_cast<int>(inpObj->inputPtr->getByteCount());
}

bool checkOutputArgString(const char* outputString, int maxlen, HelicsError* err)
{
    static constexpr char invalidOutputString[] = "Output string location is invalid";
    if ((outputString == nullptr) || (maxlen <= 0)) {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidOutputString);
        return false;
    }
    return true;
}

HelicsDataBuffer helicsInputGetDataBuffer(HelicsInput inp, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return (nullptr);
    }
    const helics::data_view bytes = inpObj->inputPtr->getBytes();
    auto* ptr = new helics::SmallBuffer(bytes.string_view());
    return createAPIDataBuffer(*ptr);
}

void helicsInputGetBytes(HelicsInput inp, void* data, int maxDatalen, int* actualSize, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (actualSize != nullptr) {  // for initialization
        *actualSize = 0;
    }
    if (inpObj == nullptr) {
        return;
    }
    if (!checkOutputArgString(static_cast<char*>(data), maxDatalen, err)) {
        return;
    }
    try {
        auto bytes = inpObj->inputPtr->getBytes();
        if (maxDatalen > static_cast<int>(bytes.size())) {
            memcpy(data, bytes.data(), bytes.size());
            if (actualSize != nullptr) {
                *actualSize = static_cast<int>(bytes.size());
            }
            return;
        }
        memcpy(data, bytes.data(), maxDatalen);
        if (actualSize != nullptr) {
            *actualSize = maxDatalen;
        }
        return;
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsInputGetString(HelicsInput inp, char* outputString, int maxStringLen, int* actualLength, HelicsError* err)
{
    if (actualLength != nullptr) {  // for initialization
        *actualLength = 0;
    }
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }

    if (!checkOutputArgString(outputString, maxStringLen, err)) {
        return;
    }
    try {
        const int length = inpObj->inputPtr->getValue(outputString, maxStringLen);
        if (actualLength != nullptr) {  // for initialization
            *actualLength = length;
        }
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

int64_t helicsInputGetInteger(HelicsInput inp, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return (-101);
    }
    try {
        return inpObj->inputPtr->getValue<int64_t>();
    }
    catch (...) {
        helicsErrorHandler(err);
        return (-101);
    }
}

HelicsBool helicsInputGetBoolean(HelicsInput inp, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return HELICS_FALSE;
    }
    try {
        const bool boolval = inpObj->inputPtr->getValue<bool>();
        return (boolval) ? HELICS_TRUE : HELICS_FALSE;
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return HELICS_FALSE;
    }
    // LCOV_EXCL_STOP
}

double helicsInputGetDouble(HelicsInput inp, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return HELICS_INVALID_DOUBLE;
    }
    try {
        return inpObj->inputPtr->getValue<double>();
    }
    catch (...) {
        helicsErrorHandler(err);
        return HELICS_INVALID_DOUBLE;
    }
}

HelicsTime helicsInputGetTime(HelicsInput inp, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return HELICS_TIME_INVALID;
    }
    try {
        auto time = inpObj->inputPtr->getValue<helics::Time>();
        return static_cast<HelicsTime>(time);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return HELICS_TIME_INVALID;
    }
    // LCOV_EXCL_STOP
}

char helicsInputGetChar(HelicsInput inp, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return '\x15';  // NAK (negative acknowledgment) symbol
    }
    try {
        return inpObj->inputPtr->getValue<char>();
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return '\x15';  // NAK (negative acknowledgment) symbol
    }
    // LCOV_EXCL_STOP
}

void helicsInputGetComplex(HelicsInput inp, double* real, double* imag, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    try {
        auto cval = inpObj->inputPtr->getValue<std::complex<double>>();
        if (real != nullptr) {
            *real = cval.real();
        }
        if (imag != nullptr) {
            *imag = cval.imag();
        }
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

HelicsComplex helicsInputGetComplexObject(HelicsInput inp, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);

    if (inpObj == nullptr) {
        // time invalid is just an invalid double
        return {HELICS_TIME_INVALID, HELICS_TIME_INVALID};
    }

    try {
        auto cval = inpObj->inputPtr->getValue<std::complex<double>>();
        return {cval.real(), cval.imag()};
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return {HELICS_TIME_INVALID, HELICS_TIME_INVALID};
    }
    // LCOV_EXCL_STOP
}

int helicsInputGetVectorSize(HelicsInput inp)
{
    auto* inpObj = verifyInput(inp, nullptr);
    if (inpObj == nullptr) {
        return 0;
    }
    try {
        return static_cast<int>(inpObj->inputPtr->getVectorSize());
    }
    // LCOV_EXCL_START
    catch (...) {
        return 0;
    }
    // LCOV_EXCL_STOP
}

int helicsInputGetStringSize(HelicsInput inp)
{
    auto* inpObj = verifyInput(inp, nullptr);
    if (inpObj == nullptr) {
        return 0;
    }
    try {
        return static_cast<int>(inpObj->inputPtr->getStringSize()) + 1;
    }
    // LCOV_EXCL_START
    catch (...) {
        return 0;
    }
    // LCOV_EXCL_STOP
}

void helicsInputGetVector(HelicsInput inp, double data[], int maxlen, int* actualSize, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (actualSize != nullptr) {
        *actualSize = 0;
    }
    if (inpObj == nullptr) {
        return;
    }
    if ((data == nullptr) || (maxlen <= 0)) {
        inpObj->inputPtr->clearUpdate();
        // this isn't an error, just no data retrieved
        return;
    }
    try {
        const int length = inpObj->inputPtr->getValue(data, maxlen);
        if (actualSize != nullptr) {
            *actualSize = length;
        }
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsInputGetComplexVector(HelicsInput inp, double data[], int maxlen, int* actualSize, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (actualSize != nullptr) {
        *actualSize = 0;
    }
    if (inpObj == nullptr) {
        return;
    }
    if ((data == nullptr) || (maxlen <= 0)) {
        inpObj->inputPtr->clearUpdate();
        // this isn't an error, just no data retrieved
        return;
    }
    try {
        const int length = inpObj->inputPtr->getComplexValue(data, maxlen);
        if (actualSize != nullptr) {
            *actualSize = length;
        }
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsInputGetNamedPoint(HelicsInput inp, char* outputString, int maxStringLen, int* actualLength, double* val, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (actualLength != nullptr) {
        *actualLength = 0;
    }
    if (inpObj == nullptr) {
        return;
    }

    try {
        helics::NamedPoint npoint = inpObj->inputPtr->getValue<helics::NamedPoint>();
        if (outputString != nullptr && maxStringLen > 0) {
            const int length = std::min(static_cast<int>(npoint.name.size()), maxStringLen);
            memcpy(outputString, npoint.name.data(), length);

            if (length == maxStringLen) {
                outputString[maxStringLen - 1] = '\0';
                if (actualLength != nullptr) {
                    *actualLength = maxStringLen;
                }
            } else {
                outputString[length] = '\0';
                if (actualLength != nullptr) {
                    *actualLength = length + 1;
                }
            }
        }
        if (val != nullptr) {
            *val = npoint.value;
        }

        return;
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsInputSetDefaultBytes(HelicsInput inp, const void* data, int dataLen, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    try {
        if ((data == nullptr) || (dataLen <= 0)) {
            inpObj->fedptr->setDefaultValue(*inpObj->inputPtr, std::string());
        } else {
            inpObj->fedptr->setDefaultValue(*inpObj->inputPtr, helics::data_view(static_cast<const char*>(data), dataLen));
        }
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsInputSetDefaultString(HelicsInput inp, const char* str, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    try {
        inpObj->inputPtr->setDefault(AS_STRING(str));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsInputSetDefaultInteger(HelicsInput inp, int64_t val, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    inpObj->inputPtr->setDefault(val);
}

void helicsInputSetDefaultBoolean(HelicsInput inp, HelicsBool val, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    inpObj->inputPtr->setDefault((val != HELICS_FALSE));
}

void helicsInputSetDefaultDouble(HelicsInput inp, double val, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    inpObj->inputPtr->setDefault(val);
}

void helicsInputSetDefaultTime(HelicsInput inp, HelicsTime val, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }

    const helics::Time tval(val);
    inpObj->inputPtr->setDefault(tval);
}

void helicsInputSetDefaultChar(HelicsInput inp, char val, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    inpObj->inputPtr->setDefault(val);
}

void helicsInputSetDefaultComplex(HelicsInput inp, double real, double imag, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }

    inpObj->inputPtr->setDefault(std::complex<double>(real, imag));
}

void helicsInputSetDefaultVector(HelicsInput inp, const double* vectorInput, int vectorLength, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    try {
        if ((vectorInput == nullptr) || (vectorLength <= 0)) {
            inpObj->inputPtr->setDefault(std::vector<double>{});
        } else {
            inpObj->inputPtr->setDefault(std::vector<double>(vectorInput, vectorInput + vectorLength));
        }
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsInputSetDefaultComplexVector(HelicsInput inp, const double* vectorInput, int vectorLength, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    try {
        if ((vectorInput == nullptr) || (vectorLength <= 0)) {
            inpObj->inputPtr->setDefault(std::vector<std::complex<double>>{});
        } else {
            std::vector<std::complex<double>> cvec;
            cvec.reserve(vectorLength);
            for (std::ptrdiff_t ii = 0; ii < vectorLength; ++ii) {
                cvec.emplace_back(vectorInput[2 * ii], vectorInput[2 * ii + 1]);
            }
            inpObj->inputPtr->setDefault(std::move(cvec));
        }
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsInputSetDefaultNamedPoint(HelicsInput inp, const char* str, double val, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    try {
        inpObj->inputPtr->setDefault(helics::NamedPoint(AS_STRING_VIEW(str), val));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

const char* helicsInputGetType(HelicsInput inp)
{
    auto* inpObj = verifyInput(inp, nullptr);
    if (inpObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }

    try {
        const std::string& type = inpObj->inputPtr->getType();
        return type.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return gHelicsEmptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

const char* helicsInputGetPublicationType(HelicsInput ipt)
{
    auto* inpObj = verifyInput(ipt, nullptr);
    if (inpObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }

    try {
        const std::string& type = inpObj->inputPtr->getPublicationType();
        return type.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return gHelicsEmptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

int helicsInputGetPublicationDataType(HelicsInput ipt)
{
    return helicsGetDataType(helicsInputGetPublicationType(ipt));
}

const char* helicsPublicationGetType(HelicsPublication pub)
{
    auto* pubObj = verifyPublication(pub, nullptr);
    if (pubObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }

    try {
        const std::string& type = pubObj->pubPtr->getType();
        return type.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return gHelicsEmptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

const char* helicsInputGetName(HelicsInput inp)
{
    auto* inpObj = verifyInput(inp, nullptr);
    if (inpObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }

    try {
        const std::string& key = inpObj->inputPtr->getName();
        return key.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return gHelicsEmptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

const char* helicsInputGetTarget(HelicsInput ipt)
{
    auto* inpObj = verifyInput(ipt, nullptr);
    if (inpObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }

    try {
        const std::string& key = inpObj->fedptr->getTarget(*(inpObj->inputPtr));
        return key.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return gHelicsEmptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

const char* helicsSubscriptionGetTarget(HelicsInput ipt)
{
    return helicsInputGetTarget(ipt);
}

const char* helicsPublicationGetName(HelicsPublication pub)
{
    auto* pubObj = verifyPublication(pub, nullptr);
    if (pubObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }
    try {
        const std::string& key = pubObj->pubPtr->getName();
        return key.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return gHelicsEmptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

const char* helicsInputGetInjectionUnits(HelicsInput ipt)
{
    auto* inpObj = verifyInput(ipt, nullptr);
    if (inpObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }
    try {
        const std::string& units = inpObj->inputPtr->getInjectionUnits();
        return units.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return gHelicsEmptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

const char* helicsInputGetExtractionUnits(HelicsInput ipt)
{
    auto* inpObj = verifyInput(ipt, nullptr);
    if (inpObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }
    try {
        const std::string& units = inpObj->inputPtr->getUnits();
        return units.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return gHelicsEmptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

const char* helicsInputGetUnits(HelicsInput inp)
{
    return helicsInputGetExtractionUnits(inp);
}

const char* helicsPublicationGetUnits(HelicsPublication pub)
{
    auto* pubObj = verifyPublication(pub, nullptr);
    if (pubObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }
    const std::string& units = pubObj->pubPtr->getUnits();
    return units.c_str();
}

const char* helicsInputGetInfo(HelicsInput inp)
{
    auto* inpObj = verifyInput(inp, nullptr);
    if (inpObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }
    try {
        const std::string& info = inpObj->inputPtr->getInfo();
        return info.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return gHelicsEmptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

void helicsInputSetInfo(HelicsInput inp, const char* info, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    try {
        inpObj->inputPtr->setInfo(AS_STRING_VIEW(info));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

const char* helicsPublicationGetInfo(HelicsPublication pub)
{
    auto* pubObj = verifyPublication(pub, nullptr);
    if (pubObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }
    try {
        const std::string& info = pubObj->pubPtr->getInfo();
        return info.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return gHelicsEmptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

void helicsPublicationSetInfo(HelicsPublication pub, const char* info, HelicsError* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    try {
        pubObj->pubPtr->setInfo(AS_STRING_VIEW(info));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

const char* helicsInputGetTag(HelicsInput inp, const char* tagname)
{
    auto* inpObj = verifyInput(inp, nullptr);
    if (inpObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }
    try {
        const std::string& info = inpObj->inputPtr->getTag(AS_STRING_VIEW(tagname));
        return info.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return gHelicsEmptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

void helicsInputSetTag(HelicsInput inp, const char* tagname, const char* tagvalue, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    try {
        inpObj->inputPtr->setTag(AS_STRING_VIEW(tagname), AS_STRING_VIEW(tagvalue));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

const char* helicsPublicationGetTag(HelicsPublication pub, const char* tagname)
{
    auto* pubObj = verifyPublication(pub, nullptr);
    if (pubObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }
    try {
        const std::string& info = pubObj->pubPtr->getTag(AS_STRING_VIEW(tagname));
        return info.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return gHelicsEmptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

void helicsPublicationSetTag(HelicsPublication pub, const char* tagname, const char* tagvalue, HelicsError* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    try {
        pubObj->pubPtr->setTag(AS_STRING_VIEW(tagname), AS_STRING_VIEW(tagvalue));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

int helicsInputGetOption(HelicsInput inp, int option)
{
    auto* inpObj = verifyInput(inp, nullptr);
    if (inpObj == nullptr) {
        return HELICS_FALSE;
    }
    try {
        return (inpObj->inputPtr->getOption(option));
    }
    // LCOV_EXCL_START
    catch (...) {
        return HELICS_FALSE;
    }
    // LCOV_EXCL_STOP
}

void helicsInputSetOption(HelicsInput inp, int option, int value, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    try {
        inpObj->inputPtr->setOption(option, value);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

int helicsPublicationGetOption(HelicsPublication pub, int option)
{
    auto* pubObj = verifyPublication(pub, nullptr);
    if (pubObj == nullptr) {
        return HELICS_FALSE;
    }
    try {
        return pubObj->pubPtr->getOption(option);
    }
    // LCOV_EXCL_START
    catch (...) {
        return HELICS_FALSE;
    }
    // LCOV_EXCL_STOP
}

void helicsPublicationSetOption(HelicsPublication pub, int option, int value, HelicsError* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    try {
        pubObj->pubPtr->setOption(option, value);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsPublicationSetMinimumChange(HelicsPublication pub, double tolerance, HelicsError* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    pubObj->pubPtr->setMinimumChange(tolerance);
}

void helicsInputSetMinimumChange(HelicsInput inp, double tolerance, HelicsError* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    inpObj->inputPtr->setMinimumChange(tolerance);
}

HelicsBool helicsInputIsUpdated(HelicsInput inp)
{
    auto* inpObj = verifyInput(inp, nullptr);
    if (inpObj == nullptr) {
        return HELICS_FALSE;
    }

    auto val = inpObj->inputPtr->isUpdated();
    return (val) ? HELICS_TRUE : HELICS_FALSE;
}

HelicsTime helicsInputLastUpdateTime(HelicsInput inp)
{
    auto* inpObj = verifyInput(inp, nullptr);
    if (inpObj == nullptr) {
        return HELICS_TIME_INVALID;
    }
    try {
        auto time = inpObj->inputPtr->getLastUpdate();
        return static_cast<HelicsTime>(time);
    }
    // LCOV_EXCL_START
    catch (...) {
        return HELICS_TIME_INVALID;
    }
    // LCOV_EXCL_STOP
}

void helicsInputClearUpdate(HelicsInput inp)
{
    auto* inpObj = verifyInput(inp, nullptr);
    if (inpObj == nullptr) {
        return;
    }
    try {
        inpObj->inputPtr->clearUpdate();
    }
    // LCOV_EXCL_START
    catch (...) {
        ;
    }
    // LCOV_EXCL_STOP
}

int helicsFederateGetPublicationCount(HelicsFederate fed)
{
    // this call should be with a nullptr since it can fail and still be a successful call
    auto* vfedObj = getValueFed(fed, nullptr);
    if (vfedObj == nullptr) {
        return 0;
    }
    return static_cast<int>(vfedObj->getPublicationCount());
}

int helicsFederateGetInputCount(HelicsFederate fed)
{
    // this call should be with a nullptr since it can fail and still be a successful call
    auto* vfedObj = getValueFed(fed, nullptr);
    if (vfedObj == nullptr) {
        return 0;
    }
    return static_cast<int>(vfedObj->getInputCount());
}
