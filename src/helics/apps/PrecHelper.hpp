/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
*/

#include "../application_api/helicsTypes.hpp"

namespace helics
{
class FederateInfo;
}

helics::helics_type_t getType (const std::string &typeString);

char typeCharacter (helics::helics_type_t type);

