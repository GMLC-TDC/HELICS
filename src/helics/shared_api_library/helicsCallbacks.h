/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

/** @file
@brief functions dealing with callbacks for the shared library
*/

#ifndef HELICS_APISHARED_CALLBACK_FUNCTIONS_H_
#define HELICS_APISHARED_CALLBACK_FUNCTIONS_H_

#include "helics.h"

#ifdef __cplusplus
extern "C" {
#endif

/** set the logging callback to a broker
    @details add a logging callback function for the C The logging callback will be called when
    a message flows into a broker from the core or from a broker
    @param broker the broker object in which to create a subscription must have been create with helicsCreateValueFederate or
    helicsCreateCombinationFederate
    @param logger a callback with signature void(int, const char *, const char *, void *);
    the function arguments are loglevel,  an identifier, and a message string, and a pointer to user data
    @param userdata a point to user data that is passed to the function when executing
    @param[in,out] err a pointer to an error object for catching errors
    */
HELICS_EXPORT void helicsBrokerSetLoggingCallback(
    helics_broker broker,
    void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
    void* userdata,
    helics_error* err);

/** set the logging callback for a core
    @details add a logging callback function for the C The logging callback will be called when
    a message flows into a core from the core or from a broker
    @param core the core object in which to create a subscription must have been create with helicsCreateValueFederate or
    helicsCreateCombinationFederate
    @param logger a callback with signature void(int, const char *, const char *, void *);
    the function arguments are loglevel,  an identifier, and a message string
    @param userdata a point to user data that is passed to the function when executing
    @param[in,out] err a pointer to an error object for catching errors
    */
HELICS_EXPORT void helicsCoreSetLoggingCallback(
    helics_core core,
    void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
    void* userdata,
    helics_error* err);

/** set the logging callback for a federate
       @details add a logging callback function for the C The logging callback will be called when
       a message flows into a federate from the core or from a federate
       @param fed the federate object in which to create a subscription must have been create with helicsCreateValueFederate or
       helicsCreateCombinationFederate
       @param logger a callback with signature void(int, const char *, const char *, void *);
        the function arguments are loglevel,  an identifier string, and a message string, and a pointer to user data
        @param userdata a point to user data that is passed to the function when executing
        @param[in,out] err a pointer to an error object for catching errors
       */
HELICS_EXPORT void helicsFederateSetLoggingCallback(
    helics_federate fed,
    void (*logger)(int loglevel, const char* identifier, const char* message, void* userData),
    void* userdata,
    helics_error* err);

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif
