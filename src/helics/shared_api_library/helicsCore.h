/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef HELICS_APISHARED_FUNCTIONS_H_
#define HELICS_APISHARED_FUNCTIONS_H_

#include "api-data.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "helics_export.h"

#include <stdlib.h>

/**
 * @file
 * @brief Common functions for the HELICS C api.
 */

/***************************************************
 * Common Functions
 ***************************************************/

/**
 * Get a version string for HELICS.
 */
HELICS_EXPORT const char* helicsGetVersion(void);

/**
 * Get the build flags used to compile HELICS.
 */
HELICS_EXPORT const char* helicsGetBuildFlags(void);

/**
 * Get the compiler version used to compile HELICS.
 */
HELICS_EXPORT const char* helicsGetCompilerVersion(void);

/**
 * Get a json formatted system information string, containing version info.
 * The string contains fields with system information like cpu, core count, operating system, and memory,
 * as well as information about the HELICS build.  Used for debugging reports and gathering other information.
 */
HELICS_EXPORT const char* helicsGetSystemInfo(void);

/**
 * Return an initialized error object.
 */
HELICS_EXPORT HelicsError helicsErrorInitialize(void);

/**
 * Clear an error object.
 */
HELICS_EXPORT void helicsErrorClear(HelicsError* err);

/** Load a signal handler that handles Ctrl-C and shuts down all HELICS brokers, cores,
and federates then exits the process.*/
HELICS_EXPORT void helicsLoadSignalHandler();

/** Load a signal handler that handles Ctrl-C and shuts down all HELICS brokers, cores,
and federates then exits the process.  This operation will execute in a newly created and detached thread returning control back to the
calling program before completing operations.*/
HELICS_EXPORT void helicsLoadThreadedSignalHandler();

/** Clear HELICS based signal handlers.*/
HELICS_EXPORT void helicsClearSignalHandler();

/** Load a custom signal handler to execute prior to the abort signal handler.
@details  This function is not 100% reliable it will most likely work but uses some functions and
techniques that are not 100% guaranteed to work in a signal handler
and in worst case it could deadlock.  That is somewhat unlikely given usage patterns
but it is possible.  The callback has signature HelicsBool(*handler)(int) and it will take the SIG_INT as an argument
and return a boolean.  If the boolean return value is HELICS_TRUE (or the callback is null) the default signal handler is run after the
callback finishes; if it is HELICS_FALSE the default callback is not run and the default signal handler is executed. If the second
argument is set to HELICS_TRUE the default signal handler will execute in a separate thread(this may be a bad idea). */
HELICS_EXPORT void helicsLoadSignalHandlerCallback(HelicsBool (*handler)(int), HelicsBool useSeparateThread);

/** Load a custom signal handler to execute prior to the abort signal handler. The signal handler does not call exit.
@details  This function is not 100% reliable. It will most likely work but uses some functions and
techniques that are not 100% guaranteed to work in a signal handler
and in worst case it could deadlock.  That is somewhat unlikely given usage patterns
but it is possible.  The callback has signature HelicsBool(*handler)(int) and it will take the SIG_INT as an argument
and return a boolean.  If the boolean return value is HELICS_TRUE (or the callback is null) the no exit signal handler is run after the
callback finishes; if it is HELICS_FALSE the default callback is not run and the default signal handler is executed. If the second
argument is set to HELICS_TRUE the default signal handler will execute in a separate thread (this may be a bad idea). */
HELICS_EXPORT void helicsLoadSignalHandlerCallbackNoExit(HelicsBool (*handler)(int), HelicsBool useSeparateThread);

/** Execute a global abort by sending an error code to all cores, brokers,
and federates that were created through the current library instance.*/
HELICS_EXPORT void helicsAbort(int errorCode, const char* errorString);

/**
 * Returns true if core/broker type specified is available in current compilation.
 *
 * @param type A string representing a core type.
 *
 * @details Options include "zmq", "udp", "ipc", "interprocess", "tcp", "default", "mpi".
 */
HELICS_EXPORT HelicsBool helicsIsCoreTypeAvailable(const char* type);

/**
 * Create a core object.
 *
 * @param type The type of the core to create.
 * @param name The name of the core. It can be a nullptr or empty string to have a name automatically assigned.
 * @param initString An initialization string to send to the core. The format is similar to command line arguments.
 *                   Typical options include a broker name, the broker address, the number of federates, etc.  Can also be a
 *                   file (toml, ini, json) or json object containing the core configuration.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 * @return A HelicsCore object.
 *
 * If the core is invalid, err will contain the corresponding error message and the returned object will be NULL.

 */
HELICS_EXPORT HelicsCore helicsCreateCore(const char* type, const char* name, const char* initString, HelicsError* err);

/**
 * Create a core object by passing command line arguments.
 *
 * @param type The type of the core to create.
 * @param name The name of the core. It can be a nullptr or empty string to have a name automatically assigned.
 *
 * @param argc The number of arguments.

 * @param argv The list of string values from a command line.
 *
 * @param[in,out] err An error object that will contain an error code and string
 *                    if any error occurred during the execution of the function.

 *
 * @return A HelicsCore object.
 */
HELICS_EXPORT HelicsCore helicsCreateCoreFromArgs(const char* type, const char* name, int argc, const char* const* argv, HelicsError* err);

/**
 * Create a new reference to an existing core.
 *
 * @details This will create a new broker object that references the existing broker. The new broker object must be freed as well.
 *
 * @param core An existing HelicsCore.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 *
 * @return A new reference to the same broker.
 */
HELICS_EXPORT HelicsCore helicsCoreClone(HelicsCore core, HelicsError* err);

/**
 * Check if a core object is a valid object.
 *
 * @param core The HelicsCore object to test.
 */
HELICS_EXPORT HelicsBool helicsCoreIsValid(HelicsCore core);

/**
 * Create a broker object.
 *
 * @param type The type of the broker to create.
 * @param name The name of the broker. It can be a nullptr or empty string to have a name automatically assigned.
 * @param initString An initialization string to send to the core-the format is similar to command line arguments.
 *                   Typical options include a broker address such as --broker="XSSAF" if this is a subbroker, or the number of federates,
 *                    or it can also be a json or toml file with broker configuration.
 * or the address.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 * @return A HelicsBroker object.
 *
 * It will be NULL if there was an error indicated in the err object.

 */
HELICS_EXPORT HelicsBroker helicsCreateBroker(const char* type, const char* name, const char* initString, HelicsError* err);

/**
 * Create a core object by passing command line arguments.
 *
 * @param type The type of the core to create.
 * @param name The name of the core. It can be a nullptr or empty string to have a name automatically assigned.
 * @param argc The number of arguments.
 * @param argv The list of string values from a command line.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 * @return A HelicsCore object.
 */
HELICS_EXPORT HelicsBroker
    helicsCreateBrokerFromArgs(const char* type, const char* name, int argc, const char* const* argv, HelicsError* err);

