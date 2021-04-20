/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "basic_core_types.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics {
/** data class defining the information about a filter*/
class FilterInfo {
  public:
    /** constructor from all fields*/
    FilterInfo(global_broker_id core_id_,
               interface_handle handle_,
               const std::string& key_,
               const std::string& type_in_,
               const std::string& type_out_,
               bool destFilter_):
        core_id(core_id_),
        handle(handle_), key(key_), inputType(type_in_), outputType(type_out_),
        dest_filter(destFilter_)
    {
    }
    const global_broker_id core_id;  //!< id of the core that manages the filter
    const interface_handle handle;  //!< id handle of the filter

    const std::string key;  //!< the identifier of the filter
    const std::string inputType;  //!< the type of data for the filter
    const std::string outputType;  //!< the outputType of data of the filter
    const bool dest_filter = false;  //! indicator that the filter is a destination filter
    bool cloning = false;  //!< indicator that the filter is a cloning filter
    uint16_t flags = 0;  //!< flags for the filter
    // there is a 4 byte gap here
    std::shared_ptr<FilterOperator> filterOp;  //!< the callback operation of the filter

    std::vector<global_handle> sourceTargets;
    std::vector<global_handle> destTargets;
    /** remove a target from interface with the filter*/
    void removeTarget(global_handle targetToRemove);
};
}  // namespace helics
