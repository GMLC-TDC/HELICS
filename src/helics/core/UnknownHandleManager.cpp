/*
Copyright (c) 2017-2023,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "UnknownHandleManager.hpp"

#include "flagOperations.hpp"

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
    auto rp = tmap.equal_range(target);
    if (rp.first != tmap.end()) {
        auto it = rp.first;
        while (it != rp.second) {
            targets.push_back(it->second);
            ++it;
        }
    }
    return targets;
}

static auto getTargets(const std::unordered_multimap<std::string, std::string>& tmap,
                       const std::string& target)
{
    std::vector<std::string> targets;
    auto rp = tmap.equal_range(target);
    if (rp.first != tmap.end()) {
        auto it = rp.first;
        while (it != rp.second) {
            targets.push_back(it->second);
            ++it;
        }
    }
    return targets;
}

/** specify a found input*/
std::vector<UnknownHandleManager::TargetInfo>
    UnknownHandleManager::checkForInputs(const std::string& newInput) const
{
    return getTargets(unknown_inputs, newInput);
}
/** specify a found input*/
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

/** specify a found input*/
std::vector<UnknownHandleManager::TargetInfo>
    UnknownHandleManager::checkForEndpoints(const std::string& newEndpoint) const
{
    return getTargets(unknown_endpoints, newEndpoint);
}

/** specify a found input*/
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
    for (const auto& upub : unknown_publications) {
        if ((upub.second.second & make_flags(optional_flag)) != 0) {
            continue;
        }
        return true;
    }
    for (const auto& uept : unknown_endpoints) {
        if ((uept.second.second & make_flags(optional_flag)) != 0) {
            continue;
        }
        return true;
    }
    for (const auto& uinp : unknown_inputs) {
        if ((uinp.second.second & make_flags(optional_flag)) != 0) {
            continue;
        }
        return true;
    }

    for (const auto& ufilt : unknown_filters) {
        if ((ufilt.second.second & make_flags(optional_flag)) != 0) {
            continue;
        }
        return true;
    }
    return false;
}

bool UnknownHandleManager::hasRequiredUnknowns() const
{
    for (const auto& upub : unknown_publications) {
        if ((upub.second.second & make_flags(required_flag)) != 0) {
            return true;
        }
    }
    for (const auto& uept : unknown_endpoints) {
        if ((uept.second.second & make_flags(required_flag)) != 0) {
            return true;
        }
    }
    for (const auto& uinp : unknown_inputs) {
        if ((uinp.second.second & make_flags(required_flag)) != 0) {
            return true;
        }
    }

    for (const auto& ufilt : unknown_filters) {
        if ((ufilt.second.second & make_flags(required_flag)) != 0) {
            return true;
        }
    }
    return false;
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
        if (predicate(*it))
            it = items.erase(it);
        else
            ++it;
    }
}

void UnknownHandleManager::clearFederateUnknowns(GlobalFederateId id)
{
    auto ck = [id](const auto& it) { return it.second.first.fed_id == id; };
    maperase_if(unknown_publications, ck);
    maperase_if(unknown_endpoints, ck);
    maperase_if(unknown_filters, ck);
    maperase_if(unknown_inputs, ck);
}

void UnknownHandleManager::clearUnknownsIf(
    const std::function<bool(const std::string& name, InterfaceType, TargetInfo)>& cfunc)
{
    InterfaceType type = InterfaceType::PUBLICATION;
    auto ck = [&cfunc, &type](const auto& it) { return cfunc(it.first, type, it.second); };
    maperase_if(unknown_publications, ck);
    type = InterfaceType::ENDPOINT;
    maperase_if(unknown_endpoints, ck);
    type = InterfaceType::FILTER;
    maperase_if(unknown_filters, ck);
    type = InterfaceType::INPUT;
    maperase_if(unknown_inputs, ck);
}

}  // namespace helics
