/*
Copyright (c) 2017-2020,
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

/** @file
    @brief common functions for the HELICS C api
    */
/***************************************************
    Common Functions
    ****************************************************/

/** get a version string for HELICS */
HELICS_EXPORT const char* helicsGetVersion(void);

/** return an initialized error object*/
HELICS_EXPORT helics_error helicsErrorInitialize(void);

/** clear an error object*/
HELICS_EXPORT void helicsErrorClear(helics_error* err);

/**
     * Returns true if core/broker type specified is available in current compilation.
     @param type a string representing a core type
     @details possible options include "test","zmq","udp","ipc","interprocess","tcp","default", "mpi"
     */
HELICS_EXPORT helics_bool helicsIsCoreTypeAvailable(const char* type);

/** create a core object
    @param type the type of the core to create
    @param name the name of the core , may be a nullptr or empty string to have a name automatically assigned
    @param initString an initialization string to send to the core-the format is similar to command line arguments
    typical options include a broker address  --broker="XSSAF" or the number of federates or the address
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return a helics_core object if the core is invalid err will contain some indication
    */
HELICS_EXPORT helics_core helicsCreateCore(const char* type, const char* name, const char* initString, helics_error* err);

/** create a core object by passing command line arguments
    @param type the type of the core to create
    @param name the name of the core , may be a nullptr or empty string to have a name automatically assigned
    @param argc the number of arguments
    @param argv the string values from a command line
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return a helics_core object
    */
HELICS_EXPORT helics_core
    helicsCreateCoreFromArgs(const char* type, const char* name, int argc, const char* const* argv, helics_error* err);

/** create a new reference to an existing core
    @details this will create a new broker object that references the existing broker it must be freed as well
    @param core an existing helics_core
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return a new reference to the same broker*/
HELICS_EXPORT helics_core helicsCoreClone(helics_core core, helics_error* err);

/** check if a core object is a valid object
    @param core the helics_core object to test*/
HELICS_EXPORT helics_bool helicsCoreIsValid(helics_core core);

/** create a broker object
    @param type the type of the broker to create
    @param name the name of the broker , may be a nullptr or empty string to have a name automatically assigned
    @param initString an initialization string to send to the core-the format is similar to command line arguments
    typical options include a broker address  --broker="XSSAF" if this is a subbroker or the number of federates or the address
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return a helics_broker object, will be NULL if there was an error indicated in the err object
    */
HELICS_EXPORT helics_broker helicsCreateBroker(const char* type, const char* name, const char* initString, helics_error* err);

/** create a core object by passing command line arguments
    @param type the type of the core to create
    @param name the name of the core , may be a nullptr or empty string to have a name automatically assigned
    @param argc the number of arguments
    @param argv the string values from a command line
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return a helics_core object
    */
HELICS_EXPORT helics_broker
    helicsCreateBrokerFromArgs(const char* type, const char* name, int argc, const char* const* argv, helics_error* err);

/** create a new reference to an existing broker
    @details this will create a new broker object that references the existing broker it must be freed as well
    @param broker an existing helics_broker
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return a new reference to the same broker*/
HELICS_EXPORT helics_broker helicsBrokerClone(helics_broker broker, helics_error* err);

/** check if a broker object is a valid object
    @param broker the helics_broker object to test*/
HELICS_EXPORT helics_bool helicsBrokerIsValid(helics_broker broker);

/** check if a broker is connected
  a connected broker implies is attached to cores or cores could reach out to communicate
  return 0 if not connected , something else if it is connected*/
HELICS_EXPORT helics_bool helicsBrokerIsConnected(helics_broker broker);

/** link a named publication and named input using a broker
    @param broker the broker to generate the connection from
    @param source the name of the publication (cannot be NULL)
    @param target the name of the target to send the publication data (cannot be NULL)
    @forcpponly
    @param[in,out] err a helics_error object, can be NULL if the errors are to be ignored
    @endforcpponly
    */
HELICS_EXPORT void helicsBrokerDataLink(helics_broker broker, const char* source, const char* target, helics_error* err);

/** link a named filter to a source endpoint
    @param broker the broker to generate the connection from
    @param filter the name of the filter (cannot be NULL)
    @param endpoint the name of the endpoint to filter the data from (cannot be NULL)
    @forcpponly
    @param[in,out] err a helics_error object, can be NULL if the errors are to be ignored
    @endforcpponly
    */
