/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#ifndef HELICS_APISHARED_FUNCTIONS_H_
#define HELICS_APISHARED_FUNCTIONS_H_

#include "api-data.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include "helics_export.h"

/** @file
@brief common functions for the HELICS C api
*/
/***************************************************
Common Functions
****************************************************/

/** get a version string for HELICS */
HELICS_EXPORT const char *helicsGetVersion ();

/**
 * Returns true if core/broker type specified is available in current compilation.
 @details possible options include "test","zmq","udp","ipc","interprocess","tcp","default", "mpi"
 */
HELICS_EXPORT helics_bool_t helicsIsCoreTypeAvailable (const char *type);

/** create a core object
@param type the type of the core to create
@param name the name of the core , may be a nullptr or empty string to have a name automatically assigned
@param initString an initialization string to send to the core-the format is similar to command line arguments
typical options include a broker address  --broker="XSSAF" or the number of federates or the address
@return a helics_core object
*/
HELICS_EXPORT helics_core helicsCreateCore (const char *type, const char *name, const char *initString);
/** create a core object by passing command line arguments
@param type the type of the core to create
@param name the name of the core , may be a nullptr or empty string to have a name automatically assigned
@param argc the number of arguments
@param argv the string values from a command line
@return a helics_core object
*/
HELICS_EXPORT helics_core helicsCreateCoreFromArgs (const char *type, const char *name, int argc, const char *const *argv);

/** create a new reference to an existing broker
@details this will create a new broker object that references the existing broker it must be freed as well
@param broker an existing helics_broker
@return a new reference to the same broker*/
HELICS_EXPORT helics_core helicsCoreClone (helics_core core);

/** create a broker object
@param type the type of the broker to create
@param name the name of the broker , may be a nullptr or empty string to have a name automatically assigned
@param initString an initialization string to send to the core-the format is similar to command line arguments
typical options include a broker address  --broker="XSSAF" if this is a subbroker or the number of federates or the address
@return a helics_core object
*/
HELICS_EXPORT helics_broker helicsCreateBroker (const char *type, const char *name, const char *initString);
/** create a core object by passing command line arguments
@param type the type of the core to create
@param name the name of the core , may be a nullptr or empty string to have a name automatically assigned
@param argc the number of arguments
@param argv the string values from a command line
@return a helics_core object
*/
HELICS_EXPORT helics_broker helicsCreateBrokerFromArgs (const char *type, const char *name, int argc, const char *const *argv);

/** create a new reference to an existing broker
@details this will create a new broker object that references the existing broker it must be freed as well
@param broker an existing helics_broker
@return a new reference to the same broker*/
HELICS_EXPORT helics_broker helicsBrokerClone (helics_broker broker);
/** check if a broker is connected
a connected broker implies is attached to cores or cores could reach out to communicate
return 0 if not connected , something else if it is connected*/
HELICS_EXPORT int helicsBrokerIsConnected (helics_broker broker);

/** check if a core is connected
a connected core implies is attached to federate or federates could be attached to it
return 0 if not connected , something else if it is connected*/
HELICS_EXPORT int helicsCoreIsConnected (helics_core core);

/** get an identifier for the broker
@param broker the broker to query
@param[out] identifier storage space to place the identifier string
@param maxlen the maximum space available in identifier
@return a helics_status enumeration indicating any error condition
*/
HELICS_EXPORT helics_status helicsBrokerGetIdentifier (helics_broker broker, char *identifier, int maxlen);

/** get an identifier for the core
@param core the core to query
@param[out] identifier storage space to place the identifier string
@param maxlen the maximum space available in identifier
@return a helics_status enumeration indicating any error condition
*/
HELICS_EXPORT helics_status helicsCoreGetIdentifier (helics_core core, char *identifier, int maxlen);

/** get the network address associated with a broker
@param broker the broker to query
@param[out] identifier storage space to place the identifier string
@param maxlen the maximum space available in identifier
@return a helics_status enumeration indicating any error condition
*/
HELICS_EXPORT helics_status helicsBrokerGetAddress (helics_broker broker, char *address, int maxlen);

