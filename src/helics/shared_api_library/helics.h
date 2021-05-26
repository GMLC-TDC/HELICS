/*
Copyright (c) 2017-2021,
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
 * Return an initialized error object.
 */
HELICS_EXPORT helics_error helicsErrorInitialize(void);

/**
 * Clear an error object.
 */
HELICS_EXPORT void helicsErrorClear(helics_error* err);

/** Load a signal handler that handles Ctrl-C and shuts down all HELICS brokers, cores,
and federates then exits the process.*/
HELICS_EXPORT void helicsLoadSignalHandler();

/** Clear HELICS based signal handlers.*/
HELICS_EXPORT void helicsClearSignalHandler();

/** Load a custom signal handler to execute prior to the abort signal handler.
@details  This function is not 100% reliable it will most likely work but uses some functions and
techniques that are not 100% guaranteed to work in a signal handler
and in worst case it could deadlock.  That is somewhat unlikely given usage patterns
but it is possible.  The callback has signature helics_bool(*handler)(int) and it will take the SIG_INT as an argument
and return a boolean.  If the boolean return value is helics_true (or the callback is null) the default signal handler is run after the
callback finishes; if it is helics_false the default callback is not run and the default signal handler is executed.*/
HELICS_EXPORT void helicsLoadSignalHandlerCallback(helics_bool (*handler)(int));

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
HELICS_EXPORT helics_bool helicsIsCoreTypeAvailable(const char* type);

/**
 * Create a core object.
 *
 * @param type The type of the core to create.
 * @param name The name of the core. It can be a nullptr or empty string to have a name automatically assigned.
 * @param initString An initialization string to send to the core. The format is similar to command line arguments.
 *                   Typical options include a broker name, the broker address, the number of federates, etc.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return A helics_core object.
 * @forcpponly
 * If the core is invalid, err will contain the corresponding error message and the returned object will be NULL.
 * @endforcpponly
 */
HELICS_EXPORT helics_core helicsCreateCore(const char* type, const char* name, const char* initString, helics_error* err);

/**
 * Create a core object by passing command line arguments.
 *
 * @param type The type of the core to create.
 * @param name The name of the core. It can be a nullptr or empty string to have a name automatically assigned.
 * @forcpponly
 * @param argc The number of arguments.
 * @endforcpponly
 * @param argv The list of string values from a command line.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string
 *                    if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return A helics_core object.
 */
HELICS_EXPORT helics_core
    helicsCreateCoreFromArgs(const char* type, const char* name, int argc, const char* const* argv, helics_error* err);

/**
 * Create a new reference to an existing core.
 *
 * @details This will create a new broker object that references the existing broker. The new broker object must be freed as well.
 *
 * @param core An existing helics_core.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return A new reference to the same broker.
 */
HELICS_EXPORT helics_core helicsCoreClone(helics_core core, helics_error* err);

/**
 * Check if a core object is a valid object.
 *
 * @param core The helics_core object to test.
 */
HELICS_EXPORT helics_bool helicsCoreIsValid(helics_core core);

/**
 * Create a broker object.
 *
 * @param type The type of the broker to create.
 * @param name The name of the broker. It can be a nullptr or empty string to have a name automatically assigned.
 * @param initString An initialization string to send to the core-the format is similar to command line arguments.
 *                   Typical options include a broker address such as --broker="XSSAF" if this is a subbroker, or the number of federates,
 * or the address.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return A helics_broker object.
 * @forcpponly
 * It will be NULL if there was an error indicated in the err object.
 * @endforcpponly
 */
HELICS_EXPORT helics_broker helicsCreateBroker(const char* type, const char* name, const char* initString, helics_error* err);

/**
 * Create a core object by passing command line arguments.
 *
 * @param type The type of the core to create.
 * @param name The name of the core. It can be a nullptr or empty string to have a name automatically assigned.
 * @forcpponly
 * @param argc The number of arguments.
 * @endforcpponly
 * @param argv The list of string values from a command line.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return A helics_core object.
 */
HELICS_EXPORT helics_broker
    helicsCreateBrokerFromArgs(const char* type, const char* name, int argc, const char* const* argv, helics_error* err);

/**
 * Create a new reference to an existing broker.
 *
 * @details This will create a new broker object that references the existing broker it must be freed as well.
 *
 * @param broker An existing helics_broker.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return A new reference to the same broker.
 */
HELICS_EXPORT helics_broker helicsBrokerClone(helics_broker broker, helics_error* err);

/**
 * Check if a broker object is a valid object.
 *
 * @param broker The helics_broker object to test.
 */
HELICS_EXPORT helics_bool helicsBrokerIsValid(helics_broker broker);

/**
 * Check if a broker is connected.
 *
 * @details A connected broker implies it is attached to cores or cores could reach out to communicate.
 *
 * @return helics_false if not connected.
 */
HELICS_EXPORT helics_bool helicsBrokerIsConnected(helics_broker broker);

/**
 * Link a named publication and named input using a broker.
 *
 * @param broker The broker to generate the connection from.
 * @param source The name of the publication (cannot be NULL).
 * @param target The name of the target to send the publication data (cannot be NULL).
 * @forcpponly
 * @param[in,out] err A helics_error object, can be NULL if the errors are to be ignored.
 * @endforcpponly
 */
HELICS_EXPORT void helicsBrokerDataLink(helics_broker broker, const char* source, const char* target, helics_error* err);

/**
 * Link a named filter to a source endpoint.
 *
 * @param broker The broker to generate the connection from.
 * @param filter The name of the filter (cannot be NULL).
 * @param endpoint The name of the endpoint to filter the data from (cannot be NULL).
 * @forcpponly
 * @param[in,out] err A helics_error object, can be NULL if the errors are to be ignored.
 * @endforcpponly
 */
HELICS_EXPORT void helicsBrokerAddSourceFilterToEndpoint(helics_broker broker, const char* filter, const char* endpoint, helics_error* err);

/**
 * Link a named filter to a destination endpoint.
 *
 * @param broker The broker to generate the connection from.
 * @param filter The name of the filter (cannot be NULL).
 * @param endpoint The name of the endpoint to filter the data going to (cannot be NULL).
 * @forcpponly
 * @param[in,out] err A helics_error object, can be NULL if the errors are to be ignored.
 * @endforcpponly
 */
HELICS_EXPORT void
    helicsBrokerAddDestinationFilterToEndpoint(helics_broker broker, const char* filter, const char* endpoint, helics_error* err);

/**
 * Load a file containing connection information.
 *
 * @param broker The broker to generate the connections from.
 * @param file A JSON or TOML file containing connection information.
 * @forcpponly
 * @param[in,out] err A helics_error object, can be NULL if the errors are to be ignored.
 * @endforcpponly
 */
HELICS_EXPORT void helicsBrokerMakeConnections(helics_broker broker, const char* file, helics_error* err);

/**
 * Wait for the core to disconnect.
 *
 * @param core The core to wait for.
 * @param msToWait The time out in millisecond (<0 for infinite timeout).
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return helics_true if the disconnect was successful, helics_false if there was a timeout.
 */
HELICS_EXPORT helics_bool helicsCoreWaitForDisconnect(helics_core core, int msToWait, helics_error* err);

/**
 * Wait for the broker to disconnect.
 *
 * @param broker The broker to wait for.
 * @param msToWait The time out in millisecond (<0 for infinite timeout).
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return helics_true if the disconnect was successful, helics_false if there was a timeout.
 */
HELICS_EXPORT helics_bool helicsBrokerWaitForDisconnect(helics_broker broker, int msToWait, helics_error* err);

/**
 * Check if a core is connected.
 *
 * @details A connected core implies it is attached to federates or federates could be attached to it
 *
 * @return helics_false if not connected, helics_true if it is connected.
 */
HELICS_EXPORT helics_bool helicsCoreIsConnected(helics_core core);

/**
 * Link a named publication and named input using a core.
 *
 * @param core The core to generate the connection from.
 * @param source The name of the publication (cannot be NULL).
 * @param target The name of the target to send the publication data (cannot be NULL).
 * @forcpponly
 * @param[in,out] err A helics_error object, can be NULL if the errors are to be ignored.
 * @endforcpponly
 */
HELICS_EXPORT void helicsCoreDataLink(helics_core core, const char* source, const char* target, helics_error* err);

/**
 * Link a named filter to a source endpoint.
 *
 * @param core The core to generate the connection from.
 * @param filter The name of the filter (cannot be NULL).
 * @param endpoint The name of the endpoint to filter the data from (cannot be NULL).
 * @forcpponly
 * @param[in,out] err A helics_error object, can be NULL if the errors are to be ignored.
 * @endforcpponly
 */
HELICS_EXPORT void helicsCoreAddSourceFilterToEndpoint(helics_core core, const char* filter, const char* endpoint, helics_error* err);

/**
 * Link a named filter to a destination endpoint.
 *
 * @param core The core to generate the connection from.
 * @param filter The name of the filter (cannot be NULL).
 * @param endpoint The name of the endpoint to filter the data going to (cannot be NULL).
 * @forcpponly
 * @param[in,out] err A helics_error object, can be NULL if the errors are to be ignored.
 * @endforcpponly
 */
HELICS_EXPORT void helicsCoreAddDestinationFilterToEndpoint(helics_core core, const char* filter, const char* endpoint, helics_error* err);

/**
 * Load a file containing connection information.
 *
 * @param core The core to generate the connections from.
 * @param file A JSON or TOML file containing connection information.
 * @forcpponly
 * @param[in,out] err A helics_error object, can be NULL if the errors are to be ignored.
 * @endforcpponly
 */
HELICS_EXPORT void helicsCoreMakeConnections(helics_core core, const char* file, helics_error* err);

/**
 * Get an identifier for the broker.
 *
 * @param broker The broker to query.
 *
 * @return A string containing the identifier for the broker.
 */
HELICS_EXPORT const char* helicsBrokerGetIdentifier(helics_broker broker);

/**
 * Get an identifier for the core.
 *
 * @param core The core to query.
 *
 * @return A string with the identifier of the core.
 */
HELICS_EXPORT const char* helicsCoreGetIdentifier(helics_core core);

