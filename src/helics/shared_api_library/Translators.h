/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

/**
 * @file
 *
@brief Functions related to message translators for the C api
*/

#ifndef HELICS_APISHARED_MESSAGE_TRANSLATOR_FUNCTIONS_H_
#define HELICS_APISHARED_MESSAGE_TRANSLATOR_FUNCTIONS_H_

#include "helicsCore.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a source Translator on the specified federate.
 *
 * @details Translators can be created through a federate or a core, linking through a federate allows
 *          a few extra features of name matching to function on the federate interface but otherwise equivalent behavior
 *
 * @param fed The federate to register through.
 * @param type The type of translator to create /ref HelicsTranslatorTypes.
 * @param name The name of the translator (can be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsTranslator object.
 */
HELICS_EXPORT HelicsTranslator helicsFederateRegisterTranslator(HelicsFederate fed, HelicsTranslatorTypes type, const char* name, HelicsError* err);
/**
 * Create a global source translator through a federate.
 *
 * @details Translators can be created through a federate or a core, linking through a federate allows
 *          a few extra features of name matching to function on the federate interface but otherwise equivalent behavior.
 *
 * @param fed The federate to register through.
 * @param type The type of translator to create /ref HelicsTranslatorTypes.
 * @param name The name of the translator (can be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsTranslator object.
 */
HELICS_EXPORT HelicsTranslator helicsFederateRegisterGlobalTranslator(HelicsFederate fed,
                                                                      HelicsTranslatorTypes type,
                                                                      const char* name,
                                                                      HelicsError* err);

/**
 * Create a source Translator on the specified core.
 *
 * @details Translators can be created through a federate or a core, linking through a federate allows
 *          a few extra features of name matching to function on the federate interface but otherwise equivalent behavior.
 *
 * @param core The core to register through.
 * @param type The type of translator to create /ref HelicsTranslatorTypes.
 * @param name The name of the translator (can be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsTranslator object.
 */
HELICS_EXPORT HelicsTranslator helicsCoreRegisterTranslator(HelicsCore core, HelicsTranslatorTypes type, const char* name, HelicsError* err);


/**
 * Get the number of translators registered through a federate.
 *
 * @param fed The federate object to use to get the translator.
 *
 * @return A count of the number of translators registered through a federate.
 */
HELICS_EXPORT int helicsFederateGetTranslatorCount(HelicsFederate fed);

/**
 * Get a translator by its name, typically already created via registerInterfaces file or something of that nature.
 *
 * @param fed The federate object to use to get the translator.
 * @param name The name of the translator.
 *
 * @param[in,out] err The error object to complete if there is an error.

 *
 * @return A HelicsTranslator object, the object will not be valid and err will contain an error code if no translator with the specified name
 * exists.
 */
HELICS_EXPORT HelicsTranslator helicsFederateGetTranslator(HelicsFederate fed, const char* name, HelicsError* err);
/**
 * Get a translator by its index, typically already created via registerInterfaces file or something of that nature.
 *
 * @param fed The federate object in which to create a publication.
 * @param index The index of the publication to get.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsTranslator, which will be NULL if an invalid index is given.
 */
HELICS_EXPORT HelicsTranslator helicsFederateGetTranslatorByIndex(HelicsFederate fed, int index, HelicsError* err);

/**
 * Check if a translator is valid.
 *
 * @param trans The translator object to check.
 *
 * @return HELICS_TRUE if the Translator object represents a valid translator.
 */
HELICS_EXPORT HelicsBool helicsTranslatorIsValid(HelicsTranslator trans);

/**
 * Get the name of the translator and store in the given string.
 *
 * @param trans The given translator.
 *
 * @return A string with the name of the translator.
 */
HELICS_EXPORT const char* helicsTranslatorGetName(HelicsTranslator trans);

/**
 * Set a property on a translator.
 *
 * @param trans The translator to modify.
 * @param prop A string containing the property to set.
 * @param val A numerical value for the property.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsTranslatorSet(HelicsTranslator trans, const char* prop, double val, HelicsError* err);

/**
 * Set a string property on a translator.
 *
 * @param trans The translator to modify.
 * @param prop A string containing the property to set.
 * @param val A string containing the new value.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsTranslatorSetString(HelicsTranslator trans, const char* prop, const char* val, HelicsError* err);

/**
 * Add an input to send a translator output.
 *
 * @details All messages going to a destination are copied to the delivery address(es).
 * @param trans The given translator to add a destination target to.
 * @param dst The name of the endpoint to add as a destination target.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsTranslatorAddInputTarget(HelicsTranslator trans, const char* input, HelicsError* err);

/**
 * Add a source publication target to a translator.
 *
 * @details All messages coming from a source are copied to the delivery address(es).
 *
 * @param trans The given translator.
 * @param source The name of the endpoint to add as a source target.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsTranslatorAddPublicationTarget(HelicsTranslator trans, const char* pub, HelicsError* err);

/**
 * Add a source endpoint target to a translator.
 *
 * @details All messages coming from a source are copied to the delivery address(es).
 *
 * @param trans The given translator.
 * @param source The name of the endpoint to add as a source target.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsTranslatorAddSourceEndpoint(HelicsTranslator trans, const char* ept, HelicsError* err);

/**
 * Add a destination target endpoint to a translator.
 *
 * @details All messages coming from a source are copied to the delivery address(es).
 *
 * @param trans The given translator.
 * @param source The name of the endpoint to add as a source target.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsTranslatorAddDestinationEndpoint(HelicsTranslator trans, const char* ept, HelicsError* err);

/**
 * \defgroup Clone translator functions
 * @details Functions that manipulate cloning translators in some way.
 * @{
 */

/**
 * Remove a destination target from a translator.
 *
 * @param trans The given translator.
 * @param target The named endpoint to remove as a target.
 *
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 */
HELICS_EXPORT void helicsTranslatorRemoveTarget(HelicsTranslator trans, const char* target, HelicsError* err);

/**
 * Get the data in the info field of a translator.
 *
 * @param trans The given translator.
 *
 * @return A string with the info field string.
 */
HELICS_EXPORT const char* helicsTranslatorGetInfo(HelicsTranslator trans);
/**
 * Set the data in the info field for a translator.
 *
 * @param trans The given translator.
 * @param info The string to set.
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */
HELICS_EXPORT void helicsTranslatorSetInfo(HelicsTranslator trans, const char* info, HelicsError* err);

/**
 * Get the data in a specified tag of a translator.
 *
 * @param trans The translator to query.
 * @param tagname The name of the tag to query.
 * @return A string with the tag data.
 */
HELICS_EXPORT const char* helicsTranslatorGetTag(HelicsTranslator trans, const char* tagname);

/**
 * Set the data in a specific tag for a translator.
 *
 * @param trans The translator object to set the tag for.
 * @param tagname The string to set.
 * @param tagvalue the string value to associate with a tag.
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */
HELICS_EXPORT void helicsTranslatorSetTag(HelicsTranslator trans, const char* tagname, const char* tagvalue, HelicsError* err);

/**
 * Set an option value for a translator.
 *
 * @param trans The given translator.
 * @param option The option to set /ref helics_handle_options.
 * @param value The value of the option commonly 0 for false 1 for true.
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */

HELICS_EXPORT void helicsTranslatorSetOption(HelicsTranslator trans, int option, int value, HelicsError* err);

/**
 * Get a handle option for the translator.
 *
 * @param trans The given translator to query.
 * @param option The option to query /ref helics_handle_options.
 */
HELICS_EXPORT int helicsTranslatorGetOption(HelicsTranslator trans, int option);

/**
 * @}
 */

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif /* HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_*/