/** set the core to ready to init
@details this function is used for cores that have filters but no federates so there needs to be
a direct signal to the core to trigger the federation initialization
*/
HELICS_EXPORT helics_status helicsCoreSetReadyToInit (helics_core core);

/** get an identifier for the core
@param core the core to query
@param[out] identifier storage space to place the identifier string
@param maxlen the maximum space available in identifier
@return a helics_status enumeration indicating any error condition
*/
HELICS_EXPORT helics_status helicsCoreDisconnect (helics_core core);

/** get an existing federate object from a core by name
@details the federate must have been created by one of the other functions and at least one of the objects referencing the created federate
must still be active in the process
@param fedName the name of the federate to retrieve
@return NULL if no fed is available by that name otherwise a helics_federate with that name*/
HELICS_EXPORT helics_federate helicsGetFederateByName (const char *fedName);

/** disconnect a broker
@param broker the broker to disconnect
@return a helics_status enumeration indicating any error condition
*/
HELICS_EXPORT helics_status helicsBrokerDisconnect (helics_broker broker);

/** release the memory associated with a core*/
HELICS_EXPORT void helicsCoreFree (helics_core core);
/** release the memory associated with a broker*/
HELICS_EXPORT void helicsBrokerFree (helics_broker broker);

/* Creation and destruction of Federates */
/** create a value federate from a federate info object
@details helics_federate objects can be used in all functions that take a helics_federate or helics_federate object as an argument
@param fi the federate info object that contains details on the federate
@return an opaque value federate object
*/
HELICS_EXPORT helics_federate helicsCreateValueFederate (const helics_federate_info_t fi);

/** create a value federate from a JSON file or JSON string
@details helics_federate objects can be used in all functions that take a helics_federate or helics_federate object as an argument
@param JSON  a JSON file or a JSON string that contains setup and configuration information
@return an opaque value federate object
*/
HELICS_EXPORT helics_federate helicsCreateValueFederateFromJson (const char *json);

/** create a message federate from a federate info object
@details helics_message_federate objects can be used in all functions that take a helics_message_federate or helics_federate object as an
argument
@param fi the federate info object that contains details on the federate
@return an opaque message federate object
*/
HELICS_EXPORT helics_federate helicsCreateMessageFederate (const helics_federate_info_t fi);

/** create a message federate from a JSON file or JSON string
@details helics_message_federate objects can be used in all functions that take a helics_message_federate or helics_federate object as an
argument
@param JSON  a JSON file or a JSON string that contains setup and configuration information
@return an opaque message federate object
*/
HELICS_EXPORT helics_federate helicsCreateMessageFederateFromJson (const char *json);

/** create a combination federate from a federate info object
@details combination federates are both value federates and message federates, objects can be used in all functions that take a
helics_federate, helics_message_federate or helics_federate object as an argument
@param fi the federate info object that contains details on the federate
@return an opaque value federate object nullptr if the object creation failed
*/
HELICS_EXPORT helics_federate helicsCreateCombinationFederate (const helics_federate_info_t fi);

/** create a combination federate from a JSON file or JSON string
@details combination federates are both value federates and message federates, objects can be used in all functions that take a
helics_federate, helics_message_federate or helics_federate object as an argument
@param JSON  a JSON file or a JSON string that contains setup and configuration information
@return an opaque combination federate object
*/
HELICS_EXPORT helics_federate helicsCreateCombinationFederateFromJson (const char *json);

/** create a new reference to an existing federate
@details this will create a new helics_federate object that references the existing federate it must be freed as well
@param fed an existing helics_federate
@return a new reference to the same federate*/
HELICS_EXPORT helics_federate helicsFederateClone (helics_federate fed);

