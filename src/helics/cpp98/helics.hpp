/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef _HELICS_CPP98_API_
#define _HELICS_CPP98_API_
#pragma once

#include "CombinationFederate.hpp"

#include <string>

/**
 * HELICS C++98 Interface
 */
namespace helicscpp {
/** get a string with the helics version info*/
std::string getHelicsVersionString()
{
    return std::string(helicsGetVersion());
}

} // namespace helicscpp
#endif