/**
 * Get the network address associated with a broker.
 *
 * @param broker The broker to query.
 *
 * @return A string with the network address of the broker.
 */
HELICS_EXPORT const char* helicsBrokerGetAddress(helics_broker broker);

/**
 * Get the network address associated with a core.
 *
 * @param core The core to query.
 *
 * @return A string with the network address of the broker.
 */
HELICS_EXPORT const char* helicsCoreGetAddress(helics_core core);

/**
 * Set the core to ready for init.
 *
 * @details This function is used for cores that have filters but no federates so there needs to be
 *          a direct signal to the core to trigger the federation initialization.
 *
 * @param core The core object to enable init values for.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsCoreSetReadyToInit(helics_core core, helics_error* err);

/**
 * Connect a core to the federate based on current configuration.
 *
 * @param core The core to connect.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return helics_false if not connected, helics_true if it is connected.
 */
HELICS_EXPORT helics_bool helicsCoreConnect(helics_core core, helics_error* err);

/**
 * Disconnect a core from the federation.
 *
 * @param core The core to query.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsCoreDisconnect(helics_core core, helics_error* err);

/**
 * Get an existing federate object from a core by name.
 *
 * @details The federate must have been created by one of the other functions and at least one of the objects referencing the created
 *          federate must still be active in the process.
 *
 * @param fedName The name of the federate to retrieve.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return NULL if no fed is available by that name otherwise a helics_federate with that name.
 */
HELICS_EXPORT helics_federate helicsGetFederateByName(const char* fedName, helics_error* err);

/**
 * Disconnect a broker.
 *
 * @param broker The broker to disconnect.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsBrokerDisconnect(helics_broker broker, helics_error* err);

/**
 * Disconnect and free a federate.
 */
HELICS_EXPORT void helicsFederateDestroy(helics_federate fed);

/**
 * Disconnect and free a broker.
 */
HELICS_EXPORT void helicsBrokerDestroy(helics_broker broker);

/**
 * Disconnect and free a core.
 */
HELICS_EXPORT void helicsCoreDestroy(helics_core core);

/**
 * Release the memory associated with a core.
 */
HELICS_EXPORT void helicsCoreFree(helics_core core);

/**
 * Release the memory associated with a broker.
 */
HELICS_EXPORT void helicsBrokerFree(helics_broker broker);

/*
 * Creation and destruction of Federates.
 */

/**
 * Create a value federate from a federate info object.
 *
 * @details helics_federate objects can be used in all functions that take a helics_federate or helics_federate object as an argument.
 *
 * @param fedName The name of the federate to create, can NULL or an empty string to use the default name from fi or an assigned name.
 * @param fi The federate info object that contains details on the federate.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return An opaque value federate object.
 */
HELICS_EXPORT helics_federate helicsCreateValueFederate(const char* fedName, helics_federate_info fi, helics_error* err);

/**
 * Create a value federate from a JSON file, JSON string, or TOML file.
 *
 * @details helics_federate objects can be used in all functions that take a helics_federate or helics_federate object as an argument.
 *
 * @param configFile A JSON file or a JSON string or TOML file that contains setup and configuration information.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return An opaque value federate object.
 */
HELICS_EXPORT helics_federate helicsCreateValueFederateFromConfig(const char* configFile, helics_error* err);

/**
 * Create a message federate from a federate info object.
 *
 * @details helics_message_federate objects can be used in all functions that take a helics_message_federate or helics_federate object as an
 * argument.
 *
 * @param fedName The name of the federate to create.
 * @param fi The federate info object that contains details on the federate.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return An opaque message federate object.
 */
HELICS_EXPORT helics_federate helicsCreateMessageFederate(const char* fedName, helics_federate_info fi, helics_error* err);

/**
 * Create a message federate from a JSON file or JSON string or TOML file.
 *
 * @details helics_message_federate objects can be used in all functions that take a helics_message_federate or helics_federate object as an
 * argument.
 *
 * @param configFile A Config(JSON,TOML) file or a JSON string that contains setup and configuration information.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return An opaque message federate object.
 */
HELICS_EXPORT helics_federate helicsCreateMessageFederateFromConfig(const char* configFile, helics_error* err);

/**
 * Create a combination federate from a federate info object.
 *
 * @details Combination federates are both value federates and message federates, objects can be used in all functions
 *                      that take a helics_federate, helics_message_federate or helics_federate object as an argument
 *
 * @param fedName A string with the name of the federate, can be NULL or an empty string to pull the default name from fi.
 * @param fi The federate info object that contains details on the federate.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return An opaque value federate object nullptr if the object creation failed.
 */
HELICS_EXPORT helics_federate helicsCreateCombinationFederate(const char* fedName, helics_federate_info fi, helics_error* err);

/**
 * Create a combination federate from a JSON file or JSON string or TOML file.
 *
 * @details Combination federates are both value federates and message federates, objects can be used in all functions
 *          that take a helics_federate, helics_message_federate or helics_federate object as an argument
 *
 * @param configFile A JSON file or a JSON string or TOML file that contains setup and configuration information.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return An opaque combination federate object.
 */
HELICS_EXPORT helics_federate helicsCreateCombinationFederateFromConfig(const char* configFile, helics_error* err);

/**
 * Create a new reference to an existing federate.
 *
 * @details This will create a new helics_federate object that references the existing federate. The new object must be freed as well.
 *
 * @param fed An existing helics_federate.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return A new reference to the same federate.
 */
HELICS_EXPORT helics_federate helicsFederateClone(helics_federate fed, helics_error* err);

/**
 * Create a federate info object for specifying federate information when constructing a federate.
 *
 * @return A helics_federate_info object which is a reference to the created object.
 */
HELICS_EXPORT helics_federate_info helicsCreateFederateInfo(void);

/**
 * Create a federate info object from an existing one and clone the information.
 *
 * @param fi A federateInfo object to duplicate.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 *  @return A helics_federate_info object which is a reference to the created object.
 */
HELICS_EXPORT helics_federate_info helicsFederateInfoClone(helics_federate_info fi, helics_error* err);

/**
 * Load federate info from command line arguments.
 *
 * @param fi A federateInfo object.
 * @param argc The number of command line arguments.
 * @param argv An array of strings from the command line.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateInfoLoadFromArgs(helics_federate_info fi, int argc, const char* const* argv, helics_error* err);

/**
 * Delete the memory associated with a federate info object.
 */
HELICS_EXPORT void helicsFederateInfoFree(helics_federate_info fi);

/**
 * Check if a federate_object is valid.
 *
 * @return helics_true if the federate is a valid active federate, helics_false otherwise
 */
HELICS_EXPORT helics_bool helicsFederateIsValid(helics_federate fed);

/**
 * Set the name of the core to link to for a federate.
 *
 * @param fi The federate info object to alter.
 * @param corename The identifier for a core to link to.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateInfoSetCoreName(helics_federate_info fi, const char* corename, helics_error* err);

/**
 * Set the initialization string for the core usually in the form of command line arguments.
 *
 * @param fi The federate info object to alter.
 * @param coreInit A string containing command line arguments to be passed to the core.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateInfoSetCoreInitString(helics_federate_info fi, const char* coreInit, helics_error* err);

/**
 * Set the initialization string that a core will pass to a generated broker usually in the form of command line arguments.
 *
 * @param fi The federate info object to alter.
 * @param brokerInit A string with command line arguments for a generated broker.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateInfoSetBrokerInitString(helics_federate_info fi, const char* brokerInit, helics_error* err);

/**
 * Set the core type by integer code.
 *
 * @details Valid values available by definitions in api-data.h.
 * @param fi The federate info object to alter.
 * @param coretype An numerical code for a core type see /ref helics_core_type.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateInfoSetCoreType(helics_federate_info fi, int coretype, helics_error* err);

/**
 * Set the core type from a string.
 *
 * @param fi The federate info object to alter.
 * @param coretype A string naming a core type.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateInfoSetCoreTypeFromString(helics_federate_info fi, const char* coretype, helics_error* err);

/**
 * Set the name or connection information for a broker.
 *
 * @details This is only used if the core is automatically created, the broker information will be transferred to the core for connection.
 * @param fi The federate info object to alter.
 * @param broker A string which defines the connection information for a broker either a name or an address.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateInfoSetBroker(helics_federate_info fi, const char* broker, helics_error* err);

/**
 * Set the key for a broker connection.
 *
 * @details This is only used if the core is automatically created, the broker information will be transferred to the core for connection.
 * @param fi The federate info object to alter.
 * @param brokerkey A string containing a key for the broker to connect.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateInfoSetBrokerKey(helics_federate_info fi, const char* brokerkey, helics_error* err);

/**
 * Set the port to use for the broker.
 *
 * @details This is only used if the core is automatically created, the broker information will be transferred to the core for connection.
 * This will only be useful for network broker connections.
 * @param fi The federate info object to alter.
 * @param brokerPort The integer port number to use for connection with a broker.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateInfoSetBrokerPort(helics_federate_info fi, int brokerPort, helics_error* err);

/**
 * Set the local port to use.
 *
 * @details This is only used if the core is automatically created, the port information will be transferred to the core for connection.
 * @param fi The federate info object to alter.
 * @param localPort A string with the port information to use as the local server port can be a number or "auto" or "os_local".
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateInfoSetLocalPort(helics_federate_info fi, const char* localPort, helics_error* err);

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
 * Set a flag in the info structure.
 *
 * @details Valid flags are available /ref helics_federate_flags.
 * @param fi The federate info object to alter.
 * @param flag A numerical index for a flag.
 * @param value The desired value of the flag helics_true or helics_false.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateInfoSetFlagOption(helics_federate_info fi, int flag, helics_bool value, helics_error* err);

/**
 * Set the separator character in the info structure.
 *
 * @details The separator character is the separation character for local publications/endpoints in creating their global name.
 * For example if the separator character is '/'  then a local endpoint would have a globally reachable name of fedName/localName.
 * @param fi The federate info object to alter.
 * @param separator The character to use as a separator.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateInfoSetSeparator(helics_federate_info fi, char separator, helics_error* err);

/**
 * Set the output delay for a federate.
 *
 * @param fi The federate info object to alter.
 * @param timeProperty An integer representation of the time based property to set see /ref helics_properties.
 * @param propertyValue The value of the property to set the timeProperty to.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void
    helicsFederateInfoSetTimeProperty(helics_federate_info fi, int timeProperty, helics_time propertyValue, helics_error* err);

// TODO(Dheepak): what are known properties. The docstring should reference all properties that can be passed here.
/**
 * Set an integer property for a federate.
 *
 * @details Set known properties.
 *
 * @param fi The federateInfo object to alter.
 * @param intProperty An int identifying the property.
 * @param propertyValue The value to set the property to.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateInfoSetIntegerProperty(helics_federate_info fi, int intProperty, int propertyValue, helics_error* err);

/**
 * Load interfaces from a file.
 *
 * @param fed The federate to which to load interfaces.
 * @param file The name of a file to load the interfaces from either JSON, or TOML.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateRegisterInterfaces(helics_federate fed, const char* file, helics_error* err);

/**
 * Generate a global error from a federate.
 *
 * @details A global error halts the co-simulation completely.
 *
 * @param fed The federate to create an error in.
 * @param errorCode The integer code for the error.
 * @param errorString A string describing the error.
 */