/** create a federate info object for specifying federate information when constructing a federate
@return a helics_federate_info_t object which is a reference to the created object
*/
HELICS_EXPORT helics_federate_info_t helicsFederateInfoCreate ();

/**load a federate info from command line arguments
@param fi a federateInfo object
@param argc the number of command line arguments
@param argv an array of strings from the command line
@return a helics_status enumeration indicating success or any potential errors
*/
HELICS_EXPORT helics_status helicsFederateInfoLoadFromArgs (helics_federate_info_t fi, int argc, const char *const *argv);

/** delete the memory associated with a federate info object*/
HELICS_EXPORT void helicsFederateInfoFree (helics_federate_info_t fi);

/** set the federate name in the Federate Info structure
@param fi the federate info object to alter
@param name the new identifier for the federate
@return a helics_status enumeration helics_ok on success
*/
HELICS_EXPORT helics_status helicsFederateInfoSetFederateName (helics_federate_info_t fi, const char *name);

/** set the name of the core to link to for a federate
@param fi the federate info object to alter
@param corename the identifier for a core to link to
@return a helics_status enumeration helics_ok on success helicsInvalidReference if fi is not a valid reference
*/
HELICS_EXPORT helics_status helicsFederateInfoSetCoreName (helics_federate_info_t fi, const char *corename);
/** set the initialization string for the core usually in the form of command line arguments
@param fi the federate info object to alter
@param coreInit a string with the core initialization strings
@return a helics_status enumeration helics_ok on success helicsInvalidReference if fi is not a valid reference
*/

HELICS_EXPORT helics_status helicsFederateInfoSetCoreInitString (helics_federate_info_t fi, const char *coreInit);
/** set the core type from a string
@param fi the federate info object to alter
@param coretype a string naming a core type
@return a helics_status enumeration helics_ok on success helicsInvalidReference if fi is not a valid reference helics_discard if the string
is not recognized
*/
HELICS_EXPORT helics_status helicsFederateInfoSetCoreTypeFromString (helics_federate_info_t fi, const char *coretype);

/** set the core type by integer code
@details valid values available by definitions in api-data.h
@param fi the federate info object to alter
@param coretype an numerical code for a core type
@return a helics_status enumeration helics_ok on success helicsInvalidReference if fi is not a valid reference helics_discard if the string
is not recognized
*/
HELICS_EXPORT helics_status helicsFederateInfoSetCoreType (helics_federate_info_t fi, int coretype);
/** set a flag in the info structure
@details valid flags are available  flag-definitions.h
@param fi the federate info object to alter
@param flag a numerical index for a flag
@param value the desired value of the flag helics_true or helics_false
@return a helics_status enumeration helics_ok on success helics_invalid_object if fi is not a valid reference helics_discard if the coretype
is not recognized
*/
HELICS_EXPORT helics_status helicsFederateInfoSetFlag (helics_federate_info_t fi, int flag, helics_bool_t value);

/** set the separator charactor in the info structure
@details the separator character is the separation charactor for local publications/endpoints in creating their global name
for example if the separator character is '/'  then a local endpoint would have a globally reachable name of fedName/localName
@param fi the federate info object to alter
@param separator the character to use as a separator
@return a helics_status enumeration helics_ok on success helics_invalid_object if fi is not a valid reference helics_discard if the coretype
is not recognized
*/
HELICS_EXPORT helics_status helicsFederateInfoSetSeparator(helics_federate_info_t fi, char separator);

/** set the output delay for a federate
@param fi the federate info object to alter
@param outputDelay the desired output delay of the federate
@return a helics_status enumeration helics_ok on success helics_invalid_object if fi is not a valid reference helics_discard if the
specified outputdelay is invalid
*/

HELICS_EXPORT helics_status helicsFederateInfoSetOutputDelay (helics_federate_info_t fi, helics_time_t outputDelay);
/** set the minimum time delta between returns for a federate info object
@param fi the federate info object to alter
@param timeDelta the desired output delay of the federate
@return a helics_status enumeration helics_ok on success helics_invalid_object if fi is not a valid reference helics_discard if the
specified time_delta is invalid
*/

