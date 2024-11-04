/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

/**
 * @file
 * @brief Functions related to using helics apps
 */

#ifndef HELICS_APISHARED_APP_FUNCTIONS_H_
#define HELICS_APISHARED_APP_FUNCTIONS_H_

#include "helicsCore.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
* Create a helics app object
*
* @details create a helics App object
*
* @param appName A string with the name of the app, can be NULL or an empty string to pull the default name from fedInfo or the config file.
* @param appType the type of app to create
* @param configFile configuration file or string to pass into the app, can be NULL or empty
* @param fedInfo the federate information to pass into the app, can be NULL
* @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

*
* @return An opaque value app object nullptr if the object creation failed.
*/
HELICS_EXPORT HelicsApp
    helicsCreateApp(const char* appName, const char* appType, const char* configFile, HelicsFederateInfo fedInfo, HelicsError* err);

/** run the App
* @details execute the app to completion
* @param app the app to execute
* @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
*
* @return An opaque value federate object that can be used in any of the federate methods, not recommended to use this object to advance
time, the app will not likely function normally, other query, or information calls, or modification calls on the federate are fine.
*/
HELICS_EXPORT HelicsFederate helicsAppGetFederate(HelicsApp app, HelicsError* err);

/**
* Create a helics app object
*
* @details create a helics App object
*
* @param appName A string with the name of the app, can be NULL or an empty string to pull the default name from fedInfo or the config file.
* @param appType the type of app to create
* @param configFile configuration file or string to pass into the app, can be NULL or empty
* @param fedInfo the federate information to pass into the app, can be NULL
* @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

*
* @return An opaque value app object nullptr if the object creation failed.
*/
HELICS_EXPORT void helicsAppLoadFile(HelicsApp app, const char* configFile, HelicsError* err);

/** initialize the App federate
 * @details generate all the interfaces and load data for the application
 * @param app the app to initialize
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsAppInitialize(HelicsApp app, HelicsError* err);

/** run the App
 * @details execute the app to completion
 * @param app the app to execute
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsAppRun(HelicsApp app, HelicsError* err);

/** run and app to a specified stop time
 * @details it is possible to call this method repeatedly with different times
 * @param app the app to run
 * @param stopTime the desired stop time
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsAppRunTo(HelicsApp app, HelicsTime stopTime, HelicsError* err);

/** finalize the app
 * @param app the app to execute
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsAppFinalize(HelicsApp app, HelicsError* err);

/** check if the App is active and ready to run*/
HELICS_EXPORT HelicsBool helicsAppIsActive(HelicsApp app);

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif
