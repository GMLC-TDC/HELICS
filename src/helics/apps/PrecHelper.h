/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef PREC_HELPER_H_
#define PREC_HELPER_H_

#include <string>
#include "../application_api/helicsTypes.hpp"

namespace helics
{
    class FederateInfo;
}

helics::helicsType_t getType(const std::string &typeString);

char typeCharacter(helics::helicsType_t type);

#endif
