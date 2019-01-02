/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "../application_api/helicsTypes.hpp"

namespace helics
{
class FederateInfo;
}

helics::data_type getType (const std::string &typeString);

char typeCharacter (helics::data_type type);

bool isBinaryData(helics::data_block &data);