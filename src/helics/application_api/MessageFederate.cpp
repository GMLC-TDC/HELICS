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
#include "Endpoints.hpp"


namespace helics
{
MessageFederate::MessageFederate (const std::string &fedName, const FederateInfo &fi) : Federate (fedName,fi)
{
    mfManager = std::make_unique<MessageFederateManager> (coreObject.get (),this, getID ());
}
MessageFederate::MessageFederate (const std::string &fedName,
                                  const std::shared_ptr<Core> &core,
                                  const FederateInfo &fi)
    : Federate (fedName,core, fi)
{
    mfManager = std::make_unique<MessageFederateManager> (coreObject.get (),this, getID ());
}
MessageFederate::MessageFederate (const std::string &configString) : Federate (std::string(),loadFederateInfo (configString))
{
    mfManager = std::make_unique<MessageFederateManager> (coreObject.get (),this, getID ());
    MessageFederate::registerInterfaces (configString);
}

MessageFederate::MessageFederate (const std::string &fedName, const std::string &configString)
    : Federate (fedName,loadFederateInfo (configString))
{
    mfManager = std::make_unique<MessageFederateManager> (coreObject.get (),this, getID ());
    MessageFederate::registerInterfaces (configString);
}

MessageFederate::MessageFederate ()
{
    // default constructor
}

MessageFederate::MessageFederate (bool)
{  // this constructor should only be called by child class that has already constructed the underlying federate in
   // a virtual inheritance
    mfManager = std::make_unique<MessageFederateManager> (coreObject.get (), this, getID ());
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


std::string MessageFederate::localQuery(const std::string &queryStr) const
{
    return mfManager->localQuery(queryStr);
}

Endpoint &MessageFederate::registerEndpoint (const std::string &eptName, const std::string &type)
{
        return mfManager->registerEndpoint ((!eptName.empty())?(getName () + separator_ + eptName):eptName, type);
}

Endpoint &MessageFederate::registerGlobalEndpoint (const std::string &eptName, const std::string &type)
{
    return mfManager->registerEndpoint (eptName, type);
}

void MessageFederate::registerInterfaces (const std::string &configString)
{
    registerMessageInterfaces (configString);
    Federate::registerFilterInterfaces (configString);
}

void MessageFederate::registerMessageInterfaces(const std::string &configString)
{
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
            auto type = jsonGetOrDefault (ept, "type", std::string ());
            bool global = jsonGetOrDefault (ept, "global", false);
            Endpoint &epObj = (global) ? registerGlobalEndpoint (eptName, type) : registerEndpoint (eptName, type);
            

            // retrieve the known paths
            if (ept.isMember ("knownDestinations"))
            {
                auto kp = ept["knownDestinations"];
                if (kp.isString ())
                {
                    registerKnownCommunicationPath (epObj, kp.asString ());
                }
                else if (kp.isArray ())
                {
                    for (const auto &path : kp)
                    {
                        registerKnownCommunicationPath (epObj, path.asString ());
                    }
                }
            }
            // endpoints can subscribe to publications
            if (ept.isMember ("subscriptions"))
            {
                auto subs = ept["subscriptions"];
                if (subs.isString ())
                {
                    subscribe (epObj, subs.asString ());
                }
                else if (subs.isArray ())
                {
                    for (const auto &sub : subs)
                    {
                        subscribe (epObj, sub.asString ());
                    }
                }
            }
            auto defTarget = (ept.isMember ("destination")) ? ept["destination"].asString () : std::string ();
            if (!defTarget.empty ())
            {
                epObj.setTargetDestination (defTarget);
            }

            auto info = jsonGetOrDefault (ept, "info", std::string ());
            if(!info.empty()){
                coreObject->setInterfaceInfo(epObj.getHandle(), info);
            }
        }
    }
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
            Endpoint &epObj = (global) ? registerGlobalEndpoint (key, type) : registerEndpoint (key, type);
            
            // retrieve the known paths
            auto kp = ept.find("knownDestinations");
            if (kp!=nullptr)
            {
                if (kp->is<toml::Array> ())
                {
                    for (const auto &path : kp->as<toml::Array>())
                    {
                        registerKnownCommunicationPath (epObj, path.as<std::string> ());
                    }
                    
                }
                else if (kp->is<std::string>())
                {
                    registerKnownCommunicationPath (epObj, kp->as<std::string> ());
                }
            }
            auto subs = ept.find ("subscriptions");
            // endpoints can subscribe to publications
            if (subs!=nullptr)
            {
                if (subs->is<std::string> ())
                {
                    subscribe (epObj, subs->as<std::string> ());
                }
                else if (subs->is<toml::Array> ())
                {
                    for (const auto &sub : subs->as<toml::Array>())
                    {
                        subscribe (epObj, sub.as<std::string> ());
                    }
                }
            }
            auto defTarget = tomlGetOrDefault (ept, "destination", std::string ());
			if (!defTarget.empty())
			{
                epObj.setTargetDestination (defTarget);
			}

