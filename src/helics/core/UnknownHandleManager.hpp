/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once
#include "Core.hpp"
#include "helics-time.hpp"
#include <unordered_map>
#include <deque>
#include <utility>

namespace helics
{
/** class for managing a coordinating the different types of handles used in helics
@details this class is not designed to be thread safe that would require a wrapper around it
*/
class UnknownHandleManager
{
  private:

    std::unordered_multimap<std::string,global_handle> unknown_publications;  //!< map of all unknown publications
    std::unordered_multimap<std::string, global_handle> unknown_source_endpoints;  //!< map of all unknown endpoints
    std::unordered_multimap<std::string, global_handle> unknown_dest_endpoints;  //!< map of all unknown endpoints
    std::unordered_multimap<std::string, global_handle> unknown_inputs;  //!< map of all unknown endpoints
    std::unordered_multimap<std::string, global_handle> unknown_source_filters;  //!< map of all unknown source filters
    std::unordered_multimap<std::string, global_handle> unknown_dest_filters;  //!< map of all unknown dest filters
  public:
    /** default constructor*/
    UnknownHandleManager () = default;
    /** add a missingPublication*/
    void addUnknownPublication (const std::string &key, global_handle target);
    /** add a missingPublication*/
    void addUnknownInput(const std::string &key, global_handle target);
    /** add a missing source endpoint*/
    void addUnknownSourceEndpoint(const std::string &key, global_handle target);
    /** add a missing destination endpoint*/
    void addUnknownDestinationEndpoint(const std::string &key, global_handle target);
    /** add a missing source filter*/
    void addUnknownSourceFilter(const std::string &key, global_handle target);

    /** add a missing dest filter*/
    void addUnknownDestinationFilter(const std::string &key, global_handle target);
    /** specify a found input*/
    std::vector<global_handle> checkForInputs(const std::string &newInput);
    /** specify a found input*/
    std::vector<global_handle> checkForPublications(const std::string &newPublication);
    /** specify a found input*/
    std::vector<global_handle> checkForSourceEndpoints(const std::string &newEndpoint);
    /** specify a found input*/
    std::vector<global_handle> checkForDestinationEndpoints(const std::string &newEndpoint);

    /** specify a found Source Filter*/
    std::vector<global_handle> checkForSourceFilters(const std::string &newFilter);
    /** specify a found Destination Filter*/
    std::vector<global_handle> checkForDestinationFilters(const std::string &newFilter);

    /** specify a found input*/
    void clearInput(const std::string &newInput);
    /** specify a found input*/
    void clearPublication(const std::string &newPublication);
    /** specify a found input*/
    void clearSourceEndpoint(const std::string &newEndpoint);
    /** specify a found destination Endpoint*/
    void clearDestinationEndpoint(const std::string &newEndpoint);

    /** specify a found source filter*/
    void clearSourceFilter(const std::string &newFilter);
    /** specify a found source filter*/
    void clearDestinationFilter(const std::string &newFilter);
};

}  // namespace helics
