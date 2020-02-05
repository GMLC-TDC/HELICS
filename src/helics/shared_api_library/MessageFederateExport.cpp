/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../core/core-exceptions.hpp"
#include "../core/flagOperations.hpp"
#include "../helics.hpp"
#include "MessageFederate.h"
#include "helics.h"
#include "internal/api_objects.h"

#include <memory>
#include <mutex>
#include <vector>

// random integer for validation purposes of endpoints
static constexpr int EndpointValidationIdentifier = 0xB453'94C2;

static inline void addEndpoint(helics_federate fed, std::unique_ptr<helics::EndpointObject> ept)
{
    auto fedObj = reinterpret_cast<helics::FedObject*>(fed);
    ept->valid = EndpointValidationIdentifier;
    fedObj->epts.push_back(std::move(ept));
}

static constexpr char nullcstr[] = "";
const std::string nullStringArgument("the supplied string argument is null and therefor invalid");

static constexpr char invalidEndpoint[] = "The given endpoint does not point to a valid object";

static helics::EndpointObject* verifyEndpoint(helics_endpoint ept, helics_error* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    auto endObj = reinterpret_cast<helics::EndpointObject*>(ept);
    if (endObj == nullptr || endObj->valid != EndpointValidationIdentifier) {
        if (err != nullptr) {
            err->error_code = helics_error_invalid_object;
            err->message = invalidEndpoint;
        }
        return nullptr;
    }
    return endObj;
}

helics_endpoint helicsFederateRegisterEndpoint(helics_federate fed, const char* name, const char* type, helics_error* err)
{
    // now generate a generic endpoint
    auto fedObj = getMessageFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto end = std::make_unique<helics::EndpointObject>();
        end->endPtr = &fedObj->registerEndpoint(AS_STRING(name), AS_STRING(type));
        end->fedptr = std::move(fedObj);
        auto ret = reinterpret_cast<helics_endpoint>(end.get());
        addEndpoint(fed, std::move(end));
        return ret;
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    return nullptr;
}

helics_endpoint helicsFederateRegisterGlobalEndpoint(helics_federate fed, const char* name, const char* type, helics_error* err)
{
    // now generate a generic subscription
    auto fedObj = getMessageFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto end = std::make_unique<helics::EndpointObject>();
        end->endPtr = &fedObj->registerGlobalEndpoint(AS_STRING(name), AS_STRING(type));
        end->fedptr = std::move(fedObj);
        auto ret = reinterpret_cast<helics_endpoint>(end.get());
        addEndpoint(fed, std::move(end));
        return ret;
    }
    catch (...) {
        helicsErrorHandler(err);
    }
    return nullptr;
}

static constexpr char invalidEndName[] = "the specified Endpoint name is not recognized";
static constexpr char invalidEndIndex[] = "the specified Endpoint index is not valid";

helics_endpoint helicsFederateGetEndpoint(helics_federate fed, const char* name, helics_error* err)
{
    auto fedObj = getMessageFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    CHECK_NULL_STRING(name, nullptr);
    try {
        auto& id = fedObj->getEndpoint(name);
        if (!id.isValid()) {
            if (err != nullptr) {
                err->error_code = helics_error_invalid_argument;
                err->message = invalidEndName;
            }
            return nullptr;
        }
        auto end = std::make_unique<helics::EndpointObject>();
        end->endPtr = &id;
        end->fedptr = std::move(fedObj);
        auto ret = reinterpret_cast<helics_endpoint>(end.get());
        addEndpoint(fed, std::move(end));
        return ret;
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    // LCOV_EXCL_STOP
}

helics_endpoint helicsFederateGetEndpointByIndex(helics_federate fed, int index, helics_error* err)
{
    auto fedObj = getMessageFedSharedPtr(fed, err);
    if (!fedObj) {
        return nullptr;
    }
    try {
        auto& id = fedObj->getEndpoint(index);
        if (!id.isValid()) {
            if (err != nullptr) {
                err->error_code = helics_error_invalid_argument;
                err->message = invalidEndIndex;
            }
            return nullptr;
        }
        auto end = std::make_unique<helics::EndpointObject>();
        end->endPtr = &id;
        end->fedptr = std::move(fedObj);
        auto ret = reinterpret_cast<helics_endpoint>(end.get());
        addEndpoint(fed, std::move(end));
        return ret;
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
    // LCOV_EXCL_STOP
}

void helicsEndpointSetDefaultDestination(helics_endpoint endpoint, const char* dest, helics_error* err)
{
    auto endObj = verifyEndpoint(endpoint, err);
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

const char* helicsEndpointGetDefaultDestination(helics_endpoint endpoint)
{
    auto endObj = verifyEndpoint(endpoint, nullptr);
    if (endObj == nullptr) {
        return nullcstr;
    }
    auto& str = endObj->endPtr->getDefaultDestination();
    return str.c_str();
}

void helicsEndpointSendMessageRaw(helics_endpoint endpoint, const char* dest, const void* data, int inputDataLength, helics_error* err)
{
    auto endObj = verifyEndpoint(endpoint, err);
    if (endObj == nullptr) {
        return;
    }
    try {
        if ((data == nullptr) || (inputDataLength <= 0)) {
            if ((dest == nullptr) || (std::string(dest).empty())) {
                endObj->endPtr->send(emptyStr);
            } else {
                endObj->endPtr->send(dest, emptyStr);
            }
        } else {
            if ((dest == nullptr) || (std::string(dest).empty())) {
                endObj->endPtr->send(reinterpret_cast<const char*>(data), inputDataLength);
            } else {
                endObj->endPtr->send(dest, reinterpret_cast<const char*>(data), inputDataLength);
            }
        }
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsEndpointSendEventRaw(
    helics_endpoint endpoint,
    const char* dest,
    const void* data,
    int inputDataLength,
    helics_time time,
    helics_error* err)
{
    auto endObj = verifyEndpoint(endpoint, err);
    if (endObj == nullptr) {
        return;
    }
    try {
        if ((data == nullptr) || (inputDataLength <= 0)) {
            if ((dest == nullptr) || (std::string(dest).empty())) {
                endObj->endPtr->send(emptyStr, time);
            } else {
                endObj->endPtr->send(dest, emptyStr, time);
            }
        } else {
            if ((dest == nullptr) || (std::string(dest).empty())) {
                endObj->endPtr->send(reinterpret_cast<const char*>(data), inputDataLength, time);
            } else {
                endObj->endPtr->send(dest, reinterpret_cast<const char*>(data), inputDataLength, time);
            }
        }
    }
    catch (...) {
        return helicsErrorHandler(err);
    }
}

static constexpr char emptyMessageErrorString[] = "the message is NULL";

void helicsEndpointSendMessage(helics_endpoint endpoint, helics_message* message, helics_error* err)
{
    auto endObj = verifyEndpoint(endpoint, err);
    if (endObj == nullptr) {
        return;
    }
    if (message == nullptr) {
        if (err != nullptr) {
            err->error_code = helics_error_invalid_argument;
            err->message = emptyMessageErrorString;
        }
        return;
    }

    try {
        if ((message->original_source == nullptr) || (endObj->endPtr->getName() == message->original_source)) {
            if (message->dest == nullptr) {
                endObj->endPtr->send(message->data, message->length, message->time);
            } else {
                endObj->endPtr->send(message->dest, message->data, message->length, message->time);
            }
        } else {
            helics::Message nmessage;
            nmessage.time = message->time;
            nmessage.source = AS_STRING(message->source);
            nmessage.dest = AS_STRING(message->dest);
            nmessage.original_dest = AS_STRING(message->original_dest);
            nmessage.original_source = message->original_source;
            if (message->data != nullptr && message->length > 0) {
                nmessage.data.assign(message->data, message->length);
            }
            endObj->endPtr->send(nmessage);
        }
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsEndpointSendMessageObject(helics_endpoint endpoint, helics_message_object message, helics_error* err)
{
    auto endObj = verifyEndpoint(endpoint, err);
    if (endObj == nullptr) {
        return;
    }
    if (message == nullptr) {
        if (err != nullptr) {
            err->error_code = helics_error_invalid_argument;
            err->message = emptyMessageErrorString;
        }
        return;
    }

    helics::Message* mess = reinterpret_cast<helics::Message*>(message);
    try {
        endObj->endPtr->send(*mess);
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsEndpointSubscribe(helics_endpoint endpoint, const char* key, helics_error* err)
{
    auto endObj = verifyEndpoint(endpoint, err);
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

helics_bool helicsFederateHasMessage(helics_federate fed)
{
    auto mFed = getMessageFed(fed, nullptr);
    if (mFed == nullptr) {
        return helics_false;
    }
    return (mFed->hasMessage()) ? helics_true : helics_false;
}

helics_bool helicsEndpointHasMessage(helics_endpoint endpoint)
{
    auto endObj = verifyEndpoint(endpoint, nullptr);
    if (endObj == nullptr) {
        return helics_false;
    }
    return (endObj->endPtr->hasMessage()) ? helics_true : helics_false;
}

int helicsFederatePendingMessages(helics_federate fed)
{
    auto mFed = getMessageFed(fed, nullptr);
    if (mFed == nullptr) {
        return 0;
    }
    return static_cast<int>(mFed->pendingMessages());
}

int helicsEndpointPendingMessages(helics_endpoint endpoint)
{
    auto endObj = verifyEndpoint(endpoint, nullptr);
    if (endObj == nullptr) {
        return 0;
    }
    return static_cast<int>(endObj->endPtr->pendingMessages());
}

static helics_message emptyMessage()
{
    helics_message empty;
    empty.time = 0;
    empty.data = nullptr;
    empty.length = 0;
    empty.dest = nullptr;
    empty.original_source = nullptr;
    empty.original_dest = nullptr;
    empty.source = nullptr;
    empty.messageID = 0;
    empty.flags = 0;
    return empty;
}

static constexpr uint16_t messageKeyCode = 0xB3;

helics_message helicsEndpointGetMessage(helics_endpoint endpoint)
{
    auto endObj = verifyEndpoint(endpoint, nullptr);
    if (endObj == nullptr) {
        return emptyMessage();
    }

    auto message = endObj->endPtr->getMessage();
    if (message) {
        helics_message mess;
        mess.data = message->data.data();
        mess.dest = message->dest.c_str();
        mess.length = message->data.size();
        mess.original_source = message->original_source.c_str();
        mess.source = message->source.c_str();
        mess.original_dest = message->original_dest.c_str();
        mess.time = static_cast<helics_time>(message->time);
        mess.flags = message->flags;
        mess.messageID = message->messageID;
        endObj->messages.push_back(std::move(message));
        return mess;
    }
    return emptyMessage();
}

helics_message_object helicsEndpointGetMessageObject(helics_endpoint endpoint)
{
    auto endObj = verifyEndpoint(endpoint, nullptr);
    if (endObj == nullptr) {
        return nullptr;
    }

    auto message = endObj->endPtr->getMessage();

    if (message) {
        message->messageValidation = messageKeyCode;
        helics_message_object mess = message.get();
        endObj->messages.push_back(std::move(message));
        return mess;
    }
    return nullptr;
}

helics_message helicsFederateGetMessage(helics_federate fed)
{
    auto mFed = getMessageFed(fed, nullptr);
    if (mFed == nullptr) {
        return emptyMessage();
    }

    auto fedObj = helics::getFedObject(fed, nullptr);

    auto message = mFed->getMessage();

    if (message) {
        helics_message mess;
        mess.data = message->data.data();
        mess.dest = message->dest.c_str();
        mess.length = message->data.size();
        mess.original_source = message->original_source.c_str();
        mess.source = message->source.c_str();
        mess.original_dest = message->original_dest.c_str();
        mess.time = static_cast<helics_time>(message->time);
        mess.messageID = message->messageID;
        mess.flags = message->flags;
        fedObj->messages.push_back(std::move(message));
        return mess;
    }
    return emptyMessage();
}

helics_message_object helicsFederateGetMessageObject(helics_federate fed)
{
    auto mFed = getMessageFed(fed, nullptr);
    if (mFed == nullptr) {
        return nullptr;
    }

    auto fedObj = helics::getFedObject(fed, nullptr);

    auto message = mFed->getMessage();

    if (message) {
        message->messageValidation = messageKeyCode;
        helics_message_object mess = message.get();
        fedObj->messages.push_back(std::move(message));
        return mess;
    }
    return nullptr;
}

helics_message_object helicsFederateCreateMessageObject(helics_federate fed, helics_error* err)
{
    auto fedObj = helics::getFedObject(fed, err);
    if (fedObj == nullptr) {
        return nullptr;
    }
    auto messptr = std::make_unique<helics::Message>();
    messptr->messageValidation = messageKeyCode;
    helics_message_object mess = messptr.get();
    fedObj->messages.push_back(std::move(messptr));
    return mess;
}

void helicsFederateClearMessages(helics_federate fed)
{
    auto fedObj = helics::getFedObject(fed, nullptr);
    if (fedObj == nullptr) {
        return;
    }
    fedObj->messages.clear();
}

/** clear all message from an endpoint
@param endpoint  the endpoint object to operate on
*/
void helicsEndpointClearMessages(helics_endpoint endpoint)
{
    auto endObj = verifyEndpoint(endpoint, nullptr);
    if (endObj == nullptr) {
        return;
    }
    endObj->messages.clear();
}

/* this function has been removed but may be added back in the future
helics_message_object helicsFederateGetLastMessage (helics_federate fed)
{
    auto fedObj = helics::getFedObject (fed, nullptr);
    if (fedObj == nullptr)
    {
        return nullptr;
    }
    if (!fedObj->messages.empty ())
    {
        helics_message_object mess = fedObj->messages.back ().get ();
        return mess;
    }
    return nullptr;
}

helics_message_object helicsEndpointGetLastMessage (helics_endpoint endpoint)
{
    auto endObj = verifyEndpoint (endpoint, nullptr);
    if (endObj == nullptr)
    {
        return nullptr;
    }
    if (!endObj->messages.empty ())
    {
        helics_message_object mess = endObj->messages.back ().get ();
        return mess;
    }
    return nullptr;
}
*/

const char* helicsEndpointGetType(helics_endpoint endpoint)
{
    auto endObj = verifyEndpoint(endpoint, nullptr);
    if (endObj == nullptr) {
        return nullcstr;
    }

    try {
        auto& type = endObj->endPtr->getType();
        return type.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return nullcstr;
    }
    // LCOV_EXCL_STOP
}

const char* helicsEndpointGetName(helics_endpoint endpoint)
{
    auto endObj = verifyEndpoint(endpoint, nullptr);
    if (endObj == nullptr) {
        return nullcstr;
    }
    auto& type = endObj->endPtr->getName();
    return type.c_str();
}

int helicsFederateGetEndpointCount(helics_federate fed)
{
    // this call should be with a nullptr since it can fail and still be a successful call
    auto mfedObj = getMessageFed(fed, nullptr);
    if (mfedObj == nullptr) {
        return 0;
    }
    return static_cast<int>(mfedObj->getEndpointCount());
}

const char* helicsEndpointGetInfo(helics_endpoint end)
{
    auto endObj = verifyEndpoint(end, nullptr);
    if (endObj == nullptr) {
        return emptyStr.c_str();
    }
    try {
        const std::string& info = endObj->endPtr->getInfo();
        return info.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        return emptyStr.c_str();
    }
    // LCOV_EXCL_STOP
}

void helicsEndpointSetInfo(helics_endpoint end, const char* info, helics_error* err)
{
    auto endObj = verifyEndpoint(end, err);
    if (endObj == nullptr) {
        return;
    }
    try {
        endObj->endPtr->setInfo(AS_STRING(info));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

helics_bool helicsEndpointGetOption(helics_endpoint end, int option)
{
    auto endObj = verifyEndpoint(end, nullptr);
    if (endObj == nullptr) {
        return helics_false;
    }
    try {
        return (endObj->endPtr->getOption(option)) ? helics_true : helics_false;
    }
    // LCOV_EXCL_START
    catch (...) {
        return helics_false;
    }
    // LCOV_EXCL_STOP
}

void helicsEndpointSetOption(helics_endpoint end, int option, helics_bool value, helics_error* err)
{
    auto endObj = verifyEndpoint(end, err);
    if (endObj == nullptr) {
        return;
    }
    try {
        endObj->endPtr->setOption(option, (value == helics_true));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

static constexpr char invalidMessageObject[] = "The message object was not valid";

helics::Message* getMessageObj(helics_message_object message, helics_error* err)
{
    HELICS_ERROR_CHECK(err, nullptr);
    helics::Message* mess = reinterpret_cast<helics::Message*>(message);
    if (mess == nullptr || mess->messageValidation != messageKeyCode) {
        if (err != nullptr) {
            err->error_code = helics_error_invalid_argument;
            err->message = invalidMessageObject;
        }
        return nullptr;
    }
    return mess;
}

const char* helicsMessageGetSource(helics_message_object message)
{
    auto mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return nullcstr;
    }
    return mess->source.c_str();
}

const char* helicsMessageGetDestination(helics_message_object message)
{
    auto mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return nullcstr;
    }
    return mess->dest.c_str();
}

const char* helicsMessageGetOriginalSource(helics_message_object message)
{
    auto mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return nullcstr;
    }
    return mess->original_source.c_str();
}

const char* helicsMessageGetOriginalDestination(helics_message_object message)
{
    auto mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return nullcstr;
    }
    return mess->original_dest.c_str();
}

helics_time helicsMessageGetTime(helics_message_object message)
{
    auto mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return helics_time_invalid;
    }
    return static_cast<double>(mess->time);
}

int32_t helicsMessageGetMessageID(helics_message_object message)
{
    auto mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return 0;
    }
    return mess->messageID;
}

helics_bool helicsMessageCheckFlag(helics_message_object message, int flag)
{
    auto mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return helics_false;
    }
    // bits in a uint16
    if (flag >= static_cast<int>(sizeof(uint16_t) * 8) || flag < 0) {
        return helics_false;
    }
    return (checkActionFlag(*mess, flag) ? helics_true : helics_false);
}

const char* helicsMessageGetString(helics_message_object message)
{
    auto mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return nullcstr;
    }
    return mess->data.data();
}

int helicsMessageGetRawDataSize(helics_message_object message)
{
    auto mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return 0;
    }
    return static_cast<int>(mess->data.size());
}

void helicsMessageGetRawData(helics_message_object message, void* data, int maxMessagelen, int* actualSize, helics_error* err)
{
    static constexpr char invalidInsufficient[] = "the given storage was not sufficient to store the message";
    auto mess = getMessageObj(message, err);
    if (mess == nullptr || mess->data.empty()) {
        if (actualSize != nullptr) {
            *actualSize = 0;
        }
        return;
    }
    if (data == nullptr || maxMessagelen <= 0 || static_cast<int>(mess->data.size()) > maxMessagelen) {
        if (actualSize != nullptr) {
            *actualSize = 0;
        }
        if (err != nullptr) {
            err->error_code = helics_error_insufficient_space;
            err->message = invalidInsufficient;
        }
        return;
    }

    memcpy(data, mess->data.data(), mess->data.size());
    if (actualSize != nullptr) {
        *actualSize = static_cast<int>(mess->data.size());
    }
    return;
}

void* helicsMessageGetRawDataPointer(helics_message_object message)
{
    auto mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return nullptr;
    }
    return mess->data.data();
}

helics_bool helicsMessageIsValid(helics_message_object message)
{
    auto mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return helics_false;
    }
    return (mess->isValid() ? helics_true : helics_false);
}

void helicsMessageSetSource(helics_message_object message, const char* src, helics_error* err)
{
    auto mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    mess->source = AS_STRING(src);
}

void helicsMessageSetDestination(helics_message_object message, const char* dest, helics_error* err)
{
    auto mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    mess->dest = AS_STRING(dest);
}
void helicsMessageSetOriginalSource(helics_message_object message, const char* src, helics_error* err)
{
    auto mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    mess->original_source = AS_STRING(src);
}
void helicsMessageSetOriginalDestination(helics_message_object message, const char* dest, helics_error* err)
{
    auto mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    mess->original_dest = AS_STRING(dest);
}
void helicsMessageSetTime(helics_message_object message, helics_time time, helics_error* err)
{
    auto mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    mess->time = time;
}

void helicsMessageResize(helics_message_object message, int newSize, helics_error* err)
{
    auto mess = getMessageObj(message, err);
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

void helicsMessageReserve(helics_message_object message, int reservedSize, helics_error* err)
{
    auto mess = getMessageObj(message, err);
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

void helicsMessageSetMessageID(helics_message_object message, int32_t messageID, helics_error* err)
{
    auto mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    mess->messageID = messageID;
}

void helicsMessageClearFlags(helics_message_object message)
{
    auto mess = getMessageObj(message, nullptr);
    if (mess == nullptr) {
        return;
    }
    mess->flags = 0;
}

void helicsMessageSetFlagOption(helics_message_object message, int flag, helics_bool flagValue, helics_error* err)
{
    auto mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    if (flag > 15 || flag < 0) {
        if (err != nullptr) {
            err->error_code = helics_error_invalid_argument;
            err->message = getMasterHolder()->addErrorString("flag variable is out of bounds must be in [0,15]");
        }
        return;
    }
    if (flagValue == helics_true) {
        setActionFlag(*mess, flag);
    } else {
        clearActionFlag(*mess, flag);
    }
}

void helicsMessageSetString(helics_message_object message, const char* str, helics_error* err)
{
    auto mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    mess->data = AS_STRING(str);
}

void helicsMessageSetData(helics_message_object message, const void* data, int inputDataLength, helics_error* err)
{
    auto mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    mess->data.assign(static_cast<const char*>(data), inputDataLength);
}

void helicsMessageAppendData(helics_message_object message, const void* data, int inputDataLength, helics_error* err)
{
    auto mess = getMessageObj(message, err);
    if (mess == nullptr) {
        return;
    }
    mess->data.append(static_cast<const char*>(data), inputDataLength);
}
