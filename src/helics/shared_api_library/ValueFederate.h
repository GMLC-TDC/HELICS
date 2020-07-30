/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

/**
 * @file
 *
 * @brief Functions related to value federates for the C api
 */

#ifndef HELICS_APISHARED_VALUE_FEDERATE_FUNCTIONS_H_
#define HELICS_APISHARED_VALUE_FEDERATE_FUNCTIONS_H_

#include "helics.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * sub/pub registration
 */

/**
 * Create a subscription.
 *
 * @details The subscription becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions and publications.
 *
 * @param fed The federate object in which to create a subscription, must have been created with /ref helicsCreateValueFederate or
 * /ref helicsCreateCombinationFederate.
 * @param key The identifier matching a publication to get a subscription for.
 * @param units A string listing the units of the subscription (may be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the subscription.
 */
HELICS_EXPORT helics_input helicsFederateRegisterSubscription(helics_federate fed, const char* key, const char* units, helics_error* err);

/**
 * Register a publication with a known type.
 *
 * @details The publication becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions and publications.
 *
 * @param fed The federate object in which to create a publication.
 * @param key The identifier for the publication the global publication key will be prepended with the federate name.
 * @param type A code identifying the type of the input see /ref helics_data_type for available options.
 * @param units A string listing the units of the subscription (may be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the publication.
 */
HELICS_EXPORT helics_publication
    helicsFederateRegisterPublication(helics_federate fed, const char* key, helics_data_type type, const char* units, helics_error* err);

/**
 * Register a publication with a defined type.
 *
 * @details The publication becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions and publications.
 *
 * @param fed The federate object in which to create a publication.
 * @param key The identifier for the publication.
 * @param type A string labeling the type of the publication.
 * @param units A string listing the units of the subscription (may be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the publication.
 */
HELICS_EXPORT helics_publication
    helicsFederateRegisterTypePublication(helics_federate fed, const char* key, const char* type, const char* units, helics_error* err);

/**
 * Register a global named publication with an arbitrary type.
 *
 * @details The publication becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions and publications.
 *
 * @param fed The federate object in which to create a publication.
 * @param key The identifier for the publication.
 * @param type A code identifying the type of the input see /ref helics_data_type for available options.
 * @param units A string listing the units of the subscription (may be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the publication.
 */
HELICS_EXPORT helics_publication helicsFederateRegisterGlobalPublication(helics_federate fed,
                                                                         const char* key,
                                                                         helics_data_type type,
                                                                         const char* units,
                                                                         helics_error* err);

/**
 * Register a global publication with a defined type.
 *
 * @details The publication becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions and publications.
 *
 * @param fed The federate object in which to create a publication.
 * @param key The identifier for the publication.
 * @param type A string describing the expected type of the publication.
 * @param units A string listing the units of the subscription (may be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the publication.
 */
HELICS_EXPORT helics_publication helicsFederateRegisterGlobalTypePublication(helics_federate fed,
                                                                             const char* key,
                                                                             const char* type,
                                                                             const char* units,
                                                                             helics_error* err);

/**
 * Register a named input.
 *
 * @details The input becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions, inputs, and publications.
 *
 * @param fed The federate object in which to create an input.
 * @param key The identifier for the publication the global input key will be prepended with the federate name.
 * @param type A code identifying the type of the input see /ref helics_data_type for available options.
 * @param units A string listing the units of the input (may be NULL).
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the input.
 */
HELICS_EXPORT helics_input
    helicsFederateRegisterInput(helics_federate fed, const char* key, helics_data_type type, const char* units, helics_error* err);

/**
 * Register an input with a defined type.
 *
 * @details The input becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions, inputs, and publications.
 *
 * @param fed The federate object in which to create an input.
 * @param key The identifier for the input.
 * @param type A string describing the expected type of the input.
 * @param units A string listing the units of the input maybe NULL.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the publication.
 */
HELICS_EXPORT helics_input
    helicsFederateRegisterTypeInput(helics_federate fed, const char* key, const char* type, const char* units, helics_error* err);

/**
 * Register a global named input.
 *
 * @details The publication becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions and publications.
 *
 * @param fed The federate object in which to create a publication.
 * @param key The identifier for the publication.
 * @param type A code identifying the type of the input see /ref helics_data_type for available options.
 * @param units A string listing the units of the subscription maybe NULL.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the publication.
 */
