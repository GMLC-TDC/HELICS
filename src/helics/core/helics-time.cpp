/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "helics-time.hpp"
#include "../common/stringOps.h"
#include <map>

namespace helics
{
const std::map<std::string, timeUnits> timeUnitStrings{{"ps", timeUnits::ps},
                                                       {"ns", timeUnits::ns},
                                                       {"us", timeUnits::us},
                                                       {"ms", timeUnits::ms},
                                                       {"s", timeUnits::s},
                                                       {"sec", timeUnits::sec},
                                                       { "", timeUnits::sec }, //don't want empty string to error default is sec
                                                       {"seconds", timeUnits::sec},
                                                       {"second", timeUnits::sec},
                                                       {"min", timeUnits::minutes},
                                                       {"minute", timeUnits::minutes},
                                                       {"minutes", timeUnits::minutes},
                                                       {"hr", timeUnits::hr},
                                                       {"hour", timeUnits::hr},
                                                       {"hours", timeUnits::hr},
                                                       {"day", timeUnits::day},
                                                       {"week", timeUnits::week},
                                                       {"wk", timeUnits::week}};

timeUnits timeUnitsFromString (const std::string &unitString)
{
    auto fnd = timeUnitStrings.find (unitString);
    if (fnd != timeUnitStrings.end ())
    {
        return fnd->second;
    }
    auto lcUstring = convertToLowerCase (stringOps::trim (unitString));
    fnd = timeUnitStrings.find (lcUstring);
    if (fnd != timeUnitStrings.end ())
    {
        return fnd->second;
    }
    throw (std::invalid_argument (std::string ("unit ") + unitString + " not recognized"));
}

helics::Time loadTimeFromString (const std::string &timeString)
{
    size_t pos;
    double val = std::stod (timeString, &pos);
    if (pos == std::string::npos)
    {
        return Time (val);
    }
    std::string units = stringOps::trim(timeString.substr (pos));
    return Time (val * toSecondMultiplier(timeUnitsFromString (units)));
}

helics::Time loadTimeFromString(const std::string &timeString,timeUnits defUnits)
{
    size_t pos;
    double val = std::stod(timeString, &pos);
    if (pos == std::string::npos)
    {
        return Time(val*toSecondMultiplier(defUnits));
    }
    std::string units = stringOps::trim(timeString.substr(pos));
    return Time(val * toSecondMultiplier(timeUnitsFromString(units)));
}
}
