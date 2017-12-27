
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

/* Type definitions */
typedef enum {
    helicsOK,
    helicsDiscard,
    helicsWarning,
    helicsError,
} helicsStatus;

typedef void *helics_subscription;
typedef void *helics_publication;
typedef void *helics_endpoint;
typedef void *helics_source_filter;
typedef void *helics_destination_filter;
typedef void *helics_core;
typedef void *helics_broker;

typedef void *helics_federate;
typedef void *helics_value_federate;
typedef void *helics_message_federate;
typedef void *helics_message_filter_federate;
typedef void *helics_combo_federate;

typedef void *helics_federate_info_t;
typedef void *helics_query;

typedef double helics_time_t;

/** enumeration of the different iteration results*/
typedef enum {
    no_iteration,
    force_iteration, 
    iterate_if_needed,
} iteration_request;

typedef enum {
    next_step,
    iteration_error,
    iteration_halted,
    iterating
} iteration_status;

typedef enum {
    helics_startup = 0,  //!< when created the federate is in startup state
    helics_initialization,  //!< entered after the enterInitializationState call has returned
    helics_execution,  //!< entered after the enterExectuationState call has returned
    helics_finalize,  //!< the federate has finished executing normally final values may be retrieved
    helics_error,  //!< error state no core communication is possible but values can be retrieved
            // the following states are for asynchronous operations
            helics_pending_init,  //!< indicator that the federate is pending entry to initialization state
            helics_pending_exec,  //!< state pending EnterExecution State
            helics_pending_time,  //!< state that the federate is pending a timeRequest
            helics_pending_iterative_time,  //!< state that the federate is pending an iterative time request
} federate_state;

typedef struct helics_iterative_time
{
    helics_time_t time;
    iteration_status status;
} helics_iterative_time;

/**
 * Data to be communicated.
 *
 * Core operates on opaque byte buffers.
 */
typedef struct data_t
{
    char *data;
    int64_t length;
} data_t;

/**
 *  Message_t mapped to a c compatible structure
 */
typedef struct message_t
{
    helics_time_t time;  //!< message time
    const char *data;  //!< message data
    int64_t length;  //!< message length
    const char *src;  //!< the most recent source
    const char *dst;  //!< the final destination
    const char *origsrc;  //!< original source
    const char *origdest; //!< the orginal destination of the message

} message_t;

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif
