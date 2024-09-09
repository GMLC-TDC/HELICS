/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "basic_CoreTypes.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics {
struct EptInformation {
    GlobalHandle id;
    std::string key;
    std::string type;
    EptInformation() = default;
    EptInformation(GlobalHandle gid, std::string_view key_, std::string_view type_):
        id(gid), key(key_), type(type_)
    {
    }
};

/** data class defining the information about a filter*/
class FilterInfo {
  public:
    /** constructor from all fields*/
    FilterInfo(GlobalBrokerId core_id_,
               InterfaceHandle handle_,
               std::string_view key_,
               std::string_view type_in_,
               std::string_view type_out_,
               bool destFilter_):
        core_id(core_id_), handle(handle_), key(key_), inputType(type_in_), outputType(type_out_),
        dest_filter(destFilter_)
    {
    }
    const GlobalBrokerId core_id;  //!< id of the core that manages the filter
    const InterfaceHandle handle;  //!< id handle of the filter

    const std::string key;  //!< the identifier of the filter
    const std::string inputType;  //!< the type of data for the filter
    const std::string outputType;  //!< the outputType of data of the filter
    std::vector<GlobalHandle> sourceTargets;
    std::vector<GlobalHandle> destTargets;
    const bool dest_filter = false;  //! indicator that the filter is a destination filter
    bool cloning = false;  //!< indicator that the filter is a cloning filter
    uint16_t flags = 0;  //!< flags for the filter
    // there is a 4 byte gap here
    std::shared_ptr<FilterOperator> filterOp;  //!< the callback operation of the filter
  private:
    std::vector<EptInformation>
        sourceEndpoints;  //!< information about the endpoints for source filters
    std::vector<EptInformation>
        destEndpoints;  //!< information about the endpoints for dest Filters
    mutable std::string sourceEpts;
    mutable std::string destEpts;

  public:
    /** add a target target*/
    void addDestinationEndpoint(GlobalHandle dest,
                                std::string_view destName,
                                std::string_view destType);
    /** add a source to an endpoint*/
    void addSourceEndpoint(GlobalHandle dest,
                           std::string_view sourceName,
                           std::string_view sourceType);
    /** remove a target from connection*/
    void removeTarget(GlobalHandle targetId);
    /** get a string with the names of the source endpoints*/
    const std::string& getSourceEndpoints() const;
    /** get a string with the names of the destination endpoints*/
    const std::string& getDestinationEndpoints() const;
};
}  // namespace helics