HELICS_EXPORT helics_status helicsFederateInfoSetTimeDelta (helics_federate_info_t fi, helics_time_t timeDelta);
/** set the input delay for a federate info object
@param fi the federate info object to alter
@param inputDelay the desired output delay of the federate
@return a helics_status enumeration helics_ok on success helics_invalid_object if fi is not a valid reference helics_discard if the
specified inputDelay is invalid
*/
HELICS_EXPORT helics_status helicsFederateInfoSetInputDelay (helics_federate_info_t fi, helics_time_t inputDelay);

/** set the time offset for federate in the info object
@details a federate will grant time only on integer periods if the period is specified>0
the offset will shift this return by some amount of time such that the federate will only grant times such as \f$ N*period+offset \f$
@param fi the federateInfo object to alter
@param timeOffset the desired timeOffset
@return a helics_status enumeration helics_ok on success helics_invalid_object if fi is not a valid reference helics_discard if the
specified offset is invalid
*/
HELICS_EXPORT helics_status helicsFederateInfoSetTimeOffset (helics_federate_info_t fi, helics_time_t timeOffset);

/** set the period for federate in the info object
@details a federate will grant time only on integer periods if the period is specified>0
the offset will shift this return by some amount of time such that the federate will only grant times such as \f$ N*period+offset \f$
period must be strictly greater than 0, though setting to 0 implies a period of the timeEpsilon used in HELICS
@param fi the federateInfo object to alter
@param period the desired period
@return a helics_status enumeration helics_ok on success helics_invalid_object if fi is not a valid reference helics_discard if the
specified period is invalid
*/
HELICS_EXPORT helics_status helicsFederateInfoSetPeriod (helics_federate_info_t fi, helics_time_t period);

/** set the max iteration count to use in federate in the info object
@details a federate will iterate for at most min(maxIterations,core maxIterations)
@param fi the federateInfo object to alter
@param maxIterations the maximum number of iterations a federate is allowed per timestep
@return a helics_status enumeration helics_ok on success helics_invalid_object if fi is not a valid reference helics_discard if the
specified offset is invalid
*/
HELICS_EXPORT helics_status helicsFederateInfoSetMaxIterations (helics_federate_info_t fi, int maxIterations);

/** set the logging level of a federate
@details<0 none, 0, errors only, 1+warnings, 2+summary, 3+debug, 4+trace
@param fi the federateInfo object to alter
@param logLevel the specified log level for a federate
@return a helics_status enumeration helics_ok on success helics_invalid_object if fi is not a valid reference
*/
HELICS_EXPORT helics_status helicsFederateInfoSetLoggingLevel (helics_federate_info_t fi, int logLevel);

/** finalize the federate this halts all communication in the federate and disconnects it from the core
 */
HELICS_EXPORT helics_status helicsFederateFinalize (helics_federate fed);
/** release the memory associated withe a federate*/
HELICS_EXPORT void helicsFederateFree (helics_federate fed);

/** call when done using the helics library,  this function will ensure the threads are closed properly if possible
this should be the last call before exiting,  */
HELICS_EXPORT void helicsCloseLibrary ();
/* initialization, execution, and time requests */
/** enter the initialization state of a federate
@details the initialization state allows initial values to be set and received if the iteration is requested on entry to
the execution state
This is a blocking call and will block until the core allows it to proceed
*/
HELICS_EXPORT helics_status helicsFederateEnterInitializationMode (helics_federate fed);
/** non blocking alternative to @helicsFederateEnterInitializationMode
the function helicsFederateEnterInitializationModeFinalize must be called to finish the operation
*/
HELICS_EXPORT helics_status helicsFederateEnterInitializationModeAsync (helics_federate fed);
/** check if the current Asynchronous operation has completed
@param fed the federate to operate on
@return 0 if not completed, 1 if completed*/
HELICS_EXPORT helics_bool_t helicsFederateIsAsyncOperationCompleted (helics_federate fed);

/** finalize the entry to initialize mode that was initiated with @heliceEnterInitializationModeAsync*/
HELICS_EXPORT helics_status helicsFederateEnterInitializationModeComplete (helics_federate fed);

/** request that the federate enter the Execution mode
@details this call is blocking until granted entry by the core object for an asynchronous alternative call
/ref helicsFederateEnterExecutionModeAsync  on return from this call the federate will be at time 0
@param fed a federate to change modes
@return a helics_status enumeration helics_error if something went wrong helicsInvalidReference if fed is invalid
*/
HELICS_EXPORT helics_status helicsFederateEnterExecutionMode (helics_federate fed);

/** request that the federate enter the Execution mode
@details this call is non-blocking and will return immediately call /ref helicsFederateEnterExecutionModeComplete to finish the call
sequence /ref
*/
HELICS_EXPORT helics_status helicsFederateEnterExecutionModeAsync (helics_federate fed);

/** complete the call to /ref EnterExecutionModeAsync
@param fed the federate object to complete the call
*/
HELICS_EXPORT helics_status helicsFederateEnterExecutionModeComplete (helics_federate fed);

/** request an iterative time
@details this call allows for finer grain control of the iterative process then /ref helicsFederateRequestTime it takes a time and and
iteration request and return a time and iteration status
@param fed the federate to make the request of
@param iterate the requested iteration mode
@param[out] outIterate  the iteration specification of the result
@return a helics_status object with a return code of the result
*/
HELICS_EXPORT helics_status helicsFederateEnterExecutionModeIterative (helics_federate fed,
                                                                       helics_iteration_request iterate,
                                                                       helics_iteration_status *outIterate);

/** request an iterative entry to the execution mode
@details this call allows for finer grain control of the iterative process then /ref helicsFederateRequestTime it takes a time and and
iteration request and return a time and iteration status
@param fed the federate to make the request of
@param iterate the requested iteration mode
@return a helics_status object with a return code of the result
*/
HELICS_EXPORT helics_status helicsFederateEnterExecutionModeIterativeAsync (helics_federate fed, helics_iteration_request iterate);

/** complete the asynchronous iterative call into ExecutionModel
@param fed the federate to make the request of
@param[out] outIterate  the iteration specification of the result
@return a helics_status object with a return code of the result helics_ok if there were no issues
*/
HELICS_EXPORT helics_status helicsFederateEnterExecutionModeIterativeComplete (helics_federate fed, helics_iteration_status *outIterate);

/** get the current state of a federate
@param fed the fed to query
@param[out] state the resulting state if helics_status return helics_ok*/
HELICS_EXPORT helics_status helicsFederateGetState (helics_federate fed, federate_state *state);

/** get the core object associated with a federate
@param fed a federate object
@return a core object, nullptr if invalid
*/
HELICS_EXPORT helics_core helicsFederateGetCoreObject (helics_federate fed);

/** request the next time for federate execution
@param fed the federate to make the request of
@param requestTime the next requested time
@param[out]  timeOut the time granted to the federate
@return a helics_status if the return value is equal to helics_ok the timeOut will contain the new granted time, otherwise timeOut is
invalid*/
HELICS_EXPORT helics_status helicsFederateRequestTime (helics_federate fed, helics_time_t requestTime, helics_time_t *timeOut);

/** request an iterative time
@details this call allows for finer grain control of the iterative process then /ref helicsFederateRequestTime it takes a time and and
iteration request and return a time and iteration status
@param fed the federate to make the request of
@param requestTime the next desired time
@param iterate the requested iteration mode
@param[out] timeOut the granted time
@param[out] outIterate  the iteration specification of the result
@return a helics_status object with a return code of the result
*/
HELICS_EXPORT helics_status helicsFederateRequestTimeIterative (helics_federate fed,
                                                                helics_time_t requestTime,
                                                                helics_iteration_request iterate,
                                                                helics_time_t *timeOut,
                                                                helics_iteration_status *outIterate);

/** request the next time for federate execution in an asynchronous call
@details call /ref helicsFederateRequestTimeComplete to finish the call
@param fed the federate to make the request of
@param requestTime the next requested time
@return a helics_status if the return value is equal to helics_ok*/
HELICS_EXPORT helics_status helicsFederateRequestTimeAsync (helics_federate fed, helics_time_t requestTime);

/** complete an asynchronous requestTime call
@param fed the federate to make the request of
@param[out]  timeOut the time granted to the federate
@return a helics_status if the return value is equal to helics_ok the timeOut will contain the new granted time, otherwise timeOut is
invalid*/
HELICS_EXPORT helics_status helicsFederateRequestTimeComplete (helics_federate fed, helics_time_t *timeOut);

/** request an iterative time through an asynchronous call
@details this call allows for finer grain control of the iterative process then /ref helicsFederateRequestTime it takes a time and and
iteration request and return a time and iteration status call /ref helicsFederateRequestTimeIterativeComplete to finish the process
@param fed the federate to make the request of
@param requestTime the next desired time
@param iterate the requested iteration mode
@return a helics_status object with a return code of the result
*/
HELICS_EXPORT helics_status helicsFederateRequestTimeIterativeAsync (helics_federate fed,
                                                                     helics_time_t requestTime,
                                                                     helics_iteration_request iterate);

/** complete an iterative time request asynchronous call
@param fed the federate to make the request of
@param[out] timeOut the granted time
@param[out] outIterate  the iteration specification of the result
@return a helics_status object with a return code of the result
*/
HELICS_EXPORT helics_status helicsFederateRequestTimeIterativeComplete (helics_federate fed,
                                                                        helics_time_t *timeOut,
                                                                        helics_iteration_status *outIterate);

/** get the name of the federate
@param fed the federate object to query
@param[out] outputString memory buffer to store the result
@param maxlen the maximum size of the buffer
@return helics_status object indicating success or error
*/
HELICS_EXPORT helics_status helicsFederateGetName (helics_federate fed, char *outputString, int maxlen);

/** set the minimum time delta for the federate
@param[in] tdelta the minimum time delta to return from a time request function
*/
HELICS_EXPORT helics_status helicsFederateSetTimeDelta (helics_federate fed, helics_time_t time);
/** set the look ahead time
@details the look ahead is the propagation time for messages/event to propagate from the Federate
the federate
@param[in] lookAhead the look ahead time
*/
HELICS_EXPORT helics_status helicsFederateSetOutputDelay (helics_federate fed, helics_time_t outputDelay);

/** set the impact Window time
@details the impact window is the time window around the time request in which other federates cannot affect
the federate
@param[in] lookAhead the look ahead time
*/
HELICS_EXPORT helics_status helicsFederateSetInputDelay (helics_federate fed, helics_time_t inputDelay);
/** set the period and offset of the federate
@details the federate will on grant time on N*period+offset interval
@param[in] period the length of time between each subsequent grants
@param[in] offset the shift of the period from 0  offset must be < period
*/
HELICS_EXPORT helics_status helicsFederateSetPeriod (helics_federate fed, helics_time_t period, helics_time_t offset);
/** set a flag for the federate
@param fed the federate to alter a flag for
@param flag the flag to change
@param flagValue the new value of the flag 0 for false !=0 for true
*/
HELICS_EXPORT helics_status helicsFederateSetFlag (helics_federate fed, int flag, helics_bool_t flagValue);

/** set the separator charactor in the info structure
@details the separator character is the separation charactor for local publications/endpoints in creating their global name
for example if the separator character is '/'  then a local endpoint would have a globally reachable name of fedName/localName
@param fi the federate info object to alter
@param separator the character to use as a separator
@return a helics_status enumeration helics_ok on success helics_invalid_object if fi is not a valid reference helics_discard if the coretype
is not recognized
*/
HELICS_EXPORT helics_status helicsFederateSetSeparator(helics_federate fed, char separator);

/**  set the logging level for the federate
@ details debug and trace only do anything if they were enabled in the compilation
@param loggingLevel (-1: none, 0: error_only, 1: warnings, 2: normal, 3: debug, 4: trace)
*/
HELICS_EXPORT helics_status helicsFederateSetLoggingLevel (helics_federate fed, int loggingLevel);

/** set the max iteration count to use in federate in the info object
@details a federate will iterate for at most min(maxIterations,core maxIterations)
@param fi the federateInfo object to alter
@param maxIterations the maximum number of iterations a federate is allowed per timestep
@return a helics_status enumeration helics_ok on success helics_invalid_object if fed is not a valid reference helics_discard if the
specified offset is invalid
*/
HELICS_EXPORT helics_status helicsFederateSetMaxIterations (helics_federate_info_t fi, int maxIterations);

/** get the current time of the federate
@param fed the federate object to query
@param[out] time storage location for the time variable
@return helics_status object indicating success or error
*/
HELICS_EXPORT helics_status helicsFederateGetCurrentTime (helics_federate fed, helics_time_t *time);
/** create a query object
@details a query object consists of a target and query string
*/
HELICS_EXPORT helics_query helicsCreateQuery (const char *target, const char *query);

/** Execute a query
@details the call will block until the query finishes which may require communication or other delays
@param query the query object to use in the query
@param fed a federate to send the query through
@return a pointer to a string.  the string will remain valid until the query is freed or executed again
the return will be nullptr if fed or query is an invalid object, the return string will be "#invalid" if the query itself was invalid
*/
HELICS_EXPORT const char *helicsQueryExecute (helics_query query, helics_federate fed);

/** Execute a query directly on a core
@details the call will block until the query finishes which may require communication or other delays
@param query the query object to use in the query
@param core the core to send the query to
@return a pointer to a string.  the string will remain valid until the query is freed or executed again
the return will be nullptr if fed or query is an invalid object, the return string will be "#invalid" if the query itself was invalid
*/
HELICS_EXPORT const char *helicsQueryCoreExecute(helics_query query, helics_core core);

/** Execute a query directly on a broker
@details the call will block until the query finishes which may require communication or other delays
@param query the query object to use in the query
@param broker the broker to send the query to
@return a pointer to a string.  the string will remain valid until the query is freed or executed again
the return will be nullptr if fed or query is an invalid object, the return string will be "#invalid" if the query itself was invalid
*/
HELICS_EXPORT const char *helicsQueryBrokerExecute(helics_query query, helics_broker broker);

/** Execute a query in a non-blocking call
@param query the query object to use in the query
@param fed a federate to send the query through
@return a helics status enumeration with the result of the query specification
*/
HELICS_EXPORT helics_status helicsQueryExecuteAsync (helics_query query, helics_federate fed);

/** complete the return from a query called with /ref helicsExecuteQueryAsync
@details the function will block until the query completes /ref isQueryComplete can be called to determine if a query has completed or not
@param query the query object to
@return a pointer to a string.  the string will remain valid until the query is freed or executed again
the return will be nullptr if query is an invalid object
*/
HELICS_EXPORT const char *helicsQueryExecuteComplete (helics_query query);

/** check if an asynchronously executed query has completed
@return will return helics_true if an asynchronous query has complete or a regular query call was made with a result
and false if an asynchronous query has not completed or is invalid
*/
HELICS_EXPORT helics_bool_t helicsQueryIsCompleted (helics_query query);

/** free the memory associated with a query object*/
HELICS_EXPORT void helicsQueryFree (helics_query);

/** function to do some housekeeping work
@details this runs some cleanup routines and tries to close out any residual thread that haven't been shutdown
yet*/
HELICS_EXPORT void helicsCleanupHelicsLibrary ();

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif
