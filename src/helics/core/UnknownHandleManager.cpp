/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "UnknownHandleManager.hpp"

#include "flagOperations.hpp"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace helics {
/** add a missingPublication*/
void UnknownHandleManager::addUnknownPublication(std::string_view key,
                                                 GlobalHandle target,
                                                 uint16_t flags)
{
    unknown_publications.emplace(key, std::make_pair(target, flags));
}
/** add a missingPublication*/
void UnknownHandleManager::addUnknownInput(std::string_view key,
                                           GlobalHandle target,
                                           uint16_t flags)
{
    unknown_inputs.emplace(key, std::make_pair(target, flags));
}

/** add a missing destination endpoint*/
void UnknownHandleManager::addUnknownEndpoint(std::string_view key,
                                              GlobalHandle target,
                                              uint16_t flags)
{
    unknown_endpoints.emplace(key, std::make_pair(target, flags));
}
/** add a missing filter*/
void UnknownHandleManager::addUnknownFilter(std::string_view key,
                                            GlobalHandle target,
                                            uint16_t flags)
{
    unknown_filters.emplace(key, std::make_pair(target, flags));
}

void UnknownHandleManager::addReconnectablePublication(std::string_view key,
                                                       GlobalHandle target,
                                                       uint16_t flags)
{
    reconnectable_publications.emplace(key, std::make_pair(target, flags));
}
/** add a missing publication*/
void UnknownHandleManager::addReconnectableInput(std::string_view key,
                                                 GlobalHandle target,
                                                 uint16_t flags)
{
    reconnectable_inputs.emplace(key, std::make_pair(target, flags));
}

/** add a missing destination endpoint*/
void UnknownHandleManager::addReconnectableEndpoint(std::string_view key,
                                                    GlobalHandle target,
                                                    uint16_t flags)
{
    reconnectable_endpoints.emplace(key, std::make_pair(target, flags));
}
/** add a missing filter*/
void UnknownHandleManager::addReconnectableFilter(std::string_view key,
                                                  GlobalHandle target,
                                                  uint16_t flags)
{
    reconnectable_filters.emplace(key, std::make_pair(target, flags));
}

void UnknownHandleManager::addDataLink(std::string_view source, std::string_view target)
{
    unknown_links.emplace(source, target);
}

void UnknownHandleManager::addEndpointLink(std::string_view source, std::string_view target)
{
    unknown_endpoint_links.emplace(source, target);
}

void UnknownHandleManager::addSourceFilterLink(std::string_view filter, std::string_view endpoint)
{
    unknown_src_filters.emplace(filter, endpoint);
}

void UnknownHandleManager::addDestinationFilterLink(std::string_view filter,
                                                    std::string_view endpoint)
{
    unknown_dest_filters.emplace(filter, endpoint);
}

static auto
    getTargets(const std::unordered_multimap<std::string, UnknownHandleManager::TargetInfo>& tmap,
               const std::string& target)
{
    std::vector<UnknownHandleManager::TargetInfo> targets;
    auto [firstTarget, lastTarget] = tmap.equal_range(target);
    if (firstTarget != tmap.end()) {
        auto currentTarget = firstTarget;
        while (currentTarget != lastTarget) {
            targets.push_back(currentTarget->second);
            ++currentTarget;
        }
    }
    return targets;
}

static auto getTargets(const std::unordered_multimap<std::string, std::string>& tmap,
                       const std::string& target)
{
    std::vector<std::string> targets;
    auto [firstTarget, lastTarget] = tmap.equal_range(target);
    if (firstTarget != tmap.end()) {
        auto currentTarget = firstTarget;
        while (currentTarget != lastTarget) {
            targets.push_back(currentTarget->second);
            ++currentTarget;
        }
    }
    return targets;
}

std::vector<UnknownHandleManager::TargetInfo>
    UnknownHandleManager::checkForInputs(const std::string& newInput) const
{
    return getTargets(unknown_inputs, newInput);
}
std::vector<UnknownHandleManager::TargetInfo>
    UnknownHandleManager::checkForPublications(const std::string& newPublication) const
{
    return getTargets(unknown_publications, newPublication);
}

std::vector<std::string> UnknownHandleManager::checkForLinks(const std::string& newSource) const
{
    return getTargets(unknown_links, newSource);
}

std::vector<std::string>
    UnknownHandleManager::checkForEndpointLinks(const std::string& newSource) const
{
    return getTargets(unknown_endpoint_links, newSource);
}

std::vector<UnknownHandleManager::TargetInfo>
    UnknownHandleManager::checkForEndpoints(const std::string& newEndpoint) const
{
    return getTargets(unknown_endpoints, newEndpoint);
}

