/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_APISHARED_FUNCTIONS_H_
#define HELICS_APISHARED_FUNCTIONS_H_

#include "api-data.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

/*
  Export HELICS API functions on Windows and under GCC.
  If custom linking is desired then the HELICS_Export must be
  defined before including this file. For instance,
  it may be set to __declspec(dllimport).
*/
#if !defined(HELICS_Export)
#if defined _WIN32 || defined __CYGWIN__
/* Note: both gcc & MSVC on Windows support this syntax. */
#define HELICS_Export __declspec(dllexport)
#else
#define HELICS_Export __attribute__ ((visibility ("default")))
#endif
#endif

/***************************************************
Common Functions
****************************************************/

/* Inquire version numbers of header files */

HELICS_Export const char *helicsGetVersion ();

/** create a core object
@param type the type of the core to create
@param name the name of the core , may be a nullptr or empty string to have a name automatically assigned
@param initString an initialization string to send to the core-the format is similar to command line arguments
typical options include a broker address  --broker="XSSAF" or the number of federates or the address
@return a helics_core object
*/
HELICS_Export helics_core helicsCreateCore (const char *type, const char *name, const char *initString);
/** create a core object by passing command line arguments
@param type the type of the core to create
@param name the name of the core , may be a nullptr or empty string to have a name automatically assigned
@param argc the number of arguments
@param argv the string values from a command line
@return a helics_core object
*/
HELICS_Export helics_core helicsCreateCoreFromArgs (const char *type, const char *name, int argc, const char *const *argv);

/** create a broker object
@param type the type of the broker to create
@param name the name of the broker , may be a nullptr or empty string to have a name automatically assigned
@param initString an initialization string to send to the core-the format is similar to command line arguments
typical options include a broker address  --broker="XSSAF" if this is a subbroker or the number of federates or the address
@return a helics_core object
*/
HELICS_Export helics_broker helicsCreateBroker (const char *type, const char *name, const char *initString);
/** create a core object by passing command line arguments
@param type the type of the core to create
@param name the name of the core , may be a nullptr or empty string to have a name automatically assigned
@param argc the number of arguments
@param argv the string values from a command line
@return a helics_core object
*/
HELICS_Export helics_broker helicsCreateBrokerFromArgs (const char *type, const char *name, int argc, const char *const *argv);

/** check if a broker is connected
a connected broker implies is attached to cores or cores could reach out to communicate
return 0 if not connected , something else if it is connected*/
HELICS_Export int helicsBrokerIsConnected (helics_broker broker);

/** check if a core is connected
a connected core implies is attached to federate or federates could be attached to it
return 0 if not connected , something else if it is connected*/
HELICS_Export int helicsCoreIsConnected (helics_core core);

/** get an identifier for the broker
@param broker the broker to query
@param[out] identifier storage space to place the identifier string
@param maxlen the maximum space available in identifier
@return a helicsStatus enumeration indicating any error condition
*/
HELICS_Export helicsStatus helicsBrokerGetIdentifier(helics_broker broker, char *identifier, int maxlen);

/** get an identifier for the core
@param core the core to query
@param[out] identifier storage space to place the identifier string
@param maxlen the maximum space available in identifier
@return a helicsStatus enumeration indicating any error condition
*/
HELICS_Export helicsStatus helicsCoreGetIdentifier(helics_core core, char *identifier, int maxlen);

/** get the network address associated with a broker
@param broker the broker to query
@param[out] identifier storage space to place the identifier string
@param maxlen the maximum space available in identifier
@return a helicsStatus enumeration indicating any error condition
*/
HELICS_Export helicsStatus helicsBrokerGetAddress(helics_broker broker, char *address, int maxlen);

/** get an identifier for the core
@param core the core to query
@param[out] identifier storage space to place the identifier string
@param maxlen the maximum space available in identifier
@return a helicsStatus enumeration indicating any error condition
*/
HELICS_Export helicsStatus helicsCoreDisconnect(helics_core core);

/** get the network address associated with a broker
@param broker the broker to query
@param[out] identifier storage space to place the identifier string
@param maxlen the maximum space available in identifier
@return a helicsStatus enumeration indicating any error condition
*/
HELICS_Export helicsStatus helicsBrokerDisconnect(helics_broker broker);

/** release the memory associated with a core*/
HELICS_Export void helicsFreeCore (helics_core core);
/** release the memory associated with a broker*/
HELICS_Export void helicsFreeBroker (helics_broker broker);

/* Creation and destruction of Federates */
/** create a value federate from a federate info object
@details helics_value_federate objects can be used in all functions that take a helics_value_federate or helics_federate object as an argument
@param fi the federate info object that contains details on the federate
@return an opaque value federate object
*/
HELICS_Export helics_value_federate helicsCreateValueFederate (const helics_federate_info_t fi);

/** create a value federate from a json file or json string
@details helics_value_federate objects can be used in all functions that take a helics_value_federate or helics_federate object as an argument
@param json  a json file or a json string that contains setup and configuration information
@return an opaque value federate object
*/
HELICS_Export helics_value_federate helicsCreateValueFederateFromJson (const char *json);



/** create a message federate from a federate info object
@details helics_message_federate objects can be used in all functions that take a helics_message_federate or helics_federate object as an argument
@param fi the federate info object that contains details on the federate
@return an opaque message federate object
*/
HELICS_Export helics_message_federate helicsCreateMessageFederate(const helics_federate_info_t fi);

/** create a message federate from a json file or json string
@details helics_message_federate objects can be used in all functions that take a helics_message_federate or helics_federate object as an argument
@param json  a json file or a json string that contains setup and configuration information
@return an opaque message federate object
*/
HELICS_Export helics_message_federate helicsCreateMessageFederateFromJson(const char *json);

/** create a combination federate from a federate info object
@details combination federates are both value federates and message federates, objects can be used in all functions that take a helics_value_federate, helics_message_federate or helics_federate object as an argument
@param fi the federate info object that contains details on the federate
@return an opaque value federate object nullptr if the object creation failed
*/
HELICS_Export helics_federate helicsCreateCombinationFederate(const helics_federate_info_t fi);

/** create a combination federate from a json file or json string
@details combination federates are both value federates and message federates, objects can be used in all functions that take a helics_value_federate, helics_message_federate or helics_federate object as an argument
@param json  a json file or a json string that contains setup and configuration information
@return an opaque combination federate object
*/
HELICS_Export helics_federate helicsCreateCombinationFederateFromJson(const char *json);

/** create a federate info object for specifying federate information when constructing a federate
@return a helics_federate_info_t object which is a reference to the created object
*/
HELICS_Export helics_federate_info_t helicsFederateInfoCreate ();
/** delete the memory associated with a federate info object*/
HELICS_Export void helicsFederateInfoFree (helics_federate_info_t fi);

/** set the federate name in the Federate Info structure
@param fi the federate info object to alter
@param name the new identifier for the federate
@return a helicsStatus enumeration helicsOK on success 
*/
HELICS_Export helicsStatus helicsFederateInfoSetFederateName (helics_federate_info_t fi, const char *name);

/** set the name of the core to link to for a federate
@param fi the federate info object to alter
@param corename the identifier for a core to link to
@return a helicsStatus enumeration helicsOK on success helicsInvalidReference if fi is not a valid reference
*/
HELICS_Export helicsStatus helicsFederateInfoSetCoreName (helics_federate_info_t fi, const char *corename);
/** set the initialization string for the core usually in the form of command line arguments
@param fi the federate info object to alter
@param coreInit a string with the core initialization strings
@return a helicsStatus enumeration helicsOK on success helicsInvalidReference if fi is not a valid reference
*/

HELICS_Export helicsStatus helicsFederateInfoSetCoreInitString (helics_federate_info_t fi, const char *coreInit);
/** set the core type from a string
@param fi the federate info object to alter
@param coretype a string naming a core type
@return a helicsStatus enumeration helicsOK on success helicsInvalidReference if fi is not a valid reference helicsDiscard if the string is not recognized
*/
HELICS_Export helicsStatus helicsFederateInfoSetCoreTypeFromString (helics_federate_info_t fi, const char *coretype);
HELICS_Export helicsStatus helicsFederateInfoSetCoreType (helics_federate_info_t fi, int coretype);
HELICS_Export helicsStatus helicsFederateInfoSetFlag (helics_federate_info_t fi, int flag, int value);
HELICS_Export helicsStatus helicsFederateInfoSetLookahead (helics_federate_info_t fi, helics_time_t lookahead);
HELICS_Export helicsStatus helicsFederateInfoSetTimeDelta (helics_federate_info_t fi, helics_time_t timeDelta);
HELICS_Export helicsStatus helicsFederateInfoSetImpactWindow (helics_federate_info_t fi, helics_time_t impactWindow);
HELICS_Export helicsStatus helicsFederateInfoSetTimeOffset (helics_federate_info_t fi, helics_time_t timeOffset);
HELICS_Export helicsStatus helicsFederateInfoSetPeriod (helics_federate_info_t fi, helics_time_t period);
HELICS_Export helicsStatus helicsFederateInfoSetMaxIterations (helics_federate_info_t fi, int max_iterations);
HELICS_Export helicsStatus helicsFederateInfoSetLoggingLevel (helics_federate_info_t fi, int logLevel);
/** finalize the federate this halts all communication in the federate and disconnects it from the core
 */
HELICS_Export helicsStatus helicsFederateFinalize (helics_federate fed);
/** release the memory associated withe a federate*/
HELICS_Export void helicsFreeFederate (helics_federate fed);

/** call when done using the helics library,  this function will ensure the threads are closed properly if possible
this should be the last call before exiting,  */
HELICS_Export void helicsCloseLibrary ();
/* initialization, execution, and time requests */
/** enter the initialization state of a federate
@details the initialization state allows initial values to be set and received if the iteration is requested on entry to
the execution state
This is a blocking call and will block until the core allows it to proceed
*/
HELICS_Export helicsStatus helicsEnterInitializationMode (helics_federate fed);
/** non blocking alternative to @helicsEnterInitializationMode
the function helicsEnterInitializationModeFinalize must be called to finish the operation
*/
HELICS_Export helicsStatus helicsEnterInitializationModeAsync (helics_federate fed);
/** check if the current Asynchronous operation has completed*/
HELICS_Export int helicsisAsyncOperationCompleted (helics_federate fed);