/**
 * Create a new reference to an existing broker.
 *
 * @details This will create a new broker object that references the existing broker it must be freed as well.
 *
 * @param broker An existing HelicsBroker.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 * @return A new reference to the same broker.
 */
HELICS_EXPORT HelicsBroker helicsBrokerClone(HelicsBroker broker, HelicsError* err);

/**
 * Check if a broker object is a valid object.
 *
 * @param broker The HelicsBroker object to test.
 */
HELICS_EXPORT HelicsBool helicsBrokerIsValid(HelicsBroker broker);

/**
 * Check if a broker is connected.
 *
 * @details A connected broker implies it is attached to cores or cores could reach out to communicate.
 *
 * @return HELICS_FALSE if not connected.
 */
HELICS_EXPORT HelicsBool helicsBrokerIsConnected(HelicsBroker broker);

/**
 * Link a named publication and named input using a broker.
 *
 * @param broker The broker to generate the connection from.
 * @param source The name of the publication (cannot be NULL).
 * @param target The name of the target to send the publication data (cannot be NULL).
 *
 * @param[in,out] err A HelicsError object, can be NULL if the errors are to be ignored.

 */
HELICS_EXPORT void helicsBrokerDataLink(HelicsBroker broker, const char* source, const char* target, HelicsError* err);

/**
 * Link a named filter to a source endpoint.
 *
 * @param broker The broker to generate the connection from.
 * @param filter The name of the filter (cannot be NULL).
 * @param endpoint The name of the endpoint to filter the data from (cannot be NULL).
 *
 * @param[in,out] err A HelicsError object, can be NULL if the errors are to be ignored.

 */
HELICS_EXPORT void helicsBrokerAddSourceFilterToEndpoint(HelicsBroker broker, const char* filter, const char* endpoint, HelicsError* err);

/**
 * Link a named filter to a destination endpoint.
 *
 * @param broker The broker to generate the connection from.
 * @param filter The name of the filter (cannot be NULL).
 * @param endpoint The name of the endpoint to filter the data going to (cannot be NULL).
 *
 * @param[in,out] err A HelicsError object, can be NULL if the errors are to be ignored.

 */
HELICS_EXPORT void
    helicsBrokerAddDestinationFilterToEndpoint(HelicsBroker broker, const char* filter, const char* endpoint, HelicsError* err);

/**
 * Load a file containing connection information.
 *
 * @param broker The broker to generate the connections from.
 * @param file A JSON or TOML file containing connection information.
 *
 * @param[in,out] err A HelicsError object, can be NULL if the errors are to be ignored.

 */
HELICS_EXPORT void helicsBrokerMakeConnections(HelicsBroker broker, const char* file, HelicsError* err);

/**
 * Wait for the core to disconnect.
 *
 * @param core The core to wait for.
 * @param msToWait The time out in millisecond (<0 for infinite timeout).
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 * @return HELICS_TRUE if the disconnect was successful, HELICS_FALSE if there was a timeout.
 */
HELICS_EXPORT HelicsBool helicsCoreWaitForDisconnect(HelicsCore core, int msToWait, HelicsError* err);

/**
 * Wait for the broker to disconnect.
 *
 * @param broker The broker to wait for.
 * @param msToWait The time out in millisecond (<0 for infinite timeout).
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 * @return HELICS_TRUE if the disconnect was successful, HELICS_FALSE if there was a timeout.
 */
HELICS_EXPORT HelicsBool helicsBrokerWaitForDisconnect(HelicsBroker broker, int msToWait, HelicsError* err);

/**
 * Check if a core is connected.
 *
 * @details A connected core implies it is attached to federates or federates could be attached to it
 *
 * @return HELICS_FALSE if not connected, HELICS_TRUE if it is connected.
 */
HELICS_EXPORT HelicsBool helicsCoreIsConnected(HelicsCore core);

/**
 * Link a named publication and named input using a core.
 *
 * @param core The core to generate the connection from.
 * @param source The name of the publication (cannot be NULL).
 * @param target The name of the target to send the publication data (cannot be NULL).
 *
 * @param[in,out] err A HelicsError object, can be NULL if the errors are to be ignored.

 */
HELICS_EXPORT void helicsCoreDataLink(HelicsCore core, const char* source, const char* target, HelicsError* err);

/**
 * Link a named filter to a source endpoint.
 *
 * @param core The core to generate the connection from.
 * @param filter The name of the filter (cannot be NULL).
 * @param endpoint The name of the endpoint to filter the data from (cannot be NULL).
 *
 * @param[in,out] err A HelicsError object, can be NULL if the errors are to be ignored.

 */
HELICS_EXPORT void helicsCoreAddSourceFilterToEndpoint(HelicsCore core, const char* filter, const char* endpoint, HelicsError* err);

/**
 * Link a named filter to a destination endpoint.
 *
 * @param core The core to generate the connection from.
 * @param filter The name of the filter (cannot be NULL).
 * @param endpoint The name of the endpoint to filter the data going to (cannot be NULL).
 *
 * @param[in,out] err A HelicsError object, can be NULL if the errors are to be ignored.

 */
HELICS_EXPORT void helicsCoreAddDestinationFilterToEndpoint(HelicsCore core, const char* filter, const char* endpoint, HelicsError* err);

/**
 * Load a file containing connection information.
 *
 * @param core The core to generate the connections from.
 * @param file A JSON or TOML file containing connection information.
 *
 * @param[in,out] err A HelicsError object, can be NULL if the errors are to be ignored.

 */
HELICS_EXPORT void helicsCoreMakeConnections(HelicsCore core, const char* file, HelicsError* err);

/**
 * Get an identifier for the broker.
 *
 * @param broker The broker to query.
 *
 * @return A string containing the identifier for the broker.
 */
HELICS_EXPORT const char* helicsBrokerGetIdentifier(HelicsBroker broker);

/**
 * Get an identifier for the core.
 *
 * @param core The core to query.
 *
 * @return A string with the identifier of the core.
 */
HELICS_EXPORT const char* helicsCoreGetIdentifier(HelicsCore core);

/**
 * Get the network address associated with a broker.
 *
 * @param broker The broker to query.
 *
 * @return A string with the network address of the broker.
 */
HELICS_EXPORT const char* helicsBrokerGetAddress(HelicsBroker broker);

/**
 * Get the network address associated with a core.
 *
 * @param core The core to query.
 *
 * @return A string with the network address of the broker.
 */
HELICS_EXPORT const char* helicsCoreGetAddress(HelicsCore core);

/**
 * Set the core to ready for init.
 *
 * @details This function is used for cores that have filters but no federates so there needs to be
 *          a direct signal to the core to trigger the federation initialization.
 *
 * @param core The core object to enable init values for.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsCoreSetReadyToInit(HelicsCore core, HelicsError* err);

/**
 * Connect a core to the federate based on current configuration.
 *
 * @param core The core to connect.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 * @return HELICS_FALSE if not connected, HELICS_TRUE if it is connected.
 */
HELICS_EXPORT HelicsBool helicsCoreConnect(HelicsCore core, HelicsError* err);

/**
 * Disconnect a core from the federation.
 *
 * @param core The core to query.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsCoreDisconnect(HelicsCore core, HelicsError* err);

/**
 * Get an existing federate object from a core by name.
 *
 * @details The federate must have been created by one of the other functions and at least one of the objects referencing the created
 *          federate must still be active in the process.
 *
 * @param fedName The name of the federate to retrieve.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 * @return NULL if no fed is available by that name otherwise a HelicsFederate with that name.
 */
HELICS_EXPORT HelicsFederate helicsGetFederateByName(const char* fedName, HelicsError* err);

/**
 * Disconnect a broker.
 *
 * @param broker The broker to disconnect.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsBrokerDisconnect(HelicsBroker broker, HelicsError* err);

/**
 * Disconnect and free a federate.
 */
HELICS_EXPORT void helicsFederateDestroy(HelicsFederate fed);

/**
 * Disconnect and free a broker.
 */
HELICS_EXPORT void helicsBrokerDestroy(HelicsBroker broker);

/**
 * Disconnect and free a core.
 */
HELICS_EXPORT void helicsCoreDestroy(HelicsCore core);

/**
 * Release the memory associated with a core.
 */
HELICS_EXPORT void helicsCoreFree(HelicsCore core);

/**
 * Release the memory associated with a broker.
 */
HELICS_EXPORT void helicsBrokerFree(HelicsBroker broker);

/*
 * Creation and destruction of Federates.
 */

/**
 * Create a value federate from a federate info object.
 *
 * @details HelicsFederate objects can be used in all functions that take a HelicsFederate or HelicsFederate object as an argument.
 *
 * @param fedName The name of the federate to create, can NULL or an empty string to use the default name from fedInfo or an assigned name.
 * @param fedInfo The federate info object that contains details on the federate.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 * @return An opaque value federate object.
 */
HELICS_EXPORT HelicsFederate helicsCreateValueFederate(const char* fedName, HelicsFederateInfo fedInfo, HelicsError* err);

/**
 * Create a value federate from a JSON file, JSON string, or TOML file.
 *
 * @details HelicsFederate objects can be used in all functions that take a HelicsFederate or HelicsFederate object as an argument.
 *
 * @param configFile A JSON file or a JSON string or TOML file that contains setup and configuration information.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 * @return An opaque value federate object.
 */
HELICS_EXPORT HelicsFederate helicsCreateValueFederateFromConfig(const char* configFile, HelicsError* err);

/**
 * Create a message federate from a federate info object.
 *
 * @details helics_message_federate objects can be used in all functions that take a helics_message_federate or HelicsFederate object as an
 * argument.
 *
 * @param fedName The name of the federate to create.
 * @param fedInfo The federate info object that contains details on the federate.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 * @return An opaque message federate object.
 */
HELICS_EXPORT HelicsFederate helicsCreateMessageFederate(const char* fedName, HelicsFederateInfo fedInfo, HelicsError* err);

/**
 * Create a message federate from a JSON file or JSON string or TOML file.
 *
 * @details helics_message_federate objects can be used in all functions that take a helics_message_federate or HelicsFederate object as an
 * argument.
 *
 * @param configFile A Config(JSON,TOML) file or a JSON string that contains setup and configuration information.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 * @return An opaque message federate object.
 */
HELICS_EXPORT HelicsFederate helicsCreateMessageFederateFromConfig(const char* configFile, HelicsError* err);

/**
 * Create a combination federate from a federate info object.
 *
 * @details Combination federates are both value federates and message federates, objects can be used in all functions
 *                      that take a HelicsFederate, helics_message_federate or HelicsFederate object as an argument
 *
 * @param fedName A string with the name of the federate, can be NULL or an empty string to pull the default name from fedInfo.
 * @param fedInfo The federate info object that contains details on the federate.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 * @return An opaque value federate object nullptr if the object creation failed.
 */
HELICS_EXPORT HelicsFederate helicsCreateCombinationFederate(const char* fedName, HelicsFederateInfo fedInfo, HelicsError* err);

/**
 * Create a combination federate from a JSON file or JSON string or TOML file.
 *
 * @details Combination federates are both value federates and message federates, objects can be used in all functions
 *          that take a HelicsFederate, helics_message_federate or HelicsFederate object as an argument
 *
 * @param configFile A JSON file or a JSON string,TOML file, or command line arguments that contains setup and configuration information.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 * @return An opaque combination federate object.
 */
HELICS_EXPORT HelicsFederate helicsCreateCombinationFederateFromConfig(const char* configFile, HelicsError* err);

/**
* Create a callback federate from a federate info object.
*
* @details Callback federates are combination federates that run a series of callback for execution in a completely automated fashion.
*
* @param fedName A string with the name of the federate, can be NULL or an empty string to pull the default name from fedInfo.
* @param fedInfo The federate info object that contains details on the federate.
*
* @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

*
* @return An opaque value federate object; nullptr if the object creation failed.
*/
HELICS_EXPORT HelicsFederate helicsCreateCallbackFederate(const char* fedName, HelicsFederateInfo fedInfo, HelicsError* err);

/**
* Create a callback federate from a JSON file or JSON string or TOML file.
*
* @details Callback federates are combination federates that run a series of callbacks for execution in a completely automated fashion.
* The enterInitializingMode call transfers complete control of the federate to the Core and no further user interaction is expected.
*
* @param configFile A JSON file or a JSON string or TOML file that contains setup and configuration information.
*
* @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

*
* @return An opaque combination federate object.
*/
HELICS_EXPORT HelicsFederate helicsCreateCallbackFederateFromConfig(const char* configFile, HelicsError* err);

/**
 * Create a new reference to an existing federate.
 *
 * @details This will create a new HelicsFederate object that references the existing federate. The new object must be freed as well.
 *
 * @param fed An existing HelicsFederate.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 * @return A new reference to the same federate.
 */
HELICS_EXPORT HelicsFederate helicsFederateClone(HelicsFederate fed, HelicsError* err);

/**
 * Protect a federate from finalizing and closing if all references go out of scope
 *
 * @details this function allows a federate to be retrieved on demand, it must be explicitly close later otherwise it will be destroyed
 * when the library is closed
 *
 * @param fedName The name of an existing HelicsFederate.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error
 occurred during the execution of the function, in particular if no federate with the given name exists
 */
HELICS_EXPORT void helicsFederateProtect(const char* fedName, HelicsError* err);

/**
 * remove the protection of an existing federate
 *
 * @details this function allows a federate to be retrieved on demand, it must be explicitly close
 later otherwise it will be destroyed
 * when the library is closed
 *
 * @param fedName the name of an existing federate that should not be protected
 *
 * @param[in,out] err An error object that will contain an error code and string if the federate was not found.
 */
HELICS_EXPORT void helicsFederateUnProtect(const char* fedName, HelicsError* err);

