/*
Copyright (c) 2017-2024,
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
#include <string>
#include <utility>

static constexpr char invalidFilterString[] = "The given filter object is not valid";

/** this is a random identifier put in place for validating filters*/
static const int filterValidationIdentifier = 0xEC26'0127;

namespace {
auto filterSearch = [](const helics::InterfaceHandle& hnd, const auto& testFilter) { return hnd < testFilter->filtPtr->getHandle(); };

HelicsFilter federateAddFilter(HelicsFederate fed, std::unique_ptr<helics::FilterObject> filt)
{
    auto* fedObj = reinterpret_cast<helics::FedObject*>(fed);
    filt->valid = filterValidationIdentifier;
    HelicsFilter hfilt = filt.get();
    if (fedObj->filters.empty() || filt->filtPtr->getHandle() > fedObj->filters.back()->filtPtr->getHandle()) {
        fedObj->filters.push_back(std::move(filt));
    } else {
        auto ind = std::upper_bound(fedObj->filters.begin(), fedObj->filters.end(), filt->filtPtr->getHandle(), filterSearch);
        fedObj->filters.insert(ind, std::move(filt));
    }
    return hfilt;
}

HelicsFilter findOrCreateFederateFilter(HelicsFederate fed, helics::Filter& filter)
{
    auto* fedObj = reinterpret_cast<helics::FedObject*>(fed);
    const auto handle = filter.getHandle();
    auto ind = std::upper_bound(fedObj->filters.begin(), fedObj->filters.end(), handle, filterSearch);
    if (ind != fedObj->filters.end() && (*ind)->filtPtr->getHandle() == handle) {
        HelicsFilter hfilt = ind->get();
        return hfilt;
    }

    auto filt = std::make_unique<helics::FilterObject>();
    filt->filtPtr = &filter;
    filt->cloning = filter.isCloningFilter();
    filt->fedptr = getFedSharedPtr(fed, nullptr);
    return federateAddFilter(fed, std::move(filt));
}

/*
HelicsFilter findCoreFilter(HelicsCore core, helics::InterfaceHandle handle)
{
    auto* coreObj = reinterpret_cast<helics::CoreObject*>(core);
    auto ind=std::upper_bound(coreObj->filters.begin(),coreObj->filters.end(),handle,filterSearch);
    if ((*ind)->filtPtr->getHandle() == handle)
    {
        HelicsFilter hfilt=ind->get();
        return hfilt;
    }
    return nullptr;
}
*/

helics::FilterObject* getFilterObj(HelicsFilter filt, HelicsError* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    if (filt == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidFilterString);
        return nullptr;
    }
    auto* fObj = reinterpret_cast<helics::FilterObject*>(filt);
    if (fObj->valid != filterValidationIdentifier) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidFilterString);
        return nullptr;
    }
    return fObj;
}

