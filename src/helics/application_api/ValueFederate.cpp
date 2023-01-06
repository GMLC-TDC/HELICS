/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "ValueFederate.hpp"

#include "../common/addTargets.hpp"
#include "../common/configFileHelpers.hpp"
#include "../core/Core.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helics_definitions.hpp"
#include "Inputs.hpp"
#include "Publications.hpp"
#include "ValueFederateManager.hpp"
#include "helicsTypes.hpp"

#include <deque>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics {
/**constructor taking a core engine and federate info structure
 */
ValueFederate::ValueFederate(std::string_view fedName, const FederateInfo& fi):
    Federate(fedName, fi)
{
    // the core object get instantiated in the Federate constructor
    vfManager = std::make_unique<ValueFederateManager>(coreObject.get(),
                                                       this,
                                                       getID(),
                                                       singleThreadFederate);
    vfManager->useJsonSerialization = fi.useJsonSerialization;
}
ValueFederate::ValueFederate(std::string_view fedName,
                             const std::shared_ptr<Core>& core,
                             const FederateInfo& fi):
    Federate(fedName, core, fi)
{
    vfManager = std::make_unique<ValueFederateManager>(coreObject.get(),
                                                       this,
                                                       getID(),
                                                       singleThreadFederate);
    vfManager->useJsonSerialization = fi.useJsonSerialization;
}

ValueFederate::ValueFederate(std::string_view fedName, CoreApp& core, const FederateInfo& fi):
    Federate(fedName, core, fi)
{
    vfManager = std::make_unique<ValueFederateManager>(coreObject.get(),
                                                       this,
                                                       getID(),
                                                       singleThreadFederate);
    vfManager->useJsonSerialization = fi.useJsonSerialization;
}

ValueFederate::ValueFederate(std::string_view fedName, const std::string& configString):
    Federate(fedName, loadFederateInfo(configString))
{
    vfManager = std::make_unique<ValueFederateManager>(coreObject.get(),
                                                       this,
                                                       getID(),
                                                       singleThreadFederate);
    vfManager->useJsonSerialization = useJsonSerialization;
    if (looksLikeFile(configString)) {
        ValueFederate::registerInterfaces(configString);
    }
}

ValueFederate::ValueFederate(const std::string& configString):
    ValueFederate(std::string{}, configString)
{
}

ValueFederate::ValueFederate(const char* configString):
    ValueFederate(std::string{}, std::string{configString})
{
}

ValueFederate::ValueFederate() = default;

ValueFederate::ValueFederate(bool /*res*/)
{
    vfManager = std::make_unique<ValueFederateManager>(coreObject.get(),
                                                       this,
                                                       getID(),
                                                       singleThreadFederate);
    vfManager->useJsonSerialization = useJsonSerialization;
}

ValueFederate::ValueFederate(ValueFederate&&) noexcept = default;

ValueFederate::~ValueFederate() = default;

void ValueFederate::disconnect()
{
    Federate::disconnect();
    vfManager->disconnect();
}

ValueFederate& ValueFederate::operator=(ValueFederate&& fed) noexcept
{
    vfManager = std::move(fed.vfManager);
    if (getID() != fed.getID()) {  // the id won't be moved, as it is copied so use it as a test if
                                   // it has moved already
        Federate::operator=(std::move(fed));
    }
    return *this;
}

Publication& ValueFederate::registerPublication(std::string_view name,
                                                std::string_view type,
                                                std::string_view units)
{
    return vfManager->registerPublication(localNameGenerator(name), type, units);
}

Publication& ValueFederate::registerGlobalPublication(std::string_view name,
                                                      std::string_view type,
                                                      std::string_view units)
{
    return vfManager->registerPublication(name, type, units);
}

Input& ValueFederate::registerInput(std::string_view name,
                                    std::string_view type,
                                    std::string_view units)
{
    return vfManager->registerInput(localNameGenerator(name), type, units);
}

Input& ValueFederate::registerGlobalInput(std::string_view name,
                                          std::string_view type,
                                          std::string_view units)
{
    return vfManager->registerInput(name, type, units);
}

Input& ValueFederate::registerSubscription(std::string_view target, std::string_view units)
{
    auto& inp = vfManager->registerInput(std::string{}, std::string{}, units);
    inp.addTarget(target);
    return inp;
}

void ValueFederate::addTarget(const Publication& pub, std::string_view target)
{
    vfManager->addTarget(pub, target);
}

void ValueFederate::addTarget(const Input& inp, std::string_view target)
{
    vfManager->addTarget(inp, target);
}

void ValueFederate::addAlias(const Input& inp, std::string_view shortcutName)
{
    vfManager->addAlias(inp, shortcutName);
}
void ValueFederate::removeTarget(const Publication& pub, std::string_view target)
{
    vfManager->removeTarget(pub, target);
}

void ValueFederate::removeTarget(const Input& inp, std::string_view target)
{
    vfManager->removeTarget(inp, target);
}

void ValueFederate::setFlagOption(int flag, bool flagValue)
{
    if (flag == HELICS_FLAG_USE_JSON_SERIALIZATION) {
        useJsonSerialization = flagValue;
        vfManager->useJsonSerialization = flagValue;
    } else {
        Federate::setFlagOption(flag, flagValue);
    }
}

void ValueFederate::addAlias(const Publication& pub, std::string_view shortcutName)
{
    vfManager->addAlias(pub, shortcutName);
}

void ValueFederate::setDefaultValue(const Input& inp, data_view block)  // NOLINT
{
    vfManager->setDefaultValue(inp, block);
}

void ValueFederate::registerInterfaces(const std::string& configString)
{
    registerValueInterfaces(configString);
    Federate::registerInterfaces(configString);
}

void ValueFederate::registerValueInterfaces(const std::string& configString)
{
    if (fileops::hasTomlExtension(configString)) {
        registerValueInterfacesToml(configString);
    } else {
        registerValueInterfacesJson(configString);
    }
}
static constexpr std::string_view emptyStr;

template<class Inp, class Obj>
static void loadOptions(ValueFederate* fed, const Inp& data, Obj& objUpdate)
{
    using fileops::getOrDefault;

    addTargets(data, "flags", [&objUpdate, fed](const std::string& target) {
        auto oindex = getOptionIndex((target.front() != '-') ? target : target.substr(1));
        int val = (target.front() != '-') ? 1 : 0;
        if (oindex == HELICS_INVALID_OPTION_INDEX) {
            fed->logWarningMessage(target + " is not a valid flag");
            return;
        }
        objUpdate.setOption(oindex, val);
    });
    processOptions(
        data,
        [](const std::string& option) { return getOptionIndex(option); },
        [](const std::string& value) { return getOptionValue(value); },
        [&objUpdate](int32_t option, int32_t value) { objUpdate.setOption(option, value); });

    fileops::callIfMember(data, "alias", [&objUpdate, fed](std::string_view val) {
        fed->addAlias(objUpdate, val);
    });

    auto tol = getOrDefault(data, "tolerance", -1.0);
    if (tol > 0.0) {
        objUpdate.setMinimumChange(tol);
    }
    auto info = getOrDefault(data, "info", emptyStr);
    if (!info.empty()) {
        objUpdate.setInfo(info);
    }
    loadTags(data, [&objUpdate](std::string_view tagname, std::string_view tagvalue) {
        objUpdate.setTag(tagname, tagvalue);
    });
    addTargets(data, "targets", [&objUpdate](std::string_view target) {
        objUpdate.addTarget(target);
    });
}