/**
 * checks if an existing federate is protected
 *
 *
 * @param fedName the name of an existing federate to check the protection status
 *
 * @param[in,out] err An error object that will contain an error code and string if the federate was not found.
 */
HELICS_EXPORT HelicsBool helicsFederateIsProtected(const char* fedName, HelicsError* err);

/**
 * Create a federate info object for specifying federate information when constructing a federate.
 *
 * @return A HelicsFederateInfo object which is a reference to the created object.
 */
HELICS_EXPORT HelicsFederateInfo helicsCreateFederateInfo(void);

/**
 * Create a federate info object from an existing one and clone the information.
 *
 * @param fedInfo A federateInfo object to duplicate.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 *  @return A HelicsFederateInfo object which is a reference to the created object.
 */
HELICS_EXPORT HelicsFederateInfo helicsFederateInfoClone(HelicsFederateInfo fedInfo, HelicsError* err);

/**
 * Load federate info from command line arguments.
 *
 * @param fedInfo A federateInfo object.
 * @param argc The number of command line arguments.
 * @param argv An array of strings from the command line.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoLoadFromArgs(HelicsFederateInfo fedInfo, int argc, const char* const* argv, HelicsError* err);

/**
 * Load federate info from command line arguments contained in a string.
 *
 * @param fedInfo A federateInfo object.
 * @param args Command line arguments specified in a string.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoLoadFromString(HelicsFederateInfo fedInfo, const char* args, HelicsError* err);

/**
 * Delete the memory associated with a federate info object.
 */
HELICS_EXPORT void helicsFederateInfoFree(HelicsFederateInfo fedInfo);

/**
 * Check if a federate_object is valid.
 *
 * @return HELICS_TRUE if the federate is a valid active federate, HELICS_FALSE otherwise
 */
HELICS_EXPORT HelicsBool helicsFederateIsValid(HelicsFederate fed);

/**
 * Set the name of the core to link to for a federate.
 *
 * @param fedInfo The federate info object to alter.
 * @param corename The identifier for a core to link to.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoSetCoreName(HelicsFederateInfo fedInfo, const char* corename, HelicsError* err);

/**
 * Set the initialization string for the core usually in the form of command line arguments.
 *
 * @param fedInfo The federate info object to alter.
 * @param coreInit A string containing command line arguments to be passed to the core.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoSetCoreInitString(HelicsFederateInfo fedInfo, const char* coreInit, HelicsError* err);

/**
 * Set the initialization string that a core will pass to a generated broker usually in the form of command line arguments.
 *
 * @param fedInfo The federate info object to alter.
 * @param brokerInit A string with command line arguments for a generated broker.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoSetBrokerInitString(HelicsFederateInfo fedInfo, const char* brokerInit, HelicsError* err);

/**
 * Set the core type by integer code.
 *
 * @details Valid values available by definitions in api-data.h.
 * @param fedInfo The federate info object to alter.
 * @param coretype An numerical code for a core type see /ref helics_CoreType.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoSetCoreType(HelicsFederateInfo fedInfo, int coretype, HelicsError* err);

/**
 * Set the core type from a string.
 *
 * @param fedInfo The federate info object to alter.
 * @param coretype A string naming a core type.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoSetCoreTypeFromString(HelicsFederateInfo fedInfo, const char* coretype, HelicsError* err);

/**
 * Set the name or connection information for a broker.
 *
 * @details This is only used if the core is automatically created, the broker information will be transferred to the core for connection.
 * @param fedInfo The federate info object to alter.
 * @param broker A string which defines the connection information for a broker either a name or an address.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoSetBroker(HelicsFederateInfo fedInfo, const char* broker, HelicsError* err);

/**
 * Set the key for a broker connection.
 *
 * @details This is only used if the core is automatically created, the broker information will be transferred to the core for connection.
 * @param fedInfo The federate info object to alter.
 * @param brokerkey A string containing a key for the broker to connect.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoSetBrokerKey(HelicsFederateInfo fedInfo, const char* brokerkey, HelicsError* err);

/**
 * Set the port to use for the broker.
 *
 * @details This is only used if the core is automatically created, the broker information will be transferred to the core for connection.
 * This will only be useful for network broker connections.
 * @param fedInfo The federate info object to alter.
 * @param brokerPort The integer port number to use for connection with a broker.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoSetBrokerPort(HelicsFederateInfo fedInfo, int brokerPort, HelicsError* err);

/**
 * Set the local port to use.
 *
 * @details This is only used if the core is automatically created, the port information will be transferred to the core for connection.
 * @param fedInfo The federate info object to alter.
 * @param localPort A string with the port information to use as the local server port can be a number or "auto" or "os_local".
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoSetLocalPort(HelicsFederateInfo fedInfo, const char* localPort, HelicsError* err);

/**
 * Get a property index for use in /ref helicsFederateInfoSetFlagOption, /ref helicsFederateInfoSetTimeProperty,
 * or /ref helicsFederateInfoSetIntegerProperty
 * @param val A string with the property name.
 * @return An int with the property code or (-1) if not a valid property.
 */
HELICS_EXPORT int helicsGetPropertyIndex(const char* val);

/**
 * Get a property index for use in /ref helicsFederateInfoSetFlagOption, /ref helicsFederateSetFlagOption,
 * @param val A string with the option name.
 * @return An int with the property code or (-1) if not a valid property.
 */
HELICS_EXPORT int helicsGetFlagIndex(const char* val);

/**
 * Get an option index for use in /ref helicsPublicationSetOption, /ref helicsInputSetOption, /ref helicsEndpointSetOption,
 * /ref helicsFilterSetOption, and the corresponding get functions.
 *
 * @param val A string with the option name.
 *
 * @return An int with the option index or (-1) if not a valid property.
 */
HELICS_EXPORT int helicsGetOptionIndex(const char* val);

/**
 * Get an option value for use in /ref helicsPublicationSetOption, /ref helicsInputSetOption, /ref helicsEndpointSetOption,
 * /ref helicsFilterSetOption.
 *
 * @param val A string representing the value.
 *
 * @return An int with the option value or (-1) if not a valid value.
 */
HELICS_EXPORT int helicsGetOptionValue(const char* val);

/**
 * Get the data type for use in /ref helicsFederateRegisterPublication, /ref helicsFederateRegisterInput,
 * /ref helicsFilterSetOption.
 *
 * @param val A string representing a data type.
 *
 * @return An int with the data type or HELICS_DATA_TYPE_UNKNOWN(-1) if not a valid value.
 */
HELICS_EXPORT int helicsGetDataType(const char* val);

/**
 * Set a flag in the info structure.
 *
 * @details Valid flags are available /ref helics_federate_flags.
 * @param fedInfo The federate info object to alter.
 * @param flag A numerical index for a flag.
 * @param value The desired value of the flag HELICS_TRUE or HELICS_FALSE.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateInfoSetFlagOption(HelicsFederateInfo fedInfo, int flag, HelicsBool value, HelicsError* err);

/**
 * Set the separator character in the info structure.
 *
 * @details The separator character is the separation character for local publications/endpoints in creating their global name.
 * For example if the separator character is '/'  then a local endpoint would have a globally reachable name of fedName/localName.
 * @param fedInfo The federate info object to alter.
 * @param separator The character to use as a separator.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateInfoSetSeparator(HelicsFederateInfo fedInfo, char separator, HelicsError* err);

/**
 * Set the output delay for a federate.
 *
 * @param fedInfo The federate info object to alter.
 * @param timeProperty An integer representation of the time based property to set see /ref helics_properties.
 * @param propertyValue The value of the property to set the timeProperty to.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void
    helicsFederateInfoSetTimeProperty(HelicsFederateInfo fedInfo, int timeProperty, HelicsTime propertyValue, HelicsError* err);

// TODO(Dheepak): what are known properties. The docstring should reference all properties that can be passed here.
/**
 * Set an integer property for a federate.
 *
 * @details Set known properties.
 *
 * @param fedInfo The federateInfo object to alter.
 * @param intProperty An int identifying the property.
 * @param propertyValue The value to set the property to.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateInfoSetIntegerProperty(HelicsFederateInfo fedInfo, int intProperty, int propertyValue, HelicsError* err);

/**
 * Load interfaces from a file.
 *
 * @param fed The federate to which to load interfaces.
 * @param file The name of a file to load the interfaces from either JSON, or TOML.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateRegisterInterfaces(HelicsFederate fed, const char* file, HelicsError* err);

/**
 * Generate a global error from a federate.
 *
 * @details A global error halts the co-simulation completely.
 *
 * @param fed The federate to create an error in.
 * @param errorCode The integer code for the error.
 * @param errorString A string describing the error.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateGlobalError(HelicsFederate fed, int errorCode, const char* errorString, HelicsError* err);

/**
 * Generate a local error in a federate.
 *
 * @details This will propagate through the co-simulation but not necessarily halt the co-simulation, it has a similar effect to finalize
 * but does allow some interaction with a core for a brief time.
 * @param fed The federate to create an error in.
 * @param errorCode The integer code for the error.
 * @param errorString A string describing the error.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateLocalError(HelicsFederate fed, int errorCode, const char* errorString, HelicsError* err);

/**
 * Disconnect/finalize the federate. This function halts all communication in the federate and disconnects it from the core.
 */
HELICS_EXPORT void helicsFederateFinalize(HelicsFederate fed, HelicsError* err);

/**
 * Disconnect/finalize the federate in an async call.
 */
HELICS_EXPORT void helicsFederateFinalizeAsync(HelicsFederate fed, HelicsError* err);

/**
 * Complete the asynchronous disconnect/finalize call.
 */
HELICS_EXPORT void helicsFederateFinalizeComplete(HelicsFederate fed, HelicsError* err);

/**
 * Disconnect/finalize the federate. This function halts all communication in the federate and disconnects it
 * from the core.  This call is identical to helicsFederateFinalize.
 */
HELICS_EXPORT void helicsFederateDisconnect(HelicsFederate fed, HelicsError* err);

/**
 * Disconnect/finalize the federate in an async call.  This call is identical to helicsFederateFinalizeAsync.
 */
HELICS_EXPORT void helicsFederateDisconnectAsync(HelicsFederate fed, HelicsError* err);

/**
 * Complete the asynchronous disconnect/finalize call.  This call is identical to helicsFederateFinalizeComplete
 */
HELICS_EXPORT void helicsFederateDisconnectComplete(HelicsFederate fed, HelicsError* err);

/**
 * Release the memory associated with a federate.
 */
HELICS_EXPORT void helicsFederateFree(HelicsFederate fed);

/**
 * Call when done using the helics library.
 * This function will ensure the threads are closed properly. If possible this should be the last call before exiting.
 */
HELICS_EXPORT void helicsCloseLibrary(void);

/*
 * Initialization, execution, and time requests.
 */

/**
 * Enter the initialization state of a federate.
 *
 * @details The initialization state allows initial values to be set and received if the iteration is requested on entry to the execution
 * state. This is a blocking call and will block until the core allows it to proceed.
 *
 * @param fed The federate to operate on.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateEnterInitializingMode(HelicsFederate fed, HelicsError* err);

/**
 * Non blocking alternative to \ref helicsFederateEnterInitializingMode.
 *
 * @details The function helicsFederateEnterInitializationModeComplete must be called to finish the operation.
 *
 * @param fed The federate to operate on.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateEnterInitializingModeAsync(HelicsFederate fed, HelicsError* err);

/**
 * Complete the entry to initialize mode that was initiated with /ref heliceEnterInitializingModeAsync.
 *
 * @param fed The federate desiring to complete the initialization step.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateEnterInitializingModeComplete(HelicsFederate fed, HelicsError* err);

/**
 * Trigger a blocking call and return to created state after all federates have either triggered an iteration or are waiting to enter
 * initializing mode.
 *
 * @details This call will return the federate to the created state to allow additional setup to occur with federates either iterating in
 * the mode or waiting.
 *
 * @param fed The federate to operate on.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateEnterInitializingModeIterative(HelicsFederate fed, HelicsError* err);

/**
 * Non blocking alternative to \ref helicsFederateEnterInitializingModeIterative.
 *
 * @details The function helicsFederateEnterInitializationModeIterativeComplete must be called to finish the operation.
 *
 * @param fed The federate to operate on.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateEnterInitializingModeIterativeAsync(HelicsFederate fed, HelicsError* err);

/**
 * Complete the call to enter initializing mode Iterative that was initiated with /ref heliceEnterInitializingModeIterativeAsync.  The
 * federate will be in created or error mode on return
 *
 * @param fed The federate used in the corresponding async call
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateEnterInitializingModeIterativeComplete(HelicsFederate fed, HelicsError* err);

/**
 * Check if the current Asynchronous operation has completed.
 *
 * @param fed The federate to operate on.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 *
 * @return HELICS_FALSE if not completed, HELICS_TRUE if completed.
 */
HELICS_EXPORT HelicsBool helicsFederateIsAsyncOperationCompleted(HelicsFederate fed, HelicsError* err);

/**
 * Request that the federate enter the Execution mode.
 *
 * @details This call is blocking until granted entry by the core object. On return from this call the federate will be at time 0.
 *          For an asynchronous alternative call see /ref helicsFederateEnterExecutingModeAsync.
 *
 * @param fed A federate to change modes.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateEnterExecutingMode(HelicsFederate fed, HelicsError* err);

/**
 * Request that the federate enter the Execution mode.
 *
 * @details This call is non-blocking and will return immediately. Call /ref helicsFederateEnterExecutingModeComplete to finish the call
 * sequence.
 *
 * @param fed The federate object to complete the call.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateEnterExecutingModeAsync(HelicsFederate fed, HelicsError* err);

/**
 * Complete the call to /ref helicsFederateEnterExecutingModeAsync.
 *
 * @param fed The federate object to complete the call.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateEnterExecutingModeComplete(HelicsFederate fed, HelicsError* err);

/**
 * Request an iterative time.
 *
 * @details This call allows for finer grain control of the iterative process than /ref helicsFederateRequestTime. It takes a time and
 *          iteration request, and returns a time and iteration status.
 *
 * @param fed The federate to make the request of.
 * @param iterate The requested iteration mode.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 *
 * @return An iteration structure with field containing the time and iteration status.
 */
HELICS_EXPORT HelicsIterationResult helicsFederateEnterExecutingModeIterative(HelicsFederate fed,
                                                                              HelicsIterationRequest iterate,
                                                                              HelicsError* err);

/**
 * Request an iterative entry to the execution mode.
 *
 * @details This call allows for finer grain control of the iterative process than /ref helicsFederateRequestTime. It takes a time and
 *          iteration request, and returns a time and iteration status
 *
 * @param fed The federate to make the request of.
 * @param iterate The requested iteration mode.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateEnterExecutingModeIterativeAsync(HelicsFederate fed, HelicsIterationRequest iterate, HelicsError* err);

/**
 * Complete the asynchronous iterative call into ExecutionMode.
 *
 * @param fed The federate to make the request of.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 *
 * @return An iteration object containing the iteration time and iteration_status.
 */
HELICS_EXPORT HelicsIterationResult helicsFederateEnterExecutingModeIterativeComplete(HelicsFederate fed, HelicsError* err);

/**
 * Get the current state of a federate.
 *
 * @param fed The federate to query.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * The err object will be removed in a future release as it is not necessary for use the function will not error, invalid federate return
 * HELICS_STATE_UNKOWN
 *
 * @return State the resulting state if the federate is invalid will return HELICS_STATE_UNKNOWN
 */
HELICS_EXPORT HelicsFederateState helicsFederateGetState(HelicsFederate fed, HelicsError* err);

/**
 * Get the core object associated with a federate.
 *
 * @param fed A federate object.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 *
 * @return A core object, nullptr if invalid.
 */
HELICS_EXPORT HelicsCore helicsFederateGetCore(HelicsFederate fed, HelicsError* err);

/**
 * Request the next time for federate execution.
 *
 * @param fed The federate to make the request of.
 * @param requestTime The next requested time.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 *
 * @return The time granted to the federate, will return HELICS_TIME_MAXTIME if the simulation has terminated or is invalid.
 */
HELICS_EXPORT HelicsTime helicsFederateRequestTime(HelicsFederate fed, HelicsTime requestTime, HelicsError* err);

/**
 * Request the next time for federate execution.
 *
 * @param fed The federate to make the request of.
 * @param timeDelta The requested amount of time to advance.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 *
 * @return The time granted to the federate, will return HELICS_TIME_MAXTIME if the simulation has terminated or is invalid
 */
HELICS_EXPORT HelicsTime helicsFederateRequestTimeAdvance(HelicsFederate fed, HelicsTime timeDelta, HelicsError* err);

/**
 * Request the next time step for federate execution.
 *
 * @details Feds should have setup the period or minDelta for this to work well but it will request the next time step which is the current
 * time plus the minimum time step.
 *
 * @param fed The federate to make the request of.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 *
 * @return The time granted to the federate, will return HELICS_TIME_MAXTIME if the simulation has terminated or is invalid
 */
HELICS_EXPORT HelicsTime helicsFederateRequestNextStep(HelicsFederate fed, HelicsError* err);

/**
 * Request an iterative time.
 *
 * @details This call allows for finer grain control of the iterative process than /ref helicsFederateRequestTime. It takes a time and and
 * iteration request, and returns a time and iteration status.
 *
 * @param fed The federate to make the request of.
 * @param requestTime The next desired time.
 * @param iterate The requested iteration mode.
 *
 * @param[out] outIteration  The iteration specification of the result.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 *
 * @return The granted time, will return HELICS_TIME_MAXTIME if the simulation has terminated along with the appropriate iteration result.
 */
HELICS_EXPORT HelicsTime helicsFederateRequestTimeIterative(HelicsFederate fed,
                                                            HelicsTime requestTime,
                                                            HelicsIterationRequest iterate,
                                                            HelicsIterationResult* outIteration,
                                                            HelicsError* err);

/**
 * Request the next time for federate execution in an asynchronous call.
 *
 * @details Call /ref helicsFederateRequestTimeComplete to finish the call.
 *
 * @param fed The federate to make the request of.
 * @param requestTime The next requested time.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateRequestTimeAsync(HelicsFederate fed, HelicsTime requestTime, HelicsError* err);

/**
 * Complete an asynchronous requestTime call.
 *
 * @param fed The federate to make the request of.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 *
 * @return The time granted to the federate, will return HELICS_TIME_MAXTIME if the simulation has terminated.
 */
HELICS_EXPORT HelicsTime helicsFederateRequestTimeComplete(HelicsFederate fed, HelicsError* err);

/**
 * Request an iterative time through an asynchronous call.
 *
 * @details This call allows for finer grain control of the iterative process than /ref helicsFederateRequestTime. It takes a time and
 * iteration request, and returns a time and iteration status. Call /ref helicsFederateRequestTimeIterativeComplete to finish the process.
 *
 * @param fed The federate to make the request of.
 * @param requestTime The next desired time.
 * @param iterate The requested iteration mode.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void
    helicsFederateRequestTimeIterativeAsync(HelicsFederate fed, HelicsTime requestTime, HelicsIterationRequest iterate, HelicsError* err);

/**
 * Complete an iterative time request asynchronous call.
 *
 * @param fed The federate to make the request of.
 *
 * @param[out] outIterate The iteration specification of the result.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 *
 * @return The granted time, will return HELICS_TIME_MAXTIME if the simulation has terminated.
 */
HELICS_EXPORT HelicsTime helicsFederateRequestTimeIterativeComplete(HelicsFederate fed,
                                                                    HelicsIterationResult* outIterate,
                                                                    HelicsError* err);
/**
 * Tell helics to process internal communications for a period of time.
 *
 * @param fed The federate to tell to process.
 *
 * @param period The length of time to process communications and then return control.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 *
 */
HELICS_EXPORT void helicsFederateProcessCommunications(HelicsFederate fed, HelicsTime period, HelicsError* err);

/**
 * Get the name of the federate.
 *
 * @param fed The federate object to query.
 *
 * @return A pointer to a string with the name.
 */
HELICS_EXPORT const char* helicsFederateGetName(HelicsFederate fed);

/**
 * Set a time based property for a federate.
 *
 * @param fed The federate object to set the property for.
 * @param timeProperty A integer code for a time property.
 * @param time The requested value of the property.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateSetTimeProperty(HelicsFederate fed, int timeProperty, HelicsTime time, HelicsError* err);

/**
 * Set a flag for the federate.
 *
 * @param fed The federate to alter a flag for.
 * @param flag The flag to change.
 * @param flagValue The new value of the flag. 0 for false, !=0 for true.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateSetFlagOption(HelicsFederate fed, int flag, HelicsBool flagValue, HelicsError* err);

/**
 * Set the separator character in a federate.
 *
 * @details The separator character is the separation character for local publications/endpoints in creating their global name.
 *          For example if the separator character is '/' then a local endpoint would have a globally reachable name of fedName/localName.
 *
 * @param fed The federate info object to alter.
 * @param separator The character to use as a separator.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateSetSeparator(HelicsFederate fed, char separator, HelicsError* err);

/**
 * Set an integer based property of a federate.
 *
 * @param fed The federate to change the property for.
 * @param intProperty The property to set.
 * @param propertyVal The value of the property.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateSetIntegerProperty(HelicsFederate fed, int intProperty, int propertyVal, HelicsError* err);

/**
 * Get the current value of a time based property in a federate.
 *
 * @param fed The federate query.
 * @param timeProperty The property to query.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT HelicsTime helicsFederateGetTimeProperty(HelicsFederate fed, int timeProperty, HelicsError* err);

/**
 * Get a flag value for a federate.
 *
 * @param fed The federate to get the flag for.
 * @param flag The flag to query.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 *
 * @return The value of the flag.
 */
HELICS_EXPORT HelicsBool helicsFederateGetFlagOption(HelicsFederate fed, int flag, HelicsError* err);

/**
 * Get the current value of an integer property (such as a logging level).
 *
 * @param fed The federate to get the flag for.
 * @param intProperty A code for the property to set /ref helics_handle_options.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 *
 * @return The value of the property.
 */
HELICS_EXPORT int helicsFederateGetIntegerProperty(HelicsFederate fed, int intProperty, HelicsError* err);

/**
 * Get the current time of the federate.
 *
 * @param fed The federate object to query.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 *
 * @return The current time of the federate.
 */
HELICS_EXPORT HelicsTime helicsFederateGetCurrentTime(HelicsFederate fed, HelicsError* err);

/**
 * Create an alias for an interface.
 *
 * @param fed The federate to use to set the alias.
 * @param interfaceName The current name of an interface.
 * @param alias The additional name to use for the given interface.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsFederateAddAlias(HelicsFederate fed, const char* interfaceName, const char* alias, HelicsError* err);

/**
 * Set a federation global value through a federate.
 *
 * @details This overwrites any previous value for this name.
 * @param fed The federate to set the global through.
 * @param valueName The name of the global to set.
 * @param value The value of the global.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsFederateSetGlobal(HelicsFederate fed, const char* valueName, const char* value, HelicsError* err);

/**
 * Set a federate tag value.
 *
 * @details This overwrites any previous value for this tag.
 * @param fed The federate to set the tag for.
 * @param tagName The name of the tag to set.
 * @param value The value of the tag.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsFederateSetTag(HelicsFederate fed, const char* tagName, const char* value, HelicsError* err);

/**
 * Get a federate tag value.
 *
 * @param fed The federate to get the tag for.
 * @param tagName The name of the tag to query.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT const char* helicsFederateGetTag(HelicsFederate fed, const char* tagName, HelicsError* err);

/**
 * Add a time dependency for a federate. The federate will depend on the given named federate for time synchronization.
 *
 * @param fed The federate to add the dependency for.
 * @param fedName The name of the federate to depend on.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsFederateAddDependency(HelicsFederate fed, const char* fedName, HelicsError* err);

/**
 * Set the logging file for a federate (actually on the core associated with a federate).
 *
 * @param fed The federate to set the log file for.
 * @param logFile The name of the log file.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsFederateSetLogFile(HelicsFederate fed, const char* logFile, HelicsError* err);

/**
 * Log an error message through a federate.
 *
 * @param fed The federate to log the error message through.
 * @param logmessage The message to put in the log.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsFederateLogErrorMessage(HelicsFederate fed, const char* logmessage, HelicsError* err);

/**
 * Log a warning message through a federate.
 *
 * @param fed The federate to log the warning message through.
 * @param logmessage The message to put in the log.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsFederateLogWarningMessage(HelicsFederate fed, const char* logmessage, HelicsError* err);

/**
 * Log an info message through a federate.
 *
 * @param fed The federate to log the info message through.
 * @param logmessage The message to put in the log.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsFederateLogInfoMessage(HelicsFederate fed, const char* logmessage, HelicsError* err);

/**
 * Log a debug message through a federate.
 *
 * @param fed The federate to log the debug message through.
 * @param logmessage The message to put in the log.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsFederateLogDebugMessage(HelicsFederate fed, const char* logmessage, HelicsError* err);

/**
 * Log a message through a federate.
 *
 * @param fed The federate to log the message through.
 * @param loglevel The level of the message to log see /ref helics_log_levels.
 * @param logmessage The message to put in the log.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsFederateLogLevelMessage(HelicsFederate fed, int loglevel, const char* logmessage, HelicsError* err);

/**
 * Send a command to another helics object through a federate.
 *
 * @param fed The federate to send the command through.
 * @param target The name of the object to send the command to.
 * @param command The command to send.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateSendCommand(HelicsFederate fed, const char* target, const char* command, HelicsError* err);

/**
 * Get a command sent to the federate.
 *
 * @param fed The federate to get the command for.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 *
 * @return A string with the command for the federate, if the string is empty no command is available.
 */
HELICS_EXPORT const char* helicsFederateGetCommand(HelicsFederate fed, HelicsError* err);

/**
 * Get the source of the most recently retrieved command sent to the federate.
 *
 * @param fed The federate to get the command for.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 *
 * @return A string with the command for the federate, if the string is empty no command is available.
 */
HELICS_EXPORT const char* helicsFederateGetCommandSource(HelicsFederate fed, HelicsError* err);

/**
 * Get a command sent to the federate. Blocks until a command is received.
 *
 * @param fed The federate to get the command for.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 *
 * @return A string with the command for the federate, if the string is empty no command is available.
 */
