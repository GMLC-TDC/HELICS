/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "FilterCoordinator.hpp"

#include "FilterInfo.hpp"
#include "flagOperations.hpp"

namespace helics {
void FilterCoordinator::closeFilter(global_handle filt)
{
    if (destFilter != nullptr) {
        if ((destFilter->core_id == filt.fed_id) && (destFilter->handle == filt.handle)) {
            setActionFlag(*destFilter, disconnected_flag);
        }
    }
    for (auto sFilt : allSourceFilters) {
        if ((sFilt->core_id == filt.fed_id) && (sFilt->handle == filt.handle)) {
            setActionFlag(*sFilt, disconnected_flag);
        }
    }
    for (auto cFilt : cloningDestFilters) {
        if ((cFilt->core_id == filt.fed_id) && (cFilt->handle == filt.handle)) {
            setActionFlag(*cFilt, disconnected_flag);
        }
    }
}
}  // namespace helics
