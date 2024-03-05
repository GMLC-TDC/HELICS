/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "CoreTypes.hpp"
#include "GlobalFederateId.hpp"

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
    using TargetInfo = std::pair<GlobalHandle, uint16_t>;

  private:
    std::unordered_multimap<std::string, TargetInfo>
        unknown_publications;  //!< map of all unknown publications
    std::unordered_multimap<std::string, TargetInfo>
        unknown_endpoints;  //!< map of all unknown endpoints
    std::unordered_multimap<std::string, TargetInfo> unknown_inputs;  //!< map of all unknown inputs
    std::unordered_multimap<std::string, TargetInfo>
        unknown_filters;  //!< map of all unknown filters

    std::unordered_multimap<std::string, std::string>
        unknown_links;  //!< map where links on either side is not known
    std::unordered_multimap<std::string, std::string>
        unknown_endpoint_links;  //!< map where endpoint links on either side is not known
    std::unordered_multimap<std::string, std::string>
        unknown_src_filters;  //!< map connecting source filters to endpoints
    std::unordered_multimap<std::string, std::string>
        unknown_dest_filters;  //!< map connecting destination filters to endpoints

    std::unordered_multimap<std::string, TargetInfo>
        reconnectable_publications;  //!< map of all reconnectable publications
    std::unordered_multimap<std::string, TargetInfo>
        reconnectable_endpoints;  //!< map of all reconnectable endpoints
    std::unordered_multimap<std::string, TargetInfo>
        reconnectable_inputs;  //!< map of all reconnectable inputs
    std::unordered_multimap<std::string, TargetInfo>
        reconnectable_filters;  //!< map of all reconnectable filters
  public:
    /** default constructor*/
    UnknownHandleManager() = default;
    /** add a missing Publication*/
    void addUnknownPublication(std::string_view key, GlobalHandle target, uint16_t flags);
    /** add a missing Input*/
    void addUnknownInput(std::string_view key, GlobalHandle target, uint16_t flags);
    /** add a missing endpoint*/
    void addUnknownEndpoint(std::string_view key, GlobalHandle target, uint16_t flags);
    /** add a missing source filter*/
    void addUnknownFilter(std::string_view key, GlobalHandle target, uint16_t flags);

    /** add a reconnectable Publication*/
    void addReconnectablePublication(std::string_view key, GlobalHandle target, uint16_t flags);
    /** add a reconnectable Input*/
    void addReconnectableInput(std::string_view key, GlobalHandle target, uint16_t flags);
    /** add a reconnectable endpoint*/
    void addReconnectableEndpoint(std::string_view key, GlobalHandle target, uint16_t flags);
    /** add a reconnectable filter*/
    void addReconnectableFilter(std::string_view key, GlobalHandle target, uint16_t flags);

    /** add a data link where neither side is known*/
    void addDataLink(std::string_view source, std::string_view target);
    /** add an endpoint link where neither side is known*/
    void addEndpointLink(std::string_view source, std::string_view target);

    void addSourceFilterLink(std::string_view filter, std::string_view endpoint);
    void addDestinationFilterLink(std::string_view filter, std::string_view endpoint);
    /** specify a found input*/
    std::vector<TargetInfo> checkForInputs(const std::string& newInput) const;
    /** specify a found input*/
    std::vector<TargetInfo> checkForPublications(const std::string& newPublication) const;
    /** specify a found input*/
    std::vector<TargetInfo> checkForEndpoints(const std::string& newEndpoint) const;

    /** specify a found Source Filter*/
    std::vector<TargetInfo> checkForFilters(const std::string& newFilter) const;

    /** specify a found input*/
    std::vector<TargetInfo> checkForReconnectionInputs(const std::string& newInput) const;
    /** specify a found input*/
    std::vector<TargetInfo>
        checkForReconnectionPublications(const std::string& newPublication) const;
    /** specify a found input*/
    std::vector<TargetInfo> checkForReconnectionEndpoints(const std::string& newEndpoint) const;

    /** specify a found Source Filter*/
    std::vector<TargetInfo> checkForReconnectionFilters(const std::string& newFilter) const;

    /** specify found data links*/
    std::vector<std::string> checkForLinks(const std::string& newSource) const;

    /** specify found endpoint links*/
    std::vector<std::string> checkForEndpointLinks(const std::string& newSource) const;

    std::vector<std::string> checkForFilterSourceTargets(const std::string& newFilter) const;
    std::vector<std::string> checkForFilterDestTargets(const std::string& newFilter) const;
    /** specify a found input*/
    void clearInput(const std::string& newInput);
    /** specify a found publication*/
    void clearPublication(const std::string& newPublication);
    /** specify a found endpoint*/
    void clearEndpoint(const std::string& newEndpoint);

    /** specify a found source filter*/
    void clearFilter(const std::string& newFilter);
    /** clear all unknowns belonging to a certain federate*/
    void clearFederateUnknowns(GlobalFederateId id);
    /** clear unknowns if callback is true*/
    void clearUnknownsIf(
        const std::function<bool(const std::string& name, InterfaceType, TargetInfo)>& cfunc);
    /** check if there are any unknowns remaining*/
    bool hasUnknowns() const;

    /** check if there are any unknowns remaining that do not specify that they are optional*/
    bool hasNonOptionalUnknowns() const;

    /** check if there are any unknowns remaining that specify they are required*/
    bool hasRequiredUnknowns() const;
    /** run a callback for each requiredUnknown
    @param cfunc a callback function with the signature of the name of the required interface a
    character with the interface type and the global handle.
    */
    void processRequiredUnknowns(
        const std::function<void(const std::string& name, InterfaceType, TargetInfo)>& cfunc) const;

    /** run a callback for each non optional Unknown
    @param cfunc a callback function with the signature of the name of the required interface a
    character with the interface type and the global handle.
    */
    void processNonOptionalUnknowns(
        const std::function<void(const std::string& name, InterfaceType, TargetInfo)>& cfunc) const;

    /** run a callback for each Unknown
    @param cfunc a callback function with the signature of the name of the required interface, a
    character with the interface type and the global handle.
    */
    void processUnknowns(
        const std::function<void(const std::string& name, InterfaceType, TargetInfo)>& cfunc) const;

    /** run a callback for each Unknown Link
    @param cfunc a callback function with the signature of the name of the interface,
    the interface type, the name of the target, and type of the target.
    */
    void processUnknownLinks(const std::function<void(const std::string& origin,
                                                      InterfaceType,
                                                      const std::string& target,
                                                      InterfaceType)>& cfunc) const;
};

}  // namespace helics
