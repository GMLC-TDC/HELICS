/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
*/
#pragma once

#include "Core.hpp"

#include <vector>
namespace helics
{
class FilterInfo;
/** data class to manage the ordering of filter operations for an endpoint
@details thread safety for this class must be managed externally
 */
class FilterCoordinator
{
  public:
	std::vector<FilterInfo *> sourceFilters; //!< ordered set of source operators
    FilterInfo *destFilter = nullptr;  //!< the destination operator handle

	std::vector<FilterInfo *> allSourceFilters; //!< storage for all the source filters before sorting
    bool hasSourceFilter = false;  //!< indicator that an endpoint has source filters
    bool hasDestFilter = false;  //!< indicator that an endpoint has a destination filter
    int ongoingTransactions = 0;  //!< counter for the number of filtered message returns expected
};
} // namespace helics

