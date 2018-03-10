/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#ifndef _HELICS_CPP98_API_
#define _HELICS_CPP98_API_
#pragma once

#include "CombinationFederate.hpp"
#include <string>


/**
 * HELICS C++98 Interface
 */
namespace helics
{

/** get a string with the helics version info*/
std::string getHelicsVersionString()
{
    return std::string(helicsGetVersion());
}

} //namespace helics
#endif

