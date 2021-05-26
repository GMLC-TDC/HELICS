/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "PrecHelper.hpp"

#include "../application_api/Federate.hpp"
#include "gmlc/utilities/stringOps.h"

#include <algorithm>
#include <string>
using helics::data_type;

data_type getType(const std::string& typeString)
{
    auto tstr = gmlc::utilities::stringOps::trim(typeString);
    // trim the string
    if (tstr.empty()) {
        return data_type::helics_custom;
    }
    if (tstr.size() == 1) {
        switch (tstr[0]) {
            case 'a':
            case 'A':
                return data_type::helics_any;
            case 's':
            case 'S':
                return data_type::helics_string;
            case 'd':
            case 'D':
            case 'f':
            case 'F':
                return data_type::helics_double;
            case 'i':
            case 'I':
                return data_type::helics_int;
            case 'c':
            case 'C':
                return data_type::helics_complex;
            case 'v':
            case 'V':
                return data_type::helics_vector;
            default:
                return data_type::helics_custom;
        }
    }

    gmlc::utilities::makeLowerCase(tstr);

    return helics::getTypeFromString(tstr);
}

char typeCharacter(data_type type)
{
    switch (type) {
        case data_type::helics_string:
            return 's';
        case data_type::helics_double:
            return 'd';
        case data_type::helics_int:
            return 'i';
        case data_type::helics_complex:
            return 'c';
        case data_type::helics_vector:
            return 'v';
        case data_type::helics_any:
            return 'a';
        case data_type::helics_custom:
        default:
            return 'u';
    }
}

bool isBinaryData(helics::data_block& data)
{
    return std::any_of(data.begin(), data.end(), [](const auto& c) {
        return ((c < 32) || (c == 34) || (c > 126));
    });
}

bool isEscapableData(helics::data_block& data)
{
    return std::all_of(data.begin(), data.end(), [](const auto& c) {
        return ((c >= 32 && c <= 126) || (c == '\t') || (c == '\n'));
    });
}
