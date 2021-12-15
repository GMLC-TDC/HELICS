/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef HELICS_C_API_H_
#define HELICS_C_API_H_

#pragma once

#include <stdint.h>
#include <stdlib.h>

#ifndef HELICS_EXPORT
#    if defined _WIN32 || defined __CYGWIN__
#        ifdef __GNUC__
#            define HELICS_EXPORT __attribute__((dllimport))
#        else
#            define HELICS_EXPORT __declspec(dllimport)
#        endif
#    else
#        define HELICS_EXPORT
#    endif
#endif

#ifndef HELICS_DEPRECATED
#    if defined _WIN32 || defined __CYGWIN__
#        ifdef __GNUC__
#            define HELICS__DEPRECATED __attribute__((deprecated))
#        else
#            define HELICS__DEPRECATED __declspec(deprecated)
#        endif
#    else
#        define HELICS__DEPRECATED __attribute__((deprecated))
#    endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
/** @file
@brief base helics enumerations for C and C++ API's
*/

/** pick a core type depending on compile configuration usually either ZMQ if available or TCP */
typedef enum {
    /** a default core type that will default to something available*/
    HELICS_CORE_TYPE_DEFAULT = 0,
    /** use the Zero MQ networking protocol */
    HELICS_CORE_TYPE_ZMQ = 1,
    /** use MPI for operation on a parallel cluster */
    HELICS_CORE_TYPE_MPI = 2,
    /** use the Test core if all federates are in the same process */
    HELICS_CORE_TYPE_TEST = 3,
    /** interprocess uses memory mapped files to transfer data (for use when all federates are
        on the same machine */
    HELICS_CORE_TYPE_INTERPROCESS = 4,
    /** interprocess uses memory mapped files to transfer data (for use when all federates are
        on the same machine ipc is the same as /ref HELICS_CORE_TYPE_interprocess*/
    HELICS_CORE_TYPE_IPC = 5,
    /** use a generic TCP protocol message stream to send messages */
    HELICS_CORE_TYPE_TCP = 6,
    /** use UDP packets to send the data */
    HELICS_CORE_TYPE_UDP = 7,
    /** single socket version of ZMQ core usually for high fed count on the same system*/
    HELICS_CORE_TYPE_ZMQ_SS = 10,
    /** for using the nanomsg communications */
    HELICS_CORE_TYPE_NNG = 9,
    /** a single socket version of the TCP core for more easily handling firewalls*/
    HELICS_CORE_TYPE_TCP_SS = 11,
    /** a core type using http for communication*/
    HELICS_CORE_TYPE_HTTP = 12,
    /** a core using websockets for communication*/
    HELICS_CORE_TYPE_WEBSOCKET = 14,
    /** an in process core type for handling communications in shared
                                     memory it is pretty similar to the test core but stripped from
                                     the "test" components*/
    HELICS_CORE_TYPE_INPROC = 18,
    /** an explicit core type that is recognized but explicitly doesn't
                                  exist, for testing and a few other assorted reasons*/
    HELICS_CORE_TYPE_NULL = 66,
    /** an explicit core type exists but does nothing but return empty values or sink calls*/
    HELICS_CORE_TYPE_EMPTY = 77
} HelicsCoreTypes;

/** enumeration of allowable data types for publications and inputs*/
typedef enum {
    HELICS_DATA_TYPE_UNKNOWN = -1,
    /** a sequence of characters*/
    HELICS_DATA_TYPE_STRING = 0,
    /** a double precision floating point number*/
    HELICS_DATA_TYPE_DOUBLE = 1,
    /** a 64 bit integer*/
    HELICS_DATA_TYPE_INT = 2,
    /** a pair of doubles representing a complex number*/
    HELICS_DATA_TYPE_COMPLEX = 3,
    /** an array of doubles*/
    HELICS_DATA_TYPE_VECTOR = 4,
    /** a complex vector object*/
    HELICS_DATA_TYPE_COMPLEX_VECTOR = 5,
    /** a named point consisting of a string and a double*/
    HELICS_DATA_TYPE_NAMED_POINT = 6,
    /** a boolean data type*/
    HELICS_DATA_TYPE_BOOLEAN = 7,
    /** time data type*/
    HELICS_DATA_TYPE_TIME = 8,
    /** raw data type*/
    HELICS_DATA_TYPE_RAW = 25,
    /** type converts to a valid json string*/
    HELICS_DATA_TYPE_JSON = 30,
    /** the data type can change*/
    HELICS_DATA_TYPE_MULTI = 33,
    /** open type that can be anything*/
    HELICS_DATA_TYPE_ANY = 25262
} HelicsDataTypes;

/** single character data type  this is intentionally the same as string*/
#define HELICS_DATA_TYPE_CHAR HELICS_DATA_TYPE_STRING

/** enumeration of possible federate flags*/
typedef enum {
    /** flag indicating that a federate is observe only*/
    HELICS_FLAG_OBSERVER = 0,
    /** flag indicating that a federate can only return requested times*/
    HELICS_FLAG_UNINTERRUPTIBLE = 1,
    /** flag indicating that a federate can be interrupted*/
    HELICS_FLAG_INTERRUPTIBLE = 2,
    /** flag indicating that a federate/interface is a signal generator only*/
    HELICS_FLAG_SOURCE_ONLY = 4,
    /** flag indicating a federate/interface should only transmit values if they have changed(binary
           equivalence)*/
    HELICS_FLAG_ONLY_TRANSMIT_ON_CHANGE = 6,
    /** flag indicating a federate/interface should only trigger an update if a value has changed
     * (binary equivalence)*/
    HELICS_FLAG_ONLY_UPDATE_ON_CHANGE = 8,
    /** flag indicating a federate should only grant time if all other federates have already passed
     * the requested time*/
    HELICS_FLAG_WAIT_FOR_CURRENT_TIME_UPDATE = 10,
    /** flag indicating a federate should operate on a restrictive time policy, which disallows some
       2nd order time evaluation and can be useful for certain types of dependency cycles
        and update patterns, but generally shouldn't be used as it can lead to some very slow update
       conditions*/
    HELICS_FLAG_RESTRICTIVE_TIME_POLICY = 11,
    /** flag indicating that a federate has rollback capability*/
    HELICS_FLAG_ROLLBACK = 12,
    /** flag indicating that a federate performs forward computation and does internal rollback*/
    HELICS_FLAG_FORWARD_COMPUTE = 14,
    /** flag indicating that a federate needs to run in real time*/
    HELICS_FLAG_REALTIME = 16,
    /** flag indicating that the federate will only interact on a single thread*/
    HELICS_FLAG_SINGLE_THREAD_FEDERATE = 27,
    /** used to not display warnings on mismatched requested times*/
    HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS = 67,
    /** specify that checking on configuration files should be strict and throw and error on any
   invalid values */
    HELICS_FLAG_STRICT_CONFIG_CHECKING = 75,
    /** specify that the federate should use json serialization for all data types*/
    HELICS_FLAG_USE_JSON_SERIALIZATION = 79,
    /** specify that the federate is event triggered-meaning (all/most) events are triggered by
       incoming events*/
    HELICS_FLAG_EVENT_TRIGGERED = 81,
    /** specify that that federate should capture the profiling data to the local federate logging
       system*/
    HELICS_FLAG_LOCAL_PROFILING_CAPTURE = 96
} HelicsFederateFlags;

/** enumeration of additional core flags*/
typedef enum {
    /** used to delay a core from entering initialization mode even if it would otherwise be ready*/
    HELICS_FLAG_DELAY_INIT_ENTRY = 45,
    /** used to clear the HELICS_DELAY_INIT_ENTRY flag in cores*/
    HELICS_FLAG_ENABLE_INIT_ENTRY = 47,
    /** ignored flag used to test some code paths*/
    HELICS_FLAG_IGNORE = 999
} HelicsCoreFlags;

/** enumeration of general flags that can be used in federates/cores/brokers */
typedef enum {
    /** flag specifying that a federate, core, or broker may be slow to respond to pings
        If the federate goes offline there is no good way to detect it so use with caution
        */
    HELICS_FLAG_SLOW_RESPONDING = 29,
    /** flag specifying the federate/core/broker is operating in a user debug mode so deadlock
    timers and timeout are disabled this flag is a combination of slow_responding and disabling of
    some timeouts*/
    HELICS_FLAG_DEBUGGING = 31,
    /** specify that a federate error should terminate the federation*/
    HELICS_FLAG_TERMINATE_ON_ERROR = 72,
    /** specify that the log files should be flushed on every log message*/
    HELICS_FLAG_FORCE_LOGGING_FLUSH = 88,
    /** specify that a full log should be dumped into a file*/
    HELICS_FLAG_DUMPLOG = 89,
    /** specify that helics should capture profiling data*/
    HELICS_FLAG_PROFILING = 93,
    /** flag trigger for generating a profiling marker*/
    HELICS_FLAG_PROFILING_MARKER = 95
} HelicsFlags;

/** log level definitions
 */
typedef enum {
    /** log level for dumping log messages*/
    HELICS_LOG_LEVEL_DUMPLOG = -10,
    /** don't print anything except a few catastrophic errors*/
    HELICS_LOG_LEVEL_NO_PRINT = -4,
    /** only print error level indicators*/
    HELICS_LOG_LEVEL_ERROR = 0,
    /** profiling log level*/
    HELICS_LOG_LEVEL_PROFILING = 2,
    /** only print warnings and errors*/
    HELICS_LOG_LEVEL_WARNING = 3,
    /** warning errors and summary level information*/
    HELICS_LOG_LEVEL_SUMMARY = 6,
    /** summary+ notices about federate and broker connections +messages about network
       connections*/
    HELICS_LOG_LEVEL_CONNECTIONS = 9,
    /** connections+ interface definitions*/
    HELICS_LOG_LEVEL_INTERFACES = 12,
    /** interfaces + timing message*/
    HELICS_LOG_LEVEL_TIMING = 15,
    /** timing+ data transfer notices*/
    HELICS_LOG_LEVEL_DATA = 18,
    /** data+ additional debug message*/
    HELICS_LOG_LEVEL_DEBUG = 21,
    /** all internal messages*/
    HELICS_LOG_LEVEL_TRACE = 24
} HelicsLogLevels;

/** enumeration of return values from the C interface functions
 */
typedef enum {
    /** global fatal error for federation */
    HELICS_ERROR_FATAL = -404,
    /** an unknown non-helics error was produced */
    HELICS_ERROR_EXTERNAL_TYPE = -203,
    /** the function produced a helics error of some other type */
    HELICS_ERROR_OTHER = -101,
    /** user system abort*/
    HELICS_ERROR_USER_ABORT = -27,
    /** insufficient space is available to store requested data */
    HELICS_ERROR_INSUFFICIENT_SPACE = -18,
    HELICS_ERROR_EXECUTION_FAILURE = -14, /*!< the function execution has failed */
    /** the call made was invalid in the present state of the calling object */
    HELICS_ERROR_INVALID_FUNCTION_CALL = -10,
    /** error issued when an invalid state transition occurred */
    HELICS_ERROR_INVALID_STATE_TRANSITION = -9,
    /** the function issued a warning of some kind */
    HELICS_WARNING = -8,
    /** the federate has terminated unexpectedly and the call cannot be completed */
    HELICS_ERROR_SYSTEM_FAILURE = -6,
    /** the input was discarded and not used for some reason */
    HELICS_ERROR_DISCARD = -5,
    /** the parameter passed was invalid and unable to be used */
    HELICS_ERROR_INVALID_ARGUMENT = -4,
    /** indicator that the object used was not a valid object */
    HELICS_ERROR_INVALID_OBJECT = -3,
    /** the operation to connect has failed */
    HELICS_ERROR_CONNECTION_FAILURE = -2,
    /** registration has failed */
    HELICS_ERROR_REGISTRATION_FAILURE = -1,
    /** the function executed successfully */
    HELICS_OK = 0
} HelicsErrorTypes;

