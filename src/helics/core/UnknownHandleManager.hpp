/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once
#include "federate_id.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace helics
{
/** class for managing a coordinating the different types of handles used in helics
@details this class is not designed to be thread safe that would require a wrapper around it
*/
class UnknownHandleManager
{
  public:
    using targetInfo = std::pair<global_handle, uint16_t>;

  private:
    std::unordered_multimap<std::string, targetInfo> unknown_publications;  //!< map of all unknown publications
    std::unordered_multimap<std::string, targetInfo> unknown_endpoints;  //!< map of all unknown endpoints
    std::unordered_multimap<std::string, targetInfo> unknown_inputs;  //!< map of all unknown endpoints
    std::unordered_multimap<std::string, targetInfo> unknown_filters;  //!< map of all unknown filters
    std::unordered_multimap<std::string, std::string>
      unknown_links;  //!< map where links on either side is not known
  public:
    /** default constructor*/
    UnknownHandleManager () = default;
    /** add a missingPublication*/
    void addUnknownPublication (const std::string &key, global_handle target, uint16_t flags);
    /** add a missingPublication*/
    void addUnknownInput (const std::string &key, global_handle target, uint16_t flags);
    /** add a missing source endpoint*/
    void addUnknownEndpoint (const std::string &key, global_handle target, uint16_t flags);

    /** add a missing source filter*/
    void addUnknownFilter (const std::string &key, global_handle target, uint16_t flags);
    /** add a data link where neither side is known*/
    void addDataLink (const std::string &source, const std::string &target);
    /** specify a found input*/
    std::vector<targetInfo> checkForInputs (const std::string &newInput);
    /** specify a found input*/
    std::vector<targetInfo> checkForPublications (const std::string &newPublication);
    /** specify a found input*/
    std::vector<targetInfo> checkForEndpoints (const std::string &newEndpoint);

    /** specify a found Source Filter*/
    std::vector<targetInfo> checkForFilters (const std::string &newFilter);

	/** specify found data links*/
    std::vector<std::string> checkForLinks (const std::string &newSource);
    /** specify a found input*/
    void clearInput (const std::string &newInput);
    /** specify a found input*/
    void clearPublication (const std::string &newPublication);
    /** specify a found input*/
    void clearEndpoint (const std::string &newEndpoint);

    /** specify a found source filter*/
    void clearFilter (const std::string &newFilter);

    /** check if there are any unknowns remaining*/
    bool hasUnknowns () const;
};

}  // namespace helics
