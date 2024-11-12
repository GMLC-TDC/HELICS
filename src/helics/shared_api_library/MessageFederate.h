/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

/**
 * @file
 * @brief Functions related to message federates for the C api
 */

#ifndef HELICS_APISHARED_MESSAGE_FEDERATE_FUNCTIONS_H_
#define HELICS_APISHARED_MESSAGE_FEDERATE_FUNCTIONS_H_

#include "helicsCore.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MessageFederate Calls*/

/**
 * Create an endpoint.
 *
 * @details The endpoint becomes part of the federate and is destroyed when the federate is freed
 *          so there are no separate free functions for endpoints.
 *
 * @param fed The federate object in which to create an endpoint must have been created
 *           with helicsCreateMessageFederate or helicsCreateCombinationFederate.
 * @param name The identifier for the endpoint. This will be prepended with the federate name for the global identifier.
 * @param type A string describing the expected type of the publication (may be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return An object containing the endpoint, or nullptr on failure.
 */
HELICS_EXPORT HelicsEndpoint helicsFederateRegisterEndpoint(HelicsFederate fed, const char* name, const char* type, HelicsError* err);

/**
 * Create an endpoint.
 *
 * @details The endpoint becomes part of the federate and is destroyed when the federate is freed
 *          so there are no separate free functions for endpoints.
 *
 * @param fed The federate object in which to create an endpoint must have been created
              with helicsCreateMessageFederate or helicsCreateCombinationFederate.
 * @param name The identifier for the endpoint, the given name is the global identifier.
 * @param type A string describing the expected type of the publication (may be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 * @return An object containing the endpoint, or nullptr on failure.
 */
HELICS_EXPORT HelicsEndpoint helicsFederateRegisterGlobalEndpoint(HelicsFederate fed, const char* name, const char* type, HelicsError* err);

/**
 * Create a targeted endpoint.  Targeted endpoints have specific destinations predefined and do not allow sending messages to other
 * endpoints
 *
 * @details The endpoint becomes part of the federate and is destroyed when the federate is freed
 *          so there are no separate free functions for endpoints.
 *
 * @param fed The federate object in which to create an endpoint must have been created
 *           with helicsCreateMessageFederate or helicsCreateCombinationFederate.
 * @param name The identifier for the endpoint. This will be prepended with the federate name for the global identifier.
 * @param type A string describing the expected type of the publication (may be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return An object containing the endpoint, or nullptr on failure.
 */
HELICS_EXPORT HelicsEndpoint helicsFederateRegisterTargetedEndpoint(HelicsFederate fed,
                                                                    const char* name,
                                                                    const char* type,
                                                                    HelicsError* err);

/**
 * Create a global targeted endpoint, Targeted endpoints have specific destinations predefined and do not allow sending messages to other
 endpoints
 *
 * @details The endpoint becomes part of the federate and is destroyed when the federate is freed
 *          so there are no separate free functions for endpoints.
 *
 * @param fed The federate object in which to create an endpoint must have been created
              with helicsCreateMessageFederate or helicsCreateCombinationFederate.
 * @param name The identifier for the endpoint, the given name is the global identifier.
 * @param type A string describing the expected type of the publication (may be NULL).
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 * @return An object containing the endpoint, or nullptr on failure.
 */
HELICS_EXPORT HelicsEndpoint helicsFederateRegisterGlobalTargetedEndpoint(HelicsFederate fed,
                                                                          const char* name,
                                                                          const char* type,
                                                                          HelicsError* err);

/**
 * Get an endpoint object from a name.
 *
 * @param fed The message federate object to use to get the endpoint.
 * @param name The name of the endpoint.
 *
 * @param[in,out] err The error object to complete if there is an error.

 *
 * @return A HelicsEndpoint object.
 *
 * The object will not be valid and err will contain an error code if no endpoint with the specified name exists.
 */
HELICS_EXPORT HelicsEndpoint helicsFederateGetEndpoint(HelicsFederate fed, const char* name, HelicsError* err);

/**
 * Get an endpoint by its index, typically already created via registerInterfaces file or something of that nature.
 *
 * @param fed The federate object in which to create a publication.
 * @param index The index of the publication to get.
 *
 * @param[in,out] err A pointer to an error object for catching errors.

 *
 * @return A HelicsEndpoint.
 *
 * The HelicsEndpoint returned will be NULL if given an invalid index.
 */
