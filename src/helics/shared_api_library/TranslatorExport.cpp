/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../application_api/TranslatorOperations.hpp"
#include "../core/core-exceptions.hpp"
#include "../helics.hpp"
#include "MessageFederate.h"
#include "Translators.h"
#include "helicsCallbacks.h"
#include "helicsData.h"
#include "internal/api_objects.h"

#include <memory>
#include <mutex>
#include <string>
#include <utility>

static constexpr char invalidTranslatorString[] = "The given translator object is not valid";

/** this is a random identifier for validating translators*/
static const int translatorValidationIdentifier = 0xB37C'352E;

static auto translatorSearch = [](const helics::InterfaceHandle& hnd, const auto& testTranslator) {
    return hnd < testTranslator->transPtr->getHandle();
};

// fed is assumed to be valid here
static inline HelicsTranslator federateAddTranslator(HelicsFederate fed, std::unique_ptr<helics::TranslatorObject> trans)
{
    auto* fedObj = reinterpret_cast<helics::FedObject*>(fed);
    trans->valid = translatorValidationIdentifier;
    HelicsTranslator hTrans = trans.get();
    if (fedObj->translators.empty() || trans->transPtr->getHandle() > fedObj->translators.back()->transPtr->getHandle()) {
        fedObj->translators.push_back(std::move(trans));
    } else {
        auto ind = std::upper_bound(fedObj->translators.begin(), fedObj->translators.end(), trans->transPtr->getHandle(), translatorSearch);
        fedObj->translators.insert(ind, std::move(trans));
    }

    return hTrans;
}

static HelicsTranslator findOrCreateFederateTranslator(HelicsFederate fed, helics::Translator& translator)
{
    auto* fedObj = reinterpret_cast<helics::FedObject*>(fed);
    const auto handle = translator.getHandle();
    auto ind = std::upper_bound(fedObj->translators.begin(), fedObj->translators.end(), handle, translatorSearch);
    if (ind != fedObj->translators.end() && (*ind)->transPtr->getHandle() == handle) {
        HelicsTranslator hTrans = ind->get();
        return hTrans;
    }
    auto trans = std::make_unique<helics::TranslatorObject>();
    trans->transPtr = &translator;
    trans->fedptr = getFedSharedPtr(fed, nullptr);
    return federateAddTranslator(fed, std::move(trans));
}

/*
static HelicsTranslator findOrCreateCoreTranslator(HelicsCore core, helics::InterfaceHandle handle)
{
    auto* coreObj = reinterpret_cast<helics::CoreObject*>(core);
    auto ind = std::upper_bound(coreObj->translators.begin(), coreObj->translators.end(), handle, translatorSearch);
    if ((*ind)->transPtr->getHandle() == handle) {
        HelicsTranslator hTrans = ind->get();
        return hTrans;
    }
    return nullptr;
}
*/

static helics::TranslatorObject* getTranslatorObj(HelicsTranslator trans, HelicsError* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    if (trans == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidTranslatorString);
        return nullptr;
    }
    auto* tObj = reinterpret_cast<helics::TranslatorObject*>(trans);
    if (tObj->valid != translatorValidationIdentifier) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidTranslatorString);
        return nullptr;
    }
    return tObj;
}

// core is assumed to be valid here
static inline HelicsTranslator coreAddTranslator(HelicsCore core, std::unique_ptr<helics::TranslatorObject> trans)
{
    auto* coreObj = reinterpret_cast<helics::CoreObject*>(core);
    trans->valid = translatorValidationIdentifier;
    HelicsTranslator hTrans = trans.get();
    if (coreObj->translators.empty() || trans->transPtr->getHandle() > coreObj->translators.back()->transPtr->getHandle()) {
        coreObj->translators.push_back(std::move(trans));
    } else {
        auto ind =
            std::upper_bound(coreObj->translators.begin(), coreObj->translators.end(), trans->transPtr->getHandle(), translatorSearch);
        coreObj->translators.insert(ind, std::move(trans));
    }

    return hTrans;
}

