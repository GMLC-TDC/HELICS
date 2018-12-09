/*

Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "FilterInfo.hpp"
//#include "core/core-data.h"
#include <algorithm>
#include <cstring>

namespace helics
{
void FilterInfo::removeTarget (global_handle targetToRemove)
{
    sourceTargets.erase (std::remove (sourceTargets.begin (), sourceTargets.end (), targetToRemove),
                         sourceTargets.end ());
    destTargets.erase (std::remove (destTargets.begin (), destTargets.end (), targetToRemove), destTargets.end ());
}
}  // namespace helics
