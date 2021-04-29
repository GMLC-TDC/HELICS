/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
/** @file
functions for dealing with query results*/

#include "helics_cxx_export.h"

#include <chrono>
#include <string>
#include <vector>

// forward declare Federate
namespace helics {
class Federate;

/** function takes a query result and vectorizes it if the query is a vector result, if not the
 * results go into the first element of the vector
 */
HELICS_CXX_EXPORT std::vector<std::string> vectorizeQueryResult(std::string&& queryres);
/** function takes a query result and vectorizes it if the query is a vector result, if not the
 * results go into the first element of the vector
 */
HELICS_CXX_EXPORT std::vector<std::string> vectorizeQueryResult(const std::string& queryres);

/** function takes a query result and vectorizes it if the query is a vector result of integer
 * indices, if not the results are an empty vector
 */
HELICS_CXX_EXPORT std::vector<int> vectorizeIndexQuery(const std::string& queryres);

/** function takes a query result, vectorizes and sorts it if the query is a vector result, if not
 * the results go into the first element of the vector
 */
HELICS_CXX_EXPORT std::vector<std::string> vectorizeAndSortQueryResult(std::string&& queryres);
/** function takes a query result, vectorizes and sorts it if the query is a vector result, if not
 * the results go into the first element of the vector
 */
HELICS_CXX_EXPORT std::vector<std::string> vectorizeAndSortQueryResult(const std::string& queryres);

/** helper function to wait for a particular federate has requested initialization mode
@details this is useful for querying information and being reasonably certain the federate is done
adding to its interface
@param fed  a pointer to the federate
@param fedName the name of the federate we are querying
@param timeout the time to wait for the fed to initialize
@return true if the federate is now trying to enter initialization false if the timeout was reached
*/
HELICS_CXX_EXPORT bool
    waitForInit(helics::Federate* fed,
                const std::string& fedName,
                std::chrono::milliseconds timeout = std::chrono::milliseconds(10000));

/** helper function to wait for a particular federate to be created
@details this is useful if some reason we need to make sure a federate is created before proceeding
@param fed  a pointer to the federate
@param fedName the name of the federate we are querying
@param timeout the amount of time in ms to wait before returning false
@return true if the federate exists, false if the timeout occurred
*/
HELICS_CXX_EXPORT bool
    waitForFed(helics::Federate* fed,
               const std::string& fedName,
               std::chrono::milliseconds timeout = std::chrono::milliseconds(10000));

/** helper function to get a list of all the publications a federate subscribes to
@param fed  a pointer to the federate
@param fedName the name of the federate we are querying
@return a string vector of the names of the publication that are subscribed to
*/
HELICS_CXX_EXPORT std::string queryFederateSubscriptions(helics::Federate* fed,
                                                         const std::string& fedName);

}  // namespace helics
