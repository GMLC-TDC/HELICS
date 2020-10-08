/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef HELICS_API_DATA_H_
#define HELICS_API_DATA_H_

/**
 * @file
 * @brief Data structures for the C api
 */

#include "../helics_enums.h"

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * opaque object representing an input
 */
typedef void* helics_input;

/**
 * opaque object representing a publication
 */
typedef void* helics_publication;

/**
 * opaque object representing an endpoint
 */
typedef void* helics_endpoint;

/**
 * opaque object representing a filter
 */
typedef void* helics_filter;

/**
 * opaque object representing a core
 */
typedef void* helics_core;

/**
 * opaque object representing a broker
 */
typedef void* helics_broker;

/**
 * opaque object representing a federate
 */
typedef void* helics_federate;

/**
 * opaque object representing a filter info object structure
 */
typedef void* helics_federate_info;

/**
 * opaque object representing a query
 */
typedef void* helics_query;

/**
 * opaque object representing a string buffer for a query
 */
typedef void* helics_query_buffer;

/**
 * opaque object representing a message
 */
typedef void* helics_message_object;

/**
 * time definition used in the C interface to helics
 */
typedef double helics_time;

const helics_time helics_time_zero = 0.0; /*!< definition of time zero-the beginning of simulation */
const helics_time helics_time_epsilon = 1.0e-9; /*!< definition of the minimum time resolution */
const helics_time helics_time_invalid = -1.785e39; /*!< definition of an invalid time that has no meaning */
const helics_time helics_time_maxtime = 9223372036.854774; /*!< definition of time signifying the federate has
                                                             terminated or to run until the end of the simulation*/

/**
 * defining a boolean type for use in the helics interface
 */
typedef int helics_bool;

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
typedef struct helics_complex {
    double real;
    double imag;
} helics_complex;

/**
 *  Message_t mapped to a c compatible structure
 *
 * @details use of this structure is deprecated in HELICS 2.5 and removed in HELICS 3.0
 */
typedef struct helics_message {
    helics_time time; /*!< message time */
    const char* data; /*!< message data */
    int64_t length; /*!< message length */
    int32_t messageID; /*!< message identification information */
    int16_t flags; /*!< flags related to the message */
    const char* original_source; /*!< original source */
    const char* source; /*!< the most recent source */
    const char* dest; /*!< the final destination */
    const char* original_dest; /*!< the original destination of the message */

} helics_message;

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

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif
