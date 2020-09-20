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

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif
