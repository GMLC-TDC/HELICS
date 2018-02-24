/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/
#pragma once

#include "../common/blocking_queue.h"
#include "Core.hpp"
#include "helics-time.hpp"
#include "helics/helics-config.h"

#include <map>
#include <mutex>
#include <thread>
#include <vector>

namespace helics
{
/** data class defining the information about a filter*/
class FilterInfo
{
  public:
    /** constructor from all fields*/
    FilterInfo (Core::federate_id_t fed_id_,
                Core::handle_id_t handle_,
                const std::string &key_,
                const std::string &target_,
                const std::string &type_in_,
                const std::string &type_out_,
                bool destFilter_)
        : fed_id (fed_id_), handle (handle_), key (key_), filterTarget (target_), inputType (type_in_),
          outputType (type_out_), dest_filter (destFilter_)
    {
    }
    const Core::federate_id_t fed_id = invalid_fed_id;  //!< id of the core that manages the filter
    const Core::handle_id_t handle = invalid_handle;  //!< id handle of the filter

    const std::string key;  //!< the identifier of the filter
    const std::string filterTarget;  //!< the target endpoint name of the filter
    const std::string inputType;  //!< the type of data for the filter
    const std::string outputType;  //!< the outputType of data of the filter
    const bool dest_filter = false;  //! indicator that the filter is a destination filter
    // there is a 7 byte gap here
    std::shared_ptr<FilterOperator> filterOp;  //!< the callback operation of the filter

    std::pair<Core::federate_id_t, Core::handle_id_t> target{
      invalid_fed_id, invalid_handle};  //!< the actual target information for the filter
};
}  // namespace helics