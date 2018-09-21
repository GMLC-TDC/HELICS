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
static constexpr char nullcstr[] = "";

static constexpr char invalidEndpoint[] = "The given endpoint does not point to a valid object";



static helics::EndpointObject *verifyEndpoint (helics_endpoint ept, helics_error *err)
{ 
	HELICS_ERROR_CHECK (err, nullptr);
	if (ept==nullptr)
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



static constexpr char invalidEndName[] = "the specified Endpoint name is not recognized";

helics_endpoint helicsFederateGetEndpoint (helics_federate fed, const char *name, helics_error *err)
{
    auto fedObj = getMessageFedSharedPtr (fed, err);
    if (!fedObj)
    {
        return nullptr;
    }
    try
    {
        auto id = fedObj->getEndpointId (name);
        if (id == helics::invalid_id_value)
        {
            err->error_code = helics_error_invalid_argument;
            err->message = invalidEndName;
            return nullptr;
        }
        auto end = std::make_unique<helics::EndpointObject> ();
        end->endptr = std::make_unique<helics::Endpoint> (fedObj.get (), id.value ());
        end->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_input> (end.get ());
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

static constexpr char emptyMessageErrorString[] = "the message is NULL";

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
            err->error_code = helics_error_invalid_argument;
            err->message = emptyMessageErrorString;
		}
    }
  
    try
    {
        if ((message->original_source==nullptr)||(endObj->endptr->getName() == message->original_source))
        {
            if (message->dest == nullptr)
            {
                endObj->endptr->send(message->data, message->length, message->time);
            }
            else
            {
                endObj->endptr->send(message->dest, message->data, message->length, message->time);
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
            endObj->endptr->send (nmessage);
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

helics_bool_t helicsFederateHasMessage (helics_federate fed)
{

    auto mFed = getMessageFed (fed,nullptr);
    if (mFed == nullptr)
    {
        return helics_false;
    }
    return (mFed->hasMessage ()) ? helics_true : helics_false;
}

helics_bool_t helicsEndpointHasMessage (helics_endpoint endpoint)
{
    auto endObj = verifyEndpoint (endpoint, nullptr);
    if (endObj == nullptr)
    {
        return helics_false;
    }
    return (endObj->endptr->hasMessage ()) ? helics_true : helics_false;
}

int helicsFederatePendingMessages (helics_federate fed)
{
    auto mFed = getMessageFed (fed,nullptr);
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

message_t helicsEndpointGetMessage (helics_endpoint endpoint)
{
    auto endObj = verifyEndpoint (endpoint, nullptr);
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

message_t helicsFederateGetMessage (helics_federate fed)
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
            err->error_code = helics_error_invalid_argument;
            err->message = invalidOutputString;
		}
        return false;
    }
    return true;
}

const char * helicsEndpointGetType (helics_endpoint endpoint)
{
    auto endObj = verifyEndpoint (endpoint, nullptr);
    if (endObj == nullptr)
    {
        return nullcstr;
    }
	
    try
    {
        auto &type = endObj->endptr->getType ();
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
        auto &type = endObj->endptr->getName ();
        return type.c_str ();
    }
    catch (...)
    {
        return nullcstr;
    }
}

int helicsFederateGetEndpointCount (helics_federate fed)
{
	//this call should be with a nullptr since it can fail and still be a successful call
    auto mfedObj = getMessageFed (fed,nullptr);
    if (mfedObj == nullptr)
    {
        return 0;
    }
    return static_cast<int> (mfedObj->getEndpointCount ());
}
