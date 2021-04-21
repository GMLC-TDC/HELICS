/*
Copyright (c) 2017-2021,
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
ValueFederate::ValueFederate(const std::string& fedName, const FederateInfo& fi):
    Federate(fedName, fi)
{
    // the core object get instantiated in the Federate constructor
    vfManager = std::make_unique<ValueFederateManager>(coreObject.get(), this, getID());
}
ValueFederate::ValueFederate(const std::string& fedName,
                             const std::shared_ptr<Core>& core,
                             const FederateInfo& fi):
    Federate(fedName, core, fi)
{
    vfManager = std::make_unique<ValueFederateManager>(coreObject.get(), this, getID());
}

ValueFederate::ValueFederate(const std::string& fedName, CoreApp& core, const FederateInfo& fi):
    Federate(fedName, core, fi)
{
    vfManager = std::make_unique<ValueFederateManager>(coreObject.get(), this, getID());
}

ValueFederate::ValueFederate(const std::string& fedName, const std::string& configString):
    Federate(fedName, loadFederateInfo(configString))
{
    vfManager = std::make_unique<ValueFederateManager>(coreObject.get(), this, getID());
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
    vfManager = std::make_unique<ValueFederateManager>(coreObject.get(), this, getID());
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

Publication& ValueFederate::registerPublication(const std::string& key,
                                                const std::string& type,
                                                const std::string& units)
{
    return vfManager->registerPublication(
        (!key.empty()) ? (getName() + nameSegmentSeparator + key) : key, type, units);
}

Publication& ValueFederate::registerGlobalPublication(const std::string& key,
                                                      const std::string& type,
                                                      const std::string& units)
{
    return vfManager->registerPublication(key, type, units);
}

Input& ValueFederate::registerInput(const std::string& key,
                                    const std::string& type,
                                    const std::string& units)
{
    return vfManager->registerInput((!key.empty()) ? (getName() + nameSegmentSeparator + key) : key,
                                    type,
                                    units);
}

Input& ValueFederate::registerGlobalInput(const std::string& key,
                                          const std::string& type,
                                          const std::string& units)
{
    return vfManager->registerInput(key, type, units);
}

Input& ValueFederate::registerSubscription(const std::string& target, const std::string& units)
{
    auto& inp = vfManager->registerInput(std::string{}, std::string{}, units);
    vfManager->addTarget(inp, target);
    return inp;
}

void ValueFederate::addTarget(const Publication& pub, const std::string& target)
{
    vfManager->addTarget(pub, target);
}

void ValueFederate::addTarget(const Input& inp, const std::string& target)
{
    vfManager->addTarget(inp, target);
}

void ValueFederate::addAlias(const Input& inp, const std::string& shortcutName)
{
    vfManager->addAlias(inp, shortcutName);
}
void ValueFederate::removeTarget(const Publication& pub, const std::string& target)
{
    vfManager->removeTarget(pub, target);
}

void ValueFederate::removeTarget(const Input& inp, const std::string& target)
{
    vfManager->removeTarget(inp, target);
}

void ValueFederate::addAlias(const Publication& pub, const std::string& shortcutName)
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
    if (hasTomlExtension(configString)) {
        registerValueInterfacesToml(configString);
    } else {
        registerValueInterfacesJson(configString);
    }
}
static const std::string emptyStr;

template<class Inp, class Obj>
static void loadOptions(ValueFederate* fed, const Inp& data, Obj& objUpdate)
{
    addTargets(data, "flags", [&objUpdate](const std::string& target) {
        if (target.front() != '-') {
            objUpdate.setOption(getOptionIndex(target), 1);
        } else {
            objUpdate.setOption(getOptionIndex(target.substr(2)), 0);
        }
    });
    processOptions(
        data,
        [](const std::string& option) { return getOptionIndex(option); },
        [](const std::string& value) { return getOptionValue(value); },
        [&objUpdate](int32_t option, int32_t value) { objUpdate.setOption(option, value); });

    callIfMember(data, "shortcut", [&objUpdate, fed](const std::string& val) {
        fed->addAlias(objUpdate, val);
    });
    callIfMember(data, "alias", [&objUpdate, fed](const std::string& val) {
        fed->addAlias(objUpdate, val);
    });

    auto tol = getOrDefault(data, "tolerance", -1.0);
    if (tol > 0.0) {
        objUpdate.setMinimumChange(tol);
    }
    auto info = getOrDefault(data, "info", emptyStr);
    if (!info.empty()) {
        fed->setInfo(objUpdate.getHandle(), info);
    }
    addTargets(data, "targets", [&objUpdate](const std::string& target) {
        objUpdate.addTarget(target);
    });
}

void ValueFederate::registerValueInterfacesJson(const std::string& jsonString)
{
    auto doc = loadJson(jsonString);
    bool defaultGlobal = false;
    replaceIfMember(doc, "defaultglobal", defaultGlobal);
    if (doc.isMember("publications")) {
        auto pubs = doc["publications"];
        for (const auto& pub : pubs) {
            auto key = getKey(pub);

            Publication* pubAct = &vfManager->getPublication(key);
            if (!pubAct->isValid()) {
                auto type = getOrDefault(pub, "type", emptyStr);
                auto units = getOrDefault(pub, "unit", emptyStr);
                replaceIfMember(pub, "units", units);
                bool global = getOrDefault(pub, "global", defaultGlobal);
                if (global) {
                    pubAct = &registerGlobalPublication(key, type, units);
                } else {
                    pubAct = &registerPublication(key, type, units);
                }
            }

            loadOptions(this, pub, *pubAct);
        }
    }
    if (doc.isMember("subscriptions")) {
        auto subs = doc["subscriptions"];
        for (const auto& sub : subs) {
            auto key = getKey(sub);
            auto* subAct = &vfManager->getSubscription(key);
            if (!subAct->isValid()) {
                auto type = getOrDefault(sub, "type", emptyStr);
                auto units = getOrDefault(sub, "unit", emptyStr);
                replaceIfMember(sub, "units", units);
                subAct = &registerInput(emptyStr, type, units);
                subAct->addTarget(key);
            }

            loadOptions(this, sub, *subAct);
        }
    }
    if (doc.isMember("inputs")) {
        auto ipts = doc["inputs"];
        for (const auto& ipt : ipts) {
            auto key = getKey(ipt);

            Input* inp = &vfManager->getInput(key);
            if (!inp->isValid()) {
                auto type = getOrDefault(ipt, "type", emptyStr);
                auto units = getOrDefault(ipt, "unit", emptyStr);
                replaceIfMember(ipt, "units", units);
                bool global = getOrDefault(ipt, "global", defaultGlobal);
                if (global) {
                    inp = &registerGlobalInput(key, type, units);
                } else {
                    inp = &registerInput(key, type, units);
                }
            }

            loadOptions(this, ipt, *inp);
        }
    }
}

void ValueFederate::registerValueInterfacesToml(const std::string& tomlString)
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

    if (isMember(doc, "publications")) {
        auto pubs = toml::find(doc, "publications");
        if (!pubs.is_array()) {
            throw(helics::InvalidParameter("publications section in toml file must be an array"));
        }
        auto& pubArray = pubs.as_array();
        for (const auto& pub : pubArray) {
            auto key = getKey(pub);

            Publication* pubObj = &vfManager->getPublication(key);
            if (!pubObj->isValid()) {
                auto type = getOrDefault(pub, "type", emptyStr);
                auto units = getOrDefault(pub, "unit", emptyStr);
                replaceIfMember(pub, "units", units);
                bool global = getOrDefault(pub, "global", defaultGlobal);
                if (global) {
                    pubObj = &registerGlobalPublication(key, type, units);
                } else {
                    pubObj = &registerPublication(key, type, units);
                }
            }
            loadOptions(this, pub, *pubObj);
        }
    }
    if (isMember(doc, "subscriptions")) {
        auto subs = toml::find(doc, "subscriptions");
        if (!subs.is_array()) {
            // this line is tested in the publications section so not really necessary to check
            // again since it is an expensive test
            throw(helics::InvalidParameter(
                "subscriptions section in toml file must be an array"));  // LCOV_EXCL_LINE
        }
        auto& subArray = subs.as_array();
        for (const auto& sub : subArray) {
            auto key = getKey(sub);
            Input* id = &vfManager->getSubscription(key);
            if (!id->isValid()) {
                auto type = getOrDefault(sub, "type", emptyStr);
                auto units = getOrDefault(sub, "unit", emptyStr);
                replaceIfMember(sub, "units", units);

                id = &registerInput(emptyStr, type, units);
                id->addTarget(key);
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
            auto key = getKey(ipt);

            Input* id = &vfManager->getInput(key);
            if (!id->isValid()) {
                auto type = getOrDefault(ipt, "type", emptyStr);
                auto units = getOrDefault(ipt, "unit", emptyStr);
                replaceIfMember(ipt, "units", units);
                bool global = getOrDefault(ipt, "global", defaultGlobal);
                if (global) {
                    id = &registerGlobalInput(key, type, units);
                } else {
                    id = &registerInput(key, type, units);
                }
            }

            loadOptions(this, ipt, *id);
        }
    }
}

data_view ValueFederate::getValueRaw(const Input& inp)
{
    return vfManager->getValue(inp);
}

double ValueFederate::getDouble(Input& inp)
{
    return inp.getValue<double>();
}
/** get a string value*/
const std::string& ValueFederate::getString(Input& inp)
{
    return inp.getValueRef<std::string>();
}