const int HELICS_INVALID_OPTION_INDEX = -101;

/** enumeration of properties that apply to federates*/
typedef enum {
    /** the property controlling the minimum time delta for a federate*/
    HELICS_PROPERTY_TIME_DELTA = 137,
    /** the property controlling the period for a federate*/
    HELICS_PROPERTY_TIME_PERIOD = 140,
    /** the property controlling time offset for the period of federate*/
    HELICS_PROPERTY_TIME_OFFSET = 141,
    /** the property controlling real time lag for a federate the max time a federate can lag
       real time*/
    HELICS_PROPERTY_TIME_RT_LAG = 143,
    /** the property controlling real time lead for a federate the max time a federate can be
       ahead of real time*/
    HELICS_PROPERTY_TIME_RT_LEAD = 144,
    /** the property controlling real time tolerance for a federate sets both rt_lag and
       rt_lead*/
    HELICS_PROPERTY_TIME_RT_TOLERANCE = 145,
    /** the property controlling input delay for a federate*/
    HELICS_PROPERTY_TIME_INPUT_DELAY = 148,
    /** the property controlling output delay for a federate*/
    HELICS_PROPERTY_TIME_OUTPUT_DELAY = 150,
    /** the property specifying a timeout to trigger actions if the time for granting exceeds a
       certain threshold*/
    HELICS_PROPERTY_TIME_GRANT_TIMEOUT = 161,
    /** integer property controlling the maximum number of iterations in a federate*/
    HELICS_PROPERTY_INT_MAX_ITERATIONS = 259,
    /** integer property controlling the log level in a federate see \ref HelicsLogLevels*/
    HELICS_PROPERTY_INT_LOG_LEVEL = 271,
    /** integer property controlling the log level for file logging in a federate see \ref
       HelicsLogLevels*/
    HELICS_PROPERTY_INT_FILE_LOG_LEVEL = 272,
    /** integer property controlling the log level for file logging in a federate see \ref
       HelicsLogLevels*/
    HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL = 274
} HelicsProperties;

/** result returned for requesting the value of an invalid/unknown property */
const int HELICS_INVALID_PROPERTY_VALUE = -972;

/** enumeration of the multi_input operations*/
typedef enum {
    /** time and priority order the inputs from the core library*/
    HELICS_MULTI_INPUT_NO_OP = 0,
    /** vectorize the inputs either double vector or string vector*/
    HELICS_MULTI_INPUT_VECTORIZE_OPERATION = 1,
    /** all inputs are assumed to be boolean and all must be true to return true*/
    HELICS_MULTI_INPUT_AND_OPERATION = 2,
    /** all inputs are assumed to be boolean and at least one must be true to return true*/
    HELICS_MULTI_INPUT_OR_OPERATION = 3,
    /** sum all the inputs*/
    HELICS_MULTI_INPUT_SUM_OPERATION = 4,
    /** do a difference operation on the inputs, first-sum(rest)
    for double input, vector diff for vector input*/
    HELICS_MULTI_INPUT_DIFF_OPERATION = 5,
    /** find the max of the inputs*/
    HELICS_MULTI_INPUT_MAX_OPERATION = 6,
    /** find the min of the inputs*/
    HELICS_MULTI_INPUT_MIN_OPERATION = 7,
    /** take the average of the inputs*/
    HELICS_MULTI_INPUT_AVERAGE_OPERATION = 8
} HelicsMultiInputModes;

/** enumeration of options that apply to handles*/
typedef enum {
    /** specify that a connection is required for an interface and will generate an error if not
       available*/
    HELICS_HANDLE_OPTION_CONNECTION_REQUIRED = 397,
    /** specify that a connection is NOT required for an interface and will only be made if
       available no warning will be issues if not available*/
    HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL = 402,
    /** specify that only a single connection is allowed for an interface*/
    HELICS_HANDLE_OPTION_SINGLE_CONNECTION_ONLY = 407,
    /** specify that multiple connections are allowed for an interface*/
    HELICS_HANDLE_OPTION_MULTIPLE_CONNECTIONS_ALLOWED = 409,
    /** specify that the last data should be buffered and sent on subscriptions after init*/
    HELICS_HANDLE_OPTION_BUFFER_DATA = 411,
    /** specify that the types should be checked strictly for pub/sub and filters*/
    HELICS_HANDLE_OPTION_STRICT_TYPE_CHECKING = 414,
    /** specify that the mismatching units should be ignored*/
    HELICS_HANDLE_OPTION_IGNORE_UNIT_MISMATCH = 447,
    /** specify that an interface will only transmit on change(only applicable to
       publications)*/
    HELICS_HANDLE_OPTION_ONLY_TRANSMIT_ON_CHANGE = 452,
    /** specify that an interface will only update if the value has actually changed*/
    HELICS_HANDLE_OPTION_ONLY_UPDATE_ON_CHANGE = 454,
    /** specify that an interface does not participate in determining time interrupts*/
    HELICS_HANDLE_OPTION_IGNORE_INTERRUPTS = 475,
    /** specify the multi-input processing method for inputs*/
    HELICS_HANDLE_OPTION_MULTI_INPUT_HANDLING_METHOD = 507,
    /** specify the source index with the highest priority*/
    HELICS_HANDLE_OPTION_INPUT_PRIORITY_LOCATION = 510,
    /** specify that the priority list should be cleared or question if it is cleared*/
    HELICS_HANDLE_OPTION_CLEAR_PRIORITY_LIST = 512,
    /** specify the required number of connections or get the actual number of connections*/
    HELICS_HANDLE_OPTION_CONNECTIONS = 522
} HelicsHandleOptions;

/** enumeration of the predefined filter types*/
typedef enum {
    /** a custom filter type that executes a user defined callback*/
    HELICS_FILTER_TYPE_CUSTOM = 0,
    /** a filter type that executes a fixed delay on a message*/
    HELICS_FILTER_TYPE_DELAY = 1,
    /** a filter type that executes a random delay on the messages*/
    HELICS_FILTER_TYPE_RANDOM_DELAY = 2,
    /** a filter type that randomly drops messages*/
    HELICS_FILTER_TYPE_RANDOM_DROP = 3,
    /** a filter type that reroutes a message to a different destination than originally
       specified*/
    HELICS_FILTER_TYPE_REROUTE = 4,
    /** a filter type that duplicates a message and sends the copy to a different destination*/
    HELICS_FILTER_TYPE_CLONE = 5,
    /** a customizable filter type that can perform different actions on a message based on
       firewall like rules*/
    HELICS_FILTER_TYPE_FIREWALL = 6
} HelicsFilterTypes;

/** enumeration of sequencing modes for queries and commands
fast is the default, meaning the query travels along priority channels and takes precedence of over
existing messages; ordered means it follows normal priority patterns and will be ordered along with
existing messages
*/
typedef enum {
    /** sequencing mode to operate on priority channels*/
    HELICS_SEQUENCING_MODE_FAST = 0,
    /** sequencing mode to operate on the normal channels*/
    HELICS_SEQUENCING_MODE_ORDERED = 1,
    /** select the default channel*/
    HELICS_SEQUENCING_MODE_DEFAULT = 2
} HelicsSequencingModes;

#define HELICS_BIG_NUMBER 9223372036.854774
const double cHelicsBigNumber = HELICS_BIG_NUMBER;

/**
 * @file
 * @brief Data structures for the C api
 */

/**
 * opaque object representing an input
 */
typedef void* HelicsInput;
// typedef void* helics_input;
/**
 * opaque object representing a publication
 */
typedef void* HelicsPublication;
// typedef void* helics_publication;
/**
 * opaque object representing an endpoint
 */
// typedef void* helics_endpoint;
typedef void* HelicsEndpoint;

/**
 * opaque object representing a filter
 */
// typedef void* helics_filter;
typedef void* HelicsFilter;

/**
 * opaque object representing a core
 */
// typedef void* helics_core;
typedef void* HelicsCore;

/**
 * opaque object representing a broker
 */
// typedef void* helics_broker;
typedef void* HelicsBroker;
/**
 * opaque object representing a federate
 */
// typedef void* helics_federate;
typedef void* HelicsFederate;

/**
 * opaque object representing a filter info object structure
 */
// typedef void* helics_federate_info;
typedef void* HelicsFederateInfo;

/**
 * opaque object representing a query
 */
// typedef void* helics_query;
typedef void* HelicsQuery;

/**
 * opaque object representing a string buffer for a query
 */
// typedef void* helics_query_buffer;
typedef void* HelicsQueryBuffer;

/**
 * opaque object representing a message
 */
// typedef void* helics_message;
typedef void* HelicsMessage;

/**
 * time definition used in the C interface to helics
 */
// typedef double helics_time;
typedef double HelicsTime;

const HelicsTime HELICS_TIME_ZERO = 0.0; /*!< definition of time zero-the beginning of simulation */
const HelicsTime HELICS_TIME_EPSILON = 1.0e-9; /*!< definition of the minimum time resolution */
const HelicsTime HELICS_TIME_INVALID = -1.785e39; /*!< definition of an invalid time that has no meaning */
const HelicsTime HELICS_TIME_MAXTIME = HELICS_BIG_NUMBER; /*!< definition of time signifying the federate has
                                                             terminated or run until the end of the simulation*/

/**
 * defining a boolean type for use in the helics interface
 */
// typedef int helics_bool;
typedef int HelicsBool;

const HelicsBool HELICS_TRUE = 1; /*!< indicator used for a true response */
const HelicsBool HELICS_FALSE = 0; /*!< indicator used for a false response */

/**
 * enumeration of the different iteration results
 */
typedef enum {
    HELICS_ITERATION_REQUEST_NO_ITERATION = 0, /*!< no iteration is requested */
    HELICS_ITERATION_REQUEST_FORCE_ITERATION = 1, /*!< force iteration return when able */
    HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED = 2 /*!< only return an iteration if necessary */
} HelicsIterationRequest;

/**
 * enumeration of possible return values from an iterative time request
 */
typedef enum {
    HELICS_ITERATION_RESULT_NEXT_STEP = 0, /*!< the iterations have progressed to the next time */
    HELICS_ITERATION_RESULT_ERROR = 1, /*!< there was an error */
    HELICS_ITERATION_RESULT_HALTED = 2, /*!< the federation has halted */
    HELICS_ITERATION_RESULT_ITERATING = 3 /*!< the federate is iterating at current time */
} HelicsIterationResult;

/**
 * enumeration of possible federate states
 */
typedef enum {
    HELICS_STATE_STARTUP = 0, /*!< when created the federate is in startup state */
    HELICS_STATE_INITIALIZATION = 1, /*!< entered after the enterInitializingMode call has returned */
    HELICS_STATE_EXECUTION = 2, /*!< entered after the enterExectuationState call has returned */
    HELICS_STATE_FINALIZE = 3, /*!< the federate has finished executing normally final values may be retrieved */
    HELICS_STATE_ERROR = 4, /*!< error state no core communication is possible but values can be retrieved */
    /* the following states are for asynchronous operations */
    HELICS_STATE_PENDING_INIT = 5, /*!< indicator that the federate is pending entry to initialization state */
    HELICS_STATE_PENDING_EXEC = 6, /*!< state pending EnterExecution State */
    HELICS_STATE_PENDING_TIME = 7, /*!< state that the federate is pending a timeRequest */
    HELICS_STATE_PENDING_ITERATIVE_TIME = 8, /*!< state that the federate is pending an iterative time request */
    HELICS_STATE_PENDING_FINALIZE = 9, /*!< state that the federate is pending a finalize request */
    HELICS_STATE_FINISHED = 10 /*!< state that the federate is finished simulating but still connected */
} HelicsFederateState;

