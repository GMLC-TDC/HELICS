/*
Copyright (c) 2017-2024,
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

#include "helicsCore.h"

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
 * @param type The type of filter to create /ref HelicsFilterTypes.
 * @param name The name of the filter (can be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsFilter object.
 */
HELICS_EXPORT HelicsFilter helicsFederateRegisterFilter(HelicsFederate fed, HelicsFilterTypes type, const char* name, HelicsError* err);
/**
 * Create a global source filter through a federate.
 *
 * @details Filters can be created through a federate or a core, linking through a federate allows
 *          a few extra features of name matching to function on the federate interface but otherwise equivalent behavior.
 *
 * @param fed The federate to register through.
 * @param type The type of filter to create /ref HelicsFilterTypes.
 * @param name The name of the filter (can be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsFilter object.
 */
HELICS_EXPORT HelicsFilter helicsFederateRegisterGlobalFilter(HelicsFederate fed,
                                                              HelicsFilterTypes type,
                                                              const char* name,
                                                              HelicsError* err);

/**
 * Create a cloning Filter on the specified federate.
 *
 * @details Cloning filters copy a message and send it to multiple locations, source and destination can be added
 *          through other functions.
 *
 * @param fed The federate to register through.
 * @param name The name of the filter (can be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsFilter object.
 */
HELICS_EXPORT HelicsFilter helicsFederateRegisterCloningFilter(HelicsFederate fed, const char* name, HelicsError* err);

/**
 * Create a global cloning Filter on the specified federate.
 *
 * @details Cloning filters copy a message and send it to multiple locations, source and destination can be added
 *          through other functions.
 *
 * @param fed The federate to register through.
 * @param name The name of the filter (can be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsFilter object.
 */
HELICS_EXPORT HelicsFilter helicsFederateRegisterGlobalCloningFilter(HelicsFederate fed, const char* name, HelicsError* err);

/**
 * Create a source Filter on the specified core.
 *
 * @details Filters can be created through a federate or a core, linking through a federate allows
 *          a few extra features of name matching to function on the federate interface but otherwise equivalent behavior.
 *
 * @param core The core to register through.
 * @param type The type of filter to create /ref HelicsFilterTypes.
 * @param name The name of the filter (can be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsFilter object.
 */
HELICS_EXPORT HelicsFilter helicsCoreRegisterFilter(HelicsCore core, HelicsFilterTypes type, const char* name, HelicsError* err);

/**
 * Create a cloning Filter on the specified core.
 *
 * @details Cloning filters copy a message and send it to multiple locations, source and destination can be added
 *          through other functions.
 *
 * @param core The core to register through.
 * @param name The name of the filter (can be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsFilter object.
 */
HELICS_EXPORT HelicsFilter helicsCoreRegisterCloningFilter(HelicsCore core, const char* name, HelicsError* err);

/**
 * Get the number of filters registered through a federate.
 *
 * @param fed The federate object to use to get the filter.
 *
 * @return A count of the number of filters registered through a federate.
 */
HELICS_EXPORT int helicsFederateGetFilterCount(HelicsFederate fed);

/**
 * Get a filter by its name, typically already created via registerInterfaces file or something of that nature.
 *
 * @param fed The federate object to use to get the filter.
 * @param name The name of the filter.
 *
 * @param[in,out] err The error object to complete if there is an error.

 *
 * @return A HelicsFilter object, the object will not be valid and err will contain an error code if no filter with the specified name
 * exists.
 */
HELICS_EXPORT HelicsFilter helicsFederateGetFilter(HelicsFederate fed, const char* name, HelicsError* err);
/**
 * Get a filter by its index, typically already created via registerInterfaces file or something of that nature.
 *
 * @param fed The federate object in which to create a publication.
 * @param index The index of the publication to get.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsFilter, which will be NULL if an invalid index is given.
 */
HELICS_EXPORT HelicsFilter helicsFederateGetFilterByIndex(HelicsFederate fed, int index, HelicsError* err);

/**
 * Check if a filter is valid.
 *
 * @param filt The filter object to check.
 *
 * @return HELICS_TRUE if the Filter object represents a valid filter.
 */
HELICS_EXPORT HelicsBool helicsFilterIsValid(HelicsFilter filt);

/**
 * Get the name of the filter and store in the given string.
 *
 * @param filt The given filter.
 *
 * @return A string with the name of the filter.
 */
HELICS_EXPORT const char* helicsFilterGetName(HelicsFilter filt);

/**
 * Set a property on a filter.
 *
 * @param filt The filter to modify.
 * @param prop A string containing the property to set.
 * @param val A numerical value for the property.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsFilterSet(HelicsFilter filt, const char* prop, double val, HelicsError* err);

/**
 * Set a string property on a filter.
 *
 * @param filt The filter to modify.
 * @param prop A string containing the property to set.
 * @param val A string containing the new value.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsFilterSetString(HelicsFilter filt, const char* prop, const char* val, HelicsError* err);

/**
 * Get a double property from a filter.
 *
 * @param filt The filter to retrieve a value from.
 * @param prop A string containing the property to get.
 *
 */
HELICS_EXPORT double helicsFilterGetPropertyDouble(HelicsFilter filt, const char* prop);

/**
* Get a string property on a filter.  The string output memory is valid until a subsequent call to getPropertyString on the particular
filter.
*
* @param filt The filter to retrieve a value from.
* @param prop A string containing the property to get.
*

*/
HELICS_EXPORT const char* helicsFilterGetPropertyString(HelicsFilter filt, const char* prop);

/**
 * Add a destination target to a filter.
 *
 * @details All messages going to a destination are copied to the delivery address(es).
 * @param filt The given filter to add a destination target to.
 * @param dst The name of the endpoint to add as a destination target.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsFilterAddDestinationTarget(HelicsFilter filt, const char* dst, HelicsError* err);

/**
 * Add a source target to a filter.
 *
 * @details All messages coming from a source are copied to the delivery address(es).
 *
 * @param filt The given filter.
 * @param source The name of the endpoint to add as a source target.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsFilterAddSourceTarget(HelicsFilter filt, const char* source, HelicsError* err);

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
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsFilterAddDeliveryEndpoint(HelicsFilter filt, const char* deliveryEndpoint, HelicsError* err);

/**
 * Remove a destination target from a filter.
 *
 * @param filt The given filter.
 * @param target The named endpoint to remove as a target.
 *
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsFilterRemoveTarget(HelicsFilter filt, const char* target, HelicsError* err);

/**
 * Remove a delivery destination from a cloning filter.
 *
 * @param filt The given filter (must be a cloning filter).
 * @param deliveryEndpoint A string with the delivery endpoint to remove.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsFilterRemoveDeliveryEndpoint(HelicsFilter filt, const char* deliveryEndpoint, HelicsError* err);

/**
 * Get the data in the info field of a filter.
 *
 * @param filt The given filter.
 *
 * @return A string with the info field string.
 */
HELICS_EXPORT const char* helicsFilterGetInfo(HelicsFilter filt);
/**
 * Set the data in the info field for a filter.
 *
 * @param filt The given filter.
 * @param info The string to set.
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */
HELICS_EXPORT void helicsFilterSetInfo(HelicsFilter filt, const char* info, HelicsError* err);

/**
 * Get the data in a specified tag of a filter.
 *
 * @param filt The filter to query.
 * @param tagname The name of the tag to query.
 * @return A string with the tag data.
 */
HELICS_EXPORT const char* helicsFilterGetTag(HelicsFilter filt, const char* tagname);

/**
 * Set the data in a specific tag for a filter.
 *
 * @param filt The filter object to set the tag for.
 * @param tagname The string to set.
 * @param tagvalue the string value to associate with a tag.
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */
HELICS_EXPORT void helicsFilterSetTag(HelicsFilter filt, const char* tagname, const char* tagvalue, HelicsError* err);

/**
 * Set an option value for a filter.
 *
 * @param filt The given filter.
 * @param option The option to set /ref helics_handle_options.
 * @param value The value of the option commonly 0 for false 1 for true.
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */

HELICS_EXPORT void helicsFilterSetOption(HelicsFilter filt, int option, int value, HelicsError* err);

/**
 * Get a handle option for the filter.
 *
 * @param filt The given filter to query.
 * @param option The option to query /ref helics_handle_options.
 */
HELICS_EXPORT int helicsFilterGetOption(HelicsFilter filt, int option);

/**
 * @}
 */

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif /* HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_*/
