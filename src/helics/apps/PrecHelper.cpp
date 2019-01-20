/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "PrecHelper.hpp"

#ifdef __GNUC__
#pragma GCC diagnostic push
//#pragma GCC diagnostic warning "-w"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#pragma GCC diagnostic pop
#else
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#endif

#include <algorithm>

#include "../application_api/Federate.hpp"

using namespace helics;

data_type getType (const std::string &typeString)
{
    auto tstr = typeString;
    // trim the string
    tstr.erase (tstr.find_last_not_of (" \t\n\0") + 1);
    tstr.erase (0, tstr.find_first_not_of (" \t\n\0"));
    if (tstr.empty ())
    {
        return data_type::helics_custom;
    }
    if (tstr.size () == 1)
    {
        switch (tstr[0])
        {
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

    std::transform (tstr.begin (), tstr.end (), tstr.begin (), ::tolower);

    return getTypeFromString (tstr);
}

char typeCharacter (data_type type)
{
    switch (type)
    {
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

bool isBinaryData (helics::data_block &data)
{
    return std::any_of (data.begin (), data.end (),
                        [](const auto &c) { return ((c < 32) || (c == 34) || (c > 126)); });
}
