/*
Copyright (c) 2017-2024,
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

#include <memory>
#include <string>
#include <utility>

namespace helics {
MessageFederate::MessageFederate(std::string_view fedName, const FederateInfo& fedInfo):
    Federate(fedName, fedInfo)
{
    loadFederateData();
}
MessageFederate::MessageFederate(std::string_view fedName,
                                 const std::shared_ptr<Core>& core,
                                 const FederateInfo& fedInfo): Federate(fedName, core, fedInfo)
{
    loadFederateData();
}

MessageFederate::MessageFederate(std::string_view fedName,
                                 CoreApp& core,
                                 const FederateInfo& fedInfo): Federate(fedName, core, fedInfo)
{
    loadFederateData();
}

MessageFederate::MessageFederate(std::string_view fedName, const std::string& configString):
    Federate(fedName, loadFederateInfo(configString))
{
    loadFederateData();
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
    loadFederateData();
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

void MessageFederate::loadFederateData()
{
    mfManager = std::make_unique<MessageFederateManager>(coreObject.get(),
                                                         this,
                                                         getID(),
                                                         singleThreadFederate);
    if (!configFile.empty()) {
        MessageFederate::registerMessageInterfaces(configFile);
    }
}
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
    Federate::registerConnectorInterfaces(configString);
}

void MessageFederate::registerMessageInterfaces(const std::string& configString)
{
    auto hint = fileops::getConfigType(configString);
    switch (hint) {
        case fileops::ConfigType::JSON_FILE:
        case fileops::ConfigType::JSON_STRING:
            try {
                registerMessageInterfacesJson(configString);
            }
            catch (const std::invalid_argument& e) {
                throw(helics::InvalidParameter(e.what()));
            }
            break;
        case fileops::ConfigType::TOML_FILE:
        case fileops::ConfigType::TOML_STRING:
            registerMessageInterfacesToml(configString);
            break;
        case fileops::ConfigType::NONE:
        default:
            break;
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
        const int val = (target.front() != '-') ? 1 : 0;
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
    addTargetVariations(data, "source", "inputs", [&ept](std::string_view ipt) {
        ept.subscribe(ipt);
    });
    addTargetVariations(data, "source", "filters", [&ept](std::string_view filt) {
        ept.addSourceFilter(filt);
    });
    addTargetVariations(data, "destination", "filters", [&ept](std::string_view filt) {
        ept.addDestinationFilter(filt);
    });
    addTargetVariations(data, "source", "endpoints", [&ept](std::string_view endpoint) {
        ept.addSourceEndpoint(endpoint);
    });
    addTargetVariations(data, "destination", "endpoints", [&ept](std::string_view endpoint) {
        ept.addDestinationEndpoint(endpoint);
    });
    addTargetVariations(data, "source", "targets", [&ept](std::string_view endpoint) {
        ept.addSourceEndpoint(endpoint);
    });
    addTargetVariations(data, "destination", "targets", [&ept](std::string_view endpoint) {
        ept.addDestinationEndpoint(endpoint);
    });
    addTargets(data, "destFilters", [&ept](std::string_view filt) {
        ept.addDestinationFilter(filt);
    });

    auto defTarget = fileops::getOrDefault(data, "target", emptyStr);
    fileops::replaceIfMember(data, "destination", defTarget);
    if (!defTarget.empty()) {
        ept.setDefaultDestination(defTarget);
    }
}

void MessageFederate::registerMessageInterfacesJson(const std::string& jsonString)
{
    auto doc = fileops::loadJson(jsonString);
    registerMessageInterfacesJsonDetail(doc, false);
}

Endpoint& MessageFederate::registerEndpoint(std::string_view eptName,
                                            std::string_view type,
                                            bool global,
                                            bool targeted)
{
    if (targeted) {
        if (global) {
            return registerGlobalTargetedEndpoint(eptName, type);
        }
        return registerTargetedEndpoint(eptName, type);
    }
    if (global) {
        return registerGlobalEndpoint(eptName, type);
    }
    return registerEndpoint(eptName, type);
}

void MessageFederate::registerMessageInterfacesJsonDetail(const fileops::JsonBuffer& jsonBuff,
                                                          bool defaultGlobal)
{
    const auto& json = jsonBuff.json();
    fileops::replaceIfMember(json, "defaultglobal", defaultGlobal);
    const bool defaultTargeted = fileops::getOrDefault(json, "targeted", false);

    const nlohmann::json& iface = (json.contains("interfaces")) ? json["interfaces"] : json;

    if (iface.contains("endpoints")) {
        for (const auto& ept : iface["endpoints"]) {
            auto eptName = fileops::getName(ept);
            auto type = fileops::getOrDefault(ept, "type", emptyStr);
            const bool global = fileops::getOrDefault(ept, "global", defaultGlobal);
            const bool targeted = fileops::getOrDefault(ept, "targeted", defaultTargeted);
            Endpoint& epObj = registerEndpoint(eptName, type, global, targeted);

            loadOptions(this, ept, epObj);
        }
    }
    if (iface.contains("datasinks")) {
        for (const auto& ept : iface["datasinks"]) {
            auto eptName = fileops::getName(ept);
            Endpoint& epObj = registerDataSink(eptName);

            loadOptions(this, ept, epObj);
        }
    }
    if (json.contains("helics")) {
        registerMessageInterfacesJsonDetail(json["helics"], defaultGlobal);
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
    const bool defaultTargeted = fileops::getOrDefault(doc, "targeted", false);
    if (fileops::isMember(doc, "endpoints")) {
        auto& epts = toml::find(doc, "endpoints");
        if (!epts.is_array()) {
            throw(helics::InvalidParameter("endpoints section in toml file must be an array"));
        }
        auto& eptArray = epts.as_array();
        for (auto& ept : eptArray) {
            auto eptName = fileops::getName(ept);
            auto type = fileops::getOrDefault(ept, "type", emptyStr);
            const bool global = fileops::getOrDefault(ept, "global", defaultGlobal);
            const bool targeted = fileops::getOrDefault(ept, "targeted", defaultTargeted);
            Endpoint& epObj = registerEndpoint(eptName, type, global, targeted);

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
    auto& ept = mfManager->getEndpoint(eptName);
    if (!ept.isValid()) {
        return mfManager->getEndpoint(localNameGenerator(eptName));
    }
    return ept;
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