HELICS_EXPORT HelicsEndpoint helicsFederateGetEndpointByIndex(HelicsFederate fed, int index, HelicsError* err);

/**
 * Check if an endpoint is valid.
 *
 * @param endpoint The endpoint object to check.
 *
 * @return HELICS_TRUE if the Endpoint object represents a valid endpoint.
 */
HELICS_EXPORT HelicsBool helicsEndpointIsValid(HelicsEndpoint endpoint);

/**
 * Set the default destination for an endpoint if no other endpoint is given.
 *
 * @param endpoint The endpoint to set the destination for.
 * @param dst A string naming the desired default endpoint.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsEndpointSetDefaultDestination(HelicsEndpoint endpoint, const char* dst, HelicsError* err);

/**
 * Get the default destination for an endpoint.
 *
 * @param endpoint The endpoint to set the destination for.
 *
 * @return A string with the default destination.
 */
HELICS_EXPORT const char* helicsEndpointGetDefaultDestination(HelicsEndpoint endpoint);

/**
 * Send a message to the targeted destination.
 *
 * @param endpoint The endpoint to send the data from.
 * @param message The string to send.
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsEndpointSendString(HelicsEndpoint endpoint, const char* message, HelicsError* err);

/**
* Send a message to the specified destination.
*
* @param endpoint The endpoint to send the data from.

* @param message The string to send.
*
* @param dst The target destination. Use nullptr to send to the default destination.
* @param[in,out] err A pointer to an error object for catching errors.
*/
HELICS_EXPORT void helicsEndpointSendStringTo(HelicsEndpoint endpoint, const char* message, const char* dst, HelicsError* err);

/**
 * Send a message to the specified destination at a specific time.
 *
 * @param endpoint The endpoint to send the data from.
 * @param message The data to send.
 *
 * @param dst The target destination. Use nullptr to send to the default destination.
 *
 * @param time The time the message should be sent.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */

HELICS_EXPORT void
    helicsEndpointSendStringToAt(HelicsEndpoint endpoint, const char* message, const char* dst, HelicsTime time, HelicsError* err);

/**
 * Send a message at a specific time to the targeted destinations.
 *
 * @param endpoint The endpoint to send the data from.
 *
 * @param message The data to send.
 *
 * @param time The time the message should be sent.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */

HELICS_EXPORT void helicsEndpointSendStringAt(HelicsEndpoint endpoint, const char* message, HelicsTime time, HelicsError* err);

/**
 * Send a message to the targeted destination.
 *
 * @param endpoint The endpoint to send the data from.
 * @param data The data to send.
 *
 * @param inputDataLength The length of the data to send.
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsEndpointSendBytes(HelicsEndpoint endpoint, const void* data, int inputDataLength, HelicsError* err);

/**
 * Send a message to the specified destination.
 *
 * @param endpoint The endpoint to send the data from.

 * @param data The data to send.
 *
 * @param inputDataLength The length of the data to send.
 * @param dst The target destination. Use nullptr to send to the default destination.
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void
    helicsEndpointSendBytesTo(HelicsEndpoint endpoint, const void* data, int inputDataLength, const char* dst, HelicsError* err);

/**
 * Send a message to the specified destination at a specific time.
 *
 * @param endpoint The endpoint to send the data from.
 * @param data The data to send.
 *
 * @param inputDataLength The length of the data to send.
 *
 * @param dst The target destination. Use nullptr to send to the default destination.
 *
 * @param time The time the message should be sent.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */

HELICS_EXPORT void helicsEndpointSendBytesToAt(HelicsEndpoint endpoint,
                                               const void* data,
                                               int inputDataLength,
                                               const char* dst,
                                               HelicsTime time,
                                               HelicsError* err);

/**
 * Send a message at a specific time to the targeted destinations
 *
 * @param endpoint The endpoint to send the data from.
 *
 * @param data The data to send.
 *
 * @param inputDataLength The length of the data to send.
 *
 * @param time The time the message should be sent.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */

HELICS_EXPORT void
    helicsEndpointSendBytesAt(HelicsEndpoint endpoint, const void* data, int inputDataLength, HelicsTime time, HelicsError* err);

/**
 * Send a message object from a specific endpoint.
 *
 * @param endpoint The endpoint to send the data from.
 * @param message The actual message to send which will be copied.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsEndpointSendMessage(HelicsEndpoint endpoint, HelicsMessage message, HelicsError* err);

/**
 * Send a message object from a specific endpoint, the message will not be copied and the message object will no longer be valid
 * after the call.
 *
 * @param endpoint The endpoint to send the data from.
 * @param message The actual message to send which will be copied.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsEndpointSendMessageZeroCopy(HelicsEndpoint endpoint, HelicsMessage message, HelicsError* err);

/**
 * Subscribe an endpoint to a publication.
 *
 * @param endpoint The endpoint to use.
 * @param key The name of the publication.
 *
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsEndpointSubscribe(HelicsEndpoint endpoint, const char* key, HelicsError* err);

/**
 * Check if the federate has any outstanding messages.
 *
 * @param fed The federate to check.
 *
 * @return HELICS_TRUE if the federate has a message waiting, HELICS_FALSE otherwise.
 */
HELICS_EXPORT HelicsBool helicsFederateHasMessage(HelicsFederate fed);

/**
 * Check if a given endpoint has any unread messages.
 *
 * @param endpoint The endpoint to check.
 *
 * @return HELICS_TRUE if the endpoint has a message, HELICS_FALSE otherwise.
 */
HELICS_EXPORT HelicsBool helicsEndpointHasMessage(HelicsEndpoint endpoint);

/**
 * Returns the number of pending receives for the specified destination endpoint.
 *
 * @param fed The federate to get the number of waiting messages from.
 */
HELICS_EXPORT int helicsFederatePendingMessageCount(HelicsFederate fed);

/**
 * Returns the number of pending receives for all endpoints of a particular federate.
 *
 * @param endpoint The endpoint to query.
 */
HELICS_EXPORT int helicsEndpointPendingMessageCount(HelicsEndpoint endpoint);

/**
 * Receive a packet from a particular endpoint.
 *
 * @param[in] endpoint The identifier for the endpoint.
 *
 * @return A message object.
 */
HELICS_EXPORT HelicsMessage helicsEndpointGetMessage(HelicsEndpoint endpoint);

/**
 * Create a new empty message object.
 *
 * @details The message is empty and isValid will return false since there is no data associated with the message yet.
 *
 * @param endpoint The endpoint object to associate the message with.
 *
 * @param[in,out] err An error object to fill out in case of an error.

 *
 * @return A new HelicsMessage.
 */
HELICS_EXPORT HelicsMessage helicsEndpointCreateMessage(HelicsEndpoint endpoint, HelicsError* err);

/**
 * Clear all stored messages stored from an endpoint.
 *
 * @details This clears messages retrieved through helicsEndpointGetMessage or helicsEndpointCreateMessage
 *
 * @param endpoint The endpoint to clear the message for.
 */
HELICS_EXPORT void helicsEndpointClearMessages(HelicsEndpoint endpoint);

/**
 * Receive a communication message for any endpoint in the federate.
 *
 * @details The return order will be in order of endpoint creation.
 *          So all messages that are available for the first endpoint, then all for the second, and so on.
 *          Within a single endpoint, the messages are ordered by time, then source_id, then order of arrival.
 *
 * @return A HelicsMessage which references the data in the message.
 */
HELICS_EXPORT HelicsMessage helicsFederateGetMessage(HelicsFederate fed);

/**
 * Create a new empty message object.
 *
 * @details The message is empty and isValid will return false since there is no data associated with the message yet.
 *
 * @param fed the federate object to associate the message with
 *
 * @param[in,out] err An error object to fill out in case of an error.

 *
 * @return A HelicsMessage containing the message data.
 */
HELICS_EXPORT HelicsMessage helicsFederateCreateMessage(HelicsFederate fed, HelicsError* err);

/**
 * Clear all stored messages from a federate.
 *
 * @details This clears messages retrieved through helicsEndpointGetMessage or helicsFederateGetMessage
 *
 * @param fed The federate to clear the message for.
 */