// core is assumed to be valid here
HelicsFilter coreAddFilter(HelicsCore core, std::unique_ptr<helics::FilterObject> filt)
{
    auto* coreObj = reinterpret_cast<helics::CoreObject*>(core);
    filt->valid = filterValidationIdentifier;
    HelicsFilter hfilt = filt.get();
    if (coreObj->filters.empty() || filt->filtPtr->getHandle() > coreObj->filters.back()->filtPtr->getHandle()) {
        coreObj->filters.push_back(std::move(filt));
    } else {
        auto ind = std::upper_bound(coreObj->filters.begin(), coreObj->filters.end(), filt->filtPtr->getHandle(), filterSearch);
        coreObj->filters.insert(ind, std::move(filt));
    }
    return hfilt;
}
}  // namespace
HelicsFilter helicsFederateRegisterFilter(HelicsFederate fed, HelicsFilterTypes type, const char* name, HelicsError* err)
{
    // now generate a generic subscription
    auto fedObj = getFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }

    try {
        auto filt = std::make_unique<helics::FilterObject>();
        filt->filtPtr = &helics::make_filter(static_cast<helics::FilterTypes>(type), fedObj.get(), AS_STRING(name));
        filt->fedptr = std::move(fedObj);
        filt->custom = (type == HELICS_FILTER_TYPE_CUSTOM);
        return federateAddFilter(fed, std::move(filt));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

HelicsFilter helicsFederateRegisterGlobalFilter(HelicsFederate fed, HelicsFilterTypes type, const char* name, HelicsError* err)
{
    auto fedObj = getFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }

    try {
        auto filt = std::make_unique<helics::FilterObject>();
        filt->filtPtr = &helics::make_filter(helics::InterfaceVisibility::GLOBAL,
                                             static_cast<helics::FilterTypes>(type),
                                             fedObj.get(),
                                             AS_STRING(name));
        filt->fedptr = std::move(fedObj);
        filt->custom = (type == HELICS_FILTER_TYPE_CUSTOM);
        return federateAddFilter(fed, std::move(filt));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

HelicsFilter helicsCoreRegisterFilter(HelicsCore core, HelicsFilterTypes type, const char* name, HelicsError* err)
{
    auto corePtr = getCoreSharedPtr(core, err);
    if (!corePtr) {
        return nullptr;
    }
    try {
        auto filt = std::make_unique<helics::FilterObject>();
        filt->uFilter = helics::make_filter(static_cast<helics::FilterTypes>(type), corePtr.get(), AS_STRING(name));
        filt->filtPtr = filt->uFilter.get();
        filt->corePtr = std::move(corePtr);
        filt->custom = (type == HELICS_FILTER_TYPE_CUSTOM);
        return coreAddFilter(core, std::move(filt));
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
}

HelicsFilter helicsFederateRegisterCloningFilter(HelicsFederate fed, const char* name, HelicsError* err)
{
    auto fedObj = getFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }

    try {
        auto filt = std::make_unique<helics::FilterObject>();
        filt->filtPtr = &helics::make_cloning_filter(helics::FilterTypes::CLONE, fedObj.get(), std::string{}, AS_STRING(name));
        filt->fedptr = std::move(fedObj);
        filt->cloning = true;
        return federateAddFilter(fed, std::move(filt));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

HelicsFilter helicsFederateRegisterGlobalCloningFilter(HelicsFederate fed, const char* name, HelicsError* err)
{
    auto fedObj = getFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }

    try {
        auto filt = std::make_unique<helics::FilterObject>();
        filt->filtPtr = &helics::make_cloning_filter(
            helics::InterfaceVisibility::GLOBAL, helics::FilterTypes::CLONE, fedObj.get(), std::string{}, AS_STRING(name));
        filt->fedptr = std::move(fedObj);
        filt->cloning = true;
        return federateAddFilter(fed, std::move(filt));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

HelicsFilter helicsCoreRegisterCloningFilter(HelicsCore core, const char* name, HelicsError* err)
{
    auto corePtr = getCoreSharedPtr(core, err);
    if (!corePtr) {
        return nullptr;
    }
    try {
        auto filt = std::make_unique<helics::FilterObject>();
        filt->uFilter = helics::make_cloning_filter(helics::FilterTypes::CLONE, corePtr.get(), std::string{}, AS_STRING(name));
        filt->filtPtr = filt->uFilter.get();
        filt->corePtr = std::move(corePtr);
        filt->cloning = true;
        return coreAddFilter(core, std::move(filt));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

static constexpr char invalidFiltName[] = "the specified Filter name is not recognized";
static constexpr char invalidFiltIndex[] = "the specified Filter index is not valid";

HelicsFilter helicsFederateGetFilter(HelicsFederate fed, const char* name, HelicsError* err)
{
    auto fedObj = getFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    CHECK_NULL_STRING(name, nullptr);
    try {
        auto& filt = fedObj->getFilter(name);
        if (!filt.isValid()) {
            err->error_code = HELICS_ERROR_INVALID_ARGUMENT;
            err->message = invalidFiltName;
            return nullptr;
        }
        return findOrCreateFederateFilter(fed, filt);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    // LCOV_EXCL_STOP
}

int helicsFederateGetFilterCount(HelicsFederate fed)
{
    auto* fedObj = getFed(fed, nullptr);
    if (fedObj == nullptr) {
        return 0;
    }
    return fedObj->getFilterCount();
}

HelicsFilter helicsFederateGetFilterByIndex(HelicsFederate fed, int index, HelicsError* err)
{
    auto fedObj = getFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto& filt = fedObj->getFilter(index);
        if (!filt.isValid()) {
            err->error_code = HELICS_ERROR_INVALID_ARGUMENT;
            err->message = invalidFiltIndex;
            return nullptr;
        }
        return findOrCreateFederateFilter(fed, filt);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    // LCOV_EXCL_STOP
}

namespace {
helics::Filter* getFilter(HelicsFilter filt, HelicsError* err)
{
    auto* fObj = getFilterObj(filt, err);
    if (fObj == nullptr) {
        return nullptr;
    }
    return fObj->filtPtr;
}

helics::CloningFilter* getCloningFilter(HelicsFilter filt, HelicsError* err)
{
    auto* fObj = getFilterObj(filt, err);
    if (fObj == nullptr) {
        return nullptr;
    }
    if (!fObj->cloning) {
        static constexpr char nonCloningFilterString[] = "filter must be a cloning filter";
        assignError(err, HELICS_ERROR_INVALID_OBJECT, nonCloningFilterString);
        return nullptr;
    }
    return dynamic_cast<helics::CloningFilter*>(fObj->filtPtr);
}
}  // namespace
HelicsBool helicsFilterIsValid(HelicsFilter filt)
{
    auto* filter = getFilter(filt, nullptr);
    if (filter == nullptr) {
        return HELICS_FALSE;
    }
    return (filter->isValid()) ? HELICS_TRUE : HELICS_FALSE;
}

/** get the name of the filter*/
const char* helicsFilterGetName(HelicsFilter filt)
{
    auto* filter = getFilter(filt, nullptr);
    if (filter == nullptr) {
        return gHelicsEmptyStr.c_str();
    }
    const auto& name = filter->getName();
    return name.c_str();
}

void helicsFilterSet(HelicsFilter filt, const char* prop, double val, HelicsError* err)
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

void helicsFilterSetString(HelicsFilter filt, const char* prop, const char* val, HelicsError* err)
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

double helicsFilterGetPropertyDouble(HelicsFilter filt, const char* prop)
{
    auto* filter = getFilter(filt, nullptr);
    if (filter == nullptr || prop == nullptr) {
        return HELICS_TIME_INVALID;
    }
    try {
        return filter->getProperty(prop);
    }
    catch (...) {
        return HELICS_TIME_INVALID;
    }
}

const char* helicsFilterGetPropertyString(HelicsFilter filt, const char* prop)
{
    static constexpr char nullStr[] = "";
    auto* fObj = getFilterObj(filt, nullptr);
    if (fObj == nullptr || prop == nullptr) {
        return nullStr;
    }
    try {
        auto res = fObj->filtPtr->getString(prop);
        if (res.empty()) {
            return nullStr;
        }
        fObj->buffer = res;
        return fObj->buffer.c_str();
    }
    catch (...) {
        return nullStr;
    }
}

void helicsFilterAddDestinationTarget(HelicsFilter filt, const char* dest, HelicsError* err)
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

void helicsFilterAddSourceTarget(HelicsFilter filt, const char* src, HelicsError* err)
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

void helicsFilterAddDeliveryEndpoint(HelicsFilter filt, const char* delivery, HelicsError* err)
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

void helicsFilterRemoveTarget(HelicsFilter filt, const char* target, HelicsError* err)
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

void helicsFilterRemoveDeliveryEndpoint(HelicsFilter filt, const char* delivery, HelicsError* err)
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

const char* helicsFilterGetInfo(HelicsFilter filt)
{
    auto* filtObj = getFilterObj(filt, nullptr);
    if (filtObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }
    try {
        const std::string& info = filtObj->filtPtr->getInfo();
        return info.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return gHelicsEmptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

void helicsFilterSetInfo(HelicsFilter filt, const char* info, HelicsError* err)
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

const char* helicsFilterGetTag(HelicsFilter filt, const char* tagname)
{
    auto* filtObj = getFilterObj(filt, nullptr);
    if (filtObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }
    try {
        const std::string& info = filtObj->filtPtr->getTag(AS_STRING(tagname));
        return info.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return gHelicsEmptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

void helicsFilterSetTag(HelicsFilter filt, const char* tagname, const char* tagvalue, HelicsError* err)
{
    auto* filtObj = getFilterObj(filt, err);
    if (filtObj == nullptr) {
        return;
    }
    try {
        filtObj->filtPtr->setTag(AS_STRING(tagname), AS_STRING(tagvalue));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsFilterSetOption(HelicsFilter filt, int option, int value, HelicsError* err)
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

int helicsFilterGetOption(HelicsFilter filt, int option)
{
    auto* filtObj = getFilterObj(filt, nullptr);
    if (filtObj == nullptr) {
        return HELICS_FALSE;
    }
    try {
        return filtObj->filtPtr->getOption(option);
    }
    // LCOV_EXCL_START
    catch (...) {
        return HELICS_FALSE;
    }
    // LCOV_EXCL_STOP
}

void helicsFilterSetCustomCallback(HelicsFilter filt,
                                   HelicsMessage (*filtCall)(HelicsMessage message, void* userData),
                                   void* userdata,
                                   HelicsError* err)
{
    auto* fObj = getFilterObj(filt, err);
    if (fObj == nullptr || fObj->filtPtr == nullptr) {
        return;
    }

    if (!fObj->custom) {
        static constexpr char nonCustomFilterString[] = "FILTER must be a custom FILTER to specify callback";
        assignError(err, HELICS_ERROR_INVALID_OBJECT, nonCustomFilterString);
        return;
    }
    auto customOp = std::make_shared<helics::CustomMessageOperator>();
    customOp->setMessageFunction([filtCall, userdata](std::unique_ptr<helics::Message> message) {
        HelicsMessage messageObj = createAPIMessage(message);
        if (filtCall != nullptr) {
            messageObj = filtCall(messageObj, userdata);
        }
        if (messageObj != nullptr && reinterpret_cast<helics::Message*>(messageObj) != message.get()) {
            return getMessageUniquePtr(messageObj, nullptr);
        }
        return message;
    });
    try {
        fObj->filtPtr->setOperator(std::move(customOp));
    }
    catch (...) {  // LCOV_EXCL_LINE
        helicsErrorHandler(err);  // LCOV_EXCL_LINE
    }
}
