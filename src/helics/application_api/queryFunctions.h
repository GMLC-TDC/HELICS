/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

/** @file
functions for dealing with query results*/

#ifndef _HELICS_QUERY_FUNCTIONS_H_
#define _HELICS_QUERY_FUNCTIONS_H_
#pragma once

#include <vector>
#include <string>

/** function takes a query result and vectorizes it if the query is a vector result, if not the results go into the first element of the vector
*/
std::vector<std::string> vectorizeQueryResult(std::string &&queryres);
/** function takes a query result and vectorizes it if the query is a vector result, if not the results go into the first element of the vector
*/
std::vector<std::string> vectorizeQueryResult(const std::string &queryres);

#endif /*_HELICS_QUERY_FUNCTIONS_H_*/
