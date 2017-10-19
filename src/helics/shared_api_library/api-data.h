
/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#ifndef HELICS_API_DATA_H_
#define HELICS_API_DATA_H_

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

typedef enum {
    nonconverged,
    converged,
    halted,
    error,
} convergence_status;

typedef struct helics_iterative_time
{
    helics_time_t time;
    convergence_status status;
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
    const char *origsrc;  //!< original source
    const char *src;  //!< the most recent source
    const char *dst;  //!< the final destination

} message_t;

#define OBSERVER_FLAG 0
#define ROLLBACK_FLAG 1
#define UNINTERRUPTIBLE_FLAG 2
#define FORWARD_COMPUTE_FLAG 3
#define TIME_AGNOSTIC_FLAG 4
#define SOURCE_ONLY_FLAG 5

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif
