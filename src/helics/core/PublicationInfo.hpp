/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "global_federate_id.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace helics {
/** data class containing the information about a publication*/
class PublicationInfo {
  public:
    /** constructor from the basic information*/
    PublicationInfo(
        global_handle pid,
        const std::string& pkey,
        const std::string& ptype,
        const std::string& punits):
        id(pid),
        key(pkey), type(ptype), units(punits)
    {
    }
    const global_handle id; //!< the identifier for the containing federate
    std::vector<global_handle> subscribers; //!< container for all the subscribers of a publication
    const std::string key; //!< the key identifier for the publication
    const std::string type; //!< the type of the publication data
    const std::string units; //!< the units of the publication data
    std::string data; //!< the most recent publication data
    bool has_update = false; //!< indicator that the publication has updates
    bool only_update_on_change = false;
    bool required = false; //!< indicator that it is required to be output someplace
    bool buffer_data = false;
    bool single_destination =
        false; //!< indicator that the publication should only have a single destination
    /** check the value if it is the same as the most recent data and if changed, store it*/
    bool CheckSetValue(const char* checkData, uint64_t len);
    /** remove a subscriber*/
    void removeSubscriber(global_handle subscriberToRemove);
};
} // namespace helics
