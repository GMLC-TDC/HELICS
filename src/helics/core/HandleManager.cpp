/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "HandleManager.hpp"

#include <algorithm>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

namespace helics {
BasicHandleInfo& HandleManager::addHandle(GlobalFederateId fed_id,
                                          InterfaceType what,
                                          std::string_view key,
                                          std::string_view type,
                                          std::string_view units)
{
    const InterfaceHandle local_id(static_cast<InterfaceHandle::BaseType>(handles.size()));
    const std::string actKey = (!key.empty()) ? std::string(key) : generateName(what);
    handles.emplace_back(fed_id, local_id, what, actKey, type, units);
    addSearchFields(handles.back(), local_id.baseValue());
    return handles.back();
}

BasicHandleInfo& HandleManager::addHandle(GlobalFederateId fed_id,
                                          InterfaceHandle local_id,
                                          InterfaceType what,
                                          std::string_view key,
                                          std::string_view type,
                                          std::string_view units)
{
    auto index = static_cast<int32_t>(handles.size());
    const std::string actKey = (!key.empty()) ? std::string{key} : generateName(what);
    handles.emplace_back(fed_id, local_id, what, actKey, type, units);
    addSearchFields(handles.back(), index);
    return handles.back();
}

void HandleManager::addHandle(const BasicHandleInfo& otherHandle)
{
    auto index = static_cast<int32_t>(handles.size());
    handles.push_back(otherHandle);
    addSearchFields(handles.back(), index);
}

void HandleManager::removeHandle(GlobalHandle handle)
{
    auto key = static_cast<uint64_t>(handle);
    auto fnd = unique_ids.find(key);
    if (fnd == unique_ids.end()) {
        return;
    }
    auto index = fnd->second;
    auto& info = handles[index];
    unique_ids.erase(fnd);
    if (!info.key.empty()) {
        switch (info.handleType) {
            case InterfaceType::ENDPOINT:
                endpoints.erase(info.key);
                break;
            case InterfaceType::PUBLICATION:
                publications.erase(info.key);
                break;
            case InterfaceType::FILTER:
                filters.erase(info.key);
                break;
            case InterfaceType::INPUT:
                inputs.erase(info.key);
                break;
            case InterfaceType::TRANSLATOR:
                inputs.erase(info.key);
                endpoints.erase(info.key);
                publications.erase(info.key);
                break;
            case InterfaceType::SINK:
                inputs.erase(info.key);
                endpoints.erase(info.key);
                break;
            default:
                break;
        }
    }
    // construct a blank at the previous index
    new (&(handles[index])) BasicHandleInfo;
}

void HandleManager::removeFederateHandles(GlobalFederateId fedToRemove)
{
    for (auto& handle : handles) {
        if (handle.getFederateId() == fedToRemove) {
            removeHandle(handle.handle);
        }
    }
}

void HandleManager::addHandleAtIndex(const BasicHandleInfo& otherHandle, int32_t index)
{
    if (index == static_cast<int32_t>(handles.size())) {
        addHandle(otherHandle);
    } else if (isValidIndex(index, handles)) {
        // use placement new to reconstruct new object
        new (&handles[index]) BasicHandleInfo(otherHandle);
        addSearchFields(handles[index], index);
    } else if (index > 0) {
        handles.resize(static_cast<size_t>(index) + 1);
        // use placement new to reconstruct new object
        new (&handles[index]) BasicHandleInfo(otherHandle);
        addSearchFields(handles[index], index);
    }
}
BasicHandleInfo* HandleManager::getHandleInfo(int32_t index)
{
    if (isValidIndex(index, handles)) {
        return &handles[index];
    }

    return nullptr;
}

const BasicHandleInfo* HandleManager::getHandleInfo(int32_t index) const
{
    if (isValidIndex(index, handles)) {
        return &handles[index];
    }

    return nullptr;
}

BasicHandleInfo* HandleManager::getHandleInfo(InterfaceHandle handle)
{
    if (isValidIndex(handle.baseValue(), handles)) {
        return &handles[handle.baseValue()];
    }

    return nullptr;
}

const BasicHandleInfo* HandleManager::getHandleInfo(InterfaceHandle handle) const
{
    if (isValidIndex(handle.baseValue(), handles)) {
        return &handles[handle.baseValue()];
    }

    return nullptr;
}

BasicHandleInfo* HandleManager::findHandle(GlobalHandle fed_id)
{
    auto key = static_cast<uint64_t>(fed_id);
    auto fnd = unique_ids.find(key);
    if (fnd != unique_ids.end()) {
        return &handles[fnd->second];
    }
    return nullptr;
}

const BasicHandleInfo* HandleManager::findHandle(GlobalHandle fed_id) const
{
    auto key = static_cast<uint64_t>(fed_id);
    auto fnd = unique_ids.find(key);
    if (fnd != unique_ids.end()) {
        return &handles[fnd->second];
    }
    return nullptr;
}
void HandleManager::setHandleOption(InterfaceHandle handle, int32_t option, int32_t val)
{
    auto index = handle.baseValue();
    if (isValidIndex(index, handles)) {
        switch (option) {
            case HELICS_HANDLE_OPTION_CONNECTION_REQUIRED:
                if (val != 0) {
                    clearActionFlag(handles[index], optional_flag);
                    setActionFlag(handles[index], required_flag);
                } else {
                    clearActionFlag(handles[index], required_flag);
                }
                break;
            case HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL:
                if (val != 0) {
                    clearActionFlag(handles[index], required_flag);
                    setActionFlag(handles[index], optional_flag);
                } else {
                    clearActionFlag(handles[index], optional_flag);
                }
                break;
            case HELICS_HANDLE_OPTION_RECEIVE_ONLY:
                if (handles[index].handleType == InterfaceType::ENDPOINT) {
                    if (val != 0) {
                        setActionFlag(handles[index], receive_only_flag);
                    } else {
                        clearActionFlag(handles[index], receive_only_flag);
                    }
                }
                break;
            case HELICS_HANDLE_OPTION_RECONNECTABLE:
                if (val != 0) {
                    setActionFlag(handles[index], reconnectable_flag);
                } else {
                    clearActionFlag(handles[index], reconnectable_flag);
                }
                break;
            default:
                break;
        }
    }
}

int32_t HandleManager::getHandleOption(InterfaceHandle handle, int32_t option) const
{
    auto index = handle.baseValue();
    bool rvalue{false};
    if (isValidIndex(index, handles)) {
        switch (option) {
            case HELICS_HANDLE_OPTION_ONLY_UPDATE_ON_CHANGE:
                rvalue = checkActionFlag(handles[index], only_update_on_change_flag);
                break;
            case HELICS_HANDLE_OPTION_ONLY_TRANSMIT_ON_CHANGE:
                rvalue = checkActionFlag(handles[index], only_transmit_on_change_flag);
                break;
            case HELICS_HANDLE_OPTION_CONNECTION_REQUIRED:
                rvalue = checkActionFlag(handles[index], required_flag);
                break;
            case HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL:
                rvalue = checkActionFlag(handles[index], optional_flag);
                break;
            case HELICS_HANDLE_OPTION_SINGLE_CONNECTION_ONLY:
                rvalue = checkActionFlag(handles[index], single_connection_flag);
                break;
            case HELICS_HANDLE_OPTION_RECONNECTABLE:
                rvalue = checkActionFlag(handles[index], reconnectable_flag);
                break;
            case HELICS_HANDLE_OPTION_RECEIVE_ONLY:
                rvalue = checkActionFlag(handles[index], receive_only_flag);
                break;
            default:
                break;
        }
    }
    return rvalue ? 1 : 0;
}

static bool interfaceTypeMatch(InterfaceType given, InterfaceType expected)
{
    if (given == expected) {
        return true;
    }
    switch (expected) {
        case InterfaceType::PUBLICATION:
            return (given == InterfaceType::TRANSLATOR);
        case InterfaceType::INPUT:
        case InterfaceType::ENDPOINT:
            return (given == InterfaceType::TRANSLATOR || given == InterfaceType::SINK);
        default:
            break;
    }
    return false;
}

HandleManager::MapType& HandleManager::getMap(InterfaceType type)
{
    switch (type) {
        case InterfaceType::ENDPOINT:
        case InterfaceType::TRANSLATOR:
        case InterfaceType::SINK:
        default:
            return endpoints;
        case InterfaceType::PUBLICATION:
            return publications;
        case InterfaceType::INPUT:
            return inputs;
        case InterfaceType::FILTER:
            return filters;
    }
}

const HandleManager::MapType& HandleManager::getMap(InterfaceType type) const
{
    {
        switch (type) {
            case InterfaceType::ENDPOINT:
            case InterfaceType::TRANSLATOR:
            case InterfaceType::SINK:
            default:
                return endpoints;
            case InterfaceType::PUBLICATION:
                return publications;
            case InterfaceType::INPUT:
                return inputs;
            case InterfaceType::FILTER:
                return filters;
        }
    }
}

BasicHandleInfo* HandleManager::getInterfaceHandle(std::string_view name, InterfaceType type)
{
    auto& imap = getMap(type);

    BasicHandleInfo* handle{nullptr};
    auto fnd = imap.find(name);
    if (fnd != imap.end()) {
        handle = &handles[fnd->second.baseValue()];
        if (type == InterfaceType::TRANSLATOR) {
            if (handle->handleType != InterfaceType::TRANSLATOR) {
                handle = nullptr;
            }
        } else if (type == InterfaceType::SINK) {
            if (handle->handleType != InterfaceType::SINK) {
                handle = nullptr;
            }
        }
    }
    return handle;
}

const BasicHandleInfo* HandleManager::getInterfaceHandle(std::string_view name,
                                                         InterfaceType type) const
{
    const auto& imap = getMap(type);
    const BasicHandleInfo* handle{nullptr};
    auto fnd = imap.find(name);
    if (fnd != imap.end()) {
        handle = &handles[fnd->second.baseValue()];
        if (type == InterfaceType::TRANSLATOR) {
            if (handle->handleType != InterfaceType::TRANSLATOR) {
                handle = nullptr;
            }
        } else if (type == InterfaceType::SINK) {
            if (handle->handleType != InterfaceType::SINK) {
                handle = nullptr;
            }
        }
    }
    return handle;
}

std::vector<GlobalHandle> HandleManager::regexSearch(const std::string& regexExpression,
                                                     InterfaceType type) const
{
    const auto& imap = getMap(type);
    std::vector<GlobalHandle> matches;
    if (regexExpression.compare(0, 6, "REGEX:") != 0) {
        return matches;
    }

    std::string rex = regexExpression.substr(6);
    if (rex == "*") {
        rex = ".*";
    }
    try {
        const std::regex rgx(rex);
        for (const auto& mres : imap) {
            if (std::regex_match(mres.first.data(), rgx)) {
                const auto* handle = getHandleInfo(mres.second);
                matches.push_back(handle->handle);
            }
        }
    }
    catch (const std::regex_error& re) {
        throw std::invalid_argument(re.what());
    }
    return matches;
}

BasicHandleInfo* HandleManager::getInterfaceHandle(InterfaceHandle handle, InterfaceType type)
{
    auto index = handle.baseValue();
    if (isValidIndex(index, handles)) {
        auto& hand = handles[index];
        if (interfaceTypeMatch(hand.handleType, type)) {
            return &hand;
        }
    }

    return nullptr;
}

const BasicHandleInfo* HandleManager::getInterfaceHandle(InterfaceHandle handle,
                                                         InterfaceType type) const
{
    auto index = handle.baseValue();
    if (isValidIndex(index, handles)) {
        const auto& hand = handles[index];
        if (interfaceTypeMatch(hand.handleType, type)) {
            return &hand;
        }
    }

    return nullptr;
}

LocalFederateId HandleManager::getLocalFedID(InterfaceHandle handle) const
{
    auto index = handle.baseValue();
    return (isValidIndex(index, handles)) ? handles[index].local_fed_id : LocalFederateId{};
}

void HandleManager::addAlias(std::string_view interfaceName, std::string_view alias)
{
    // interfaceName needs to be stable to emplace here so we create a stable string in the aliases
    // map
    auto [aliasName, existing1] = alias_names.emplace(alias);
    auto [iName, existing2] = alias_names.emplace(interfaceName);
    const std::string& aliasStableName = *aliasName;
    const std::string& interfaceStableName = *iName;

    bool cascade = addAliasName(interfaceStableName, aliasStableName);

    if (cascade) {
        auto& aliasRange = aliases[interfaceStableName];
        for (auto& interfaceAlias : aliasRange) {
            if (interfaceAlias != alias) {
                addPublicationAlias(interfaceAlias, interfaceStableName);
                addInputAlias(interfaceAlias, interfaceStableName);
                addEndpointAlias(interfaceAlias, interfaceStableName);
                addFilterAlias(interfaceAlias, interfaceStableName);
            }
        }
    }

    // add aliases for existing interfaces
    addPublicationAlias(*iName, *aliasName);
    addInputAlias(*iName, *aliasName);
    addEndpointAlias(*iName, *aliasName);
    addFilterAlias(*iName, *aliasName);

    /** aliases should be reciprocal*/
    cascade = addAliasName(aliasStableName, interfaceStableName);

    if (cascade) {
        auto& aliasRange = aliases[aliasStableName];
        for (auto& interfaceAlias : aliasRange) {
            if (interfaceAlias != alias) {
                addPublicationAlias(interfaceAlias, aliasStableName);
                addInputAlias(interfaceAlias, aliasStableName);
                addEndpointAlias(interfaceAlias, aliasStableName);
                addFilterAlias(interfaceAlias, aliasStableName);
            }
        }
    }

    // add aliases for existing interfaces
    addPublicationAlias(*aliasName, *iName);
    addInputAlias(*aliasName, *iName);
    addEndpointAlias(*aliasName, *iName);
    addFilterAlias(*aliasName, *iName);
}

void HandleManager::addPublicationAlias(std::string_view interfaceName, std::string_view alias)
{
    auto fnd = publications.find(interfaceName);
    if (fnd != publications.end()) {
        auto res = publications.try_emplace(alias, fnd->second.baseValue());
        if (!res.second && res.first->second != fnd->second) {
            throw std::runtime_error("publication name and alias already exists");
        }
    } else {
        fnd = publications.find(alias);
        if (fnd != publications.end()) {
            publications.emplace(interfaceName, fnd->second.baseValue());
        }
    }
}

void HandleManager::addEndpointAlias(std::string_view interfaceName, std::string_view alias)
{
    auto fnd = endpoints.find(interfaceName);
    if (fnd != endpoints.end()) {
        auto res = endpoints.try_emplace(alias, fnd->second.baseValue());
        if (!res.second && res.first->second != fnd->second) {
            throw std::runtime_error("endpoint name and alias already exists");
        }
    } else {
        fnd = endpoints.find(alias);
        if (fnd != endpoints.end()) {
            endpoints.emplace(interfaceName, fnd->second.baseValue());
        }
    }
}

void HandleManager::addFilterAlias(std::string_view interfaceName, std::string_view alias)
{
    auto fnd = filters.find(interfaceName);
    if (fnd != filters.end()) {
        auto res = filters.try_emplace(alias, fnd->second.baseValue());
        if (!res.second && res.first->second != fnd->second) {
            throw std::runtime_error("filter name and alias already exists");
        }
    } else {
        fnd = filters.find(alias);
        if (fnd != filters.end()) {
            filters.emplace(interfaceName, fnd->second.baseValue());
        }
    }
}

void HandleManager::addInputAlias(std::string_view interfaceName, std::string_view alias)
{
    auto fnd = inputs.find(interfaceName);
    if (fnd != inputs.end()) {
        auto res = inputs.try_emplace(alias, fnd->second.baseValue());
        if (!res.second && res.first->second != fnd->second) {
            throw std::runtime_error("input name and alias already exists");
        }
    } else {
        fnd = inputs.find(alias);
        if (fnd != inputs.end()) {
            inputs.emplace(interfaceName, fnd->second.baseValue());
        }
    }
}

bool HandleManager::addAliasName(std::string_view interfaceName, std::string_view alias)
{
    if (interfaceName == alias) {
        return false;
    }
    bool cascading{false};
    auto fnd = aliases.find(interfaceName);
    if (fnd == aliases.end()) {
        aliases[interfaceName].emplace_back(alias);
        cascading = addAliasName(alias, interfaceName);
    } else {
        auto& aliasVector = aliases[interfaceName];
        auto aliasIterator = std::lower_bound(aliasVector.begin(), aliasVector.end(), alias);
        if (aliasIterator == aliasVector.end() || *aliasIterator != alias) {
            aliasVector.insert(aliasIterator, alias);
            cascading = true;
            for (auto& otherAlias : aliasVector) {
                addAliasName(otherAlias, alias);
            }
            addAliasName(alias, interfaceName);
        }
    }

    return cascading;
}

using MapType = std::unordered_map<std::string_view, InterfaceHandle>;
using AliasT = std::unordered_map<std::string_view, std::vector<std::string_view>>;
static void addFields(std::string_view key,
                      std::string_view typeName,
                      InterfaceHandle hid,
                      MapType& searchMap,
                      AliasT aliases)
{
    auto placed = searchMap.try_emplace(key, hid);
    if (!placed.second) {
        throw std::runtime_error(std::string("duplicate ") + std::string(typeName) + " key found");
    }
    auto aliasRange = aliases.find(key);
    if (aliasRange != aliases.end()) {
        for (auto& alias : aliasRange->second) {
            placed = searchMap.try_emplace(alias, hid);
            if (!placed.second) {
                throw std::runtime_error(std::string("duplicate ") + std::string(typeName) +
                                         " alias key(" + std::string(alias) + ") found");
            }
        }
    }
}

void HandleManager::addSearchFields(const BasicHandleInfo& handle, int32_t index)
{
    if (!handle.key.empty()) {
        switch (handle.handleType) {
            case InterfaceType::ENDPOINT:
                addFields(handle.key, "endpoint", InterfaceHandle(index), endpoints, aliases);
                break;
            case InterfaceType::PUBLICATION:
                addFields(handle.key, "publication", InterfaceHandle(index), publications, aliases);
                break;
            case InterfaceType::FILTER:
                addFields(handle.key, "filter", InterfaceHandle(index), filters, aliases);
                break;
            case InterfaceType::INPUT:
                addFields(handle.key, "input", InterfaceHandle(index), inputs, aliases);
                break;
            case InterfaceType::TRANSLATOR:
                addFields(handle.key, "publication", InterfaceHandle(index), publications, aliases);
                addFields(handle.key, "endpoint", InterfaceHandle(index), endpoints, aliases);
                addFields(handle.key, "input", InterfaceHandle(index), inputs, aliases);
                break;
            case InterfaceType::SINK:
                addFields(handle.key, "endpoint", InterfaceHandle(index), endpoints, aliases);
                addFields(handle.key, "input", InterfaceHandle(index), inputs, aliases);
                break;
            default:
                break;
        }
    }
    // generate a key of the fed and handle
    unique_ids.emplace(static_cast<uint64_t>(handle.handle), index);
}

std::string HandleManager::generateName(InterfaceType what) const
{
    std::string base;
    switch (what) {
        case InterfaceType::ENDPOINT:
            base = "_ept_";
            break;
        case InterfaceType::INPUT:
            base = "_input_";
            break;
        case InterfaceType::PUBLICATION:
            base = "_pub_";
            break;
        case InterfaceType::FILTER:
            base = "_filter_";
            break;
        case InterfaceType::TRANSLATOR:
            base = "_translator_";
            break;
        case InterfaceType::SINK:
            base = "_sink_";
            break;
        default:
            base = "_handle_";
            break;
    }
    base.append(std::to_string(handles.size()));
    return base;
}
}  // namespace helics
