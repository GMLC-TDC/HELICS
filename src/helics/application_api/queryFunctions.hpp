/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

/** @file
functions for dealing with query results*/



#include <string>
#include <vector>

// forward declare Federate
namespace helics
{
class Federate;
}
/** function takes a query result and vectorizes it if the query is a vector result, if not the results go into the
 * first element of the vector
 */
std::vector<std::string> vectorizeQueryResult (std::string &&queryres);
/** function takes a query result and vectorizes it if the query is a vector result, if not the results go into the
 * first element of the vector
 */
std::vector<std::string> vectorizeQueryResult (const std::string &queryres);

/** function takes a query result, vectorizes and sorts it if the query is a vector result, if not the results go
 * into the first element of the vector
 */
std::vector<std::string> vectorizeAndSortQueryResult (std::string &&queryres);
/** function takes a query result, vectorizes and sorts it if the query is a vector result, if not the results go
 * into the first element of the vector
 */
std::vector<std::string> vectorizeAndSortQueryResult (const std::string &queryres);

/** helper function to wait for a particular federate has requested initialization mode
@details this is useful for querying information and being reasonably certain the federate is done adding to its
interface
@param[in] fed  a pointer to the federate
@param[in] fedName the name of the federate we are querying
@return true if the federate is now trying to enter initialization false if the timeout was reached
*/
bool waitForInit (helics::Federate *fed, const std::string &fedName, int timeout = 10000 /*time in ms*/);

/** helper function to wait for a particular federate to be created
@details this is useful if some reason we need to make sure a federate is created before proceeding
@param[in] fed  a pointer to the federate
@param[in] fedName the name of the federate we are querying
@param[in] timeout the amount of time in ms to wait before returning false
@return true if the federate exists, false if the timeout occurred
*/
bool waitForFed (helics::Federate *fed, const std::string &fedName, int timeout = 10000 /*time in ms*/);