HELICS_EXPORT void helicsBrokerAddSourceFilterToEndpoint(helics_broker broker, const char* filter, const char* endpoint, helics_error* err);

/** link a named filter to a destination endpoint
    @param broker the broker to generate the connection from
    @param filter the name of the filter (cannot be NULL)
    @param endpoint the name of the endpoint to filter the data going to (cannot be NULL)
    @forcpponly
    @param[in,out] err a helics_error object, can be NULL if the errors are to be ignored
    @endforcpponly
    */
HELICS_EXPORT void
    helicsBrokerAddDestinationFilterToEndpoint(helics_broker broker, const char* filter, const char* endpoint, helics_error* err);

/** load a file containing connection information
    @param broker the broker to generate the connections from
    @param file a JSON or TOML file containing connection information
    @forcpponly
    @param[in,out] err a helics_error object, can be NULL if the errors are to be ignored
    @endforcpponly
    */
HELICS_EXPORT void helicsBrokerMakeConnections(helics_broker broker, const char* file, helics_error* err);

/** wait for the core to disconnect
  @param core the core to wait for
  @param msToWait the time out in millisecond (<0 for infinite timeout)
  @forcpponly
  @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
  @endforcpponly
  @return helics_true if the disconnect was successful,  helics_false if there was a timeout
  */
HELICS_EXPORT helics_bool helicsCoreWaitForDisconnect(helics_core core, int msToWait, helics_error* err);
/** wait for the broker to disconnect
 @param broker the broker to wait for
 @param msToWait the time out in millisecond (<0 for infinite timeout)
 @forcpponly
 @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
 @endforcpponly
 @return helics_true if the disconnect was successful,  helics_false if there was a timeout
 */
HELICS_EXPORT helics_bool helicsBrokerWaitForDisconnect(helics_broker broker, int msToWait, helics_error* err);

/** check if a core is connected
    a connected core implies is attached to federate or federates could be attached to it
    return helics_false if not connected, helics_true if it is connected*/
HELICS_EXPORT helics_bool helicsCoreIsConnected(helics_core core);

/** link a named publication and named input using a core
    @param core the core to generate the connection from
    @param source the name of the publication (cannot be NULL)
    @param target the named of the target to send the publication data (cannot be NULL)
    @forcpponly
    @param[in,out] err a helics_error object, can be NULL if the errors are to be ignored
    @endforcpponly
    */
HELICS_EXPORT void helicsCoreDataLink(helics_core core, const char* source, const char* target, helics_error* err);

/** link a named filter to a source endpoint
    @param core the core to generate the connection from
    @param filter the name of the filter (cannot be NULL)
    @param endpoint the name of the endpoint to filter the data from (cannot be NULL)
    @forcpponly
    @param[in,out] err a helics_error object, can be NULL if the errors are to be ignored
    @endforcpponly
    */
HELICS_EXPORT void helicsCoreAddSourceFilterToEndpoint(helics_core core, const char* filter, const char* endpoint, helics_error* err);

/** link a named filter to a destination endpoint
    @param core the core to generate the connection from
    @param filter the name of the filter (cannot be NULL)
    @param endpoint the name of the endpoint to filter the data going to (cannot be NULL)
    @forcpponly
    @param[in,out] err a helics_error object, can be NULL if the errors are to be ignored
    @endforcpponly
    */
HELICS_EXPORT void helicsCoreAddDestinationFilterToEndpoint(helics_core core, const char* filter, const char* endpoint, helics_error* err);

/** load a file containing connection information
    @param core the core to generate the connections from
    @param file a JSON or TOML file containing connection information
    @forcpponly
    @param[in,out] err a helics_error object, can be NULL if the errors are to be ignored
    @endforcpponly
    */
HELICS_EXPORT void helicsCoreMakeConnections(helics_core core, const char* file, helics_error* err);

/** get an identifier for the broker
    @param broker the broker to query
    @return a string containing the identifier for the broker
    */
HELICS_EXPORT const char* helicsBrokerGetIdentifier(helics_broker broker);

/** get an identifier for the core
    @param core the core to query
    @return a string with the identifier of the core
    */
HELICS_EXPORT const char* helicsCoreGetIdentifier(helics_core core);

/** get the network address associated with a broker
    @param broker the broker to query
    @return a string with the network address of the broker
    */
HELICS_EXPORT const char* helicsBrokerGetAddress(helics_broker broker);
/** get the network address associated with a core
    @param core the core to query
    @return a string with the network address of the broker
    */
HELICS_EXPORT const char* helicsCoreGetAddress(helics_core core);

/** set the core to ready for init
    @details this function is used for cores that have filters but no federates so there needs to be
    a direct signal to the core to trigger the federation initialization
    @param core the core object to enable init values for
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsCoreSetReadyToInit(helics_core core, helics_error* err);

/** connect a core to the federate based on current configuration
    @param core the core to connect
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT helics_bool helicsCoreConnect(helics_core core, helics_error* err);

/** disconnect a core from the federation
    @param core the core to query
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsCoreDisconnect(helics_core core, helics_error* err);

/** get an existing federate object from a core by name
    @details the federate must have been created by one of the other functions and at least one of the objects referencing the created
    federate must still be active in the process
    @param fedName the name of the federate to retrieve
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return NULL if no fed is available by that name otherwise a helics_federate with that name*/
HELICS_EXPORT helics_federate helicsGetFederateByName(const char* fedName, helics_error* err);

/** disconnect a broker
    @param broker the broker to disconnect
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsBrokerDisconnect(helics_broker broker, helics_error* err);

/** disconnect and free a broker*/
HELICS_EXPORT void helicsFederateDestroy(helics_federate fed);

/** disconnect and free a broker*/
HELICS_EXPORT void helicsBrokerDestroy(helics_broker broker);

/** disconnect and free a core*/
HELICS_EXPORT void helicsCoreDestroy(helics_core core);

/** release the memory associated with a core*/
HELICS_EXPORT void helicsCoreFree(helics_core core);
/** release the memory associated with a broker*/
HELICS_EXPORT void helicsBrokerFree(helics_broker broker);

/* Creation and destruction of Federates */
/** create a value federate from a federate info object
    @details helics_federate objects can be used in all functions that take a helics_federate or helics_federate object as an argument
    @param fedName the name of the federate to create, can NULL or an empty string to use the default name from fi or an assigned name
    @param fi the federate info object that contains details on the federate
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return an opaque value federate object
    */
HELICS_EXPORT helics_federate helicsCreateValueFederate(const char* fedName, helics_federate_info fi, helics_error* err);

/** create a value federate from a JSON file, JSON string, or TOML file
    @details helics_federate objects can be used in all functions that take a helics_federate or helics_federate object as an argument
    @param configFile  a JSON file or a JSON string or TOML file that contains setup and configuration information
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return an opaque value federate object
    */
HELICS_EXPORT helics_federate helicsCreateValueFederateFromConfig(const char* configFile, helics_error* err);

/** create a message federate from a federate info object
    @details helics_message_federate objects can be used in all functions that take a helics_message_federate or helics_federate object as
    an argument
    @param fedName the name of the federate to create
    @param fi the federate info object that contains details on the federate
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return an opaque message federate object
    */
HELICS_EXPORT helics_federate helicsCreateMessageFederate(const char* fedName, helics_federate_info fi, helics_error* err);

/** create a message federate from a JSON file or JSON string or TOML file
    @details helics_message_federate objects can be used in all functions that take a helics_message_federate or helics_federate object as
    an argument
    @param configFile  a Config(JSON,TOML) file or a JSON string that contains setup and configuration information
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return an opaque message federate object
    */
HELICS_EXPORT helics_federate helicsCreateMessageFederateFromConfig(const char* configFile, helics_error* err);

/** create a combination federate from a federate info object
    @details combination federates are both value federates and message federates, objects can be used in all functions that take a
    helics_federate, helics_message_federate or helics_federate object as an argument
    @param fedName a string with the name of the federate, can be NULL or an empty string to pull the default name from fi
    @param fi the federate info object that contains details on the federate
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return an opaque value federate object nullptr if the object creation failed
    */
HELICS_EXPORT helics_federate helicsCreateCombinationFederate(const char* fedName, helics_federate_info fi, helics_error* err);

/** create a combination federate from a JSON file or JSON string
    @details combination federates are both value federates and message federates, objects can be used in all functions that take a
    helics_federate, helics_message_federate or helics_federate object as an argument
    @param configFile  a JSON file or a JSON string or TOML file that contains setup and configuration information
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return an opaque combination federate object
    */
HELICS_EXPORT helics_federate helicsCreateCombinationFederateFromConfig(const char* configFile, helics_error* err);

/** create a new reference to an existing federate
    @details this will create a new helics_federate object that references the existing federate it must be freed as well
    @param fed an existing helics_federate
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return a new reference to the same federate*/
HELICS_EXPORT helics_federate helicsFederateClone(helics_federate fed, helics_error* err);

/** create a federate info object for specifying federate information when constructing a federate
    @return a helics_federate_info object which is a reference to the created object
    */
HELICS_EXPORT helics_federate_info helicsCreateFederateInfo(void);

/** create a federate info object from an existing one and clone the information
    @param fi a federateInfo object to duplicate
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
     @return a helics_federate_info object which is a reference to the created object
    */
HELICS_EXPORT helics_federate_info helicsFederateInfoClone(helics_federate_info fi, helics_error* err);

/**load a federate info from command line arguments
    @param fi a federateInfo object
    @param argc the number of command line arguments
    @param argv an array of strings from the command line
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateInfoLoadFromArgs(helics_federate_info fi, int argc, const char* const* argv, helics_error* err);

/** delete the memory associated with a federate info object*/
HELICS_EXPORT void helicsFederateInfoFree(helics_federate_info fi);

/** check if a federate_object is valid
    @return helics_true if the federate is a valid active federate, helics_false otherwise*/
HELICS_EXPORT helics_bool helicsFederateIsValid(helics_federate fed);

/** set the name of the core to link to for a federate
  @param fi the federate info object to alter
  @param corename the identifier for a core to link to
  @forcpponly
  @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
  @endforcpponly
  */
HELICS_EXPORT void helicsFederateInfoSetCoreName(helics_federate_info fi, const char* corename, helics_error* err);

/** set the initialization string for the core usually in the form of command line arguments
    @param fi the federate info object to alter
    @param coreInit a string containing command line arguments to be passed to the core
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateInfoSetCoreInitString(helics_federate_info fi, const char* coreInit, helics_error* err);

/** set the initialization string that a core will pass to a generated broker usually in the form of command line arguments
    @param fi the federate info object to alter
    @param brokerInit a string with command line arguments for a generated broker
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateInfoSetBrokerInitString(helics_federate_info fi, const char* brokerInit, helics_error* err);

/** set the core type by integer code
    @details valid values available by definitions in api-data.h
    @param fi the federate info object to alter
    @param coretype an numerical code for a core type see /ref helics_core_type
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateInfoSetCoreType(helics_federate_info fi, int coretype, helics_error* err);

/** set the core type from a string
    @param fi the federate info object to alter
    @param coretype a string naming a core type
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateInfoSetCoreTypeFromString(helics_federate_info fi, const char* coretype, helics_error* err);

/** set the name or connection information for a broker
    @details this is only used if the core is automatically created, the broker information will be transferred to the core for connection
    @param fi the federate info object to alter
    @param broker a string which defines the connection information for a broker either a name or an address
    @forcpponly
   @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
   @endforcpponly
    */
HELICS_EXPORT void helicsFederateInfoSetBroker(helics_federate_info fi, const char* broker, helics_error* err);

/** set the key for a broker connection
    @details this is only used if the core is automatically created, the broker information will be transferred to the core for connection
    @param fi the federate info object to alter
    @param brokerkey a string containing a key for the broker to connect
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateInfoSetBrokerKey(helics_federate_info fi, const char* brokerkey, helics_error* err);

/** set the port to use for the broker
    @details this is only used if the core is automatically created, the broker information will be transferred to the core for connection
    this will only be useful for network broker connections
    @param fi the federate info object to alter
    @param brokerPort the integer port number to use for connection with a broker
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateInfoSetBrokerPort(helics_federate_info fi, int brokerPort, helics_error* err);

/** set the local port to use
    @details this is only used if the core is automatically created, the port information will be transferred to the core for connection
    @param fi the federate info object to alter
    @param localPort a string with the port information to use as the local server port can be a number or "auto" or "os_local"
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */

HELICS_EXPORT void helicsFederateInfoSetLocalPort(helics_federate_info fi, const char* localPort, helics_error* err);

/** get a property index for use in /ref helicsFederateInfoSetFlagOption, /ref helicsFederateInfoSetTimeProperty,
    helicsFederateInfoSetIntegerProperty
    @param val a string with the property name
    @return an int with the property code (-1) if not a valid property
    */
HELICS_EXPORT int helicsGetPropertyIndex(const char* val);

/** get an option index for use in /ref helicsPublicationSetOption, /ref helicsInputSetOption, /ref helicsEndpointSetOption, /ref
    helicsFilterSetOption, and the corresponding get functions
    @param val a string with the option name
    @return an int with the option index (-1) if not a valid property
    */
HELICS_EXPORT int helicsGetOptionIndex(const char* val);

/** set a flag in the info structure
    @details valid flags are available /ref helics_federate_flags
    @param fi the federate info object to alter
    @param flag a numerical index for a flag
    @param value the desired value of the flag helics_true or helics_false
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateInfoSetFlagOption(helics_federate_info fi, int flag, helics_bool value, helics_error* err);

/** set the separator character in the info structure
    @details the separator character is the separation character for local publications/endpoints in creating their global name
    for example if the separator character is '/'  then a local endpoint would have a globally reachable name of fedName/localName
    @param fi the federate info object to alter
    @param separator the character to use as a separator
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateInfoSetSeparator(helics_federate_info fi, char separator, helics_error* err);

/** set the output delay for a federate
    @param fi the federate info object to alter
    @param timeProperty an integer representation of the time based property to set see /ref helics_properties
    @param propertyValue the value of the property to set the timeProperty to
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */

HELICS_EXPORT void
    helicsFederateInfoSetTimeProperty(helics_federate_info fi, int timeProperty, helics_time propertyValue, helics_error* err);

/** set an integer property for a federate
    @details some known properties are
    @param fi the federateInfo object to alter
    @param intProperty an int identifying the property
    @param propertyValue the value to set the property to
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateInfoSetIntegerProperty(helics_federate_info fi, int intProperty, int propertyValue, helics_error* err);

/** load interfaces from a file
    @param fed the federate to which to load interfaces
    @param file the name of a file to load the interfaces from either JSON, or TOML
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateRegisterInterfaces(helics_federate fed, const char* file, helics_error* err);

/** generate a global Error from a federate
A global error halts the co-simulation completely
@param fed the federate to create an error in
@param error_code the integer code for the error
@param error_string a string describing the error
*/
HELICS_EXPORT void helicsFederateGlobalError(helics_federate fed, int error_code, const char* error_string);

/** generate a local error in a federate
this will propagate through the co-simulation but not necessarily halt the co-simulation, it has a similar effect to finalize but does
allow some interaction with a core for a brief time. 
@param fed the federate to create an error in
@param error_code the integer code for the error
@param error_string a string describing the error
*/
HELICS_EXPORT void helicsFederateLocalError(helics_federate fed, int error_code, const char* error_string);

/** finalize the federate this function halts all communication in the federate and disconnects it from the core
     */
HELICS_EXPORT void helicsFederateFinalize(helics_federate fed, helics_error* err);
/** finalize the federate in an async call*/
HELICS_EXPORT void helicsFederateFinalizeAsync(helics_federate fed, helics_error* err);
/** complete the asynchronous finalize call*/
HELICS_EXPORT void helicsFederateFinalizeComplete(helics_federate fed, helics_error* err);

/** release the memory associated withe a federate*/
HELICS_EXPORT void helicsFederateFree(helics_federate fed);

/** call when done using the helics library,  this function will ensure the threads are closed properly if possible
    this should be the last call before exiting,  */
HELICS_EXPORT void helicsCloseLibrary(void);

/* initialization, execution, and time requests */
/** enter the initialization state of a federate
    @details the initialization state allows initial values to be set and received if the iteration is requested on entry to
    the execution state
    This is a blocking call and will block until the core allows it to proceed
    @param fed the federate to operate on
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateEnterInitializingMode(helics_federate fed, helics_error* err);

/** non blocking alternative to \ref helicsFederateEnterInitializingMode
    the function helicsFederateEnterInitializationModeFinalize must be called to finish the operation
    @param fed the federate to operate on
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateEnterInitializingModeAsync(helics_federate fed, helics_error* err);

/** check if the current Asynchronous operation has completed
    @param fed the federate to operate on
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return helics_false if not completed, helics_true if completed*/
HELICS_EXPORT helics_bool helicsFederateIsAsyncOperationCompleted(helics_federate fed, helics_error* err);

