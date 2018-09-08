/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "UnknownHandleManager.hpp"

namespace helics
{
/** add a missingPublication*/
void UnknownHandleManager::addUnknownPublication(const std::string &key, global_handle target, uint16_t flags)
{
	unknown_publications.emplace(key, std::make_pair(target,flags));
}
/** add a missingPublication*/
void UnknownHandleManager::addUnknownInput(const std::string &key, global_handle target, uint16_t flags)
{
    unknown_inputs.emplace(key, std::make_pair(target, flags));
}

/** add a missing destination endpoint*/
void UnknownHandleManager::addUnknownEndpoint(const std::string &key, global_handle target, uint16_t flags)
{
    unknown_endpoints.emplace(key, std::make_pair(target, flags));
}
/** add a missing filter*/
void UnknownHandleManager::addUnknownFilter(const std::string &key, global_handle target, uint16_t flags)
{
    unknown_filters.emplace(key, std::make_pair(target, flags));
}


/** specify a found input*/
std::vector<UnknownHandleManager::targetInfo> UnknownHandleManager::checkForInputs(const std::string &newInput)
{
    std::vector<targetInfo> targets;
    auto rp = unknown_inputs.equal_range(newInput);
    if (rp.first != unknown_inputs.end())
    {
        auto it = rp.first;
        while (it != rp.second)
        {
            targets.push_back(it->second);
            ++it;
        }
    }
    return targets;
}
/** specify a found input*/
std::vector<UnknownHandleManager::targetInfo> UnknownHandleManager::checkForPublications(const std::string &newPublication)
    {
        std::vector<targetInfo> targets;
        auto rp = unknown_publications.equal_range(newPublication);
        if (rp.first != unknown_publications.end())
        {
            auto it = rp.first;
            while (it != rp.second)
            {
                targets.push_back(it->second);
                ++it;
            }
        }
        return targets;
    }

/** specify a found input*/
std::vector<UnknownHandleManager::targetInfo> UnknownHandleManager::checkForEndpoints(const std::string &newEndpoint) {
    std::vector<targetInfo> targets;
    auto rp = unknown_endpoints.equal_range(newEndpoint);
    if (rp.first != unknown_endpoints.end())
    {
        auto it = rp.first;
        while (it != rp.second)
        {
            targets.push_back(it->second);
            ++it;
        }
    }
    return targets;
}

/** specify a found input*/
std::vector<UnknownHandleManager::targetInfo> UnknownHandleManager::checkForFilters(const std::string &newFilter)
{
    std::vector<targetInfo> targets;
    auto rp = unknown_filters.equal_range(newFilter);
    if (rp.first != unknown_filters.end())
    {
        auto it = rp.first;
        while (it != rp.second)
        {
            targets.push_back(it->second);
            ++it;
        }
    }
    return targets;
}

bool UnknownHandleManager::hasUnknowns() const
{
    return (!(unknown_publications.empty() && unknown_endpoints.empty() && unknown_inputs.empty() && unknown_filters.empty()));
}

/** specify a found input*/
void UnknownHandleManager::clearInput(const std::string &newInput)
{
    unknown_inputs.erase(newInput);
}

/** specify a found input*/
void UnknownHandleManager::clearPublication(const std::string &newPublication)
{
    unknown_publications.erase(newPublication);
}
/** specify a found input*/
void UnknownHandleManager::clearEndpoint(const std::string &newEndpoint)
{
    unknown_endpoints.erase(newEndpoint);
}

/** specify a found input*/
void UnknownHandleManager::clearFilter(const std::string &newFilter)
{
    unknown_filters.erase(newFilter);
}

}  // namespace helics