            auto info = tomlGetOrDefault (ept, "info", std::string ());
            if(!info.empty()){
                coreObject->setInterfaceInfo(epObj.getHandle(), info);
            }
        }
    }
   
}

void MessageFederate::subscribe (const Endpoint &ept, const std::string &key)
{
        mfManager->subscribe (ept, key);
        return;
}

void MessageFederate::registerKnownCommunicationPath (const Endpoint &localEndpoint,
                                                      const std::string &remoteEndpoint)
{
        mfManager->registerKnownCommunicationPath (localEndpoint, remoteEndpoint);
        return;
}

bool MessageFederate::hasMessage () const
{
    if (state >= op_states::initialization)
    {
        return mfManager->hasMessage ();
    }
    return false;
}

bool MessageFederate::hasMessage (const Endpoint &ept) const
{
    if (state >= op_states::initialization)
    {
        return mfManager->hasMessage (ept);
    }
    return false;
}

/**
 * Returns the number of pending receives for the specified destination endpoint.
 */
uint64_t MessageFederate::pendingMessages (const Endpoint &ept) const
{
    if (state >= op_states::initialization)
    {
        return mfManager->pendingMessages (ept);
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
    if (state >= op_states::initialization)
    {
        return mfManager->pendingMessages ();
    }
    return 0;
}

std::unique_ptr<Message> MessageFederate::getMessage ()
{
    if (state >= op_states::initialization)
    {
        return mfManager->getMessage ();
    }
    return nullptr;
}

std::unique_ptr<Message> MessageFederate::getMessage (const Endpoint &ept)
{
    if (state >= op_states::initialization)
    {
        return mfManager->getMessage (ept);
    }
    return nullptr;
}

void MessageFederate::sendMessage (const Endpoint &source, const std::string &dest, const data_view &data)
{
        mfManager->sendMessage (source, dest, data);
}

void MessageFederate::sendMessage (const Endpoint &source,
                                   const std::string &dest,
                                   const data_view &data,
                                   Time sendTime)
{
        mfManager->sendMessage (source, dest, data, sendTime);
}

void MessageFederate::sendMessage (const Endpoint &source, std::unique_ptr<Message> message)
{
        mfManager->sendMessage (source, std::move (message));
}

void MessageFederate::sendMessage (const Endpoint &source, const Message &message)
{
    
    mfManager->sendMessage (source, std::make_unique<Message> (message));
}

Endpoint &MessageFederate::getEndpoint (const std::string &eptName) const
{
    auto &id = mfManager->getEndpoint (eptName);
    if (!id.isValid())
    {
        return mfManager->getEndpoint (getName () + separator_ + eptName);
    }
    return id;
}

Endpoint &MessageFederate::getEndpoint(int index) const
{ return mfManager->getEndpoint (index); }

const std::string &MessageFederate::getEndpointName (const Endpoint &ept) const { return ept.getName (); }

const std::string &MessageFederate::getEndpointType (const Endpoint &ept) const { return mfManager->getEndpointType (ept); }

void MessageFederate::setMessageNotificationCallback (const std::function<void(Endpoint &ept, Time)> &func)
{
    mfManager->setEndpointNotificationCallback (func);
}
void MessageFederate::setMessageNotificationCallback (const Endpoint &ept,
                                                const std::function<void(Endpoint &ept, Time)> &func)
{
    mfManager->setEndpointNotificationCallback (ept, func);
}

/** get a count of the number endpoints registered*/
int MessageFederate::getEndpointCount () const { return mfManager->getEndpointCount (); }


void MessageFederate::setEndpointOption(const Endpoint &ept, int32_t option, bool option_value)
{
	mfManager->setEndpointOption(ept, option, option_value);
}

void MessageFederate::addSourceFilter(const Endpoint &ept, const std::string &filterName)
{
    mfManager->addSourceFilter (ept, filterName);
}

void MessageFederate::addDestinationFilter(const Endpoint &ept, const std::string &filterName)
{
    mfManager->addDestinationFilter (ept, filterName);
}

}  // namespace helics