HELICS_EXPORT void helicsFederateGlobalError(helics_federate fed, int errorCode, const char* errorString);

/**
 * Generate a local error in a federate.
 *
 * @details This will propagate through the co-simulation but not necessarily halt the co-simulation, it has a similar effect to finalize
 * but does allow some interaction with a core for a brief time.
 * @param fed The federate to create an error in.
 * @param errorCode The integer code for the error.
 * @param errorString A string describing the error.
 */
HELICS_EXPORT void helicsFederateLocalError(helics_federate fed, int errorCode, const char* errorString);

/**
 * Finalize the federate. This function halts all communication in the federate and disconnects it from the core.
 */
HELICS_EXPORT void helicsFederateFinalize(helics_federate fed, helics_error* err);

/**
 * Finalize the federate in an async call.
 */
HELICS_EXPORT void helicsFederateFinalizeAsync(helics_federate fed, helics_error* err);

/**
 * Complete the asynchronous finalize call.
 */
HELICS_EXPORT void helicsFederateFinalizeComplete(helics_federate fed, helics_error* err);

/**
 * Release the memory associated with a federate.
 */
HELICS_EXPORT void helicsFederateFree(helics_federate fed);

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
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateEnterInitializingMode(helics_federate fed, helics_error* err);

/**
 * Non blocking alternative to \ref helicsFederateEnterInitializingMode.
 *
 * @details The function helicsFederateEnterInitializationModeFinalize must be called to finish the operation.
 *
 * @param fed The federate to operate on.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateEnterInitializingModeAsync(helics_federate fed, helics_error* err);

/**
 * Check if the current Asynchronous operation has completed.
 *
 * @param fed The federate to operate on.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return helics_false if not completed, helics_true if completed.
 */
HELICS_EXPORT helics_bool helicsFederateIsAsyncOperationCompleted(helics_federate fed, helics_error* err);

/**
 * Finalize the entry to initialize mode that was initiated with /ref heliceEnterInitializingModeAsync.
 *
 * @param fed The federate desiring to complete the initialization step.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateEnterInitializingModeComplete(helics_federate fed, helics_error* err);

/**
 * Request that the federate enter the Execution mode.
 *
 * @details This call is blocking until granted entry by the core object. On return from this call the federate will be at time 0.
 *          For an asynchronous alternative call see /ref helicsFederateEnterExecutingModeAsync.
 *
 * @param fed A federate to change modes.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateEnterExecutingMode(helics_federate fed, helics_error* err);

/**
 * Request that the federate enter the Execution mode.
 *
 * @details This call is non-blocking and will return immediately. Call /ref helicsFederateEnterExecutingModeComplete to finish the call
 * sequence.
 *
 * @param fed The federate object to complete the call.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateEnterExecutingModeAsync(helics_federate fed, helics_error* err);

/**
 * Complete the call to /ref helicsFederateEnterExecutingModeAsync.
 *
 * @param fed The federate object to complete the call.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateEnterExecutingModeComplete(helics_federate fed, helics_error* err);

/**
 * Request an iterative time.
 *
 * @details This call allows for finer grain control of the iterative process than /ref helicsFederateRequestTime. It takes a time and
 *          iteration request, and returns a time and iteration status.
 *
 * @param fed The federate to make the request of.
 * @param iterate The requested iteration mode.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return An iteration structure with field containing the time and iteration status.
 */
HELICS_EXPORT helics_iteration_result helicsFederateEnterExecutingModeIterative(helics_federate fed,
                                                                                helics_iteration_request iterate,
                                                                                helics_error* err);

