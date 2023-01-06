/*
Copyright (c) 2017-2023,
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
MessageFederate::MessageFederate(std::string_view fedName, const FederateInfo& fi):
    Federate(fedName, fi)
{
    mfManager = std::make_unique<MessageFederateManager>(coreObject.get(),
                                                         this,
                                                         getID(),
                                                         singleThreadFederate);
}
MessageFederate::MessageFederate(std::string_view fedName,
                                 const std::shared_ptr<Core>& core,
                                 const FederateInfo& fi):
    Federate(fedName, core, fi)
{
    mfManager = std::make_unique<MessageFederateManager>(coreObject.get(),
                                                         this,
                                                         getID(),
                                                         singleThreadFederate);
}

MessageFederate::MessageFederate(std::string_view fedName, CoreApp& core, const FederateInfo& fi):
    Federate(fedName, core, fi)
{
    mfManager = std::make_unique<MessageFederateManager>(coreObject.get(),
                                                         this,
                                                         getID(),
                                                         singleThreadFederate);
}

MessageFederate::MessageFederate(std::string_view fedName, const std::string& configString):
    Federate(fedName, loadFederateInfo(configString))
{
    mfManager = std::make_unique<MessageFederateManager>(coreObject.get(),
                                                         this,
                                                         getID(),
                                                         singleThreadFederate);
    if (looksLikeFile(configString)) {
        MessageFederate::registerInterfaces(configString);
    }
}

MessageFederate::MessageFederate(const std::string& configString):
    MessageFederate(std::string_view{}, configString)
{
}

MessageFederate::MessageFederate(const char* configString):
    MessageFederate(std::string_view{}, std::string{configString})
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
    mfManager = std::make_unique<MessageFederateManager>(coreObject.get(),
                                                         this,
                                                         getID(),
                                                         singleThreadFederate);
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
void MessageFederate::initializeToExecuteStateTransition(iteration_time result)
{
    mfManager->initializeToExecuteStateTransition(result);
}

std::string MessageFederate::localQuery(std::string_view queryStr) const
{
    return mfManager->localQuery(queryStr);
}

Endpoint& MessageFederate::registerEndpoint(std::string_view eptName, std::string_view type)
{
    return mfManager->registerEndpoint(localNameGenerator(eptName), type);
}

Endpoint& MessageFederate::registerTargetedEndpoint(std::string_view eptName, std::string_view type)
{
    return mfManager->registerTargetedEndpoint(localNameGenerator(eptName), type);
}

Endpoint& MessageFederate::registerGlobalEndpoint(std::string_view eptName, std::string_view type)
{
    return mfManager->registerEndpoint(eptName, type);
}

Endpoint& MessageFederate::registerGlobalTargetedEndpoint(std::string_view eptName,
                                                          std::string_view type)
{
    return mfManager->registerTargetedEndpoint(eptName, type);
}

Endpoint& MessageFederate::registerDataSink(std::string_view sinkName)
{
    return mfManager->registerDataSink(sinkName);
}

void MessageFederate::registerInterfaces(const std::string& configString)
{
    registerMessageInterfaces(configString);
    Federate::registerFilterInterfaces(configString);
}

void MessageFederate::registerMessageInterfaces(const std::string& configString)
{
    if (fileops::hasTomlExtension(configString)) {
        registerMessageInterfacesToml(configString);
    } else {
        registerMessageInterfacesJson(configString);
    }
}

// NOLINTNEXTLINE
static constexpr std::string_view emptyStr;

template<class Inp>
static void loadOptions(MessageFederate* fed, const Inp& data, Endpoint& ept)
{
    using fileops::getOrDefault;
    addTargets(data, "flags", [&ept, fed](const std::string& target) {
        auto oindex = getOptionIndex((target.front() != '-') ? target : target.substr(1));
        int val = (target.front() != '-') ? 1 : 0;
        if (oindex == HELICS_INVALID_OPTION_INDEX) {
            fed->logWarningMessage(target + " is not a recognized flag");
            return;
        }
        ept.setOption(oindex, val);
    });
    processOptions(
        data,
        [](const std::string& option) { return getOptionIndex(option); },
        [](const std::string& value) { return getOptionValue(value); },
        [&ept](int32_t option, int32_t value) { ept.setOption(option, value); });

    auto info = getOrDefault(data, "info", emptyStr);
    if (!info.empty()) {
        ept.setInfo(info);
    }
    loadTags(data, [&ept](std::string_view tagname, std::string_view tagvalue) {
        ept.setTag(tagname, tagvalue);
    });
    addTargets(data, "subscriptions", [&ept](std::string_view sub) { ept.subscribe(sub); });
    addTargets(data, "filters", [&ept](std::string_view filt) { ept.addSourceFilter(filt); });
    addTargets(data, "sourceFilters", [&ept](std::string_view filt) { ept.addSourceFilter(filt); });
    addTargets(data, "destFilters", [&ept](std::string_view filt) {
        ept.addDestinationFilter(filt);
    });

    auto defTarget = getOrDefault(data, "target", emptyStr);
    fileops::replaceIfMember(data, "destination", defTarget);
    if (!defTarget.empty()) {
        ept.setDefaultDestination(defTarget);
    }
}

void MessageFederate::registerMessageInterfacesJson(const std::string& jsonString)
{
    auto doc = fileops::loadJson(jsonString);
    bool defaultGlobal = false;
    fileops::replaceIfMember(doc, "defaultglobal", defaultGlobal);
    if (doc.isMember("endpoints")) {
        for (const auto& ept : doc["endpoints"]) {
            auto eptName = fileops::getName(ept);
            auto type = fileops::getOrDefault(ept, "type", emptyStr);
            bool global = fileops::getOrDefault(ept, "global", defaultGlobal);
            Endpoint& epObj =
                (global) ? registerGlobalEndpoint(eptName, type) : registerEndpoint(eptName, type);

            loadOptions(this, ept, epObj);
        }
    }
    if (doc.isMember("datasinks")) {
        for (const auto& ept : doc["datasinks"]) {
            auto eptName = fileops::getName(ept);
            Endpoint& epObj = registerDataSink(eptName);

            loadOptions(this, ept, epObj);
        }
    }
}

void MessageFederate::registerMessageInterfacesToml(const std::string& tomlString)
{
    toml::value doc;
    try {
        doc = fileops::loadToml(tomlString);
    }
    catch (const std::invalid_argument& ia) {
        throw(helics::InvalidParameter(ia.what()));
    }
    bool defaultGlobal = false;
    fileops::replaceIfMember(doc, "defaultglobal", defaultGlobal);

    if (fileops::isMember(doc, "endpoints")) {
        auto& epts = toml::find(doc, "endpoints");
        if (!epts.is_array()) {
            throw(helics::InvalidParameter("endpoints section in toml file must be an array"));
        }
        auto& eptArray = epts.as_array();
        for (auto& ept : eptArray) {
            auto key = fileops::getName(ept);
            auto type = fileops::getOrDefault(ept, "type", emptyStr);
            bool global = fileops::getOrDefault(ept, "global", defaultGlobal);
            Endpoint& epObj =
                (global) ? registerGlobalEndpoint(key, type) : registerEndpoint(key, type);

            loadOptions(this, ept, epObj);
        }
    }
    if (fileops::isMember(doc, "datasinks")) {
        auto& epts = toml::find(doc, "datasinks");
        if (!epts.is_array()) {
            throw(helics::InvalidParameter("datasinks section in toml file must be an array"));
        }
        auto& eptArray = epts.as_array();
        for (auto& ept : eptArray) {
            auto key = fileops::getName(ept);
            Endpoint& epObj = registerDataSink(key);

            loadOptions(this, ept, epObj);
        }
    }
}

void MessageFederate::subscribe(const Endpoint& ept, std::string_view key)
{
    coreObject->addSourceTarget(ept, key);
}

bool MessageFederate::hasMessage() const
{
    if (currentMode >= Modes::INITIALIZING) {
        return mfManager->hasMessage();
    }
    return false;
}

bool MessageFederate::hasMessage(const Endpoint& ept) const
{
    if (currentMode >= Modes::INITIALIZING) {
        return mfManager->hasMessage(ept);
    }
    return false;
}

uint64_t MessageFederate::pendingMessageCount(const Endpoint& ept) const
{
    if (currentMode >= Modes::INITIALIZING) {
        return mfManager->pendingMessageCount(ept);
    }
    return 0;
}

uint64_t MessageFederate::pendingMessageCount() const
{
    if (currentMode >= Modes::INITIALIZING) {
        return mfManager->pendingMessageCount();
    }
    return 0;
}

std::unique_ptr<Message> MessageFederate::getMessage()
{
    if (currentMode >= Modes::INITIALIZING) {
        return mfManager->getMessage();
    }
    return nullptr;
}

std::unique_ptr<Message> MessageFederate::getMessage(const Endpoint& ept)
{
    if (currentMode >= Modes::INITIALIZING) {
        return mfManager->getMessage(ept);
    }
    return nullptr;
}

Endpoint& MessageFederate::getEndpoint(std::string_view eptName) const
{
    auto& id = mfManager->getEndpoint(eptName);
    if (!id.isValid()) {
        return mfManager->getEndpoint(localNameGenerator(eptName));
    }
    return id;
}

Endpoint& MessageFederate::getDataSink(std::string_view sinkName) const
{
    return mfManager->getDataSink(sinkName);
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

}  // namespace helics
