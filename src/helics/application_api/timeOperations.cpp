/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "timeOperations.hpp"

#include "../utilities/timeStringOps.hpp"

#include <string>

namespace helics {

time_units timeUnitsFromString(std::string_view unitString)
{
    return gmlc::utilities::timeUnitsFromString(std::string(unitString));
}

Time loadTimeFromString(std::string_view timeString)
{
    return gmlc::utilities::loadTimeFromString<Time>(std::string(timeString));
}

Time loadTimeFromString(std::string_view timeString, time_units defUnits)
{
    return gmlc::utilities::loadTimeFromString<Time>(std::string(timeString), defUnits);
}

}  // namespace helics
