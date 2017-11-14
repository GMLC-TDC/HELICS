/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "application_api/Endpoints.hpp"
#include "application_api/application_api.h"
#include "core/helics-time.h"
#include "helics.h"
#include "shared_api_library/internal/api_objects.h"
#include <memory>
#include <mutex>
#include <vector>

static inline void addEndpoint (helics_message_federate fed, helics::EndpointObject *ept)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    fedObj->epts.push_back (ept);
}

helics_endpoint helicsRegisterEndpoint (helics_message_federate fed, const char *name, const char *type)
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
        end->endptr = std::make_unique<helics::Endpoint> (fedObj.get (), name, type);
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

helics_endpoint helicsRegisterGlobalEndpoint (helics_message_federate fed, const char *name, const char *type)
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
        end->endptr = std::make_unique<helics::Endpoint> (helics::GLOBAL, fedObj.get (), name, type);
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

helicsStatus helicsSetDefaultDestination (helics_endpoint endpoint, const char *dest)
{
    if (endpoint == nullptr)
    {
        return helicsError;
    }
    auto endObj = reinterpret_cast<helics::EndpointObject *> (endpoint);
    endObj->endptr->setTargetDestination (dest);
    return helicsOK;
}

helicsStatus helicsSendMessageRaw (helics_endpoint endpoint, const char *dest, const char *data, int len)
{
    if (endpoint == nullptr)
    {
        return helicsError;
    }
    auto endObj = reinterpret_cast<helics::EndpointObject *> (endpoint);
    if (dest == nullptr)
    {
        endObj->endptr->send (data, len);
    }
    else
    {
        endObj->endptr->send (dest, data, len);
    }
    return helicsOK;
}

helicsStatus
helicsSendEventRaw (helics_endpoint endpoint, const char *dest, const char *data, int len, helics_time_t time)
{
    if (endpoint == nullptr)
    {
        return helicsError;
    }
    auto endObj = reinterpret_cast<helics::EndpointObject *> (endpoint);
    if (dest == nullptr)
    {
        endObj->endptr->send (data, len, time);
    }
    else
    {
        endObj->endptr->send (dest, data, len, time);
    }
    return helicsOK;
}

helicsStatus helicsSendMessage (helics_endpoint endpoint, message_t *message)
{
    if (message == nullptr)
    {
        return helicsDiscard;
    }
    if (endpoint == nullptr)
    {
        return helicsError;
    }

    auto endObj = reinterpret_cast<helics::EndpointObject *> (endpoint);
    // TODO this isn't correct yet (need to translate to a Message_view if origSrc is not this name
    if (message->dst == nullptr)
    {
        endObj->endptr->send (message->data, message->length, message->time);
    }
    else
    {
        endObj->endptr->send (message->dst, message->data, message->length, message->time);
    }
    return helicsOK;
}

helicsStatus helicsSubscribe (helics_endpoint endpoint, const char *name, const char *type)
{
    if (endpoint == nullptr)
    {
        return helicsError;
    }
    auto endObj = reinterpret_cast<helics::EndpointObject *> (endpoint);

    endObj->endptr->subscribe (name, type);
    return helicsOK;
}

int helicsFederateHasMessage (helics_message_federate fed)
{
    if (fed == nullptr)
    {
        return false;
    }
    auto mFed = getMessageFed (fed);
    return (mFed->hasMessage ()) ? 1 : 0;
}

int helicsEndpointHasMessage (helics_endpoint endpoint)
{
    if (endpoint == nullptr)
    {
        return false;
    }

    auto endObj = reinterpret_cast<helics::EndpointObject *> (endpoint);
    return (endObj->endptr->hasMessage ()) ? 1 : 0;
}

int helicsFederateReceiveCount (helics_message_federate fed)
{
    if (fed == nullptr)
    {
        return 0;
    }
    auto mFed = getMessageFed (fed);
    return mFed->receiveCount ();
}

uint64_t helicsEndpointReceiveCount (helics_endpoint endpoint)
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
    message_t empty;
    empty.time = 0;
    empty.data = nullptr;
    empty.length = 0;
    empty.dst = nullptr;
    empty.origsrc = nullptr;
    empty.src = nullptr;
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
    mess.dst = endObj->lastMessage->dest.c_str ();
    mess.length = endObj->lastMessage->data.size ();
    mess.origsrc = endObj->lastMessage->origsrc.c_str ();
    mess.src = endObj->lastMessage->src.c_str ();
    mess.time = endObj->lastMessage->time.getBaseTimeCode ();
    return mess;
}

message_t helicsFederateGetMessage (helics_message_federate fed)
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
    mess.dst = fedObj->lastMessage->dest.c_str ();
    mess.length = fedObj->lastMessage->data.size ();
    mess.origsrc = fedObj->lastMessage->origsrc.c_str ();
    mess.src = fedObj->lastMessage->src.c_str ();
    mess.time = fedObj->lastMessage->time.getBaseTimeCode ();
    return mess;
}

helicsStatus helicsGetEndpointType (helics_endpoint endpoint, char *str, int maxlen)
{
    if (endpoint == nullptr)
    {
        return helicsError;
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
    return helicsOK;
}

helicsStatus helicsGetEndpointName (helics_endpoint endpoint, char *str, int maxlen)
{
    if (endpoint == nullptr)
    {
        return helicsError;
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
    return helicsOK;
}
