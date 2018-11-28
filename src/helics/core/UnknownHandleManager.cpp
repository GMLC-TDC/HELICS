/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "UnknownHandleManager.hpp"
#include "flagOperations.hpp"

namespace helics
{
/** add a missingPublication*/
void UnknownHandleManager::addUnknownPublication (const std::string &key, global_handle target, uint16_t flags)
{
    unknown_publications.emplace (key, std::make_pair (target, flags));
}
/** add a missingPublication*/
void UnknownHandleManager::addUnknownInput (const std::string &key, global_handle target, uint16_t flags)
{
    unknown_inputs.emplace (key, std::make_pair (target, flags));
}

/** add a missing destination endpoint*/
void UnknownHandleManager::addUnknownEndpoint (const std::string &key, global_handle target, uint16_t flags)
{
    unknown_endpoints.emplace (key, std::make_pair (target, flags));
}
/** add a missing filter*/
void UnknownHandleManager::addUnknownFilter (const std::string &key, global_handle target, uint16_t flags)
{
    unknown_filters.emplace (key, std::make_pair (target, flags));
}

void UnknownHandleManager::addDataLink (const std::string &source, const std::string &target)
{
    unknown_links.emplace (source, target);
}

void UnknownHandleManager::addSourceFilterLink (const std::string &filter, const std::string &endpoint)
{
    unknown_src_filters.emplace (filter, endpoint);
}

void UnknownHandleManager::addDestinationFilterLink (const std::string &filter, const std::string &endpoint)
{
    unknown_dest_filters.emplace (filter, endpoint);
}

static auto getTargets (const std::unordered_multimap<std::string, UnknownHandleManager::targetInfo> &tmap,
                        const std::string &target)
{
    std::vector<UnknownHandleManager::targetInfo> targets;
    auto rp = tmap.equal_range (target);
    if (rp.first != tmap.end ())
    {
        auto it = rp.first;
        while (it != rp.second)
        {
            targets.push_back (it->second);
            ++it;
        }
    }
    return targets;
}

static auto getTargets (const std::unordered_multimap<std::string, std::string> &tmap, const std::string &target)
{
    std::vector<std::string> targets;
    auto rp = tmap.equal_range (target);
    if (rp.first != tmap.end ())
    {
        auto it = rp.first;
        while (it != rp.second)
        {
            targets.push_back (it->second);
            ++it;
        }
    }
    return targets;
}

/** specify a found input*/
std::vector<UnknownHandleManager::targetInfo>
UnknownHandleManager::checkForInputs (const std::string &newInput) const
{
    return getTargets (unknown_inputs, newInput);
}
/** specify a found input*/
std::vector<UnknownHandleManager::targetInfo>
UnknownHandleManager::checkForPublications (const std::string &newPublication) const
{
    return getTargets (unknown_publications, newPublication);
}

std::vector<std::string> UnknownHandleManager::checkForLinks (const std::string &newSource) const
{
    return getTargets (unknown_links, newSource);
}

/** specify a found input*/
std::vector<UnknownHandleManager::targetInfo>
UnknownHandleManager::checkForEndpoints (const std::string &newEndpoint) const
{
    return getTargets (unknown_endpoints, newEndpoint);
}

/** specify a found input*/
std::vector<UnknownHandleManager::targetInfo>
UnknownHandleManager::checkForFilters (const std::string &newFilter) const
{
    return getTargets (unknown_filters, newFilter);
}

std::vector<std::string> UnknownHandleManager::checkForFilterSourceTargets (const std::string &newFilter) const
{
    return getTargets (unknown_src_filters, newFilter);
}

std::vector<std::string> UnknownHandleManager::checkForFilterDestTargets (const std::string &newFilter) const
{
    return getTargets (unknown_dest_filters, newFilter);
}

bool UnknownHandleManager::hasUnknowns () const
{
    return (!(unknown_publications.empty () && unknown_endpoints.empty () && unknown_inputs.empty () &&
              unknown_filters.empty () && unknown_links.empty () && unknown_dest_filters.empty () &&
              unknown_src_filters.empty ()));
}

bool UnknownHandleManager::hasNonOptionalUnknowns () const
{
    if (!(unknown_links.empty () && unknown_dest_filters.empty () && unknown_src_filters.empty ()))
    {
        return true;
    }
    for (auto &upub : unknown_publications)
    {
        if ((upub.second.second & make_flags (optional_flag)) != 0)
        {
            continue;
        }
        return true;
    }
    for (auto &uept : unknown_endpoints)
    {
        if ((uept.second.second & make_flags (optional_flag)) != 0)
        {
            continue;
        }
        return true;
    }
    for (auto &uinp : unknown_inputs)
    {
        if ((uinp.second.second & make_flags (optional_flag)) != 0)
        {
            continue;
        }
        return true;
    }

    for (auto &ufilt : unknown_filters)
    {
        if ((ufilt.second.second & make_flags (optional_flag)) != 0)
        {
            continue;
        }
        return true;
    }
    return false;
}

bool UnknownHandleManager::hasRequiredUnknowns () const
{
    if (!(unknown_links.empty () && unknown_dest_filters.empty () && unknown_src_filters.empty ()))
    {
        return true;
    }
    for (auto &upub : unknown_publications)
    {
        if ((upub.second.second & make_flags (optional_flag)) != 0)
        {
            continue;
        }
        return true;
    }
    for (auto &uept : unknown_endpoints)
    {
        if ((uept.second.second & make_flags (optional_flag)) != 0)
        {
            continue;
        }
        return true;
    }
    for (auto &uinp : unknown_inputs)
    {
        if ((uinp.second.second & make_flags (optional_flag)) != 0)
        {
            continue;
        }
        return true;
    }

    for (auto &ufilt : unknown_filters)
    {
        if ((ufilt.second.second & make_flags (optional_flag)) != 0)
        {
            continue;
        }
        return true;
    }
    return false;
}

/** specify a found input*/
void UnknownHandleManager::clearInput (const std::string &newInput) { unknown_inputs.erase (newInput); }

/** specify a found input*/
void UnknownHandleManager::clearPublication (const std::string &newPublication)
{
    unknown_publications.erase (newPublication);
    unknown_links.erase (newPublication);
}
/** specify a found input*/
void UnknownHandleManager::clearEndpoint (const std::string &newEndpoint)
{
    unknown_endpoints.erase (newEndpoint);
}

/** specify a found input*/
void UnknownHandleManager::clearFilter (const std::string &newFilter)
{
    unknown_filters.erase (newFilter);
    unknown_src_filters.erase (newFilter);
    unknown_dest_filters.erase (newFilter);
}

}  // namespace helics
