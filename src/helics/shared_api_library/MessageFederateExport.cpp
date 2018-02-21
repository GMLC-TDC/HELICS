/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

#include "../core/core-exceptions.hpp"
#include "../helics.hpp"
#include "MessageFederate.h"
#include "helics.h"
#include "internal/api_objects.h"
#include <memory>
#include <mutex>
#include <vector>

static inline void addEndpoint (helics_federate fed, helics::EndpointObject *ept)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    fedObj->epts.push_back (ept);
}

static const std::string nullStr;
helics_endpoint helicsFederateRegisterEndpoint (helics_federate fed, const char *name, const char *type)
{
    // now generate a generic subscription
    auto fedObj = getMessageFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }
    helics::EndpointObject *end = nullptr;
    try
    {
        end = new helics::EndpointObject ();
        end->endptr = std::make_unique<helics::Endpoint> (fedObj.get (), (name != nullptr) ? std::string (name) : nullStr,
                                                          (type == nullptr) ? nullStr : std::string (type));
        end->fedptr = std::move (fedObj);
        addEndpoint (fed, end);
        return reinterpret_cast<helics_endpoint> (end);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete end;
    }
    return nullptr;
}

helics_endpoint helicsFederateRegisterGlobalEndpoint (helics_federate fed, const char *name, const char *type)
{
    // now generate a generic subscription
    auto fedObj = getMessageFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }
    helics::EndpointObject *end = nullptr;
    try
    {
        end = new helics::EndpointObject ();
        end->endptr = std::make_unique<helics::Endpoint> (helics::GLOBAL, fedObj.get (), (name != nullptr) ? std::string (name) : nullStr,
                                                          (type == nullptr) ? nullStr : std::string (type));
        end->fedptr = std::move (fedObj);
        addEndpoint (fed, end);
        return reinterpret_cast<helics_endpoint> (end);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete end;
    }
    return nullptr;
}

helics_status helicsEndpointSetDefaultDestination (helics_endpoint endpoint, const char *dest)
{
    if (endpoint == nullptr)
    {
        return helics_invalid_object;
    }
    auto endObj = reinterpret_cast<helics::EndpointObject *> (endpoint);
    endObj->endptr->setTargetDestination (dest);
    return helics_ok;
}

helics_status helicsEndpointSendMessageRaw (helics_endpoint endpoint, const char *dest, const char *data, int len)
{
    if (endpoint == nullptr)
    {
        return helics_invalid_object;
    }
    auto endObj = reinterpret_cast<helics::EndpointObject *> (endpoint);
    if ((dest == nullptr) || (std::string (dest).empty ()))
    {
        endObj->endptr->send (data, len);
    }
    else
    {
        endObj->endptr->send (dest, data, len);
    }
    return helics_ok;
}

helics_status helicsEndpointSendEventRaw (helics_endpoint endpoint, const char *dest, const char *data, int len, helics_time_t time)
{
    if (endpoint == nullptr)
    {
        return helics_invalid_object;
    }
    auto endObj = reinterpret_cast<helics::EndpointObject *> (endpoint);
    if ((dest == nullptr) || (std::string (dest).empty ()))
    {
        endObj->endptr->send (data, len, time);
    }
    else
    {
        endObj->endptr->send (dest, data, len, time);
    }
    return helics_ok;
}

helics_status helicsEndpointSendMessage (helics_endpoint endpoint, message_t *message)
{
    if (message == nullptr)
    {
        return helics_invalid_argument;
    }
    if (endpoint == nullptr)
    {
        return helics_invalid_object;
    }

    auto endObj = reinterpret_cast<helics::EndpointObject *> (endpoint);
    // TODO this isn't correct yet (need to translate to a Message_view if origSrc is not this name
    if (message->dest == nullptr)
    {
        endObj->endptr->send (message->data, message->length, message->time);
    }
    else
    {
        endObj->endptr->send (message->dest, message->data, message->length, message->time);
    }
    return helics_ok;
}

helics_status helicsEndpointSubscribe (helics_endpoint endpoint, const char *key, const char *type)
{
    if (endpoint == nullptr)
    {
        return helics_error;
    }
    auto endObj = reinterpret_cast<helics::EndpointObject *> (endpoint);

    endObj->endptr->subscribe (key, (type == nullptr) ? nullStr : std::string (type));
    return helics_ok;
}

helics_bool_t helicsFederateHasMessage (helics_federate fed)
{
    if (fed == nullptr)
    {
        return false;
    }
    auto mFed = getMessageFed (fed);
    if (mFed == nullptr)
    {
        return false;
    }
    return (mFed->hasMessage ()) ? helics_true : helics_false;
}

