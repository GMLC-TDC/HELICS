/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "timeOperations.hpp"

#include "../utilities/timeStringOps.hpp"

namespace helics {

time_units timeUnitsFromString(const std::string& unitString)
{
    return gmlc::utilities::timeUnitsFromString(unitString);
}

Time loadTimeFromString(const std::string& timeString)
{
    return gmlc::utilities::loadTimeFromString<Time>(timeString);
}

Time loadTimeFromString(std::string timeString, time_units defUnits)
{
    return gmlc::utilities::loadTimeFromString<Time>(timeString, defUnits);
}

}  // namespace helics
