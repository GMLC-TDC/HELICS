/*
Copyright © 2017-2018,
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
static const int EndpointValidationIdentifier = 0xB453'94C2;

static inline void addEndpoint (helics_federate fed, std::unique_ptr<helics::EndpointObject> ept)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    ept->valid = EndpointValidationIdentifier;
    fedObj->epts.push_back (std::move(ept));
}

static const std::string nullStr;

static const char invalidEndpoint[] = "The given endpoint does not point to a valid object";



static helics::EndpointObject *verifyEndpoint (helics_endpoint ept, helics_error *err)
{ 
	HELICS_ERROR_CHECK (err, nullptr);
	if (ept==nullptr)
    {
		if (err != nullptr)
		{
            err->error_code = helics_invalid_object;
            err->message = invalidEndpoint;
		}
        return nullptr;
    }
    auto endObj = reinterpret_cast<helics::EndpointObject *> (ept);
	if (endObj->valid == EndpointValidationIdentifier)
	{
        if (err != nullptr)
        {
            err->error_code = helics_invalid_object;
            err->message = invalidEndpoint;
        }
        return nullptr;
	}
    return endObj;
}

    helics_endpoint helicsFederateRegisterEndpoint (helics_federate fed, const char *name, const char *type, helics_error *err)
    {
    // now generate a generic endpoint
    auto fedObj = getMessageFedSharedPtr (fed,err);
    if (!fedObj)
    {
        return nullptr;
    }
    try
    {
        auto end = std::make_unique<helics::EndpointObject> ();
        end->endptr = std::make_unique<helics::Endpoint> (fedObj.get (), (name != nullptr) ? std::string (name) : nullStr,
                                                          (type == nullptr) ? nullStr : std::string (type));
        end->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_endpoint> (end.get());
        addEndpoint (fed, std::move(end));
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
    auto fedObj = getMessageFedSharedPtr (fed,err);
    if (!fedObj)
    {
        return nullptr;
    }
    try
    {
        auto end = std::make_unique<helics::EndpointObject>();
        end->endptr = std::make_unique<helics::Endpoint> (helics::GLOBAL, fedObj.get (), (name != nullptr) ? std::string (name) : nullStr,
                                                          (type == nullptr) ? nullStr : std::string (type));
        end->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_endpoint> (end.get());
        addEndpoint(fed, std::move(end));
        return ret;
    }
    catch (...)
    {
        return nullptr;
    }
    return nullptr;
}

void helicsEndpointSetDefaultDestination (helics_endpoint endpoint, const char *dest, helics_error *err)
{
    auto endObj = verifyEndpoint (endpoint, err);
	if (endObj == nullptr)
	{
        return;
	}
    try
    {
        endObj->endptr->setTargetDestination (dest);
    }
    catch (...)
    {
         helicsErrorHandler (err);
    }
}

void
helicsEndpointSendMessageRaw (helics_endpoint endpoint, const char *dest, const void *data, int inputDataLength, helics_error *err)
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
                endObj->endptr->send (std::string ());
            }
            else
            {
                endObj->endptr->send (dest, std::string ());
            }
        }
        else
        {
            if ((dest == nullptr) || (std::string (dest).empty ()))
            {
                endObj->endptr->send ((const char *)data, inputDataLength);
            }
            else
            {
                endObj->endptr->send (dest, (const char *)data, inputDataLength);
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
                                          helics_time_t time,
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
                endObj->endptr->send (std::string (), time);
            }
            else
            {
                endObj->endptr->send (dest, std::string (), time);
            }
        }
        else
        {
            if ((dest == nullptr) || (std::string (dest).empty ()))
            {
                endObj->endptr->send ((const char *)data, inputDataLength, time);
            }
            else
            {
                endObj->endptr->send (dest, (const char *)data, inputDataLength, time);
            }
        }
    }
    catch (...)
    {
        return helicsErrorHandler (err);
    }
}

void helicsEndpointSendMessage (helics_endpoint endpoint, message_t *message, helics_error *err)
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
            err->error_code = helics_invalid_argument;
            err->message = getMasterHolder ()->addErrorString ("message is null");
		}
    }
  
    try
    {
        // TODO this isn't correct yet (need to translate to a Message_view if origSrc is not this name
        if (message->dest == nullptr)
        {
            endObj->endptr->send (message->data, message->length, message->time);
        }
        else
        {
            endObj->endptr->send (message->dest, message->data, message->length, message->time);
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
    try
    {
        endObj->endptr->subscribe (key);
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

helics_bool_t helicsFederateHasMessage (helics_federate fed, helics_error *err)
{

    auto mFed = getMessageFed (fed,err);
    if (mFed == nullptr)
    {
        return helics_false;
    }
    return (mFed->hasMessage ()) ? helics_true : helics_false;
}

helics_bool_t helicsEndpointHasMessage (helics_endpoint endpoint, helics_error *err)
{
    auto endObj = verifyEndpoint (endpoint, err);
    if (endObj == nullptr)
    {
        return helics_false;
    }
    return (endObj->endptr->hasMessage ()) ? helics_true : helics_false;
}

int helicsFederatePendingMessages (helics_federate fed, helics_error *err)
{
    auto mFed = getMessageFed (fed,err);
    if (mFed == nullptr)
    {
        return 0;
    }
    return mFed->pendingMessages ();
}

int helicsEndpointPendingMessages (helics_endpoint endpoint, helics_error *err)
{
    auto endObj = verifyEndpoint (endpoint, err);
    if (endObj == nullptr)
    {
        return 0;
    }
    return endObj->endptr->pendingMessages ();
}

static message_t emptyMessage ()
{
    message_t empty;
    empty.time = 0;
    empty.data = nullptr;
    empty.length = 0;
    empty.dest = nullptr;
    empty.original_source = nullptr;
    empty.original_dest = nullptr;
    empty.source = nullptr;
    return empty;
}

message_t helicsEndpointGetMessage (helics_endpoint endpoint, helics_error *err)
{
    auto endObj = verifyEndpoint (endpoint, err);
    if (endObj == nullptr)
    {
        return emptyMessage ();
    }

    endObj->lastMessage = endObj->endptr->getMessage ();

    if (endObj->lastMessage)
    {
        message_t mess{};
        mess.data = endObj->lastMessage->data.data ();
        mess.dest = endObj->lastMessage->dest.c_str ();
        mess.length = endObj->lastMessage->data.size ();
        mess.original_source = endObj->lastMessage->original_source.c_str ();
        mess.source = endObj->lastMessage->source.c_str ();
        mess.original_dest = endObj->lastMessage->original_dest.c_str ();
        mess.time = static_cast<helics_time_t> (endObj->lastMessage->time);
        return mess;
    }
    return emptyMessage ();
}

message_t helicsFederateGetMessage (helics_federate fed, helics_error *err)
{
    auto mFed = getMessageFed (fed, err);
    if (mFed == nullptr)
    {
        return emptyMessage ();
    }

    auto fedObj = helics::getFedObject (fed, err);

    fedObj->lastMessage = mFed->getMessage ();

    if (fedObj->lastMessage)
    {
        message_t mess{};
        mess.data = fedObj->lastMessage->data.data ();
        mess.dest = fedObj->lastMessage->dest.c_str ();
        mess.length = fedObj->lastMessage->data.size ();
        mess.original_source = fedObj->lastMessage->original_source.c_str ();
        mess.original_dest = fedObj->lastMessage->original_dest.c_str ();
        mess.source = fedObj->lastMessage->source.c_str ();
        mess.time = static_cast<helics_time_t> (fedObj->lastMessage->time);
        return mess;
    }
    return emptyMessage ();
}

bool checkOutArgString(char *outputString, int maxlen, helics_error *err)
{
    static constexpr char invalidOutputString[] = "Output string location is invalid";
    if ((outputString == nullptr) || (maxlen <= 0))
    {
		if (err != nullptr)
		{
            err->error_code = helics_invalid_argument;
            err->message = invalidOutputString;
		}
        return false;
    }
    return true;
}

void helicsEndpointGetType (helics_endpoint endpoint, char *outputString, int maxlen, helics_error *err)
{
    auto endObj = verifyEndpoint (endpoint, err);
    if (endObj == nullptr)
    {
        return ;
    }
	if (!checkOutArgString(outputString, maxlen, err))
	{
        return;
	}
    
    try
    {
        auto type = endObj->endptr->getType ();
        if (static_cast<int> (type.size ()) > maxlen)
        {
            strncpy (outputString, type.c_str (), maxlen);
            outputString[maxlen - 1] = 0;
        }
        else
        {
            strcpy (outputString, type.c_str ());
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsEndpointGetName (helics_endpoint endpoint, char *outputString, int maxlen, helics_error *err)
{
    auto endObj = verifyEndpoint (endpoint, err);
    if (endObj == nullptr)
    {
        return;
    }
    if (!checkOutArgString (outputString, maxlen, err))
    {
        return;
    }
    try
    {
        auto type = endObj->endptr->getName ();
        if (static_cast<int> (type.size ()) > maxlen)
        {
            strncpy (outputString, type.c_str (), maxlen);
            outputString[maxlen - 1] = 0;
        }
        else
        {
            strcpy (outputString, type.c_str ());
        }
    }
    catch (...)
    {
        return helicsErrorHandler (err);
    }
}

int helicsFederateGetEndpointCount (helics_federate fed, helics_error *err)
{
    HELICS_ERROR_CHECK (err, -1);
	//this call should be with a nullptr since it can fail and still be a successful call
    auto mfedObj = getMessageFed (fed,nullptr);
    if (mfedObj == nullptr)
    {
        auto fedObj = getFed (fed,err);
        // if this is not nullptr than it is a valid fed object just not a message federate object so it has 0 endpoints
        return (fedObj != nullptr) ? 0 : (-1);
    }
    return static_cast<int> (mfedObj->getEndpointCount ());
}