helics_bool_t helicsEndpointHasMessage (helics_endpoint endpoint)
{
    if (endpoint == nullptr)
    {
        return false;
    }

    auto endObj = reinterpret_cast<helics::EndpointObject *> (endpoint);
    return (endObj->endptr->hasMessage ()) ? helics_true : helics_false;
}

int helicsFederateReceiveCount (helics_federate fed)
{
    if (fed == nullptr)
    {
        return 0;
    }
    auto mFed = getMessageFed (fed);
    if (mFed == nullptr)
    {
        return 0;
    }
    return mFed->receiveCount ();
}

int helicsEndpointReceiveCount (helics_endpoint endpoint)
{
    if (endpoint == nullptr)
    {
        return 0;
    }

    auto endObj = reinterpret_cast<helics::EndpointObject *> (endpoint);
    return endObj->endptr->receiveCount ();
}

static message_t emptyMessage ()
{
    message_t empty{};
    empty.time = 0;
    empty.data = nullptr;
    empty.length = 0;
    empty.dest = nullptr;
    empty.original_source = nullptr;
    empty.source = nullptr;
    return empty;
}

message_t helicsEndpointGetMessage (helics_endpoint endpoint)
{
    if (endpoint == nullptr)
    {
        return emptyMessage ();
    }

    auto endObj = reinterpret_cast<helics::EndpointObject *> (endpoint);
    endObj->lastMessage = endObj->endptr->getMessage ();
    message_t mess{};
    mess.data = endObj->lastMessage->data.data ();
    mess.dest = endObj->lastMessage->dest.c_str ();
    mess.length = endObj->lastMessage->data.size ();
    mess.original_source = endObj->lastMessage->original_source.c_str ();
    mess.source = endObj->lastMessage->source.c_str ();
    mess.time = endObj->lastMessage->time.getBaseTimeCode ();
    return mess;
}

message_t helicsFederateGetMessage (helics_federate fed)
{
    if (fed == nullptr)
    {
        return emptyMessage ();
    }
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    auto mFed = dynamic_cast<helics::MessageFederate *> (fedObj->fedptr.get ());
    if (mFed == nullptr)
    {
        return emptyMessage ();
    }
    fedObj->lastMessage = mFed->getMessage ();
    message_t mess{};
    mess.data = fedObj->lastMessage->data.data ();
    mess.dest = fedObj->lastMessage->dest.c_str ();
    mess.length = fedObj->lastMessage->data.size ();
    mess.original_source = fedObj->lastMessage->original_source.c_str ();
    mess.source = fedObj->lastMessage->source.c_str ();
    mess.time = fedObj->lastMessage->time.getBaseTimeCode ();
    return mess;
}

helics_status helicsEndpointGetType (helics_endpoint endpoint, char *str, int maxlen)
{
    if (endpoint == nullptr)
    {
        return helics_invalid_object;
    }
    if ((str == nullptr) || (maxlen <= 0))
    {
        return helics_invalid_argument;
    }
    auto endObj = reinterpret_cast<helics::EndpointObject *> (endpoint);
    auto type = endObj->endptr->getType ();
    if (static_cast<int> (type.size ()) > maxlen)
    {
        strncpy (str, type.c_str (), maxlen);
        str[maxlen - 1] = 0;
    }
    else
    {
        strcpy (str, type.c_str ());
    }
    return helics_ok;
}

helics_status helicsEndpointGetName (helics_endpoint endpoint, char *str, int maxlen)
{
    if (endpoint == nullptr)
    {
        return helics_invalid_object;
    }
    if ((str == nullptr) || (maxlen <= 0))
    {
        return helics_invalid_argument;
    }
    auto endObj = reinterpret_cast<helics::EndpointObject *> (endpoint);
    auto type = endObj->endptr->getName ();
    if (static_cast<int> (type.size ()) > maxlen)
    {
        strncpy (str, type.c_str (), maxlen);
        str[maxlen - 1] = 0;
    }
    else
    {
        strcpy (str, type.c_str ());
    }
    return helics_ok;
}

int helicsFederateGetEndpointCount(helics_federate fed)
{
    if (fed == nullptr)
    {
        return (-1);
    }
    auto mfedObj = getMessageFed(fed);
    if (mfedObj == nullptr)
    {
        auto fedObj = getFed(fed);
        //if this is not nullptr than it is a valid fed object just not a message federate object so it has 0 endpoints
        return (fedObj != nullptr) ? 0 : (-1);
    }
    return static_cast<int>(mfedObj->getEndpointCount());
}
