/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "../core/core-exceptions.hpp"
#include "../helics.hpp"
#include "ValueFederate.h"
#include "internal/api_objects.h"

#include <cstring>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

/** random integer for validation purposes of inputs */
static const int InputValidationIdentifier = 0x3456'E052;

/** random integer for validation purposes of publications */
static const int PublicationValidationIdentifier = 0x97B1'00A5;

static const char* invalidInputString = "The given input object does not point to a valid object";

static const char* invalidPublicationString = "The given publication object does not point to a valid object";

static helics::InputObject* verifyInput(helics_input inp, helics_error* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    if (inp == nullptr) {
        assignError(err, helics_error_invalid_object, invalidInputString);
        return nullptr;
    }
    auto* inpObj = reinterpret_cast<helics::InputObject*>(inp);
    if (inpObj->valid != InputValidationIdentifier) {
        assignError(err, helics_error_invalid_object, invalidInputString);
        return nullptr;
    }
    return inpObj;
}

static helics::PublicationObject* verifyPublication(helics_publication pub, helics_error* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    if (pub == nullptr) {
        assignError(err, helics_error_invalid_object, invalidPublicationString);
        return nullptr;
    }
    auto* pubObj = reinterpret_cast<helics::PublicationObject*>(pub);
    if (pubObj->valid != PublicationValidationIdentifier) {
        assignError(err, helics_error_invalid_object, invalidPublicationString);
        return nullptr;
    }
    return pubObj;
}

static inline helics_input addInput(helics_federate fed, std::unique_ptr<helics::InputObject> inp)
{
    auto* fedObj = reinterpret_cast<helics::FedObject*>(fed);
    inp->valid = InputValidationIdentifier;
    helics_input hinp = inp.get();
    fedObj->inputs.push_back(std::move(inp));
    return hinp;
}

static inline helics_publication addPublication(helics_federate fed, std::unique_ptr<helics::PublicationObject> pub)
{
    auto* fedObj = reinterpret_cast<helics::FedObject*>(fed);
    pub->valid = PublicationValidationIdentifier;
    helics_publication hpub = pub.get();
    fedObj->pubs.push_back(std::move(pub));
    return hpub;
}

/* input/pub registration */
helics_input helicsFederateRegisterSubscription(helics_federate fed, const char* key, const char* units, helics_error* err)
{
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto sub = std::make_unique<helics::InputObject>();
        sub->inputPtr = &fedObj->registerSubscription(AS_STRING(key), AS_STRING(units));
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

helics_publication
    helicsFederateRegisterTypePublication(helics_federate fed, const char* key, const char* type, const char* units, helics_error* err)
{
    // now generate a generic subscription
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto pub = std::make_unique<helics::PublicationObject>();
        pub->pubPtr = &fedObj->registerPublication(AS_STRING(key), AS_STRING(type), AS_STRING(units));
        pub->fedptr = std::move(fedObj);
        return addPublication(fed, std::move(pub));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}
static constexpr char unknownTypeCode[] = "unrecognized type code";
helics_publication
    helicsFederateRegisterPublication(helics_federate fed, const char* key, helics_data_type type, const char* units, helics_error* err)
{
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    if ((type < helics_data_type_string) || (type > helics_data_type_time)) {
        if (type == helics_data_type_raw) {
            return helicsFederateRegisterTypePublication(fed, key, "raw", units, err);
        }
        assignError(err, helics_error_invalid_argument, unknownTypeCode);
        return nullptr;
    }
    try {
        auto pub = std::make_unique<helics::PublicationObject>();
        pub->pubPtr = &(
            fedObj->registerPublication(AS_STRING(key), helics::typeNameStringRef(static_cast<helics::data_type>(type)), AS_STRING(units)));
        pub->fedptr = std::move(fedObj);
        return addPublication(fed, std::move(pub));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

helics_publication helicsFederateRegisterGlobalTypePublication(helics_federate fed,
                                                               const char* key,
                                                               const char* type,
                                                               const char* units,
                                                               helics_error* err)
{
    // now generate a generic subscription
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto pub = std::make_unique<helics::PublicationObject>();
        pub->pubPtr = &fedObj->registerGlobalPublication(AS_STRING(key), AS_STRING(type), AS_STRING(units));
        pub->fedptr = std::move(fedObj);
        return addPublication(fed, std::move(pub));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

helics_publication helicsFederateRegisterGlobalPublication(helics_federate fed,
                                                           const char* key,
                                                           helics_data_type type,
                                                           const char* units,
                                                           helics_error* err)
{
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    if ((type < 0) || (type > helics_data_type_time)) {
        if (type == helics_data_type_raw) {
            return helicsFederateRegisterGlobalTypePublication(fed, key, "raw", units, err);
        }
        assignError(err, helics_error_invalid_argument, unknownTypeCode);
        return nullptr;
    }

    try {
        auto pub = std::make_unique<helics::PublicationObject>();
        pub->pubPtr = &(fedObj->registerGlobalPublication(AS_STRING(key),
                                                          helics::typeNameStringRef(static_cast<helics::data_type>(type)),
                                                          AS_STRING(units)));
        pub->fedptr = std::move(fedObj);
        return addPublication(fed, std::move(pub));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

helics_input helicsFederateRegisterTypeInput(helics_federate fed, const char* key, const char* type, const char* units, helics_error* err)
{
    // now generate a generic subscription
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto inp = std::make_unique<helics::InputObject>();
        inp->inputPtr = &(fedObj->registerInput(AS_STRING(key), AS_STRING(type), AS_STRING(units)));
        inp->fedptr = std::move(fedObj);
        return addInput(fed, std::move(inp));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

helics_input helicsFederateRegisterInput(helics_federate fed, const char* key, helics_data_type type, const char* units, helics_error* err)
{
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    if ((type < helics_data_type_string) || (type > helics_data_type_time)) {
        if (type == helics_data_type_raw) {
            return helicsFederateRegisterTypeInput(fed, key, "raw", units, err);
        }
        if (type != helics_data_type_any) {
            assignError(err, helics_error_invalid_argument, unknownTypeCode);
            return nullptr;
        }
    }

    try {
        auto inp = std::make_unique<helics::InputObject>();
        inp->inputPtr =
            &(fedObj->registerInput(AS_STRING(key), helics::typeNameStringRef(static_cast<helics::data_type>(type)), AS_STRING(units)));
        inp->fedptr = std::move(fedObj);
        return addInput(fed, std::move(inp));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

helics_input
    helicsFederateRegisterGlobalTypeInput(helics_federate fed, const char* key, const char* type, const char* units, helics_error* err)
{
    // now generate a generic subscription
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto inp = std::make_unique<helics::InputObject>();
        inp->inputPtr = &(fedObj->registerGlobalInput(AS_STRING(key), AS_STRING(type), AS_STRING(units)));
        inp->fedptr = std::move(fedObj);
        return addInput(fed, std::move(inp));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

helics_input
    helicsFederateRegisterGlobalInput(helics_federate fed, const char* key, helics_data_type type, const char* units, helics_error* err)
{
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    if ((type < helics_data_type_string) || (type > helics_data_type_time)) {
        if (type == helics_data_type_raw) {
            return helicsFederateRegisterGlobalTypeInput(fed, key, "raw", units, err);
        }
        if (type != helics_data_type_any) {
            assignError(err, helics_error_invalid_argument, unknownTypeCode);
            return nullptr;
        }
    }

    try {
        auto inp = std::make_unique<helics::InputObject>();
        inp->inputPtr =
            &(fedObj->registerInput(AS_STRING(key), helics::typeNameStringRef(static_cast<helics::data_type>(type)), AS_STRING(units)));
        inp->fedptr = std::move(fedObj);
        return addInput(fed, std::move(inp));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

void helicsFederateRegisterFromPublicationJSON(helics_federate fed, const char* json, helics_error* err)
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

void helicsFederatePublishJSON(helics_federate fed, const char* json, helics_error* err)
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

helics_publication helicsFederateGetPublication(helics_federate fed, const char* key, helics_error* err)
{
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto& pub = fedObj->getPublication(key);
        if (!pub.isValid()) {
            assignError(err, helics_error_invalid_argument, invalidPubName);
            return nullptr;
        }
        auto pubObj = std::make_unique<helics::PublicationObject>();
        pubObj->pubPtr = &pub;
        pubObj->fedptr = std::move(fedObj);
        return addPublication(fed, std::move(pubObj));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    // LCOV_EXCL_STOP
}

helics_publication helicsFederateGetPublicationByIndex(helics_federate fed, int index, helics_error* err)
{
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto& id = fedObj->getPublication(index);
        if (!id.isValid()) {
            assignError(err, helics_error_invalid_argument, invalidPubIndex);
            return nullptr;
        }
        auto pub = std::make_unique<helics::PublicationObject>();
        pub->pubPtr = &id;

        pub->fedptr = std::move(fedObj);
        return addPublication(fed, std::move(pub));
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

helics_input helicsFederateGetInput(helics_federate fed, const char* key, helics_error* err)
{
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    CHECK_NULL_STRING(key, nullptr);
    try {
        auto& id = fedObj->getInput(key);
        if (!id.isValid()) {
            assignError(err, helics_error_invalid_argument, invalidInputName);
            return nullptr;
        }
        auto inp = std::make_unique<helics::InputObject>();
        inp->inputPtr = &id;
        inp->fedptr = std::move(fedObj);
        return addInput(fed, std::move(inp));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    // LCOV_EXCL_STOP
}

helics_input helicsFederateGetInputByIndex(helics_federate fed, int index, helics_error* err)
{
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto& id = fedObj->getInput(index);
        if (!id.isValid()) {
            assignError(err, helics_error_invalid_argument, invalidInputIndex);
            return nullptr;
        }
        auto inp = std::make_unique<helics::InputObject>();
        inp->inputPtr = &id;
        inp->fedptr = std::move(fedObj);
        return addInput(fed, std::move(inp));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    // LCOV_EXCL_STOP
}

static constexpr char invalidSubKey[] = "the specified subscription key is a not a recognized key";

helics_input helicsFederateGetSubscription(helics_federate fed, const char* key, helics_error* err)
{
    auto fedObj = getValueFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    CHECK_NULL_STRING(key, nullptr);
    try {
        auto& id = fedObj->getSubscription(key);
        if (!id.isValid()) {
            assignError(err, helics_error_invalid_argument, invalidSubKey);
            return nullptr;
        }
        auto inp = std::make_unique<helics::InputObject>();
        inp->inputPtr = &id;
        inp->fedptr = std::move(fedObj);
        return addInput(fed, std::move(inp));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    // LCOV_EXCL_STOP
}

void helicsFederateClearUpdates(helics_federate fed)
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
    }
    // LCOV_EXCL_STOP
}

/* getting and publishing values */
void helicsPublicationPublishRaw(helics_publication pub, const void* data, int datalen, helics_error* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    try {
        pubObj->fedptr->publishRaw(*pubObj->pubPtr, reinterpret_cast<const char*>(data), datalen);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsPublicationPublishString(helics_publication pub, const char* str, helics_error* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    try {
        pubObj->pubPtr->publish(AS_STRING(str));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsPublicationPublishInteger(helics_publication pub, int64_t val, helics_error* err)
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

void helicsPublicationPublishBoolean(helics_publication pub, helics_bool val, helics_error* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    try {
        pubObj->pubPtr->publish((val != helics_false));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsPublicationPublishDouble(helics_publication pub, double val, helics_error* err)
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

void helicsPublicationPublishTime(helics_publication pub, helics_time val, helics_error* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    try {
        helics::Time tval(val);
        pubObj->pubPtr->publish(tval);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsPublicationPublishChar(helics_publication pub, char val, helics_error* err)
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

void helicsPublicationPublishComplex(helics_publication pub, double real, double imag, helics_error* err)
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

void helicsPublicationPublishVector(helics_publication pub, const double* vectorInput, int vectorLength, helics_error* err)
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

void helicsPublicationPublishNamedPoint(helics_publication pub, const char* str, double val, helics_error* err)
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

void helicsPublicationAddTarget(helics_publication pub, const char* target, helics_error* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    CHECK_NULL_STRING(target, void());

    pubObj->pubPtr->addTarget(target);
}

helics_bool helicsPublicationIsValid(helics_publication pub)
{
    auto* pubObj = verifyPublication(pub, nullptr);
    if (pubObj == nullptr) {
        return helics_false;
    }
    return (pubObj->pubPtr->isValid()) ? helics_true : helics_false;
}

helics_bool helicsInputIsValid(helics_input ipt)
{
    auto* inpObj = verifyInput(ipt, nullptr);
    if (inpObj == nullptr) {
        return helics_false;
    }
    return (inpObj->inputPtr->isValid()) ? helics_true : helics_false;
}

void helicsInputAddTarget(helics_input ipt, const char* target, helics_error* err)
{
    auto* inpObj = verifyInput(ipt, err);
    if (inpObj == nullptr) {
        return;
    }
    CHECK_NULL_STRING(target, void());
    inpObj->inputPtr->addTarget(target);
}

int helicsInputGetRawValueSize(helics_input inp)
{
    auto* inpObj = verifyInput(inp, nullptr);
    if (inpObj == nullptr) {
        return (0);
    }
    return static_cast<int>(inpObj->inputPtr->getRawSize());
}

bool checkOutArgString(const char* outputString, int maxlen, helics_error* err)
{
    static constexpr char invalidOutputString[] = "Output string location is invalid";
    if ((outputString == nullptr) || (maxlen <= 0)) {
        assignError(err, helics_error_invalid_argument, invalidOutputString);
        return false;
    }
    return true;
}

void helicsInputGetRawValue(helics_input inp, void* data, int maxDatalen, int* actualSize, helics_error* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (actualSize != nullptr) {  // for initialization
        *actualSize = 0;
    }
    if (inpObj == nullptr) {
        return;
    }
    if (!checkOutArgString(static_cast<char*>(data), maxDatalen, err)) {
        return;
    }
    try {
        auto dv = inpObj->inputPtr->getRawValue();
        if (maxDatalen > static_cast<int>(dv.size())) {
            memcpy(data, dv.data(), dv.size());
            if (actualSize != nullptr) {
                *actualSize = static_cast<int>(dv.size());
            }
            return;
        }
        memcpy(data, dv.data(), maxDatalen);
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

void helicsInputGetString(helics_input inp, char* outputString, int maxStringLen, int* actualLength, helics_error* err)
{
    if (actualLength != nullptr) {  // for initialization
        *actualLength = 0;
    }
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }

    if (!checkOutArgString(outputString, maxStringLen, err)) {
        return;
    }
    try {
        int length = inpObj->inputPtr->getValue(outputString, maxStringLen);
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

int64_t helicsInputGetInteger(helics_input inp, helics_error* err)
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

helics_bool helicsInputGetBoolean(helics_input inp, helics_error* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return helics_false;
    }
    try {
        bool boolval = inpObj->inputPtr->getValue<bool>();
        return (boolval) ? helics_true : helics_false;
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return helics_false;
    }
    // LCOV_EXCL_STOP
}

double helicsInputGetDouble(helics_input inp, helics_error* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return helics_time_invalid;
    }
    try {
        return inpObj->inputPtr->getValue<double>();
    }
    catch (...) {
        helicsErrorHandler(err);
        return helics_time_invalid;
    }
}

helics_time helicsInputGetTime(helics_input inp, helics_error* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return helics_time_invalid;
    }
    try {
        auto T = inpObj->inputPtr->getValue<helics::Time>();
        return static_cast<helics_time>(T);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return helics_time_invalid;
    }
    // LCOV_EXCL_STOP
}

char helicsInputGetChar(helics_input inp, helics_error* err)
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

void helicsInputGetComplex(helics_input inp, double* real, double* imag, helics_error* err)
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

helics_complex helicsInputGetComplexObject(helics_input inp, helics_error* err)
{
    auto* inpObj = verifyInput(inp, err);

    if (inpObj == nullptr) {
        // time invalid is just an invalid double
        return {helics_time_invalid, helics_time_invalid};
    }

    try {
        auto cval = inpObj->inputPtr->getValue<std::complex<double>>();
        return {cval.real(), cval.imag()};
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return {helics_time_invalid, helics_time_invalid};
    }
    // LCOV_EXCL_STOP
}

int helicsInputGetVectorSize(helics_input inp)
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

int helicsInputGetStringSize(helics_input inp)
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

void helicsInputGetVector(helics_input inp, double data[], int maxlen, int* actualSize, helics_error* err)
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
        int length = inpObj->inputPtr->getValue(data, maxlen);
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

void helicsInputGetNamedPoint(helics_input inp, char* outputString, int maxStringLen, int* actualLength, double* val, helics_error* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (actualLength != nullptr) {
        *actualLength = 0;
    }
    if (inpObj == nullptr) {
        return;
    }

    try {
        helics::NamedPoint np = inpObj->inputPtr->getValue<helics::NamedPoint>();
        if (outputString != nullptr && maxStringLen > 0) {
            int length = std::min(static_cast<int>(np.name.size()), maxStringLen);
            memcpy(outputString, np.name.data(), length);

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
            *val = np.value;
        }

        return;
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsInputSetDefaultRaw(helics_input inp, const void* data, int dataLen, helics_error* err)
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

void helicsInputSetDefaultString(helics_input inp, const char* str, helics_error* err)
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

void helicsInputSetDefaultInteger(helics_input inp, int64_t val, helics_error* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    inpObj->inputPtr->setDefault(val);
}

void helicsInputSetDefaultBoolean(helics_input inp, helics_bool val, helics_error* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    inpObj->inputPtr->setDefault((val != helics_false));
}

void helicsInputSetDefaultDouble(helics_input inp, double val, helics_error* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    inpObj->inputPtr->setDefault(val);
}

void helicsInputSetDefaultTime(helics_input inp, helics_time val, helics_error* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }

    helics::Time tval(val);
    inpObj->inputPtr->setDefault(tval);
}

void helicsInputSetDefaultChar(helics_input inp, char val, helics_error* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    inpObj->inputPtr->setDefault(val);
}

void helicsInputSetDefaultComplex(helics_input inp, double real, double imag, helics_error* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }

    inpObj->inputPtr->setDefault(std::complex<double>(real, imag));
}

void helicsInputSetDefaultVector(helics_input inp, const double* vectorInput, int vectorLength, helics_error* err)
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

void helicsInputSetDefaultNamedPoint(helics_input inp, const char* str, double val, helics_error* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    try {
        inpObj->inputPtr->setDefault(helics::NamedPoint(AS_STRING(str), val));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

const char* helicsInputGetType(helics_input inp)
{
    auto* inpObj = verifyInput(inp, nullptr);
    if (inpObj == nullptr) {
        return emptyStr.c_str();
    }

    try {
        return inpObj->inputPtr->getType().c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return emptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

const char* helicsInputGetPublicationType(helics_input ipt)
{
    auto* inpObj = verifyInput(ipt, nullptr);
    if (inpObj == nullptr) {
        return emptyStr.c_str();
    }

    try {
        return inpObj->inputPtr->getPublicationType().c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return emptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

const char* helicsPublicationGetType(helics_publication pub)
{
    auto* pubObj = verifyPublication(pub, nullptr);
    if (pubObj == nullptr) {
        return emptyStr.c_str();
    }

    try {
        return pubObj->pubPtr->getType().c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return emptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

const char* helicsInputGetKey(helics_input inp)
{
    auto* inpObj = verifyInput(inp, nullptr);
    if (inpObj == nullptr) {
        return emptyStr.c_str();
    }

    try {
        return inpObj->inputPtr->getKey().c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return emptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

const char* helicsSubscriptionGetKey(helics_input sub)
{
    auto* inpObj = verifyInput(sub, nullptr);
    if (inpObj == nullptr) {
        return emptyStr.c_str();
    }

    try {
        return inpObj->fedptr->getTarget(*(inpObj->inputPtr)).c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return emptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

const char* helicsPublicationGetKey(helics_publication pub)
{
    auto* pubObj = verifyPublication(pub, nullptr);
    if (pubObj == nullptr) {
        return emptyStr.c_str();
    }
    try {
        return pubObj->pubPtr->getKey().c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return emptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

const char* helicsInputGetInjectionUnits(helics_input ipt)
{
    auto* inpObj = verifyInput(ipt, nullptr);
    if (inpObj == nullptr) {
        return emptyStr.c_str();
    }
    try {
        return inpObj->inputPtr->getInjectionUnits().c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return emptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

const char* helicsInputGetExtractionUnits(helics_input ipt)
{
    auto* inpObj = verifyInput(ipt, nullptr);
    if (inpObj == nullptr) {
        return emptyStr.c_str();
    }
    try {
        return inpObj->inputPtr->getUnits().c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return emptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

const char* helicsInputGetUnits(helics_input inp)
{
    return helicsInputGetExtractionUnits(inp);
}

const char* helicsPublicationGetUnits(helics_publication pub)
{
    auto* pubObj = verifyPublication(pub, nullptr);
    if (pubObj == nullptr) {
        return emptyStr.c_str();
    }
    return pubObj->pubPtr->getUnits().c_str();
}

const char* helicsInputGetInfo(helics_input inp)
{
    auto* inpObj = verifyInput(inp, nullptr);
    if (inpObj == nullptr) {
        return emptyStr.c_str();
    }
    try {
        return inpObj->inputPtr->getInfo().c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return emptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

void helicsInputSetInfo(helics_input inp, const char* info, helics_error* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    try {
        inpObj->inputPtr->setInfo(AS_STRING(info));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}
const char* helicsPublicationGetInfo(helics_publication pub)
{
    auto* pubObj = verifyPublication(pub, nullptr);
    if (pubObj == nullptr) {
        return emptyStr.c_str();
    }
    try {
        return pubObj->pubPtr->getInfo().c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return emptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

void helicsPublicationSetInfo(helics_publication pub, const char* info, helics_error* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    try {
        pubObj->pubPtr->setInfo(AS_STRING(info));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

int helicsInputGetOption(helics_input inp, int option)
{
    auto* inpObj = verifyInput(inp, nullptr);
    if (inpObj == nullptr) {
        return helics_false;
    }
    try {
        return (inpObj->inputPtr->getOption(option));
    }
    // LCOV_EXCL_START
    catch (...) {
        return helics_false;
    }
    // LCOV_EXCL_STOP
}

void helicsInputSetOption(helics_input inp, int option, int value, helics_error* err)
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

int helicsPublicationGetOption(helics_publication pub, int option)
{
    auto* pubObj = verifyPublication(pub, nullptr);
    if (pubObj == nullptr) {
        return helics_false;
    }
    try {
        return pubObj->pubPtr->getOption(option);
    }
    // LCOV_EXCL_START
    catch (...) {
        return helics_false;
    }
    // LCOV_EXCL_STOP
}

void helicsPublicationSetOption(helics_publication pub, int option, int value, helics_error* err)
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

void helicsPublicationSetMinimumChange(helics_publication pub, double tolerance, helics_error* err)
{
    auto* pubObj = verifyPublication(pub, err);
    if (pubObj == nullptr) {
        return;
    }
    pubObj->pubPtr->setMinimumChange(tolerance);
}

void helicsInputSetMinimumChange(helics_input inp, double tolerance, helics_error* err)
{
    auto* inpObj = verifyInput(inp, err);
    if (inpObj == nullptr) {
        return;
    }
    inpObj->inputPtr->setMinimumChange(tolerance);
}

helics_bool helicsInputIsUpdated(helics_input inp)
{
    auto* inpObj = verifyInput(inp, nullptr);
    if (inpObj == nullptr) {
        return helics_false;
    }

    auto val = inpObj->inputPtr->isUpdated();
    return (val) ? helics_true : helics_false;
}

helics_time helicsInputLastUpdateTime(helics_input inp)
{
    auto* inpObj = verifyInput(inp, nullptr);
    if (inpObj == nullptr) {
        return helics_time_invalid;
    }
    try {
        auto time = inpObj->inputPtr->getLastUpdate();
        return static_cast<helics_time>(time);
    }
    // LCOV_EXCL_START
    catch (...) {
        return helics_time_invalid;
    }
    // LCOV_EXCL_STOP
}

void helicsInputClearUpdate(helics_input inp)
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
    }
    // LCOV_EXCL_STOP
}

int helicsFederateGetPublicationCount(helics_federate fed)
{
    // this call should be with a nullptr since it can fail and still be a successful call
    auto* vfedObj = getValueFed(fed, nullptr);
    if (vfedObj == nullptr) {
        return 0;
    }
    return static_cast<int>(vfedObj->getPublicationCount());
}

int helicsFederateGetInputCount(helics_federate fed)
{
    // this call should be with a nullptr since it can fail and still be a successful call
    auto* vfedObj = getValueFed(fed, nullptr);
    if (vfedObj == nullptr) {
        return 0;
    }
    return static_cast<int>(vfedObj->getInputCount());
}
