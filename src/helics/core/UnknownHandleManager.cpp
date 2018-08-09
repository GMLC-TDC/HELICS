/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "UnknownHandleManager.hpp"

namespace helics
{
/** add a missingPublication*/
void UnknownHandleManager::addUnknownPublication(const std::string &key, global_handle target)
{
    unknown_publications.emplace(key, target);
}
/** add a missingPublication*/
void UnknownHandleManager::addUnknownInput(const std::string &key, global_handle target)
{
    unknown_inputs.emplace(key, target);
}
/** add a missing source endpoint*/
void UnknownHandleManager::addUnknownSourceEndpoint(const std::string &key, global_handle target)
{
    unknown_source_endpoints.emplace(key, target);
}
/** add a missing destination endpoint*/
void UnknownHandleManager::addUnknownDestinationEndpoint(const std::string &key, global_handle target)
{
    unknown_dest_endpoints.emplace(key, target);
}
/** add a missing filter*/
void UnknownHandleManager::addUnknownSourceFilter(const std::string &key, global_handle target)
{
    unknown_source_filters.emplace(key, target);
}

/** add a missing destination filter*/
void UnknownHandleManager::addUnknownDestinationFilter(const std::string &key, global_handle target)
{
    unknown_dest_filters.emplace(key, target);
}
/** specify a found input*/
std::vector<global_handle> UnknownHandleManager::checkForInputs(const std::string &newInput)
{
    std::vector<global_handle> targets;
    auto rp = unknown_inputs.equal_range(newInput);
    if (rp.first != unknown_inputs.end())
    {
        auto it = rp.first;
        while (it != rp.second)
        {
            targets.push_back(it->second);
        }
    }
    return targets;
}
/** specify a found input*/
std::vector<global_handle> UnknownHandleManager::checkForPublications(const std::string &newPublication)
    {
        std::vector<global_handle> targets;
        auto rp = unknown_publications.equal_range(newPublication);
        if (rp.first != unknown_publications.end())
        {
            auto it = rp.first;
            while (it != rp.second)
            {
                targets.push_back(it->second);
            }
        }
        return targets;
    }

/** specify a found input*/
std::vector<global_handle> UnknownHandleManager::checkForSourceEndpoints(const std::string &newEndpoint) {
    std::vector<global_handle> targets;
    auto rp = unknown_source_endpoints.equal_range(newEndpoint);
    if (rp.first != unknown_source_endpoints.end())
    {
        auto it = rp.first;
        while (it != rp.second)
        {
            targets.push_back(it->second);
        }
    }
    return targets;
}
/** specify a found input*/
std::vector<global_handle> UnknownHandleManager::checkForDestinationEndpoints(const std::string &newEndpoint)
{
    std::vector<global_handle> targets;
    auto rp = unknown_dest_endpoints.equal_range(newEndpoint);
    if (rp.first != unknown_dest_endpoints.end())
    {
        auto it = rp.first;
        while (it != rp.second)
        {
            targets.push_back(it->second);
        }
    }
    return targets;
}

/** specify a found input*/
std::vector<global_handle> UnknownHandleManager::checkForSourceFilters(const std::string &newFilter)
{
    std::vector<global_handle> targets;
    auto rp = unknown_source_filters.equal_range(newFilter);
    if (rp.first != unknown_source_filters.end())
    {
        auto it = rp.first;
        while (it != rp.second)
        {
            targets.push_back(it->second);
        }
    }
    return targets;
}

std::vector<global_handle> UnknownHandleManager::checkForDestinationFilters(const std::string &newFilter)
{
    std::vector<global_handle> targets;
    auto rp = unknown_dest_filters.equal_range(newFilter);
    if (rp.first != unknown_dest_filters.end())
    {
        auto it = rp.first;
        while (it != rp.second)
        {
            targets.push_back(it->second);
        }
    }
    return targets;
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
void UnknownHandleManager::clearSourceEndpoint(const std::string &newEndpoint)
{
    unknown_source_endpoints.erase(newEndpoint);
}
/** specify a found input*/
void UnknownHandleManager::clearDestinationEndpoint(const std::string &newEndpoint)
{
    unknown_dest_endpoints.erase(newEndpoint);
}

/** specify a found input*/
void UnknownHandleManager::clearSourceFilter(const std::string &newFilter)
{
    unknown_source_filters.erase(newFilter);
}

void UnknownHandleManager::clearDestinationFilter(const std::string &newFilter)
{
    unknown_dest_filters.erase(newFilter);
}
}  // namespace helics
