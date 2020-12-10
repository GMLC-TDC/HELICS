/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef HELICS_C_API_H_
#define HELICS_C_API_H_

#pragma once

#include <stdint.h>
#include <stdlib.h>

#if defined _WIN32 || defined __CYGWIN__
#    ifdef __GNUC__
#        define HELICS_EXPORT __attribute__((dllimport))
#    else
#        define HELICS_EXPORT __declspec(dllimport)
#    endif
#else
#    define HELICS_EXPORT
#endif

#define HELICS_DEPRECATED

#ifdef __cplusplus
extern "C" {
#endif
/** @file
@brief base helics enumerations for C and C++ API's
*/

/** pick a core type depending on compile configuration usually either ZMQ if available or TCP */
typedef enum {
    helics_core_type_default = 0, /*!< a default core type that will default to something available*/
    helics_core_type_zmq = 1, /*!< use the Zero MQ networking protocol */
    helics_core_type_mpi = 2, /*!< use MPI for operation on a parallel cluster */
    helics_core_type_test = 3, /*!< use the Test core if all federates are in the same process */
    /** interprocess uses memory mapped files to transfer data (for use when all federates are
        on the same machine */
    helics_core_type_interprocess = 4,
    /** interprocess uses memory mapped files to transfer data (for use when all federates are
        on the same machine ipc is the same as /ref helics_core_type_interprocess*/
    helics_core_type_ipc = 5,
    helics_core_type_tcp = 6, /*!< use a generic TCP protocol message stream to send messages */
    helics_core_type_udp = 7, /*!< use UDP packets to send the data */
    helics_core_type_zmq_test = 10, /*!< single socket version of ZMQ core usually for high fed count on the same system*/
    helics_core_type_nng = 9, /*!< for using the nanomsg communications */
    helics_core_type_tcp_ss = 11, /*!< a single socket version of the TCP core for more easily handling firewalls*/
    helics_core_type_http = 12, /*!< a core type using http for communication*/
    helics_core_type_websocket = 14, /*!< a core using websockets for communication*/
    helics_core_type_inproc = 18, /*!< an in process core type for handling communications in shared
                                     memory it is pretty similar to the test core but stripped from
                                     the "test" components*/
    helics_core_type_null = 66 /*!< an explicit core type that is recognized but explicitly doesn't
                                  exist, for testing and a few other assorted reasons*/
} helics_core_type;

/** enumeration of allowable data types for publications and inputs*/
typedef enum {
    /** a sequence of characters*/
    helics_data_type_string = 0,
    /** a double precision floating point number*/
    helics_data_type_double = 1,
    /** a 64 bit integer*/
    helics_data_type_int = 2,
    /** a pair of doubles representing a complex number*/
    helics_data_type_complex = 3,
    /** an array of doubles*/
    helics_data_type_vector = 4,
    /** a complex vector object*/
    helics_data_type_complex_vector = 5,
    /** a named point consisting of a string and a double*/
    helics_data_type_named_point = 6,
    /** a boolean data type*/
    helics_data_type_boolean = 7,
    /** time data type*/
    helics_data_type_time = 8,
    /** raw data type*/
    helics_data_type_raw = 25,
    /** the data type can change*/
    helics_data_type_multi = 33,
    /** open type that can be anything*/
    helics_data_type_any = 25262
} helics_data_type;

/** single character data type  this is intentionally the same as string*/
#define helics_data_type_char helics_data_type_string

/** enumeration of possible federate flags*/
typedef enum {
    /** flag indicating that a federate is observe only*/
    helics_flag_observer = 0,
    /** flag indicating that a federate can only return requested times*/
    helics_flag_uninterruptible = 1,
    /** flag indicating that a federate can be interrupted*/
    helics_flag_interruptible = 2,
    /** flag indicating that a federate/interface is a signal generator only*/
    helics_flag_source_only = 4,
    /** flag indicating a federate/interface should only transmit values if they have changed(binary
           equivalence)*/
    helics_flag_only_transmit_on_change = 6,
    /** flag indicating a federate/interface should only trigger an update if a value has changed
     * (binary equivalence)*/
    helics_flag_only_update_on_change = 8,
    /** flag indicating a federate should only grant time if all other federates have already passed
     * the requested time*/
    helics_flag_wait_for_current_time_update = 10,
    /** flag indicating a federate should operate on a restrictive time policy, which disallows some
       2nd order time evaluation and can be useful for certain types of dependency cycles
        and update patterns, but generally shouldn't be used as it can lead to some very slow update
       conditions*/
    helics_flag_restrictive_time_policy = 11,
    /** flag indicating that a federate has rollback capability*/
    helics_flag_rollback = 12,
    /** flag indicating that a federate performs forward computation and does internal rollback*/
    helics_flag_forward_compute = 14,
    /** flag indicating that a federate needs to run in real time*/
    helics_flag_realtime = 16,
    /** flag indicating that the federate will only interact on a single thread*/
    helics_flag_single_thread_federate = 27,
    /** used to not display warnings on mismatched requested times*/
    helics_flag_ignore_time_mismatch_warnings = 67,
    /** specify that checking on configuration files should be strict and throw and error on any
   invalid values */
    helics_flag_strict_config_checking = 75,

} helics_federate_flags;

/** enumeration of additional core flags*/
typedef enum {
    /** used to delay a core from entering initialization mode even if it would otherwise be ready*/
    helics_flag_delay_init_entry = 45,
    /** used to clear the HELICS_DELAY_INIT_ENTRY flag in cores*/
    helics_flag_enable_init_entry = 47,
} helics_core_flags;

/** enumeration of general flags that can be used in federates/cores/brokers */
typedef enum {
    /** flag specifying that a federate, core, or broker may be slow to respond to pings
        If the federate goes offline there is no good way to detect it so use with caution
        */
    helics_flag_slow_responding = 29,
    /** flag specifying the federate/core/broker is operating in a user debug mode so deadlock
    timers and timeout are disabled this flag is a combination of slow_responding and disabling of
    some timeouts*/
    helics_flag_debugging = 31,
    /** specify that a federate error should terminate the federation*/
    helics_flag_terminate_on_error = 72,
    /** specify that the log files should be flushed on every log message*/
    helics_flag_force_logging_flush = 88,
    /** specify that a full log should be dumped into a file*/
    helics_flag_dumplog = 89
} helics_flags;

/** log level definitions
 */
typedef enum { /** don't print anything except a few catastrophic errors*/
               helics_log_level_no_print = -1,
               /** only print error level indicators*/
               helics_log_level_error = 0,
               /** only print warnings and errors*/
               helics_log_level_warning = 1,
               /** warning errors and summary level information*/
               helics_log_level_summary = 2,
               /** summary+ notices about federate and broker connections +messages about network
                  connections*/
               helics_log_level_connections = 3,
               /** connections+ interface definitions*/
               helics_log_level_interfaces = 4,
               /** interfaces + timing message*/
               helics_log_level_timing = 5,
               /** timing+ data transfer notices*/
               helics_log_level_data = 6,
               /** all internal messages*/
               helics_log_level_trace = 7
} helics_log_levels;

/** enumeration of return values from the C interface functions
 */
typedef enum {
    helics_error_fatal = -404, /*!< global fatal error for federation */
    helics_error_external_type = -203, /*!< an unknown non-helics error was produced */
    helics_error_other = -101, /*!< the function produced a helics error of some other type */
    helics_error_insufficient_space = -18, /*!< insufficient space is available to store requested data */
    helics_error_execution_failure = -14, /*!< the function execution has failed */
    helics_error_invalid_function_call = -10, /*!< the call made was invalid in the present state of the calling object */
    helics_error_invalid_state_transition = -9, /*!< error issued when an invalid state transition occurred */
    helics_warning = -8, /*!< the function issued a warning of some kind */
    helics_error_system_failure = -6, /*!< the federate has terminated unexpectedly and the call cannot be completed */
    helics_error_discard = -5, /*!< the input was discarded and not used for some reason */
    helics_error_invalid_argument = -4, /*!< the parameter passed was invalid and unable to be used */
    helics_error_invalid_object = -3, /*!< indicator that the object used was not a valid object */
    helics_error_connection_failure = -2, /*!< the operation to connect has failed */
    helics_error_registration_failure = -1, /*!< registration has failed */
    helics_ok = 0 /*!< the function executed successfully */
} helics_error_types;

/** enumeration of properties that apply to federates*/
typedef enum {
    /** the property controlling the minimum time delta for a federate*/
    helics_property_time_delta = 137,
    /** the property controlling the period for a federate*/
    helics_property_time_period = 140,
    /** the property controlling time offset for the period of federate*/
    helics_property_time_offset = 141,
    /** the property controlling real time lag for a federate the max time a federate can lag
       real time*/
    helics_property_time_rt_lag = 143,
    /** the property controlling real time lead for a federate the max time a federate can be
       ahead of real time*/
    helics_property_time_rt_lead = 144,
    /** the property controlling real time tolerance for a federate sets both rt_lag and
       rt_lead*/
    helics_property_time_rt_tolerance = 145,
    /** the property controlling input delay for a federate*/
    helics_property_time_input_delay = 148,
    /** the property controlling output delay for a federate*/
    helics_property_time_output_delay = 150,
    /** integer property controlling the maximum number of iterations in a federate*/
    helics_property_int_max_iterations = 259,
    /** integer property controlling the log level in a federate see \ref helics_log_levels*/
    helics_property_int_log_level = 271,
    /** integer property controlling the log level for file logging in a federate see \ref
       helics_log_levels*/
    helics_property_int_file_log_level = 272,
    /** integer property controlling the log level for file logging in a federate see \ref
       helics_log_levels*/
    helics_property_int_console_log_level = 274
} helics_properties;

/** enumeration of the multi_input operations*/
typedef enum {
    /** time and priority order the inputs from the core library*/
    helics_multi_input_no_op = 0,
    /** vectorize the inputs either double vector or string vector*/
    helics_multi_input_vectorize_operation = 1,
    /** all inputs are assumed to be boolean and all must be true to return true*/
    helics_multi_input_and_operation = 2,
    /** all inputs are assumed to be boolean and at least one must be true to return true*/
    helics_multi_input_or_operation = 3,
    /** sum all the inputs*/
    helics_multi_input_sum_operation = 4,
    /** do a difference operation on the inputs, first-sum(rest)
    for double input, vector diff for vector input*/
    helics_multi_input_diff_operation = 5,
    /** find the max of the inputs*/
    helics_multi_input_max_operation = 6,
    /** find the min of the inputs*/
    helics_multi_input_min_operation = 7,
    /** take the average of the inputs*/
    helics_multi_input_average_operation = 8

} helics_multi_input_mode;

/** enumeration of options that apply to handles*/
typedef enum {
    /** specify that a connection is required for an interface and will generate an error if not
       available*/
    helics_handle_option_connection_required = 397,
    /** specify that a connection is NOT required for an interface and will only be made if
       available no warning will be issues if not available*/
    helics_handle_option_connection_optional = 402,
    /** specify that only a single connection is allowed for an interface*/
    helics_handle_option_single_connection_only = 407,
    /** specify that multiple connections are allowed for an interface*/
    helics_handle_option_multiple_connections_allowed = 409,
    /** specify that the last data should be buffered and sent on subscriptions after init*/
    helics_handle_option_buffer_data = 411,
    /** specify that the types should be checked strictly for pub/sub and filters*/
    helics_handle_option_strict_type_checking = 414,
    /** specify that the mismatching units should be ignored*/
    helics_handle_option_ignore_unit_mismatch = 447,
    /** specify that an interface will only transmit on change(only applicable to
       publications)*/
    helics_handle_option_only_transmit_on_change = 452,
    /** specify that an interface will only update if the value has actually changed*/
    helics_handle_option_only_update_on_change = 454,
    /** specify that an interface does not participate in determining time interrupts*/
    helics_handle_option_ignore_interrupts = 475,
    /** specify the multi-input processing method for inputs*/
    helics_handle_option_multi_input_handling_method = 507,
    /** specify the source index with the highest priority*/
    helics_handle_option_input_priority_location = 510,
    /** specify that the priority list should be cleared or question if it is cleared*/
    helics_handle_option_clear_priority_list = 512,
    /** specify the required number of connections or get the actual number of connections*/
    helics_handle_option_connections = 522
} helics_handle_options;

/** enumeration of the predefined filter types*/
typedef enum {
    /** a custom filter type that executes a user defined callback*/
    helics_filter_type_custom = 0,
    /** a filter type that executes a fixed delay on a message*/
    helics_filter_type_delay = 1,
    /** a filter type that executes a random delay on the messages*/
    helics_filter_type_random_delay = 2,
    /** a filter type that randomly drops messages*/
    helics_filter_type_random_drop = 3,
    /** a filter type that reroutes a message to a different destination than originally
       specified*/
    helics_filter_type_reroute = 4,
    /** a filter type that duplicates a message and sends the copy to a different destination*/
    helics_filter_type_clone = 5,
    /** a customizable filter type that can perform different actions on a message based on
       firewall like rules*/
    helics_filter_type_firewall = 6

} helics_filter_type;

/**
 * @file
 * @brief Data structures for the C api
 */

/**
 * opaque object representing an input
 */
typedef void* helics_input;
typedef void* HelicsInput;
/**
 * opaque object representing a publication
 */
typedef void* helics_publication;
typedef void* HelicsPublication;
/**
 * opaque object representing an endpoint
 */
typedef void* helics_endpoint;
typedef void* HelicsEndpoint;

/**
 * opaque object representing a filter
 */
typedef void* helics_filter;
typedef void* HelicsFilter;

/**
 * opaque object representing a core
 */
typedef void* helics_core;
typedef void* HelicsCore;

/**
 * opaque object representing a broker
 */
typedef void* helics_broker;
typedef void* HelicsBroker;
/**
 * opaque object representing a federate
 */
typedef void* helics_federate;
typedef void* HelicsFederate;

/**
 * opaque object representing a filter info object structure
 */
typedef void* helics_federate_info;
typedef void* HelicsFederateInfo;

/**
 * opaque object representing a query
 */
typedef void* helics_query;
typedef void* HelicsQuery;

/**
 * opaque object representing a string buffer for a query
 */
typedef void* helics_query_buffer;

/**
 * opaque object representing a message
 */
typedef void* helics_message;
typedef void* HelicsMessage;

/**
 * time definition used in the C interface to helics
 */
typedef double helics_time;
typedef double HelicsTime;

const HelicsTime helics_time_zero = 0.0; /*!< definition of time zero-the beginning of simulation */
const HelicsTime helics_time_epsilon = 1.0e-9; /*!< definition of the minimum time resolution */
const HelicsTime helics_time_invalid = -1.785e39; /*!< definition of an invalid time that has no meaning */
const HelicsTime helics_time_maxtime = 9223372036.854774; /*!< definition of time signifying the federate has
                                                             terminated or to run until the end of the simulation*/

/**
 * defining a boolean type for use in the helics interface
 */
typedef int helics_bool;
typedef int HelicsBool;

const helics_bool helics_true = 1; /*!< indicator used for a true response */
const helics_bool helics_false = 0; /*!< indicator used for a false response */

/**
 * enumeration of the different iteration results
 */
typedef enum {
    helics_iteration_request_no_iteration, /*!< no iteration is requested */
    helics_iteration_request_force_iteration, /*!< force iteration return when able */
    helics_iteration_request_iterate_if_needed /*!< only return an iteration if necessary */
} helics_iteration_request;

/**
 * enumeration of possible return values from an iterative time request
 */
typedef enum {
    helics_iteration_result_next_step, /*!< the iterations have progressed to the next time */
    helics_iteration_result_error, /*!< there was an error */
    helics_iteration_result_halted, /*!< the federation has halted */
    helics_iteration_result_iterating /*!< the federate is iterating at current time */
} helics_iteration_result;

/**
 * enumeration of possible federate states
 */
typedef enum {
    helics_state_startup = 0, /*!< when created the federate is in startup state */
    helics_state_initialization, /*!< entered after the enterInitializingMode call has returned */
    helics_state_execution, /*!< entered after the enterExectuationState call has returned */
    helics_state_finalize, /*!< the federate has finished executing normally final values may be retrieved */
    helics_state_error, /*!< error state no core communication is possible but values can be retrieved */
    /* the following states are for asynchronous operations */
    helics_state_pending_init, /*!< indicator that the federate is pending entry to initialization state */
    helics_state_pending_exec, /*!< state pending EnterExecution State */
    helics_state_pending_time, /*!< state that the federate is pending a timeRequest */
    helics_state_pending_iterative_time, /*!< state that the federate is pending an iterative time request */
    helics_state_pending_finalize /*!< state that the federate is pending a finalize request */
} helics_federate_state;

/**
 *  structure defining a basic complex type
 */
typedef struct HelicsComplex {
    double real;
    double imag;
} HelicsComplex;

typedef HelicsComplex helics_complex;

/**
 * helics error object
 *
 * if error_code==0 there is no error, if error_code!=0 there is an error and message will contain a string,
 * otherwise it will be an empty string
 */
typedef struct helics_error {
    int32_t error_code; /*!< an error code associated with the error*/
    const char* message; /*!< a message associated with the error*/
} helics_error;

typedef helics_error HelicsError;

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
 * @param error_code The integer code for the error.
 * @param error_string A string describing the error.
 */
HELICS_EXPORT void helicsFederateGlobalError(helics_federate fed, int error_code, const char* error_string);

/**
 * Generate a local error in a federate.
 *
 * @details This will propagate through the co-simulation but not necessarily halt the co-simulation, it has a similar effect to finalize
 * but does allow some interaction with a core for a brief time.
 * @param fed The federate to create an error in.
 * @param error_code The integer code for the error.
 * @param error_string A string describing the error.
 */
HELICS_EXPORT void helicsFederateLocalError(helics_federate fed, int error_code, const char* error_string);

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
HELICS_EXPORT helics_core helicsFederateGetCore(helics_federate fed, helics_error* err);

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
 * Send a command to another helics object through a federate.
 *
 * @param fed The federate to send the command through.
 * @param target The name of the object to send the command to.
 * @param command The command to send.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederateSendCommand(helics_federate fed, const char* target, const char* command, helics_error* err);

/**
 * Get a command sent to the federate.
 *
 * @param fed The federate to get the command for.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return A string with the command for the federate, if the string is empty no command is available.
 */
HELICS_EXPORT const char* helicsFederateGetCommand(helics_federate fed, helics_error* err);

/**
 * Get the source of the most recently retrieved command sent to the federate.
 *
 * @param fed The federate to get the command for.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return A string with the command for the federate, if the string is empty no command is available.
 */
HELICS_EXPORT const char* helicsFederateGetCommandSource(helics_federate fed, helics_error* err);

/**
 * Get a command sent to the federate. Blocks until a command is received.
 *
 * @param fed The federate to get the command for.
 * @param target The name of the object to send the command to.
 * @param command The command to send.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @return A string with the command for the federate, if the string is empty no command is available.
 */
HELICS_EXPORT const char* helicsFederateWaitCommand(helics_federate fed, helics_error* err);
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
 * Send a command to another helics object though a core.
 *
 * @param core The core to send the command through.
 * @param target The name of the object to send the command to.
 * @param command The command to send.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsCoreSendCommand(helics_core core, const char* target, const char* command, helics_error* err);

/**
 * Send a command to another helics object through a broker.
 *
 * @param core The core to send the command through.
 * @param target The name of the object to send the command to.
 * @param command The command to send.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsBrokerSendCommand(helics_broker broker, const char* target, const char* command, helics_error* err);

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
 * Free the memory associated with a query object.
 */
HELICS_EXPORT void helicsQueryFree(helics_query query);

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
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the subscription.
 */
HELICS_EXPORT helics_input helicsFederateRegisterSubscription(helics_federate fed, const char* key, const char* units, helics_error* err);

/**
 * Register a publication with a known type.
 *
 * @details The publication becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions and publications.
 *
 * @param fed The federate object in which to create a publication.
 * @param key The identifier for the publication the global publication key will be prepended with the federate name.
 * @param type A code identifying the type of the input see /ref helics_data_type for available options.
 * @param units A string listing the units of the subscription (may be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the publication.
 */
HELICS_EXPORT helics_publication
    helicsFederateRegisterPublication(helics_federate fed, const char* key, helics_data_type type, const char* units, helics_error* err);

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
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the publication.
 */
HELICS_EXPORT helics_publication
    helicsFederateRegisterTypePublication(helics_federate fed, const char* key, const char* type, const char* units, helics_error* err);

/**
 * Register a global named publication with an arbitrary type.
 *
 * @details The publication becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions and publications.
 *
 * @param fed The federate object in which to create a publication.
 * @param key The identifier for the publication.
 * @param type A code identifying the type of the input see /ref helics_data_type for available options.
 * @param units A string listing the units of the subscription (may be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the publication.
 */
HELICS_EXPORT helics_publication helicsFederateRegisterGlobalPublication(helics_federate fed,
                                                                         const char* key,
                                                                         helics_data_type type,
                                                                         const char* units,
                                                                         helics_error* err);

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
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the publication.
 */
HELICS_EXPORT helics_publication helicsFederateRegisterGlobalTypePublication(helics_federate fed,
                                                                             const char* key,
                                                                             const char* type,
                                                                             const char* units,
                                                                             helics_error* err);

/**
 * Register a named input.
 *
 * @details The input becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions, inputs, and publications.
 *
 * @param fed The federate object in which to create an input.
 * @param key The identifier for the publication the global input key will be prepended with the federate name.
 * @param type A code identifying the type of the input see /ref helics_data_type for available options.
 * @param units A string listing the units of the input (may be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the input.
 */
HELICS_EXPORT helics_input
    helicsFederateRegisterInput(helics_federate fed, const char* key, helics_data_type type, const char* units, helics_error* err);

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
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the publication.
 */
HELICS_EXPORT helics_input
    helicsFederateRegisterTypeInput(helics_federate fed, const char* key, const char* type, const char* units, helics_error* err);

/**
 * Register a global named input.
 *
 * @details The publication becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions and publications.
 *
 * @param fed The federate object in which to create a publication.
 * @param key The identifier for the publication.
 * @param type A code identifying the type of the input see /ref helics_data_type for available options.
 * @param units A string listing the units of the subscription maybe NULL.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the publication.
 */
HELICS_EXPORT helics_publication
    helicsFederateRegisterGlobalInput(helics_federate fed, const char* key, helics_data_type type, const char* units, helics_error* err);

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
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the publication.
 */
HELICS_EXPORT helics_publication
    helicsFederateRegisterGlobalTypeInput(helics_federate fed, const char* key, const char* type, const char* units, helics_error* err);

/**
 * Get a publication object from a key.
 *
 * @param fed The value federate object to use to get the publication.
 * @param key The name of the publication.
 * @forcpponly
 * @param[in,out] err The error object to complete if there is an error.
 * @endforcpponly
 *
 * @return A helics_publication object, the object will not be valid and err will contain an error code if no publication with the
 * specified key exists.
 */
HELICS_EXPORT helics_publication helicsFederateGetPublication(helics_federate fed, const char* key, helics_error* err);

/**
 * Get a publication by its index, typically already created via registerInterfaces file or something of that nature.
 *
 * @param fed The federate object in which to create a publication.
 * @param index The index of the publication to get.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A helics_publication.
 */
HELICS_EXPORT helics_publication helicsFederateGetPublicationByIndex(helics_federate fed, int index, helics_error* err);

/**
 * Get an input object from a key.
 *
 * @param fed The value federate object to use to get the publication.
 * @param key The name of the input.
 * @forcpponly
 * @param[in,out] err The error object to complete if there is an error.
 * @endforcpponly
 *
 * @return A helics_input object, the object will not be valid and err will contain an error code if no input with the specified
 * key exists.
 */
HELICS_EXPORT helics_input helicsFederateGetInput(helics_federate fed, const char* key, helics_error* err);

/**
 * Get an input by its index, typically already created via registerInterfaces file or something of that nature.
 *
 * @param fed The federate object in which to create a publication.
 * @param index The index of the publication to get.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A helics_input, which will be NULL if an invalid index.
 */
HELICS_EXPORT helics_input helicsFederateGetInputByIndex(helics_federate fed, int index, helics_error* err);

/**
 * Get an input object from a subscription target.
 *
 * @param fed The value federate object to use to get the publication.
 * @param key The name of the publication that a subscription is targeting.
 * @forcpponly
 * @param[in,out] err The error object to complete if there is an error.
 * @endforcpponly
 *
 * @return A helics_input object, the object will not be valid and err will contain an error code if no input with the specified
 * key exists.
 */
HELICS_EXPORT helics_input helicsFederateGetSubscription(helics_federate fed, const char* key, helics_error* err);

/**
 * Clear all the update flags from a federates inputs.
 *
 * @param fed The value federate object for which to clear update flags.
 */
HELICS_EXPORT void helicsFederateClearUpdates(helics_federate fed);

/**
 * Register the publications via JSON publication string.
 *
 * @param fed The value federate object to use to register the publications.
 * @param json The JSON publication string.
 * @forcpponly
 * @param[in,out] err The error object to complete if there is an error.
 * @endforcpponly
 *
 * @details This would be the same JSON that would be used to publish data.
 */
HELICS_EXPORT void helicsFederateRegisterFromPublicationJSON(helics_federate fed, const char* json, helics_error* err);

/**
 * Publish data contained in a JSON file or string.
 *
 * @param fed The value federate object through which to publish the data.
 * @param json The publication file name or literal JSON data string.
 * @forcpponly
 * @param[in,out] err The error object to complete if there is an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederatePublishJSON(helics_federate fed, const char* json, helics_error* err);

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
 * @return helics_true if the publication is a valid publication.
 */
HELICS_EXPORT helics_bool helicsPublicationIsValid(helics_publication pub);

/**
 * Publish raw data from a char * and length.
 *
 * @param pub The publication to publish for.
 * @param data A pointer to the raw data.
 * @param inputDataLength The size in bytes of the data to publish.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationPublishBytes(helics_publication pub, const void* data, int inputDataLength, helics_error* err);

/**
 * Publish a string.
 *
 * @param pub The publication to publish for.
 * @param str The string to publish.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationPublishString(helics_publication pub, const char* str, helics_error* err);

/**
 * Publish an integer value.
 *
 * @param pub The publication to publish for.
 * @param val The numerical value to publish.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationPublishInteger(helics_publication pub, int64_t val, helics_error* err);

/**
 * Publish a Boolean Value.
 *
 * @param pub The publication to publish for.
 * @param val The boolean value to publish.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationPublishBoolean(helics_publication pub, helics_bool val, helics_error* err);

/**
 * Publish a double floating point value.
 *
 * @param pub The publication to publish for.
 * @param val The numerical value to publish.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationPublishDouble(helics_publication pub, double val, helics_error* err);

/**
 * Publish a time value.
 *
 * @param pub The publication to publish for.
 * @param val The numerical value to publish.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationPublishTime(helics_publication pub, helics_time val, helics_error* err);

/**
 * Publish a single character.
 *
 * @param pub The publication to publish for.
 * @param val The numerical value to publish.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationPublishChar(helics_publication pub, char val, helics_error* err);

/**
 * Publish a complex value (or pair of values).
 *
 * @param pub The publication to publish for.
 * @param real The real part of a complex number to publish.
 * @param imag The imaginary part of a complex number to publish.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationPublishComplex(helics_publication pub, double real, double imag, helics_error* err);

/**
 * Publish a vector of doubles.
 *
 * @param pub The publication to publish for.
 * @param vectorInput A pointer to an array of double data.
 * @forcpponly
 * @param vectorLength The number of points to publish.
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationPublishVector(helics_publication pub, const double* vectorInput, int vectorLength, helics_error* err);

/**
 * Publish a named point.
 *
 * @param pub The publication to publish for.
 * @param str A string for the name to publish.
 * @param val A double for the value to publish.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationPublishNamedPoint(helics_publication pub, const char* str, double val, helics_error* err);

/**
 * Add a named input to the list of targets a publication publishes to.
 *
 * @param pub The publication to add the target for.
 * @param target The name of an input that the data should be sent to.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationAddTarget(helics_publication pub, const char* target, helics_error* err);

/**
 * Check if an input is valid.
 *
 * @param ipt The input to check.
 *
 * @return helics_true if the Input object represents a valid input.
 */
HELICS_EXPORT helics_bool helicsInputIsValid(helics_input ipt);

/**
 * Add a publication to the list of data that an input subscribes to.
 *
 * @param ipt The named input to modify.
 * @param target The name of a publication that an input should subscribe to.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputAddTarget(helics_input ipt, const char* target, helics_error* err);

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
HELICS_EXPORT int helicsInputGetByteCount(helics_input ipt);

/**
 * Get the raw data for the latest value of a subscription.
 *
 * @param ipt The input to get the data for.
 * @forcpponly
 * @param[out] data The memory location of the data
 * @param maxDataLength The maximum size of information that data can hold.
 * @param[out] actualSize The actual length of data copied to data.
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @beginPythonOnly
 * @return  raw Bytes of the value, the value is uninterpreted raw bytes.
 * @endPythonOnly
 */
HELICS_EXPORT void helicsInputGetBytes(helics_input ipt, void* data, int maxDataLength, int* actualSize, helics_error* err);

/**
 * Get the size of a value for subscription assuming return as a string.
 *
 * @return The size of the string.
 */
HELICS_EXPORT int helicsInputGetStringSize(helics_input ipt);

/**
 * Get a string value from a subscription.
 *
 * @param ipt The input to get the data for.
 * @forcpponly
 * @param[out] outputString Storage for copying a null terminated string.
 * @param maxStringLength The maximum size of information that str can hold.
 * @param[out] actualLength The actual length of the string.
 * @param[in,out] err Error term for capturing errors.
 * @endforcpponly
 *
 * @beginPythonOnly
 * @return A string data
 * @endPythonOnly
 */
HELICS_EXPORT void helicsInputGetString(helics_input ipt, char* outputString, int maxStringLength, int* actualLength, helics_error* err);

/**
 * Get an integer value from a subscription.
 *
 * @param ipt The input to get the data for.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An int64_t value with the current value of the input.
 */
HELICS_EXPORT int64_t helicsInputGetInteger(helics_input ipt, helics_error* err);

/**
 * Get a boolean value from a subscription.
 *
 * @param ipt The input to get the data for.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A boolean value of current input value.
 */
HELICS_EXPORT helics_bool helicsInputGetBoolean(helics_input ipt, helics_error* err);

/**
 * Get a double value from a subscription.
 *
 * @param ipt The input to get the data for.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return The double value of the input.
 */
HELICS_EXPORT double helicsInputGetDouble(helics_input ipt, helics_error* err);

/**
 * Get a time value from a subscription.
 *
 * @param ipt The input to get the data for.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return The resulting time value.
 */
HELICS_EXPORT helics_time helicsInputGetTime(helics_input ipt, helics_error* err);

/**
 * Get a single character value from an input.
 *
 * @param ipt The input to get the data for.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return The resulting character value.
 * @forcpponly
 *         NAK (negative acknowledgment) symbol returned on error
 * @endforcpponly
 */
HELICS_EXPORT char helicsInputGetChar(helics_input ipt, helics_error* err);

/**
 * Get a complex object from an input object.
 *
 * @param ipt The input to get the data for.
 * @forcpponly
 * @param[in,out] err A helics error object, if the object is not empty the function is bypassed otherwise it is filled in if there is an
 * error.
 * @endforcpponly
 *
 * @return A HelicsComplex structure with the value.
 */
HELICS_EXPORT helics_complex helicsInputGetComplexObject(helics_input ipt, helics_error* err);

/**
 * Get a pair of double forming a complex number from a subscriptions.
 *
 * @param ipt The input to get the data for.
 * @forcpponly
 * @param[out] real Memory location to place the real part of a value.
 * @param[out] imag Memory location to place the imaginary part of a value.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * On error the values will not be altered.
 * @endforcpponly
 *
 * @beginPythonOnly
 * @return a pair of floating point values that represent the real and imag values
 * @endPythonOnly
 */
HELICS_EXPORT void helicsInputGetComplex(helics_input ipt, double* real, double* imag, helics_error* err);

/**
 * Get the size of a value for subscription assuming return as an array of doubles.
 *
 * @return The number of doubles in a returned vector.
 */
HELICS_EXPORT int helicsInputGetVectorSize(helics_input ipt);

/**
 * Get a vector from a subscription.
 *
 * @param ipt The input to get the result for.
 * @forcpponly
 * @param[out] data The location to store the data.
 * @param maxLength The maximum size of the vector.
 * @param[out] actualSize Location to place the actual length of the resulting vector.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @beginPythonOnly
 * @return a list of floating point values
 * @endPythonOnly
 */
HELICS_EXPORT void helicsInputGetVector(helics_input ipt, double data[], int maxLength, int* actualSize, helics_error* err);

/**
 * Get a named point from a subscription.
 *
 * @param ipt The input to get the result for.
 * @forcpponly
 * @param[out] outputString Storage for copying a null terminated string.
 * @param maxStringLength The maximum size of information that str can hold.
 * @param[out] actualLength The actual length of the string
 * @param[out] val The double value for the named point.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @beginPythonOnly
 * @return a string and a double value for the named point
 * @endPythonOnly
 */
HELICS_EXPORT void
    helicsInputGetNamedPoint(helics_input ipt, char* outputString, int maxStringLength, int* actualLength, double* val, helics_error* err);

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
 * @forcpponly
 * @param inputDataLength The size of the raw data.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetDefaultBytes(helics_input ipt, const void* data, int inputDataLength, helics_error* err);

/**
 * Set the default as a string.
 *
 * @param ipt The input to set the default for.
 * @param str A pointer to the default string.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetDefaultString(helics_input ipt, const char* str, helics_error* err);

/**
 * Set the default as an integer.
 *
 * @param ipt The input to set the default for.
 * @param val The default integer.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetDefaultInteger(helics_input ipt, int64_t val, helics_error* err);

/**
 * Set the default as a boolean.
 *
 * @param ipt The input to set the default for.
 * @param val The default boolean value.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetDefaultBoolean(helics_input ipt, helics_bool val, helics_error* err);

/**
 * Set the default as a time.
 *
 * @param ipt The input to set the default for.
 * @param val The default time value.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetDefaultTime(helics_input ipt, helics_time val, helics_error* err);

/**
 * Set the default as a char.
 *
 * @param ipt The input to set the default for.
 * @param val The default char value.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetDefaultChar(helics_input ipt, char val, helics_error* err);

/**
 * Set the default as a double.
 *
 * @param ipt The input to set the default for.
 * @param val The default double value.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetDefaultDouble(helics_input ipt, double val, helics_error* err);

/**
 * Set the default as a complex number.
 *
 * @param ipt The input to set the default for.
 * @param real The default real value.
 * @param imag The default imaginary value.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetDefaultComplex(helics_input ipt, double real, double imag, helics_error* err);

/**
 * Set the default as a vector of doubles.
 *
 * @param ipt The input to set the default for.
 * @param vectorInput A pointer to an array of double data.
 * @param vectorLength The number of points to publish.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetDefaultVector(helics_input ipt, const double* vectorInput, int vectorLength, helics_error* err);

/**
 * Set the default as a NamedPoint.
 *
 * @param ipt The input to set the default for.
 * @param str A pointer to a string representing the name.
 * @param val A double value for the value of the named point.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetDefaultNamedPoint(helics_input ipt, const char* str, double val, helics_error* err);

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
 * @return A void enumeration, helics_ok if everything worked.
 */
HELICS_EXPORT const char* helicsInputGetType(helics_input ipt);

/**
 * Get the type the publisher to an input is sending.
 *
 * @param ipt The input to query.
 *
 * @return A const char * with the type name.
 */
HELICS_EXPORT const char* helicsInputGetPublicationType(helics_input ipt);

/**
 * Get the type of a publication.
 *
 * @param pub The publication to query.
 *
 * @return A void enumeration, helics_ok if everything worked.
 */
HELICS_EXPORT const char* helicsPublicationGetType(helics_publication pub);

/**
 * Get the key of an input.
 *
 * @param ipt The input to query.
 *
 * @return A void enumeration, helics_ok if everything worked.
 */
HELICS_EXPORT const char* helicsInputGetKey(helics_input ipt);

/**
 * Get the key of a subscription.
 *
 * @return A const char with the subscription key.
 */
HELICS_EXPORT const char* helicsSubscriptionGetKey(helics_input ipt);

/**
 * Get the key of a publication.
 *
 * @details This will be the global key used to identify the publication to the federation.
 *
 * @param pub The publication to query.
 *
 * @return A void enumeration, helics_ok if everything worked.
 */
HELICS_EXPORT const char* helicsPublicationGetKey(helics_publication pub);

/**
 * Get the units of an input.
 *
 * @param ipt The input to query.
 *
 * @return A void enumeration, helics_ok if everything worked.
 */
HELICS_EXPORT const char* helicsInputGetUnits(helics_input ipt);

/**
 * Get the units of the publication that an input is linked to.
 *
 * @param ipt The input to query.
 *
 * @return A void enumeration, helics_ok if everything worked.
 */
HELICS_EXPORT const char* helicsInputGetInjectionUnits(helics_input ipt);

/**
 * Get the units of an input.
 *
 * @details The same as helicsInputGetUnits.
 *
 * @param ipt The input to query.
 *
 * @return A void enumeration, helics_ok if everything worked.
 */
HELICS_EXPORT const char* helicsInputGetExtractionUnits(helics_input ipt);

/**
 * Get the units of a publication.
 *
 * @param pub The publication to query.
 *
 * @return A void enumeration, helics_ok if everything worked.
 */
HELICS_EXPORT const char* helicsPublicationGetUnits(helics_publication pub);

/**
 * Get the data in the info field of an input.
 *
 * @param inp The input to query.
 *
 * @return A string with the info field string.
 */
HELICS_EXPORT const char* helicsInputGetInfo(helics_input inp);

/**
 * Set the data in the info field for an input.
 *
 * @param inp The input to query.
 * @param info The string to set.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetInfo(helics_input inp, const char* info, helics_error* err);

/**
 * Get the data in the info field of an publication.
 *
 * @param pub The publication to query.
 *
 * @return A string with the info field string.
 */
HELICS_EXPORT const char* helicsPublicationGetInfo(helics_publication pub);

/**
 * Set the data in the info field for a publication.
 *
 * @param pub The publication to set the info field for.
 * @param info The string to set.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationSetInfo(helics_publication pub, const char* info, helics_error* err);

/**
 * Get the current value of an input handle option
 *
 * @param inp The input to query.
 * @param option Integer representation of the option in question see /ref helics_handle_options.
 *
 * @return An integer value with the current value of the given option.
 */
HELICS_EXPORT int helicsInputGetOption(helics_input inp, int option);

/**
 * Set an option on an input
 *
 * @param inp The input to query.
 * @param option The option to set for the input /ref helics_handle_options.
 * @param value The value to set the option to.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetOption(helics_input inp, int option, int value, helics_error* err);

/**
 * Get the value of an option for a publication
 *
 * @param pub The publication to query.
 * @param option The value to query see /ref helics_handle_options.
 *
 * @return A string with the info field string.
 */
HELICS_EXPORT int helicsPublicationGetOption(helics_publication pub, int option);

/**
 * Set the value of an option for a publication
 *
 * @param pub The publication to query.
 * @param option Integer code for the option to set /ref helics_handle_options.
 * @param val The value to set the option to.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationSetOption(helics_publication pub, int option, int val, helics_error* err);

/**
 * Set the minimum change detection tolerance.
 *
 * @param pub The publication to modify.
 * @param tolerance The tolerance level for publication, values changing less than this value will not be published.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationSetMinimumChange(helics_publication pub, double tolerance, helics_error* err);

/**
 * Set the minimum change detection tolerance.
 *
 * @param inp The input to modify.
 * @param tolerance The tolerance level for registering an update, values changing less than this value will not show as being updated.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetMinimumChange(helics_input inp, double tolerance, helics_error* err);

/**@}*/

/**
 * Check if a particular subscription was updated.
 *
 * @return helics_true if it has been updated since the last value retrieval.
 */
HELICS_EXPORT helics_bool helicsInputIsUpdated(helics_input ipt);

/**
 * Get the last time a subscription was updated.
 */
HELICS_EXPORT helics_time helicsInputLastUpdateTime(helics_input ipt);

/**
 * Clear the updated flag from an input.
 */
HELICS_EXPORT void helicsInputClearUpdate(helics_input ipt);

/**
 * Get the number of publications in a federate.
 *
 * @return (-1) if fed was not a valid federate otherwise returns the number of publications.
 */
HELICS_EXPORT int helicsFederateGetPublicationCount(helics_federate fed);

/**
 * Get the number of subscriptions in a federate.
 *
 * @return (-1) if fed was not a valid federate otherwise returns the number of subscriptions.
 */
HELICS_EXPORT int helicsFederateGetInputCount(helics_federate fed);

/* HELICS_APISHARED_VALUE_FEDERATE_FUNCTIONS_H_*/
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
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the endpoint.
 * @forcpponly
 *         nullptr on failure.
 * @endforcpponly
 */
HELICS_EXPORT helics_endpoint helicsFederateRegisterEndpoint(helics_federate fed, const char* name, const char* type, helics_error* err);

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
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 * @return An object containing the endpoint.
 * @forcpponly
 *         nullptr on failure.
 * @endforcpponly
 */
HELICS_EXPORT helics_endpoint helicsFederateRegisterGlobalEndpoint(helics_federate fed,
                                                                   const char* name,
                                                                   const char* type,
                                                                   helics_error* err);

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
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the endpoint.
 * @forcpponly
 *         nullptr on failure.
 * @endforcpponly
 */
HELICS_EXPORT helics_endpoint helicsFederateRegisterTargetedEndpoint(helics_federate fed,
                                                                     const char* name,
                                                                     const char* type,
                                                                     helics_error* err);

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
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 * @return An object containing the endpoint.
 * @forcpponly
 *         nullptr on failure.
 * @endforcpponly
 */
HELICS_EXPORT helics_endpoint helicsFederateRegisterGlobalTargetedEndpoint(helics_federate fed,
                                                                           const char* name,
                                                                           const char* type,
                                                                           helics_error* err);

/**
 * Get an endpoint object from a name.
 *
 * @param fed The message federate object to use to get the endpoint.
 * @param name The name of the endpoint.
 * @forcpponly
 * @param[in,out] err The error object to complete if there is an error.
 * @endforcpponly
 *
 * @return A helics_endpoint object.
 * @forcpponly
 *         The object will not be valid and err will contain an error code if no endpoint with the specified name exists.
 * @endforcpponly
 */
HELICS_EXPORT helics_endpoint helicsFederateGetEndpoint(helics_federate fed, const char* name, helics_error* err);

/**
 * Get an endpoint by its index, typically already created via registerInterfaces file or something of that nature.
 *
 * @param fed The federate object in which to create a publication.
 * @param index The index of the publication to get.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A helics_endpoint.
 * @forcpponly
 *         It will be NULL if given an invalid index.
 * @endforcpponly
 */
HELICS_EXPORT helics_endpoint helicsFederateGetEndpointByIndex(helics_federate fed, int index, helics_error* err);

/**
 * Check if an endpoint is valid.
 *
 * @param endpoint The endpoint object to check.
 *
 * @return helics_true if the Endpoint object represents a valid endpoint.
 */
HELICS_EXPORT helics_bool helicsEndpointIsValid(helics_endpoint endpoint);

/**
 * Set the default destination for an endpoint if no other endpoint is given.
 *
 * @param endpoint The endpoint to set the destination for.
 * @param dst A string naming the desired default endpoint.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsEndpointSetDefaultDestination(helics_endpoint endpoint, const char* dst, helics_error* err);

/**
 * Get the default destination for an endpoint.
 *
 * @param endpoint The endpoint to set the destination for.
 *
 * @return A string with the default destination.
 */
HELICS_EXPORT const char* helicsEndpointGetDefaultDestination(helics_endpoint endpoint);

/**
 * Send a message to the targeted destination.
 *
 * @param endpoint The endpoint to send the data from.
 * @param data The data to send.
 * @forcpponly
 * @param inputDataLength The length of the data to send.
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsEndpointSendBytes(helics_endpoint endpoint, const void* data, int inputDataLength, helics_error* err);

/**
 * Send a message to the specified destination.
 *
 * @param endpoint The endpoint to send the data from.

 * @param data The data to send.
 * @forcpponly
 * @param inputDataLength The length of the data to send.
 * @param dst The target destination.
 * @forcpponly
 *             nullptr to use the default destination.
 * @endforcpponly
 * @beginpythononly
 *             "" to use the default destination.
 * @endpythononly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void
    helicsEndpointSendBytesTo(helics_endpoint endpoint, const void* data, int inputDataLength, const char* dst, helics_error* err);

/**
 * Send a message to the specified destination at a specific time.
 *
 * @param endpoint The endpoint to send the data from.
 * @param data The data to send.
 * @forcpponly
 * @param inputDataLength The length of the data to send.
 * @endforcpponly
 * @param dst The target destination.
 * @forcpponly
 *             nullptr to use the default destination.
 * @endforcpponly
 * @beginpythononly
 *             "" to use the default destination.
 * @endpythononly
 * @param time The time the message should be sent.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */

HELICS_EXPORT void helicsEndpointSendBytesToAt(helics_endpoint endpoint,
                                               const void* data,
                                               int inputDataLength,
                                               const char* dst,
                                               helics_time time,
                                               helics_error* err);

/**
 * Send a message at a specific time to the targeted destinations
 *
 * @param endpoint The endpoint to send the data from.
 *
 * @param data The data to send.
 * @forcpponly
 * @param inputDataLength The length of the data to send.
 * @endforcpponly
  @param time The time the message should be sent.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */

HELICS_EXPORT void
    helicsEndpointSendBytesAt(helics_endpoint endpoint, const void* data, int inputDataLength, helics_time time, helics_error* err);

/**
 * Send a message object from a specific endpoint.
 *
 * @param endpoint The endpoint to send the data from.
 * @param message The actual message to send which will be copied.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsEndpointSendMessage(helics_endpoint endpoint, helics_message message, helics_error* err);

/**
 * Send a message object from a specific endpoint, the message will not be copied and the message object will no longer be valid
 * after the call.
 *
 * @param endpoint The endpoint to send the data from.
 * @param message The actual message to send which will be copied.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsEndpointSendMessageZeroCopy(helics_endpoint endpoint, helics_message message, helics_error* err);

/**
 * Subscribe an endpoint to a publication.
 *
 * @param endpoint The endpoint to use.
 * @param key The name of the publication.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsEndpointSubscribe(helics_endpoint endpoint, const char* key, helics_error* err);

/**
 * Check if the federate has any outstanding messages.
 *
 * @param fed The federate to check.
 *
 * @return helics_true if the federate has a message waiting, helics_false otherwise.
 */
HELICS_EXPORT helics_bool helicsFederateHasMessage(helics_federate fed);

/**
 * Check if a given endpoint has any unread messages.
 *
 * @param endpoint The endpoint to check.
 *
 * @return helics_true if the endpoint has a message, helics_false otherwise.
 */
HELICS_EXPORT helics_bool helicsEndpointHasMessage(helics_endpoint endpoint);

/**
 * Returns the number of pending receives for the specified destination endpoint.
 *
 * @param fed The federate to get the number of waiting messages from.
 */
HELICS_EXPORT int helicsFederatePendingMessagesCount(helics_federate fed);

/**
 * Returns the number of pending receives for all endpoints of a particular federate.
 *
 * @param endpoint The endpoint to query.
 */
HELICS_EXPORT int helicsEndpointPendingMessagesCount(helics_endpoint endpoint);

/**
 * Receive a packet from a particular endpoint.
 *
 * @param[in] endpoint The identifier for the endpoint.
 *
 * @return A message object.
 */
HELICS_EXPORT helics_message helicsEndpointGetMessage(helics_endpoint endpoint);

/**
 * Create a new empty message object.
 *
 * @details The message is empty and isValid will return false since there is no data associated with the message yet.
 *
 * @param endpoint The endpoint object to associate the message with.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 *
 * @return A new helics_message.
 */
HELICS_EXPORT helics_message helicsEndpointCreateMessage(helics_endpoint endpoint, helics_error* err);

/**
 * Receive a communication message for any endpoint in the federate.
 *
 * @details The return order will be in order of endpoint creation.
 *          So all messages that are available for the first endpoint, then all for the second, and so on.
 *          Within a single endpoint, the messages are ordered by time, then source_id, then order of arrival.
 *
 * @return A helics_message which references the data in the message.
 */
HELICS_EXPORT helics_message helicsFederateGetMessage(helics_federate fed);

/**
 * Create a new empty message object.
 *
 * @details The message is empty and isValid will return false since there is no data associated with the message yet.
 *
 * @param fed the federate object to associate the message with
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 *
 * @return A helics_message containing the message data.
 */
HELICS_EXPORT helics_message helicsFederateCreateMessage(helics_federate fed, helics_error* err);

/**
 * Clear all stored messages from a federate.
 *
 * @details This clears messages retrieved through helicsEndpointGetMessage or helicsFederateGetMessage
 *
 * @param fed The federate to clear the message for.
 */
HELICS_EXPORT void helicsFederateClearMessages(helics_federate fed);

/**
 * Get the type specified for an endpoint.
 *
 * @param endpoint The endpoint object in question.
 *
 * @return The defined type of the endpoint.
 */
HELICS_EXPORT const char* helicsEndpointGetType(helics_endpoint endpoint);

/**
 * Get the name of an endpoint.
 *
 * @param endpoint The endpoint object in question.
 *
 * @return The name of the endpoint.
 */
HELICS_EXPORT const char* helicsEndpointGetName(helics_endpoint endpoint);

/**
 * Get the number of endpoints in a federate.
 *
 * @param fed The message federate to query.
 *
 * @return (-1) if fed was not a valid federate, otherwise returns the number of endpoints.
 */
HELICS_EXPORT int helicsFederateGetEndpointCount(helics_federate fed);

/**
 * Get the data in the info field of a filter.
 *
 * @param end The filter to query.
 *
 * @return A string with the info field string.
 */
HELICS_EXPORT const char* helicsEndpointGetInfo(helics_endpoint end);

/**
 * Set the data in the info field for a filter.
 *
 * @param end The endpoint to query.
 * @param info The string to set.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsEndpointSetInfo(helics_endpoint endpoint, const char* info, helics_error* err);

/**
 * Set a handle option on an endpoint.
 *
 * @param endpoint The endpoint to modify.
 * @param option Integer code for the option to set /ref helics_handle_options.
 * @param value The value to set the option to.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsEndpointSetOption(helics_endpoint endpoint, int option, int value, helics_error* err);

/**
 * Set a handle option on an endpoint.
 *
 * @param endpoint The endpoint to modify.
 * @param option Integer code for the option to set /ref helics_handle_options.
 * @return the value of the option, for boolean options will be 0 or 1
 */
HELICS_EXPORT int helicsEndpointGetOption(helics_endpoint endpoint, int option);

/**
 * add a source target to an endpoint,  Specifying an endpoint to receive undirected messages from
 *
 * @param endpoint The endpoint to modify.
 * @param targetEndpoint the endpoint to get messages from
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsEndpointAddSourceTarget(helics_endpoint endpoint, const char* targetEndpoint, helics_error* err);

/**
 * add a destination target to an endpoint,  Specifying an endpoint to send undirected messages to
 *
 * @param endpoint The endpoint to modify.
 * @param targetEndpoint the name of the endpoint to send messages to
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsEndpointAddDestinationTarget(helics_endpoint endpoint, const char* targetEndpoint, helics_error* err);

/**
 * remove an endpoint from being targeted
 *
 * @param endpoint The endpoint to modify.
 * @param targetEndpoint the name of the endpoint to send messages to
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsEndpointRemoveTarget(helics_endpoint endpoint, const char* targetEndpoint, helics_error* err);

/**
 * add a source Filter to an endpoint
 *
 * @param endpoint The endpoint to modify.
 * @param filterName the name of the filter to add
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsEndpointAddSourceFilter(helics_endpoint endpoint, const char* filterName, helics_error* err);

/**
 * add a destination filter to an endpoint
 *
 * @param endpoint The endpoint to modify.
 * @param targetEndpoint the name of the filter to add
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsEndpointAddDestinationFilter(helics_endpoint endpoint, const char* filterName, helics_error* err);

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
HELICS_EXPORT const char* helicsMessageGetSource(helics_message message);

/**
 * Get the destination endpoint of a message.
 *
 * @param message The message object in question.
 *
 * @return A string with the destination endpoint.
 */
HELICS_EXPORT const char* helicsMessageGetDestination(helics_message message);

/**
 * Get the original source endpoint of a message, the source may have been modified by filters or other actions.
 *
 * @param message The message object in question.
 *
 * @return A string with the source of a message.
 */
HELICS_EXPORT const char* helicsMessageGetOriginalSource(helics_message message);

/**
 * Get the original destination endpoint of a message, the destination may have been modified by filters or other actions.
 *
 * @param message The message object in question.
 *
 * @return A string with the original destination of a message.
 */
HELICS_EXPORT const char* helicsMessageGetOriginalDestination(helics_message message);

/**
 * Get the helics time associated with a message.
 *
 * @param message The message object in question.
 *
 * @return The time associated with a message.
 */
HELICS_EXPORT helics_time helicsMessageGetTime(helics_message message);

/**
 * Get the payload of a message as a string.
 *
 * @param message The message object in question.
 *
 * @return A string representing the payload of a message.
 */
HELICS_EXPORT const char* helicsMessageGetString(helics_message message);

/**
 * Get the messageID of a message.
 *
 * @param message The message object in question.
 *
 * @return The messageID.
 */
HELICS_EXPORT int helicsMessageGetMessageID(helics_message message);

/**
 * Check if a flag is set on a message.
 *
 * @param message The message object in question.
 * @param flag The flag to check should be between [0,15].
 *
 * @return The flags associated with a message.
 */
HELICS_EXPORT helics_bool helicsMessageGetFlagOption(helics_message message, int flag);

/**
 * Get the size of the data payload in bytes.
 *
 * @param message The message object in question.
 *
 * @return The size of the data payload.
 */
HELICS_EXPORT int helicsMessageGetByteCount(helics_message message);

/**
 * Get the raw data for a message object.
 *
 * @param message A message object to get the data for.
 * @forcpponly
 * @param[out] data The memory location of the data.
 * @param maxMessageLength The maximum size of information that data can hold.
 * @param[out] actualSize The actual length of data copied to data.
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @beginPythonOnly
 * @return Raw string data.
 * @endPythonOnly
 */
HELICS_EXPORT void helicsMessageGetBytes(helics_message message, void* data, int maxMessageLength, int* actualSize, helics_error* err);

/**
 * Get a pointer to the raw data of a message.
 *
 * @param message A message object to get the data for.
 *
 * @return A pointer to the raw data in memory, the pointer may be NULL if the message is not a valid message.
 */
HELICS_EXPORT void* helicsMessageGetBytesPointer(helics_message message);

/**
 * A check if the message contains a valid payload.
 *
 * @param message The message object in question.
 *
 * @return helics_true if the message contains a payload.
 */
HELICS_EXPORT helics_bool helicsMessageIsValid(helics_message message);

/**
 * Set the source of a message.
 *
 * @param message The message object in question.
 * @param src A string containing the source.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageSetSource(helics_message message, const char* src, helics_error* err);

/**
 * Set the destination of a message.
 *
 * @param message The message object in question.
 * @param dst A string containing the new destination.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageSetDestination(helics_message message, const char* dst, helics_error* err);

/**
 * Set the original source of a message.
 *
 * @param message The message object in question.
 * @param src A string containing the new original source.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageSetOriginalSource(helics_message message, const char* src, helics_error* err);

/**
 * Set the original destination of a message.
 *
 * @param message The message object in question.
 * @param dst A string containing the new original source.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageSetOriginalDestination(helics_message message, const char* dst, helics_error* err);

/**
 * Set the delivery time for a message.
 *
 * @param message The message object in question.
 * @param time The time the message should be delivered.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageSetTime(helics_message message, helics_time time, helics_error* err);

/**
 * Resize the data buffer for a message.
 *
 * @details The message data buffer will be resized. There are no guarantees on what is in the buffer in newly allocated space.
 *          If the allocated space is not sufficient new allocations will occur.
 *
 * @param message The message object in question.
 * @param newSize The new size in bytes of the buffer.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageResize(helics_message message, int newSize, helics_error* err);

/**
 * Reserve space in a buffer but don't actually resize.
 *
 * @details The message data buffer will be reserved but not resized.
 *
 * @param message The message object in question.
 * @param reserveSize The number of bytes to reserve in the message object.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageReserve(helics_message message, int reserveSize, helics_error* err);

/**
 * Set the message ID for the message.
 *
 * @details Normally this is not needed and the core of HELICS will adjust as needed.
 *
 * @param message The message object in question.
 * @param messageID A new message ID.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageSetMessageID(helics_message message, int32_t messageID, helics_error* err);

/**
 * Clear the flags of a message.
 *
 * @param message The message object in question
 */
HELICS_EXPORT void helicsMessageClearFlags(helics_message message);

/**
 * Set a flag on a message.
 *
 * @param message The message object in question.
 * @param flag An index of a flag to set on the message.
 * @param flagValue The desired value of the flag.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageSetFlagOption(helics_message message, int flag, helics_bool flagValue, helics_error* err);

/**
 * Set the data payload of a message as a string.
 *
 * @param message The message object in question.
 * @param str A string containing the message data.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageSetString(helics_message message, const char* str, helics_error* err);

/**
 * Set the data payload of a message as raw data.
 *
 * @param message The message object in question.
 * @param data A string containing the message data.
 * @param inputDataLength The length of the data to input.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageSetData(helics_message message, const void* data, int inputDataLength, helics_error* err);

/**
 * Append data to the payload.
 *
 * @param message The message object in question.
 * @param data A string containing the message data to append.
 * @param inputDataLength The length of the data to input.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageAppendData(helics_message message, const void* data, int inputDataLength, helics_error* err);

/**
 * Copy a message object.
 *
 * @param src_message The message object to copy from.
 * @param dst_message The message object to copy to.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageCopy(helics_message src_message, helics_message dst_message, helics_error* err);

/**
 * Clone a message object.
 *
 * @param message The message object to copy from.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT helics_message helicsMessageClone(helics_message message, helics_error* err);

/**
 * Free a message object from memory
 * @param message The message object to copy from.
 * @details memory for message is managed so not using this function does not create memory leaks, this is an indication
 * to the system that the memory for this message is done being used and can be reused for a new message.
 * helicsFederateClearMessages() can also be used to clear up all stored messages at once
 */
HELICS_EXPORT void helicsMessageFree(helics_message message);

/**
 * Reset a message to empty state
 * @param message The message object to copy from.
 * @details The message after this function will be empty, with no source or destination
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageClear(helics_message message, helics_error* err);

/**@}*/

/*HELICS_APISHARED_MESSAGE_FEDERATE_FUNCTIONS_H_*/
/**
 * Create a source Filter on the specified federate.
 *
 * @details Filters can be created through a federate or a core, linking through a federate allows
 *          a few extra features of name matching to function on the federate interface but otherwise equivalent behavior
 *
 * @param fed The federate to register through.
 * @param type The type of filter to create /ref helics_filter_type.
 * @param name The name of the filter (can be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A helics_filter object.
 */
HELICS_EXPORT helics_filter helicsFederateRegisterFilter(helics_federate fed, helics_filter_type type, const char* name, helics_error* err);
/**
 * Create a global source filter through a federate.
 *
 * @details Filters can be created through a federate or a core, linking through a federate allows
 *          a few extra features of name matching to function on the federate interface but otherwise equivalent behavior.
 *
 * @param fed The federate to register through.
 * @param type The type of filter to create /ref helics_filter_type.
 * @param name The name of the filter (can be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A helics_filter object.
 */
HELICS_EXPORT helics_filter helicsFederateRegisterGlobalFilter(helics_federate fed,
                                                               helics_filter_type type,
                                                               const char* name,
                                                               helics_error* err);

/**
 * Create a cloning Filter on the specified federate.
 *
 * @details Cloning filters copy a message and send it to multiple locations, source and destination can be added
 *          through other functions.
 *
 * @param fed The federate to register through.
 * @param name The name of the filter (can be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A helics_filter object.
 */
HELICS_EXPORT helics_filter helicsFederateRegisterCloningFilter(helics_federate fed, const char* name, helics_error* err);

/**
 * Create a global cloning Filter on the specified federate.
 *
 * @details Cloning filters copy a message and send it to multiple locations, source and destination can be added
 *          through other functions.
 *
 * @param fed The federate to register through.
 * @param name The name of the filter (can be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A helics_filter object.
 */
HELICS_EXPORT helics_filter helicsFederateRegisterGlobalCloningFilter(helics_federate fed, const char* name, helics_error* err);

/**
 * Create a source Filter on the specified core.
 *
 * @details Filters can be created through a federate or a core, linking through a federate allows
 *          a few extra features of name matching to function on the federate interface but otherwise equivalent behavior.
 *
 * @param core The core to register through.
 * @param type The type of filter to create /ref helics_filter_type.
 * @param name The name of the filter (can be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A helics_filter object.
 */
HELICS_EXPORT helics_filter helicsCoreRegisterFilter(helics_core core, helics_filter_type type, const char* name, helics_error* err);

/**
 * Create a cloning Filter on the specified core.
 *
 * @details Cloning filters copy a message and send it to multiple locations, source and destination can be added
 *          through other functions.
 *
 * @param core The core to register through.
 * @param name The name of the filter (can be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A helics_filter object.
 */
HELICS_EXPORT helics_filter helicsCoreRegisterCloningFilter(helics_core core, const char* name, helics_error* err);

/**
 * Get the number of filters registered through a federate.
 *
 * @param fed The federate object to use to get the filter.
 *
 * @return A count of the number of filters registered through a federate.
 */
HELICS_EXPORT int helicsFederateGetFilterCount(helics_federate fed);

/**
 * Get a filter by its name, typically already created via registerInterfaces file or something of that nature.
 *
 * @param fed The federate object to use to get the filter.
 * @param name The name of the filter.
 * @forcpponly
 * @param[in,out] err The error object to complete if there is an error.
 * @endforcpponly
 *
 * @return A helics_filter object, the object will not be valid and err will contain an error code if no filter with the specified name
 * exists.
 */
HELICS_EXPORT helics_filter helicsFederateGetFilter(helics_federate fed, const char* name, helics_error* err);
/**
 * Get a filter by its index, typically already created via registerInterfaces file or something of that nature.
 *
 * @param fed The federate object in which to create a publication.
 * @param index The index of the publication to get.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A helics_filter, which will be NULL if an invalid index is given.
 */
HELICS_EXPORT helics_filter helicsFederateGetFilterByIndex(helics_federate fed, int index, helics_error* err);

/**
 * Check if a filter is valid.
 *
 * @param filt The filter object to check.
 *
 * @return helics_true if the Filter object represents a valid filter.
 */
HELICS_EXPORT helics_bool helicsFilterIsValid(helics_filter filt);

/**
 * Get the name of the filter and store in the given string.
 *
 * @param filt The given filter.
 *
 * @return A string with the name of the filter.
 */
HELICS_EXPORT const char* helicsFilterGetName(helics_filter filt);

/**
 * Set a property on a filter.
 *
 * @param filt The filter to modify.
 * @param prop A string containing the property to set.
 * @param val A numerical value for the property.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFilterSet(helics_filter filt, const char* prop, double val, helics_error* err);

/**
 * Set a string property on a filter.
 *
 * @param filt The filter to modify.
 * @param prop A string containing the property to set.
 * @param val A string containing the new value.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFilterSetString(helics_filter filt, const char* prop, const char* val, helics_error* err);

/**
 * Add a destination target to a filter.
 *
 * @details All messages going to a destination are copied to the delivery address(es).
 * @param filt The given filter to add a destination target to.
 * @param dst The name of the endpoint to add as a destination target.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFilterAddDestinationTarget(helics_filter filt, const char* dst, helics_error* err);

/**
 * Add a source target to a filter.
 *
 * @details All messages coming from a source are copied to the delivery address(es).
 *
 * @param filt The given filter.
 * @param source The name of the endpoint to add as a source target.
 * @forcpponly.
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFilterAddSourceTarget(helics_filter filt, const char* source, helics_error* err);

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
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFilterAddDeliveryEndpoint(helics_filter filt, const char* deliveryEndpoint, helics_error* err);

/**
 * Remove a destination target from a filter.
 *
 * @param filt The given filter.
 * @param target The named endpoint to remove as a target.
 * @forcpponly
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFilterRemoveTarget(helics_filter filt, const char* target, helics_error* err);

/**
 * Remove a delivery destination from a cloning filter.
 *
 * @param filt The given filter (must be a cloning filter).
 * @param deliveryEndpoint A string with the delivery endpoint to remove.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFilterRemoveDeliveryEndpoint(helics_filter filt, const char* deliveryEndpoint, helics_error* err);

/**
 * Get the data in the info field of a filter.
 *
 * @param filt The given filter.
 *
 * @return A string with the info field string.
 */
HELICS_EXPORT const char* helicsFilterGetInfo(helics_filter filt);
/**
 * Set the data in the info field for a filter.
 *
 * @param filt The given filter.
 * @param info The string to set.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFilterSetInfo(helics_filter filt, const char* info, helics_error* err);

/**
 * Set the data in the info field for a filter.
 *
 * @param filt The given filter.
 * @param option The option to set /ref helics_handle_options.
 * @param value The value of the option commonly 0 for false 1 for true.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */

HELICS_EXPORT void helicsFilterSetOption(helics_filter filt, int option, int value, helics_error* err);

/**
 * Get a handle option for the filter.
 *
 * @param filt The given filter to query.
 * @param option The option to query /ref helics_handle_options.
 */
HELICS_EXPORT int helicsFilterGetOption(helics_filter filt, int option);

/**
 * @}
 */

/* HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_*/
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
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsBrokerSetLoggingCallback(helics_broker broker,
                                                  void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
                                                  void* userdata,
                                                  helics_error* err);

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
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsCoreSetLoggingCallback(helics_core core,
                                                void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
                                                void* userdata,
                                                helics_error* err);

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
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void
    helicsFederateSetLoggingCallback(helics_federate fed,
                                     void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
                                     void* userdata,
                                     helics_error* err);

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
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFilterSetCustomCallback(helics_filter filter,
                                                 void (*filtCall)(helics_message message, void* userData),
                                                 void* userdata,
                                                 helics_error* err);

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
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */

HELICS_EXPORT void
    helicsFederateSetQueryCallback(helics_federate fed,
                                   void (*queryAnswer)(const char* query, int querySize, helics_query_buffer buffer, void* userdata),
                                   void* userdata,
                                   helics_error* err);

/**
 * Set the data for a query callback.
 *
 * @details There are many queries that HELICS understands directly, but it is occasionally useful to have a federate be able to respond
 * to specific queries with answers specific to a federate.
 *
 * @param buffer The buffer received in a helicsQueryCallback.
 * @param str Pointer to the data to fill the buffer with.
 * @param strSize The size of the string.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsQueryBufferFill(helics_query_buffer buffer, const char* str, int strSize, helics_error* err);

#ifdef __cplusplus
} /* end of extern "C" { */
#endif
#endif
