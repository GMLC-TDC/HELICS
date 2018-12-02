/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "helics-time.hpp"
#include <vector>

namespace helics
{
/** class defining some required information about the federate*/
class CoreFederateInfo
{
  public:
    std::vector<std::pair<int, Time>> timeProps;
    std::vector<std::pair<int, int>> intProps;
    std::vector<std::pair<int, bool>> flagProps;

  public:
    void setProperty (int propId, int propVal) { intProps.emplace_back (propId, propVal); }
    void setProperty (int propId, Time propVal) { timeProps.emplace_back (propId, propVal); }
    void setFlagOption (int flagId, bool propVal = true) { flagProps.emplace_back (flagId, propVal); }
};

}  // namespace helics
