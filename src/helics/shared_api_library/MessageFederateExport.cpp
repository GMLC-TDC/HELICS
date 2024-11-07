/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../core/core-exceptions.hpp"
#include "../core/flagOperations.hpp"
#include "../helics.hpp"
#include "MessageFederate.h"
#include "helicsCore.h"
#include "internal/api_objects.h"

#include <algorithm>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace {
// random integer for validation purposes of endpoints
constexpr int EndpointValidationIdentifier = 0xB453'94C2;

auto endpointSearch = [](const helics::InterfaceHandle& hnd, const auto& testEndpoint) { return hnd < testEndpoint->endPtr->getHandle(); };

inline HelicsEndpoint addEndpoint(HelicsFederate fed, std::unique_ptr<helics::EndpointObject> ept)
{
    auto* fedObj = reinterpret_cast<helics::FedObject*>(fed);
    ept->valid = EndpointValidationIdentifier;
    ept->fed = fedObj;
    HelicsEndpoint hept = ept.get();

    if (fedObj->epts.empty() || ept->endPtr->getHandle() > fedObj->epts.back()->endPtr->getHandle()) {
        fedObj->epts.push_back(std::move(ept));
    } else {
        auto ind = std::upper_bound(fedObj->epts.begin(), fedObj->epts.end(), ept->endPtr->getHandle(), endpointSearch);
        fedObj->epts.insert(ind, std::move(ept));
    }
    return hept;
}

HelicsEndpoint findOrCreateEndpoint(HelicsFederate fed, helics::Endpoint& endp)
{
    auto* fedObj = reinterpret_cast<helics::FedObject*>(fed);
    const auto handle = endp.getHandle();
    auto ind = std::upper_bound(fedObj->epts.begin(), fedObj->epts.end(), handle, endpointSearch);
    if (ind != fedObj->epts.end() && (*ind)->endPtr->getHandle() == handle) {
        HelicsEndpoint hend = ind->get();
        return hend;
    }
    auto end = std::make_unique<helics::EndpointObject>();
    end->endPtr = &endp;
    end->fedptr = getMessageFedSharedPtr(fed, nullptr);
    return addEndpoint(fed, std::move(end));
}

constexpr char nullcstr[] = "";

constexpr char invalidEndpoint[] = "The given endpoint does not point to a valid object";

helics::EndpointObject* verifyEndpoint(HelicsEndpoint ept, HelicsError* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    auto* endObj = reinterpret_cast<helics::EndpointObject*>(ept);
    if (endObj == nullptr || endObj->valid != EndpointValidationIdentifier) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidEndpoint);
        return nullptr;
    }
    return endObj;
}
}  // namespace

HelicsEndpoint helicsFederateRegisterEndpoint(HelicsFederate fed, const char* name, const char* type, HelicsError* err)
{
    // now generate a generic endpoint
    auto fedObj = getMessageFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto end = std::make_unique<helics::EndpointObject>();
        end->endPtr = &fedObj->registerEndpoint(AS_STRING_VIEW(name), AS_STRING_VIEW(type));
        end->fedptr = std::move(fedObj);
        end->fed = helics::getFedObject(fed, nullptr);
        return addEndpoint(fed, std::move(end));
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    return nullptr;
}

HelicsEndpoint helicsFederateRegisterTargetedEndpoint(HelicsFederate fed, const char* name, const char* type, HelicsError* err)
{
    // now generate a generic endpoint
    auto fedObj = getMessageFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto end = std::make_unique<helics::EndpointObject>();
        end->endPtr = &fedObj->registerTargetedEndpoint(AS_STRING_VIEW(name), AS_STRING_VIEW(type));
        end->fedptr = std::move(fedObj);
        end->fed = helics::getFedObject(fed, nullptr);
        return addEndpoint(fed, std::move(end));
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    return nullptr;
}

HelicsEndpoint helicsFederateRegisterGlobalEndpoint(HelicsFederate fed, const char* name, const char* type, HelicsError* err)
{
    // now generate a generic subscription
    auto fedObj = getMessageFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto end = std::make_unique<helics::EndpointObject>();
        end->endPtr = &fedObj->registerGlobalEndpoint(AS_STRING_VIEW(name), AS_STRING_VIEW(type));
        end->fedptr = std::move(fedObj);
        end->fed = helics::getFedObject(fed, nullptr);
        return addEndpoint(fed, std::move(end));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

HelicsEndpoint helicsFederateRegisterGlobalTargetedEndpoint(HelicsFederate fed, const char* name, const char* type, HelicsError* err)
{
    // now generate a generic subscription
    auto fedObj = getMessageFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto end = std::make_unique<helics::EndpointObject>();
        end->endPtr = &fedObj->registerGlobalTargetedEndpoint(AS_STRING_VIEW(name), AS_STRING_VIEW(type));
        end->fedptr = std::move(fedObj);
        end->fed = helics::getFedObject(fed, nullptr);
        return addEndpoint(fed, std::move(end));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

static constexpr char invalidEndName[] = "the specified Endpoint name is not recognized";
static constexpr char invalidEndIndex[] = "the specified Endpoint index is not valid";

HelicsEndpoint helicsFederateGetEndpoint(HelicsFederate fed, const char* name, HelicsError* err)
{
    auto fedObj = getMessageFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    CHECK_NULL_STRING(name, nullptr);
    try {
        auto& ept = fedObj->getEndpoint(name);
        if (!ept.isValid()) {
            assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidEndName);
            return nullptr;
        }
        return findOrCreateEndpoint(fed, ept);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    // LCOV_EXCL_STOP
}

HelicsEndpoint helicsFederateGetEndpointByIndex(HelicsFederate fed, int index, HelicsError* err)
{
    auto fedObj = getMessageFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto& ept = fedObj->getEndpoint(index);
        if (!ept.isValid()) {
            assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidEndIndex);
            return nullptr;
        }
        return findOrCreateEndpoint(fed, ept);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    // LCOV_EXCL_STOP
}

HelicsBool helicsEndpointIsValid(HelicsEndpoint endpoint)
{
    auto* endObj = verifyEndpoint(endpoint, nullptr);
    if (endObj == nullptr) {
        return HELICS_FALSE;
    }
    return (endObj->endPtr->isValid()) ? HELICS_TRUE : HELICS_FALSE;
}

void helicsEndpointSetDefaultDestination(HelicsEndpoint endpoint, const char* dest, HelicsError* err)
{
    auto* endObj = verifyEndpoint(endpoint, err);
    if (endObj == nullptr) {
        return;
    }
    CHECK_NULL_STRING(dest, void());
    try {
        endObj->endPtr->setDefaultDestination(dest);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

const char* helicsEndpointGetDefaultDestination(HelicsEndpoint endpoint)
{
    auto* endObj = verifyEndpoint(endpoint, nullptr);
    if (endObj == nullptr) {
        return nullcstr;
    }
    const auto& str = endObj->endPtr->getDefaultDestination();
    return str.c_str();
}

void helicsEndpointSendString(HelicsEndpoint endpoint, const char* message, HelicsError* err)
{
    auto* endObj = verifyEndpoint(endpoint, err);
    if (endObj == nullptr) {
        return;
    }
    try {
        if (message == nullptr) {
            endObj->endPtr->send(gHelicsEmptyStr);
        } else {
            endObj->endPtr->send(message);
        }
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsEndpointSendStringTo(HelicsEndpoint endpoint, const char* message, const char* dest, HelicsError* err)
{
    auto* endObj = verifyEndpoint(endpoint, err);
    if (endObj == nullptr) {
        return;
    }
    try {
        endObj->endPtr->sendTo(AS_STRING_VIEW(message), AS_STRING_VIEW(dest));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsEndpointSendStringAt(HelicsEndpoint endpoint, const char* message, HelicsTime time, HelicsError* err)
{
    auto* endObj = verifyEndpoint(endpoint, err);
    if (endObj == nullptr) {
        return;
    }
    try {
        endObj->endPtr->sendAt(AS_STRING_VIEW(message), time);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsEndpointSendStringToAt(HelicsEndpoint endpoint, const char* message, const char* dest, HelicsTime time, HelicsError* err)
{
    auto* endObj = verifyEndpoint(endpoint, err);
    if (endObj == nullptr) {
        return;
    }
    try {
        endObj->endPtr->sendToAt(AS_STRING_VIEW(message), AS_STRING_VIEW(dest), time);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsEndpointSendBytes(HelicsEndpoint endpoint, const void* data, int inputDataLength, HelicsError* err)
{
    auto* endObj = verifyEndpoint(endpoint, err);
    if (endObj == nullptr) {
        return;
    }
    try {
        if ((data == nullptr) || (inputDataLength <= 0)) {
            endObj->endPtr->send(gHelicsEmptyStr);
        } else {
            endObj->endPtr->send(reinterpret_cast<const char*>(data), inputDataLength);
        }
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsEndpointSendBytesTo(HelicsEndpoint endpoint, const void* data, int inputDataLength, const char* dest, HelicsError* err)
{
    auto* endObj = verifyEndpoint(endpoint, err);
    if (endObj == nullptr) {
        return;
    }
    try {
        if ((data == nullptr) || (inputDataLength <= 0)) {
            endObj->endPtr->sendTo(gHelicsEmptyStr, AS_STRING_VIEW(dest));
        } else {
            endObj->endPtr->sendTo(reinterpret_cast<const char*>(data), inputDataLength, AS_STRING_VIEW(dest));
        }
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsEndpointSendBytesAt(HelicsEndpoint endpoint, const void* data, int inputDataLength, HelicsTime time, HelicsError* err)
{
    auto* endObj = verifyEndpoint(endpoint, err);
    if (endObj == nullptr) {
        return;
    }
    try {
        if ((data == nullptr) || (inputDataLength <= 0)) {
            endObj->endPtr->sendAt(gHelicsEmptyStr, time);
        } else {
            endObj->endPtr->sendAt(reinterpret_cast<const char*>(data), inputDataLength, time);
        }
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsEndpointSendBytesToAt(HelicsEndpoint endpoint,
                                 const void* data,
                                 int inputDataLength,
                                 const char* dest,
                                 HelicsTime time,
                                 HelicsError* err)
{
    auto* endObj = verifyEndpoint(endpoint, err);
    if (endObj == nullptr) {
        return;
    }
    try {
        if ((data == nullptr) || (inputDataLength <= 0)) {
            endObj->endPtr->sendToAt(gHelicsEmptyStr, AS_STRING_VIEW(dest), time);
        } else {
            endObj->endPtr->sendToAt(reinterpret_cast<const char*>(data), inputDataLength, AS_STRING_VIEW(dest), time);
        }
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

static constexpr char emptyMessageErrorString[] = "the message is NULL";

void helicsEndpointSendMessage(HelicsEndpoint endpoint, HelicsMessage message, HelicsError* err)
{
    auto* endObj = verifyEndpoint(endpoint, err);
    if (endObj == nullptr) {
        return;
    }
    auto* mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    try {
        endObj->endPtr->send(*mess);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsEndpointSendMessageZeroCopy(HelicsEndpoint endpoint, HelicsMessage message, HelicsError* err)
{
    auto* endObj = verifyEndpoint(endpoint, err);
    if (endObj == nullptr) {
        return;
    }

    auto ptr = getMessageUniquePtr(message, err);
    if (!ptr) {
        return;
    }

    try {
        endObj->endPtr->send(std::move(ptr));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsEndpointSubscribe(HelicsEndpoint endpoint, const char* key, HelicsError* err)
{
    auto* endObj = verifyEndpoint(endpoint, err);
    if (endObj == nullptr) {
        return;
    }
    CHECK_NULL_STRING(key, void());
    try {
        endObj->endPtr->subscribe(key);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

HelicsBool helicsFederateHasMessage(HelicsFederate fed)
{
    auto* mFed = getMessageFed(fed, nullptr);
    if (mFed == nullptr) {
        return HELICS_FALSE;
    }
    return (mFed->hasMessage()) ? HELICS_TRUE : HELICS_FALSE;
}

HelicsBool helicsEndpointHasMessage(HelicsEndpoint endpoint)
{
    auto* endObj = verifyEndpoint(endpoint, nullptr);
    if (endObj == nullptr) {
        return HELICS_FALSE;
    }
    return (endObj->endPtr->hasMessage()) ? HELICS_TRUE : HELICS_FALSE;
}

int helicsFederatePendingMessageCount(HelicsFederate fed)
{
    auto* mFed = getMessageFed(fed, nullptr);
    if (mFed == nullptr) {
        return 0;
    }
    return static_cast<int>(mFed->pendingMessageCount());
}

int helicsEndpointPendingMessageCount(HelicsEndpoint endpoint)
{
    auto* endObj = verifyEndpoint(endpoint, nullptr);
    if (endObj == nullptr) {
        return 0;
    }
    return static_cast<int>(endObj->endPtr->pendingMessageCount());
}
static constexpr uint16_t messageKeyCode = 0xB3;

namespace helics {

Message* MessageHolder::addMessage(std::unique_ptr<helics::Message>& mess)
{
    if (!mess) {
        return nullptr;
    }
    Message* message = mess.get();
    mess->backReference = static_cast<void*>(this);
    if (!freeMessageSlots.empty()) {
        auto index = freeMessageSlots.back();
        freeMessageSlots.pop_back();
        mess->counter = index;
        messages[index] = std::move(mess);
    } else {
        mess->counter = static_cast<int32_t>(messages.size());
        messages.push_back(std::move(mess));
    }
    return message;
}
Message* MessageHolder::newMessage()
{
    Message* message{nullptr};
    if (!freeMessageSlots.empty()) {
        auto index = freeMessageSlots.back();
        freeMessageSlots.pop_back();
        messages[index] = std::make_unique<Message>();
        message = messages[index].get();
        message->counter = index;

    } else {
        messages.push_back(std::make_unique<Message>());
        message = messages.back().get();
        message->counter = static_cast<int32_t>(messages.size()) - 1;
    }

    message->messageValidation = messageKeyCode;
    message->backReference = static_cast<void*>(this);
    return message;
}

std::unique_ptr<Message> MessageHolder::extractMessage(int index)
{
    if (isValidIndex(index, messages)) {
        if (messages[index]) {
            freeMessageSlots.push_back(index);
            messages[index]->backReference = nullptr;
            messages[index]->messageValidation = 0;
            return std::move(messages[index]);
        }
    }
    return nullptr;
}

void MessageHolder::freeMessage(int index)
{
    if (isValidIndex(index, messages)) {
        if (messages[index]) {
            messages[index]->backReference = nullptr;
            messages[index]->messageValidation = 0;
            messages[index].reset();
            freeMessageSlots.push_back(index);
        }
    }
}

void MessageHolder::clear()
{
    freeMessageSlots.clear();
    for (auto& message : messages) {
        if (message) {
            message->backReference = nullptr;
            message->messageValidation = 0;
        }
    }
    messages.clear();
}

}  // namespace helics

HelicsMessage helicsEndpointGetMessage(HelicsEndpoint endpoint)
{
    auto* endObj = verifyEndpoint(endpoint, nullptr);
    if (endObj == nullptr) {
        return nullptr;
    }

    auto message = endObj->endPtr->getMessage();

    if (!message) {
        return nullptr;
    }
    message->messageValidation = messageKeyCode;
    return endObj->fed->messages.addMessage(message);
}

HelicsMessage helicsFederateGetMessage(HelicsFederate fed)
{
    auto* mFed = getMessageFed(fed, nullptr);
    if (mFed == nullptr) {
        return nullptr;
    }

    auto* fedObj = helics::getFedObject(fed, nullptr);

    auto message = mFed->getMessage();

    if (!message) {
        return nullptr;
    }
    message->messageValidation = messageKeyCode;
    return fedObj->messages.addMessage(message);
}

HelicsMessage helicsFederateCreateMessage(HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = helics::getFedObject(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    return fedObj->messages.newMessage();
}

HelicsMessage helicsEndpointCreateMessage(HelicsEndpoint endpoint, HelicsError* err)
{
    auto* endObj = verifyEndpoint(endpoint, err);
    if (endObj == nullptr) {
        return nullptr;
    }
    if (endObj->fed == nullptr) {
        return nullptr;
    }
    return endObj->fed->messages.newMessage();
}

void helicsFederateClearMessages(HelicsFederate fed)
{
    auto* fedObj = helics::getFedObject(fed, nullptr);
    if (fedObj == nullptr) {
        return;
    }
    fedObj->messages.clear();
}

void helicsEndpointClearMessages(HelicsEndpoint /*endpoint*/) {}

/* this function has been removed but may be added back in the future
HelicsMessage helicsFederateGetLastMessage (HelicsFederate fed)
{
    auto fedObj = helics::getFedObject (fed, nullptr);
    if (fedObj == nullptr)
    {
        return nullptr;
    }
    if (!fedObj->messages.empty ())
    {
        HelicsMessage mess = fedObj->messages.back ().get ();
        return mess;
    }
    return nullptr;
}

HelicsMessage helicsEndpointGetLastMessage (HelicsEndpoint endpoint)
{
    auto* endObj = verifyEndpoint (endpoint, nullptr);
    if (endObj == nullptr)
    {
        return nullptr;
    }
    if (!endObj->messages.empty ())
    {
        HelicsMessage mess = endObj->messages.back ().get ();
        return mess;
    }
    return nullptr;
}
*/

const char* helicsEndpointGetType(HelicsEndpoint endpoint)
{
    auto* endObj = verifyEndpoint(endpoint, nullptr);
    if (endObj == nullptr) {
        return nullcstr;
    }

    try {
        const auto& type = endObj->endPtr->getType();
        return type.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return nullcstr;
    }
    // LCOV_EXCL_STOP
}

const char* helicsEndpointGetName(HelicsEndpoint endpoint)
{
    auto* endObj = verifyEndpoint(endpoint, nullptr);
    if (endObj == nullptr) {
        return nullcstr;
    }
    const auto& type = endObj->endPtr->getName();
    return type.c_str();
}

int helicsFederateGetEndpointCount(HelicsFederate fed)
{
    // this call should be with a nullptr since it can fail and still be a successful call
    auto* mfedObj = getMessageFed(fed, nullptr);
    if (mfedObj == nullptr) {
        return 0;
    }
    return static_cast<int>(mfedObj->getEndpointCount());
}

const char* helicsEndpointGetInfo(HelicsEndpoint end)
{
    auto* endObj = verifyEndpoint(end, nullptr);
    if (endObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }
    try {
        const std::string& info = endObj->endPtr->getInfo();
        return info.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return gHelicsEmptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

void helicsEndpointSetInfo(HelicsEndpoint end, const char* info, HelicsError* err)
{
    auto* endObj = verifyEndpoint(end, err);
    if (endObj == nullptr) {
        return;
    }
    try {
        endObj->endPtr->setInfo(AS_STRING_VIEW(info));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

const char* helicsEndpointGetTag(HelicsEndpoint endpoint, const char* tagname)
{
    auto* endObj = verifyEndpoint(endpoint, nullptr);
    if (endObj == nullptr) {
        return gHelicsEmptyStr.c_str();
    }
    try {
        const std::string& info = endObj->endPtr->getTag(AS_STRING_VIEW(tagname));
        return info.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return gHelicsEmptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

void helicsEndpointSetTag(HelicsEndpoint end, const char* tagname, const char* tagvalue, HelicsError* err)
{
    auto* endObj = verifyEndpoint(end, err);
    if (endObj == nullptr) {
        return;
    }
    try {
        endObj->endPtr->setTag(AS_STRING_VIEW(tagname), AS_STRING_VIEW(tagvalue));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

int helicsEndpointGetOption(HelicsEndpoint end, int option)
{
    auto* endObj = verifyEndpoint(end, nullptr);
    if (endObj == nullptr) {
        return HELICS_FALSE;
    }
    try {
        return endObj->endPtr->getOption(option);
    }
    // LCOV_EXCL_START
    catch (...) {
        return HELICS_FALSE;
    }
    // LCOV_EXCL_STOP
}

void helicsEndpointSetOption(HelicsEndpoint end, int option, int value, HelicsError* err)
{
    auto* endObj = verifyEndpoint(end, err);
    if (endObj == nullptr) {
        return;
    }
    try {
        endObj->endPtr->setOption(option, value);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsEndpointAddSourceTarget(HelicsEndpoint end, const char* targetEndpoint, HelicsError* err)
{
    auto* endObj = verifyEndpoint(end, err);
    if (endObj == nullptr) {
        return;
    }
    try {
        endObj->endPtr->addSourceTarget(targetEndpoint);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsEndpointAddDestinationTarget(HelicsEndpoint end, const char* targetEndpoint, HelicsError* err)
{
    auto* endObj = verifyEndpoint(end, err);
    if (endObj == nullptr) {
        return;
    }
    try {
        endObj->endPtr->addDestinationTarget(targetEndpoint);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsEndpointRemoveTarget(HelicsEndpoint end, const char* targetEndpoint, HelicsError* err)
{
    auto* endObj = verifyEndpoint(end, err);
    if (endObj == nullptr) {
        return;
    }
    try {
        endObj->endPtr->removeTarget(targetEndpoint);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsEndpointAddSourceFilter(HelicsEndpoint end, const char* filterName, HelicsError* err)
{
    auto* endObj = verifyEndpoint(end, err);
    if (endObj == nullptr) {
        return;
    }
    try {
        endObj->endPtr->addSourceFilter(filterName);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsEndpointAddDestinationFilter(HelicsEndpoint end, const char* filterName, HelicsError* err)
{
    auto* endObj = verifyEndpoint(end, err);
    if (endObj == nullptr) {
        return;
    }
    try {
        endObj->endPtr->addDestinationFilter(filterName);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

HelicsMessage wrapMessage(std::unique_ptr<helics::Message>& mess)
{
    mess->messageValidation = messageKeyCode;
    return reinterpret_cast<HelicsMessage>(mess.get());
}

static constexpr char invalidMessageObject[] = "The message object was not valid";

helics::Message* getMessageObj(HelicsMessage message, HelicsError* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    auto* mess = reinterpret_cast<helics::Message*>(message);
    if (mess == nullptr || mess->messageValidation != messageKeyCode) {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidMessageObject);
        return nullptr;
    }
    return mess;
}

std::unique_ptr<helics::Message> getMessageUniquePtr(HelicsMessage message, HelicsError* err)
{
    static constexpr char invalidLocationString[] = "the message is NULL";
    auto* mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return nullptr;
    }
    auto* messages = reinterpret_cast<helics::MessageHolder*>(mess->backReference);
    if (messages != nullptr) {
        auto ptr = messages->extractMessage(mess->counter);
        if (!ptr) {
            assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidLocationString);
        }
        return ptr;
    }
    assignError(err, HELICS_ERROR_INVALID_ARGUMENT, emptyMessageErrorString);
    return nullptr;
}
HelicsMessage createAPIMessage(std::unique_ptr<helics::Message>& message)
{
    if (message) {
        message->messageValidation = messageKeyCode;
    }
    HelicsMessage mess = message.get();
    return mess;
}

const char* helicsMessageGetSource(HelicsMessage message)
{
    auto* mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return nullcstr;
    }
    return mess->source.c_str();
}

const char* helicsMessageGetDestination(HelicsMessage message)
{
    auto* mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return nullcstr;
    }
    return mess->dest.c_str();
}

const char* helicsMessageGetOriginalSource(HelicsMessage message)
{
    auto* mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return nullcstr;
    }
    return mess->original_source.c_str();
}

const char* helicsMessageGetOriginalDestination(HelicsMessage message)
{
    auto* mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return nullcstr;
    }
    return mess->original_dest.c_str();
}

HelicsTime helicsMessageGetTime(HelicsMessage message)
{
    auto* mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return HELICS_TIME_INVALID;
    }
    return static_cast<double>(mess->time);
}

int32_t helicsMessageGetMessageID(HelicsMessage message)
{
    auto* mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return 0;
    }
    return mess->messageID;
}

HelicsBool helicsMessageGetFlagOption(HelicsMessage message, int flag)
{
    auto* mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return HELICS_FALSE;
    }
    // bits in a uint16
    if (flag >= static_cast<int>(sizeof(uint16_t) * 8) || flag < 0) {
        return HELICS_FALSE;
    }
    return (checkActionFlag(*mess, flag) ? HELICS_TRUE : HELICS_FALSE);
}

const char* helicsMessageGetString(HelicsMessage message)
{
    auto* mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return nullcstr;
    }
    // enforce the null termination
    mess->data.null_terminate();
    return mess->data.char_data();
}

int helicsMessageGetByteCount(HelicsMessage message)
{
    auto* mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return 0;
    }
    return static_cast<int>(mess->data.size());
}

void helicsMessageGetBytes(HelicsMessage message, void* data, int maxMessagelen, int* actualSize, HelicsError* err)
{
    auto* mess = getMessageObj(message, err);
    if (mess == nullptr || mess->data.empty()) {
        if (actualSize != nullptr) {
            *actualSize = 0;
        }
        return;
    }
    if (data == nullptr || maxMessagelen <= 0 || static_cast<int>(mess->data.size()) > maxMessagelen) {
        static constexpr char invalidInsufficient[] = "the given storage was not sufficient to store the message";
        if (actualSize != nullptr) {
            *actualSize = 0;
        }
        assignError(err, HELICS_ERROR_INSUFFICIENT_SPACE, invalidInsufficient);
        return;
    }

    memcpy(data, mess->data.data(), mess->data.size());
    if (actualSize != nullptr) {
        *actualSize = static_cast<int>(mess->data.size());
    }
}

void* helicsMessageGetBytesPointer(HelicsMessage message)
{
    auto* mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return nullptr;
    }
    return mess->data.data();
}

HelicsDataBuffer helicsMessageDataBuffer(HelicsMessage message, HelicsError* err)
{
    auto* mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return nullptr;
    }
    return message;
}

HelicsBool helicsMessageIsValid(HelicsMessage message)
{
    auto* mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return HELICS_FALSE;
    }
    return (mess->isValid() ? HELICS_TRUE : HELICS_FALSE);
}

void helicsMessageSetSource(HelicsMessage message, const char* src, HelicsError* err)
{
    auto* mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    mess->source = AS_STRING(src);
}

void helicsMessageSetDestination(HelicsMessage message, const char* dest, HelicsError* err)
{
    auto* mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    mess->dest = AS_STRING(dest);
}
void helicsMessageSetOriginalSource(HelicsMessage message, const char* src, HelicsError* err)
{
    auto* mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    mess->original_source = AS_STRING(src);
}
void helicsMessageSetOriginalDestination(HelicsMessage message, const char* dest, HelicsError* err)
{
    auto* mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    mess->original_dest = AS_STRING(dest);
}
void helicsMessageSetTime(HelicsMessage message, HelicsTime time, HelicsError* err)
{
    auto* mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    mess->time = time;
}

void helicsMessageResize(HelicsMessage message, int newSize, HelicsError* err)
{
    auto* mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    try {
        mess->data.resize(newSize);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsMessageReserve(HelicsMessage message, int reservedSize, HelicsError* err)
{
    auto* mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    try {
        mess->data.reserve(reservedSize);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsMessageSetMessageID(HelicsMessage message, int32_t messageID, HelicsError* err)
{
    auto* mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    mess->messageID = messageID;
}

void helicsMessageClearFlags(HelicsMessage message)
{
    auto* mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return;
    }
    mess->flags = 0;
}

void helicsMessageSetFlagOption(HelicsMessage message, int flag, HelicsBool flagValue, HelicsError* err)
{
    auto* mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    if (flag > 15 || flag < 0) {
        static constexpr const char invalidFlagIndex[] = "flag variable is out of bounds must be in [0,15]";
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidFlagIndex);
        return;
    }
    if (flagValue == HELICS_TRUE) {
        setActionFlag(*mess, flag);
    } else {
        clearActionFlag(*mess, flag);
    }
}

void helicsMessageSetString(HelicsMessage message, const char* str, HelicsError* err)
{
    auto* mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    mess->data = AS_STRING_VIEW(str);
    mess->data.null_terminate();
}

void helicsMessageSetData(HelicsMessage message, const void* data, int inputDataLength, HelicsError* err)
{
    auto* mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    mess->data = std::string_view(static_cast<const char*>(data), inputDataLength);
}

void helicsMessageSetDataBuffer(HelicsMessage message, HelicsDataBuffer data, HelicsError* err)
{
    auto* mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    auto* ptr = getBuffer(data);
    if (ptr == nullptr) {
        mess->data.clear();
        return;
    }
    mess->data = *ptr;
}

void helicsMessageAppendData(HelicsMessage message, const void* data, int inputDataLength, HelicsError* err)
{
    auto* mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    mess->data.append(std::string_view{static_cast<const char*>(data), static_cast<std::size_t>(inputDataLength)});
}

void helicsMessageClear(HelicsMessage message, HelicsError* err)
{
    auto* mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    mess->clear();
}

void helicsMessageCopy(HelicsMessage source_message, HelicsMessage dest_message, HelicsError* err)
{
    auto* mess_src = getMessageObj(source_message, err);
    if (mess_src == nullptr) {
        return;
    }
    auto* mess_dest = getMessageObj(dest_message, err);
    if (mess_dest == nullptr) {
        return;
    }
    mess_dest->data = mess_src->data;
    mess_dest->dest = mess_src->dest;
    mess_dest->original_source = mess_src->original_source;
    mess_dest->source = mess_src->source;
    mess_dest->original_dest = mess_src->original_dest;
    mess_dest->time = mess_src->time;
    mess_dest->messageID = mess_src->messageID;
    mess_dest->flags = mess_src->flags;
}

HelicsMessage helicsMessageClone(HelicsMessage message, HelicsError* err)
{
    auto* mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return nullptr;
    }
    auto* messages = reinterpret_cast<helics::MessageHolder*>(mess->backReference);
    if (messages == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, emptyMessageErrorString);
        return nullptr;
    }
    auto* mess_clone = messages->newMessage();

    mess_clone->data = mess->data;
    mess_clone->dest = mess->dest;
    mess_clone->original_source = mess->original_source;
    mess_clone->source = mess->source;
    mess_clone->original_dest = mess->original_dest;
    mess_clone->time = mess->time;
    mess_clone->messageID = mess->messageID;
    mess_clone->flags = mess->flags;
    return mess_clone;
}

void helicsMessageFree(HelicsMessage message)
{
    auto* mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return;
    }
    auto* messages = reinterpret_cast<helics::MessageHolder*>(mess->backReference);
    if (messages == nullptr) {
        return;
    }
    messages->freeMessage(mess->counter);
}
