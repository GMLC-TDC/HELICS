/*

Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "FilterInfo.hpp"
//#include "core/core-data.h"
#include <algorithm>
#include <cstring>

namespace helics {
void FilterInfo::removeTarget(global_handle targetToRemove)
{
    sourceTargets.erase(std::remove(sourceTargets.begin(), sourceTargets.end(), targetToRemove),
                        sourceTargets.end());
    destTargets.erase(std::remove(destTargets.begin(), destTargets.end(), targetToRemove),
                      destTargets.end());
}
}  // namespace helics
