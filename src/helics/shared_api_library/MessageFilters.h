/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

/** @file
@brief functions related the message filters for the C api
*/

#ifndef HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_
#define HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_

#include "helics.h"

#ifdef __cplusplus
extern "C" {
#endif

/** create a source Filter on the specified federate
    @details filters can be created through a federate or a core , linking through a federate allows
    a few extra features of name matching to function on the federate interface but otherwise equivalent behavior
    @param fed the fed to register through
   @param type the type of filter to create /ref helics_filter_type
    @param name the name of the filter (can be NULL)
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    @return a helics_filter object
    */
HELICS_EXPORT helics_filter helicsFederateRegisterFilter(helics_federate fed, helics_filter_type type, const char* name, helics_error* err);
/** create a globl source filter through a federate
    @details filters can be created through a federate or a core , linking through a federate allows
    a few extra features of name matching to function on the federate interface but otherwise equivalent behavior
    @param fed the fed to register through
    @param type the type of filter to create /ref helics_filter_type
    @param name the name of the filter (can be NULL)
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    @return a helics_filter object
    */
HELICS_EXPORT helics_filter
    helicsFederateRegisterGlobalFilter(helics_federate fed, helics_filter_type type, const char* name, helics_error* err);

/** create a cloning Filter on the specified federate
    @details cloning filters copy a message and send it to multiple locations source and destination can be added
    through other functions
    @param fed the fed to register through
    @param name the name of the filter (can be NULL)
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    @return a helics_filter object
    */
HELICS_EXPORT helics_filter helicsFederateRegisterCloningFilter(helics_federate fed, const char* name, helics_error* err);

/** create a global cloning Filter on the specified federate
    @details cloning filters copy a message and send it to multiple locations source and destination can be added
    through other functions
    @param fed the fed to register through
    @param name the name of the filter (can be NULL)
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    @return a helics_filter object
    */
HELICS_EXPORT helics_filter helicsFederateRegisterGlobalCloningFilter(helics_federate fed, const char* name, helics_error* err);

/** create a source Filter on the specified core
    @details filters can be created through a federate or a core , linking through a federate allows
    a few extra features of name matching to function on the federate interface but otherwise equivalent behavior
    @param core the core to register through
    @param type the type of filter to create /ref helics_filter_type
    @param name the name of the filter (can be NULL)
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    @return a helics_filter object
    */
HELICS_EXPORT helics_filter helicsCoreRegisterFilter(helics_core core, helics_filter_type type, const char* name, helics_error* err);

/** create a cloning Filter on the specified core
    @details cloning filters copy a message and send it to multiple locations source and destination can be added
    through other functions
    @param core the core to register through
    @param name the name of the filter (can be NULL)
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    @return a helics_filter object
    */
HELICS_EXPORT helics_filter helicsCoreRegisterCloningFilter(helics_core core, const char* name, helics_error* err);

/** get a the number of filters registered through a federate
    @param fed the federate object to use to get the filter
    @return a count of the number of filters registered through a federate
    */
HELICS_EXPORT int helicsFederateGetFilterCount(helics_federate fed);

/** get a filter by its name typically already created via registerInterfaces file or something of that nature
    @param fed the federate object to use to get the filter
    @param name the name of the filter
    @forcpponly
    @param[in,out] err the error object to complete if there is an error
    @endforcpponly
    @return a helics_filter object, the object will not be valid and err will contain an error code if no filter with the specified
    name exists
    */
HELICS_EXPORT helics_filter helicsFederateGetFilter(helics_federate fed, const char* name, helics_error* err);
/** get a filter by its index typically already created via registerInterfaces file or something of that nature
    @param fed the federate object in which to create a publication
    @param index the index of the publication to get
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    @return a helics_filter, which will be NULL if an invalid index
    */
HELICS_EXPORT helics_filter helicsFederateGetFilterByIndex(helics_federate fed, int index, helics_error* err);

/** get the name of the filter and store in the given string
    @param filt the given filter
    @return a string with the name of the filter*/
HELICS_EXPORT const char* helicsFilterGetName(helics_filter filt);

/** set a property on a filter
    @param filt the filter to modify
    @param prop a string containing the property to set
    @param val a numerical value of the property
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    */
HELICS_EXPORT void helicsFilterSet(helics_filter filt, const char* prop, double val, helics_error* err);

/** set a string property on a filter
    @param filt the filter to modify
    @param prop a string containing the property to set
    @param val a string containing the new value
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    */
HELICS_EXPORT void helicsFilterSetString(helics_filter filt, const char* prop, const char* val, helics_error* err);

/** add a destination target to a filter
    @details all messages going to a destination are copied to the delivery address(es)
    @param filt the given filter to add a destination target
    @param dest the name of the endpoint to add as a destination target
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    */
HELICS_EXPORT void helicsFilterAddDestinationTarget(helics_filter filt, const char* dest, helics_error* err);

/** add a source target to a filter
    @details all messages coming from a source are copied to the delivery address(es)
    @param filt the given filter
    @param source the name of the endpoint to add as a source target
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    */
HELICS_EXPORT void helicsFilterAddSourceTarget(helics_filter filt, const char* source, helics_error* err);

/**
    * \defgroup clone filter functions
    @details functions that manipulate cloning filters in some way
    * @{
    */

/** add a delivery endpoint to a cloning filter
    @details all cloned messages are sent to the delivery address(es)
    @param filt the given filter
    @param deliveryEndpoint the name of the endpoint to deliver messages to
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    */
HELICS_EXPORT void helicsFilterAddDeliveryEndpoint(helics_filter filt, const char* deliveryEndpoint, helics_error* err);

/** remove a destination target from a filter
    @param filt the given filter
    @param target the named endpoint to remove as a target
    @forcpponly
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    */
HELICS_EXPORT void helicsFilterRemoveTarget(helics_filter filt, const char* target, helics_error* err);

/** remove a delivery destination from a cloning filter
    @param filt the given filter (must be a cloning filter)
    @param deliveryEndpoint a string with the deliverEndpoint to remove
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    */
HELICS_EXPORT void helicsFilterRemoveDeliveryEndpoint(helics_filter filt, const char* deliveryEndpoint, helics_error* err);

/** get the data in the info field of an filter
    @param filt the given filter
    @return a string with the info field string*/
HELICS_EXPORT const char* helicsFilterGetInfo(helics_filter filt);
/** set the data in the info field for an filter
    @param filt the given filter
    @param info the string to set
    @forcpponly
    @param[in,out] err an error object to fill out in case of an error
    @endforcpponly
    */
HELICS_EXPORT void helicsFilterSetInfo(helics_filter filt, const char* info, helics_error* err);

/** set the data in the info field for an filter
    @param filt the given filter
    @param option the option to set /ref helics_handle_options
    @param value the value of the option (helics_true or helics_false)
    @forcpponly
    @param[in,out] err an error object to fill out in case of an error
    @endforcpponly
    */

HELICS_EXPORT void helicsFilterSetOption(helics_filter filt, int option, helics_bool value, helics_error* err);

/** get a handle option for the filter
    @param filt the given filter to query
    @param option the option to query /ref helics_handle_options*/
HELICS_EXPORT helics_bool helicsFilterGetOption(helics_filter filt, int option);
/**@}*/
#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif /* HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_*/
