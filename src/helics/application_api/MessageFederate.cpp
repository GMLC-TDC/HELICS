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
#include "../common/TomlProcessingFunctions.hpp"


namespace helics
{
MessageFederate::MessageFederate (const std::string &fedName, const FederateInfo &fi) : Federate (fedName,fi)
{
    mfManager = std::make_unique<MessageFederateManager> (coreObject.get (), getID ());
}
MessageFederate::MessageFederate (const std::string &fedName,
                                  const std::shared_ptr<Core> &core,
                                  const FederateInfo &fi)
    : Federate (fedName,core, fi)
{
    mfManager = std::make_unique<MessageFederateManager> (coreObject.get (), getID ());
}
MessageFederate::MessageFederate (const std::string &configString) : Federate (std::string(),loadFederateInfo (configString))
{
    mfManager = std::make_unique<MessageFederateManager> (coreObject.get (), getID ());
    MessageFederate::registerInterfaces (configString);
}

MessageFederate::MessageFederate (const std::string &fedName, const std::string &configString)
    : Federate (fedName,loadFederateInfo (configString))
{
    mfManager = std::make_unique<MessageFederateManager> (coreObject.get (), getID ());
    MessageFederate::registerInterfaces (configString);
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

endpoint_id_t MessageFederate::registerEndpoint (const std::string &eptName, const std::string &type)
{
    if (state == op_states::startup)
    {
        return mfManager->registerEndpoint ((!eptName.empty())?(getName () + separator_ + eptName):eptName, type);
    }
    throw (InvalidFunctionCall ("cannot call register endpoint after entering initialization mode"));
}

endpoint_id_t MessageFederate::registerGlobalEndpoint (const std::string &eptName, const std::string &type)
{
    if (state == op_states::startup)
    {
        return mfManager->registerEndpoint (eptName, type);
    }
    throw (InvalidFunctionCall ("cannot call register endpoint after entering initialization mode"));
}

void MessageFederate::registerInterfaces (const std::string &configString)
{
    registerMessageInterfaces (configString);
    Federate::registerFilterInterfaces (configString);
}

void MessageFederate::registerMessageInterfaces(const std::string &configString)
{
    if (state != op_states::startup)
    {
        throw (InvalidFunctionCall ("cannot call register Interfaces after entering initialization mode"));
    }
    if (hasTomlExtension(configString))
    {
        registerMessageInterfacesToml (configString);
    }
    else
    {
        registerMessageInterfacesJson (configString);
    }
}

void MessageFederate::registerMessageInterfacesJson (const std::string &jsonString)
{
    
    auto doc = loadJson (jsonString);

    if (doc.isMember ("endpoints"))
    {
        for (const auto &ept : doc["endpoints"])
        {
            auto eptName = getKey (ept);
            auto type = (ept.isMember ("type")) ? ept["type"].asString () : "";
            bool global = (ept.isMember ("global")) ? (ept["global"].asBool ()) : false;
            endpoint_id_t epid;
            if (global)
            {
                epid = registerGlobalEndpoint (eptName, type);
            }
            else
            {
                epid = registerEndpoint (eptName, type);
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
                    subscribe (epid, subs.asString ());
                }
                else if (subs.isArray ())
                {
                    for (const auto &sub : subs)
                    {
                        subscribe (epid, sub.asString ());
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

void MessageFederate::registerMessageInterfacesToml (const std::string &tomlString)
{
    
    toml::Value doc;
    try
    {
        doc = loadToml (tomlString);
    }
    catch (const std::invalid_argument &ia)
    {
        throw (helics::InvalidParameter (ia.what ()));
    }

   auto epts = doc.find ("endpoints");
    if (epts != nullptr)
    {
        auto &eptArray = epts->as<toml::Array> ();
        for (auto &ept:eptArray)
        {
            auto key = getKey (ept);
            auto type = tomlGetOrDefault (ept, "type", std::string ());
            bool global = tomlGetOrDefault(ept,"global",false);
            endpoint_id_t epid;
            if (global)
            {
                epid = registerGlobalEndpoint (key, type);
            }
            else
            {
                epid = registerEndpoint (key, type);
            }

            // retrieve the known paths
            auto kp = ept.find("knownDestinations");
            if (kp!=nullptr)
            {
                if (kp->is<toml::Array> ())
                {
                    for (const auto &path : kp->as<toml::Array>())
                    {
                        registerKnownCommunicationPath (epid, path.as<std::string> ());
                    }
                    
                }
                else if (kp->is<std::string>())
                {
                    registerKnownCommunicationPath (epid, kp->as<std::string> ());
                }
            }
            auto subs = ept.find ("subscriptions");
            // endpoints can subscribe to publications
            if (subs!=nullptr)
            {
                if (subs->is<std::string> ())
                {
                    subscribe (epid, subs->as<std::string> ());
                }
                else if (subs->is<toml::Array> ())
                {
                    for (const auto &sub : subs->as<toml::Array>())
                    {
                        subscribe (epid, sub.as<std::string> ());
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

void MessageFederate::subscribe (endpoint_id_t endpoint, const std::string &key)
{
    if (state == op_states::startup)
    {
        mfManager->subscribe (endpoint, key);
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
uint64_t MessageFederate::pendingMessages (endpoint_id_t id) const
{
    if (state == op_states::execution)
    {
        return mfManager->pendingMessages (id);
    }
    return 0;
}
/**
* Returns the number of pending receives for the specified destination endpoint.
@details this function is not preferred in multithreaded contexts due to the required locking
prefer to just use getMessage until it returns an invalid Message.
*/
uint64_t MessageFederate::pendingMessages () const
{
    if (state == op_states::execution)
    {
        return mfManager->pendingMessages ();
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

endpoint_id_t MessageFederate::getEndpointId (const std::string &eptName) const
{
    auto id = mfManager->getEndpointId (eptName);
    if (id == invalid_id_value)
    {
        id = mfManager->getEndpointId (getName () + '.' + eptName);
    }
    return id;
}

const std::string &MessageFederate::getEndpointName (endpoint_id_t id) const { return mfManager->getEndpointName (id); }

const std::string &MessageFederate::getEndpointType (endpoint_id_t ep) { return mfManager->getEndpointType (ep); }

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


void MessageFederate::setEndpointOption(endpoint_id_t id, int32_t option, bool option_value)
{
	mfManager->setEndpointOption(id, option, option_value);
}

}  // namespace helics
