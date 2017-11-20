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

//forward declare Federate
namespace helics
{
    class Federate;
}
/** function takes a query result and vectorizes it if the query is a vector result, if not the results go into the first element of the vector
*/
std::vector<std::string> vectorizeQueryResult(std::string &&queryres);
/** function takes a query result and vectorizes it if the query is a vector result, if not the results go into the first element of the vector
*/
std::vector<std::string> vectorizeQueryResult(const std::string &queryres);

/** function takes a query result, vectorizes and sorts it if the query is a vector result, if not the results go into the first element of the vector
*/
std::vector<std::string> vectorizeAndSortQueryResult(std::string &&queryres);
/** function takes a query result, vectorizes and sorts it if the query is a vector result, if not the results go into the first element of the vector
*/
std::vector<std::string> vectorizeAndSortQueryResult(const std::string &queryres);

/** helper function to wait for a particular federate has requested initialization mode
@details this is useful for querying information and being reasonably certain the federate is done adding to its interface
@param[in] fed  a pointer to the federate
@param[in] fedName the name of the federate we are querying
*/
bool waitForInit(helics::Federate *fed, const std::string &fedName);
#endif /*_HELICS_QUERY_FUNCTIONS_H_*/