/**
 *  structure defining a basic complex type
 */
typedef struct HelicsComplex {
    double real;
    double imag;
} HelicsComplex;

// typedef HelicsComplex helics_complex;

/**
 * helics error object
 *
 * if error_code==0 there is no error, if error_code!=0 there is an error and message will contain a string,
 * otherwise it will be an empty string
 */
typedef struct HelicsError {
    int32_t error_code; /*!< an error code associated with the error*/
    const char* message; /*!< a message associated with the error*/
} HelicsError;

// typedef helics_error HelicsError;

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

/** Clear HELICS based signal handlers.*/
HELICS_EXPORT void helicsClearSignalHandler();

/** Load a custom signal handler to execute prior to the abort signal handler.
@details  This function is not 100% reliable it will most likely work but uses some functions and
techniques that are not 100% guaranteed to work in a signal handler
and in worst case it could deadlock.  That is somewhat unlikely given usage patterns
but it is possible.  The callback has signature helics_bool(*handler)(int) and it will take the SIG_INT as an argument
and return a boolean.  If the boolean return value is helics_true (or the callback is null) the default signal handler is run after the
callback finishes; if it is helics_false the default callback is not run and the default signal handler is executed.*/
HELICS_EXPORT void helicsLoadSignalHandlerCallback(HelicsBool (*handler)(int));

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
 *                   Typical options include a broker name, the broker address, the number of federates, etc.
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
 *
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
 *
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
 * @param fedName The name of the federate to create, can NULL or an empty string to use the default name from fi or an assigned name.
 * @param fi The federate info object that contains details on the federate.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 * @return An opaque value federate object.
 */
HELICS_EXPORT HelicsFederate helicsCreateValueFederate(const char* fedName, HelicsFederateInfo fi, HelicsError* err);

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
 * @param fi The federate info object that contains details on the federate.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 * @return An opaque message federate object.
 */
HELICS_EXPORT HelicsFederate helicsCreateMessageFederate(const char* fedName, HelicsFederateInfo fi, HelicsError* err);

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
 * @param fedName A string with the name of the federate, can be NULL or an empty string to pull the default name from fi.
 * @param fi The federate info object that contains details on the federate.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 * @return An opaque value federate object nullptr if the object creation failed.
 */
HELICS_EXPORT HelicsFederate helicsCreateCombinationFederate(const char* fedName, HelicsFederateInfo fi, HelicsError* err);

/**
 * Create a combination federate from a JSON file or JSON string or TOML file.
 *
 * @details Combination federates are both value federates and message federates, objects can be used in all functions
 *          that take a HelicsFederate, helics_message_federate or HelicsFederate object as an argument
 *
 * @param configFile A JSON file or a JSON string or TOML file that contains setup and configuration information.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 * @return An opaque combination federate object.
 */
HELICS_EXPORT HelicsFederate helicsCreateCombinationFederateFromConfig(const char* configFile, HelicsError* err);

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
 * Create a federate info object for specifying federate information when constructing a federate.
 *
 * @return A HelicsFederateInfo object which is a reference to the created object.
 */
HELICS_EXPORT HelicsFederateInfo helicsCreateFederateInfo(void);

/**
 * Create a federate info object from an existing one and clone the information.
 *
 * @param fi A federateInfo object to duplicate.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 *
 *  @return A HelicsFederateInfo object which is a reference to the created object.
 */
HELICS_EXPORT HelicsFederateInfo helicsFederateInfoClone(HelicsFederateInfo fi, HelicsError* err);

/**
 * Load federate info from command line arguments.
 *
 * @param fi A federateInfo object.
 * @param argc The number of command line arguments.
 * @param argv An array of strings from the command line.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoLoadFromArgs(HelicsFederateInfo fi, int argc, const char* const* argv, HelicsError* err);

/**
 * Load federate info from command line arguments contained in a string.
 *
 * @param fi A federateInfo object.
 * @param args Command line arguments specified in a string.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoLoadFromString(HelicsFederateInfo fi, const char* args, HelicsError* err);

/**
 * Delete the memory associated with a federate info object.
 */
HELICS_EXPORT void helicsFederateInfoFree(HelicsFederateInfo fi);

/**
 * Check if a federate_object is valid.
 *
 * @return HELICS_TRUE if the federate is a valid active federate, HELICS_FALSE otherwise
 */
HELICS_EXPORT HelicsBool helicsFederateIsValid(HelicsFederate fed);

/**
 * Set the name of the core to link to for a federate.
 *
 * @param fi The federate info object to alter.
 * @param corename The identifier for a core to link to.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoSetCoreName(HelicsFederateInfo fi, const char* corename, HelicsError* err);

/**
 * Set the initialization string for the core usually in the form of command line arguments.
 *
 * @param fi The federate info object to alter.
 * @param coreInit A string containing command line arguments to be passed to the core.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoSetCoreInitString(HelicsFederateInfo fi, const char* coreInit, HelicsError* err);

/**
 * Set the initialization string that a core will pass to a generated broker usually in the form of command line arguments.
 *
 * @param fi The federate info object to alter.
 * @param brokerInit A string with command line arguments for a generated broker.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoSetBrokerInitString(HelicsFederateInfo fi, const char* brokerInit, HelicsError* err);

/**
 * Set the core type by integer code.
 *
 * @details Valid values available by definitions in api-data.h.
 * @param fi The federate info object to alter.
 * @param coretype An numerical code for a core type see /ref helics_CoreType.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoSetCoreType(HelicsFederateInfo fi, int coretype, HelicsError* err);

/**
 * Set the core type from a string.
 *
 * @param fi The federate info object to alter.
 * @param coretype A string naming a core type.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoSetCoreTypeFromString(HelicsFederateInfo fi, const char* coretype, HelicsError* err);

/**
 * Set the name or connection information for a broker.
 *
 * @details This is only used if the core is automatically created, the broker information will be transferred to the core for connection.
 * @param fi The federate info object to alter.
 * @param broker A string which defines the connection information for a broker either a name or an address.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoSetBroker(HelicsFederateInfo fi, const char* broker, HelicsError* err);

/**
 * Set the key for a broker connection.
 *
 * @details This is only used if the core is automatically created, the broker information will be transferred to the core for connection.
 * @param fi The federate info object to alter.
 * @param brokerkey A string containing a key for the broker to connect.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoSetBrokerKey(HelicsFederateInfo fi, const char* brokerkey, HelicsError* err);

/**
 * Set the port to use for the broker.
 *
 * @details This is only used if the core is automatically created, the broker information will be transferred to the core for connection.
 * This will only be useful for network broker connections.
 * @param fi The federate info object to alter.
 * @param brokerPort The integer port number to use for connection with a broker.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoSetBrokerPort(HelicsFederateInfo fi, int brokerPort, HelicsError* err);

/**
 * Set the local port to use.
 *
 * @details This is only used if the core is automatically created, the port information will be transferred to the core for connection.
 * @param fi The federate info object to alter.
 * @param localPort A string with the port information to use as the local server port can be a number or "auto" or "os_local".
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.

 */
HELICS_EXPORT void helicsFederateInfoSetLocalPort(HelicsFederateInfo fi, const char* localPort, HelicsError* err);

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
 * @param fi The federate info object to alter.
 * @param flag A numerical index for a flag.
 * @param value The desired value of the flag HELICS_TRUE or HELICS_FALSE.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateInfoSetFlagOption(HelicsFederateInfo fi, int flag, HelicsBool value, HelicsError* err);

/**
 * Set the separator character in the info structure.
 *
 * @details The separator character is the separation character for local publications/endpoints in creating their global name.
 * For example if the separator character is '/'  then a local endpoint would have a globally reachable name of fedName/localName.
 * @param fi The federate info object to alter.
 * @param separator The character to use as a separator.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateInfoSetSeparator(HelicsFederateInfo fi, char separator, HelicsError* err);

/**
 * Set the output delay for a federate.
 *
 * @param fi The federate info object to alter.
 * @param timeProperty An integer representation of the time based property to set see /ref helics_properties.
 * @param propertyValue The value of the property to set the timeProperty to.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateInfoSetTimeProperty(HelicsFederateInfo fi, int timeProperty, HelicsTime propertyValue, HelicsError* err);

// TODO(Dheepak): what are known properties. The docstring should reference all properties that can be passed here.
/**
 * Set an integer property for a federate.
 *
 * @details Set known properties.
 *
 * @param fi The federateInfo object to alter.
 * @param intProperty An int identifying the property.
 * @param propertyValue The value to set the property to.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateInfoSetIntegerProperty(HelicsFederateInfo fi, int intProperty, int propertyValue, HelicsError* err);

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
 * @details The function helicsFederateEnterInitializationModeFinalize must be called to finish the operation.
 *
 * @param fed The federate to operate on.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateEnterInitializingModeAsync(HelicsFederate fed, HelicsError* err);

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
 * Finalize the entry to initialize mode that was initiated with /ref heliceEnterInitializingModeAsync.
 *
 * @param fed The federate desiring to complete the initialization step.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsFederateEnterInitializingModeComplete(HelicsFederate fed, HelicsError* err);

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
 *
 * @return State the resulting state if void return HELICS_OK.
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
 * Send a command to another helics object though a core.
 *
 * @param core The core to send the command through.
 * @param target The name of the object to send the command to.
 * @param command The command to send.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsCoreSendCommand(HelicsCore core, const char* target, const char* command, HelicsError* err);

/**
 * Send a command to another helics object through a broker.
 *
 * @param broker The broker to send the command through.
 * @param target The name of the object to send the command to.
 * @param command The command to send.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsBrokerSendCommand(HelicsBroker broker, const char* target, const char* command, HelicsError* err);

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
 *
 *         The return will be nullptr if fed or query is an invalid object, the return string will be "#invalid" if the query itself was
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
 *
 *         The return will be nullptr if core or query is an invalid object, the return string will be "#invalid" if the query itself was
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
 *
 *         The return will be nullptr if broker or query is an invalid object, the return string will be "#invalid" if the query itself was
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
 *
 *         The return will be nullptr if query is an invalid object
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
 *         and false if an asynchronous query has not completed or is invalid
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

/**
 * sub/pub registration
 */

/**
 * Create a subscription.
 *
 * @details The subscription becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions and publications.
 *
 * @param fed The federate object in which to create a subscription, must have been created with /ref helicsCreateValueFederate or
 * /ref helicsCreateCombinationFederate.
 * @param key The identifier matching a publication to get a subscription for.
 * @param units A string listing the units of the subscription (may be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return An object containing the subscription.
 */
HELICS_EXPORT HelicsInput helicsFederateRegisterSubscription(HelicsFederate fed, const char* key, const char* units, HelicsError* err);

/**
 * Register a publication with a known type.
 *
 * @details The publication becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions and publications.
 *
 * @param fed The federate object in which to create a publication.
 * @param key The identifier for the publication the global publication key will be prepended with the federate name.
 * @param type A code identifying the type of the input see /ref HelicsDataTypes for available options.
 * @param units A string listing the units of the subscription (may be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return An object containing the publication.
 */
HELICS_EXPORT HelicsPublication
    helicsFederateRegisterPublication(HelicsFederate fed, const char* key, HelicsDataTypes type, const char* units, HelicsError* err);

/**
 * Register a publication with a defined type.
 *
 * @details The publication becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions and publications.
 *
 * @param fed The federate object in which to create a publication.
 * @param key The identifier for the publication.
 * @param type A string labeling the type of the publication.
 * @param units A string listing the units of the subscription (may be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return An object containing the publication.
 */
HELICS_EXPORT HelicsPublication
    helicsFederateRegisterTypePublication(HelicsFederate fed, const char* key, const char* type, const char* units, HelicsError* err);

/**
 * Register a global named publication with an arbitrary type.
 *
 * @details The publication becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions and publications.
 *
 * @param fed The federate object in which to create a publication.
 * @param key The identifier for the publication.
 * @param type A code identifying the type of the input see /ref HelicsDataTypes for available options.
 * @param units A string listing the units of the subscription (may be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return An object containing the publication.
 */
HELICS_EXPORT HelicsPublication
    helicsFederateRegisterGlobalPublication(HelicsFederate fed, const char* key, HelicsDataTypes type, const char* units, HelicsError* err);

/**
 * Register a global publication with a defined type.
 *
 * @details The publication becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions and publications.
 *
 * @param fed The federate object in which to create a publication.
 * @param key The identifier for the publication.
 * @param type A string describing the expected type of the publication.
 * @param units A string listing the units of the subscription (may be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return An object containing the publication.
 */
HELICS_EXPORT HelicsPublication
    helicsFederateRegisterGlobalTypePublication(HelicsFederate fed, const char* key, const char* type, const char* units, HelicsError* err);

/**
 * Register a named input.
 *
 * @details The input becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions, inputs, and publications.
 *
 * @param fed The federate object in which to create an input.
 * @param key The identifier for the publication the global input key will be prepended with the federate name.
 * @param type A code identifying the type of the input see /ref HelicsDataTypes for available options.
 * @param units A string listing the units of the input (may be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return An object containing the input.
 */
HELICS_EXPORT HelicsInput
    helicsFederateRegisterInput(HelicsFederate fed, const char* key, HelicsDataTypes type, const char* units, HelicsError* err);

/**
 * Register an input with a defined type.
 *
 * @details The input becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions, inputs, and publications.
 *
 * @param fed The federate object in which to create an input.
 * @param key The identifier for the input.
 * @param type A string describing the expected type of the input.
 * @param units A string listing the units of the input maybe NULL.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return An object containing the publication.
 */
HELICS_EXPORT HelicsInput
    helicsFederateRegisterTypeInput(HelicsFederate fed, const char* key, const char* type, const char* units, HelicsError* err);

/**
 * Register a global named input.
 *
 * @details The publication becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions and publications.
 *
 * @param fed The federate object in which to create a publication.
 * @param key The identifier for the publication.
 * @param type A code identifying the type of the input see /ref HelicsDataTypes for available options.
 * @param units A string listing the units of the subscription maybe NULL.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return An object containing the publication.
 */
HELICS_EXPORT HelicsPublication
    helicsFederateRegisterGlobalInput(HelicsFederate fed, const char* key, HelicsDataTypes type, const char* units, HelicsError* err);

/**
 * Register a global publication with an arbitrary type.
 *
 * @details The publication becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions and publications.
 *
 * @param fed The federate object in which to create a publication.
 * @param key The identifier for the publication.
 * @param type A string defining the type of the input.
 * @param units A string listing the units of the subscription maybe NULL.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return An object containing the publication.
 */
HELICS_EXPORT HelicsPublication
    helicsFederateRegisterGlobalTypeInput(HelicsFederate fed, const char* key, const char* type, const char* units, HelicsError* err);

/**
 * Get a publication object from a key.
 *
 * @param fed The value federate object to use to get the publication.
 * @param key The name of the publication.
 *
 * @param[in,out] err The error object to complete if there is an error.

 *
 * @return A HelicsPublication object, the object will not be valid and err will contain an error code if no publication with the
 * specified key exists.
 */
HELICS_EXPORT HelicsPublication helicsFederateGetPublication(HelicsFederate fed, const char* key, HelicsError* err);

/**
 * Get a publication by its index, typically already created via registerInterfaces file or something of that nature.
 *
 * @param fed The federate object in which to create a publication.
 * @param index The index of the publication to get.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsPublication.
 */
HELICS_EXPORT HelicsPublication helicsFederateGetPublicationByIndex(HelicsFederate fed, int index, HelicsError* err);

/**
 * Get an input object from a key.
 *
 * @param fed The value federate object to use to get the publication.
 * @param key The name of the input.
 *
 * @param[in,out] err The error object to complete if there is an error.

 *
 * @return A HelicsInput object, the object will not be valid and err will contain an error code if no input with the specified
 * key exists.
 */
HELICS_EXPORT HelicsInput helicsFederateGetInput(HelicsFederate fed, const char* key, HelicsError* err);

/**
 * Get an input by its index, typically already created via registerInterfaces file or something of that nature.
 *
 * @param fed The federate object in which to create a publication.
 * @param index The index of the publication to get.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsInput, which will be NULL if an invalid index.
 */
HELICS_EXPORT HelicsInput helicsFederateGetInputByIndex(HelicsFederate fed, int index, HelicsError* err);

/**
 * Get an input object from a subscription target.
 *
 * @param fed The value federate object to use to get the publication.
 * @param key The name of the publication that a subscription is targeting.
 *
 * @param[in,out] err The error object to complete if there is an error.

 *
 * @return A HelicsInput object, the object will not be valid and err will contain an error code if no input with the specified
 * key exists.
 */
HELICS_EXPORT HelicsInput helicsFederateGetSubscription(HelicsFederate fed, const char* key, HelicsError* err);

/**
 * Clear all the update flags from a federates inputs.
 *
 * @param fed The value federate object for which to clear update flags.
 */
HELICS_EXPORT void helicsFederateClearUpdates(HelicsFederate fed);

/**
 * Register the publications via JSON publication string.
 *
 * @param fed The value federate object to use to register the publications.
 * @param json The JSON publication string.
 *
 * @param[in,out] err The error object to complete if there is an error.

 *
 * @details This would be the same JSON that would be used to publish data.
 */
HELICS_EXPORT void helicsFederateRegisterFromPublicationJSON(HelicsFederate fed, const char* json, HelicsError* err);

/**
 * Publish data contained in a JSON file or string.
 *
 * @param fed The value federate object through which to publish the data.
 * @param json The publication file name or literal JSON data string.
 *
 * @param[in,out] err The error object to complete if there is an error.

 */
HELICS_EXPORT void helicsFederatePublishJSON(HelicsFederate fed, const char* json, HelicsError* err);

/**
 * \defgroup publications Publication functions
 * @details Functions for publishing data of various kinds.
 * The data will get translated to the type specified when the publication was constructed automatically
 * regardless of the function used to publish the data.
 * @{
 */

/**
 * Check if a publication is valid.
 *
 * @param pub The publication to check.
 *
 * @return HELICS_TRUE if the publication is a valid publication.
 */
HELICS_EXPORT HelicsBool helicsPublicationIsValid(HelicsPublication pub);

/**
 * Publish raw data from a char * and length.
 *
 * @param pub The publication to publish for.
 * @param data A pointer to the raw data.
 * @param inputDataLength The size in bytes of the data to publish.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsPublicationPublishBytes(HelicsPublication pub, const void* data, int inputDataLength, HelicsError* err);

/**
 * Publish a string.
 *
 * @param pub The publication to publish for.
 * @param str The string to publish.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsPublicationPublishString(HelicsPublication pub, const char* str, HelicsError* err);

/**
 * Publish an integer value.
 *
 * @param pub The publication to publish for.
 * @param val The numerical value to publish.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsPublicationPublishInteger(HelicsPublication pub, int64_t val, HelicsError* err);

/**
 * Publish a Boolean Value.
 *
 * @param pub The publication to publish for.
 * @param val The boolean value to publish.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsPublicationPublishBoolean(HelicsPublication pub, HelicsBool val, HelicsError* err);

/**
 * Publish a double floating point value.
 *
 * @param pub The publication to publish for.
 * @param val The numerical value to publish.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsPublicationPublishDouble(HelicsPublication pub, double val, HelicsError* err);

/**
 * Publish a time value.
 *
 * @param pub The publication to publish for.
 * @param val The numerical value to publish.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsPublicationPublishTime(HelicsPublication pub, HelicsTime val, HelicsError* err);

/**
 * Publish a single character.
 *
 * @param pub The publication to publish for.
 * @param val The numerical value to publish.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsPublicationPublishChar(HelicsPublication pub, char val, HelicsError* err);

/**
 * Publish a complex value (or pair of values).
 *
 * @param pub The publication to publish for.
 * @param real The real part of a complex number to publish.
 * @param imag The imaginary part of a complex number to publish.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsPublicationPublishComplex(HelicsPublication pub, double real, double imag, HelicsError* err);

/**
 * Publish a vector of doubles.
 *
 * @param pub The publication to publish for.
 * @param vectorInput A pointer to an array of double data.
 *
 * @param vectorLength The number of points to publish.
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsPublicationPublishVector(HelicsPublication pub, const double* vectorInput, int vectorLength, HelicsError* err);

/**
 * Publish a vector of complex doubles.
 *
 * @param pub The publication to publish for.
 * @param vectorInput A pointer to an array of complex double data (alternating real and imaginary values).
 *
 * @param vectorLength The number of values to publish; vectorInput must contain 2xvectorLength values.
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void
    helicsPublicationPublishComplexVector(HelicsPublication pub, const double* vectorInput, int vectorLength, HelicsError* err);

/**
 * Publish a named point.
 *
 * @param pub The publication to publish for.
 * @param str A string for the name to publish.
 * @param val A double for the value to publish.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsPublicationPublishNamedPoint(HelicsPublication pub, const char* str, double val, HelicsError* err);

/**
 * Add a named input to the list of targets a publication publishes to.
 *
 * @param pub The publication to add the target for.
 * @param target The name of an input that the data should be sent to.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsPublicationAddTarget(HelicsPublication pub, const char* target, HelicsError* err);

/**
 * Check if an input is valid.
 *
 * @param ipt The input to check.
 *
 * @return HELICS_TRUE if the Input object represents a valid input.
 */
HELICS_EXPORT HelicsBool helicsInputIsValid(HelicsInput ipt);

/**
 * Add a publication to the list of data that an input subscribes to.
 *
 * @param ipt The named input to modify.
 * @param target The name of a publication that an input should subscribe to.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsInputAddTarget(HelicsInput ipt, const char* target, HelicsError* err);

/**@}*/

/**
 * \defgroup getValue GetValue functions
 * @details Data can be returned in a number of formats,  for instance if data is published as a double it can be returned as a string and
 * vice versa,  not all translations make that much sense but they do work.
 * @{
 */

/**
 * Get the size of the raw value for subscription.
 *
 * @return The size of the raw data/string in bytes.
 */
HELICS_EXPORT int helicsInputGetByteCount(HelicsInput ipt);

/**
 * Get the raw data for the latest value of a subscription.
 *
 * @param ipt The input to get the data for.
 *
 * @param[out] data The memory location of the data
 * @param maxDataLength The maximum size of information that data can hold.
 * @param[out] actualSize The actual length of data copied to data.
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsInputGetBytes(HelicsInput ipt, void* data, int maxDataLength, int* actualSize, HelicsError* err);

/**
 * Get the size of a value for subscription assuming return as a string.
 *
 * @return The size of the string.
 */
HELICS_EXPORT int helicsInputGetStringSize(HelicsInput ipt);

/**
 * Get a string value from a subscription.
 *
 * @param ipt The input to get the data for.
 *
 * @param[out] outputString Storage for copying a null terminated string.
 * @param maxStringLength The maximum size of information that str can hold.
 * @param[out] actualLength The actual length of the string.
 * @param[in,out] err Error term for capturing errors.
 */
HELICS_EXPORT void helicsInputGetString(HelicsInput ipt, char* outputString, int maxStringLength, int* actualLength, HelicsError* err);

/**
 * Get an integer value from a subscription.
 *
 * @param ipt The input to get the data for.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 *
 * @return An int64_t value with the current value of the input.
 */
HELICS_EXPORT int64_t helicsInputGetInteger(HelicsInput ipt, HelicsError* err);

/**
 * Get a boolean value from a subscription.
 *
 * @param ipt The input to get the data for.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 *
 * @return A boolean value of current input value.
 */
HELICS_EXPORT HelicsBool helicsInputGetBoolean(HelicsInput ipt, HelicsError* err);

/**
 * Get a double value from a subscription.
 *
 * @param ipt The input to get the data for.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 *
 * @return The double value of the input.
 */
HELICS_EXPORT double helicsInputGetDouble(HelicsInput ipt, HelicsError* err);

/**
 * Get a time value from a subscription.
 *
 * @param ipt The input to get the data for.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 *
 * @return The resulting time value.
 */
HELICS_EXPORT HelicsTime helicsInputGetTime(HelicsInput ipt, HelicsError* err);

/**
 * Get a single character value from an input.
 *
 * @param ipt The input to get the data for.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 *
 * @return The resulting character value.
 *         NAK (negative acknowledgment) symbol returned on error
 */
HELICS_EXPORT char helicsInputGetChar(HelicsInput ipt, HelicsError* err);

/**
 * Get a complex object from an input object.
 *
 * @param ipt The input to get the data for.
 *
 * @param[in,out] err A helics error object, if the object is not empty the function is bypassed otherwise it is filled in if there is an
 * error.
 *
 * @return A HelicsComplex structure with the value.
 */
HELICS_EXPORT HelicsComplex helicsInputGetComplexObject(HelicsInput ipt, HelicsError* err);

/**
 * Get a pair of double forming a complex number from a subscriptions.
 *
 * @param ipt The input to get the data for.
 *
 * @param[out] real Memory location to place the real part of a value.
 * @param[out] imag Memory location to place the imaginary part of a value.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * On error the values will not be altered.
 */
HELICS_EXPORT void helicsInputGetComplex(HelicsInput ipt, double* real, double* imag, HelicsError* err);

/**
 * Get the size of a value for subscription assuming return as an array of doubles.
 *
 * @return The number of doubles in a returned vector.
 */
HELICS_EXPORT int helicsInputGetVectorSize(HelicsInput ipt);

/**
 * Get a vector from a subscription.
 *
 * @param ipt The input to get the result for.
 *
 * @param[out] data The location to store the data.
 * @param maxLength The maximum size of the vector.
 * @param[out] actualSize Location to place the actual length of the resulting vector.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsInputGetVector(HelicsInput ipt, double data[], int maxLength, int* actualSize, HelicsError* err);

/**
 * Get a complex vector from an input.
 *
 * @param ipt The input to get the result for.
 *
 * @param[out] data The location to store the data. The data will be stored in alternating real and imaginary values.
 * @param maxLength The maximum number of values data can hold.
 * @param[out] actualSize Location to place the actual length of the resulting complex vector (will be 1/2 the number of values assigned).
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsInputGetComplexVector(HelicsInput ipt, double data[], int maxLength, int* actualSize, HelicsError* err);

/**
 * Get a named point from a subscription.
 *
 * @param ipt The input to get the result for.
 *
 * @param[out] outputString Storage for copying a null terminated string.
 * @param maxStringLength The maximum size of information that str can hold.
 * @param[out] actualLength The actual length of the string
 * @param[out] val The double value for the named point.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void
    helicsInputGetNamedPoint(HelicsInput ipt, char* outputString, int maxStringLength, int* actualLength, double* val, HelicsError* err);

/**@}*/

/**
 * \defgroup default_values Default Value functions
 * @details These functions set the default value for a subscription. That is the value returned if nothing was published from elsewhere.
 * @{
 */

/**
 * Set the default as a raw data array.
 *
 * @param ipt The input to set the default for.
 * @param data A pointer to the raw data to use for the default.
 *
 * @param inputDataLength The size of the raw data.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsInputSetDefaultBytes(HelicsInput ipt, const void* data, int inputDataLength, HelicsError* err);

/**
 * Set the default as a string.
 *
 * @param ipt The input to set the default for.
 * @param str A pointer to the default string.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsInputSetDefaultString(HelicsInput ipt, const char* str, HelicsError* err);

/**
 * Set the default as an integer.
 *
 * @param ipt The input to set the default for.
 * @param val The default integer.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsInputSetDefaultInteger(HelicsInput ipt, int64_t val, HelicsError* err);

/**
 * Set the default as a boolean.
 *
 * @param ipt The input to set the default for.
 * @param val The default boolean value.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsInputSetDefaultBoolean(HelicsInput ipt, HelicsBool val, HelicsError* err);

/**
 * Set the default as a time.
 *
 * @param ipt The input to set the default for.
 * @param val The default time value.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsInputSetDefaultTime(HelicsInput ipt, HelicsTime val, HelicsError* err);

/**
 * Set the default as a char.
 *
 * @param ipt The input to set the default for.
 * @param val The default char value.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsInputSetDefaultChar(HelicsInput ipt, char val, HelicsError* err);

/**
 * Set the default as a double.
 *
 * @param ipt The input to set the default for.
 * @param val The default double value.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsInputSetDefaultDouble(HelicsInput ipt, double val, HelicsError* err);

/**
 * Set the default as a complex number.
 *
 * @param ipt The input to set the default for.
 * @param real The default real value.
 * @param imag The default imaginary value.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsInputSetDefaultComplex(HelicsInput ipt, double real, double imag, HelicsError* err);

/**
 * Set the default as a vector of doubles.
 *
 * @param ipt The input to set the default for.
 * @param vectorInput A pointer to an array of double data.
 * @param vectorLength The number of doubles in the vector.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsInputSetDefaultVector(HelicsInput ipt, const double* vectorInput, int vectorLength, HelicsError* err);

/**
 * Set the default as a vector of complex doubles. The format is alternating real, imag doubles.
 *
 * @param ipt The input to set the default for.
 * @param vectorInput A pointer to an array of double data alternating between real and imaginary.
 * @param vectorLength the number of complex values in the publication (vectorInput must contain 2xvectorLength elements).
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsInputSetDefaultComplexVector(HelicsInput ipt, const double* vectorInput, int vectorLength, HelicsError* err);

/**
 * Set the default as a NamedPoint.
 *
 * @param ipt The input to set the default for.
 * @param str A pointer to a string representing the name.
 * @param val A double value for the value of the named point.
 *
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 */
HELICS_EXPORT void helicsInputSetDefaultNamedPoint(HelicsInput ipt, const char* str, double val, HelicsError* err);

/**@}*/

/**
 * \defgroup Information retrieval
 * @{
 */

/**
 * Get the type of an input.
 *
 * @param ipt The input to query.
 *
 * @return A void enumeration, HELICS_OK if everything worked.
 */
HELICS_EXPORT const char* helicsInputGetType(HelicsInput ipt);

/**
 * Get the type the publisher to an input is sending.
 *
 * @param ipt The input to query.
 *
 * @return A const char * with the type name.
 */
HELICS_EXPORT const char* helicsInputGetPublicationType(HelicsInput ipt);

/**
 * Get the type the publisher to an input is sending.
 *
 * @param ipt The input to query.
 *
 * @return An int containing the enumeration value of the publication type.
 */
HELICS_EXPORT int helicsInputGetPublicationDataType(HelicsInput ipt);

/**
 * Get the type of a publication.
 *
 * @param pub The publication to query.
 *
 * @return A void enumeration, HELICS_OK if everything worked.
 */
HELICS_EXPORT const char* helicsPublicationGetType(HelicsPublication pub);

/**
 * Get the key of an input.
 *
 * @param ipt The input to query.
 *
 * @return A const char with the input name.
 */
HELICS_EXPORT const char* helicsInputGetName(HelicsInput ipt);

/**
 * Get the target of a subscription.
 *
 * @return A const char with the subscription target.
 */
HELICS_EXPORT const char* helicsSubscriptionGetTarget(HelicsInput ipt);

/**
 * Get the name of a publication.
 *
 * @details This will be the global key used to identify the publication to the federation.
 *
 * @param pub The publication to query.
 *
 * @return A const char with the publication name.
 */
HELICS_EXPORT const char* helicsPublicationGetName(HelicsPublication pub);

/**
 * Get the units of an input.
 *
 * @param ipt The input to query.
 *
 * @return A void enumeration, HELICS_OK if everything worked.
 */
HELICS_EXPORT const char* helicsInputGetUnits(HelicsInput ipt);

/**
 * Get the units of the publication that an input is linked to.
 *
 * @param ipt The input to query.
 *
 * @return A void enumeration, HELICS_OK if everything worked.
 */
HELICS_EXPORT const char* helicsInputGetInjectionUnits(HelicsInput ipt);

/**
 * Get the units of an input.
 *
 * @details The same as helicsInputGetUnits.
 *
 * @param ipt The input to query.
 *
 * @return A void enumeration, HELICS_OK if everything worked.
 */
HELICS_EXPORT const char* helicsInputGetExtractionUnits(HelicsInput ipt);

/**
 * Get the units of a publication.
 *
 * @param pub The publication to query.
 *
 * @return A void enumeration, HELICS_OK if everything worked.
 */
HELICS_EXPORT const char* helicsPublicationGetUnits(HelicsPublication pub);

/**
 * Get the data in the info field of an input.
 *
 * @param inp The input to query.
 *
 * @return A string with the info field string.
 */
HELICS_EXPORT const char* helicsInputGetInfo(HelicsInput inp);

/**
 * Set the data in the info field for an input.
 *
 * @param inp The input to query.
 * @param info The string to set.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsInputSetInfo(HelicsInput inp, const char* info, HelicsError* err);

/**
 * Get the data in a specified tag of an input.
 *
 * @param inp The input object to query.
 * @param tagname The name of the tag to get the value for.
 * @return A string with the tag data.
 */
HELICS_EXPORT const char* helicsInputGetTag(HelicsInput inp, const char* tagname);

/**
 * Set the data in a specific tag for an input.
 *
 * @param inp The input object to query.
 * @param tagname The string to set.
 * @param tagvalue The string value to associate with a tag.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsInputSetTag(HelicsInput inp, const char* tagname, const char* tagvalue, HelicsError* err);

/**
 * Get the data in the info field of an publication.
 *
 * @param pub The publication to query.
 *
 * @return A string with the info field string.
 */
HELICS_EXPORT const char* helicsPublicationGetInfo(HelicsPublication pub);

/**
 * Set the data in the info field for a publication.
 *
 * @param pub The publication to set the info field for.
 * @param info The string to set.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsPublicationSetInfo(HelicsPublication pub, const char* info, HelicsError* err);

/**
 * Get the data in a specified tag of a publication.
 *
 * @param pub The publication object to query.
 * @param tagname The name of the tag to query.
 * @return A string with the tag data.
 */
HELICS_EXPORT const char* helicsPublicationGetTag(HelicsPublication pub, const char* tagname);

/**
 * Set the data in a specific tag for a publication.
 *
 * @param pub The publication object to set a tag for.
 * @param tagname The name of the tag to set.
 * @param tagvalue The string value to associate with a tag.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsPublicationSetTag(HelicsPublication pub, const char* tagname, const char* tagvalue, HelicsError* err);

/**
 * Get the current value of an input handle option
 *
 * @param inp The input to query.
 * @param option Integer representation of the option in question see /ref helics_handle_options.
 *
 * @return An integer value with the current value of the given option.
 */
HELICS_EXPORT int helicsInputGetOption(HelicsInput inp, int option);

/**
 * Set an option on an input
 *
 * @param inp The input to query.
 * @param option The option to set for the input /ref helics_handle_options.
 * @param value The value to set the option to.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsInputSetOption(HelicsInput inp, int option, int value, HelicsError* err);

/**
 * Get the value of an option for a publication
 *
 * @param pub The publication to query.
 * @param option The value to query see /ref helics_handle_options.
 *
 * @return A string with the info field string.
 */
HELICS_EXPORT int helicsPublicationGetOption(HelicsPublication pub, int option);