std::vector<UnknownHandleManager::TargetInfo>
    UnknownHandleManager::checkForFilters(const std::string& newFilter) const
{
    return getTargets(unknown_filters, newFilter);
}

std::vector<std::string>
    UnknownHandleManager::checkForFilterSourceTargets(const std::string& newFilter) const
{
    return getTargets(unknown_src_filters, newFilter);
}

std::vector<std::string>
    UnknownHandleManager::checkForFilterDestTargets(const std::string& newFilter) const
{
    return getTargets(unknown_dest_filters, newFilter);
}

std::vector<UnknownHandleManager::TargetInfo>
    UnknownHandleManager::checkForReconnectionInputs(const std::string& newInput) const
{
    return getTargets(reconnectable_inputs, newInput);
}
std::vector<UnknownHandleManager::TargetInfo>
    UnknownHandleManager::checkForReconnectionPublications(const std::string& newPublication) const
{
    return getTargets(reconnectable_publications, newPublication);
}

std::vector<UnknownHandleManager::TargetInfo>
    UnknownHandleManager::checkForReconnectionEndpoints(const std::string& newEndpoint) const
{
    return getTargets(reconnectable_endpoints, newEndpoint);
}

std::vector<UnknownHandleManager::TargetInfo>
    UnknownHandleManager::checkForReconnectionFilters(const std::string& newFilter) const
{
    return getTargets(reconnectable_filters, newFilter);
}

bool UnknownHandleManager::hasUnknowns() const
{
    return (!(unknown_publications.empty() && unknown_endpoints.empty() && unknown_inputs.empty() &&
              unknown_filters.empty() && unknown_links.empty() && unknown_endpoint_links.empty() &&
              unknown_dest_filters.empty() && unknown_src_filters.empty()));
}

bool UnknownHandleManager::hasNonOptionalUnknowns() const
{
    if (!(unknown_links.empty() && unknown_endpoint_links.empty() && unknown_dest_filters.empty() &&
          unknown_src_filters.empty())) {
        return true;
    }

    auto optionalCheck = [](const auto& uInterface) {
        return ((uInterface.second.second & make_flags(optional_flag)) != 0);
    };
    return (
        !((std::all_of(unknown_publications.begin(), unknown_publications.end(), optionalCheck)) &&
          (std::all_of(unknown_inputs.begin(), unknown_inputs.end(), optionalCheck)) &&
          (std::all_of(unknown_endpoints.begin(), unknown_endpoints.end(), optionalCheck)) &&
          (std::all_of(unknown_filters.begin(), unknown_filters.end(), optionalCheck))));
}

bool UnknownHandleManager::hasRequiredUnknowns() const
{
    auto requiredCheck = [](const auto& uInterface) {
        return ((uInterface.second.second & make_flags(required_flag)) != 0);
    };
    return (
        (std::any_of(unknown_publications.begin(), unknown_publications.end(), requiredCheck)) ||
        (std::any_of(unknown_inputs.begin(), unknown_inputs.end(), requiredCheck)) ||
        (std::any_of(unknown_endpoints.begin(), unknown_endpoints.end(), requiredCheck)) ||
        (std::any_of(unknown_filters.begin(), unknown_filters.end(), requiredCheck)));
}

void UnknownHandleManager::processUnknowns(
    const std::function<void(const std::string&, InterfaceType, TargetInfo)>& cfunc) const
{
    for (const auto& upub : unknown_publications) {
        cfunc(upub.first, InterfaceType::PUBLICATION, upub.second);
    }
    for (const auto& uept : unknown_endpoints) {
        cfunc(uept.first, InterfaceType::ENDPOINT, uept.second);
    }
    for (const auto& uinp : unknown_inputs) {
        cfunc(uinp.first, InterfaceType::INPUT, uinp.second);
    }
    for (const auto& ufilt : unknown_filters) {
        cfunc(ufilt.first, InterfaceType::FILTER, ufilt.second);
    }
}

void UnknownHandleManager::processUnknownLinks(
    const std::function<
        void(const std::string& origin, InterfaceType, const std::string& target, InterfaceType)>&
        cfunc) const
{
    for (const auto& dlink : unknown_links) {
        cfunc(dlink.first, InterfaceType::PUBLICATION, dlink.second, InterfaceType::INPUT);
    }
    for (const auto& uept : unknown_endpoint_links) {
        cfunc(uept.first, InterfaceType::ENDPOINT, uept.second, InterfaceType::ENDPOINT);
    }
    for (const auto& ufilt : unknown_dest_filters) {
        cfunc(ufilt.first, InterfaceType::FILTER, ufilt.second, InterfaceType::ENDPOINT);
    }
    for (const auto& ufilt : unknown_src_filters) {
        cfunc(ufilt.first, InterfaceType::FILTER, ufilt.second, InterfaceType::ENDPOINT);
    }
}

