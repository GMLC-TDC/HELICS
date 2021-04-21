/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "helics-time.hpp"

#include <utility>
#include <vector>

namespace helics {
/** class defining some required information about the federate and any optional properties*/
class CoreFederateInfo {
  public:
    std::vector<std::pair<int, Time>> timeProps;  //!< container for the timeProperties
    std::vector<std::pair<int, int>> intProps;  //!< container for the integer properties
    std::vector<std::pair<int, bool>> flagProps;  //!< container for the binary flag options

    /** double overload for timeprops needs to be there since a literal double will convert to int
    more easily than to Time*/
    void setProperty(int propId, double propVal) { timeProps.emplace_back(propId, propVal); }
    void setProperty(int propId, int propVal) { intProps.emplace_back(propId, propVal); }
    void setProperty(int propId, Time propVal) { timeProps.emplace_back(propId, propVal); }
    /** set a flag property for a federate*/
    void setFlagOption(int flagId, bool propVal = true) { flagProps.emplace_back(flagId, propVal); }
};

}  // namespace helics