void ValueFederate::publishRaw(const Publication& pub, data_view block)  // NOLINT
{
    if ((currentMode == modes::executing) || (currentMode == modes::initializing)) {
        vfManager->publish(pub, block);
    } else {
        throw(InvalidFunctionCall(
            "publications not allowed outside of execution and initialization state"));
    }
}

void ValueFederate::publish(Publication& pub, const std::string& str)
{
    pub.publish(str);
}

void ValueFederate::publish(Publication& pub, double val)
{
    pub.publish(val);
}

using dvalue = mpark::variant<double, std::string>;

static void generateData(std::vector<std::pair<std::string, dvalue>>& vpairs,
                         const std::string& prefix,
                         char separator,
                         Json::Value val)
{
    auto mn = val.getMemberNames();
    for (auto& name : mn) {
        auto so = val[name];
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
            return loadJson(jsonString);
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
            return loadJson(jsonString);
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
                pub.publish(mpark::get<double>(vp.second));
            } else {
                pub.publish(mpark::get<std::string>(vp.second));
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
void ValueFederate::initializeToExecuteStateTransition(iteration_result result)
{
    vfManager->initializeToExecuteStateTransition(result);
}

std::string ValueFederate::localQuery(const std::string& queryStr) const
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

const Input& ValueFederate::getInput(const std::string& key) const
{
    auto& inp = vfManager->getInput(key);
    if (!inp.isValid()) {
        return vfManager->getInput(getName() + nameSegmentSeparator + key);
    }
    return inp;
}

Input& ValueFederate::getInput(const std::string& key)
{
    auto& inp = vfManager->getInput(key);
    if (!inp.isValid()) {
        return vfManager->getInput(getName() + nameSegmentSeparator + key);
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

const Input& ValueFederate::getInput(const std::string& key, int index1) const
{
    return vfManager->getInput(key + '_' + std::to_string(index1));
}

const Input& ValueFederate::getInput(const std::string& key, int index1, int index2) const
{
    return vfManager->getInput(key + '_' + std::to_string(index1) + '_' + std::to_string(index2));
}

const Input& ValueFederate::getSubscription(const std::string& target) const
{
    return vfManager->getSubscription(target);
}

Input& ValueFederate::getSubscription(const std::string& target)
{
    return vfManager->getSubscription(target);
}

Publication& ValueFederate::getPublication(const std::string& key)
{
    auto& pub = vfManager->getPublication(key);
    if (!pub.isValid()) {
        return vfManager->getPublication(getName() + nameSegmentSeparator + key);
    }
    return pub;
}

const Publication& ValueFederate::getPublication(const std::string& key) const
{
    auto& pub = vfManager->getPublication(key);
    if (!pub.isValid()) {
        return vfManager->getPublication(getName() + nameSegmentSeparator + key);
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

const Publication& ValueFederate::getPublication(const std::string& key, int index1) const
{
    return vfManager->getPublication(key + '_' + std::to_string(index1));
}

const Publication&
    ValueFederate::getPublication(const std::string& key, int index1, int index2) const
{
    return vfManager->getPublication(key + '_' + std::to_string(index1) + '_' +
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