/**
 * Request an iterative entry to the execution mode.
 *
 * @details This call allows for finer grain control of the iterative process than /ref helicsFederateRequestTime. It takes a time and
 *          iteration request, and returns a time and iteration status
 *
 * @param fed The federate to make the request of.
 * @param iterate The requested iteration mode.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateEnterExecutingModeIterativeAsync(helics_federate fed, helics_iteration_request iterate, helics_error* err);

/**
 * Complete the asynchronous iterative call into ExecutionMode.
 *
 * @param fed The federate to make the request of.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return An iteration object containing the iteration time and iteration_status.
 */
HELICS_EXPORT helics_iteration_result helicsFederateEnterExecutingModeIterativeComplete(helics_federate fed, helics_error* err);

/**
 * Get the current state of a federate.
 *
 * @param fed The federate to query.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return State the resulting state if void return helics_ok.
 */
HELICS_EXPORT helics_federate_state helicsFederateGetState(helics_federate fed, helics_error* err);

/**
 * Get the core object associated with a federate.
 *
 * @param fed A federate object.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return A core object, nullptr if invalid.
 */
HELICS_EXPORT helics_core helicsFederateGetCoreObject(helics_federate fed, helics_error* err);

/**
 * Request the next time for federate execution.
 *
 * @param fed The federate to make the request of.
 * @param requestTime The next requested time.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return The time granted to the federate, will return helics_time_maxtime if the simulation has terminated or is invalid.
 */
HELICS_EXPORT helics_time helicsFederateRequestTime(helics_federate fed, helics_time requestTime, helics_error* err);

/**
 * Request the next time for federate execution.
 *
 * @param fed The federate to make the request of.
 * @param timeDelta The requested amount of time to advance.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return The time granted to the federate, will return helics_time_maxtime if the simulation has terminated or is invalid
 */
HELICS_EXPORT helics_time helicsFederateRequestTimeAdvance(helics_federate fed, helics_time timeDelta, helics_error* err);

/**
 * Request the next time step for federate execution.
 *
 * @details Feds should have setup the period or minDelta for this to work well but it will request the next time step which is the current
 * time plus the minimum time step.
 *
 * @param fed The federate to make the request of.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return The time granted to the federate, will return helics_time_maxtime if the simulation has terminated or is invalid
 */
HELICS_EXPORT helics_time helicsFederateRequestNextStep(helics_federate fed, helics_error* err);

/**
 * Request an iterative time.
 *
 * @details This call allows for finer grain control of the iterative process than /ref helicsFederateRequestTime. It takes a time and and
 * iteration request, and returns a time and iteration status.
 *
 * @param fed The federate to make the request of.
 * @param requestTime The next desired time.
 * @param iterate The requested iteration mode.
 * @forcpponly
 * @param[out] outIteration  The iteration specification of the result.
 * @endforcpponly
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return The granted time, will return helics_time_maxtime if the simulation has terminated along with the appropriate iteration result.
 * @beginPythonOnly
 * This function also returns the iteration specification of the result.
 * @endPythonOnly
 */
HELICS_EXPORT helics_time helicsFederateRequestTimeIterative(helics_federate fed,
                                                             helics_time requestTime,
                                                             helics_iteration_request iterate,
                                                             helics_iteration_result* outIteration,
                                                             helics_error* err);

/**
 * Request the next time for federate execution in an asynchronous call.
 *
 * @details Call /ref helicsFederateRequestTimeComplete to finish the call.
 *
 * @param fed The federate to make the request of.
 * @param requestTime The next requested time.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateRequestTimeAsync(helics_federate fed, helics_time requestTime, helics_error* err);

/**
 * Complete an asynchronous requestTime call.
 *
 * @param fed The federate to make the request of.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return The time granted to the federate, will return helics_time_maxtime if the simulation has terminated.
 */
HELICS_EXPORT helics_time helicsFederateRequestTimeComplete(helics_federate fed, helics_error* err);

/**
 * Request an iterative time through an asynchronous call.
 *
 * @details This call allows for finer grain control of the iterative process than /ref helicsFederateRequestTime. It takes a time and
 * iteration request, and returns a time and iteration status. Call /ref helicsFederateRequestTimeIterativeComplete to finish the process.
 *
 * @param fed The federate to make the request of.
 * @param requestTime The next desired time.
 * @param iterate The requested iteration mode.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateRequestTimeIterativeAsync(helics_federate fed,
                                                           helics_time requestTime,
                                                           helics_iteration_request iterate,
                                                           helics_error* err);

/**
 * Complete an iterative time request asynchronous call.
 *
 * @param fed The federate to make the request of.
 * @forcpponly
 * @param[out] outIterate The iteration specification of the result.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return The granted time, will return helics_time_maxtime if the simulation has terminated.
 * @beginPythonOnly
 * This function also returns the iteration specification of the result.
 * @endPythonOnly
 */
HELICS_EXPORT helics_time helicsFederateRequestTimeIterativeComplete(helics_federate fed,
                                                                     helics_iteration_result* outIterate,
                                                                     helics_error* err);

/**
 * Get the name of the federate.
 *
 * @param fed The federate object to query.
 *
 * @return A pointer to a string with the name.
 */
HELICS_EXPORT const char* helicsFederateGetName(helics_federate fed);

/**
 * Set a time based property for a federate.
 *
 * @param fed The federate object to set the property for.
 * @param timeProperty A integer code for a time property.
 * @param time The requested value of the property.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateSetTimeProperty(helics_federate fed, int timeProperty, helics_time time, helics_error* err);

/**
 * Set a flag for the federate.
 *
 * @param fed The federate to alter a flag for.
 * @param flag The flag to change.
 * @param flagValue The new value of the flag. 0 for false, !=0 for true.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateSetFlagOption(helics_federate fed, int flag, helics_bool flagValue, helics_error* err);

/**
 * Set the separator character in a federate.
 *
 * @details The separator character is the separation character for local publications/endpoints in creating their global name.
 *          For example if the separator character is '/' then a local endpoint would have a globally reachable name of fedName/localName.
 *
 * @param fed The federate info object to alter.
 * @param separator The character to use as a separator.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateSetSeparator(helics_federate fed, char separator, helics_error* err);

/**
 * Set an integer based property of a federate.
 *
 * @param fed The federate to change the property for.
 * @param intProperty The property to set.
 * @param propertyVal The value of the property.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateSetIntegerProperty(helics_federate fed, int intProperty, int propertyVal, helics_error* err);

/**
 * Get the current value of a time based property in a federate.
 *
 * @param fed The federate query.
 * @param timeProperty The property to query.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT helics_time helicsFederateGetTimeProperty(helics_federate fed, int timeProperty, helics_error* err);

/**
 * Get a flag value for a federate.
 *
 * @param fed The federate to get the flag for.
 * @param flag The flag to query.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return The value of the flag.
 */
HELICS_EXPORT helics_bool helicsFederateGetFlagOption(helics_federate fed, int flag, helics_error* err);

/**
 * Get the current value of an integer property (such as a logging level).
 *
 * @param fed The federate to get the flag for.
 * @param intProperty A code for the property to set /ref helics_handle_options.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return The value of the property.
 */
HELICS_EXPORT int helicsFederateGetIntegerProperty(helics_federate fed, int intProperty, helics_error* err);

/**
 * Get the current time of the federate.
 *
 * @param fed The federate object to query.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return The current time of the federate.
 */
HELICS_EXPORT helics_time helicsFederateGetCurrentTime(helics_federate fed, helics_error* err);

/**
 * Set a federation global value through a federate.
 *
 * @details This overwrites any previous value for this name.
 * @param fed The federate to set the global through.
 * @param valueName The name of the global to set.
 * @param value The value of the global.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateSetGlobal(helics_federate fed, const char* valueName, const char* value, helics_error* err);

/**
 * Add a time dependency for a federate. The federate will depend on the given named federate for time synchronization.
 *
 * @param fed The federate to add the dependency for.
 * @param fedName The name of the federate to depend on.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateAddDependency(helics_federate fed, const char* fedName, helics_error* err);

/**
 * Set the logging file for a federate (actually on the core associated with a federate).
 *
 * @param fed The federate to set the log file for.
 * @param logFile The name of the log file.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateSetLogFile(helics_federate fed, const char* logFile, helics_error* err);

/**
 * Log an error message through a federate.
 *
 * @param fed The federate to log the error message through.
 * @param logmessage The message to put in the log.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateLogErrorMessage(helics_federate fed, const char* logmessage, helics_error* err);

/**
 * Log a warning message through a federate.
 *
 * @param fed The federate to log the warning message through.
 * @param logmessage The message to put in the log.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateLogWarningMessage(helics_federate fed, const char* logmessage, helics_error* err);

/**
 * Log an info message through a federate.
 *
 * @param fed The federate to log the info message through.
 * @param logmessage The message to put in the log.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateLogInfoMessage(helics_federate fed, const char* logmessage, helics_error* err);

/**
 * Log a debug message through a federate.
 *
 * @param fed The federate to log the debug message through.
 * @param logmessage The message to put in the log.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateLogDebugMessage(helics_federate fed, const char* logmessage, helics_error* err);

/**
 * Log a message through a federate.
 *
 * @param fed The federate to log the message through.
 * @param loglevel The level of the message to log see /ref helics_log_levels.
 * @param logmessage The message to put in the log.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateLogLevelMessage(helics_federate fed, int loglevel, const char* logmessage, helics_error* err);

/**
 * Set a global value in a core.
 *
 * @details This overwrites any previous value for this name.
 *
 * @param core The core to set the global through.
 * @param valueName The name of the global to set.
 * @param value The value of the global.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsCoreSetGlobal(helics_core core, const char* valueName, const char* value, helics_error* err);

/**
 * Set a federation global value.
 *
 * @details This overwrites any previous value for this name.
 *
 * @param broker The broker to set the global through.
 * @param valueName The name of the global to set.
 * @param value The value of the global.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsBrokerSetGlobal(helics_broker broker, const char* valueName, const char* value, helics_error* err);

/**
 * Set the log file on a core.
 *
 * @param core The core to set the log file for.
 * @param logFileName The name of the file to log to.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsCoreSetLogFile(helics_core core, const char* logFileName, helics_error* err);

/**
 * Set the log file on a broker.
 *
 * @param broker The broker to set the log file for.
 * @param logFileName The name of the file to log to.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsBrokerSetLogFile(helics_broker broker, const char* logFileName, helics_error* err);

/**
 * Set a broker time barrier.
 *
 * @param broker The broker to set the time barrier for.
 * @param barrierTime The time to set the barrier at.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsBrokerSetTimeBarrier(helics_broker broker, helics_time barrierTime, helics_error* err);

/**
 * Clear any time barrier on a broker.
 *
 * @param broker The broker to clear the barriers on.
 */
HELICS_EXPORT void helicsBrokerClearTimeBarrier(helics_broker broker);

/**
 * Generate a global error through a broker. This will terminate the federation.
 *
 * @param broker The broker to generate the global error on.
 * @param errorCode The error code to associate with the global error.
 * @param errorString An error message to associate with the global error.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsBrokerGlobalError(helics_broker broker, int errorCode, const char* errorString, helics_error* err);

/**
 * Generate a global error through a broker. This will terminate the federation.
 *
 * @param core The core to generate the global error.
 * @param errorCode The error code to associate with the global error.
 * @param errorString An error message to associate with the global error.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsCoreGlobalError(helics_core core, int errorCode, const char* errorString, helics_error* err);
/**
 * Create a query object.
 *
 * @details A query object consists of a target and query string.
 *
 * @param target The name of the target to query.
 * @param query The query to make of the target.
 */
HELICS_EXPORT helics_query helicsCreateQuery(const char* target, const char* query);

/**
 * Execute a query.
 *
 * @details The call will block until the query finishes which may require communication or other delays.
 *
 * @param query The query object to use in the query.
 * @param fed A federate to send the query through.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return A pointer to a string.  The string will remain valid until the query is freed or executed again.
 * @forcpponly
 *         The return will be nullptr if fed or query is an invalid object, the return string will be "#invalid" if the query itself was
 * invalid.
 * @endforcpponly
 */
HELICS_EXPORT const char* helicsQueryExecute(helics_query query, helics_federate fed, helics_error* err);

/**
 * Execute a query directly on a core.
 *
 * @details The call will block until the query finishes which may require communication or other delays.
 *
 * @param query The query object to use in the query.
 * @param core The core to send the query to.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return A pointer to a string.  The string will remain valid until the query is freed or executed again.
 * @forcpponly
 *         The return will be nullptr if core or query is an invalid object, the return string will be "#invalid" if the query itself was
 * invalid.
 * @endforcpponly
 */
HELICS_EXPORT const char* helicsQueryCoreExecute(helics_query query, helics_core core, helics_error* err);

/**
 * Execute a query directly on a broker.
 *
 * @details The call will block until the query finishes which may require communication or other delays.
 *
 * @param query The query object to use in the query.
 * @param broker The broker to send the query to.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return A pointer to a string.  The string will remain valid until the query is freed or executed again.
 * @forcpponly
 *         The return will be nullptr if broker or query is an invalid object, the return string will be "#invalid" if the query itself was
 * invalid
 * @endforcpponly
 */
HELICS_EXPORT const char* helicsQueryBrokerExecute(helics_query query, helics_broker broker, helics_error* err);

/**
 * Execute a query in a non-blocking call.
 *
 * @param query The query object to use in the query.
 * @param fed A federate to send the query through.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsQueryExecuteAsync(helics_query query, helics_federate fed, helics_error* err);

/**
 * Complete the return from a query called with /ref helicsExecuteQueryAsync.
 *
 * @details The function will block until the query completes /ref isQueryComplete can be called to determine if a query has completed or
 * not.
 *
 * @param query The query object to complete execution of.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return A pointer to a string. The string will remain valid until the query is freed or executed again.
 * @forcpponly
 *         The return will be nullptr if query is an invalid object
 * @endforcpponly
 */
HELICS_EXPORT const char* helicsQueryExecuteComplete(helics_query query, helics_error* err);

/**
 * Check if an asynchronously executed query has completed.
 *
 * @details This function should usually be called after a QueryExecuteAsync function has been called.
 *
 * @param query The query object to check if completed.
 *
 * @return Will return helics_true if an asynchronous query has completed or a regular query call was made with a result,
 *         and false if an asynchronous query has not completed or is invalid
 */
HELICS_EXPORT helics_bool helicsQueryIsCompleted(helics_query query);

/**
 * Update the target of a query.
 *
 * @param query The query object to change the target of.
 * @param target the name of the target to query
 *
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsQuerySetTarget(helics_query query, const char* target, helics_error* err);

/**
 * Update the queryString of a query.
 *
 * @param query The query object to change the target of.
 * @param queryString the new queryString
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsQuerySetQueryString(helics_query query, const char* queryString, helics_error* err);

/**
 * Update the ordering mode of the query, fast runs on priority channels, ordered goes on normal channels but goes in sequence
 *
 * @param query The query object to change the order for.
 * @param mode 0 for fast, 1 for ordered
 *
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsQuerySetOrdering(helics_query query, int32_t mode, helics_error* err);

/**
 * Free the memory associated with a query object.
 */
HELICS_EXPORT void helicsQueryFree(helics_query query);

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