/**
 * Set the value of an option for a publication
 *
 * @param pub The publication to query.
 * @param option Integer code for the option to set /ref helics_handle_options.
 * @param val The value to set the option to.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsPublicationSetOption(HelicsPublication pub, int option, int val, HelicsError* err);

/**
 * Set the minimum change detection tolerance.
 *
 * @param pub The publication to modify.
 * @param tolerance The tolerance level for publication, values changing less than this value will not be published.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsPublicationSetMinimumChange(HelicsPublication pub, double tolerance, HelicsError* err);

/**
 * Set the minimum change detection tolerance.
 *
 * @param inp The input to modify.
 * @param tolerance The tolerance level for registering an update, values changing less than this value will not show as being updated.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsInputSetMinimumChange(HelicsInput inp, double tolerance, HelicsError* err);

/**@}*/

/**
 * Check if a particular subscription was updated.
 *
 * @return HELICS_TRUE if it has been updated since the last value retrieval.
 */
HELICS_EXPORT HelicsBool helicsInputIsUpdated(HelicsInput ipt);

/**
 * Get the last time a subscription was updated.
 */
HELICS_EXPORT HelicsTime helicsInputLastUpdateTime(HelicsInput ipt);

/**
 * Clear the updated flag from an input.
 */
HELICS_EXPORT void helicsInputClearUpdate(HelicsInput ipt);

/**
 * Get the number of publications in a federate.
 *
 * @return (-1) if fed was not a valid federate otherwise returns the number of publications.
 */
HELICS_EXPORT int helicsFederateGetPublicationCount(HelicsFederate fed);

/**
 * Get the number of subscriptions in a federate.
 *
 * @return (-1) if fed was not a valid federate otherwise returns the number of subscriptions.
 */
HELICS_EXPORT int helicsFederateGetInputCount(HelicsFederate fed);

/* MessageFederate Calls*/

/**
 * Create an endpoint.
 *
 * @details The endpoint becomes part of the federate and is destroyed when the federate is freed
 *          so there are no separate free functions for endpoints.
 *
 * @param fed The federate object in which to create an endpoint must have been created
 *           with helicsCreateMessageFederate or helicsCreateCombinationFederate.
 * @param name The identifier for the endpoint. This will be prepended with the federate name for the global identifier.
 * @param type A string describing the expected type of the publication (may be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return An object containing the endpoint.
 *
 *         nullptr on failure.
 */
HELICS_EXPORT HelicsEndpoint helicsFederateRegisterEndpoint(HelicsFederate fed, const char* name, const char* type, HelicsError* err);

/**
 * Create an endpoint.
 *
 * @details The endpoint becomes part of the federate and is destroyed when the federate is freed
 *          so there are no separate free functions for endpoints.
 *
 * @param fed The federate object in which to create an endpoint must have been created
              with helicsCreateMessageFederate or helicsCreateCombinationFederate.
 * @param name The identifier for the endpoint, the given name is the global identifier.
 * @param type A string describing the expected type of the publication (may be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 * @return An object containing the endpoint.
 *
 *         nullptr on failure.

 */
HELICS_EXPORT HelicsEndpoint helicsFederateRegisterGlobalEndpoint(HelicsFederate fed, const char* name, const char* type, HelicsError* err);

/**
 * Create a targeted endpoint.  Targeted endpoints have specific destinations predefined and do not allow sending messages to other
 * endpoints
 *
 * @details The endpoint becomes part of the federate and is destroyed when the federate is freed
 *          so there are no separate free functions for endpoints.
 *
 * @param fed The federate object in which to create an endpoint must have been created
 *           with helicsCreateMessageFederate or helicsCreateCombinationFederate.
 * @param name The identifier for the endpoint. This will be prepended with the federate name for the global identifier.
 * @param type A string describing the expected type of the publication (may be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return An object containing the endpoint.
 *
 *         nullptr on failure.

 */
HELICS_EXPORT HelicsEndpoint helicsFederateRegisterTargetedEndpoint(HelicsFederate fed,
                                                                    const char* name,
                                                                    const char* type,
                                                                    HelicsError* err);

/**
 * Create a global targeted endpoint, Targeted endpoints have specific destinations predefined and do not allow sending messages to other
 endpoints
 *
 * @details The endpoint becomes part of the federate and is destroyed when the federate is freed
 *          so there are no separate free functions for endpoints.
 *
 * @param fed The federate object in which to create an endpoint must have been created
              with helicsCreateMessageFederate or helicsCreateCombinationFederate.
 * @param name The identifier for the endpoint, the given name is the global identifier.
 * @param type A string describing the expected type of the publication (may be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 * @return An object containing the endpoint.
 *
 *         nullptr on failure.

 */
HELICS_EXPORT HelicsEndpoint helicsFederateRegisterGlobalTargetedEndpoint(HelicsFederate fed,
                                                                          const char* name,
                                                                          const char* type,
                                                                          HelicsError* err);

/**
 * Get an endpoint object from a name.
 *
 * @param fed The message federate object to use to get the endpoint.
 * @param name The name of the endpoint.
 *
 * @param[in,out] err The error object to complete if there is an error.

 *
 * @return A HelicsEndpoint object.
 *
 *         The object will not be valid and err will contain an error code if no endpoint with the specified name exists.

 */
HELICS_EXPORT HelicsEndpoint helicsFederateGetEndpoint(HelicsFederate fed, const char* name, HelicsError* err);

/**
 * Get an endpoint by its index, typically already created via registerInterfaces file or something of that nature.
 *
 * @param fed The federate object in which to create a publication.
 * @param index The index of the publication to get.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsEndpoint.
 *
 *         It will be NULL if given an invalid index.

 */
HELICS_EXPORT HelicsEndpoint helicsFederateGetEndpointByIndex(HelicsFederate fed, int index, HelicsError* err);

/**
 * Check if an endpoint is valid.
 *
 * @param endpoint The endpoint object to check.
 *
 * @return HELICS_TRUE if the Endpoint object represents a valid endpoint.
 */
HELICS_EXPORT HelicsBool helicsEndpointIsValid(HelicsEndpoint endpoint);

/**
 * Set the default destination for an endpoint if no other endpoint is given.
 *
 * @param endpoint The endpoint to set the destination for.
 * @param dst A string naming the desired default endpoint.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsEndpointSetDefaultDestination(HelicsEndpoint endpoint, const char* dst, HelicsError* err);

/**
 * Get the default destination for an endpoint.
 *
 * @param endpoint The endpoint to set the destination for.
 *
 * @return A string with the default destination.
 */
HELICS_EXPORT const char* helicsEndpointGetDefaultDestination(HelicsEndpoint endpoint);

/**
 * Send a message to the targeted destination.
 *
 * @param endpoint The endpoint to send the data from.
 * @param data The data to send.
 *
 * @param inputDataLength The length of the data to send.
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsEndpointSendBytes(HelicsEndpoint endpoint, const void* data, int inputDataLength, HelicsError* err);

/**
 * Send a message to the specified destination.
 *
 * @param endpoint The endpoint to send the data from.

 * @param data The data to send.
 *
 * @param inputDataLength The length of the data to send.
 * @param dst The target destination.
 *
 *             nullptr to use the default destination.
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void
    helicsEndpointSendBytesTo(HelicsEndpoint endpoint, const void* data, int inputDataLength, const char* dst, HelicsError* err);

/**
 * Send a message to the specified destination at a specific time.
 *
 * @param endpoint The endpoint to send the data from.
 * @param data The data to send.
 *
 * @param inputDataLength The length of the data to send.

 * @param dst The target destination.
 *
 *             nullptr to use the default destination.
 * @param time The time the message should be sent.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */

HELICS_EXPORT void helicsEndpointSendBytesToAt(HelicsEndpoint endpoint,
                                               const void* data,
                                               int inputDataLength,
                                               const char* dst,
                                               HelicsTime time,
                                               HelicsError* err);

/**
 * Send a message at a specific time to the targeted destinations
 *
 * @param endpoint The endpoint to send the data from.
 *
 * @param data The data to send.
 *
 * @param inputDataLength The length of the data to send.

  @param time The time the message should be sent.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */

HELICS_EXPORT void
    helicsEndpointSendBytesAt(HelicsEndpoint endpoint, const void* data, int inputDataLength, HelicsTime time, HelicsError* err);

/**
 * Send a message object from a specific endpoint.
 *
 * @param endpoint The endpoint to send the data from.
 * @param message The actual message to send which will be copied.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsEndpointSendMessage(HelicsEndpoint endpoint, HelicsMessage message, HelicsError* err);

/**
 * Send a message object from a specific endpoint, the message will not be copied and the message object will no longer be valid
 * after the call.
 *
 * @param endpoint The endpoint to send the data from.
 * @param message The actual message to send which will be copied.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsEndpointSendMessageZeroCopy(HelicsEndpoint endpoint, HelicsMessage message, HelicsError* err);

/**
 * Subscribe an endpoint to a publication.
 *
 * @param endpoint The endpoint to use.
 * @param key The name of the publication.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsEndpointSubscribe(HelicsEndpoint endpoint, const char* key, HelicsError* err);

/**
 * Check if the federate has any outstanding messages.
 *
 * @param fed The federate to check.
 *
 * @return HELICS_TRUE if the federate has a message waiting, HELICS_FALSE otherwise.
 */
HELICS_EXPORT HelicsBool helicsFederateHasMessage(HelicsFederate fed);

/**
 * Check if a given endpoint has any unread messages.
 *
 * @param endpoint The endpoint to check.
 *
 * @return HELICS_TRUE if the endpoint has a message, HELICS_FALSE otherwise.
 */
HELICS_EXPORT HelicsBool helicsEndpointHasMessage(HelicsEndpoint endpoint);

/**
 * Returns the number of pending receives for the specified destination endpoint.
 *
 * @param fed The federate to get the number of waiting messages from.
 */
HELICS_EXPORT int helicsFederatePendingMessageCount(HelicsFederate fed);

/**
 * Returns the number of pending receives for all endpoints of a particular federate.
 *
 * @param endpoint The endpoint to query.
 */
HELICS_EXPORT int helicsEndpointPendingMessageCount(HelicsEndpoint endpoint);

/**
 * Receive a packet from a particular endpoint.
 *
 * @param[in] endpoint The identifier for the endpoint.
 *
 * @return A message object.
 */
HELICS_EXPORT HelicsMessage helicsEndpointGetMessage(HelicsEndpoint endpoint);

/**
 * Create a new empty message object.
 *
 * @details The message is empty and isValid will return false since there is no data associated with the message yet.
 *
 * @param endpoint The endpoint object to associate the message with.
 *
 * @param[in,out] err An error object to fill out in case of an error.

 *
 * @return A new HelicsMessage.
 */
HELICS_EXPORT HelicsMessage helicsEndpointCreateMessage(HelicsEndpoint endpoint, HelicsError* err);

/**
 * Receive a communication message for any endpoint in the federate.
 *
 * @details The return order will be in order of endpoint creation.
 *          So all messages that are available for the first endpoint, then all for the second, and so on.
 *          Within a single endpoint, the messages are ordered by time, then source_id, then order of arrival.
 *
 * @return A HelicsMessage which references the data in the message.
 */
HELICS_EXPORT HelicsMessage helicsFederateGetMessage(HelicsFederate fed);

/**
 * Create a new empty message object.
 *
 * @details The message is empty and isValid will return false since there is no data associated with the message yet.
 *
 * @param fed the federate object to associate the message with
 *
 * @param[in,out] err An error object to fill out in case of an error.

 *
 * @return A HelicsMessage containing the message data.
 */
HELICS_EXPORT HelicsMessage helicsFederateCreateMessage(HelicsFederate fed, HelicsError* err);

/**
 * Clear all stored messages from a federate.
 *
 * @details This clears messages retrieved through helicsEndpointGetMessage or helicsFederateGetMessage
 *
 * @param fed The federate to clear the message for.
 */
HELICS_EXPORT void helicsFederateClearMessages(HelicsFederate fed);

/**
 * Get the type specified for an endpoint.
 *
 * @param endpoint The endpoint object in question.
 *
 * @return The defined type of the endpoint.
 */
HELICS_EXPORT const char* helicsEndpointGetType(HelicsEndpoint endpoint);

/**
 * Get the name of an endpoint.
 *
 * @param endpoint The endpoint object in question.
 *
 * @return The name of the endpoint.
 */
HELICS_EXPORT const char* helicsEndpointGetName(HelicsEndpoint endpoint);

/**
 * Get the number of endpoints in a federate.
 *
 * @param fed The message federate to query.
 *
 * @return (-1) if fed was not a valid federate, otherwise returns the number of endpoints.
 */
HELICS_EXPORT int helicsFederateGetEndpointCount(HelicsFederate fed);

/**
 * Get the local information field of an endpoint.
 *
 * @param end The endpoint to query.
 *
 * @return A string with the info field string.
 */
HELICS_EXPORT const char* helicsEndpointGetInfo(HelicsEndpoint end);

/**
 * Set the data in the interface information field for an endpoint.
 *
 * @param endpoint The endpoint to set the information for
 * @param info The string to store in the field
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */
HELICS_EXPORT void helicsEndpointSetInfo(HelicsEndpoint endpoint, const char* info, HelicsError* err);

/**
 * Get the data in a specified tag of an endpoint
 *
 * @param endpoint The endpoint to query.
 * @param tagname The name of the tag to query.
 * @return A string with the tag data.
 */
HELICS_EXPORT const char* helicsEndpointGetTag(HelicsEndpoint endpoint, const char* tagname);

/**
 * Set the data in a specific tag for an endpoint.
 *
 * @param endpoint The endpoint to query.
 * @param tagname The string to set.
 * @param tagvalue The string value to associate with a tag.
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */
HELICS_EXPORT void helicsEndpointSetTag(HelicsEndpoint endpoint, const char* tagname, const char* tagvalue, HelicsError* err);

/**
 * Set a handle option on an endpoint.
 *
 * @param endpoint The endpoint to modify.
 * @param option Integer code for the option to set /ref helics_handle_options.
 * @param value The value to set the option to.
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */
HELICS_EXPORT void helicsEndpointSetOption(HelicsEndpoint endpoint, int option, int value, HelicsError* err);

/**
 * Set a handle option on an endpoint.
 *
 * @param endpoint The endpoint to modify.
 * @param option Integer code for the option to set /ref helics_handle_options.
 * @return the value of the option, for boolean options will be 0 or 1
 */
HELICS_EXPORT int helicsEndpointGetOption(HelicsEndpoint endpoint, int option);

/**
 * add a source target to an endpoint,  Specifying an endpoint to receive undirected messages from
 *
 * @param endpoint The endpoint to modify.
 * @param targetEndpoint the endpoint to get messages from
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */
HELICS_EXPORT void helicsEndpointAddSourceTarget(HelicsEndpoint endpoint, const char* targetEndpoint, HelicsError* err);

/**
 * add a destination target to an endpoint,  Specifying an endpoint to send undirected messages to
 *
 * @param endpoint The endpoint to modify.
 * @param targetEndpoint the name of the endpoint to send messages to
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */
HELICS_EXPORT void helicsEndpointAddDestinationTarget(HelicsEndpoint endpoint, const char* targetEndpoint, HelicsError* err);

/**
 * remove an endpoint from being targeted
 *
 * @param endpoint The endpoint to modify.
 * @param targetEndpoint the name of the endpoint to send messages to
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */
HELICS_EXPORT void helicsEndpointRemoveTarget(HelicsEndpoint endpoint, const char* targetEndpoint, HelicsError* err);

/**
 * add a source Filter to an endpoint
 *
 * @param endpoint The endpoint to modify.
 * @param filterName the name of the filter to add
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */
HELICS_EXPORT void helicsEndpointAddSourceFilter(HelicsEndpoint endpoint, const char* filterName, HelicsError* err);

/**
 * add a destination filter to an endpoint
 *
 * @param endpoint The endpoint to modify.
 * @param filterName The name of the filter to add.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsEndpointAddDestinationFilter(HelicsEndpoint endpoint, const char* filterName, HelicsError* err);

/**
 * \defgroup Message operation functions
 * @details Functions for working with helics message envelopes.
 * @{
 */

/**
 * Get the source endpoint of a message.
 *
 * @param message The message object in question.
 *
 * @return A string with the source endpoint.
 */
HELICS_EXPORT const char* helicsMessageGetSource(HelicsMessage message);

/**
 * Get the destination endpoint of a message.
 *
 * @param message The message object in question.
 *
 * @return A string with the destination endpoint.
 */
HELICS_EXPORT const char* helicsMessageGetDestination(HelicsMessage message);

/**
 * Get the original source endpoint of a message, the source may have been modified by filters or other actions.
 *
 * @param message The message object in question.
 *
 * @return A string with the source of a message.
 */
HELICS_EXPORT const char* helicsMessageGetOriginalSource(HelicsMessage message);

/**
 * Get the original destination endpoint of a message, the destination may have been modified by filters or other actions.
 *
 * @param message The message object in question.
 *
 * @return A string with the original destination of a message.
 */
HELICS_EXPORT const char* helicsMessageGetOriginalDestination(HelicsMessage message);

/**
 * Get the helics time associated with a message.
 *
 * @param message The message object in question.
 *
 * @return The time associated with a message.
 */
HELICS_EXPORT HelicsTime helicsMessageGetTime(HelicsMessage message);

/**
 * Get the payload of a message as a string.
 *
 * @param message The message object in question.
 *
 * @return A string representing the payload of a message.
 */
HELICS_EXPORT const char* helicsMessageGetString(HelicsMessage message);

/**
 * Get the messageID of a message.
 *
 * @param message The message object in question.
 *
 * @return The messageID.
 */
HELICS_EXPORT int helicsMessageGetMessageID(HelicsMessage message);

/**
 * Check if a flag is set on a message.
 *
 * @param message The message object in question.
 * @param flag The flag to check should be between [0,15].
 *
 * @return The flags associated with a message.
 */
HELICS_EXPORT HelicsBool helicsMessageGetFlagOption(HelicsMessage message, int flag);

/**
 * Get the size of the data payload in bytes.
 *
 * @param message The message object in question.
 *
 * @return The size of the data payload.
 */
HELICS_EXPORT int helicsMessageGetByteCount(HelicsMessage message);

/**
 * Get the raw data for a message object.
 *
 * @param message A message object to get the data for.
 *
 * @param[out] data The memory location of the data.
 * @param maxMessageLength The maximum size of information that data can hold.
 * @param[out] actualSize The actual length of data copied to data.
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsMessageGetBytes(HelicsMessage message, void* data, int maxMessageLength, int* actualSize, HelicsError* err);

/**
 * Get a pointer to the raw data of a message.
 *
 * @param message A message object to get the data for.
 *
 * @return A pointer to the raw data in memory, the pointer may be NULL if the message is not a valid message.
 */
HELICS_EXPORT void* helicsMessageGetBytesPointer(HelicsMessage message);

/**
 * A check if the message contains a valid payload.
 *
 * @param message The message object in question.
 *
 * @return HELICS_TRUE if the message contains a payload.
 */
HELICS_EXPORT HelicsBool helicsMessageIsValid(HelicsMessage message);

/**
 * Set the source of a message.
 *
 * @param message The message object in question.
 * @param src A string containing the source.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageSetSource(HelicsMessage message, const char* src, HelicsError* err);

/**
 * Set the destination of a message.
 *
 * @param message The message object in question.
 * @param dst A string containing the new destination.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageSetDestination(HelicsMessage message, const char* dst, HelicsError* err);

/**
 * Set the original source of a message.
 *
 * @param message The message object in question.
 * @param src A string containing the new original source.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageSetOriginalSource(HelicsMessage message, const char* src, HelicsError* err);

/**
 * Set the original destination of a message.
 *
 * @param message The message object in question.
 * @param dst A string containing the new original source.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageSetOriginalDestination(HelicsMessage message, const char* dst, HelicsError* err);

/**
 * Set the delivery time for a message.
 *
 * @param message The message object in question.
 * @param time The time the message should be delivered.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageSetTime(HelicsMessage message, HelicsTime time, HelicsError* err);

/**
 * Resize the data buffer for a message.
 *
 * @details The message data buffer will be resized. There are no guarantees on what is in the buffer in newly allocated space.
 *          If the allocated space is not sufficient new allocations will occur.
 *
 * @param message The message object in question.
 * @param newSize The new size in bytes of the buffer.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageResize(HelicsMessage message, int newSize, HelicsError* err);

/**
 * Reserve space in a buffer but don't actually resize.
 *
 * @details The message data buffer will be reserved but not resized.
 *
 * @param message The message object in question.
 * @param reserveSize The number of bytes to reserve in the message object.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageReserve(HelicsMessage message, int reserveSize, HelicsError* err);

/**
 * Set the message ID for the message.
 *
 * @details Normally this is not needed and the core of HELICS will adjust as needed.
 *
 * @param message The message object in question.
 * @param messageID A new message ID.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageSetMessageID(HelicsMessage message, int32_t messageID, HelicsError* err);

/**
 * Clear the flags of a message.
 *
 * @param message The message object in question
 */
HELICS_EXPORT void helicsMessageClearFlags(HelicsMessage message);

/**
 * Set a flag on a message.
 *
 * @param message The message object in question.
 * @param flag An index of a flag to set on the message.
 * @param flagValue The desired value of the flag.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageSetFlagOption(HelicsMessage message, int flag, HelicsBool flagValue, HelicsError* err);

/**
 * Set the data payload of a message as a string.
 *
 * @param message The message object in question.
 * @param str A string containing the message data.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageSetString(HelicsMessage message, const char* str, HelicsError* err);

/**
 * Set the data payload of a message as raw data.
 *
 * @param message The message object in question.
 * @param data A string containing the message data.
 * @param inputDataLength The length of the data to input.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageSetData(HelicsMessage message, const void* data, int inputDataLength, HelicsError* err);

/**
 * Append data to the payload.
 *
 * @param message The message object in question.
 * @param data A string containing the message data to append.
 * @param inputDataLength The length of the data to input.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageAppendData(HelicsMessage message, const void* data, int inputDataLength, HelicsError* err);

/**
 * Copy a message object.
 *
 * @param src_message The message object to copy from.
 * @param dst_message The message object to copy to.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageCopy(HelicsMessage src_message, HelicsMessage dst_message, HelicsError* err);

/**
 * Clone a message object.
 *
 * @param message The message object to copy from.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT HelicsMessage helicsMessageClone(HelicsMessage message, HelicsError* err);

/**
 * Free a message object from memory
 * @param message The message object to copy from.
 * @details memory for message is managed so not using this function does not create memory leaks, this is an indication
 * to the system that the memory for this message is done being used and can be reused for a new message.
 * helicsFederateClearMessages() can also be used to clear up all stored messages at once
 */
HELICS_EXPORT void helicsMessageFree(HelicsMessage message);

/**
 * Reset a message to empty state
 * @param message The message object to copy from.
 * @details The message after this function will be empty, with no source or destination
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageClear(HelicsMessage message, HelicsError* err);

/**@}*/

/**
 * Create a source Filter on the specified federate.
 *
 * @details Filters can be created through a federate or a core, linking through a federate allows
 *          a few extra features of name matching to function on the federate interface but otherwise equivalent behavior
 *
 * @param fed The federate to register through.
 * @param type The type of filter to create /ref HelicsFilterTypes.
 * @param name The name of the filter (can be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsFilter object.
 */
HELICS_EXPORT HelicsFilter helicsFederateRegisterFilter(HelicsFederate fed, HelicsFilterTypes type, const char* name, HelicsError* err);
/**
 * Create a global source filter through a federate.
 *
 * @details Filters can be created through a federate or a core, linking through a federate allows
 *          a few extra features of name matching to function on the federate interface but otherwise equivalent behavior.
 *
 * @param fed The federate to register through.
 * @param type The type of filter to create /ref HelicsFilterTypes.
 * @param name The name of the filter (can be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsFilter object.
 */
HELICS_EXPORT HelicsFilter helicsFederateRegisterGlobalFilter(HelicsFederate fed,
                                                              HelicsFilterTypes type,
                                                              const char* name,
                                                              HelicsError* err);

/**
 * Create a cloning Filter on the specified federate.
 *
 * @details Cloning filters copy a message and send it to multiple locations, source and destination can be added
 *          through other functions.
 *
 * @param fed The federate to register through.
 * @param name The name of the filter (can be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsFilter object.
 */
HELICS_EXPORT HelicsFilter helicsFederateRegisterCloningFilter(HelicsFederate fed, const char* name, HelicsError* err);

/**
 * Create a global cloning Filter on the specified federate.
 *
 * @details Cloning filters copy a message and send it to multiple locations, source and destination can be added
 *          through other functions.
 *
 * @param fed The federate to register through.
 * @param name The name of the filter (can be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsFilter object.
 */
HELICS_EXPORT HelicsFilter helicsFederateRegisterGlobalCloningFilter(HelicsFederate fed, const char* name, HelicsError* err);

/**
 * Create a source Filter on the specified core.
 *
 * @details Filters can be created through a federate or a core, linking through a federate allows
 *          a few extra features of name matching to function on the federate interface but otherwise equivalent behavior.
 *
 * @param core The core to register through.
 * @param type The type of filter to create /ref HelicsFilterTypes.
 * @param name The name of the filter (can be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsFilter object.
 */
HELICS_EXPORT HelicsFilter helicsCoreRegisterFilter(HelicsCore core, HelicsFilterTypes type, const char* name, HelicsError* err);

/**
 * Create a cloning Filter on the specified core.
 *
 * @details Cloning filters copy a message and send it to multiple locations, source and destination can be added
 *          through other functions.
 *
 * @param core The core to register through.
 * @param name The name of the filter (can be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsFilter object.
 */
HELICS_EXPORT HelicsFilter helicsCoreRegisterCloningFilter(HelicsCore core, const char* name, HelicsError* err);

/**
 * Get the number of filters registered through a federate.
 *
 * @param fed The federate object to use to get the filter.
 *
 * @return A count of the number of filters registered through a federate.
 */
HELICS_EXPORT int helicsFederateGetFilterCount(HelicsFederate fed);

/**
 * Get a filter by its name, typically already created via registerInterfaces file or something of that nature.
 *
 * @param fed The federate object to use to get the filter.
 * @param name The name of the filter.
 *
 * @param[in,out] err The error object to complete if there is an error.

 *
 * @return A HelicsFilter object, the object will not be valid and err will contain an error code if no filter with the specified name
 * exists.
 */
HELICS_EXPORT HelicsFilter helicsFederateGetFilter(HelicsFederate fed, const char* name, HelicsError* err);
/**
 * Get a filter by its index, typically already created via registerInterfaces file or something of that nature.
 *
 * @param fed The federate object in which to create a publication.
 * @param index The index of the publication to get.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsFilter, which will be NULL if an invalid index is given.
 */
HELICS_EXPORT HelicsFilter helicsFederateGetFilterByIndex(HelicsFederate fed, int index, HelicsError* err);

/**
 * Check if a filter is valid.
 *
 * @param filt The filter object to check.
 *
 * @return HELICS_TRUE if the Filter object represents a valid filter.
 */
HELICS_EXPORT HelicsBool helicsFilterIsValid(HelicsFilter filt);

/**
 * Get the name of the filter and store in the given string.
 *
 * @param filt The given filter.
 *
 * @return A string with the name of the filter.
 */
HELICS_EXPORT const char* helicsFilterGetName(HelicsFilter filt);

/**
 * Set a property on a filter.
 *
 * @param filt The filter to modify.
 * @param prop A string containing the property to set.
 * @param val A numerical value for the property.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsFilterSet(HelicsFilter filt, const char* prop, double val, HelicsError* err);

/**
 * Set a string property on a filter.
 *
 * @param filt The filter to modify.
 * @param prop A string containing the property to set.
 * @param val A string containing the new value.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsFilterSetString(HelicsFilter filt, const char* prop, const char* val, HelicsError* err);

/**
 * Add a destination target to a filter.
 *
 * @details All messages going to a destination are copied to the delivery address(es).
 * @param filt The given filter to add a destination target to.
 * @param dst The name of the endpoint to add as a destination target.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsFilterAddDestinationTarget(HelicsFilter filt, const char* dst, HelicsError* err);

/**
 * Add a source target to a filter.
 *
 * @details All messages coming from a source are copied to the delivery address(es).
 *
 * @param filt The given filter.
 * @param source The name of the endpoint to add as a source target.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsFilterAddSourceTarget(HelicsFilter filt, const char* source, HelicsError* err);

/**
 * \defgroup Clone filter functions
 * @details Functions that manipulate cloning filters in some way.
 * @{
 */

/**
 * Add a delivery endpoint to a cloning filter.
 *
 * @details All cloned messages are sent to the delivery address(es).
 *
 * @param filt The given filter.
 * @param deliveryEndpoint The name of the endpoint to deliver messages to.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsFilterAddDeliveryEndpoint(HelicsFilter filt, const char* deliveryEndpoint, HelicsError* err);

/**
 * Remove a destination target from a filter.
 *
 * @param filt The given filter.
 * @param target The named endpoint to remove as a target.
 *
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsFilterRemoveTarget(HelicsFilter filt, const char* target, HelicsError* err);

/**
 * Remove a delivery destination from a cloning filter.
 *
 * @param filt The given filter (must be a cloning filter).
 * @param deliveryEndpoint A string with the delivery endpoint to remove.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsFilterRemoveDeliveryEndpoint(HelicsFilter filt, const char* deliveryEndpoint, HelicsError* err);

/**
 * Get the data in the info field of a filter.
 *
 * @param filt The given filter.
 *
 * @return A string with the info field string.
 */
HELICS_EXPORT const char* helicsFilterGetInfo(HelicsFilter filt);
/**
 * Set the data in the info field for a filter.
 *
 * @param filt The given filter.
 * @param info The string to set.
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */
HELICS_EXPORT void helicsFilterSetInfo(HelicsFilter filt, const char* info, HelicsError* err);

/**
 * Get the data in a specified tag of a filter.
 *
 * @param filt The filter to query.
 * @param tagname The name of the tag to query.
 * @return A string with the tag data.
 */
HELICS_EXPORT const char* helicsFilterGetTag(HelicsFilter filt, const char* tagname);

/**
 * Set the data in a specific tag for a filter.
 *
 * @param filt The filter object to set the tag for.
 * @param tagname The string to set.
 * @param tagvalue the string value to associate with a tag.
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */
HELICS_EXPORT void helicsFilterSetTag(HelicsFilter filt, const char* tagname, const char* tagvalue, HelicsError* err);

/**
 * Set an option value for a filter.
 *
 * @param filt The given filter.
 * @param option The option to set /ref helics_handle_options.
 * @param value The value of the option commonly 0 for false 1 for true.
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */

HELICS_EXPORT void helicsFilterSetOption(HelicsFilter filt, int option, int value, HelicsError* err);

/**
 * Get a handle option for the filter.
 *
 * @param filt The given filter to query.
 * @param option The option to query /ref helics_handle_options.
 */
HELICS_EXPORT int helicsFilterGetOption(HelicsFilter filt, int option);

/**
 * @}
 */

/**
 * Set the logging callback to a broker.
 *
 * @details Add a logging callback function to a broker.
 *          The logging callback will be called when
 *          a message flows into a broker from the core or from a broker.
 *
 * @param broker The broker object in which to set the callback.
 * @param logger A callback with signature void(int, const char *, const char *, void *);
 *               the function arguments are loglevel, an identifier, a message string, and a pointer to user data.
 * @param userdata A pointer to user data that is passed to the function when executing.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsBrokerSetLoggingCallback(HelicsBroker broker,
                                                  void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
                                                  void* userdata,
                                                  HelicsError* err);

/**
 * Set the logging callback for a core.
 *
 * @details Add a logging callback function to a core. The logging callback will be called when
 *          a message flows into a core from the core or from a broker.
 *
 * @param core The core object in which to set the callback.
 * @param logger A callback with signature void(int, const char *, const char *, void *);
 *               The function arguments are loglevel, an identifier, a message string, and a pointer to user data.
 * @param userdata A pointer to user data that is passed to the function when executing.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsCoreSetLoggingCallback(HelicsCore core,
                                                void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
                                                void* userdata,
                                                HelicsError* err);

/**
 * Set the logging callback for a federate.
 *
 * @details Add a logging callback function to a federate. The logging callback will be called when
 *          a message flows into a federate from the core or from a federate.
 *
 * @param fed The federate object in which to create a subscription must have been created with
 *            helicsCreateValueFederate or helicsCreateCombinationFederate.
 * @param logger A callback with signature void(int, const char *, const char *, void *);
 *        The function arguments are loglevel, an identifier string, a message string, and a pointer to user data.
 * @param userdata A pointer to user data that is passed to the function when executing.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void
    helicsFederateSetLoggingCallback(HelicsFederate fed,
                                     void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
                                     void* userdata,
                                     HelicsError* err);

/**
 * Set a general callback for a custom filter.
 *
 * @details Add a custom filter callback for creating a custom filter operation in the C shared library.
 *
 * @param filter The filter object to set the callback for.
 * @param filtCall A callback with signature helics_message_object(helics_message_object, void *);
 *                 The function arguments are the message to filter and a pointer to user data.
 *                 The filter should return a new message.
 * @param userdata A pointer to user data that is passed to the function when executing.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsFilterSetCustomCallback(HelicsFilter filter,
                                                 void (*filtCall)(HelicsMessage message, void* userData),
                                                 void* userdata,
                                                 HelicsError* err);

/**
 * Set callback for queries executed against a federate.
 *
 * @details There are many queries that HELICS understands directly, but it is occasionally useful to have a federate be able to respond
 * to specific queries with answers specific to a federate.
 *
 * @param fed The federate to set the callback for.
 * @param queryAnswer A callback with signature const char *(const char *query, int querySize,int *answerSize, void *userdata);
 *                 The function arguments are the query string requesting an answer along with its size, the string is not guaranteed to be
 * null terminated answerSize is an outputParameter intended to filled out by the userCallback and should contain the length of the return
 * string. The return pointer can be NULL if no answer is given and HELICS will generate the appropriate response.
 * @param userdata A pointer to user data that is passed to the function when executing.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */

HELICS_EXPORT void
    helicsFederateSetQueryCallback(HelicsFederate fed,
                                   void (*queryAnswer)(const char* query, int querySize, HelicsQueryBuffer buffer, void* userdata),
                                   void* userdata,
                                   HelicsError* err);

/**
 * Set the data for a query callback.
 *
 * @details There are many queries that HELICS understands directly, but it is occasionally useful to have a federate be able to respond
 * to specific queries with answers specific to a federate.
 *
 * @param buffer The buffer received in a helicsQueryCallback.
 * @param str Pointer to the data to fill the buffer with.
 * @param strSize The size of the string.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsQueryBufferFill(HelicsQueryBuffer buffer, const char* str, int strSize, HelicsError* err);

#ifdef __cplusplus
} /* end of extern "C" { */
#endif
#endif
