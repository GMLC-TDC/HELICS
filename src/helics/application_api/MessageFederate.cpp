/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "MessageFederate.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../common/TomlProcessingFunctions.hpp"
#include "../common/addTargets.hpp"
#include "../core/Core.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helics_definitions.hpp"
#include "Endpoints.hpp"
#include "MessageFederateManager.hpp"

#include <utility>

namespace helics {
MessageFederate::MessageFederate(const std::string& fedName, const FederateInfo& fi):
    Federate(fedName, fi)
{
    mfManager = std::make_unique<MessageFederateManager>(coreObject.get(), this, getID());
}
MessageFederate::MessageFederate(const std::string& fedName,
                                 const std::shared_ptr<Core>& core,
                                 const FederateInfo& fi):
    Federate(fedName, core, fi)
{
    mfManager = std::make_unique<MessageFederateManager>(coreObject.get(), this, getID());
}

MessageFederate::MessageFederate(const std::string& fedName, CoreApp& core, const FederateInfo& fi):
    Federate(fedName, core, fi)
{
    mfManager = std::make_unique<MessageFederateManager>(coreObject.get(), this, getID());
}

MessageFederate::MessageFederate(const std::string& fedName, const std::string& configString):
    Federate(fedName, loadFederateInfo(configString))
{
    mfManager = std::make_unique<MessageFederateManager>(coreObject.get(), this, getID());
    if (looksLikeFile(configString)) {
        MessageFederate::registerInterfaces(configString);
    }
}

MessageFederate::MessageFederate(const std::string& configString):
    MessageFederate(std::string{}, configString)
{
}

MessageFederate::MessageFederate(const char* configString):
    MessageFederate(std::string{}, std::string{configString})
{
}

MessageFederate::MessageFederate()
{
    // default constructor
}

MessageFederate::MessageFederate(bool /*unused*/)
{  // this constructor should only be called by child class that has already constructed the
   // underlying federate in
    // a virtual inheritance
    mfManager = std::make_unique<MessageFederateManager>(coreObject.get(), this, getID());
}
MessageFederate::MessageFederate(MessageFederate&&) noexcept = default;

MessageFederate& MessageFederate::operator=(MessageFederate&& mFed) noexcept
{
    mfManager = std::move(mFed.mfManager);
    if (getID() != mFed.getID()) {  // the id won't be moved, as it is copied so use it as a test if
                                    // it has moved already
        Federate::operator=(std::move(mFed));
    }
    return *this;
}

MessageFederate::~MessageFederate() = default;

void MessageFederate::disconnect()
{
    Federate::disconnect();
    mfManager->disconnect();
}

void MessageFederate::updateTime(Time newTime, Time oldTime)
{
    mfManager->updateTime(newTime, oldTime);
}

void MessageFederate::startupToInitializeStateTransition()
{
    mfManager->startupToInitializeStateTransition();
}
void MessageFederate::initializeToExecuteStateTransition(iteration_result result)
{
    mfManager->initializeToExecuteStateTransition(result);
}

std::string MessageFederate::localQuery(const std::string& queryStr) const
{
    return mfManager->localQuery(queryStr);
}

Endpoint& MessageFederate::registerEndpoint(const std::string& eptName, const std::string& type)
{
    return mfManager->registerEndpoint((!eptName.empty()) ?
                                           (getName() + nameSegmentSeparator + eptName) :
                                           eptName,
                                       type);
}

Endpoint& MessageFederate::registerGlobalEndpoint(const std::string& eptName,
                                                  const std::string& type)
{
    return mfManager->registerEndpoint(eptName, type);
}

void MessageFederate::registerInterfaces(const std::string& configString)
{
    registerMessageInterfaces(configString);
    Federate::registerFilterInterfaces(configString);
}

void MessageFederate::registerMessageInterfaces(const std::string& configString)
{
    if (hasTomlExtension(configString)) {
        registerMessageInterfacesToml(configString);
    } else {
        registerMessageInterfacesJson(configString);
    }
}

static const std::string emptyStr;
template<class Inp>
static void loadOptions(MessageFederate* fed, const Inp& data, Endpoint& ept)
{
    addTargets(data, "flags", [&ept](const std::string& target) {
        if (target.front() != '-') {
            ept.setOption(getOptionIndex(target), true);
        } else {
            ept.setOption(getOptionIndex(target.substr(2)), false);
        }
    });
    processOptions(
        data,
        [](const std::string& option) { return getOptionIndex(option); },
        [](const std::string& value) { return getOptionValue(value); },
        [&ept](int32_t option, int32_t value) { ept.setOption(option, value); });

    auto info = getOrDefault(data, "info", emptyStr);
    if (!info.empty()) {
        fed->setInfo(ept.getHandle(), info);
    }
    addTargets(data, "knownDestinations", [&ept, fed](const std::string& dest) {
        fed->registerKnownCommunicationPath(ept, dest);
    });
    addTargets(data, "subscriptions", [&ept, fed](const std::string& sub) {
        fed->subscribe(ept, sub);
    });
    addTargets(data, "filters", [&ept](const std::string& filt) { ept.addSourceFilter(filt); });
    addTargets(data, "sourceFilters", [&ept](const std::string& filt) {
        ept.addSourceFilter(filt);
    });
    addTargets(data, "destFilters", [&ept](const std::string& filt) {
        ept.addDestinationFilter(filt);
    });

    auto defTarget = getOrDefault(data, "target", emptyStr);
    replaceIfMember(data, "destination", defTarget);
    if (!defTarget.empty()) {
        ept.setDefaultDestination(defTarget);
    }
}

void MessageFederate::registerMessageInterfacesJson(const std::string& jsonString)
{
    auto doc = loadJson(jsonString);
    bool defaultGlobal = false;
    replaceIfMember(doc, "defaultglobal", defaultGlobal);
    if (doc.isMember("endpoints")) {
        for (const auto& ept : doc["endpoints"]) {
            auto eptName = getKey(ept);
            auto type = getOrDefault(ept, "type", emptyStr);
            bool global = getOrDefault(ept, "global", defaultGlobal);
            Endpoint& epObj =
                (global) ? registerGlobalEndpoint(eptName, type) : registerEndpoint(eptName, type);

            loadOptions(this, ept, epObj);
        }
    }
}

void MessageFederate::registerMessageInterfacesToml(const std::string& tomlString)
{
    toml::value doc;
    try {
        doc = loadToml(tomlString);
    }
    catch (const std::invalid_argument& ia) {
        throw(helics::InvalidParameter(ia.what()));
    }
    bool defaultGlobal = false;
    replaceIfMember(doc, "defaultglobal", defaultGlobal);

    if (isMember(doc, "endpoints")) {
        auto epts = toml::find(doc, "endpoints");
        if (!epts.is_array()) {
            throw(helics::InvalidParameter("endpoints section in toml file must be an array"));
        }
        auto& eptArray = epts.as_array();
        for (auto& ept : eptArray) {
            auto key = getKey(ept);
            auto type = getOrDefault(ept, "type", emptyStr);
            bool global = getOrDefault(ept, "global", defaultGlobal);
            Endpoint& epObj =
                (global) ? registerGlobalEndpoint(key, type) : registerEndpoint(key, type);

            loadOptions(this, ept, epObj);
        }
    }
}

void MessageFederate::subscribe(const Endpoint& ept, const std::string& key)
{
    mfManager->subscribe(ept, key);
}

void MessageFederate::registerKnownCommunicationPath(const Endpoint& localEndpoint,
                                                     const std::string& remoteEndpoint)
{
    mfManager->registerKnownCommunicationPath(localEndpoint, remoteEndpoint);
}

bool MessageFederate::hasMessage() const
{
    if (currentMode >= modes::initializing) {
        return mfManager->hasMessage();
    }
    return false;
}

bool MessageFederate::hasMessage(const Endpoint& ept) const
{
    if (currentMode >= modes::initializing) {
        return mfManager->hasMessage(ept);
    }
    return false;
}

uint64_t MessageFederate::pendingMessages(const Endpoint& ept) const
{
    if (currentMode >= modes::initializing) {
        return mfManager->pendingMessages(ept);
    }
    return 0;
}

uint64_t MessageFederate::pendingMessages() const
{
    if (currentMode >= modes::initializing) {
        return mfManager->pendingMessages();
    }
    return 0;
}

std::unique_ptr<Message> MessageFederate::getMessage()
{
    if (currentMode >= modes::initializing) {
        return mfManager->getMessage();
    }
    return nullptr;
}

std::unique_ptr<Message> MessageFederate::getMessage(const Endpoint& ept)
{
    if (currentMode >= modes::initializing) {
        return mfManager->getMessage(ept);
    }
    return nullptr;
}

void MessageFederate::sendMessage(const Endpoint& source,
                                  const std::string& dest,
                                  const data_view& message)
{
    if ((currentMode == modes::executing) || (currentMode == modes::initializing)) {
        mfManager->sendMessage(source, dest, message);
    } else {
        throw(InvalidFunctionCall(
            "messages not allowed outside of execution and initialization mode"));
    }
}

void MessageFederate::sendMessage(const Endpoint& source,
                                  const std::string& dest,
                                  const data_view& message,
                                  Time sendTime)
{
    if ((currentMode == modes::executing) || (currentMode == modes::initializing)) {
        mfManager->sendMessage(source, dest, message, sendTime);
    } else {
        throw(InvalidFunctionCall(
            "messages not allowed outside of execution and initialization mode"));
    }
}

void MessageFederate::sendMessage(const Endpoint& source, std::unique_ptr<Message> message)
{
    if ((currentMode == modes::executing) || (currentMode == modes::initializing)) {
        mfManager->sendMessage(source, std::move(message));
    } else {
        throw(InvalidFunctionCall(
            "messages not allowed outside of execution and initialization mode"));
    }
}

void MessageFederate::sendMessage(const Endpoint& source, const Message& message)
{
    if ((currentMode == modes::executing) || (currentMode == modes::initializing)) {
        mfManager->sendMessage(source, std::make_unique<Message>(message));
    } else {
        throw(InvalidFunctionCall(
            "messages not allowed outside of execution and initialization mode"));
    }
}

Endpoint& MessageFederate::getEndpoint(const std::string& eptName) const
{
    auto& id = mfManager->getEndpoint(eptName);
    if (!id.isValid()) {
        return mfManager->getEndpoint(getName() + nameSegmentSeparator + eptName);
    }
    return id;
}

Endpoint& MessageFederate::getEndpoint(int index) const
{
    return mfManager->getEndpoint(index);
}

void MessageFederate::setMessageNotificationCallback(
    const std::function<void(Endpoint& ept, Time)>& callback)
{
    mfManager->setEndpointNotificationCallback(callback);
}
void MessageFederate::setMessageNotificationCallback(
    const Endpoint& ept,
    const std::function<void(Endpoint& ept, Time)>& callback)
{
    mfManager->setEndpointNotificationCallback(ept, callback);
}

/** get a count of the number endpoints registered*/
int MessageFederate::getEndpointCount() const
{
    return mfManager->getEndpointCount();
}

void MessageFederate::addSourceFilter(const Endpoint& ept, const std::string& filterName)
{
    mfManager->addSourceFilter(ept, filterName);
}

void MessageFederate::addDestinationFilter(const Endpoint& ept, const std::string& filterName)
{
    mfManager->addDestinationFilter(ept, filterName);
}

}  // namespace helics