HELICS_EXPORT void helicsFederateClearMessages(HelicsFederate fed);

/**
 * Get the type specified for an endpoint.
 *
 * @param endpoint The endpoint object in question.
 *
 * @return The defined type of the endpoint.
 */
HELICS_EXPORT const char* helicsEndpointGetType(HelicsEndpoint endpoint);

/**
 * Get the name of an endpoint.
 *
 * @param endpoint The endpoint object in question.
 *
 * @return The name of the endpoint.
 */
HELICS_EXPORT const char* helicsEndpointGetName(HelicsEndpoint endpoint);

/**
 * Get the number of endpoints in a federate.
 *
 * @param fed The message federate to query.
 *
 * @return (-1) if fed was not a valid federate, otherwise returns the number of endpoints.
 */
HELICS_EXPORT int helicsFederateGetEndpointCount(HelicsFederate fed);

/**
 * Get the local information field of an endpoint.
 *
 * @param end The endpoint to query.
 *
 * @return A string with the info field string.
 */
HELICS_EXPORT const char* helicsEndpointGetInfo(HelicsEndpoint end);

/**
 * Set the data in the interface information field for an endpoint.
 *
 * @param endpoint The endpoint to set the information for
 * @param info The string to store in the field
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */
HELICS_EXPORT void helicsEndpointSetInfo(HelicsEndpoint endpoint, const char* info, HelicsError* err);

/**
 * Get the data in a specified tag of an endpoint
 *
 * @param endpoint The endpoint to query.
 * @param tagname The name of the tag to query.
 * @return A string with the tag data.
 */
HELICS_EXPORT const char* helicsEndpointGetTag(HelicsEndpoint endpoint, const char* tagname);

/**
 * Set the data in a specific tag for an endpoint.
 *
 * @param endpoint The endpoint to query.
 * @param tagname The string to set.
 * @param tagvalue The string value to associate with a tag.
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */
HELICS_EXPORT void helicsEndpointSetTag(HelicsEndpoint endpoint, const char* tagname, const char* tagvalue, HelicsError* err);

/**
 * Set a handle option on an endpoint.
 *
 * @param endpoint The endpoint to modify.
 * @param option Integer code for the option to set /ref helics_handle_options.
 * @param value The value to set the option to.
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */
HELICS_EXPORT void helicsEndpointSetOption(HelicsEndpoint endpoint, int option, int value, HelicsError* err);

/**
 * Set a handle option on an endpoint.
 *
 * @param endpoint The endpoint to modify.
 * @param option Integer code for the option to set /ref helics_handle_options.
 * @return the value of the option, for boolean options will be 0 or 1
 */
HELICS_EXPORT int helicsEndpointGetOption(HelicsEndpoint endpoint, int option);

/**
 * add a source target to an endpoint,  Specifying an endpoint to receive undirected messages from
 *
 * @param endpoint The endpoint to modify.
 * @param targetEndpoint the endpoint to get messages from
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */
HELICS_EXPORT void helicsEndpointAddSourceTarget(HelicsEndpoint endpoint, const char* targetEndpoint, HelicsError* err);

/**
 * add a destination target to an endpoint,  Specifying an endpoint to send undirected messages to
 *
 * @param endpoint The endpoint to modify.
 * @param targetEndpoint the name of the endpoint to send messages to
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */
HELICS_EXPORT void helicsEndpointAddDestinationTarget(HelicsEndpoint endpoint, const char* targetEndpoint, HelicsError* err);

/**
 * remove an endpoint from being targeted
 *
 * @param endpoint The endpoint to modify.
 * @param targetEndpoint the name of the endpoint to send messages to
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */
HELICS_EXPORT void helicsEndpointRemoveTarget(HelicsEndpoint endpoint, const char* targetEndpoint, HelicsError* err);

/**
 * add a source Filter to an endpoint
 *
 * @param endpoint The endpoint to modify.
 * @param filterName the name of the filter to add
 *
 * @param[in,out] err An error object to fill out in case of an error.

 */
HELICS_EXPORT void helicsEndpointAddSourceFilter(HelicsEndpoint endpoint, const char* filterName, HelicsError* err);

/**
 * add a destination filter to an endpoint
 *
 * @param endpoint The endpoint to modify.
 * @param filterName The name of the filter to add.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsEndpointAddDestinationFilter(HelicsEndpoint endpoint, const char* filterName, HelicsError* err);

/**
 * \defgroup Message operation functions
 * @details Functions for working with helics message envelopes.
 * @{
 */

/**
 * Get the source endpoint of a message.
 *
 * @param message The message object in question.
 *
 * @return A string with the source endpoint.
 */
HELICS_EXPORT const char* helicsMessageGetSource(HelicsMessage message);

/**
 * Get the destination endpoint of a message.
 *
 * @param message The message object in question.
 *
 * @return A string with the destination endpoint.
 */
HELICS_EXPORT const char* helicsMessageGetDestination(HelicsMessage message);

/**
 * Get the original source endpoint of a message, the source may have been modified by filters or other actions.
 *
 * @param message The message object in question.
 *
 * @return A string with the source of a message.
 */
HELICS_EXPORT const char* helicsMessageGetOriginalSource(HelicsMessage message);

/**
 * Get the original destination endpoint of a message, the destination may have been modified by filters or other actions.
 *
 * @param message The message object in question.
 *
 * @return A string with the original destination of a message.
 */
HELICS_EXPORT const char* helicsMessageGetOriginalDestination(HelicsMessage message);

/**
 * Get the helics time associated with a message.
 *
 * @param message The message object in question.
 *
 * @return The time associated with a message.
 */
HELICS_EXPORT HelicsTime helicsMessageGetTime(HelicsMessage message);

/**
 * Get the payload of a message as a string.
 *
 * @param message The message object in question.
 *
 * @return A string representing the payload of a message.
 */
HELICS_EXPORT const char* helicsMessageGetString(HelicsMessage message);

/**
 * Get the messageID of a message.
 *
 * @param message The message object in question.
 *
 * @return The messageID.
 */
HELICS_EXPORT int helicsMessageGetMessageID(HelicsMessage message);

/**
 * Check if a flag is set on a message.
 *
 * @param message The message object in question.
 * @param flag The flag to check should be between [0,15].
 *
 * @return The flags associated with a message.
 */
HELICS_EXPORT HelicsBool helicsMessageGetFlagOption(HelicsMessage message, int flag);

/**
 * Get the size of the data payload in bytes.
 *
 * @param message The message object in question.
 *
 * @return The size of the data payload.
 */
HELICS_EXPORT int helicsMessageGetByteCount(HelicsMessage message);

/**
 * Get the raw data for a message object.
 *
 * @param message A message object to get the data for.
 *
 * @param[out] data The memory location of the data.
 * @param maxMessageLength The maximum size of information that data can hold.
 * @param[out] actualSize The actual length of data copied to data.
 * @param[in,out] err A pointer to an error object for catching errors.
 */
HELICS_EXPORT void helicsMessageGetBytes(HelicsMessage message, void* data, int maxMessageLength, int* actualSize, HelicsError* err);

/**
 * Get a pointer to the raw data of a message.
 *
 * @param message A message object to get the data for.
 *
 * @return A pointer to the raw data in memory, the pointer may be NULL if the message is not a valid message.
 */
HELICS_EXPORT void* helicsMessageGetBytesPointer(HelicsMessage message);

/**
 * Get a data buffer to the message object
 *
 * @param message A message object to get the dataBuffer for
 *
 * @return A HelicsDataBuffer object to the data in a message.  Modifying the buffer will directly modify the message contents.
 */
HELICS_EXPORT HelicsDataBuffer helicsMessageDataBuffer(HelicsMessage message, HelicsError* err);

/**
 * A check if the message contains a valid payload.
 *
 * @param message The message object in question.
 *
 * @return HELICS_TRUE if the message contains a payload.
 */
HELICS_EXPORT HelicsBool helicsMessageIsValid(HelicsMessage message);

