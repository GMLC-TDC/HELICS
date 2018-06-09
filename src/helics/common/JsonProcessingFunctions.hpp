/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

/** @file
@details functions related to loading and evaluating JSON files and helper functions for reading them
using the jsoncpp library
*/

#include "json/jsoncpp.h"

#include "../core/helics-time.hpp"
/** load a JSON string or filename that points to a JSON file and return a
json::Value to the root object
*/
Json_helics::Value loadJson (const std::string &jsonString);

/** load a JSON object in a string
*/
Json_helics::Value loadJsonStr(const std::string &jsonString);

/** read a time from a JSON value element*/
helics::Time loadJsonTime (const Json_helics::Value &timeElement, timeUnits defaultUnits = timeUnits::sec);

/** get a name or key from the element*/
std::string getKey (const Json_helics::Value &Element);