/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
/** @file
@brief base helics enumerations for C++ API's, a namespace wrapper for the definitions defined in
\ref helics_enums.h
*/
#include "helics/helics_enums.h"

namespace helics {
/** @namespace defs enum definitions for use in the C++ api
The defs namespace contains several enumerations for use in functions that take property types or
values or small sets of options*/
namespace defs {
    /** flags that can be used to set different options for a federate*/
    enum Flags : int32_t {
        /** flag indicating that a federate is observe only*/
        OBSERVER = HELICS_FLAG_OBSERVER,
        /** flag indicating that a federate can only return requested times*/
        UNINTERRUPTIBLE = HELICS_FLAG_UNINTERRUPTIBLE,
        /** flag indicating that a federate can be interrupted*/
        INTERRUPTIBLE = HELICS_FLAG_INTERRUPTIBLE,
        /** flag specifying that a federate may be slow to respond to pings
        If the federate goes offline there is no good way to detect it so use with caution
        */
        SLOW_RESPONDING = HELICS_FLAG_SLOW_RESPONDING,
        /** flag indicating that a federate is observe only*/
        REENTRANT = HELICS_FLAG_REENTRANT,
        /** flag specifying that a federate encountering an internal error should cause and abort
         * for the entire co-simulation
         */
        TERMINATE_ON_ERROR = HELICS_FLAG_TERMINATE_ON_ERROR,
        /** flag indicating that a federate/interface is a signal generator only*/
        SOURCE_ONLY = HELICS_FLAG_SOURCE_ONLY,
        /** flag indicating a federate/interface should only transmit values if they have
           changed(binary equivalence)*/
        ONLY_TRANSMIT_ON_CHANGE = HELICS_FLAG_ONLY_TRANSMIT_ON_CHANGE,
        /** flag indicating a federate/interface should only trigger an update if a value has
         * changed (binary equivalence)*/
        ONLY_UPDATE_ON_CHANGE = HELICS_FLAG_ONLY_UPDATE_ON_CHANGE,
        /** flag indicating a federate should only grant time if all other federates have already
         * passed the requested time*/
        WAIT_FOR_CURRENT_TIME_UPDATE = HELICS_FLAG_WAIT_FOR_CURRENT_TIME_UPDATE,
        /** flag indicating a federate should only operate on a restrictive time policy which means
    no second order projections and potentially very slow time advancement on gap conditions. Should
    only be used in selective circumstances*/
        RESTRICTIVE_TIME_POLICY = HELICS_FLAG_RESTRICTIVE_TIME_POLICY,
        /** flag indicating that a federate has rollback capability*/
        ROLLBACK = HELICS_FLAG_ROLLBACK,
        /** flag indicating that a federate performs forward computation and does internal
           rollback*/
        FORWARD_COMPUTE = HELICS_FLAG_FORWARD_COMPUTE,
        /** flag indicating that a federate needs to run in real time*/
        REALTIME = HELICS_FLAG_REALTIME,
        /** flag indicating that the federate will only interact on a single thread*/
        SINGLE_THREAD_FEDERATE = HELICS_FLAG_SINGLE_THREAD_FEDERATE,
        /** used to delay a core from entering initialization mode even if it would otherwise be
           ready*/
        DELAY_INIT_ENTRY = HELICS_FLAG_DELAY_INIT_ENTRY,
        /** used to clear the HELICS_DELAY_INIT_ENTRY flag in cores*/
        ENABLE_INIT_ENTRY = HELICS_FLAG_ENABLE_INIT_ENTRY,
        /** used to not display warnings on mismatched requested times*/
        IGNORE_TIME_MISMATCH_WARNINGS = HELICS_FLAG_IGNORE_TIME_MISMATCH_WARNINGS,
        /** force logging flush*/
        FORCE_LOGGING_FLUSH = HELICS_FLAG_FORCE_LOGGING_FLUSH,
        /** user debugging mode*/
        DEBUGGING = HELICS_FLAG_DEBUGGING,
        /** dump the logs to a file at the end*/
        DUMPLOG = HELICS_FLAG_DUMPLOG,
        /** make all connections required*/
        CONNECTIONS_REQUIRED = HELICS_HANDLE_OPTION_CONNECTION_REQUIRED,
        /** make all connections optional*/
        CONNECTIONS_OPTIONAL = HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL,
        /** make all inputs have strict type checking*/
        STRICT_INPUT_TYPE_CHECKING = HELICS_HANDLE_OPTION_STRICT_TYPE_CHECKING,
        /** be strict about config files*/
        STRICT_CONFIG_CHECKING = HELICS_FLAG_STRICT_CONFIG_CHECKING,
        /** ignore mismatching units*/
        IGNORE_INPUT_UNIT_MISMATCH = HELICS_HANDLE_OPTION_IGNORE_UNIT_MISMATCH,
        /** flag indicating that a federate can only return requested times*/
        EVENT_TRIGGERED = HELICS_FLAG_EVENT_TRIGGERED,
        /** flag indicating that profiling should be active*/
        PROFILING = HELICS_FLAG_PROFILING,
        /** trigger for a profiling marker*/
        PROFILING_MARKER = HELICS_FLAG_PROFILING_MARKER,
        /** flag indicating that profiling should captured to federate log file*/
        LOCAL_PROFILING_CAPTURE = HELICS_FLAG_LOCAL_PROFILING_CAPTURE,
        /** flag indicating the federate is a callback federate */
        CALLBACK_FEDERATE = HELICS_FLAG_CALLBACK_FEDERATE,
        /** flag indicating that the federate/core/broker should allow remote commands affecting
           operations*/
        ALLOW_REMOTE_CONTROL = HELICS_FLAG_ALLOW_REMOTE_CONTROL,
        /** flag indicating that the federate/core/broker should *NOT* allow remote commands
           affecting operations*/
        DISABLE_REMOTE_CONTROL = HELICS_FLAG_DISABLE_REMOTE_CONTROL,
        /** flag indicating that a federate should use json serialization*/
        USE_JSON_SERIALIZATION = HELICS_FLAG_USE_JSON_SERIALIZATION,
        /** flag indicating that a federate should have an automated time request*/
        AUTOMATED_TIME_REQUEST = HELICS_FLAG_AUTOMATED_TIME_REQUEST
    };
    /** potential errors that might be generated by a helics federate/core/broker */
    enum Errors : int32_t {
        /** the function executed successfully */
        OK = HELICS_OK,
        /** registration has failed*/
        REGISTRATION_FAILURE = HELICS_ERROR_REGISTRATION_FAILURE,
        /** the operation to connect has failed*/
        CONNECTION_FAILURE = HELICS_ERROR_CONNECTION_FAILURE,
        /** indicator that the object used was not a valid object */
        INVALID_OBJECT = HELICS_ERROR_INVALID_OBJECT,
        /** the parameter passed was invalid and unable to be used*/
        INVALID_ARGUMENT = HELICS_ERROR_INVALID_ARGUMENT,
        /** the input was discarded and not used for some reason */
        DISCARD = HELICS_ERROR_DISCARD,
        /** the federate has terminated unexpectedly and the call cannot be completed*/
        SYSTEM_FAILURE = HELICS_ERROR_SYSTEM_FAILURE,
        /** the function issued a warning of some kind */
        WARNING = HELICS_WARNING,
        /** error issued when an invalid state transition occurred */
        INVALID_STATE_TRANSITION = HELICS_ERROR_INVALID_STATE_TRANSITION,
        /** the call made was invalid in the present state of the calling object*/
        INVALID_FUNCTION_CALL = HELICS_ERROR_INVALID_FUNCTION_CALL,
        /** the function execution has failed*/
        EXECUTION_FAILURE = HELICS_ERROR_EXECUTION_FAILURE,
        /** the function produced a helics error of some other type */
        OTHER = HELICS_ERROR_OTHER,
    };

    /** integer and time properties that can be set for federates*/
    enum Properties : int32_t {
        TIME_DELTA = HELICS_PROPERTY_TIME_DELTA,
        PERIOD = HELICS_PROPERTY_TIME_PERIOD,
        OFFSET = HELICS_PROPERTY_TIME_OFFSET,
        RT_LAG = HELICS_PROPERTY_TIME_RT_LAG,
        RT_LEAD = HELICS_PROPERTY_TIME_RT_LEAD,
        RT_TOLERANCE = HELICS_PROPERTY_TIME_RT_TOLERANCE,
        GRANT_TIMEOUT = HELICS_PROPERTY_TIME_GRANT_TIMEOUT,
        INPUT_DELAY = HELICS_PROPERTY_TIME_INPUT_DELAY,
        OUTPUT_DELAY = HELICS_PROPERTY_TIME_OUTPUT_DELAY,
        CURRENT_ITERATION = HELICS_PROPERTY_INT_CURRENT_ITERATION,
        MAX_ITERATIONS = HELICS_PROPERTY_INT_MAX_ITERATIONS,
        LOG_LEVEL = HELICS_PROPERTY_INT_LOG_LEVEL,
        STOPTIME = HELICS_PROPERTY_TIME_STOPTIME,
        FILE_LOG_LEVEL = HELICS_PROPERTY_INT_FILE_LOG_LEVEL,
        CONSOLE_LOG_LEVEL = HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL,
        LOG_BUFFER = HELICS_PROPERTY_INT_LOG_BUFFER,
        INDEX_GROUP = HELICS_PROPERTY_INT_INDEX_GROUP
    };

    /** options for handles */
    enum Options : int32_t {
        CONNECTION_REQUIRED = HELICS_HANDLE_OPTION_CONNECTION_REQUIRED,
        CONNECTION_OPTIONAL = HELICS_HANDLE_OPTION_CONNECTION_OPTIONAL,
        SINGLE_CONNECTION_ONLY = HELICS_HANDLE_OPTION_SINGLE_CONNECTION_ONLY,
        MULTIPLE_CONNECTIONS_ALLOWED = HELICS_HANDLE_OPTION_MULTIPLE_CONNECTIONS_ALLOWED,
        HANDLE_ONLY_TRANSMIT_ON_CHANGE = HELICS_HANDLE_OPTION_ONLY_TRANSMIT_ON_CHANGE,
        HANDLE_ONLY_UPDATE_ON_CHANGE = HELICS_HANDLE_OPTION_ONLY_UPDATE_ON_CHANGE,
        BUFFER_DATA = HELICS_HANDLE_OPTION_BUFFER_DATA,
        RECONNECTABLE = HELICS_HANDLE_OPTION_RECONNECTABLE,
        RECEIVE_ONLY = HELICS_HANDLE_OPTION_RECEIVE_ONLY,
        SEND_ONLY = HELICS_HANDLE_OPTION_SOURCE_ONLY,
        IGNORE_INTERRUPTS = HELICS_HANDLE_OPTION_IGNORE_INTERRUPTS,
        STRICT_TYPE_CHECKING = HELICS_HANDLE_OPTION_STRICT_TYPE_CHECKING,
        IGNORE_UNIT_MISMATCH = HELICS_HANDLE_OPTION_IGNORE_UNIT_MISMATCH,
        MULTI_INPUT_HANDLING_METHOD = HELICS_HANDLE_OPTION_MULTI_INPUT_HANDLING_METHOD,
        INPUT_PRIORITY_LOCATION = HELICS_HANDLE_OPTION_INPUT_PRIORITY_LOCATION,
        CLEAR_PRIORITY_LIST = HELICS_HANDLE_OPTION_CLEAR_PRIORITY_LIST,
        CONNECTIONS = HELICS_HANDLE_OPTION_CONNECTIONS,
        TIME_RESTRICTED = HELICS_HANDLE_OPTION_TIME_RESTRICTED
    };

}  // namespace defs
}  // namespace helics