/**
 * Set the source of a message.
 *
 * @param message The message object in question.
 * @param src A string containing the source.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageSetSource(HelicsMessage message, const char* src, HelicsError* err);

/**
 * Set the destination of a message.
 *
 * @param message The message object in question.
 * @param dst A string containing the new destination.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageSetDestination(HelicsMessage message, const char* dst, HelicsError* err);

/**
 * Set the original source of a message.
 *
 * @param message The message object in question.
 * @param src A string containing the new original source.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageSetOriginalSource(HelicsMessage message, const char* src, HelicsError* err);

/**
 * Set the original destination of a message.
 *
 * @param message The message object in question.
 * @param dst A string containing the new original source.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageSetOriginalDestination(HelicsMessage message, const char* dst, HelicsError* err);

/**
 * Set the delivery time for a message.
 *
 * @param message The message object in question.
 * @param time The time the message should be delivered.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageSetTime(HelicsMessage message, HelicsTime time, HelicsError* err);

/**
 * Resize the data buffer for a message.
 *
 * @details The message data buffer will be resized. There are no guarantees on what is in the buffer in newly allocated space.
 *          If the allocated space is not sufficient new allocations will occur.
 *
 * @param message The message object in question.
 * @param newSize The new size in bytes of the buffer.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageResize(HelicsMessage message, int newSize, HelicsError* err);

/**
 * Reserve space in a buffer but don't actually resize.
 *
 * @details The message data buffer will be reserved but not resized.
 *
 * @param message The message object in question.
 * @param reserveSize The number of bytes to reserve in the message object.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageReserve(HelicsMessage message, int reserveSize, HelicsError* err);

/**
 * Set the message ID for the message.
 *
 * @details Normally this is not needed and the core of HELICS will adjust as needed.
 *
 * @param message The message object in question.
 * @param messageID A new message ID.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageSetMessageID(HelicsMessage message, int32_t messageID, HelicsError* err);

/**
 * Clear the flags of a message.
 *
 * @param message The message object in question
 */
HELICS_EXPORT void helicsMessageClearFlags(HelicsMessage message);

/**
 * Set a flag on a message.
 *
 * @param message The message object in question.
 * @param flag An index of a flag to set on the message.
 * @param flagValue The desired value of the flag.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageSetFlagOption(HelicsMessage message, int flag, HelicsBool flagValue, HelicsError* err);

/**
 * Set the data payload of a message as a string.
 *
 * @param message The message object in question.
 * @param data A null terminated string containing the message data.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageSetString(HelicsMessage message, const char* data, HelicsError* err);

/**
 * Set the data payload of a message as raw data.
 *
 * @param message The message object in question.
 * @param data A string containing the message data.
 * @param inputDataLength The length of the data to input.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageSetData(HelicsMessage message, const void* data, int inputDataLength, HelicsError* err);

/**
 * Set the data payload of a message from a HelicsDataBuffer Object
 *
 * @param message The message object in question.
 * @param data the dataBuffer containing the appropriate data, if null will clear the message payload
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageSetDataBuffer(HelicsMessage message, HelicsDataBuffer data, HelicsError* err);

/**
 * Append data to the payload.
 *
 * @param message The message object in question.
 * @param data A string containing the message data to append.
 * @param inputDataLength The length of the data to input.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageAppendData(HelicsMessage message, const void* data, int inputDataLength, HelicsError* err);

/**
 * Copy a message object.
 *
 * @param src_message The message object to copy from.
 * @param dst_message The message object to copy to.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageCopy(HelicsMessage src_message, HelicsMessage dst_message, HelicsError* err);

/**
 * Clone a message object.
 *
 * @param message The message object to copy from.
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT HelicsMessage helicsMessageClone(HelicsMessage message, HelicsError* err);

/**
 * Free a message object from memory
 * @param message The message object to copy from.
 * @details memory for message is managed so not using this function does not create memory leaks, this is an indication
 * to the system that the memory for this message is done being used and can be reused for a new message.
 * helicsFederateClearMessages() can also be used to clear up all stored messages at once
 */
HELICS_EXPORT void helicsMessageFree(HelicsMessage message);

/**
 * Reset a message to empty state
 * @param message The message object to copy from.
 * @details The message after this function will be empty, with no source or destination
 *
 * @param[in,out] err An error object to fill out in case of an error.
 */
HELICS_EXPORT void helicsMessageClear(HelicsMessage message, HelicsError* err);

/**@}*/
#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif /*HELICS_APISHARED_MESSAGE_FEDERATE_FUNCTIONS_H_*/
