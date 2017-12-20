/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "PrecHelper.h"

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


#include "../application_api/Federate.h"

using namespace helics;

helicsType_t getType (const std::string &typeString)
{
    auto tstr = typeString;
    // trim the string
    tstr.erase (tstr.find_last_not_of (" \t\n\0") + 1);
    tstr.erase (0, tstr.find_first_not_of (" \t\n\0"));
    if (tstr.empty ())
    {
        return helicsType_t::helicsInvalid;
    }
    if (tstr.size () == 1)
    {
        switch (tstr[0])
        {
        case 'a':
        case 'A':
            return helicsType_t::helicsAny;
        case 's':
        case 'S':
            return helicsType_t::helicsString;
        case 'd':
        case 'D':
        case 'f':
        case 'F':
            return helicsType_t::helicsDouble;
        case 'i':
        case 'I':
            return helicsType_t::helicsInt;
        case 'c':
        case 'C':
            return helicsType_t::helicsComplex;
        case 'v':
        case 'V':
            return helicsType_t::helicsVector;
        default:
            return helicsType_t::helicsInvalid;
        }
    }

    std::transform (tstr.begin (), tstr.end (), tstr.begin (), ::tolower);

    return getTypeFromString(tstr);
   
}


char typeCharacter (helicsType_t type)
{
    switch (type)
    {
    case helicsType_t::helicsString:
        return 's';
    case helicsType_t::helicsDouble:
        return 'd';
    case helicsType_t::helicsInt:
        return 'i';
    case helicsType_t::helicsComplex:
        return 'c';
    case helicsType_t::helicsVector:
        return 'v';
    case helicsType_t::helicsAny:
        return 'a';
    case helicsType_t::helicsInvalid:
    default:
        return 'u';
    }
}




std::vector<std::string> splitline (const std::string &line, const std::string &delimiters, bool compression)
{
    std::vector<std::string> strVec;
    auto comp = (compression) ? boost::token_compress_on : boost::token_compress_off;
    boost::algorithm::split (strVec, line, boost::is_any_of (delimiters), comp);
    return strVec;
}
