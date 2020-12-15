/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "fmt_format.h"

#include "json/json.h"
#include "json/writer.h"
#include <string>

namespace helics {

inline std::string generateJsonQuotedString(const std::string& string)
{
    Json::String V = Json::valueToQuotedString(string.c_str());
    return V.c_str();
}

/** generate a json error response string*/
inline std::string generateJsonErrorResponse(int code, const std::string& message)
{
    return fmt::format("{{\n  \"error\":{{\n    \"code\":{},\n    \"message\":{}\n  }}\n}}",
                       code,
                       generateJsonQuotedString(message));
}

}  // namespace helics
