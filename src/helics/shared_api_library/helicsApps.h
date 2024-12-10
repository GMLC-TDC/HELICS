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

/** HELICS_TRUE if the app library is enabled*/
HELICS_EXPORT HelicsBool helicsAppEnabled();
/**
* Create a HelicsApp object.
*
* @details Create a HelicsApp object.
*
* @param appName A string with the name of the app, can be NULL or an empty string to pull the default name from fedInfo or the config file.
* @param appType The type of app to create.
* @param configFile Configuration file or string to pass into the app, can be NULL or empty.
* @param fedInfo The federate information to pass into the app, can be NULL.
* @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

*
* @return An opaque value app object, or nullptr if the object creation failed.
*/
HELICS_EXPORT HelicsApp
    helicsCreateApp(const char* appName, const char* appType, const char* configFile, HelicsFederateInfo fedInfo, HelicsError* err);

/** Run the HelicsApp.
* @details Execute the HelicsApp to completion
* @param app The app to execute.
* @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
*
* @return An opaque value federate object that can be used in any of the federate methods, not recommended to use this object to advance
time, the app will not likely function normally; other query, information calls, or modification calls on the federate are fine.
*/
HELICS_EXPORT HelicsFederate helicsAppGetFederate(HelicsApp app, HelicsError* err);

/**
 * Load a file to an App.
 *
 * @details Loads a configuration file for an app.
 *
 * @param appName A string with the name of the app, can be NULL or an empty string to pull the default name from fedInfo or the config
 * file.
 * @param app The app to load a file.
 * @param configFile Configuration file or string to pass into the app, can be NULL or empty.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsAppLoadFile(HelicsApp app, const char* configFile, HelicsError* err);

/** Initialize the HelicsApp federate.
 * @details Generate all the interfaces and load data for the application.
 * @param app The app to initialize.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsAppInitialize(HelicsApp app, HelicsError* err);

/** Run the HelicsApp.
 * @details Execute the app to completion.
 * @param app The app to execute.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsAppRun(HelicsApp app, HelicsError* err);

/** Run a HelicsApp to a specified stop time.
 * @details It is possible to call this method repeatedly with different times.
 * @param app The app to run.
 * @param stopTime The desired stop time.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsAppRunTo(HelicsApp app, HelicsTime stopTime, HelicsError* err);

/** Finalize the HelicsApp.
 * @param app The app to finalize.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsAppFinalize(HelicsApp app, HelicsError* err);

/** Free the HelicsApp object.
 * @param app The app to free.
 */
HELICS_EXPORT void helicsAppFree(HelicsApp app);

/** Disconnect and free a HelicsApp.
 * @param app The app to destroy.
 */
HELICS_EXPORT void helicsAppDestroy(HelicsApp app);

/** Check if the HelicsApp is active and ready to run.
 * @param app The app to check.
 * @return True if the app is active, otherwise false.
 */
HELICS_EXPORT HelicsBool helicsAppIsActive(HelicsApp app);

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif
