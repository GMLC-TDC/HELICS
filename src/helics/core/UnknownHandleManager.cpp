/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "UnknownHandleManager.hpp"

#include "flagOperations.hpp"

namespace helics {
/** add a missingPublication*/
void UnknownHandleManager::addUnknownPublication(const std::string& key,
                                                 global_handle target,
                                                 uint16_t flags)
{
    unknown_publications.emplace(key, std::make_pair(target, flags));
}
/** add a missingPublication*/
void UnknownHandleManager::addUnknownInput(const std::string& key,
                                           global_handle target,
                                           uint16_t flags)
{
    unknown_inputs.emplace(key, std::make_pair(target, flags));
}

/** add a missing destination endpoint*/
void UnknownHandleManager::addUnknownEndpoint(const std::string& key,
                                              global_handle target,
                                              uint16_t flags)
{
    unknown_endpoints.emplace(key, std::make_pair(target, flags));
}
/** add a missing filter*/
void UnknownHandleManager::addUnknownFilter(const std::string& key,
                                            global_handle target,
                                            uint16_t flags)
{
    unknown_filters.emplace(key, std::make_pair(target, flags));
}

void UnknownHandleManager::addDataLink(const std::string& source, const std::string& target)
{
    unknown_links.emplace(source, target);
}

void UnknownHandleManager::addSourceFilterLink(const std::string& filter,
                                               const std::string& endpoint)
{
    unknown_src_filters.emplace(filter, endpoint);
}

void UnknownHandleManager::addDestinationFilterLink(const std::string& filter,
                                                    const std::string& endpoint)
{
    unknown_dest_filters.emplace(filter, endpoint);
}

static auto
    getTargets(const std::unordered_multimap<std::string, UnknownHandleManager::targetInfo>& tmap,
               const std::string& target)
{
    std::vector<UnknownHandleManager::targetInfo> targets;
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
std::vector<UnknownHandleManager::targetInfo>
    UnknownHandleManager::checkForInputs(const std::string& newInput) const
{
    return getTargets(unknown_inputs, newInput);
}
/** specify a found input*/
std::vector<UnknownHandleManager::targetInfo>
    UnknownHandleManager::checkForPublications(const std::string& newPublication) const
{
    return getTargets(unknown_publications, newPublication);
}

std::vector<std::string> UnknownHandleManager::checkForLinks(const std::string& newSource) const
{
    return getTargets(unknown_links, newSource);
}

/** specify a found input*/
std::vector<UnknownHandleManager::targetInfo>
    UnknownHandleManager::checkForEndpoints(const std::string& newEndpoint) const
{
    return getTargets(unknown_endpoints, newEndpoint);
}

/** specify a found input*/
std::vector<UnknownHandleManager::targetInfo>
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
              unknown_filters.empty() && unknown_links.empty() && unknown_dest_filters.empty() &&
              unknown_src_filters.empty()));
}

bool UnknownHandleManager::hasNonOptionalUnknowns() const
{
    if (!(unknown_links.empty() && unknown_dest_filters.empty() && unknown_src_filters.empty())) {
        return true;
    }
    for (auto& upub : unknown_publications) {
        if ((upub.second.second & make_flags(optional_flag)) != 0) {
            continue;
        }
        return true;
    }
    for (auto& uept : unknown_endpoints) {
        if ((uept.second.second & make_flags(optional_flag)) != 0) {
            continue;
        }
        return true;
    }
    for (auto& uinp : unknown_inputs) {
        if ((uinp.second.second & make_flags(optional_flag)) != 0) {
            continue;
        }
        return true;
    }

    for (auto& ufilt : unknown_filters) {
        if ((ufilt.second.second & make_flags(optional_flag)) != 0) {
            continue;
        }
        return true;
    }
    return false;
}

bool UnknownHandleManager::hasRequiredUnknowns() const
{
    for (auto& upub : unknown_publications) {
        if ((upub.second.second & make_flags(required_flag)) != 0) {
            return true;
        }
    }
    for (auto& uept : unknown_endpoints) {
        if ((uept.second.second & make_flags(required_flag)) != 0) {
            return true;
        }
    }
    for (auto& uinp : unknown_inputs) {
        if ((uinp.second.second & make_flags(required_flag)) != 0) {
            return true;
        }
    }

    for (auto& ufilt : unknown_filters) {
        if ((ufilt.second.second & make_flags(required_flag)) != 0) {
            return true;
        }
    }
    return false;
}

void UnknownHandleManager::processNonOptionalUnknowns(
    std::function<void(const std::string&, char, global_handle handle)> cfunc) const
{
    for (auto& upub : unknown_publications) {
        if ((upub.second.second & make_flags(optional_flag)) != 0) {
            continue;
        }
        cfunc(upub.first, 'p', upub.second.first);
    }
    for (auto& uept : unknown_endpoints) {
        if ((uept.second.second & make_flags(optional_flag)) != 0) {
            continue;
        }
        cfunc(uept.first, 'e', uept.second.first);
    }
    for (auto& uinp : unknown_inputs) {
        if ((uinp.second.second & make_flags(optional_flag)) != 0) {
            continue;
        }
        cfunc(uinp.first, 'i', uinp.second.first);
    }

    for (auto& ufilt : unknown_filters) {
        if ((ufilt.second.second & make_flags(optional_flag)) != 0) {
            continue;
        }
        cfunc(ufilt.first, 'f', ufilt.second.first);
    }
}

void UnknownHandleManager::processRequiredUnknowns(
    std::function<void(const std::string&, char, global_handle handle)> cfunc) const
{
    for (auto& upub : unknown_publications) {
        if ((upub.second.second & make_flags(required_flag)) != 0) {
            cfunc(upub.first, 'p', upub.second.first);
        }
    }
    for (auto& uept : unknown_endpoints) {
        if ((uept.second.second & make_flags(required_flag)) != 0) {
            cfunc(uept.first, 'e', uept.second.first);
        }
    }
    for (auto& uinp : unknown_inputs) {
        if ((uinp.second.second & make_flags(required_flag)) != 0) {
            cfunc(uinp.first, 'i', uinp.second.first);
        }
    }

    for (auto& ufilt : unknown_filters) {
        if ((ufilt.second.second & make_flags(required_flag)) != 0) {
            cfunc(ufilt.first, 'f', ufilt.second.first);
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
}

/** specify a found input*/
void UnknownHandleManager::clearFilter(const std::string& newFilter)
{
    unknown_filters.erase(newFilter);
    unknown_src_filters.erase(newFilter);
    unknown_dest_filters.erase(newFilter);
}

void UnknownHandleManager::clearFederateUnknowns(global_federate_id id)
{
    for (auto it = std::begin(unknown_publications); it != std::end(unknown_publications);) {
        if (it->second.first.fed_id == id) {
            it = unknown_publications.erase(
                it);  // previously this was something like m_map.erase(it++);
        } else {
            ++it;
        }
    }
    for (auto it = std::begin(unknown_endpoints); it != std::end(unknown_endpoints);) {
        if (it->second.first.fed_id == id) {
            it = unknown_endpoints.erase(
                it);  // previously this was something like m_map.erase(it++);
        } else {
            ++it;
        }
    }
    for (auto it = std::begin(unknown_filters); it != std::end(unknown_filters);) {
        if (it->second.first.fed_id == id) {
            it =
                unknown_filters.erase(it);  // previously this was something like m_map.erase(it++);
        } else {
            ++it;
        }
    }
    for (auto it = std::begin(unknown_inputs); it != std::end(unknown_inputs);) {
        if (it->second.first.fed_id == id) {
            it = unknown_inputs.erase(it);  // previously this was something like m_map.erase(it++);
        } else {
            ++it;
        }
    }
}

}  // namespace helics
