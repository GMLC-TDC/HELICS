/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "../core/core-exceptions.hpp"
#include "../helics.hpp"
#include "MessageFederate.h"
#include "helics.h"
#include "internal/api_objects.h"
#include <memory>
#include <mutex>
#include <vector>

// random integer for validation purposes of endpoints
static constexpr int EndpointValidationIdentifier = 0xB453'94C2;

static inline void addEndpoint (helics_federate fed, std::unique_ptr<helics::EndpointObject> ept)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    ept->valid = EndpointValidationIdentifier;
    fedObj->epts.push_back (std::move (ept));
}

static constexpr char nullcstr[] = "";
const std::string nullStringArgument ("the supplied string argument is null and therefor invalid");

static constexpr char invalidEndpoint[] = "The given endpoint does not point to a valid object";

static helics::EndpointObject *verifyEndpoint (helics_endpoint ept, helics_error *err)
{
    HELICS_ERROR_CHECK (err, nullptr);
    if (ept == nullptr)
    {
        if (err != nullptr)
        {
            err->error_code = helics_error_invalid_object;
            err->message = invalidEndpoint;
        }
        return nullptr;
    }
    auto endObj = reinterpret_cast<helics::EndpointObject *> (ept);
    if (endObj->valid != EndpointValidationIdentifier)
    {
        if (err != nullptr)
        {
            err->error_code = helics_error_invalid_object;
            err->message = invalidEndpoint;
        }
        return nullptr;
    }
    return endObj;
}

helics_endpoint helicsFederateRegisterEndpoint (helics_federate fed, const char *name, const char *type, helics_error *err)
{
    // now generate a generic endpoint
    auto fedObj = getMessageFedSharedPtr (fed, err);
    if (!fedObj)
    {
        return nullptr;
    }
    try
    {
        auto end = std::make_unique<helics::EndpointObject> ();
        end->endPtr = &fedObj->registerEndpoint (AS_STRING (name), AS_STRING (type));
        end->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_endpoint> (end.get ());
        addEndpoint (fed, std::move (end));
        return ret;
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return nullptr;
    }
    return nullptr;
}

helics_endpoint helicsFederateRegisterGlobalEndpoint (helics_federate fed, const char *name, const char *type, helics_error *err)
{
    // now generate a generic subscription
    auto fedObj = getMessageFedSharedPtr (fed, err);
    if (!fedObj)
    {
        return nullptr;
    }
    try
    {
        auto end = std::make_unique<helics::EndpointObject> ();
        end->endPtr = &fedObj->registerGlobalEndpoint (AS_STRING (name), AS_STRING (type));
        end->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_endpoint> (end.get ());
        addEndpoint (fed, std::move (end));
        return ret;
    }
    catch (...)
    {
        return nullptr;
    }
    return nullptr;
}

static constexpr char invalidEndName[] = "the specified Endpoint name is not recognized";
static constexpr char invalidEndIndex[] = "the specified Endpoint index is not valid";

helics_endpoint helicsFederateGetEndpoint (helics_federate fed, const char *name, helics_error *err)
{
    auto fedObj = getMessageFedSharedPtr (fed, err);
    if (!fedObj)
    {
        return nullptr;
    }
    CHECK_NULL_STRING (name, nullptr);
    try
    {
        auto &id = fedObj->getEndpoint (name);
        if (!id.isValid ())
        {
            if (err != nullptr)
            {
                err->error_code = helics_error_invalid_argument;
                err->message = invalidEndName;
            }
            return nullptr;
        }
        auto end = std::make_unique<helics::EndpointObject> ();
        end->endPtr = &id;
        end->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_endpoint> (end.get ());
        addEndpoint (fed, std::move (end));
        return ret;
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return nullptr;
    }
}

helics_endpoint helicsFederateGetEndpointByIndex (helics_federate fed, int index, helics_error *err)
{
    auto fedObj = getMessageFedSharedPtr (fed, err);
    if (!fedObj)
    {
        return nullptr;
    }
    try
    {
        auto &id = fedObj->getEndpoint (index);
        if (!id.isValid ())
        {
            if (err != nullptr)
            {
                err->error_code = helics_error_invalid_argument;
                err->message = invalidEndIndex;
            }
            return nullptr;
        }
        auto end = std::make_unique<helics::EndpointObject> ();
        end->endPtr = &id;
        end->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_endpoint> (end.get ());
        addEndpoint (fed, std::move (end));
        return ret;
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return nullptr;
    }
}