HELICS_EXPORT HelicsTranslator helicsFederateRegisterTranslator(HelicsFederate fed,
                                                                HelicsTranslatorTypes type,
                                                                const char* name,
                                                                HelicsError* err)
{
    // now generate a generic subscription
    auto fedObj = getFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }

    try {
        auto trans = std::make_unique<helics::TranslatorObject>();
        trans->transPtr = &fedObj->registerTranslator(type, AS_STRING_VIEW(name));
        trans->fedptr = std::move(fedObj);
        trans->custom = (type == HELICS_TRANSLATOR_TYPE_CUSTOM);
        return federateAddTranslator(fed, std::move(trans));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

HelicsTranslator helicsFederateRegisterGlobalTranslator(HelicsFederate fed, HelicsTranslatorTypes type, const char* name, HelicsError* err)
{
    auto fedObj = getFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }

    try {
        auto trans = std::make_unique<helics::TranslatorObject>();
        trans->transPtr = &fedObj->registerGlobalTranslator(type, AS_STRING_VIEW(name));
        trans->fedptr = std::move(fedObj);
        trans->custom = (type == HELICS_TRANSLATOR_TYPE_CUSTOM);
        return federateAddTranslator(fed, std::move(trans));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

HelicsTranslator helicsCoreRegisterTranslator(HelicsCore cr, HelicsTranslatorTypes type, const char* name, HelicsError* err)
{
    auto core = getCoreSharedPtr(cr, err);
    if (!core) {
        return nullptr;
    }
    try {
        auto trans = std::make_unique<helics::TranslatorObject>();

        trans->mTrans = std::make_unique<helics::Translator>(core.get(), AS_STRING_VIEW(name));
        trans->mTrans->setTranslatorType(type);
        trans->transPtr = trans->mTrans.get();
        trans->corePtr = std::move(core);
        trans->custom = (type == HELICS_TRANSLATOR_TYPE_CUSTOM);
        return coreAddTranslator(cr, std::move(trans));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

static constexpr char invalidTransName[] = "the specified Translator name is not recognized";
static constexpr char invalidTransIndex[] = "the specified Translator index is not valid";

HelicsTranslator helicsFederateGetTranslator(HelicsFederate fed, const char* name, HelicsError* err)
{
    auto fedObj = getFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    CHECK_NULL_STRING(name, nullptr);
    try {
        auto& id = fedObj->getTranslator(name);
        if (!id.isValid()) {
            err->error_code = HELICS_ERROR_INVALID_ARGUMENT;
            err->message = invalidTransName;
            return nullptr;
        }
        return findOrCreateFederateTranslator(fed, id);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    // LCOV_EXCL_STOP
}

int helicsFederateGetTranslatorCount(HelicsFederate fed)
{
    auto* fedObj = getFed(fed, nullptr);
    if (fedObj == nullptr) {
        return 0;
    }
    return fedObj->getTranslatorCount();
}

HelicsTranslator helicsFederateGetTranslatorByIndex(HelicsFederate fed, int index, HelicsError* err)
{
    auto fedObj = getFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto& id = fedObj->getTranslator(index);
        if (!id.isValid()) {
            err->error_code = HELICS_ERROR_INVALID_ARGUMENT;
            err->message = invalidTransIndex;
            return nullptr;
        }
        return findOrCreateFederateTranslator(fed, id);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    // LCOV_EXCL_STOP
}

static helics::Translator* getTranslator(HelicsTranslator trans, HelicsError* err)
{
    auto* fObj = getTranslatorObj(trans, err);
    if (fObj == nullptr) {
        return nullptr;
    }
    return fObj->transPtr;
}

HelicsBool helicsTranslatorIsValid(HelicsTranslator trans)
{
    auto* translator = getTranslator(trans, nullptr);
    if (translator == nullptr) {
        return HELICS_FALSE;
    }
    return (translator->isValid()) ? HELICS_TRUE : HELICS_FALSE;
}

/** get the name of the translator*/
const char* helicsTranslatorGetName(HelicsTranslator trans)
{
    auto* translator = getTranslator(trans, nullptr);
    if (translator == nullptr) {
        return gHelicsEmptyStr.c_str();
    }
    const auto& name = translator->getName();
    return name.c_str();
}

void helicsTranslatorSet(HelicsTranslator trans, const char* prop, double val, HelicsError* err)
{
    auto* translator = getTranslator(trans, err);
    if (translator == nullptr) {
        return;
    }
    CHECK_NULL_STRING(prop, void());
    try {
        translator->set(prop, val);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsTranslatorSetString(HelicsTranslator trans, const char* prop, const char* val, HelicsError* err)
{
    auto* translator = getTranslator(trans, err);
    if (translator == nullptr) {
        return;
    }
    CHECK_NULL_STRING(prop, void());
    try {
        translator->setString(prop, val);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsTranslatorAddInputTarget(HelicsTranslator trans, const char* inp, HelicsError* err)
{
    auto* translator = getTranslator(trans, err);
    if (translator == nullptr) {
        return;
    }
    CHECK_NULL_STRING(inp, void());
    try {
        translator->addInputTarget(inp);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsTranslatorAddPublicationTarget(HelicsTranslator trans, const char* pub, HelicsError* err)
{
    auto* translator = getTranslator(trans, err);
    if (translator == nullptr) {
        return;
    }
    CHECK_NULL_STRING(pub, void());
    try {
        translator->addPublication(pub);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsTranslatorAddDestinationEndpoint(HelicsTranslator trans, const char* ept, HelicsError* err)
{
    auto* translator = getTranslator(trans, err);
    if (translator == nullptr) {
        return;
    }
    CHECK_NULL_STRING(ept, void());
    try {
        translator->addDestinationEndpoint(ept);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsTranslatorAddSourceEndpoint(HelicsTranslator trans, const char* ept, HelicsError* err)
{
    auto* translator = getTranslator(trans, err);
    if (translator == nullptr) {
        return;
    }
    CHECK_NULL_STRING(ept, void());
    try {
        translator->addSourceEndpoint(ept);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsTranslatorRemoveTarget(HelicsTranslator trans, const char* target, HelicsError* err)
{
    auto* translator = getTranslator(trans, err);
    if (translator == nullptr) {
        return;
    }
    CHECK_NULL_STRING(target, void());
    try {
        translator->removeTarget(target);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

const char* helicsTranslatorGetInfo(HelicsTranslator trans)
{
    auto* transObj = getTranslatorObj(trans, nullptr);
    if (transObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }
    try {
        const std::string& info = transObj->transPtr->getInfo();
        return info.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return gHelicsEmptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

void helicsTranslatorSetInfo(HelicsTranslator trans, const char* info, HelicsError* err)
{
    auto* transObj = getTranslatorObj(trans, err);
    if (transObj == nullptr) {
        return;
    }
    try {
        transObj->transPtr->setInfo(AS_STRING_VIEW(info));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

const char* helicsTranslatorGetTag(HelicsTranslator trans, const char* tagname)
{
    auto* transObj = getTranslatorObj(trans, nullptr);
    if (transObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }
    try {
        const std::string& info = transObj->transPtr->getTag(AS_STRING_VIEW(tagname));
        return info.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return gHelicsEmptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

void helicsTranslatorSetTag(HelicsTranslator trans, const char* tagname, const char* tagvalue, HelicsError* err)
{
    auto* transObj = getTranslatorObj(trans, err);
    if (transObj == nullptr) {
        return;
    }
    try {
        transObj->transPtr->setTag(AS_STRING_VIEW(tagname), AS_STRING_VIEW(tagvalue));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsTranslatorSetOption(HelicsTranslator trans, int option, int value, HelicsError* err)
{
    auto* transObj = getTranslatorObj(trans, err);
    if (transObj == nullptr) {
        return;
    }
    try {
        transObj->transPtr->setOption(option, value);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

int helicsTranslatorGetOption(HelicsTranslator trans, int option)
{
    auto* transObj = getTranslatorObj(trans, nullptr);
    if (transObj == nullptr) {
        return HELICS_FALSE;
    }
    try {
        return transObj->transPtr->getOption(option);
    }
    // LCOV_EXCL_START
    catch (...) {
        return HELICS_FALSE;
    }
    // LCOV_EXCL_STOP
}

HELICS_EXPORT void helicsTranslatorSetCustomCallback(HelicsTranslator translator,
                                                     void (*toMessageCall)(HelicsDataBuffer value, HelicsMessage message, void* userData),
                                                     void (*toValueCall)(HelicsMessage message, HelicsDataBuffer value, void* userData),
                                                     void* userdata,
                                                     HelicsError* err)
{
    auto* fObj = getTranslatorObj(translator, err);
    if (fObj == nullptr || fObj->transPtr == nullptr) {
        return;
    }

    if (!fObj->custom) {
        static constexpr char nonCustomTranslatorString[] = "Translator must be a custom Translator to specify callback";
        assignError(err, HELICS_ERROR_INVALID_OBJECT, nonCustomTranslatorString);
        return;
    }
    auto op = std::make_shared<helics::CustomTranslatorOperator>();

    op->setToMessageFunction([userdata, toMessageCall](helics::SmallBuffer data) {
        std::unique_ptr<helics::Message> mess = std::make_unique<helics::Message>();

        auto* buff = createAPIDataBuffer(data);
        auto* mm = createAPIMessage(mess);
        toMessageCall(buff, mm, userdata);
        return mess;
    });

    op->setToValueFunction([userdata, toValueCall](std::unique_ptr<helics::Message> mess) {
        helics::SmallBuffer data;

        auto* buff = createAPIDataBuffer(data);
        auto* mm = createAPIMessage(mess);
        toValueCall(mm, buff, userdata);
        return data;
    });

    try {
        fObj->transPtr->setOperator(std::move(op));
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}
