/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "global_federate_id.hpp"

#include <vector>
namespace helics {
class FilterInfo;
/** data class to manage the ordering of filter operations for an endpoint
@details thread safety for this class must be managed externally
 */
class FilterCoordinator {
  public:
    std::vector<FilterInfo*> sourceFilters;  //!< ordered set of source operators
    FilterInfo* destFilter = nullptr;  //!< the destination operator handle

    std::vector<FilterInfo*>
        allSourceFilters;  //!< storage for all the source filters before sorting
    std::vector<FilterInfo*> cloningDestFilters;  //!< storage for cloning destination filters
    bool hasSourceFilters = false;  //!< indicator that an endpoint has source filters
    bool hasDestFilters = false;  //!< indicator that an endpoint has a destination filter
    int ongoingSourceTransactions =
        0;  //!< counter for the number of filtered message returns expected
    int ongoingDestTransactions =
        0;  //!< counter for the number of filtered message returns expected on Destination
    /** make a filter as closed within the coordinator*/
    void closeFilter(global_handle filt);
};
}  // namespace helics