void helicsEndpointSetDefaultDestination (helics_endpoint endpoint, const char *dest, helics_error *err)
{
    auto endObj = verifyEndpoint (endpoint, err);
    if (endObj == nullptr)
    {
        return;
    }
    CHECK_NULL_STRING (dest, void());
    try
    {
        endObj->endPtr->setDefaultDestination (dest);
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

const char *helicsEndpointGetDefaultDestination (helics_endpoint endpoint)
{
    auto endObj = verifyEndpoint (endpoint, nullptr);
    if (endObj == nullptr)
    {
        return nullcstr;
    }
    try
    {
        auto &str = endObj->endPtr->getDefaultDestination ();
        return str.c_str ();
    }
    catch (...)
    {
        return nullcstr;
    }
}

void helicsEndpointSendMessageRaw (helics_endpoint endpoint, const char *dest, const void *data, int inputDataLength, helics_error *err)
{
    auto endObj = verifyEndpoint (endpoint, err);
    if (endObj == nullptr)
    {
        return;
    }
    try
    {
        if ((data == nullptr) || (inputDataLength <= 0))
        {
            if ((dest == nullptr) || (std::string (dest).empty ()))
            {
                endObj->endPtr->send (emptyStr);
            }
            else
            {
                endObj->endPtr->send (dest, emptyStr);
            }
        }
        else
        {
            if ((dest == nullptr) || (std::string (dest).empty ()))
            {
                endObj->endPtr->send (reinterpret_cast<const char *> (data), inputDataLength);
            }
            else
            {
                endObj->endPtr->send (dest, reinterpret_cast<const char *> (data), inputDataLength);
            }
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsEndpointSendEventRaw (helics_endpoint endpoint,
                                 const char *dest,
                                 const void *data,
                                 int inputDataLength,
                                 helics_time time,
                                 helics_error *err)
{
    auto endObj = verifyEndpoint (endpoint, err);
    if (endObj == nullptr)
    {
        return;
    }
    try
    {
        if ((data == nullptr) || (inputDataLength <= 0))
        {
            if ((dest == nullptr) || (std::string (dest).empty ()))
            {
                endObj->endPtr->send (emptyStr, time);
            }
            else
            {
                endObj->endPtr->send (dest, emptyStr, time);
            }
        }
        else
        {
            if ((dest == nullptr) || (std::string (dest).empty ()))
            {
                endObj->endPtr->send (reinterpret_cast<const char *> (data), inputDataLength, time);
            }
            else
            {
                endObj->endPtr->send (dest, reinterpret_cast<const char *> (data), inputDataLength, time);
            }
        }
    }
    catch (...)
    {
        return helicsErrorHandler (err);
    }
}

static constexpr char emptyMessageErrorString[] = "the message is NULL";

void helicsEndpointSendMessage (helics_endpoint endpoint, helics_message *message, helics_error *err)
{
    auto endObj = verifyEndpoint (endpoint, err);
    if (endObj == nullptr)
    {
        return;
    }
    if (message == nullptr)
    {
        if (err != nullptr)
        {
            err->error_code = helics_error_invalid_argument;
            err->message = emptyMessageErrorString;
        }
        return;
    }

    try
    {
        if ((message->original_source == nullptr) || (endObj->endPtr->getName () == message->original_source))
        {
            if (message->dest == nullptr)
            {
                endObj->endPtr->send (message->data, message->length, message->time);
            }
            else
            {
                endObj->endPtr->send (message->dest, message->data, message->length, message->time);
            }
        }
        else
        {
            helics::Message nmessage;
            nmessage.time = message->time;
            nmessage.source = message->source;
            nmessage.dest = message->dest;
            nmessage.original_dest = message->original_dest;
            nmessage.original_source = message->original_source;
            nmessage.data.assign (message->data, message->length);
            endObj->endPtr->send (nmessage);
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsEndpointSubscribe (helics_endpoint endpoint, const char *key, helics_error *err)
{
    auto endObj = verifyEndpoint (endpoint, err);
    if (endObj == nullptr)
    {
        return;
    }
    CHECK_NULL_STRING (key, void());
    try
    {
        endObj->endPtr->subscribe (key);
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

helics_bool helicsFederateHasMessage (helics_federate fed)
{
    auto mFed = getMessageFed (fed, nullptr);
    if (mFed == nullptr)
    {
        return helics_false;
    }
    return (mFed->hasMessage ()) ? helics_true : helics_false;
}

helics_bool helicsEndpointHasMessage (helics_endpoint endpoint)
{
    auto endObj = verifyEndpoint (endpoint, nullptr);
    if (endObj == nullptr)
    {
        return helics_false;
    }
    return (endObj->endPtr->hasMessage ()) ? helics_true : helics_false;
}

int helicsFederatePendingMessages (helics_federate fed)
{
    auto mFed = getMessageFed (fed, nullptr);
    if (mFed == nullptr)
    {
        return 0;
    }
    return mFed->pendingMessages ();
}

int helicsEndpointPendingMessages (helics_endpoint endpoint)
{
    auto endObj = verifyEndpoint (endpoint, nullptr);
    if (endObj == nullptr)
    {
        return 0;
    }
    return endObj->endPtr->pendingMessages ();
}

static helics_message emptyMessage ()
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

helics_message helicsEndpointGetMessage (helics_endpoint endpoint)
{
    auto endObj = verifyEndpoint (endpoint, nullptr);
    if (endObj == nullptr)
    {
        return emptyMessage ();
    }

    endObj->lastMessage = endObj->endPtr->getMessage ();

    if (endObj->lastMessage)
    {
        helics_message mess{};
        mess.data = endObj->lastMessage->data.data ();
        mess.dest = endObj->lastMessage->dest.c_str ();
        mess.length = endObj->lastMessage->data.size ();
        mess.original_source = endObj->lastMessage->original_source.c_str ();
        mess.source = endObj->lastMessage->source.c_str ();
        mess.original_dest = endObj->lastMessage->original_dest.c_str ();
        mess.time = static_cast<helics_time> (endObj->lastMessage->time);
        return mess;
    }
    return emptyMessage ();
}

helics_message helicsFederateGetMessage (helics_federate fed)
{
    auto mFed = getMessageFed (fed, nullptr);
    if (mFed == nullptr)
    {
        return emptyMessage ();
    }

    auto fedObj = helics::getFedObject (fed, nullptr);

    fedObj->lastMessage = mFed->getMessage ();

    if (fedObj->lastMessage)
    {
        helics_message mess{};
        mess.data = fedObj->lastMessage->data.data ();
        mess.dest = fedObj->lastMessage->dest.c_str ();
        mess.length = fedObj->lastMessage->data.size ();
        mess.original_source = fedObj->lastMessage->original_source.c_str ();
        mess.original_dest = fedObj->lastMessage->original_dest.c_str ();
        mess.source = fedObj->lastMessage->source.c_str ();
        mess.time = static_cast<helics_time> (fedObj->lastMessage->time);
        return mess;
    }
    return emptyMessage ();
}

bool checkOutArgString (const char *outputString, int maxlen, helics_error *err)
{
    static constexpr char invalidOutputString[] = "Output string location is invalid";
    if ((outputString == nullptr) || (maxlen <= 0))
    {
        if (err != nullptr)
        {
            err->error_code = helics_error_invalid_argument;
            err->message = invalidOutputString;
        }
        return false;
    }
    return true;
}

const char *helicsEndpointGetType (helics_endpoint endpoint)
{
    auto endObj = verifyEndpoint (endpoint, nullptr);
    if (endObj == nullptr)
    {
        return nullcstr;
    }

    try
    {
        auto &type = endObj->endPtr->getType ();
        return type.c_str ();
    }
    catch (...)
    {
        return nullcstr;
    }
}

const char *helicsEndpointGetName (helics_endpoint endpoint)
{
    auto endObj = verifyEndpoint (endpoint, nullptr);
    if (endObj == nullptr)
    {
        return nullcstr;
    }

    try
    {
        auto &type = endObj->endPtr->getName ();
        return type.c_str ();
    }
    catch (...)
    {
        return nullcstr;
    }
}

int helicsFederateGetEndpointCount (helics_federate fed)
{
    // this call should be with a nullptr since it can fail and still be a successful call
    auto mfedObj = getMessageFed (fed, nullptr);
    if (mfedObj == nullptr)
    {
        return 0;
    }
    return static_cast<int> (mfedObj->getEndpointCount ());
}

const char *helicsEndpointGetInfo (helics_endpoint end)
{
    auto endObj = verifyEndpoint (end, nullptr);
    if (endObj == nullptr)
    {
        return emptyStr.c_str ();
    }
    try
    {
        const std::string &info = endObj->endPtr->getInfo ();
        return info.c_str ();
    }
    catch (...)
    {
        return emptyStr.c_str ();
    }
}

void helicsEndpointSetInfo (helics_endpoint end, const char *info, helics_error *err)
{
    auto endObj = verifyEndpoint (end, err);
    if (endObj == nullptr)
    {
        return;
    }
    try
    {
        endObj->endPtr->setInfo (AS_STRING (info));
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

helics_bool helicsEndpointGetOption (helics_endpoint end, int option)
{
    auto endObj = verifyEndpoint (end, nullptr);
    if (endObj == nullptr)
    {
        return helics_false;
    }
    try
    {
        return (endObj->endPtr->getOption (option)) ? helics_true : helics_false;
    }
    catch (...)
    {
        return helics_false;
    }
}

void helicsEndpointSetOption (helics_endpoint end, int option, helics_bool value, helics_error *err)
{
    auto endObj = verifyEndpoint (end, err);
    if (endObj == nullptr)
    {
        return;
    }
    try
    {
        endObj->endPtr->setOption (option, (value == helics_true));
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}
