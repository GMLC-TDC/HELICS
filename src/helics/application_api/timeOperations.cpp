/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "timeOperations.hpp"
#include "../core/coreTimeOperations.hpp"

namespace helics
{

time_units timeUnitsFromString (const std::string &unitString) { return core::timeUnitsFromString (unitString); }

Time loadTimeFromString (const std::string &timeString) { return core::loadTimeFromString (timeString); }

Time loadTimeFromString (std::string timeString, time_units defUnits)
{
    return core::loadTimeFromString (timeString, defUnits);
}

}  // namespace helics


