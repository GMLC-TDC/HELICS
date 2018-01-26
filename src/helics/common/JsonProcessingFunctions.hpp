/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#pragma once
/** @file
@details functions related to loading and evaluating json files and helper functions for reading them
using the json cpp library
*/

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#include "json/json.h"
#pragma warning(pop)
#else
#include "json/json.h"
#endif

#include "../core/helics-time.hpp"
/** load a json string or filename that points to a json file and return a
json::Value to the root object
*/
Json_helics::Value loadJsonString (const std::string &jsonString);

/** read a time from a JSON value element*/
helics::Time loadJsonTime(const Json_helics::Value &timeElement);

/** generate a time from a string, 
@details the string can be a double or with units
@example "1.234",  or "1032ms"
@return a helics time generated from the string
@throw, invalid_argument if the string is not a valid time
*/
helics::Time loadTimeFromString(const std::string &timeString);