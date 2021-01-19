/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "FilterFederate.hpp"

namespace helics {

/** process any filter or route the message*/
void FilterFederate::processMessageFilter(ActionMessage& cmd) {}
/** process a filter message return*/
void FilterFederate::processFilterReturn(ActionMessage& cmd) {}
/** process a destination filter message return*/
void FilterFederate::processDestFilterReturn(ActionMessage& command) {}

FilterInfo* FilterFederate::createFilter(global_broker_id dest,
                         interface_handle handle,
                         const std::string& key,
                         const std::string& type_in,
                         const std::string& type_out,
                         bool cloning)
    {
    auto filt = std::make_unique<FilterInfo>((dest == parent_broker_id) ? fedID : dest,
                                             handle,
                                             key,
                                             type_in,
                                             type_out,
                                             false);

    auto* retTarget = filt.get();
    auto actualKey = key;
    retTarget->cloning = cloning;
    if (actualKey.empty()) {
        actualKey = "sFilter_";
        actualKey.append(std::to_string(handle.baseValue()));
    }
    if (filt->core_id == fedID) {
        filters.insert(actualKey, global_handle(dest, filt->handle), std::move(filt));
    } else {
        actualKey.push_back('_');
        actualKey.append(std::to_string(filt->core_id.baseValue()));
        filters.insert(actualKey, {filt->core_id, filt->handle}, std::move(filt));
    }

    return retTarget;
}

}  // namespace helics