HELICS_EXPORT helics_publication
    helicsFederateRegisterGlobalInput(helics_federate fed, const char* key, helics_data_type type, const char* units, helics_error* err);

/**
 * Register a global publication with an arbitrary type.
 *
 * @details The publication becomes part of the federate and is destroyed when the federate is freed so there are no separate free
 * functions for subscriptions and publications.
 *
 * @param fed The federate object in which to create a publication.
 * @param key The identifier for the publication.
 * @param type A string defining the type of the input.
 * @param units A string listing the units of the subscription maybe NULL.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the publication.
 */
HELICS_EXPORT helics_publication
    helicsFederateRegisterGlobalTypeInput(helics_federate fed, const char* key, const char* type, const char* units, helics_error* err);

/**
 * Get a publication object from a key.
 *
 * @param fed The value federate object to use to get the publication.
 * @param key The name of the publication.
 * @forcpponly
 * @param[in,out] err The error object to complete if there is an error.
 * @endforcpponly
 *
 * @return A helics_publication object, the object will not be valid and err will contain an error code if no publication with the
 * specified key exists.
 */
HELICS_EXPORT helics_publication helicsFederateGetPublication(helics_federate fed, const char* key, helics_error* err);

/**
 * Get a publication by its index, typically already created via registerInterfaces file or something of that nature.
 *
 * @param fed The federate object in which to create a publication.
 * @param index The index of the publication to get.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A helics_publication.
 */
HELICS_EXPORT helics_publication helicsFederateGetPublicationByIndex(helics_federate fed, int index, helics_error* err);

/**
 * Get an input object from a key.
 *
 * @param fed The value federate object to use to get the publication.
 * @param key The name of the input.
 * @forcpponly
 * @param[in,out] err The error object to complete if there is an error.
 * @endforcpponly
 *
 * @return A helics_input object, the object will not be valid and err will contain an error code if no input with the specified
 * key exists.
 */
HELICS_EXPORT helics_input helicsFederateGetInput(helics_federate fed, const char* key, helics_error* err);

/**
 * Get an input by its index, typically already created via registerInterfaces file or something of that nature.
 *
 * @param fed The federate object in which to create a publication.
 * @param index The index of the publication to get.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A helics_input, which will be NULL if an invalid index.
 */
HELICS_EXPORT helics_input helicsFederateGetInputByIndex(helics_federate fed, int index, helics_error* err);

/**
 * Get an input object from a subscription target.
 *
 * @param fed The value federate object to use to get the publication.
 * @param key The name of the publication that a subscription is targeting.
 * @forcpponly
 * @param[in,out] err The error object to complete if there is an error.
 * @endforcpponly
 *
 * @return A helics_input object, the object will not be valid and err will contain an error code if no input with the specified
 * key exists.
 */
HELICS_EXPORT helics_input helicsFederateGetSubscription(helics_federate fed, const char* key, helics_error* err);

/**
 * Clear all the update flags from a federates inputs.
 *
 * @param fed The value federate object for which to clear update flags.
 */
HELICS_EXPORT void helicsFederateClearUpdates(helics_federate fed);

/**
 * Register the publications via JSON publication string.
 *
 * @param fed The value federate object to use to register the publications.
 * @param json The JSON publication string.
 * @forcpponly
 * @param[in,out] err The error object to complete if there is an error.
 * @endforcpponly
 *
 * @details This would be the same JSON that would be used to publish data.
 */
HELICS_EXPORT void helicsFederateRegisterFromPublicationJSON(helics_federate fed, const char* json, helics_error* err);

/**
 * Publish data contained in a JSON file or string.
 *
 * @param fed The value federate object through which to publish the data.
 * @param json The publication file name or literal JSON data string.
 * @forcpponly
 * @param[in,out] err The error object to complete if there is an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsFederatePublishJSON(helics_federate fed, const char* json, helics_error* err);

/**
 * \defgroup publications Publication functions
 * @details Functions for publishing data of various kinds.
 * The data will get translated to the type specified when the publication was constructed automatically
 * regardless of the function used to publish the data.
 * @{
 */

