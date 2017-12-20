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
HELICS_Export helics_value_federate helicsCreateValueFederate (const helics_federate_info_t fi);
HELICS_Export helics_value_federate helicsCreateValueFederateFromFile (const char *filename);

HELICS_Export helics_federate helicsCreateCombinationFederate (const helics_federate_info_t fi);
HELICS_Export helics_federate helicsCreateCombinationFederateFromFile (const char *filename);

/** create a federate info object for specifying federate information when constructing a federate
@return a helics_federate_info_t object which is a reference to the created object
*/
HELICS_Export helics_federate_info_t helicsFederateInfoCreate ();
/** delete the memory associated with a federate info object*/
HELICS_Export void helicsFederateInfoFree (helics_federate_info_t fi);

HELICS_Export helicsStatus helicsFederateInfoSetFederateName (helics_federate_info_t fi, const char *name);
HELICS_Export helicsStatus helicsFederateInfoSetCoreName (helics_federate_info_t fi, const char *corename);
HELICS_Export helicsStatus helicsFederateInfoSetCoreInitString (helics_federate_info_t fi, const char *coreInit);
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
HELICS_Export helicsStatus helicsFinalize (helics_federate fed);
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
HELICS_Export int helicsAsyncOperationCompleted (helics_federate fed);

/** finalize the entry to initialize mode that was initiated with @heliceEnterInitializationModeAsync*/
HELICS_Export helicsStatus helicsEnterInitializationModeFinalize (helics_federate fed);

/** request that the federate enter the initialization mode
@details this call is blocking until granted entry by the core object for an asynchronous alternative call
@helicsEnterExecutionModeAsync*/
HELICS_Export helicsStatus helicsEnterExecutionMode (helics_federate fed);
HELICS_Export helicsStatus helicsEnterExecutionModeIterative (helics_federate fed, iteration_request iterate, iteration_status *outIterate);

HELICS_Export helicsStatus helicsEnterExecutionModeAsync (helics_federate fed);
HELICS_Export helicsStatus helicsEnterExecutionModeIterativeAsync (helics_federate fed, iteration_request iterate);

HELICS_Export helicsStatus helicsEnterExecutionModeFinalize (helics_federate fed);
HELICS_Export helicsStatus helicsEnterExecutionModeIterativeFinalize (helics_federate fed, iteration_status *outIterate);
/** get the core object associated with a federate
@param fed a federate object
@return a core object, nullptr if invalid
*/
HELICS_Export helics_core helicsGetCoreObject(helics_federate fed);

HELICS_Export helics_time_t helicsRequestTime (helics_federate fed, helics_time_t requestTime);
HELICS_Export helics_iterative_time helicsRequestTimeIterative (helics_federate fed, helics_time_t requestTime, iteration_request iterate);

HELICS_Export helicsStatus helicsRequestTimeAsync (helics_federate fed, helics_time_t requestTime);
HELICS_Export helicsStatus helicsRequestTimeIterativeAsync (helics_federate fed, helics_time_t requestTime, iteration_request iterate);
HELICS_Export helics_time_t helicsRequestTimeFinalize (helics_federate fed);
HELICS_Export helics_iterative_time helicsRequestTimeIterativeFinalize (helics_federate fed);

/** get the name of the federate 
@param fed the federate object to query
@param[out] str memory buffer to store the result
@param maxlen the maximum size of the buffer
@return helicsStatus object indicating success or error
*/
HELICS_Export helicsStatus helicsFederateGetName(helics_federate fed, char *str, int maxlen);

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