/** finalize the entry to initialize mode that was initiated with @heliceEnterInitializationModeAsync*/
HELICS_Export helicsStatus helicsEnterInitializationModeComplete (helics_federate fed);


/** request that the federate enter the Execution mode
@details this call is blocking until granted entry by the core object for an asynchronous alternative call
@helicsEnterExecutionModeAsync
@param fed a federate to change modes
@return a helicsStatus enumeration helicsError if something went wrong helicsInvalidReference if fed is invalid
*/
HELICS_Export helicsStatus helicsEnterExecutionMode (helics_federate fed);
HELICS_Export helicsStatus helicsEnterExecutionModeIterative (helics_federate fed, iteration_request iterate, iteration_status *outIterate);

HELICS_Export helicsStatus helicsEnterExecutionModeAsync (helics_federate fed);
HELICS_Export helicsStatus helicsEnterExecutionModeIterativeAsync (helics_federate fed, iteration_request iterate);

HELICS_Export helicsStatus helicsEnterExecutionModeComplete (helics_federate fed);
HELICS_Export helicsStatus helicsEnterExecutionModeIterativeComplete (helics_federate fed, iteration_status *outIterate);

/** get the current state of a federate
@param fed the fed to query
@param[out] state the resulting state if helicsStatus return helicsOK*/
HELICS_Export helicsStatus helicsFederateGetState(helics_federate fed, federate_state *state);

/** get the core object associated with a federate
@param fed a federate object
@return a core object, nullptr if invalid
*/
HELICS_Export helics_core helicsGetCoreObject(helics_federate fed);

HELICS_Export helics_time_t helicsRequestTime (helics_federate fed, helics_time_t requestTime);
HELICS_Export helics_iterative_time helicsRequestTimeIterative (helics_federate fed, helics_time_t requestTime, iteration_request iterate);

HELICS_Export helicsStatus helicsRequestTimeAsync (helics_federate fed, helics_time_t requestTime);
HELICS_Export helicsStatus helicsRequestTimeIterativeAsync (helics_federate fed, helics_time_t requestTime, iteration_request iterate);
HELICS_Export helics_time_t helicsRequestTimeComplete (helics_federate fed);
HELICS_Export helics_iterative_time helicsRequestTimeIterativeComplete (helics_federate fed);

/** get the name of the federate 
@param fed the federate object to query
@param[out] str memory buffer to store the result
@param maxlen the maximum size of the buffer
@return helicsStatus object indicating success or error
*/
HELICS_Export helicsStatus helicsFederateGetName(helics_federate fed, char *str, int maxlen);


/** set the minimum time delta for the federate
@param[in] tdelta the minimum time delta to return from a time request function
*/
HELICS_Export helicsStatus helicsFederateSetTimeDelta(helics_federate fed, helics_time_t time);
/** set the look ahead time
@details the look ahead is the propagation time for messages/event to propagate from the Federate
the federate
@param[in] lookAhead the look ahead time
*/
HELICS_Export helicsStatus helicsFederateSetLookAhead(helics_federate fed, helics_time_t lookAhead);

/** set the impact Window time
@details the impact window is the time window around the time request in which other federates cannot affect
the federate
@param[in] lookAhead the look ahead time
*/
HELICS_Export helicsStatus helicsFederateSetImpactWindow(helics_federate fed, helics_time_t window);
/** set the period and offset of the federate
@details the federate will on grant time on N*period+offset interval
@param[in] period the length of time between each subsequent grants
@param[in] offset the shift of the period from 0  offset must be < period
*/
HELICS_Export helicsStatus helicsFederateSetPeriod(helics_federate fed, helics_time_t period, helics_time_t offset);
/** set a flag for the federate
@param fed the federate to alter a flag for
@param flag the flag to change
@param flagValue the new value of the flag 0 for false !=0 for true
*/
HELICS_Export helicsStatus helicsFederateSetFlag(helics_federate fed, int flag, int flagValue);
/**  set the logging level for the federate
@ details debug and trace only do anything if they were enabled in the compilation
@param loggingLevel (-1: none, 0: error_only, 1: warnings, 2: normal, 3: debug, 4: trace)
*/
HELICS_Export helicsStatus helicsFederateSetLoggingLevel(helics_federate fed, int loggingLevel);

/** get the current time of the federate
@param fed the federate object to query
@param[out] time storage location for the time variable
@return helicsStatus object indicating success or error
*/
HELICS_Export helicsStatus helicsFederateGetCurrentTime(helics_federate fed, helics_time_t *time);
/** create a query object
@details a query object consists of a target and query string
*/
HELICS_Export helics_query helicsCreateQuery (const char *target, const char *query);
/** Execute a query
@param fed a federate to send the query through
@param query the query object to use in the query
*/
HELICS_Export const char *helicsExecuteQuery (helics_federate fed, helics_query query);

/** free the memory associated with a query object*/
HELICS_Export void helicsFreeQuery (helics_query);

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif
