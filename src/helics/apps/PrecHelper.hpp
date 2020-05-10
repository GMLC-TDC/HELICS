/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../application_api/helicsTypes.hpp"

#include <string>

namespace helics {
class FederateInfo;
}  // namespace helics

helics::data_type getType(const std::string& typeString);

char typeCharacter(helics::data_type type);

bool isBinaryData(helics::data_block& data);
