/*
Copyright (c) 2017-2023,
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

enum class JsonErrorCodes : std::int32_t {
    BAD_REQUEST = 400,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    METHOD_NOT_ALLOWED = 405,
    TIMEOUT = 408,
    DISCONNECTED = 410,
    INTERNAL_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    BAD_GATEWAY = 502,
    SERVICE_UNAVAILABLE = 503,
    GATEWAY_TIMEOUT = 504
};
/** generate a json error response string*/
inline std::string generateJsonErrorResponse(JsonErrorCodes code, const std::string& message)
{
    return fmt::format("{{\n  \"error\":{{\n    \"code\":{},\n    \"message\":{}\n  }}\n}}",
                       static_cast<std::int32_t>(code),
                       generateJsonQuotedString(message));
}

}  // namespace helics
