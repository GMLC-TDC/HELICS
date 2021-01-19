/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "global_federate_id.hpp"

#include <map>
#include <memory>
#include "gmlc/containers/DualMappedPointerVector.hpp"
#include "FilterCoordinator.hpp"
#include "FilterInfo.hpp"
#include "TimeCoordinator.hpp"

namespace helics {
    class FilterFederate
    {
  private:
        global_federate_id fedID;
    global_broker_id coreID;

        /// map of all local filters
        std::map<interface_handle, std::unique_ptr<FilterCoordinator>>
            filterCoord;  
        // The interface_handle used is here is usually referencing an endpoint
        /// storage for all the filters
        gmlc::containers::DualMappedPointerVector<FilterInfo,
                                                  std::string,
                                                  global_handle>
            filters;

        TimeCoordinator coord;

      public:
        /** process any filter or route the message*/
        void processMessageFilter(ActionMessage& cmd);
        /** process a filter message return*/
        void processFilterReturn(ActionMessage& cmd);
        /** process a destination filter message return*/
        void processDestFilterReturn(ActionMessage& command);
        /** create a filter */
        FilterInfo* createFilter(global_broker_id dest,
                                 interface_handle handle,
                                 const std::string& key,
                                 const std::string& type_in,
                                 const std::string& type_out,
                                 bool cloning);

    };
}
