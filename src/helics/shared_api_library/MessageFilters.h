/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

/**
 * @file
 *
@brief Functions related to message filters for the C api
*/

#ifndef HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_
#define HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_

#include "helics.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a source Filter on the specified federate.
 *
 * @details Filters can be created through a federate or a core, linking through a federate allows
 *          a few extra features of name matching to function on the federate interface but otherwise equivalent behavior
 *
 * @param fed The federate to register through.
 * @param type The type of filter to create /ref helics_filter_type.
 * @param name The name of the filter (can be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A helics_filter object.
 */
HELICS_EXPORT helics_filter helicsFederateRegisterFilter(helics_federate fed, helics_filter_type type, const char* name, helics_error* err);
/**
 * Create a global source filter through a federate.
 *
 * @details Filters can be created through a federate or a core, linking through a federate allows
 *          a few extra features of name matching to function on the federate interface but otherwise equivalent behavior.
 *
 * @param fed The federate to register through.
 * @param type The type of filter to create /ref helics_filter_type.
 * @param name The name of the filter (can be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A helics_filter object.
 */
HELICS_EXPORT helics_filter helicsFederateRegisterGlobalFilter(helics_federate fed,
                                                               helics_filter_type type,
                                                               const char* name,
                                                               helics_error* err);

/**
 * Create a cloning Filter on the specified federate.
 *
 * @details Cloning filters copy a message and send it to multiple locations, source and destination can be added
 *          through other functions.
 *
 * @param fed The federate to register through.
 * @param name The name of the filter (can be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A helics_filter object.
 */
HELICS_EXPORT helics_filter helicsFederateRegisterCloningFilter(helics_federate fed, const char* name, helics_error* err);

/**
 * Create a global cloning Filter on the specified federate.
 *
 * @details Cloning filters copy a message and send it to multiple locations, source and destination can be added
 *          through other functions.
 *
 * @param fed The federate to register through.
 * @param name The name of the filter (can be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A helics_filter object.
 */
HELICS_EXPORT helics_filter helicsFederateRegisterGlobalCloningFilter(helics_federate fed, const char* name, helics_error* err);

/**
 * Create a source Filter on the specified core.
 *
 * @details Filters can be created through a federate or a core, linking through a federate allows
 *          a few extra features of name matching to function on the federate interface but otherwise equivalent behavior.
 *
 * @param core The core to register through.
 * @param type The type of filter to create /ref helics_filter_type.
 * @param name The name of the filter (can be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A helics_filter object.
 */
HELICS_EXPORT helics_filter helicsCoreRegisterFilter(helics_core core, helics_filter_type type, const char* name, helics_error* err);

/**
 * Create a cloning Filter on the specified core.
 *
 * @details Cloning filters copy a message and send it to multiple locations, source and destination can be added
 *          through other functions.
 *
 * @param core The core to register through.
 * @param name The name of the filter (can be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A helics_filter object.
 */
HELICS_EXPORT helics_filter helicsCoreRegisterCloningFilter(helics_core core, const char* name, helics_error* err);

/**
 * Get the number of filters registered through a federate.
 *
 * @param fed The federate object to use to get the filter.
 *
 * @return A count of the number of filters registered through a federate.
 */
HELICS_EXPORT int helicsFederateGetFilterCount(helics_federate fed);

/**
 * Get a filter by its name, typically already created via registerInterfaces file or something of that nature.
 *
 * @param fed The federate object to use to get the filter.
 * @param name The name of the filter.
 * @forcpponly
 * @param[in,out] err The error object to complete if there is an error.
 * @endforcpponly
 *
 * @return A helics_filter object, the object will not be valid and err will contain an error code if no filter with the specified name
 * exists.
 */
HELICS_EXPORT helics_filter helicsFederateGetFilter(helics_federate fed, const char* name, helics_error* err);
/**
 * Get a filter by its index, typically already created via registerInterfaces file or something of that nature.
 *
 * @param fed The federate object in which to create a publication.
 * @param index The index of the publication to get.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A helics_filter, which will be NULL if an invalid index is given.
 */
HELICS_EXPORT helics_filter helicsFederateGetFilterByIndex(helics_federate fed, int index, helics_error* err);

/**
 * Check if a filter is valid.
 *
 * @param filt The filter object to check.
 *
 * @return helics_true if the Filter object represents a valid filter.
 */
HELICS_EXPORT helics_bool helicsFilterIsValid(helics_filter filt);

/**
 * Get the name of the filter and store in the given string.
 *
 * @param filt The given filter.
 *
 * @return A string with the name of the filter.
 */
HELICS_EXPORT const char* helicsFilterGetName(helics_filter filt);

/**
 * Set a property on a filter.
 *
 * @param filt The filter to modify.
 * @param prop A string containing the property to set.
 * @param val A numerical value for the property.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFilterSet(helics_filter filt, const char* prop, double val, helics_error* err);

/**
 * Set a string property on a filter.
 *
 * @param filt The filter to modify.
 * @param prop A string containing the property to set.
 * @param val A string containing the new value.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFilterSetString(helics_filter filt, const char* prop, const char* val, helics_error* err);

/**
 * Add a destination target to a filter.
 *
 * @details All messages going to a destination are copied to the delivery address(es).
 * @param filt The given filter to add a destination target to.
 * @param dst The name of the endpoint to add as a destination target.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFilterAddDestinationTarget(helics_filter filt, const char* dst, helics_error* err);

/**
 * Add a source target to a filter.
 *
 * @details All messages coming from a source are copied to the delivery address(es).
 *
 * @param filt The given filter.
 * @param source The name of the endpoint to add as a source target.
 * @forcpponly.
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFilterAddSourceTarget(helics_filter filt, const char* source, helics_error* err);

/**
 * \defgroup Clone filter functions
 * @details Functions that manipulate cloning filters in some way.
 * @{
 */

/**
 * Add a delivery endpoint to a cloning filter.
 *
 * @details All cloned messages are sent to the delivery address(es).
 *
 * @param filt The given filter.
 * @param deliveryEndpoint The name of the endpoint to deliver messages to.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFilterAddDeliveryEndpoint(helics_filter filt, const char* deliveryEndpoint, helics_error* err);

/**
 * Remove a destination target from a filter.
 *
 * @param filt The given filter.
 * @param target The named endpoint to remove as a target.
 * @forcpponly
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFilterRemoveTarget(helics_filter filt, const char* target, helics_error* err);

/**
 * Remove a delivery destination from a cloning filter.
 *
 * @param filt The given filter (must be a cloning filter).
 * @param deliveryEndpoint A string with the delivery endpoint to remove.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFilterRemoveDeliveryEndpoint(helics_filter filt, const char* deliveryEndpoint, helics_error* err);

/**
 * Get the data in the info field of a filter.
 *
 * @param filt The given filter.
 *
 * @return A string with the info field string.
 */
HELICS_EXPORT const char* helicsFilterGetInfo(helics_filter filt);
/**
 * Set the data in the info field for a filter.
 *
 * @param filt The given filter.
 * @param info The string to set.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFilterSetInfo(helics_filter filt, const char* info, helics_error* err);

/**
 * Set the data in the info field for a filter.
 *
 * @param filt The given filter.
 * @param option The option to set /ref helics_handle_options.
 * @param value The value of the option commonly 0 for false 1 for true.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */

HELICS_EXPORT void helicsFilterSetOption(helics_filter filt, int option, int value, helics_error* err);

/**
 * Get a handle option for the filter.
 *
 * @param filt The given filter to query.
 * @param option The option to query /ref helics_handle_options.
 */
HELICS_EXPORT int helicsFilterGetOption(helics_filter filt, int option);

/**
 * @}
 */

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif /* HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_*/
