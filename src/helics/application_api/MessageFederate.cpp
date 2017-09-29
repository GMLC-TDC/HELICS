/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "MessageFederate.h"
#include "MessageFederateManager.h"
#include "core/core.h"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#include "json/json.h"
#pragma warning(pop)
#else
#include "json/json.h"
#endif

#include <fstream>

namespace helics
{
MessageFederate::MessageFederate (const FederateInfo &fi) : Federate (fi)
{
    mfManager = std::make_unique<MessageFederateManager> (coreObject, getID ());
}
MessageFederate::MessageFederate (std::shared_ptr<Core> core, const FederateInfo &fi)
    : Federate (std::move (core), fi)
{
    mfManager = std::make_unique<MessageFederateManager> (coreObject, getID ());
}
MessageFederate::MessageFederate (const std::string &file) : Federate (file)
{
    mfManager = std::make_unique<MessageFederateManager> (coreObject, getID ());
}

MessageFederate::MessageFederate ()
{
    // default constructor
}

MessageFederate::MessageFederate (bool)
{  // this constructor should only be called by child class that has already constructed the underlying federate in
   // a virtual inheritance
    mfManager = std::make_unique<MessageFederateManager> (coreObject, getID ());
}
MessageFederate::MessageFederate (MessageFederate &&mFed) noexcept = default;

MessageFederate &MessageFederate::operator= (MessageFederate &&mFed) noexcept
{
    if (getID () !=
        mFed.getID ())  // the id won't be moved, as it is copied so use it as a test if it has moved already
    {
        Federate::operator= (std::move (mFed));
    }
    mfManager = std::move (mFed.mfManager);
    return *this;
}

MessageFederate::~MessageFederate () = default;

void MessageFederate::updateTime (Time newTime, Time oldTime) { mfManager->updateTime (newTime, oldTime); }

void MessageFederate::StartupToInitializeStateTransition () { mfManager->StartupToInitializeStateTransition (); }
void MessageFederate::InitializeToExecuteStateTransition () { mfManager->InitializeToExecuteStateTransition (); }

endpoint_id_t MessageFederate::registerEndpoint (const std::string &name, const std::string &type)
{
    if (state == op_states::startup)
    {
        return mfManager->registerEndpoint (getName () + separator_ + name, type);
    }
    throw (InvalidFunctionCall ("cannot call register endpoint after entering initialization mode"));
}

endpoint_id_t MessageFederate::registerGlobalEndpoint (const std::string &name, const std::string &type)
{
    if (state == op_states::startup)
    {
        return mfManager->registerEndpoint (name, type);
    }
    throw (InvalidFunctionCall ("cannot call register endpoint after entering initialization mode"));
}

void MessageFederate::registerInterfaces (const std::string &jsonString)
{
    if (state != op_states::startup)
    {
        throw (InvalidFunctionCall ("cannot call register Interfaces after entering initialization mode"));
    }
    std::ifstream file (jsonString);
    Json_helics::Value doc;

    if (file.is_open ())
    {
        Json_helics::CharReaderBuilder rbuilder;
        std::string errs;
        bool ok = Json_helics::parseFromStream (rbuilder, file, &doc, &errs);
        if (!ok)
        {
            // should I throw an error here?
            return;
        }
    }
    else
    {
        Json_helics::Reader stringReader;
        bool ok = stringReader.parse (jsonString, doc, false);
        if (!ok)
        {
            // should I throw an error here?
            return;
        }
    }
    if (doc.isMember ("endpoints"))
    {
        auto epts = doc["endpoints"];
        for (auto eptIt = epts.begin (); eptIt != epts.end (); ++eptIt)
        {
            auto ept = (*eptIt);
            auto name = ept["name"].asString ();
            auto type = (ept.isMember ("type")) ? ept["type"].asString () : "";
            auto epid = registerEndpoint (name, type);
            // retrieve the known paths
            if (ept.isMember ("knownPaths"))
            {
                auto kp = ept["knownPaths"];
                if (kp.isString ())
                {
                    registerKnownCommunicationPath (epid, kp.asString ());
                }
                else if (kp.isArray ())
                {
                    for (auto kpIt = kp.begin (); kpIt != kp.end (); ++kpIt)
                    {
                        registerKnownCommunicationPath (epid, (*kpIt).asString ());
                    }
                }
            }
            // endpoints can subscribe to publications
            if (ept.isMember ("subscriptions"))
            {
                auto sub = ept["subscriptions"];
                if (sub.isString ())
                {
                    subscribe (epid, sub.asString (), "");
                }
                else if (sub.isArray ())
                {
                    for (auto subIt = sub.begin (); subIt != sub.end (); ++subIt)
                    {
                        subscribe (epid, (*subIt).asString (), "");
                    }
                }
            }
        }
    }
}

void MessageFederate::subscribe (endpoint_id_t endpoint, const std::string &name, const std::string &type)
{
    if (state == op_states::startup)
    {
        mfManager->subscribe (endpoint, name, type);
        return;
    }
    throw (InvalidFunctionCall ("subscriptions can only be created in startup mode"));
}

void MessageFederate::registerKnownCommunicationPath (endpoint_id_t localEndpoint,
                                                      const std::string &remoteEndpoint)
{
    if (state == op_states::startup)
    {
        mfManager->registerKnownCommunicationPath (localEndpoint, remoteEndpoint);
        return;
    }
    throw (InvalidFunctionCall ("paths can only be registered in startup mode"));
}

bool MessageFederate::hasMessage () const
{
    if (state == op_states::execution)
    {
        return mfManager->hasMessage ();
    }
    return false;
}

bool MessageFederate::hasMessage (endpoint_id_t id) const
{
    if (state == op_states::execution)
    {
        return mfManager->hasMessage (id);
    }
    return false;
}

/**
 * Returns the number of pending receives for the specified destination endpoint.
 */
uint64_t MessageFederate::receiveCount (endpoint_id_t id) const
{
    if (state == op_states::execution)
    {
        return mfManager->receiveCount (id);
    }
    return 0;
}
/**
* Returns the number of pending receives for the specified destination endpoint.
@details this function is not preferred in multithreaded contexts due to the required locking
prefer to just use getMessage until it returns an invalid Message.
*/
uint64_t MessageFederate::receiveCount () const
{
    if (state == op_states::execution)
    {
        return mfManager->receiveCount ();
    }
    return 0;
}

std::unique_ptr<Message> MessageFederate::getMessage ()
{
    if (state == op_states::execution)
    {
        return mfManager->getMessage ();
    }
    return nullptr;
}

std::unique_ptr<Message> MessageFederate::getMessage (endpoint_id_t endpoint)
{
    if (state == op_states::execution)
    {
        return mfManager->getMessage (endpoint);
    }
    return nullptr;
}

void MessageFederate::sendMessage (endpoint_id_t source, const std::string &dest, const data_view &data)
{
    if (state == op_states::execution)
    {
        mfManager->sendMessage (source, dest, data);
        return;
    }
    throw (InvalidFunctionCall ("cannot send message outside of execution state"));
}

void MessageFederate::sendMessage (endpoint_id_t source,
                                   const std::string &dest,
                                   const data_view &data,
                                   Time sendTime)
{
    if (state == op_states::execution)
    {
        mfManager->sendMessage (source, dest, data, sendTime);
        return;
    }
    throw (InvalidFunctionCall ("cannot send message outside of execution state"));
}

void MessageFederate::sendMessage (endpoint_id_t source, std::unique_ptr<Message> message)
{
    if (state == op_states::execution)
    {
        mfManager->sendMessage (source, std::move (message));
        return;
    }
    throw (InvalidFunctionCall ("cannot send message outside of execution state"));
}

void MessageFederate::sendMessage (endpoint_id_t source, const Message &message)
{
    if (state == op_states::execution)
    {
        mfManager->sendMessage (source, std::make_unique<Message> (message));
        return;
    }
    throw (InvalidFunctionCall ("cannot send message outside of execution state"));
}

endpoint_id_t MessageFederate::getEndpointId (const std::string &name) const
{
    auto id = mfManager->getEndpointId (name);
    if (id == invalid_id_value)
    {
        id = mfManager->getEndpointId (getName () + '.' + name);
    }
    return id;
}

std::string MessageFederate::getEndpointName (endpoint_id_t id) const { return mfManager->getEndpointName (id); }

std::string MessageFederate::getEndpointType (endpoint_id_t ep) { return mfManager->getEndpointType (ep); }

void MessageFederate::registerEndpointCallback (std::function<void(endpoint_id_t, Time)> func)
{
    mfManager->registerCallback (func);
}
void MessageFederate::registerEndpointCallback (endpoint_id_t ep, std::function<void(endpoint_id_t, Time)> func)
{
    mfManager->registerCallback (ep, func);
}
void MessageFederate::registerEndpointCallback (const std::vector<endpoint_id_t> &ep,
                                                std::function<void(endpoint_id_t, Time)> func)
{
    mfManager->registerCallback (ep, func);
}
}