void ValueFederate::registerValueInterfacesJson(const std::string& jsonString)
{
    auto doc = fileops::loadJson(jsonString);
    bool defaultGlobal = false;
    fileops::replaceIfMember(doc, "defaultglobal", defaultGlobal);
    if (doc.isMember("publications")) {
        auto pubs = doc["publications"];
        for (const auto& pub : pubs) {
            auto name = fileops::getName(pub);

            Publication* pubAct = &vfManager->getPublication(name);
            if (!pubAct->isValid()) {
                auto type = fileops::getOrDefault(pub, "type", emptyStr);
                auto units = fileops::getOrDefault(pub, "unit", emptyStr);
                fileops::replaceIfMember(pub, "units", units);
                bool global = fileops::getOrDefault(pub, "global", defaultGlobal);
                if (global) {
                    pubAct = &registerGlobalPublication(name, type, units);
                } else {
                    pubAct = &registerPublication(name, type, units);
                }
            }

            loadOptions(this, pub, *pubAct);
        }
    }
    if (doc.isMember("subscriptions")) {
        auto& subs = doc["subscriptions"];
        for (const auto& sub : subs) {
            bool skipNameTarget{false};
            auto name = fileops::getName(sub);
            if (name.empty()) {
                fileops::replaceIfMember(sub, "target", name);
                skipNameTarget = true;
            }
            auto* subAct = &vfManager->getInputByTarget(name);
            if (!subAct->isValid()) {
                auto type = fileops::getOrDefault(sub, "type", emptyStr);
                auto units = fileops::getOrDefault(sub, "unit", emptyStr);
                fileops::replaceIfMember(sub, "units", units);
                subAct = &registerInput(emptyStr, type, units);
                if (!skipNameTarget) {
                    // this check is to prevent some warnings since targets get added later
                    subAct->addTarget(name);
                }
            }
            auto defStr = fileops::getOrDefault(sub, "default", emptyStr);
            if (!defStr.empty()) {
                subAct->setDefault(defStr);
            }
            loadOptions(this, sub, *subAct);
        }
    }
    if (doc.isMember("inputs")) {
        auto ipts = doc["inputs"];
        for (const auto& ipt : ipts) {
            auto name = fileops::getName(ipt);

            Input* inp = &vfManager->getInput(name);
            if (!inp->isValid()) {
                auto type = fileops::getOrDefault(ipt, "type", emptyStr);
                auto units = fileops::getOrDefault(ipt, "unit", emptyStr);
                fileops::replaceIfMember(ipt, "units", units);
                bool global = fileops::getOrDefault(ipt, "global", defaultGlobal);
                if (global) {
                    inp = &registerGlobalInput(name, type, units);
                } else {
                    inp = &registerInput(name, type, units);
                }
            }
            auto defStr = fileops::getOrDefault(ipt, "default", emptyStr);
            if (!defStr.empty()) {
                inp->setDefault(defStr);
            }
            loadOptions(this, ipt, *inp);
        }
    }
}

void ValueFederate::registerValueInterfacesToml(const std::string& tomlString)
{
    using fileops::getOrDefault;
    using fileops::isMember;
    using fileops::replaceIfMember;

    toml::value doc;
    try {
        doc = fileops::loadToml(tomlString);
    }
    catch (const std::invalid_argument& ia) {
        throw(helics::InvalidParameter(ia.what()));
    }
    bool defaultGlobal = false;
    replaceIfMember(doc, "defaultglobal", defaultGlobal);

    if (isMember(doc, "publications")) {
        auto& pubs = toml::find(doc, "publications");
        if (!pubs.is_array()) {
            throw(helics::InvalidParameter("publications section in toml file must be an array"));
        }
        auto& pubArray = pubs.as_array();
        for (const auto& pub : pubArray) {
            auto name = fileops::getName(pub);

            Publication* pubObj = &vfManager->getPublication(name);
            if (!pubObj->isValid()) {
                auto type = getOrDefault(pub, "type", emptyStr);
                auto units = getOrDefault(pub, "unit", emptyStr);
                replaceIfMember(pub, "units", units);
                bool global = getOrDefault(pub, "global", defaultGlobal);
                if (global) {
                    pubObj = &registerGlobalPublication(name, type, units);
                } else {
                    pubObj = &registerPublication(name, type, units);
                }
            }
            loadOptions(this, pub, *pubObj);
        }
    }
    if (isMember(doc, "subscriptions")) {
        auto& subs = toml::find(doc, "subscriptions");
        if (!subs.is_array()) {
            // this line is tested in the publications section so not really necessary to check
            // again since it is an expensive test
            throw(helics::InvalidParameter(
                "subscriptions section in toml file must be an array"));  // LCOV_EXCL_LINE
        }
        auto& subArray = subs.as_array();
        for (const auto& sub : subArray) {
            auto name = fileops::getName(sub);
            bool skipNameTarget{false};
            if (name.empty()) {
                fileops::replaceIfMember(sub, "target", name);
                skipNameTarget = true;
            }
            Input* id = &vfManager->getInputByTarget(name);
            if (!id->isValid()) {
                auto type = getOrDefault(sub, "type", emptyStr);
                auto units = getOrDefault(sub, "unit", emptyStr);
                replaceIfMember(sub, "units", units);

                id = &registerInput(emptyStr, type, units);
                if (!skipNameTarget) {
                    // this check is to prevent some warnings since targets get added later
                    id->addTarget(name);
                }
            }
            auto defStr = fileops::getOrDefault(sub, "default", emptyStr);
            if (!defStr.empty()) {
                id->setDefault(defStr);
            }
            loadOptions(this, sub, *id);
        }
    }
    if (isMember(doc, "inputs")) {
        auto ipts = toml::find(doc, "inputs");
        if (!ipts.is_array()) {
            throw(helics::InvalidParameter(
                "inputs section in toml file must be an array"));  // LCOV_EXCL_LINE
        }
        auto& iptArray = ipts.as_array();
        for (const auto& ipt : iptArray) {
            auto name = fileops::getName(ipt);

            Input* id = &vfManager->getInput(name);
            if (!id->isValid()) {
                auto type = getOrDefault(ipt, "type", emptyStr);
                auto units = getOrDefault(ipt, "unit", emptyStr);
                replaceIfMember(ipt, "units", units);
                bool global = getOrDefault(ipt, "global", defaultGlobal);
                if (global) {
                    id = &registerGlobalInput(name, type, units);
                } else {
                    id = &registerInput(name, type, units);
                }
            }
            auto defStr = fileops::getOrDefault(ipt, "default", emptyStr);
            if (!defStr.empty()) {
                id->setDefault(defStr);
            }
            loadOptions(this, ipt, *id);
        }
    }
}

data_view ValueFederate::getBytes(const Input& inp)
{
    return vfManager->getValue(inp);
}

bool ValueFederate::forceCoreUpdate(Input& inp)
{
    return vfManager->getUpdateFromCore(inp);
}

void ValueFederate::publishBytes(const Publication& pub, data_view block)  // NOLINT
{
    if ((currentMode == Modes::EXECUTING) || (currentMode == Modes::INITIALIZING)) {
        vfManager->publish(pub, block);
    } else {
        throw(InvalidFunctionCall(
            "publications not allowed outside of execution and initialization state"));
    }
}

using dvalue = std::variant<double, std::string>;

static void generateData(std::vector<std::pair<std::string, dvalue>>& vpairs,
                         const std::string& prefix,
                         char separator,
                         Json::Value val)
{
    auto mn = val.getMemberNames();
    for (auto& name : mn) {
        auto& so = val[name];
        if (so.isObject()) {
            generateData(vpairs, prefix + name + separator, separator, so);
        } else {
            if (so.isDouble()) {
                vpairs.emplace_back(prefix + name, so.asDouble());
            } else {
                vpairs.emplace_back(prefix + name, so.asString());
            }
        }
    }
}

void ValueFederate::registerFromPublicationJSON(const std::string& jsonString)
{
    auto jv = [&]() {
        try {
            return fileops::loadJson(jsonString);
        }
        catch (const std::invalid_argument&) {
            throw(helics::InvalidParameter("unable to load file or string"));
        }
    }();

    std::vector<std::pair<std::string, dvalue>> vpairs;
    generateData(vpairs, "", nameSegmentSeparator, jv);

    for (auto& vp : vpairs) {
        try {
            if (vp.second.index() == 0) {
                registerPublication<double>(vp.first);
            } else {
                registerPublication<std::string>(vp.first);
            }
        }
        catch (const helics::RegistrationFailure&) {
            continue;
        }
    }
}

void ValueFederate::publishJSON(const std::string& jsonString)
{
    auto jv = [&]() {
        try {
            return fileops::loadJson(jsonString);
        }
        catch (const std::invalid_argument&) {
            throw(helics::InvalidParameter("unable to load file or string"));
        }
    }();
    std::vector<std::pair<std::string, dvalue>> vpairs;
    generateData(vpairs, "", nameSegmentSeparator, jv);

    for (auto& vp : vpairs) {
        auto& pub = getPublication(vp.first);
        if (pub.isValid()) {
            if (vp.second.index() == 0) {
                pub.publish(std::get<double>(vp.second));
            } else {
                pub.publish(std::get<std::string>(vp.second));
            }
        }
    }
}

bool ValueFederate::isUpdated(const Input& inp) const
{
    return vfManager->hasUpdate(inp);
}

Time ValueFederate::getLastUpdateTime(const Input& inp) const
{
    return vfManager->getLastUpdateTime(inp);
}

void ValueFederate::updateTime(Time newTime, Time oldTime)
{
    vfManager->updateTime(newTime, oldTime);
}

void ValueFederate::startupToInitializeStateTransition()
{
    vfManager->startupToInitializeStateTransition();
}
void ValueFederate::initializeToExecuteStateTransition(iteration_time result)
{
    vfManager->initializeToExecuteStateTransition(result);
}

std::string ValueFederate::localQuery(std::string_view queryStr) const
{
    return vfManager->localQuery(queryStr);
}

std::vector<int> ValueFederate::queryUpdates()
{
    return vfManager->queryUpdates();
}

const std::string& ValueFederate::getTarget(const Input& inp) const
{
    return vfManager->getTarget(inp);
}

const Input& ValueFederate::getInput(std::string_view name) const
{
    auto& inp = vfManager->getInput(name);
    if (!inp.isValid()) {
        return vfManager->getInput(localNameGenerator(name));
    }
    return inp;
}

Input& ValueFederate::getInput(std::string_view name)
{
    auto& inp = vfManager->getInput(name);
    if (!inp.isValid()) {
        return vfManager->getInput(localNameGenerator(name));
    }
    return inp;
}

const Input& ValueFederate::getInput(int index) const
{
    return vfManager->getInput(index);
}

Input& ValueFederate::getInput(int index)
{
    return vfManager->getInput(index);
}

const Input& ValueFederate::getInput(std::string_view name, int index1) const
{
    return vfManager->getInput(std::string(name) + '_' + std::to_string(index1));
}

const Input& ValueFederate::getInput(std::string_view name, int index1, int index2) const
{
    return vfManager->getInput(std::string(name) + '_' + std::to_string(index1) + '_' +
                               std::to_string(index2));
}

const Input& ValueFederate::getInputByTarget(std::string_view target) const
{
    return vfManager->getInputByTarget(target);
}

Input& ValueFederate::getInputByTarget(std::string_view target)
{
    return vfManager->getInputByTarget(target);
}

Publication& ValueFederate::getPublication(std::string_view name)
{
    auto& pub = vfManager->getPublication(name);
    if (!pub.isValid()) {
        return vfManager->getPublication(localNameGenerator(name));
    }
    return pub;
}

const Publication& ValueFederate::getPublication(std::string_view name) const
{
    auto& pub = vfManager->getPublication(name);
    if (!pub.isValid()) {
        return vfManager->getPublication(localNameGenerator(name));
    }
    return pub;
}

Publication& ValueFederate::getPublication(int index)
{
    return vfManager->getPublication(index);
}

const Publication& ValueFederate::getPublication(int index) const
{
    return vfManager->getPublication(index);
}

const Publication& ValueFederate::getPublication(std::string_view name, int index1) const
{
    return vfManager->getPublication(std::string(name) + '_' + std::to_string(index1));
}

const Publication&
    ValueFederate::getPublication(std::string_view name, int index1, int index2) const
{
    return vfManager->getPublication(std::string(name) + '_' + std::to_string(index1) + '_' +
                                     std::to_string(index2));
}

void ValueFederate::setInputNotificationCallback(std::function<void(Input&, Time)> callback)
{
    vfManager->setInputNotificationCallback(std::move(callback));
}

void ValueFederate::setInputNotificationCallback(Input& inp,
                                                 std::function<void(Input&, Time)> callback)
{
    vfManager->setInputNotificationCallback(inp, std::move(callback));
}

int ValueFederate::getPublicationCount() const
{
    return vfManager->getPublicationCount();
}

int ValueFederate::getInputCount() const
{
    return vfManager->getInputCount();
}

void ValueFederate::clearUpdates()
{
    vfManager->clearUpdates();
}

void ValueFederate::clearUpdate(const Input& inp)
{
    vfManager->clearUpdate(inp);
}
}  // namespace helics