void UnknownHandleManager::processNonOptionalUnknowns(
    const std::function<void(const std::string&, InterfaceType, TargetInfo handle)>& cfunc) const
{
    for (const auto& upub : unknown_publications) {
        if ((upub.second.second & make_flags(optional_flag)) != 0) {
            continue;
        }
        cfunc(upub.first, InterfaceType::PUBLICATION, upub.second);
    }
    for (const auto& uept : unknown_endpoints) {
        if ((uept.second.second & make_flags(optional_flag)) != 0) {
            continue;
        }
        cfunc(uept.first, InterfaceType::ENDPOINT, uept.second);
    }
    for (const auto& uinp : unknown_inputs) {
        if ((uinp.second.second & make_flags(optional_flag)) != 0) {
            continue;
        }
        cfunc(uinp.first, InterfaceType::INPUT, uinp.second);
    }

    for (const auto& ufilt : unknown_filters) {
        if ((ufilt.second.second & make_flags(optional_flag)) != 0) {
            continue;
        }
        cfunc(ufilt.first, InterfaceType::FILTER, ufilt.second);
    }
}

void UnknownHandleManager::processRequiredUnknowns(
    const std::function<void(const std::string&, InterfaceType, TargetInfo)>& cfunc) const
{
    for (const auto& upub : unknown_publications) {
        if ((upub.second.second & make_flags(required_flag)) != 0) {
            cfunc(upub.first, InterfaceType::PUBLICATION, upub.second);
        }
    }
    for (const auto& uept : unknown_endpoints) {
        if ((uept.second.second & make_flags(required_flag)) != 0) {
            cfunc(uept.first, InterfaceType::ENDPOINT, uept.second);
        }
    }
    for (const auto& uinp : unknown_inputs) {
        if ((uinp.second.second & make_flags(required_flag)) != 0) {
            cfunc(uinp.first, InterfaceType::INPUT, uinp.second);
        }
    }

    for (const auto& ufilt : unknown_filters) {
        if ((ufilt.second.second & make_flags(required_flag)) != 0) {
            cfunc(ufilt.first, InterfaceType::FILTER, ufilt.second);
        }
    }
}

/** specify a found input*/
void UnknownHandleManager::clearInput(const std::string& newInput)
{
    unknown_inputs.erase(newInput);
}

/** specify a found input*/
void UnknownHandleManager::clearPublication(const std::string& newPublication)
{
    unknown_publications.erase(newPublication);
    unknown_links.erase(newPublication);
}
/** specify a found input*/
void UnknownHandleManager::clearEndpoint(const std::string& newEndpoint)
{
    unknown_endpoints.erase(newEndpoint);
    unknown_endpoint_links.erase(newEndpoint);
}

/** specify a found input*/
void UnknownHandleManager::clearFilter(const std::string& newFilter)
{
    unknown_filters.erase(newFilter);
    unknown_src_filters.erase(newFilter);
    unknown_dest_filters.erase(newFilter);
}

// TODO(PT):  When move to C++20 use std::erase_if
// remove_if doesn't work since this is used on maps
template<typename ContainerT, typename PredicateT>
void maperase_if(ContainerT& items, const PredicateT& predicate)
{
    for (auto it = items.begin(); it != items.end();) {
        if (predicate(*it)) {
            it = items.erase(it);
        } else {
            ++it;
        }
    }
}

void UnknownHandleManager::clearFederateUnknowns(GlobalFederateId gid)
{
    auto clearCall = [gid](const auto& it) { return it.second.first.fed_id == gid; };
    maperase_if(unknown_publications, clearCall);
    maperase_if(unknown_endpoints, clearCall);
    maperase_if(unknown_filters, clearCall);
    maperase_if(unknown_inputs, clearCall);
}

void UnknownHandleManager::clearUnknownsIf(
    const std::function<bool(const std::string& name, InterfaceType, TargetInfo)>& cfunc)
{
    InterfaceType type = InterfaceType::PUBLICATION;
    auto clearCall = [&cfunc, &type](const auto& it) { return cfunc(it.first, type, it.second); };
    maperase_if(unknown_publications, clearCall);
    type = InterfaceType::ENDPOINT;
    maperase_if(unknown_endpoints, clearCall);
    type = InterfaceType::FILTER;
    maperase_if(unknown_filters, clearCall);
    type = InterfaceType::INPUT;
    maperase_if(unknown_inputs, clearCall);
}

}  // namespace helics
