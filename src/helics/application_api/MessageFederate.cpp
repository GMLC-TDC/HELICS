/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "MessageFederate.hpp"
#include "../common/JsonProcessingFunctions.hpp"
#include "../common/TomlProcessingFunctions.hpp"
#include "../common/addTargets.hpp"
#include "../core/Core.hpp"
#include "../core/core-exceptions.hpp"
#include "Endpoints.hpp"
#include "MessageFederateManager.hpp"

namespace helics
{
MessageFederate::MessageFederate (const std::string &fedName, const FederateInfo &fi) : Federate (fedName, fi)
{
    mfManager = std::make_unique<MessageFederateManager> (coreObject.get (), this, getID ());
}
MessageFederate::MessageFederate (const std::string &fedName,
                                  const std::shared_ptr<Core> &core,
                                  const FederateInfo &fi)
    : Federate (fedName, core, fi)
{
    mfManager = std::make_unique<MessageFederateManager> (coreObject.get (), this, getID ());
}
MessageFederate::MessageFederate (const std::string &configString)
    : Federate (std::string (), loadFederateInfo (configString))
{
    mfManager = std::make_unique<MessageFederateManager> (coreObject.get (), this, getID ());
    MessageFederate::registerInterfaces (configString);
}

MessageFederate::MessageFederate (const std::string &fedName, const std::string &configString)
    : Federate (fedName, loadFederateInfo (configString))
{
    mfManager = std::make_unique<MessageFederateManager> (coreObject.get (), this, getID ());
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

std::string MessageFederate::localQuery (const std::string &queryStr) const
{
    return mfManager->localQuery (queryStr);
}

Endpoint &MessageFederate::registerEndpoint (const std::string &eptName, const std::string &type)
{
    return mfManager->registerEndpoint ((!eptName.empty ()) ? (getName () + separator_ + eptName) : eptName, type);
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

void MessageFederate::registerMessageInterfaces (const std::string &configString)
{
    if (hasTomlExtension (configString))
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
            auto type = getOrDefault (ept, "type", std::string ());
            bool global = getOrDefault (ept, "global", false);
            Endpoint &epObj = (global) ? registerGlobalEndpoint (eptName, type) : registerEndpoint (eptName, type);

            addTargets (ept, "knownDestinations",
                        [&epObj, this](const std::string &dest) { registerKnownCommunicationPath (epObj, dest); });
            addTargets (ept, "subscriptions", [&epObj, this](const std::string &sub) { subscribe (epObj, sub); });
            addTargets (ept, "filters", [&epObj](const std::string &filt) { epObj.addSourceFilter (filt); });
            addTargets (ept, "sourceFilters", [&epObj](const std::string &filt) { epObj.addSourceFilter (filt); });
            addTargets (ept, "destFilters",
                        [&epObj](const std::string &filt) { epObj.addDestinationFilter (filt); });
            auto defTarget = getOrDefault (ept, "target", std::string ());
            replaceIfMember (ept, "destination", defTarget);
            if (!defTarget.empty ())
            {
                epObj.setTargetDestination (defTarget);
            }

            auto info = getOrDefault (ept, "info", std::string ());
            if (!info.empty ())
            {
                setInfo (epObj.getHandle (), info);
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
        for (auto &ept : eptArray)
        {
            auto key = getKey (ept);
            auto type = getOrDefault (ept, "type", std::string ());
            bool global = getOrDefault (ept, "global", false);
            Endpoint &epObj = (global) ? registerGlobalEndpoint (key, type) : registerEndpoint (key, type);

            addTargets (ept, "knownDestinations",
                        [&epObj, this](const std::string &dest) { registerKnownCommunicationPath (epObj, dest); });
            addTargets (ept, "subscriptions", [&epObj, this](const std::string &sub) { subscribe (epObj, sub); });
            addTargets (ept, "filters", [&epObj](const std::string &filt) { epObj.addSourceFilter (filt); });
            addTargets (ept, "sourceFilters", [&epObj](const std::string &filt) { epObj.addSourceFilter (filt); });
            addTargets (ept, "destFilters",
                        [&epObj](const std::string &filt) { epObj.addDestinationFilter (filt); });

            auto defTarget = getOrDefault (ept, "target", std::string ());
            replaceIfMember (ept, "destination", defTarget);
            if (!defTarget.empty ())
            {
                epObj.setTargetDestination (defTarget);
            }

            auto info = getOrDefault (ept, "info", std::string ());
            if (!info.empty ())
            {
                setInfo (epObj.getHandle (), info);
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
    if (state >= states::initialization)
    {
        return mfManager->hasMessage ();
    }
    return false;
}

bool MessageFederate::hasMessage (const Endpoint &ept) const
{
    if (state >= states::initialization)
    {
        return mfManager->hasMessage (ept);
    }
    return false;
}

uint64_t MessageFederate::pendingMessages (const Endpoint &ept) const
{
    if (state >= states::initialization)
    {
        return mfManager->pendingMessages (ept);
    }
    return 0;
}

uint64_t MessageFederate::pendingMessages () const
{
    if (state >= states::initialization)
    {
        return mfManager->pendingMessages ();
    }
    return 0;
}

std::unique_ptr<Message> MessageFederate::getMessage ()
{
    if (state >= states::initialization)
    {
        return mfManager->getMessage ();
    }
    return nullptr;
}

std::unique_ptr<Message> MessageFederate::getMessage (const Endpoint &ept)
{
    if (state >= states::initialization)
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
    if (!id.isValid ())
    {
        return mfManager->getEndpoint (getName () + separator_ + eptName);
    }
    return id;
}

Endpoint &MessageFederate::getEndpoint (int index) const { return mfManager->getEndpoint (index); }

const std::string &MessageFederate::getEndpointName (const Endpoint &ept) const { return ept.getName (); }

const std::string &MessageFederate::getEndpointType (const Endpoint &ept) const
{
    return mfManager->getEndpointType (ept);
}

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

void MessageFederate::addSourceFilter (const Endpoint &ept, const std::string &filterName)
{
    mfManager->addSourceFilter (ept, filterName);
}

void MessageFederate::addDestinationFilter (const Endpoint &ept, const std::string &filterName)
{
    mfManager->addDestinationFilter (ept, filterName);
}

}  // namespace helics
