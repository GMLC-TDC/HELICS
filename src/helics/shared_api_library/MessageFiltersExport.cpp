/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../core/core-exceptions.hpp"
#include "../helics.hpp"
#include "MessageFederate.h"
#include "MessageFilters.h"
#include "helicsCallbacks.h"
#include "internal/api_objects.h"

#include <memory>
#include <mutex>

static constexpr char invalidFilterString[] = "The given filter object is not valid";

/** this is a random identifier put in place when the federate or core or broker gets created*/
static const int filterValidationIdentifier = 0xEC26'0127;

static helics::FilterObject* getFilterObj(helics_filter filt, helics_error* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    if (filt == nullptr) {
        assignError(err, helics_error_invalid_object, invalidFilterString);
        return nullptr;
    }
    auto* fObj = reinterpret_cast<helics::FilterObject*>(filt);
    if (fObj->valid != filterValidationIdentifier) {
        assignError(err, helics_error_invalid_object, invalidFilterString);
        return nullptr;
    }
    return fObj;
}

// fed is assumed to be valid here
static inline helics_filter federateAddFilter(helics_federate fed, std::unique_ptr<helics::FilterObject> filt)
{
    auto* fedObj = reinterpret_cast<helics::FedObject*>(fed);
    filt->valid = filterValidationIdentifier;
    helics_filter ret = filt.get();
    fedObj->filters.push_back(std::move(filt));
    return ret;
}

// core is assumed to be valid here
static inline helics_filter coreAddFilter(helics_core core, std::unique_ptr<helics::FilterObject> filt)
{
    auto* coreObj = reinterpret_cast<helics::CoreObject*>(core);
    filt->valid = filterValidationIdentifier;
    helics_filter ret = filt.get();
    coreObj->filters.push_back(std::move(filt));
    return ret;
}

