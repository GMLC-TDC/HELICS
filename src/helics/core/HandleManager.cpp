/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "HandleManager.hpp"

#include "ActionMessage.hpp"
// TODO(PT): move the flags out of actionMessage

namespace helics {
BasicHandleInfo& HandleManager::addHandle(GlobalFederateId fed_id,
                                          handle_type what,
                                          const std::string& key,
                                          const std::string& type,
                                          const std::string& units)
{
    InterfaceHandle local_id(static_cast<InterfaceHandle::BaseType>(handles.size()));
    std::string actKey = (!key.empty()) ? key : generateName(what);
    handles.emplace_back(fed_id, local_id, what, actKey, type, units);
    addSearchFields(handles.back(), local_id.baseValue());
    return handles.back();
}

BasicHandleInfo& HandleManager::addHandle(GlobalFederateId fed_id,
                                          InterfaceHandle local_id,
                                          handle_type what,
                                          const std::string& key,
                                          const std::string& type,
                                          const std::string& units)
{
    auto index = static_cast<int32_t>(handles.size());
    std::string actKey = (!key.empty()) ? key : generateName(what);
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
            case handle_type::endpoint:
                endpoints.erase(info.key);
                break;
            case handle_type::publication:
                publications.erase(info.key);
                break;
            case handle_type::filter:
                filters.erase(info.key);
                break;
            case handle_type::input:
                inputs.erase(info.key);
                break;
            default:
                break;
        }
    }
    // construct a blank at the previous index
    new (&(handles[index])) BasicHandleInfo;
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
            case helics_handle_option_connection_required:
                if (val != 0) {
                    clearActionFlag(handles[index], optional_flag);
                    setActionFlag(handles[index], required_flag);
                } else {
                    clearActionFlag(handles[index], required_flag);
                }
                break;
            case helics_handle_option_connection_optional:
                if (val != 0) {
                    clearActionFlag(handles[index], required_flag);
                    setActionFlag(handles[index], optional_flag);
                } else {
                    clearActionFlag(handles[index], optional_flag);
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
            case helics_handle_option_only_update_on_change:
                rvalue = checkActionFlag(handles[index], extra_flag1);
                break;
            case helics_handle_option_only_transmit_on_change:
                rvalue = checkActionFlag(handles[index], extra_flag2);
                break;
            case helics_handle_option_connection_required:
                rvalue = checkActionFlag(handles[index], required_flag);
                break;
            case helics_handle_option_connection_optional:
                rvalue = checkActionFlag(handles[index], optional_flag);
                break;
            case helics_handle_option_single_connection_only:
                rvalue = checkActionFlag(handles[index], extra_flag4);
                break;
            default:
                break;
        }
    }
    return rvalue;
}

BasicHandleInfo* HandleManager::getEndpoint(std::string_view name)
{
    auto fnd = endpoints.find(name);
    if (fnd != endpoints.end()) {
        return &handles[fnd->second.baseValue()];
    }
    return nullptr;
}

const BasicHandleInfo* HandleManager::getEndpoint(std::string_view name) const
{
    auto fnd = endpoints.find(name);
    if (fnd != endpoints.end()) {
        return &handles[fnd->second.baseValue()];
    }
    return nullptr;
}

BasicHandleInfo* HandleManager::getEndpoint(InterfaceHandle handle)
{
    auto index = handle.baseValue();
    if (isValidIndex(index, handles)) {
        auto& hand = handles[index];
        if (hand.handleType == handle_type::endpoint) {
            return &hand;
        }
    }

    return nullptr;
}

const BasicHandleInfo* HandleManager::getEndpoint(InterfaceHandle handle) const
{
    auto index = handle.baseValue();
    if (isValidIndex(index, handles)) {
        auto& hand = handles[index];
        if (hand.handleType == handle_type::endpoint) {
            return &hand;
        }
    }

    return nullptr;
}

BasicHandleInfo* HandleManager::getPublication(std::string_view name)
{
    auto fnd = publications.find(name);
    if (fnd != publications.end()) {
        return &(handles[fnd->second.baseValue()]);
    }
    return nullptr;
}

const BasicHandleInfo* HandleManager::getPublication(std::string_view name) const
{
    auto fnd = publications.find(name);
    if (fnd != publications.end()) {
        return &(handles[fnd->second.baseValue()]);
    }
    return nullptr;
}

BasicHandleInfo* HandleManager::getPublication(InterfaceHandle handle)
{
    auto index = handle.baseValue();
    if (isValidIndex(index, handles)) {
        auto& hand = handles[index];
        if (hand.handleType == handle_type::publication) {
            return &hand;
        }
    }

    return nullptr;
}

BasicHandleInfo* HandleManager::getInput(std::string_view name)
{
    auto fnd = inputs.find(name);
    if (fnd != inputs.end()) {
        return &(handles[fnd->second.baseValue()]);
    }
    return nullptr;
}

const BasicHandleInfo* HandleManager::getInput(std::string_view name) const
{
    auto fnd = inputs.find(name);
    if (fnd != inputs.end()) {
        return &(handles[fnd->second.baseValue()]);
    }
    return nullptr;
}

BasicHandleInfo* HandleManager::getFilter(std::string_view name)
{
    auto ar = filters.equal_range(name);
    if (ar.first == ar.second) {
        return nullptr;
    }
    return &(handles[ar.first->second.baseValue()]);
}

const BasicHandleInfo* HandleManager::getFilter(std::string_view name) const
{
    auto ar = filters.equal_range(name);
    if (ar.first == ar.second) {
        return nullptr;
    }
    return &(handles[ar.first->second.baseValue()]);
}
BasicHandleInfo* HandleManager::getFilter(InterfaceHandle handle)
{
    auto index = handle.baseValue();
    if (isValidIndex(index, handles)) {
        auto& hand = handles[index];
        if (hand.handleType == handle_type::filter) {
            return &hand;
        }
    }

    return nullptr;
}

LocalFederateId HandleManager::getLocalFedID(InterfaceHandle handle) const
{
    // only activate the lock if we not in an operating state
    auto index = handle.baseValue();
    return (isValidIndex(index, handles)) ? handles[index].local_fed_id : LocalFederateId{};
}

void HandleManager::addSearchFields(const BasicHandleInfo& handle, int32_t index)
{
    switch (handle.handleType) {
        case handle_type::endpoint:
            endpoints.emplace(handle.key, InterfaceHandle(index));
            break;
        case handle_type::publication:
            publications.emplace(handle.key, InterfaceHandle(index));
            break;
        case handle_type::filter:
            if (!handle.key.empty()) {
                filters.emplace(handle.key, InterfaceHandle(index));
            }
            break;
        case handle_type::input:
            inputs.emplace(handle.key, InterfaceHandle(index));
            break;
        default:
            break;
    }
    // generate a key of the fed and handle
    unique_ids.emplace(static_cast<uint64_t>(handle.handle), index);
}

std::string HandleManager::generateName(handle_type what) const
{
    switch (what) {
        case handle_type::endpoint:
            return std::string("_ept_") + std::to_string(handles.size());
        case handle_type::input:
            return std::string("_input_") + std::to_string(handles.size());
        case handle_type::publication:
            return std::string("_pub_") + std::to_string(handles.size());
        case handle_type::filter:
            return std::string("_filter_") + std::to_string(handles.size());
        default:
            return std::string("_handle_") + std::to_string(handles.size());
    }
}
}  // namespace helics
