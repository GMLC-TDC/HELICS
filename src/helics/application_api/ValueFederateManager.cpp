/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "ValueFederateManager.hpp"

#include "../common/JsonBuilder.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/queryHelpers.hpp"
#include "Inputs.hpp"
#include "Publications.hpp"

#include <utility>
namespace helics {
ValueFederateManager::ValueFederateManager(Core* coreOb, ValueFederate* vfed, local_federate_id id):
    coreObject(coreOb), fed(vfed), fedID(id)
{
}
ValueFederateManager::~ValueFederateManager() = default;

void ValueFederateManager::disconnect()
{
    // checks for the calls are handled in the MessageFederate itself
    coreObject = nullptr;
}

static const std::map<std::string, int> typeSizes = {
    {"char", 2},
    {"uchar", 2},
    {"block_4", 5},
    {"block_8", 9},
    {"block_12", 13},
    {"block_16", 17},
    {"block_20", 24},
    {"block_24", 30},
    {"double", 9},
    {"float", 5},
    {"int32", 5},
    {"uint32", 5},
    {"int64", 9},
    {"uint64", 9},
    {"complex", 17},
    {"complex_f", 9},
};

int getTypeSize(const std::string& type)
{
    auto ret = typeSizes.find(type);
    return (ret == typeSizes.end()) ? (-1) : ret->second;
}

Publication& ValueFederateManager::registerPublication(const std::string& key,
                                                       const std::string& type,
                                                       const std::string& units)
{
    auto coreID = coreObject->registerPublication(fedID, key, type, units);

    auto pubHandle = publications.lock();
    decltype(pubHandle->insert(key, coreID, fed, coreID, key, type, units)) active;
    if (!key.empty()) {
        active = pubHandle->insert(key, coreID, fed, coreID, key, type, units);
    } else {
        active = pubHandle->insert(no_search, coreID, fed, coreID, key, type, units);
    }

    if (active) {
        return pubHandle->back();
    }
    throw(RegistrationFailure("Unable to register Publication"));
}

Input& ValueFederateManager::registerInput(const std::string& key,
                                           const std::string& type,
                                           const std::string& units)
{
    auto coreID = coreObject->registerInput(fedID, key, type, units);
    auto inpHandle = inputs.lock();
    decltype(inpHandle->insert(key, coreID, fed, coreID, key, units)) active;
    if (!key.empty()) {
        active = inpHandle->insert(key, coreID, fed, coreID, key, units);
    } else {
        active = inpHandle->insert(no_search, coreID, fed, coreID, key, units);
    }
    if (active) {
        auto& ref = inpHandle->back();
        auto edat = std::make_unique<input_info>(key, type, units);
        // non-owning pointer
        ref.dataReference = edat.get();
        auto datHandle = inputData.lock();
        datHandle->push_back(std::move(edat));
        ref.referenceIndex = static_cast<int>(datHandle->size() - 1);
        return ref;
    }
    throw(RegistrationFailure("Unable to register Input"));
}

void ValueFederateManager::addAlias(const Input& inp, const std::string& shortcutName)
{
    if (inp.isValid()) {
        auto inpHandle = inputs.lock();
        inpHandle->addSearchTerm(shortcutName, inp.handle);
        targetIDs.lock()->emplace(shortcutName, inp.handle);
    } else {
        throw(InvalidIdentifier("input is invalid"));
    }
}

void ValueFederateManager::addAlias(const Publication& pub, const std::string& shortcutName)
{
    if (pub.isValid()) {
        auto pubHandle = publications.lock();
        pubHandle->addSearchTerm(shortcutName, pub.handle);
    } else {
        throw(InvalidIdentifier("publication is invalid"));
    }
}

void ValueFederateManager::addTarget(const Publication& pub, const std::string& target)
{
    coreObject->addDestinationTarget(pub.handle, target);
    targetIDs.lock()->emplace(target, pub.handle);
}

void ValueFederateManager::addTarget(const Input& inp, const std::string& target)
{
    {
        auto iTHandle = inputTargets.lock();
        auto rng = iTHandle->equal_range(inp.handle);
        for (auto el = rng.first; el != rng.second; ++el) {
            if (el->second == target) {
                fed->logWarningMessage(std::string("Duplicate input targets detected for ") +
                                       inp.actualName + "::" + target);
                return;
            }
        }
    }

    coreObject->addSourceTarget(inp.handle, target);
    inputTargets.lock()->emplace(inp.handle, target);
    targetIDs.lock()->emplace(target, inp.handle);
}

void ValueFederateManager::removeTarget(const Publication& pub, const std::string& target)
{
    // TODO(PT): erase from targetID's
    coreObject->removeTarget(pub.handle, target);
}

void ValueFederateManager::removeTarget(const Input& inp, const std::string& target)
{
    auto iTHandle = inputTargets.lock();
    auto rng = iTHandle->equal_range(inp.handle);
    for (auto el = rng.first; el != rng.second; ++el) {
        if (el->second == target) {
            coreObject->removeTarget(inp.handle, target);
            iTHandle->erase(el);
            break;
        }
    }
    // TODO(PT): erase from targetID's
}

void ValueFederateManager::setDefaultValue(const Input& inp, const data_view& block)
{
    if (inp.isValid()) {
        auto* info = static_cast<input_info*>(inp.dataReference);

        /** copy the data first since we are not entirely sure of the lifetime of the data_view*/
        info->lastData = data_view(std::make_shared<data_block>(block.data(), block.size()));
        info->lastUpdate = CurrentTime;
    } else {
        throw(InvalidIdentifier("Input id is invalid"));
    }
}

/** we have a new message from the core*/
void ValueFederateManager::getUpdateFromCore(interface_handle updatedHandle)
{
    auto data = coreObject->getValue(updatedHandle);
    auto inpHandle = inputs.lock();
    /** find the id*/
    auto fid = inpHandle->find(updatedHandle);
    if (fid != inpHandle->end()) {  // assign the data

        auto* info = static_cast<input_info*>(fid->dataReference);
        info->lastData = data_view(std::move(data));
        info->lastUpdate = CurrentTime;
    }
}

data_view ValueFederateManager::getValue(const Input& inp)
{
    auto* iData = static_cast<input_info*>(inp.dataReference);
    if (iData != nullptr) {
        iData->lastQuery = CurrentTime;
        iData->hasUpdate = false;
        return iData->lastData;
    }
    return data_view();
}

/** function to check if the size is valid for the given type*/
inline bool isBlockSizeValid(int size, const publication_info& pubI)
{
    return ((pubI.size < 0) || (pubI.size == size));
}

void ValueFederateManager::publish(const Publication& pub, const data_view& block)
{
    coreObject->setValue(pub.handle, block.data(), block.size());
}

bool ValueFederateManager::hasUpdate(const Input& inp)
{
    auto* iData = static_cast<input_info*>(inp.dataReference);
    if (iData != nullptr) {
        return iData->hasUpdate;
    }
    return false;
}

Time ValueFederateManager::getLastUpdateTime(const Input& inp)
{
    auto* iData = static_cast<input_info*>(inp.dataReference);
    if (iData != nullptr) {
        return iData->lastUpdate;
    }
    return Time::minVal();
}

void ValueFederateManager::updateTime(Time newTime, Time /*oldTime*/)
{
    CurrentTime = newTime;
    auto handles = coreObject->getValueUpdates(fedID);
    if (handles.empty()) {
        return;
    }
    // lock the data updates
    auto inpHandle = inputs.lock();
    auto allCall = allCallback.load();
    for (auto handle : handles) {
        /** find the id*/
        auto fid = inpHandle->find(handle);
        if (fid != inpHandle->end()) {  // assign the data
            auto* iData = static_cast<input_info*>(fid->dataReference);
            iData->lastUpdate = CurrentTime;

            bool updated = false;
            if (fid->getMultiInputMode() == multi_input_handling_method::no_op) {
                const auto& data = coreObject->getValue(handle);
                iData->lastData = data;
                iData->hasUpdate = true;
                updated = fid->checkUpdate(true);
            } else {
                const auto& dataV = coreObject->getAllValues(handle);
                iData->hasUpdate = false;
                updated = fid->vectorDataProcess(dataV);
            }

            if (updated) {
                if (iData->callback) {
                    Input& inp = *fid;

                    inpHandle.unlock();  // need to free the lock

                    // callbacks can do all sorts of things, best not to have it locked during the
                    // callback
                    iData->callback(inp, CurrentTime);
                    inpHandle = inputs.lock();
                } else if (allCall) {
                    Input& inp = *fid;
                    inpHandle.unlock();  // need to free the lock
                    // callbacks can do all sorts of strange things, best not to have it locked
                    // during the callback
                    allCall(inp, CurrentTime);
                    inpHandle = inputs.lock();
                }
            }
        }
    }
}

void ValueFederateManager::startupToInitializeStateTransition()
{
    // get the actual publication types
    auto inpHandle = inputs.lock();
    inpHandle->apply([](auto& inp) { inp.loadSourceInformation(); });
}

void ValueFederateManager::initializeToExecuteStateTransition(iteration_result result)
{
    Time ctime = result == iteration_result::next_step ? timeZero : initializationTime;
    updateTime(ctime, initializationTime);
}

std::string ValueFederateManager::localQuery(const std::string& queryStr) const
{
    std::string ret;
    if (queryStr == "inputs") {
        ret = generateStringVector_if(
            inputs.lock_shared(),
            [](const auto& info) { return info.actualName; },
            [](const auto& info) { return (!info.actualName.empty()); });
    } else if (queryStr == "publications") {
        ret = generateStringVector_if(
            publications.lock_shared(),
            [](const auto& info) { return info.getName(); },
            [](const auto& info) { return (!info.getName().empty()); });
    } else if (queryStr == "subscriptions") {
        ret = generateStringVector(targetIDs.lock_shared(),
                                   [](const auto& target) { return target.first; });
    } else if (queryStr == "updated_input_indices") {
        ret = "[";
        auto hand = inputs.lock();
        int ii = 0;
        for (const auto& inp : *hand) {
            if (inp.isUpdated()) {
                ret.append(std::to_string(ii));
                ret.push_back(';');
            }
            ++ii;
        }
        if (ret.back() == ';') {
            ret.pop_back();
        }
        ret.push_back(']');
    } else if (queryStr == "updated_input_names") {
        ret = generateStringVector_if(
            inputs.lock_shared(),
            [](const auto& inp) { return inp.getDisplayName(); },
            [](const auto& inp) { return (inp.isUpdated()); });
    } else if (queryStr == "updates") {
        JsonBuilder JB;
        for (const auto& inp : inputs.lock_shared()) {
            if (inp.isUpdated()) {
                auto inpTemp = inp;
                auto iType = inpTemp.getHelicsType();
                if (iType == data_type::helics_any || iType == data_type::helics_unknown) {
                    iType = inp.getHelicsInjectionType();
                }
                if (iType == data_type::helics_double) {
                    JB.addElement(inp.getDisplayName(), inpTemp.getValue<double>());
                } else if (iType == data_type::helics_vector) {
                    JB.addElement(inp.getDisplayName(), inpTemp.getValue<std::vector<double>>());
                } else {
                    JB.addElement(inp.getDisplayName(), inpTemp.getValue<std::string>());
                }
            }
        }
        ret = JB.generate();
    } else if (queryStr == "values") {
        JsonBuilder JB;
        for (const auto& inp : inputs.lock_shared()) {
            auto inpTemp = inp;
            inpTemp.checkUpdate(true);
            auto iType = inpTemp.getHelicsType();
            if (iType == data_type::helics_any || iType == data_type::helics_unknown) {
                iType = inp.getHelicsInjectionType();
            }
            if (iType == data_type::helics_double) {
                JB.addElement(inp.getDisplayName(), inpTemp.getValue<double>());
            } else if (iType == data_type::helics_vector) {
                JB.addElement(inp.getDisplayName(), inpTemp.getValue<std::vector<double>>());
            } else {
                JB.addElement(inp.getDisplayName(), inpTemp.getValue<std::string>());
            }
        }
        ret = JB.generate();
    }
    return ret;
}

std::vector<int> ValueFederateManager::queryUpdates()
{
    std::vector<int> updates;
    auto inpHandle = inputs.lock_shared();
    int ii = 0;
    for (const auto& inp : *inpHandle) {
        if (inp.hasUpdate) {
            updates.push_back(ii);
        }
        ++ii;
    }
    return updates;
}

static const std::string emptyStr;

const std::string& ValueFederateManager::getTarget(const Input& inp) const
{
    auto inpHandle = inputTargets.lock_shared();
    auto fnd = inpHandle->find(inp.handle);
    if (fnd != inpHandle->end()) {
        return fnd->second;
    }
    return emptyStr;
}

static const Input invalidIpt{};
static Input invalidIptNC{};

const Input& ValueFederateManager::getInput(const std::string& key) const
{
    auto inpHandle = inputs.lock_shared();
    auto inpF = inpHandle->find(key);
    if (inpF != inpHandle->end()) {
        return *inpF;
    }
    return invalidIpt;
}

Input& ValueFederateManager::getInput(const std::string& key)
{
    auto inpHandle = inputs.lock();
    auto inpF = inpHandle->find(key);
    if (inpF != inpHandle->end()) {
        return *inpF;
    }
    return invalidIptNC;
}

const Input& ValueFederateManager::getInput(int index) const
{
    auto inpHandle = inputs.lock_shared();
    if (isValidIndex(index, *inpHandle)) {
        return (*inpHandle)[index];
    }
    return invalidIpt;
}

Input& ValueFederateManager::getInput(int index)
{
    auto inpHandle = inputs.lock();
    if (isValidIndex(index, *inpHandle)) {
        return (*inpHandle)[index];
    }
    return invalidIptNC;
}

const Input& ValueFederateManager::getSubscription(const std::string& key) const
{
    auto TIDhandle = targetIDs.lock_shared();
    auto res = TIDhandle->equal_range(key);
    if (res.first != res.second) {
        auto inps = inputs.lock_shared();
        auto ret = inps->find(res.first->second);
        if (ret != inps->end()) {
            return *ret;
        }
    }
    return invalidIpt;
}

Input& ValueFederateManager::getSubscription(const std::string& key)
{
    auto TIDhandle = targetIDs.lock_shared();
    auto res = TIDhandle->equal_range(key);
    if (res.first != res.second) {
        auto inps = inputs.lock();
        auto ret = inps->find(res.first->second);
        if (ret != inps->end()) {
            return *ret;
        }
    }
    return invalidIptNC;
}

static const Publication invalidPub{};
static Publication invalidPubNC{};

const Publication& ValueFederateManager::getPublication(const std::string& key) const
{
    auto pubHandle = publications.lock_shared();
    auto pubF = pubHandle->find(key);
    if (pubF != pubHandle->end()) {
        return *pubF;
    }
    return invalidPub;
}

Publication& ValueFederateManager::getPublication(const std::string& key)
{
    auto pubHandle = publications.lock();
    auto pubF = pubHandle->find(key);
    if (pubF != pubHandle->end()) {
        return *pubF;
    }
    return invalidPubNC;
}

const Publication& ValueFederateManager::getPublication(int index) const
{
    auto pubHandle = publications.lock_shared();
    if (isValidIndex(index, *pubHandle)) {
        return (*pubHandle)[index];
    }
    return invalidPub;
}

Publication& ValueFederateManager::getPublication(int index)
{
    auto pubHandle = publications.lock();
    if (isValidIndex(index, *pubHandle)) {
        return (*pubHandle)[index];
    }
    return invalidPubNC;
}

/** get a count of the number publications registered*/
int ValueFederateManager::getPublicationCount() const
{
    return static_cast<int>(publications.lock_shared()->size());
}
/** get a count of the number inputs registered*/
int ValueFederateManager::getInputCount() const
{
    return static_cast<int>(inputs.lock_shared()->size());
}

void ValueFederateManager::clearUpdates()
{
    inputs.lock()->apply([](auto& inp) { inp.clearUpdate(); });
}

void ValueFederateManager::clearUpdate(const Input& inp)
{
    auto* iData = static_cast<input_info*>(inp.dataReference);
    if (iData != nullptr) {
        iData->hasUpdate = false;
    }
}

void ValueFederateManager::setInputNotificationCallback(std::function<void(Input&, Time)> callback)
{
    allCallback.store(std::move(callback));
}

void ValueFederateManager::setInputNotificationCallback(const Input& inp,
                                                        std::function<void(Input&, Time)> callback)
{
    auto* iData = static_cast<input_info*>(inp.dataReference);
    if (iData != nullptr) {
        iData->callback = std::move(callback);
    } else {
        throw(InvalidIdentifier("Input is not valid"));
    }
}

}  // namespace helics