HELICS_EXPORT const char* helicsFederateWaitCommand(HelicsFederate fed, HelicsError* err);
/**
 * Set a global value in a core.
 *
 * @details This overwrites any previous value for this name.
 *
 * @param core The core to set the global through.
 * @param valueName The name of the global to set.
 * @param value The value of the global.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsCoreSetGlobal(HelicsCore core, const char* valueName, const char* value, HelicsError* err);

/**
 * Set a federation global value.
 *
 * @details This overwrites any previous value for this name.
 *
 * @param broker The broker to set the global through.
 * @param valueName The name of the global to set.
 * @param value The value of the global.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsBrokerSetGlobal(HelicsBroker broker, const char* valueName, const char* value, HelicsError* err);

/**
 * Create an alias for an interface.
 *
 * @param core The core to use to set the alias.
 * @param interfaceName The current name of an interface.
 * @param alias The additional name to use for the given interface.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsCoreAddAlias(HelicsCore core, const char* interfaceName, const char* alias, HelicsError* err);

/**
 * Create an alias for an interface.
 *
 * @param broker The broker to use to set the alias.
 * @param interfaceName The current name of an interface.
 * @param alias The additional name to use for the given interface.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsBrokerAddAlias(HelicsBroker broker, const char* interfaceName, const char* alias, HelicsError* err);

/**
 * Send a command to another helics object though a core using asynchronous(fast) operations.
 *
 * @param core The core to send the command through.
 * @param target The name of the object to send the command to.
 * @param command The command to send.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsCoreSendCommand(HelicsCore core, const char* target, const char* command, HelicsError* err);

/**
 * Send a command to another helics object though a core using ordered operations.
 *
 * @param core The core to send the command through.
 * @param target The name of the object to send the command to.
 * @param command The command to send.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsCoreSendOrderedCommand(HelicsCore core, const char* target, const char* command, HelicsError* err);

/**
 * Send a command to another helics object through a broker using asynchronous(fast) messages.
 *
 * @param broker The broker to send the command through.
 * @param target The name of the object to send the command to.
 * @param command The command to send.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsBrokerSendCommand(HelicsBroker broker, const char* target, const char* command, HelicsError* err);

/**
 * Send a command to another helics object through a broker using ordered sequencing.
 *
 * @param broker The broker to send the command through.
 * @param target The name of the object to send the command to.
 * @param command The command to send.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsBrokerSendOrderedCommand(HelicsBroker broker, const char* target, const char* command, HelicsError* err);

/**
 * Set the log file on a core.
 *
 * @param core The core to set the log file for.
 * @param logFileName The name of the file to log to.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsCoreSetLogFile(HelicsCore core, const char* logFileName, HelicsError* err);

/**
 * Set the log file on a broker.
 *
 * @param broker The broker to set the log file for.
 * @param logFileName The name of the file to log to.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsBrokerSetLogFile(HelicsBroker broker, const char* logFileName, HelicsError* err);

/**
 * Set a broker time barrier.
 *
 * @param broker The broker to set the time barrier for.
 * @param barrierTime The time to set the barrier at.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsBrokerSetTimeBarrier(HelicsBroker broker, HelicsTime barrierTime, HelicsError* err);

/**
 * Clear any time barrier on a broker.
 *
 * @param broker The broker to clear the barriers on.
 */
HELICS_EXPORT void helicsBrokerClearTimeBarrier(HelicsBroker broker);

/**
 * Generate a global error through a broker. This will terminate the federation.
 *
 * @param broker The broker to generate the global error on.
 * @param errorCode The error code to associate with the global error.
 * @param errorString An error message to associate with the global error.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsBrokerGlobalError(HelicsBroker broker, int errorCode, const char* errorString, HelicsError* err);

/**
 * Generate a global error through a broker. This will terminate the federation.
 *
 * @param core The core to generate the global error.
 * @param errorCode The error code to associate with the global error.
 * @param errorString An error message to associate with the global error.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsCoreGlobalError(HelicsCore core, int errorCode, const char* errorString, HelicsError* err);
/**
 * Create a query object.
 *
 * @details A query object consists of a target and query string.
 *
 * @param target The name of the target to query.
 * @param query The query to make of the target.
 */
HELICS_EXPORT HelicsQuery helicsCreateQuery(const char* target, const char* query);

/**
 * Execute a query.
 *
 * @details The call will block until the query finishes which may require communication or other delays.
 *
 * @param query The query object to use in the query.
 * @param fed A federate to send the query through.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 * @return A pointer to a string.  The string will remain valid until the query is freed or executed again.
 * The return will be nullptr if fed or query is an invalid object, the return string will be "#invalid" if the query itself was
 * invalid.
 */
HELICS_EXPORT const char* helicsQueryExecute(HelicsQuery query, HelicsFederate fed, HelicsError* err);

/**
 * Execute a query directly on a core.
 *
 * @details The call will block until the query finishes which may require communication or other delays.
 *
 * @param query The query object to use in the query.
 * @param core The core to send the query to.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 *
 * @return A pointer to a string.  The string will remain valid until the query is freed or executed again.
 * The return will be nullptr if core or query is an invalid object, the return string will be "#invalid" if the query itself was
 * invalid.
 */
HELICS_EXPORT const char* helicsQueryCoreExecute(HelicsQuery query, HelicsCore core, HelicsError* err);

/**
 * Execute a query directly on a broker.
 *
 * @details The call will block until the query finishes which may require communication or other delays.
 *
 * @param query The query object to use in the query.
 * @param broker The broker to send the query to.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 *
 * @return A pointer to a string.  The string will remain valid until the query is freed or executed again.
 * The return will be nullptr if broker or query is an invalid object, the return string will be "#invalid" if the query itself was
 * invalid
 */
HELICS_EXPORT const char* helicsQueryBrokerExecute(HelicsQuery query, HelicsBroker broker, HelicsError* err);

/**
 * Execute a query in a non-blocking call.
 *
 * @param query The query object to use in the query.
 * @param fed A federate to send the query through.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsQueryExecuteAsync(HelicsQuery query, HelicsFederate fed, HelicsError* err);

/**
 * Complete the return from a query called with /ref helicsExecuteQueryAsync.
 *
 * @details The function will block until the query completes /ref isQueryComplete can be called to determine if a query has completed or
 * not.
 *
 * @param query The query object to complete execution of.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 *
 * @return A pointer to a string. The string will remain valid until the query is freed or executed again.
 * The return will be nullptr if query is an invalid object
 */
HELICS_EXPORT const char* helicsQueryExecuteComplete(HelicsQuery query, HelicsError* err);

/**
 * Check if an asynchronously executed query has completed.
 *
 * @details This function should usually be called after a QueryExecuteAsync function has been called.
 *
 * @param query The query object to check if completed.
 *
 * @return Will return HELICS_TRUE if an asynchronous query has completed or a regular query call was made with a result,
 * and false if an asynchronous query has not completed or is invalid
 */
HELICS_EXPORT HelicsBool helicsQueryIsCompleted(HelicsQuery query);

/**
 * Update the target of a query.
 *
 * @param query The query object to change the target of.
 * @param target the name of the target to query
 *
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsQuerySetTarget(HelicsQuery query, const char* target, HelicsError* err);

/**
 * Update the queryString of a query.
 *
 * @param query The query object to change the target of.
 * @param queryString the new queryString
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsQuerySetQueryString(HelicsQuery query, const char* queryString, HelicsError* err);

/**
 * Update the ordering mode of the query, fast runs on priority channels, ordered goes on normal channels but goes in sequence
 *
 * @param query The query object to change the order for.
 * @param mode 0 for fast, 1 for ordered
 *
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsQuerySetOrdering(HelicsQuery query, int32_t mode, HelicsError* err);

/**
 * Free the memory associated with a query object.
 */
HELICS_EXPORT void helicsQueryFree(HelicsQuery query);

/**
 * Function to do some housekeeping work.
 *
 * @details This runs some cleanup routines and tries to close out any residual thread that haven't been shutdown yet.
 */
HELICS_EXPORT void helicsCleanupLibrary(void);

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif
