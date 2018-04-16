/*

Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "MessageFederate.hpp"
#include "../common/JsonProcessingFunctions.hpp"
#include "../core/Core.hpp"
#include "../core/core-exceptions.hpp"
#include "MessageFederateManager.hpp"

namespace helics
{
MessageFederate::MessageFederate (const FederateInfo &fi) : Federate (fi)
{
    mfManager = std::make_unique<MessageFederateManager> (coreObject.get (), getID ());
}
MessageFederate::MessageFederate (const std::shared_ptr<Core> &core, const FederateInfo &fi) : Federate (core, fi)
{
    mfManager = std::make_unique<MessageFederateManager> (coreObject.get (), getID ());
}
MessageFederate::MessageFederate (const std::string &jsonString) : Federate (loadFederateInfo (jsonString))
{
    mfManager = std::make_unique<MessageFederateManager> (coreObject.get (), getID ());
    registerInterfaces (jsonString);
}

MessageFederate::MessageFederate (const std::string &name, const std::string &jsonString)
    : Federate (loadFederateInfo (name, jsonString))
{
    mfManager = std::make_unique<MessageFederateManager> (coreObject.get (), getID ());
    registerInterfaces (jsonString);
}

MessageFederate::MessageFederate ()
{
    // default constructor
}

MessageFederate::MessageFederate (bool)
{  // this constructor should only be called by child class that has already constructed the underlying federate in
   // a virtual inheritance
    mfManager = std::make_unique<MessageFederateManager> (coreObject.get (), getID ());
}
MessageFederate::MessageFederate (MessageFederate &&) noexcept = default;

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

void MessageFederate::disconnect ()
{
    Federate::disconnect ();
    mfManager->disconnect ();
}

void MessageFederate::updateTime (Time newTime, Time oldTime) { mfManager->updateTime (newTime, oldTime); }

void MessageFederate::startupToInitializeStateTransition () { mfManager->startupToInitializeStateTransition (); }
void MessageFederate::initializeToExecuteStateTransition () { mfManager->initializeToExecuteStateTransition (); }

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
    registerMessageInterfaces (jsonString);
    Federate::registerFilterInterfaces (jsonString);
}

void MessageFederate::registerMessageInterfaces (const std::string &jsonString)
{
    if (state != op_states::startup)
    {
        throw (InvalidFunctionCall ("cannot call register Interfaces after entering initialization mode"));
    }
    auto doc = loadJsonString (jsonString);

    if (doc.isMember ("endpoints"))
    {
        for (const auto &ept : doc["endpoints"])
        {
            auto name = getKey (ept);
            auto type = (ept.isMember ("type")) ? ept["type"].asString () : "";
            bool global = (ept.isMember ("global")) ? (ept["global"].asBool ()) : false;
            endpoint_id_t epid;
            if (global)
            {
                epid = registerGlobalEndpoint (name, type);
            }
            else
            {
                epid = registerEndpoint (name, type);
            }

            // retrieve the known paths
            if (ept.isMember ("knownDestinations"))
            {
                auto kp = ept["knownDestinations"];
                if (kp.isString ())
                {
                    registerKnownCommunicationPath (epid, kp.asString ());
                }
                else if (kp.isArray ())
                {
                    for (const auto &path : kp)
                    {
                        registerKnownCommunicationPath (epid, path.asString ());
                    }
                }
            }
            // endpoints can subscribe to publications
            if (ept.isMember ("subscriptions"))
            {
                auto subs = ept["subscriptions"];
                if (subs.isString ())
                {
                    subscribe (epid, subs.asString (), std::string ());
                }
                else if (subs.isArray ())
                {
                    for (const auto &sub : subs)
                    {
                        subscribe (epid, sub.asString (), std::string ());
                    }
                }
            }
        }
    }
    /*
    // retrieve the known paths
    if (doc.isMember("knownDestinations"))
    {
        auto kp = doc["knownDestinations"];
        if (kp.isString())
        {
           // registerKnownCommunicationPath(epid, kp.asString());
        }
        else if (kp.isArray())
        {
           for (const auto &path : kp)
            {
           //     registerKnownCommunicationPath(epid, (*kpIt).asString());
            }
        }
    }
    */
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

void MessageFederate::registerEndpointCallback (const std::function<void(endpoint_id_t, Time)> &func)
{
    mfManager->registerCallback (func);
}
void MessageFederate::registerEndpointCallback (endpoint_id_t ep,
                                                const std::function<void(endpoint_id_t, Time)> &func)
{
    mfManager->registerCallback (ep, func);
}
void MessageFederate::registerEndpointCallback (const std::vector<endpoint_id_t> &ep,
                                                const std::function<void(endpoint_id_t, Time)> &func)
{
    mfManager->registerCallback (ep, func);
}

/** get a count of the number endpoints registered*/
int MessageFederate::getEndpointCount () const { return mfManager->getEndpointCount (); }

}  // namespace helics
