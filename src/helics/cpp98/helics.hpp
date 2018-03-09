/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.
*/

#ifndef _HELICS_CPP98_API_
#define _HELICS_CPP98_API_
#pragma once

#include "chelics.h"
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