helics_filter helicsFederateRegisterFilter(helics_federate fed, helics_filter_type type, const char* name, helics_error* err)
{
    // now generate a generic subscription
    auto fedObj = getFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }

    try {
        auto filt = std::make_unique<helics::FilterObject>();
        filt->filtPtr = &helics::make_filter(static_cast<helics::filter_types>(type), fedObj.get(), AS_STRING(name));
        filt->fedptr = std::move(fedObj);
        filt->custom = (type == helics_filter_type_custom);
        return federateAddFilter(fed, std::move(filt));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

helics_filter helicsFederateRegisterGlobalFilter(helics_federate fed, helics_filter_type type, const char* name, helics_error* err)
{
    // now generate a generic subscription
    auto fedObj = getFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }

    try {
        auto filt = std::make_unique<helics::FilterObject>();
        filt->filtPtr = &helics::make_filter(helics::interface_visibility::global,
                                             static_cast<helics::filter_types>(type),
                                             fedObj.get(),
                                             AS_STRING(name));
        filt->fedptr = std::move(fedObj);
        filt->custom = (type == helics_filter_type_custom);
        return federateAddFilter(fed, std::move(filt));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

helics_filter helicsCoreRegisterFilter(helics_core cr, helics_filter_type type, const char* name, helics_error* err)
{
    auto core = getCoreSharedPtr(cr, err);
    if (!core) {
        return nullptr;
    }
    try {
        auto filt = std::make_unique<helics::FilterObject>();
        filt->uFilter = helics::make_filter(static_cast<helics::filter_types>(type), core.get(), AS_STRING(name));
        filt->filtPtr = filt->uFilter.get();
        filt->corePtr = std::move(core);
        filt->custom = (type == helics_filter_type_custom);
        return coreAddFilter(cr, std::move(filt));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

helics_filter helicsFederateRegisterCloningFilter(helics_federate fed, const char* name, helics_error* err)
{
    auto fedObj = getFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }

    try {
        auto filt = std::make_unique<helics::FilterObject>();
        filt->filtPtr = &helics::make_cloning_filter(helics::filter_types::clone, fedObj.get(), std::string{}, AS_STRING(name));
        filt->fedptr = std::move(fedObj);
        filt->cloning = true;
        return federateAddFilter(fed, std::move(filt));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

helics_filter helicsFederateRegisterGlobalCloningFilter(helics_federate fed, const char* name, helics_error* err)
{
    auto fedObj = getFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }

    try {
        auto filt = std::make_unique<helics::FilterObject>();
        filt->filtPtr =
            &helics::make_cloning_filter(helics::GLOBAL, helics::filter_types::clone, fedObj.get(), std::string{}, AS_STRING(name));
        filt->fedptr = std::move(fedObj);
        filt->cloning = true;
        return federateAddFilter(fed, std::move(filt));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

helics_filter helicsCoreRegisterCloningFilter(helics_core cr, const char* name, helics_error* err)
{
    auto core = getCoreSharedPtr(cr, err);
    if (!core) {
        return nullptr;
    }
    try {
        auto filt = std::make_unique<helics::FilterObject>();
        filt->uFilter = helics::make_cloning_filter(helics::filter_types::clone, core.get(), std::string{}, AS_STRING(name));
        filt->filtPtr = filt->uFilter.get();
        filt->corePtr = std::move(core);
        filt->cloning = true;
        return coreAddFilter(cr, std::move(filt));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

static constexpr char invalidFiltName[] = "the specified Filter name is not recognized";
static constexpr char invalidFiltIndex[] = "the specified Filter index is not valid";

helics_filter helicsFederateGetFilter(helics_federate fed, const char* name, helics_error* err)
{
    auto fedObj = getFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    CHECK_NULL_STRING(name, nullptr);
    try {
        auto& id = fedObj->getFilter(name);
        if (!id.isValid()) {
            err->error_code = helics_error_invalid_argument;
            err->message = invalidFiltName;
            return nullptr;
        }
        auto filt = std::make_unique<helics::FilterObject>();
        filt->filtPtr = &id;
        filt->cloning = id.isCloningFilter();
        filt->fedptr = std::move(fedObj);
        return federateAddFilter(fed, std::move(filt));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    // LCOV_EXCL_STOP
}

int helicsFederateGetFilterCount(helics_federate fed)
{
    auto* fedObj = getFed(fed, nullptr);
    if (fedObj == nullptr) {
        return 0;
    }
    return fedObj->getFilterCount();
}

helics_filter helicsFederateGetFilterByIndex(helics_federate fed, int index, helics_error* err)
{
    auto fedObj = getFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto& id = fedObj->getFilter(index);
        if (!id.isValid()) {
            err->error_code = helics_error_invalid_argument;
            err->message = invalidFiltIndex;
            return nullptr;
        }
        auto filt = std::make_unique<helics::FilterObject>();
        filt->filtPtr = &id;
        filt->fedptr = std::move(fedObj);
        filt->cloning = id.isCloningFilter();
        return federateAddFilter(fed, std::move(filt));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    // LCOV_EXCL_STOP
}

static helics::Filter* getFilter(helics_filter filt, helics_error* err)
{
    auto* fObj = getFilterObj(filt, err);
    if (fObj == nullptr) {
        return nullptr;
    }
    return fObj->filtPtr;
}

static helics::CloningFilter* getCloningFilter(helics_filter filt, helics_error* err)
{
    auto* fObj = getFilterObj(filt, err);
    if (fObj == nullptr) {
        return nullptr;
    }
    if (!fObj->cloning) {
        static constexpr char nonCloningFilterString[] = "filter must be a cloning filter";
        assignError(err, helics_error_invalid_object, nonCloningFilterString);
        return nullptr;
    }
    return dynamic_cast<helics::CloningFilter*>(fObj->filtPtr);
}

helics_bool helicsFilterIsValid(helics_filter filt)
{
    auto* filter = getFilter(filt, nullptr);
    if (filter == nullptr) {
        return helics_false;
    }
    return (filter->isValid()) ? helics_true : helics_false;
}

/** get the name of the filter*/
const char* helicsFilterGetName(helics_filter filt)
{
    auto* filter = getFilter(filt, nullptr);
    if (filter == nullptr) {
        return emptyStr.c_str();
    }
    const auto& name = filter->getName();
    return name.c_str();
}

void helicsFilterSet(helics_filter filt, const char* prop, double val, helics_error* err)
{
    auto* filter = getFilter(filt, err);
    if (filter == nullptr) {
        return;
    }
    CHECK_NULL_STRING(prop, void());
    try {
        filter->set(prop, val);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsFilterSetString(helics_filter filt, const char* prop, const char* val, helics_error* err)
{
    auto* filter = getFilter(filt, err);
    if (filter == nullptr) {
        return;
    }
    CHECK_NULL_STRING(prop, void());
    try {
        filter->setString(prop, val);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsFilterAddDestinationTarget(helics_filter filt, const char* dest, helics_error* err)
{
    auto* filter = getFilter(filt, err);
    if (filter == nullptr) {
        return;
    }
    CHECK_NULL_STRING(dest, void());
    try {
        filter->addDestinationTarget(dest);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsFilterAddSourceTarget(helics_filter filt, const char* src, helics_error* err)
{
    auto* filter = getFilter(filt, err);
    if (filter == nullptr) {
        return;
    }
    CHECK_NULL_STRING(src, void());
    try {
        filter->addSourceTarget(src);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsFilterAddDeliveryEndpoint(helics_filter filt, const char* delivery, helics_error* err)
{
    auto* filter = getCloningFilter(filt, err);
    if (filter == nullptr) {
        return;
    }
    CHECK_NULL_STRING(delivery, void());
    try {
        filter->addDeliveryEndpoint(delivery);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsFilterRemoveTarget(helics_filter filt, const char* target, helics_error* err)
{
    auto* filter = getFilter(filt, err);
    if (filter == nullptr) {
        return;
    }
    CHECK_NULL_STRING(target, void());
    try {
        filter->removeTarget(target);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsFilterRemoveDeliveryEndpoint(helics_filter filt, const char* delivery, helics_error* err)
{
    auto* filter = getCloningFilter(filt, err);
    if (filter == nullptr) {
        return;
    }
    CHECK_NULL_STRING(delivery, void());
    try {
        filter->removeDeliveryEndpoint(delivery);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

const char* helicsFilterGetInfo(helics_filter filt)
{
    auto* filtObj = getFilterObj(filt, nullptr);
    if (filtObj == nullptr) {
        return emptyStr.c_str();
    }
    try {
        return filtObj->filtPtr->getInfo().c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return emptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

void helicsFilterSetInfo(helics_filter filt, const char* info, helics_error* err)
{
    auto* filtObj = getFilterObj(filt, err);
    if (filtObj == nullptr) {
        return;
    }
    try {
        filtObj->filtPtr->setInfo(AS_STRING(info));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsFilterSetOption(helics_filter filt, int option, int value, helics_error* err)
{
    auto* filtObj = getFilterObj(filt, err);
    if (filtObj == nullptr) {
        return;
    }
    try {
        filtObj->filtPtr->setOption(option, value);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

int helicsFilterGetOption(helics_filter filt, int option)
{
    auto* filtObj = getFilterObj(filt, nullptr);
    if (filtObj == nullptr) {
        return helics_false;
    }
    try {
        return filtObj->filtPtr->getOption(option);
    }
    // LCOV_EXCL_START
    catch (...) {
        return helics_false;
    }
    // LCOV_EXCL_STOP
}

void helicsFilterSetCustomCallback(helics_filter filt,
                                   void (*filtCall)(helics_message_object message, void* userData),
                                   void* userdata,
                                   helics_error* err)
{
    auto* fObj = getFilterObj(filt, err);
    if (fObj == nullptr || fObj->filtPtr == nullptr) {
        return;
    }

    if (!fObj->custom) {
        static constexpr char nonCustomFilterString[] = "filter must be a custom filter to specify callback";
        assignError(err, helics_error_invalid_object, nonCustomFilterString);
        return;
    }
    auto op = std::make_shared<helics::CustomMessageOperator>();
    op->setMessageFunction([filtCall, userdata](std::unique_ptr<helics::Message> message) {
        auto* ms = createMessageObject(message);
        if (filtCall != nullptr) {
            filtCall(ms, userdata);
        }
        return message;
    });
    try {
        fObj->filtPtr->setOperator(std::move(op));
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}
