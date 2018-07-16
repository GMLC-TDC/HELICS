/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "Core.hpp"
#include "helics-time.hpp"
#include "helics/helics-config.h"

#include <memory>
#include <utility>

namespace helics
{
/** data class defining the information about a filter*/
class FilterInfo
{
  public:
    /** constructor from all fields*/
    FilterInfo (global_broker_id_t core_id_,
                interface_handle handle_,
                const std::string &key_,
                const std::string &type_in_,
                const std::string &type_out_,
                bool destFilter_)
        : core_id (core_id_), handle (handle_), key (key_), inputType (type_in_),
          outputType (type_out_), dest_filter (destFilter_)
    {
    }
    const global_broker_id_t core_id;  //!< id of the core that manages the filter
    const interface_handle handle;  //!< id handle of the filter

    const std::string key;  //!< the identifier of the filter
    const std::string inputType;  //!< the type of data for the filter
    const std::string outputType;  //!< the outputType of data of the filter
    const bool dest_filter = false;  //! indicator that the filter is a destination filter
    bool cloning = false;  //!< indicator that the filter is a cloning filter
    // there is a 6 byte gap here
    std::shared_ptr<FilterOperator> filterOp;  //!< the callback operation of the filter

    std::vector<global_handle> sourceTargets;
    std::vector<global_handle> destTargets;
};
}  // namespace helics
