/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "helics-time.hpp"
#include "../common/stringOps.h"
#include <map>

namespace helics
{
const std::map<std::string, time_units> time_unitstrings{
  {"ps", time_units::ps},
  {"ns", time_units::ns},
  {"us", time_units::us},
  {"ms", time_units::ms},
  {"s", time_units::s},
  {"sec", time_units::sec},
  {"", time_units::sec},  // don't want empty string to error default is sec
  {"seconds", time_units::sec},
  {"second", time_units::sec},
  {"min", time_units::minutes},
  {"minute", time_units::minutes},
  {"minutes", time_units::minutes},
  {"hr", time_units::hr},
  {"hour", time_units::hr},
  {"hours", time_units::hr},
  {"day", time_units::day},
  {"week", time_units::week},
  {"wk", time_units::week}};

time_units timeUnitsFromString (const std::string &unitString)
{
    auto fnd = time_unitstrings.find (unitString);
    if (fnd != time_unitstrings.end ())
    {
        return fnd->second;
    }
    auto lcUstring = convertToLowerCase (stringOps::trim (unitString));
    fnd = time_unitstrings.find (lcUstring);
    if (fnd != time_unitstrings.end ())
    {
        return fnd->second;
    }
    throw (std::invalid_argument (std::string ("unit ") + unitString + " not recognized"));
}

helics::Time loadTimeFromString (const std::string &timeString)
{
    size_t pos;
    double val = std::stod (timeString, &pos);
    if (pos >= timeString.size ())
    {
        return Time (val);
    }
    std::string units = stringOps::trim (timeString.substr (pos));
    return Time (val * toSecondMultiplier (timeUnitsFromString (units)));
}

helics::Time loadTimeFromString (const std::string &timeString, time_units defUnits)
{
    size_t pos;
    double val = std::stod (timeString, &pos);
    if (pos >= timeString.size ())
    {
        return Time (val * toSecondMultiplier (defUnits));
    }
    std::string units = stringOps::trim (timeString.substr (pos));
    return Time (val * toSecondMultiplier (timeUnitsFromString (units)));
}
} //namespace helics