/** finalize the entry to initialize mode that was initiated with /ref heliceEnterInitializingModeAsync
    @param fed the federate desiring to complete the initialization step
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateEnterInitializingModeComplete(helics_federate fed, helics_error* err);

/** request that the federate enter the Execution mode
    @details this call is blocking until granted entry by the core object for an asynchronous alternative call
    /ref helicsFederateEnterExecutingModeAsync  on return from this call the federate will be at time 0
    @param fed a federate to change modes
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateEnterExecutingMode(helics_federate fed, helics_error* err);

/** request that the federate enter the Execution mode
    @details this call is non-blocking and will return immediately call /ref helicsFederateEnterExecutingModeComplete to finish the call
    sequence /ref helicsFederateEnterExecutingModeComplete
    @param fed the federate object to complete the call
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateEnterExecutingModeAsync(helics_federate fed, helics_error* err);

/** complete the call to /ref EnterExecutingModeAsync
    @param fed the federate object to complete the call
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateEnterExecutingModeComplete(helics_federate fed, helics_error* err);

/** request an iterative time
    @details this call allows for finer grain control of the iterative process then /ref helicsFederateRequestTime it takes a time and
    iteration request and return a time and iteration status
    @param fed the federate to make the request of
    @param iterate the requested iteration mode
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return an iteration structure with field containing the time and iteration status
    */
HELICS_EXPORT helics_iteration_result
    helicsFederateEnterExecutingModeIterative(helics_federate fed, helics_iteration_request iterate, helics_error* err);

/** request an iterative entry to the execution mode
    @details this call allows for finer grain control of the iterative process then /ref helicsFederateRequestTime it takes a time and and
    iteration request and return a time and iteration status
    @param fed the federate to make the request of
    @param iterate the requested iteration mode
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateEnterExecutingModeIterativeAsync(helics_federate fed, helics_iteration_request iterate, helics_error* err);

/** complete the asynchronous iterative call into ExecutionModel
    @param fed the federate to make the request of
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return an iteration object containing the iteration time and iteration_status
    */
HELICS_EXPORT helics_iteration_result helicsFederateEnterExecutingModeIterativeComplete(helics_federate fed, helics_error* err);

/** get the current state of a federate
    @param fed the fed to query
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return state the resulting state if void return helics_ok*/
HELICS_EXPORT helics_federate_state helicsFederateGetState(helics_federate fed, helics_error* err);

/** get the core object associated with a federate
    @param fed a federate object
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return a core object, nullptr if invalid
    */
HELICS_EXPORT helics_core helicsFederateGetCoreObject(helics_federate fed, helics_error* err);

/** request the next time for federate execution
    @param fed the federate to make the request of
    @param requestTime the next requested time
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return the time granted to the federate, will return helics_time_maxtime if the simulation has terminated
    invalid*/
HELICS_EXPORT helics_time helicsFederateRequestTime(helics_federate fed, helics_time requestTime, helics_error* err);

/** request the next time for federate execution
    @param fed the federate to make the request of
    @param timeDelta the requested amount of time to advance
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return the time granted to the federate, will return helics_time_maxtime if the simulation has terminated
    invalid*/
HELICS_EXPORT helics_time helicsFederateRequestTimeAdvance(helics_federate fed, helics_time timeDelta, helics_error* err);

/** request the next time step for federate execution
    @details feds should have setup the period or minDelta for this to work well but it will request the next time step which is the current
    time plus the minimum time step
    @param fed the federate to make the request of
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return the time granted to the federate, will return helics_time_maxtime if the simulation has terminated
    invalid*/
HELICS_EXPORT helics_time helicsFederateRequestNextStep(helics_federate fed, helics_error* err);

/** request an iterative time
    @details this call allows for finer grain control of the iterative process then /ref helicsFederateRequestTime it takes a time and and
    iteration request and return a time and iteration status
    @param fed the federate to make the request of
    @param requestTime the next desired time
    @param iterate the requested iteration mode
    @param[out] outIteration  the iteration specification of the result
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return the granted time, will return helics_time_maxtime if the simulation has terminated along with the appropriate iteration result
    value
    */
HELICS_EXPORT helics_time helicsFederateRequestTimeIterative(
    helics_federate fed,
    helics_time requestTime,
    helics_iteration_request iterate,
    helics_iteration_result* outIteration,
    helics_error* err);

/** request the next time for federate execution in an asynchronous call
    @details call /ref helicsFederateRequestTimeComplete to finish the call
    @param fed the federate to make the request of
    @param requestTime the next requested time
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateRequestTimeAsync(helics_federate fed, helics_time requestTime, helics_error* err);

/** complete an asynchronous requestTime call
    @param fed the federate to make the request of
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return the time granted to the federate, will return helics_time_maxtime if the simulation has terminated*/
HELICS_EXPORT helics_time helicsFederateRequestTimeComplete(helics_federate fed, helics_error* err);

