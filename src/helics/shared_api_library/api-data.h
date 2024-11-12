/*
Copyright (c) 2017-2024,
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
 * opaque object representing a translator
 */
typedef void* HelicsTranslator;

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
 * opaque object representing a helics app
 */
// typedef void* helics_federate;
typedef void* HelicsApp;

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
 * opaque object representing a data buffer in HELICS
 */
typedef void* HelicsDataBuffer;

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
    HELICS_ITERATION_REQUEST_ITERATE_IF_NEEDED = 2, /*!< only return an iteration if necessary */
    HELICS_ITERATION_REQUEST_HALT_OPERATIONS = 5, /*!< halt the federate*/
    HELICS_ITERATION_REQUEST_ERROR = 7 /*!< indicate there was an error */
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
    HELICS_STATE_UNKNOWN = -1, /*! used when no information is available about the federate state*/
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

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif
