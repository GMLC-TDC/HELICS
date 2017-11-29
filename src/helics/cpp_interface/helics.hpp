/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_CPP98_API_
#define _HELICS_CPP98_API_
#pragma once

#include "helics.h"
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