/** request an iterative time through an asynchronous call
    @details this call allows for finer grain control of the iterative process then /ref helicsFederateRequestTime it takes a time an
    iteration request and returns a time and iteration status call /ref helicsFederateRequestTimeIterativeComplete to finish the process
    @param fed the federate to make the request of
    @param requestTime the next desired time
    @param iterate the requested iteration mode
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateRequestTimeIterativeAsync(
    helics_federate fed,
    helics_time requestTime,
    helics_iteration_request iterate,
    helics_error* err);

/** complete an iterative time request asynchronous call
    @param fed the federate to make the request of
    @param[out] outIterate  the iteration specification of the result
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return the granted time, will return helics_time_maxtime if the simulation has terminated
    */
HELICS_EXPORT helics_time
    helicsFederateRequestTimeIterativeComplete(helics_federate fed, helics_iteration_result* outIterate, helics_error* err);

/** get the name of the federate
    @param fed the federate object to query
    @return a pointer to a string with the name
    */
HELICS_EXPORT const char* helicsFederateGetName(helics_federate fed);

/** set a time based property for a federate
    @param fed the federate object set the property for
    @param timeProperty a integer code for a time property
    @param time the requested value of the property
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateSetTimeProperty(helics_federate fed, int timeProperty, helics_time time, helics_error* err);

/** set a flag for the federate
    @param fed the federate to alter a flag for
    @param flag the flag to change
    @param flagValue the new value of the flag 0 for false !=0 for true
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateSetFlagOption(helics_federate fed, int flag, helics_bool flagValue, helics_error* err);

/** set the separator character in a federate
    @details the separator character is the separation character for local publications/endpoints in creating their global name
    for example if the separator character is '/'  then a local endpoint would have a globally reachable name of fedName/localName
    @param fed the federate info object to alter
    @param separator the character to use as a separator
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateSetSeparator(helics_federate fed, char separator, helics_error* err);

/**  set an integer based property of a federate
    @param fed the federate to change the property for
    @param intProperty the property to set
    @param propertyVal the value of the property
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateSetIntegerProperty(helics_federate fed, int intProperty, int propertyVal, helics_error* err);

/** get the current value of a time based property in a federate
    @param fed the federate query
    @param timeProperty the property to query
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT helics_time helicsFederateGetTimeProperty(helics_federate fed, int timeProperty, helics_error* err);

/** get a flag value for a federate
    @param fed the federate to get the flag for
    @param flag the flag to query
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    @return the value of the flag
    */
HELICS_EXPORT helics_bool helicsFederateGetFlagOption(helics_federate fed, int flag, helics_error* err);

/**  Get the current value of an integer property (such as a logging level)
    @param fed the federate to get the flag for
    @param intProperty a code for the property to set /ref helics_handle_options
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    @return the value of the property
    */
HELICS_EXPORT int helicsFederateGetIntegerProperty(helics_federate fed, int intProperty, helics_error* err);

/** get the current time of the federate
    @param fed the federate object to query
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    @return the current time of the federate
    */
HELICS_EXPORT helics_time helicsFederateGetCurrentTime(helics_federate fed, helics_error* err);

/** set a federation global value through a federate
    @details this overwrites any previous value for this name
    @param fed the federate to set the global through
    @param valueName the name of the global to set
    @param value the value of the global
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateSetGlobal(helics_federate fed, const char* valueName, const char* value, helics_error* err);

/** set the logging file for a federate(actually on the core associated with a federate)
    @param fed the federate to set the log file for
    @param logFile the name of the log file
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateSetLogFile(helics_federate fed, const char* logFile, helics_error* err);

/** log an error message through a federate
    @param fed the federate to set the global through
    @param logmessage the message to put in the log
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateLogErrorMessage(helics_federate fed, const char* logmessage, helics_error* err);

/** log a warning message through a federate
    @param fed the federate to set the global through
    @param logmessage the message to put in the log
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateLogWarningMessage(helics_federate fed, const char* logmessage, helics_error* err);

/** log a message through a federate
    @param fed the federate to set the global through
    @param logmessage the message to put in the log
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateLogInfoMessage(helics_federate fed, const char* logmessage, helics_error* err);

/** log a message through a federate
    @param fed the federate to set the global through
    @param logmessage the message to put in the log
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateLogDebugMessage(helics_federate fed, const char* logmessage, helics_error* err);

/** log a message through a federate
    @param fed the federate to set the global through
    @param loglevel the level of the message to log see /ref helics_log_levels
    @param logmessage the message to put in the log
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    */
HELICS_EXPORT void helicsFederateLogLevelMessage(helics_federate fed, int loglevel, const char* logmessage, helics_error* err);

