
/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#ifndef HELICS_API_DATA_H_
#define HELICS_API_DATA_H_

#include "../flag-definitions.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/** enumeration of return values from the C interface functions
 */
typedef enum {
    helics_invalid_object, /*!< indicator that the object used was not a valid object */
    helics_ok,  /*!< the function executed successfully */
    helics_discard,  /*!< the input was discarded for some reason */
    helics_warning,  /*!< the function issued a warning of some kind */
    helics_error  /*!< the function produced an error */
} helics_status;

/** opaque object representing a subscription*/
typedef void *helics_subscription;
/** opaque object representing a publication*/
typedef void *helics_publication;
/** opaque object representing an endpoint*/
typedef void *helics_endpoint;
/** opaque object representing a source filter*/
typedef void *helics_source_filter;
/** opaque object representing a destination filter*/
typedef void *helics_destination_filter;
/** opaque object representing a cloning filter*/
typedef void *helics_cloning_filter;
/** opaque object representing a core */
typedef void *helics_core;
/** opaque object representing a broker*/
typedef void *helics_broker;

/** opaque object representing a federate*/
typedef void *helics_federate;
/** opaque object representing a value federate
@details this is a specialization of a \ref helics_federate */
typedef void *helics_value_federate;
/** opaque object representing a message federate
@details this is a specialization of a \ref helics_federate */
typedef void *helics_message_federate;

/** opaque object representing a filter info object structure*/
typedef void *helics_federate_info_t;
/** opaque object representing a query*/
typedef void *helics_query;

/** time definition used in the C interface to helics*/
typedef double helics_time_t;

/** enumeration of the different iteration results*/
typedef enum {
    no_iteration,  /*!< no iteration is requested */
    force_iteration,  /*!< force iteration return when able */
    iterate_if_needed  /*!< only return an iteration if necessary */
} iteration_request;

/** enumeration of possible return values from an iterative time request*/
typedef enum {
    next_step,  /*!< the iterations have progressed to the next time */
    iteration_error,  /*!< there was an error */
    iteration_halted,  /*!< the federation has halted */
    iterating  /*!< the federate is iterating at current time */
} iteration_status;

/** enumeration of possible federate states*/
typedef enum {
    helics_startup_state = 0,  /*!< when created the federate is in startup state */
    helics_initialization_state,  /*!< entered after the enterInitializationState call has returned */
    helics_execution_state,  /*!< entered after the enterExectuationState call has returned */
    helics_finalize_state,  /*!< the federate has finished executing normally final values may be retrieved */
    helics_error_state,  /*!< error state no core communication is possible but values can be retrieved */
                         /* the following states are for asynchronous operations */
    helics_pending_init_state,  /*!< indicator that the federate is pending entry to initialization state */
    helics_pending_exec_state,  /*!< state pending EnterExecution State */
    helics_pending_time_state,  /*!< state that the federate is pending a timeRequest */
    helics_pending_iterative_time_state  /*!< state that the federate is pending an iterative time request */
} federate_state;

/** return structure from an iterative time request*/
typedef struct helics_iterative_time
{
    helics_time_t time;  /*!< the current federate time */
    iteration_status status;  /*!< the status of the iterations */
} helics_iterative_time;

/**
 * Data to be communicated.
 *
 * Core operates on opaque byte buffers.
 */
typedef struct data_t
{
    char *data;  /*!< pointer to the data */
    int64_t length;  /*!< the size of the data */
} data_t;

/**
 *  Message_t mapped to a c compatible structure
 */
typedef struct message_t
{
    helics_time_t time;  /*!< message time */
    const char *data;  /*!< message data */
    int64_t length;  /*!< message length */
    const char *original_source;  /** original source */
    const char *source;  /*!< the most recent source */
    const char *dest;  /*!< the final destination */
    const char *original_dest;  /*!< the original destination of the message */

} message_t;

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif
