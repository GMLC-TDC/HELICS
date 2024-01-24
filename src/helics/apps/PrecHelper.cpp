/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "PrecHelper.hpp"

#include "../application_api/Federate.hpp"
#include "gmlc/utilities/string_viewOps.h"

#include <algorithm>
#include <string>
using helics::DataType;

DataType getType(const std::string& typeString)
{
    auto tstr = gmlc::utilities::string_viewOps::trim(typeString);
    // trim the string
    if (tstr.empty()) {
        return DataType::HELICS_CUSTOM;
    }
    if (tstr.size() == 1) {
        switch (tstr[0]) {
            case 'a':
            case 'A':
                return DataType::HELICS_ANY;
            case 's':
            case 'S':
                return DataType::HELICS_STRING;
            case 'd':
            case 'D':
            case 'f':
            case 'F':
                return DataType::HELICS_DOUBLE;
            case 'i':
            case 'I':
                return DataType::HELICS_INT;
            case 'c':
            case 'C':
                return DataType::HELICS_COMPLEX;
            case 'v':
            case 'V':
                return DataType::HELICS_VECTOR;
            default:
                return DataType::HELICS_CUSTOM;
        }
    }

    return helics::getTypeFromString(tstr);
}

char typeCharacter(DataType type)
{
    switch (type) {
        case DataType::HELICS_STRING:
        case DataType::HELICS_CHAR:
            return 's';
        case DataType::HELICS_DOUBLE:
            return 'd';
        case DataType::HELICS_INT:
            return 'i';
        case DataType::HELICS_COMPLEX:
            return 'c';
        case DataType::HELICS_VECTOR:
            return 'v';
        case DataType::HELICS_ANY:
            return 'a';
        case DataType::HELICS_CUSTOM:
        default:
            return 'u';
    }
}

bool isBinaryData(helics::SmallBuffer& data)
{
    auto str = data.to_string();
    return std::any_of(str.begin(), str.end(), [](const auto& c) {
        return ((c < 32) || (c == 34) || (c > 126));
    });
}

bool isEscapableData(helics::SmallBuffer& data)
{
    auto str = data.to_string();
    return std::all_of(str.begin(), str.end(), [](const auto& c) {
        return ((c >= 32 && c <= 126) || (c == '\t') || (c == '\n'));
    });
}
