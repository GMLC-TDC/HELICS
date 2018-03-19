/*
Copyright © 2017-2018,
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
#include <complex>
#include <regex>
#include <sstream>


#include "../application_api/Federate.hpp"

using namespace helics;

helics_type_t getType (const std::string &typeString)
{
    auto tstr = typeString;
    // trim the string
    tstr.erase (tstr.find_last_not_of (" \t\n\0") + 1);
    tstr.erase (0, tstr.find_first_not_of (" \t\n\0"));
    if (tstr.empty ())
    {
        return helics_type_t::helicsInvalid;
    }
    if (tstr.size () == 1)
    {
        switch (tstr[0])
        {
        case 'a':
        case 'A':
            return helics_type_t::helicsAny;
        case 's':
        case 'S':
            return helics_type_t::helicsString;
        case 'd':
        case 'D':
        case 'f':
        case 'F':
            return helics_type_t::helicsDouble;
        case 'i':
        case 'I':
            return helics_type_t::helicsInt;
        case 'c':
        case 'C':
            return helics_type_t::helicsComplex;
        case 'v':
        case 'V':
            return helics_type_t::helicsVector;
        default:
            return helics_type_t::helicsInvalid;
        }
    }

    std::transform (tstr.begin (), tstr.end (), tstr.begin (), ::tolower);

    return getTypeFromString(tstr);

}


char typeCharacter (helics_type_t type)
{
    switch (type)
    {
    case helics_type_t::helicsString:
        return 's';
    case helics_type_t::helicsDouble:
        return 'd';
    case helics_type_t::helicsInt:
        return 'i';
    case helics_type_t::helicsComplex:
        return 'c';
    case helics_type_t::helicsVector:
        return 'v';
    case helics_type_t::helicsAny:
        return 'a';
    case helics_type_t::helicsInvalid:
    default:
        return 'u';
    }
}

