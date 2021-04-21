/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

/**
 * @file
 * @brief Functions dealing with callbacks for the shared library
 */

#ifndef HELICS_APISHARED_CALLBACK_FUNCTIONS_H_
#define HELICS_APISHARED_CALLBACK_FUNCTIONS_H_

#include "helics.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Set the logging callback to a broker.
 *
 * @details Add a logging callback function to a broker.
 *          The logging callback will be called when
 *          a message flows into a broker from the core or from a broker.
 *
 * @param broker The broker object in which to set the callback.
 * @param logger A callback with signature void(int, const char *, const char *, void *);
 *               the function arguments are loglevel, an identifier, a message string, and a pointer to user data.
 * @param userdata A pointer to user data that is passed to the function when executing.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsBrokerSetLoggingCallback(helics_broker broker,
                                                  void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
                                                  void* userdata,
                                                  helics_error* err);

/**
 * Set the logging callback for a core.
 *
 * @details Add a logging callback function to a core. The logging callback will be called when
 *          a message flows into a core from the core or from a broker.
 *
 * @param core The core object in which to set the callback.
 * @param logger A callback with signature void(int, const char *, const char *, void *);
 *               The function arguments are loglevel, an identifier, a message string, and a pointer to user data.
 * @param userdata A pointer to user data that is passed to the function when executing.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsCoreSetLoggingCallback(helics_core core,
                                                void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
                                                void* userdata,
                                                helics_error* err);

/**
 * Set the logging callback for a federate.
 *
 * @details Add a logging callback function to a federate. The logging callback will be called when
 *          a message flows into a federate from the core or from a federate.
 *
 * @param fed The federate object in which to create a subscription must have been created with
 *            helicsCreateValueFederate or helicsCreateCombinationFederate.
 * @param logger A callback with signature void(int, const char *, const char *, void *);
 *        The function arguments are loglevel, an identifier string, a message string, and a pointer to user data.
 * @param userdata A pointer to user data that is passed to the function when executing.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void
    helicsFederateSetLoggingCallback(helics_federate fed,
                                     void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
                                     void* userdata,
                                     helics_error* err);

/**
 * Set a general callback for a custom filter.
 *
 * @details Add a custom filter callback for creating a custom filter operation in the C shared library.
 *
 * @param filter The filter object to set the callback for.
 * @param filtCall A callback with signature helics_message_object(helics_message_object, void *);
 *                 The function arguments are the message to filter and a pointer to user data.
 *                 The filter should return a new message.
 * @param userdata A pointer to user data that is passed to the function when executing.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFilterSetCustomCallback(helics_filter filter,
                                                 void (*filtCall)(helics_message_object message, void* userData),
                                                 void* userdata,
                                                 helics_error* err);

/**
 * Set callback for queries executed against a federate.
 *
 * @details There are many queries that HELICS understands directly, but it is occasionally useful to have a federate be able to respond
 * to specific queries with answers specific to a federate.
 *
 * @param fed The federate to set the callback for.
 * @param queryAnswer A callback with signature const char *(const char *query, int querySize,int *answerSize, void *userdata);
 *                 The function arguments are the query string requesting an answer along with its size, the string is not guaranteed to be
 * null terminated answerSize is an outputParameter intended to filled out by the userCallback and should contain the length of the return
 * string. The return pointer can be NULL if no answer is given and HELICS will generate the appropriate response.
 * @param userdata A pointer to user data that is passed to the function when executing.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */

HELICS_EXPORT void
    helicsFederateSetQueryCallback(helics_federate fed,
                                   void (*queryAnswer)(const char* query, int querySize, helics_query_buffer buffer, void* userdata),
                                   void* userdata,
                                   helics_error* err);

/**
 * Set the data for a query callback.
 *
 * @details There are many queries that HELICS understands directly, but it is occasionally useful to have a federate be able to respond
 * to specific queries with answers specific to a federate.
 *
 * @param buffer The buffer received in a helicsQueryCallback.
 * @param str Pointer to the data to fill the buffer with.
 * @param strSize The size of the string.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsQueryBufferFill(helics_query_buffer buffer, const char* str, int strSize, helics_error* err);

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif
