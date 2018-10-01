/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "Core.hpp"
#include "helics-time.hpp"
#include "helics/helics-config.h"

#include <cstdint>
#include <map>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

namespace helics
{
/** data class containing the information about a publication*/
class PublicationInfo
{
  public:
    /** constructor from the basic information*/
    PublicationInfo (global_handle id_, 
                     const std::string &key_,
                     const std::string &type_,
                     const std::string &units_)
        :  id (id_), key (key_), type (type_), units (units_)
    {
    }
    const global_handle id;  //!< the identifier for the containing federate
    std::vector<global_handle>
      subscribers;  //!< container for all the subscribers of a publication
    const std::string key;  //!< the key identifier for the publication
    const std::string type;  //!< the type of the publication data
    const std::string units;  //!< the units of the publication data
    std::string data;  //!< the most recent publication data
    bool has_update = false;  //!< indicator that the publication has updates
    bool only_update_on_change = false;
    bool required = false; //!< indicator that it is required to be output someplace

    /** check the value if it is the same as the most recent data and if changed store it*/
    bool CheckSetValue (const char *checkData, uint64_t len);
};
}  // namespace helics
