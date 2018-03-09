/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

#include "../application_api/helicsTypes.hpp"

namespace helics
{
class FederateInfo;
}

helics::helics_type_t getType (const std::string &typeString);

char typeCharacter (helics::helics_type_t type);