/**
 * Check if a publication is valid.
 *
 * @param pub The publication to check.
 *
 * @return helics_true if the publication is a valid publication.
 */
HELICS_EXPORT helics_bool helicsPublicationIsValid(helics_publication pub);

/**
 * Publish raw data from a char * and length.
 *
 * @param pub The publication to publish for.
 * @param data A pointer to the raw data.
 * @param inputDataLength The size in bytes of the data to publish.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationPublishRaw(helics_publication pub, const void* data, int inputDataLength, helics_error* err);

/**
 * Publish a string.
 *
 * @param pub The publication to publish for.
 * @param str The string to publish.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationPublishString(helics_publication pub, const char* str, helics_error* err);

/**
 * Publish an integer value.
 *
 * @param pub The publication to publish for.
 * @param val The numerical value to publish.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationPublishInteger(helics_publication pub, int64_t val, helics_error* err);

/**
 * Publish a Boolean Value.
 *
 * @param pub The publication to publish for.
 * @param val The boolean value to publish.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationPublishBoolean(helics_publication pub, helics_bool val, helics_error* err);

/**
 * Publish a double floating point value.
 *
 * @param pub The publication to publish for.
 * @param val The numerical value to publish.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationPublishDouble(helics_publication pub, double val, helics_error* err);

/**
 * Publish a time value.
 *
 * @param pub The publication to publish for.
 * @param val The numerical value to publish.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationPublishTime(helics_publication pub, helics_time val, helics_error* err);

/**
 * Publish a single character.
 *
 * @param pub The publication to publish for.
 * @param val The numerical value to publish.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationPublishChar(helics_publication pub, char val, helics_error* err);

/**
 * Publish a complex value (or pair of values).
 *
 * @param pub The publication to publish for.
 * @param real The real part of a complex number to publish.
 * @param imag The imaginary part of a complex number to publish.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationPublishComplex(helics_publication pub, double real, double imag, helics_error* err);

/**
 * Publish a vector of doubles.
 *
 * @param pub The publication to publish for.
 * @param vectorInput A pointer to an array of double data.
 * @forcpponly
 * @param vectorLength The number of points to publish.
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationPublishVector(helics_publication pub, const double* vectorInput, int vectorLength, helics_error* err);

/**
 * Publish a named point.
 *
 * @param pub The publication to publish for.
 * @param str A string for the name to publish.
 * @param val A double for the value to publish.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationPublishNamedPoint(helics_publication pub, const char* str, double val, helics_error* err);

/**
 * Add a named input to the list of targets a publication publishes to.
 *
 * @param pub The publication to add the target for.
 * @param target The name of an input that the data should be sent to.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationAddTarget(helics_publication pub, const char* target, helics_error* err);

/**
 * Check if an input is valid.
 *
 * @param ipt The input to check.
 *
 * @return helics_true if the Input object represents a valid input.
 */
HELICS_EXPORT helics_bool helicsInputIsValid(helics_input ipt);

/**
 * Add a publication to the list of data that an input subscribes to.
 *
 * @param ipt The named input to modify.
 * @param target The name of a publication that an input should subscribe to.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputAddTarget(helics_input ipt, const char* target, helics_error* err);

/**@}*/

/**
 * \defgroup getValue GetValue functions
 * @details Data can be returned in a number of formats,  for instance if data is published as a double it can be returned as a string and
 * vice versa,  not all translations make that much sense but they do work.
 * @{
 */

/**
 * Get the size of the raw value for subscription.
 *
 * @return The size of the raw data/string in bytes.
 */
HELICS_EXPORT int helicsInputGetRawValueSize(helics_input ipt);

/**
 * Get the raw data for the latest value of a subscription.
 *
 * @param ipt The input to get the data for.
 * @forcpponly
 * @param[out] data The memory location of the data
 * @param maxDataLength The maximum size of information that data can hold.
 * @param[out] actualSize The actual length of data copied to data.
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @beginPythonOnly
 * @return Raw string data.
 * @endPythonOnly
 */
HELICS_EXPORT void helicsInputGetRawValue(helics_input ipt, void* data, int maxDataLength, int* actualSize, helics_error* err);

/**
 * Get the size of a value for subscription assuming return as a string.
 *
 * @return The size of the string.
 */
HELICS_EXPORT int helicsInputGetStringSize(helics_input ipt);

/**
 * Get a string value from a subscription.
 *
 * @param ipt The input to get the data for.
 * @forcpponly
 * @param[out] outputString Storage for copying a null terminated string.
 * @param maxStringLength The maximum size of information that str can hold.
 * @param[out] actualLength The actual length of the string.
 * @param[in,out] err Error term for capturing errors.
 * @endforcpponly
 *
 * @beginPythonOnly
 * @return A string data
 * @endPythonOnly
 */
HELICS_EXPORT void helicsInputGetString(helics_input ipt, char* outputString, int maxStringLength, int* actualLength, helics_error* err);

/**
 * Get an integer value from a subscription.
 *
 * @param ipt The input to get the data for.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An int64_t value with the current value of the input.
 */
HELICS_EXPORT int64_t helicsInputGetInteger(helics_input ipt, helics_error* err);

/**
 * Get a boolean value from a subscription.
 *
 * @param ipt The input to get the data for.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A boolean value of current input value.
 */
HELICS_EXPORT helics_bool helicsInputGetBoolean(helics_input ipt, helics_error* err);

/**
 * Get a double value from a subscription.
 *
 * @param ipt The input to get the data for.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return The double value of the input.
 */
HELICS_EXPORT double helicsInputGetDouble(helics_input ipt, helics_error* err);

/**
 * Get a time value from a subscription.
 *
 * @param ipt The input to get the data for.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return The resulting time value.
 */
HELICS_EXPORT helics_time helicsInputGetTime(helics_input ipt, helics_error* err);

/**
 * Get a single character value from an input.
 *
 * @param ipt The input to get the data for.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return The resulting character value.
 * @forcpponly
 *         NAK (negative acknowledgment) symbol returned on error
 * @endforcpponly
 */
HELICS_EXPORT char helicsInputGetChar(helics_input ipt, helics_error* err);

/**
 * Get a complex object from an input object.
 *
 * @param ipt The input to get the data for.
 * @forcpponly
 * @param[in,out] err A helics error object, if the object is not empty the function is bypassed otherwise it is filled in if there is an
 * error.
 * @endforcpponly
 *
 * @return A helics_complex structure with the value.
 */
HELICS_EXPORT helics_complex helicsInputGetComplexObject(helics_input ipt, helics_error* err);

/**
 * Get a pair of double forming a complex number from a subscriptions.
 *
 * @param ipt The input to get the data for.
 * @forcpponly
 * @param[out] real Memory location to place the real part of a value.
 * @param[out] imag Memory location to place the imaginary part of a value.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * On error the values will not be altered.
 * @endforcpponly
 *
 * @beginPythonOnly
 * @return a pair of floating point values that represent the real and imag values
 * @endPythonOnly
 */
HELICS_EXPORT void helicsInputGetComplex(helics_input ipt, double* real, double* imag, helics_error* err);

/**
 * Get the size of a value for subscription assuming return as an array of doubles.
 *
 * @return The number of doubles in a returned vector.
 */
HELICS_EXPORT int helicsInputGetVectorSize(helics_input ipt);

/**
 * Get a vector from a subscription.
 *
 * @param ipt The input to get the result for.
 * @forcpponly
 * @param[out] data The location to store the data.
 * @param maxLength The maximum size of the vector.
 * @param[out] actualSize Location to place the actual length of the resulting vector.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @beginPythonOnly
 * @return a list of floating point values
 * @endPythonOnly
 */
HELICS_EXPORT void helicsInputGetVector(helics_input ipt, double data[], int maxLength, int* actualSize, helics_error* err);

/**
 * Get a named point from a subscription.
 *
 * @param ipt The input to get the result for.
 * @forcpponly
 * @param[out] outputString Storage for copying a null terminated string.
 * @param maxStringLength The maximum size of information that str can hold.
 * @param[out] actualLength The actual length of the string
 * @param[out] val The double value for the named point.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 *
 * @beginPythonOnly
 * @return a string and a double value for the named point
 * @endPythonOnly
 */
HELICS_EXPORT void
    helicsInputGetNamedPoint(helics_input ipt, char* outputString, int maxStringLength, int* actualLength, double* val, helics_error* err);

/**@}*/

/**
 * \defgroup default_values Default Value functions
 * @details These functions set the default value for a subscription. That is the value returned if nothing was published from elsewhere.
 * @{
 */

/**
 * Set the default as a raw data array.
 *
 * @param ipt The input to set the default for.
 * @param data A pointer to the raw data to use for the default.
 * @forcpponly
 * @param inputDataLength The size of the raw data.
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetDefaultRaw(helics_input ipt, const void* data, int inputDataLength, helics_error* err);

/**
 * Set the default as a string.
 *
 * @param ipt The input to set the default for.
 * @param str A pointer to the default string.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetDefaultString(helics_input ipt, const char* str, helics_error* err);

/**
 * Set the default as an integer.
 *
 * @param ipt The input to set the default for.
 * @param val The default integer.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetDefaultInteger(helics_input ipt, int64_t val, helics_error* err);

/**
 * Set the default as a boolean.
 *
 * @param ipt The input to set the default for.
 * @param val The default boolean value.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetDefaultBoolean(helics_input ipt, helics_bool val, helics_error* err);

/**
 * Set the default as a time.
 *
 * @param ipt The input to set the default for.
 * @param val The default time value.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetDefaultTime(helics_input ipt, helics_time val, helics_error* err);

/**
 * Set the default as a char.
 *
 * @param ipt The input to set the default for.
 * @param val The default char value.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetDefaultChar(helics_input ipt, char val, helics_error* err);

/**
 * Set the default as a double.
 *
 * @param ipt The input to set the default for.
 * @param val The default double value.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetDefaultDouble(helics_input ipt, double val, helics_error* err);

/**
 * Set the default as a complex number.
 *
 * @param ipt The input to set the default for.
 * @param real The default real value.
 * @param imag The default imaginary value.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetDefaultComplex(helics_input ipt, double real, double imag, helics_error* err);

/**
 * Set the default as a vector of doubles.
 *
 * @param ipt The input to set the default for.
 * @param vectorInput A pointer to an array of double data.
 * @param vectorLength The number of points to publish.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetDefaultVector(helics_input ipt, const double* vectorInput, int vectorLength, helics_error* err);

/**
 * Set the default as a NamedPoint.
 *
 * @param ipt The input to set the default for.
 * @param str A pointer to a string representing the name.
 * @param val A double value for the value of the named point.
 * @forcpponly
 * @param[in,out] err An error object that will contain an error code and string if any error occurred during the execution of the function.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetDefaultNamedPoint(helics_input ipt, const char* str, double val, helics_error* err);

/**@}*/

/**
 * \defgroup Information retrieval
 * @{
 */

/**
 * Get the type of an input.
 *
 * @param ipt The input to query.
 *
 * @return A void enumeration, helics_ok if everything worked.
 */
HELICS_EXPORT const char* helicsInputGetType(helics_input ipt);

/**
 * Get the type the publisher to an input is sending.
 *
 * @param ipt The input to query.
 *
 * @return A const char * with the type name.
 */
HELICS_EXPORT const char* helicsInputGetPublicationType(helics_input ipt);

/**
 * Get the type of a publication.
 *
 * @param pub The publication to query.
 *
 * @return A void enumeration, helics_ok if everything worked.
 */
HELICS_EXPORT const char* helicsPublicationGetType(helics_publication pub);

/**
 * Get the key of an input.
 *
 * @param ipt The input to query.
 *
 * @return A void enumeration, helics_ok if everything worked.
 */
HELICS_EXPORT const char* helicsInputGetKey(helics_input ipt);

/**
 * Get the key of a subscription.
 *
 * @return A const char with the subscription key.
 */
HELICS_EXPORT const char* helicsSubscriptionGetKey(helics_input ipt);

/**
 * Get the key of a publication.
 *
 * @details This will be the global key used to identify the publication to the federation.
 *
 * @param pub The publication to query.
 *
 * @return A void enumeration, helics_ok if everything worked.
 */
HELICS_EXPORT const char* helicsPublicationGetKey(helics_publication pub);

/**
 * Get the units of an input.
 *
 * @param ipt The input to query.
 *
 * @return A void enumeration, helics_ok if everything worked.
 */
HELICS_EXPORT const char* helicsInputGetUnits(helics_input ipt);

