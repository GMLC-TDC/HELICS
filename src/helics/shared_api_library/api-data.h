/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#ifndef HELICS_API_DATA_H_
#define HELICS_API_DATA_H_

/** @file
@brief data structures for the C-API
*/

#include "../flag-definitions.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/** enumeration of return values from the C interface functions
 */
typedef enum {

    helics_ok = 0, /*!< the function executed successfully */
    helics_registration_failure = ERROR_CODE_REGISTRATION_FAILURE, /*!< registration has failed*/
    helics_connection_failure = ERROR_CODE_CONNECTION_FAILURE, /*!< the operation to connect has failed*/
    helics_invalid_object = ERROR_CODE_INVALID_OBJECT, /*!< indicator that the object used was not a valid object */
    helics_invalid_argument = ERROR_CODE_INVALID_ARGUMENT, /*!< the parameter passed was invalid and unable to be used*/
    helics_discard = ERROR_CODE_DISCARD, /*!< the input was discarded and not used for some reason */
    helics_system_failure = ERROR_CODE_SYSTEM_FAILURE, /*!< the federate has terminated unexpectedly and the call cannot be completed*/
    helics_warning = -8, /*!< the function issued a warning of some kind */
    helics_invalid_state_transition = ERROR_CODE_INVALID_STATE_TRANSITION, /*!< error issued when an invalid state transition occurred */
    helics_invalid_function_call =
      ERROR_CODE_INVALID_FUNCTION_CALL, /*!< the call made was invalid in the present state of the calling object*/
    helics_execution_failure = ERROR_CODE_EXECUTION_FAILURE, /*!< the function execution has failed*/
    helics_other_error = -101, /*!< the function produced a helics error of some other type */
    other_error_type = -203 /*!< a non helics error was produced*/
} helics_error_types;

/** opaque object representing an input*/
typedef void *helics_input;
/** opaque object representing a publication*/
typedef void *helics_publication;
/** opaque object representing an endpoint*/
typedef void *helics_endpoint;
/** opaque object representing a filter*/
typedef void *helics_filter;
/** opaque object representing a core */
typedef void *helics_core;
/** opaque object representing a broker*/
typedef void *helics_broker;

/** opaque object representing a federate*/
typedef void *helics_federate;

/** opaque object representing a filter info object structure*/
typedef void *helics_federate_info_t;
/** opaque object representing a query*/
typedef void *helics_query;

/** time definition used in the C interface to helics*/
typedef double helics_time_t;
const helics_time_t helics_time_zero = 0.0;
const helics_time_t helics_time_epsilon = 1.0e-9;
const helics_time_t helics_time_invalid = -1.785e39;

/** defining a boolean type for use in the helics interface*/
typedef int helics_bool_t;

const helics_bool_t helics_true = 1;
const helics_bool_t helics_false = 0;

/** enumeration of the different iteration results*/
typedef enum {
    no_iteration, /*!< no iteration is requested */
    force_iteration, /*!< force iteration return when able */
    iterate_if_needed /*!< only return an iteration if necessary */
} helics_iteration_request;

/** enumeration of possible return values from an iterative time request*/
typedef enum {
    next_step, /*!< the iterations have progressed to the next time */
    iteration_error, /*!< there was an error */
    iteration_halted, /*!< the federation has halted */
    iterating /*!< the federate is iterating at current time */
} helics_iteration_status;

/** enumeration of possible federate states*/
typedef enum {
    helics_startup_state = 0, /*!< when created the federate is in startup state */
    helics_initialization_state, /*!< entered after the enterInitializingMode call has returned */
    helics_execution_state, /*!< entered after the enterExectuationState call has returned */
    helics_finalize_state, /*!< the federate has finished executing normally final values may be retrieved */
    helics_error_state, /*!< error state no core communication is possible but values can be retrieved */
    /* the following states are for asynchronous operations */
    helics_pending_init_state, /*!< indicator that the federate is pending entry to initialization state */
    helics_pending_exec_state, /*!< state pending EnterExecution State */
    helics_pending_time_state, /*!< state that the federate is pending a timeRequest */
    helics_pending_iterative_time_state /*!< state that the federate is pending an iterative time request */
} federate_state;

/** enumeration of the predefined filter types*/
typedef enum {
    helics_custom_filter = 0,
    helics_delay_filter = 1,
    helics_randomDelay_filter = 2,
    helics_randomDrop_filter = 3,
    helics_reroute_filter = 4,
    helics_clone_filter = 5

} helics_filter_type_t;

/**
 * Data to be communicated.
 *
 * Core operates on opaque byte buffers.
 */
typedef struct data_t
{
    char *data; /*!< pointer to the data */
    int64_t length; /*!< the size of the data */
} data_t;

/**
 *  structure defining a basic complex type
 */
typedef struct helics_complex
{
    double real;
    double imag;
} helics_complex;

/**
 *  Message_t mapped to a c compatible structure
 */
typedef struct message_t
{
    helics_time_t time; /*!< message time */
    const char *data; /*!< message data */
    int64_t length; /*!< message length */
    const char *original_source; /** original source */
    const char *source; /*!< the most recent source */
    const char *dest; /*!< the final destination */
    const char *original_dest; /*!< the original destination of the message */

} message_t;

/**
* helics error object
*
* if error_code==0 there is no error, if error_code!=0 there is an error and message will contain a string
otherwise it will be an empty string
*/
typedef struct helics_error
{
    int32_t error_code; /*!< an error code associated with the error*/
    const char *message; /*!< a message associated with the error*/
} helics_error;

/** pick a core type depending on compile configuration usually either ZMQ if available or UDP */
#define HELICS_CORE_TYPE_DEFAULT 0
/** use the Zero MQ networking protocol */
#define HELICS_CORE_TYPE_ZMQ 1
/** use MPI for operation on a parallel cluster */
#define HELICS_CORE_TYPE_MPI 2
/** use the Test core if all federates are in the same process */
#define HELICS_CORE_TYPE_TEST 3
/** interprocess uses memory mapped files to transfer data (for use when all federates are
on the same machine */
#define HELICS_CORE_TYPE_INTERPROCESS 4
/** same as INTERPROCESS */
#define HELICS_CORE_TYPE_IPC 5
/** use a generic TCP protocol message stream to send messages */
#define HELICS_CORE_TYPE_TCP 6
/** use UDP packets to send the data */
#define HELICS_CORE_TYPE_UDP 7

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif
