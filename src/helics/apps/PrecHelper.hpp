/*
Copyright (c) 2017-2024,
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

helics::DataType getType(std::string_view typeString);

char typeCharacter(helics::DataType type);

bool isBinaryData(helics::SmallBuffer& data);
/**Returns true if the data is escapable per json.  Ie. a normal character string with a few
 * escapable characters*/
bool isEscapableData(helics::SmallBuffer& data);