/**
 * Get the units of the publication that an input is linked to.
 *
 * @param ipt The input to query.
 *
 * @return A void enumeration, helics_ok if everything worked.
 */
HELICS_EXPORT const char* helicsInputGetInjectionUnits(helics_input ipt);

/**
 * Get the units of an input.
 *
 * @details The same as helicsInputGetUnits.
 *
 * @param ipt The input to query.
 *
 * @return A void enumeration, helics_ok if everything worked.
 */
HELICS_EXPORT const char* helicsInputGetExtractionUnits(helics_input ipt);

/**
 * Get the units of a publication.
 *
 * @param pub The publication to query.
 *
 * @return A void enumeration, helics_ok if everything worked.
 */
HELICS_EXPORT const char* helicsPublicationGetUnits(helics_publication pub);

/**
 * Get the data in the info field of an input.
 *
 * @param inp The input to query.
 *
 * @return A string with the info field string.
 */
HELICS_EXPORT const char* helicsInputGetInfo(helics_input inp);

/**
 * Set the data in the info field for an input.
 *
 * @param inp The input to query.
 * @param info The string to set.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetInfo(helics_input inp, const char* info, helics_error* err);

/**
 * Get the data in the info field of an publication.
 *
 * @param pub The publication to query.
 *
 * @return A string with the info field string.
 */
HELICS_EXPORT const char* helicsPublicationGetInfo(helics_publication pub);

/**
 * Set the data in the info field for a publication.
 *
 * @param pub The publication to set the info field for.
 * @param info The string to set.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationSetInfo(helics_publication pub, const char* info, helics_error* err);

/**
 * Get the current value of an input handle option
 *
 * @param inp The input to query.
 * @param option Integer representation of the option in question see /ref helics_handle_options.
 *
 * @return An integer value with the current value of the given option.
 */
HELICS_EXPORT int helicsInputGetOption(helics_input inp, int option);

/**
 * Set an option on an input
 *
 * @param inp The input to query.
 * @param option The option to set for the input /ref helics_handle_options.
 * @param value The value to set the option to.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetOption(helics_input inp, int option, int value, helics_error* err);

/**
 * Get the value of an option for a publication
 *
 * @param pub The publication to query.
 * @param option The value to query see /ref helics_handle_options.
 *
 * @return A string with the info field string.
 */
HELICS_EXPORT int helicsPublicationGetOption(helics_publication pub, int option);

/**
 * Set the value of an option for a publication
 *
 * @param pub The publication to query.
 * @param option Integer code for the option to set /ref helics_handle_options.
 * @param val The value to set the option to.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationSetOption(helics_publication pub, int option, int val, helics_error* err);

/**
 * Set the minimum change detection tolerance.
 *
 * @param pub The publication to modify.
 * @param tolerance The tolerance level for publication, values changing less than this value will not be published.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsPublicationSetMinimumChange(helics_publication pub, double tolerance, helics_error* err);

/**
 * Set the minimum change detection tolerance.
 *
 * @param inp The input to modify.
 * @param tolerance The tolerance level for registering an update, values changing less than this value will not show as being updated.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsInputSetMinimumChange(helics_input inp, double tolerance, helics_error* err);

/**@}*/

/**
 * Check if a particular subscription was updated.
 *
 * @return helics_true if it has been updated since the last value retrieval.
 */
HELICS_EXPORT helics_bool helicsInputIsUpdated(helics_input ipt);

/**
 * Get the last time a subscription was updated.
 */
HELICS_EXPORT helics_time helicsInputLastUpdateTime(helics_input ipt);

/**
 * Clear the updated flag from an input.
 */
HELICS_EXPORT void helicsInputClearUpdate(helics_input ipt);

/**
 * Get the number of publications in a federate.
 *
 * @return (-1) if fed was not a valid federate otherwise returns the number of publications.
 */
HELICS_EXPORT int helicsFederateGetPublicationCount(helics_federate fed);

/**
 * Get the number of subscriptions in a federate.
 *
 * @return (-1) if fed was not a valid federate otherwise returns the number of subscriptions.
 */
HELICS_EXPORT int helicsFederateGetInputCount(helics_federate fed);

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif /* HELICS_APISHARED_VALUE_FEDERATE_FUNCTIONS_H_*/