/** set a global value in a core
    @details this overwrites any previous value for this name
    @param core the core to set the global through
    @param valueName the name of the global to set
    @param value the value of the global
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsCoreSetGlobal(helics_core core, const char* valueName, const char* value, helics_error* err);

/** set a federation global value
    @details this overwrites any previous value for this name
    @param broker the broker to set the global through
    @param valueName the name of the global to set
    @param value the value of the global
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsBrokerSetGlobal(helics_broker broker, const char* valueName, const char* value, helics_error* err);

/** set a the log file on a core
    @param core the core to set the global through
    @param logFileName the name of the file to log to
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsCoreSetLogFile(helics_core core, const char* logFileName, helics_error* err);

/** set a the log file on a broker
    @param broker the broker to set the global through
    @param logFileName the name of the file to log to
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsBrokerSetLogFile(helics_broker broker, const char* logFileName, helics_error* err);

/** create a query object
    @details a query object consists of a target and query string
    @param target the name of the target to query
    @param query the query to make of the target
    */
HELICS_EXPORT helics_query helicsCreateQuery(const char* target, const char* query);

/** Execute a query
    @details the call will block until the query finishes which may require communication or other delays
    @param query the query object to use in the query
    @param fed a federate to send the query through
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return a pointer to a string.  the string will remain valid until the query is freed or executed again
    the return will be nullptr if fed or query is an invalid object, the return string will be "#invalid" if the query itself was invalid
    */
HELICS_EXPORT const char* helicsQueryExecute(helics_query query, helics_federate fed, helics_error* err);

/** Execute a query directly on a core
    @details the call will block until the query finishes which may require communication or other delays
    @param query the query object to use in the query
    @param core the core to send the query to
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return a pointer to a string.  the string will remain valid until the query is freed or executed again
    the return will be nullptr if fed or query is an invalid object, the return string will be "#invalid" if the query itself was invalid
    */
HELICS_EXPORT const char* helicsQueryCoreExecute(helics_query query, helics_core core, helics_error* err);

/** Execute a query directly on a broker
    @details the call will block until the query finishes which may require communication or other delays
    @param query the query object to use in the query
    @param broker the broker to send the query to
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return a pointer to a string.  the string will remain valid until the query is freed or executed again
    the return will be nullptr if fed or query is an invalid object, the return string will be "#invalid" if the query itself was invalid
    */
HELICS_EXPORT const char* helicsQueryBrokerExecute(helics_query query, helics_broker broker, helics_error* err);

/** Execute a query in a non-blocking call
    @param query the query object to use in the query
    @param fed a federate to send the query through
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    */
HELICS_EXPORT void helicsQueryExecuteAsync(helics_query query, helics_federate fed, helics_error* err);

/** complete the return from a query called with /ref helicsExecuteQueryAsync
    @details the function will block until the query completes /ref isQueryComplete can be called to determine if a query has completed or
    not
    @param query the query object to complete execution of
    @forcpponly
    @param[in,out] err an error object that will contain an error code and string if any error occurred during the execution of the function
    @endforcpponly
    @return a pointer to a string.  the string will remain valid until the query is freed or executed again
    the return will be nullptr if query is an invalid object
    */
HELICS_EXPORT const char* helicsQueryExecuteComplete(helics_query query, helics_error* err);

/** check if an asynchronously executed query has completed
    @details this function should usually be called after a QueryExecuteAsync function has been called
    @param query the query object to check if completed
    @return will return helics_true if an asynchronous query has complete or a regular query call was made with a result
    and false if an asynchronous query has not completed or is invalid
    */
HELICS_EXPORT helics_bool helicsQueryIsCompleted(helics_query query);

/** free the memory associated with a query object*/
HELICS_EXPORT void helicsQueryFree(helics_query query);

/** function to do some housekeeping work
    @details this runs some cleanup routines and tries to close out any residual thread that haven't been shutdown
    yet*/
HELICS_EXPORT void helicsCleanupLibrary(void);

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif
