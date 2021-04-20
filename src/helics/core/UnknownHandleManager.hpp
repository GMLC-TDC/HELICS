/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "global_federate_id.hpp"

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace helics {
/** class for managing a coordinating the different types of handles used in helics
@details this class is not designed to be thread safe that would require a wrapper around it
*/
class UnknownHandleManager {
  public:
    using targetInfo = std::pair<global_handle, uint16_t>;

  private:
    std::unordered_multimap<std::string, targetInfo>
        unknown_publications;  //!< map of all unknown publications
    std::unordered_multimap<std::string, targetInfo>
        unknown_endpoints;  //!< map of all unknown endpoints
    std::unordered_multimap<std::string, targetInfo>
        unknown_inputs;  //!< map of all unknown endpoints
    std::unordered_multimap<std::string, targetInfo>
        unknown_filters;  //!< map of all unknown filters
    std::unordered_multimap<std::string, std::string>
        unknown_links;  //!< map where links on either side is not known
    std::unordered_multimap<std::string, std::string>
        unknown_src_filters;  //!< map connecting source filters to endpoints
    std::unordered_multimap<std::string, std::string>
        unknown_dest_filters;  //!< map connecting destination filters to endpoints
  public:
    /** default constructor*/
    UnknownHandleManager() = default;
    /** add a missingPublication*/
    void addUnknownPublication(const std::string& key, global_handle target, uint16_t flags);
    /** add a missingPublication*/
    void addUnknownInput(const std::string& key, global_handle target, uint16_t flags);
    /** add a missing source endpoint*/
    void addUnknownEndpoint(const std::string& key, global_handle target, uint16_t flags);

    /** add a missing source filter*/
    void addUnknownFilter(const std::string& key, global_handle target, uint16_t flags);
    /** add a data link where neither side is known*/
    void addDataLink(const std::string& source, const std::string& target);
    void addSourceFilterLink(const std::string& filter, const std::string& endpoint);
    void addDestinationFilterLink(const std::string& filter, const std::string& endpoint);
    /** specify a found input*/
    std::vector<targetInfo> checkForInputs(const std::string& newInput) const;
    /** specify a found input*/
    std::vector<targetInfo> checkForPublications(const std::string& newPublication) const;
    /** specify a found input*/
    std::vector<targetInfo> checkForEndpoints(const std::string& newEndpoint) const;

    /** specify a found Source Filter*/
    std::vector<targetInfo> checkForFilters(const std::string& newFilter) const;

    /** specify found data links*/
    std::vector<std::string> checkForLinks(const std::string& newSource) const;

    std::vector<std::string> checkForFilterSourceTargets(const std::string& newFilter) const;
    std::vector<std::string> checkForFilterDestTargets(const std::string& newFilter) const;
    /** specify a found input*/
    void clearInput(const std::string& newInput);
    /** specify a found input*/
    void clearPublication(const std::string& newPublication);
    /** specify a found input*/
    void clearEndpoint(const std::string& newEndpoint);

    /** specify a found source filter*/
    void clearFilter(const std::string& newFilter);
    /** clear all unknowns belonging to a certain federate*/
    void clearFederateUnknowns(global_federate_id id);
    /** check if there are any unknowns remaining*/
    bool hasUnknowns() const;

    /** check if there are any unknowns remaining that do not specify that they are optional*/
    bool hasNonOptionalUnknowns() const;

    /** check if there are any unknowns remaining that specify they are required*/
    bool hasRequiredUnknowns() const;
    /** run a callback for each requiredUnknown
    @param cfunc a callback function with the signature of the name of the required interface a
    character with the type 'p' for publication, 'i' for input, 'f' for filter, 'e' for endpoint and
    the global handle.
    */
    void processRequiredUnknowns(
        std::function<void(const std::string& name, char type, global_handle)> cfunc) const;

    /** run a callback for each non optional Unknown
    @param cfunc a callback function with the signature of the name of the required interface a
    character with the type 'p' for publication, 'i' for input, 'f' for filter, 'e' for endpoint and
    the global handle.
    */
    void processNonOptionalUnknowns(
        std::function<void(const std::string& name, char type, global_handle)> cfunc) const;
};

}  // namespace helics
