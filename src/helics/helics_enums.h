/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

/*defines left in this code as it is used in the shared library*/
#ifndef _HELICS_ENUMS_
#define _HELICS_ENUMS_
#pragma once

/** @file
@brief base helics enumerations for C and C++ API's
*/

#ifdef __cplusplus
extern "C" {
#endif

/** pick a core type depending on compile configuration usually either ZMQ if available or TCP */
typedef enum {
    helics_core_type_default =
        0, /*!< a default core type that will default to something available*/
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
    helics_core_type_zmq_test =
        10, /*!< single socket version of ZMQ core usually for high fed count on the same system*/
    helics_core_type_nng = 9, /*!< for using the nanomsg communications */
    helics_core_type_tcp_ss =
        11, /*!< a single socket version of the TCP core for more easily handling firewalls*/
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
    helics_error_insufficient_space =
        -18, /*!< insufficient space is available to store requested data */
    helics_error_execution_failure = -14, /*!< the function execution has failed */
    helics_error_invalid_function_call =
        -10, /*!< the call made was invalid in the present state of the calling object */
    helics_error_invalid_state_transition =
        -9, /*!< error issued when an invalid state transition occurred */
    helics_warning = -8, /*!< the function issued a warning of some kind */
    helics_error_system_failure =
        -6, /*!< the federate has terminated unexpectedly and the call cannot be completed */
    helics_error_discard = -5, /*!< the input was discarded and not used for some reason */
    helics_error_invalid_argument =
        -4, /*!< the parameter passed was invalid and unable to be used */
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

#ifdef __cplusplus
} /* end of extern "C" { */
#endif
#endif
