/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_API_DATA_H_
#define HELICS_API_DATA_H_

#include "stdint.h"

/* Type definitions */
typedef enum {
	helicsOK,
	helicsDiscard,
	helicsError,
} helicsStatus;

typedef uint32_t helics_subscription_id_t;
typedef uint32_t helics_publication_id_t;
typedef uint32_t helics_federate_id_t;

const helics_subscription_id_t invalid_subscription_id = 0xFFFFFFFF;
const helics_publication_id_t invalid_publication_id = 0xFFFFFFFF;
const helics_federate_id_t invalid_federate_id = 0xFFFFFFFF;

typedef int64_t helics_time_t;
/**
* Data to be communicated.
*
* Core operates on opaque byte buffers.
*/
typedef struct data_t
{
	char *data;
	uint64_t len;
} data_t;

/**
*  Message.
*/
typedef struct message_t
{
	const char *origsrc;
	const char *src;
	const char *dst;
	const char *data;
	uint64_t len;
	helics_time_t time;
} message_t;

typedef struct federate_info_t
{
	const char *name;  //!< federate name
	bool obeserver = false; //!< indicator that the federate is an observer and doesn't participate in time advancement
	bool rollback = false; //!< indicator that the federate has rollback features
	bool timeAgnostic = false;	//!< indicator that the federate doesn't use time
	bool forwardCompute = false; //!< indicator that the federate does computation ahead of the timing call[must support rollback if set to true]
	bool interruptible = true;  //!< indicator that the time request can return something other than the requested time
	const char* coreType;  //!< the type of the core
	const char* coreName;  //!< the name of the core
	helics_time_t timeDelta;  //!< the period of the federate
	helics_time_t lookAhead;	//!< the lookahead value 
	helics_time_t impactWindow; //!< the impact window
	const char* coreInitString;  //!< an initialization string for the core API object
} federate_info_t;

#endif
