/*
Copyright (c) 2017-2021,
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

#include "helics.h"

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
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return An object containing the endpoint.
 * @forcpponly
 *         nullptr on failure.
 * @endforcpponly
 */
HELICS_EXPORT helics_endpoint helicsFederateRegisterEndpoint(helics_federate fed, const char* name, const char* type, helics_error* err);

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
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 * @return An object containing the endpoint.
 * @forcpponly
 *         nullptr on failure.
 * @endforcpponly
 */
HELICS_EXPORT helics_endpoint helicsFederateRegisterGlobalEndpoint(helics_federate fed,
                                                                   const char* name,
                                                                   const char* type,
                                                                   helics_error* err);

/**
 * Get an endpoint object from a name.
 *
 * @param fed The message federate object to use to get the endpoint.
 * @param name The name of the endpoint.
 * @forcpponly
 * @param[in,out] err The error object to complete if there is an error.
 * @endforcpponly
 *
 * @return A helics_endpoint object.
 * @forcpponly
 *         The object will not be valid and err will contain an error code if no endpoint with the specified name exists.
 * @endforcpponly
 */
HELICS_EXPORT helics_endpoint helicsFederateGetEndpoint(helics_federate fed, const char* name, helics_error* err);

/**
 * Get an endpoint by its index, typically already created via registerInterfaces file or something of that nature.
 *
 * @param fed The federate object in which to create a publication.
 * @param index The index of the publication to get.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @return A helics_endpoint.
 * @forcpponly
 *         It will be NULL if given an invalid index.
 * @endforcpponly
 */
HELICS_EXPORT helics_endpoint helicsFederateGetEndpointByIndex(helics_federate fed, int index, helics_error* err);

/**
 * Check if an endpoint is valid.
 *
 * @param endpoint The endpoint object to check.
 *
 * @return helics_true if the Endpoint object represents a valid endpoint.
 */
HELICS_EXPORT helics_bool helicsEndpointIsValid(helics_endpoint endpoint);

/**
 * Set the default destination for an endpoint if no other endpoint is given.
 *
 * @param endpoint The endpoint to set the destination for.
 * @param dst A string naming the desired default endpoint.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsEndpointSetDefaultDestination(helics_endpoint endpoint, const char* dst, helics_error* err);

/**
 * Get the default destination for an endpoint.
 *
 * @param endpoint The endpoint to set the destination for.
 *
 * @return A string with the default destination.
 */
HELICS_EXPORT const char* helicsEndpointGetDefaultDestination(helics_endpoint endpoint);

/**
 * Send a message to the specified destination.
 *
 * @param endpoint The endpoint to send the data from.
 * @param dst The target destination.
 * @forcpponly
 *             nullptr to use the default destination.
 * @endforcpponly
 * @beginpythononly
 *             "" to use the default destination.
 * @endpythononly
 * @param data The data to send.
 * @forcpponly
 * @param inputDataLength The length of the data to send.
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void
    helicsEndpointSendMessageRaw(helics_endpoint endpoint, const char* dst, const void* data, int inputDataLength, helics_error* err);

/**
 * Send a message at a specific time to the specified destination.
 *
 * @param endpoint The endpoint to send the data from.
 * @param dst The target destination.
 * @forcpponly
 *             nullptr to use the default destination.
 * @endforcpponly
 * @beginpythononly
 *             "" to use the default destination.
 * @endpythononly
 * @param data The data to send.
 * @forcpponly
 * @param inputDataLength The length of the data to send.
 * @endforcpponly
 * @param time The time the message should be sent.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsEndpointSendEventRaw(helics_endpoint endpoint,
                                              const char* dst,
                                              const void* data,
                                              int inputDataLength,
                                              helics_time time,
                                              helics_error* err);

/**
 * Send a message object from a specific endpoint.
 * @deprecated Use helicsEndpointSendMessageObject instead.
 * @param endpoint The endpoint to send the data from.
 * @param message The actual message to send.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */

HELICS_DEPRECATED_EXPORT void helicsEndpointSendMessage(helics_endpoint endpoint, helics_message* message, helics_error* err);

/**
 * Send a message object from a specific endpoint.
 *
 * @param endpoint The endpoint to send the data from.
 * @param message The actual message to send which will be copied.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsEndpointSendMessageObject(helics_endpoint endpoint, helics_message_object message, helics_error* err);

/**
 * Send a message object from a specific endpoint, the message will not be copied and the message object will no longer be valid
 * after the call.
 *
 * @param endpoint The endpoint to send the data from.
 * @param message The actual message to send which will be copied.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsEndpointSendMessageObjectZeroCopy(helics_endpoint endpoint, helics_message_object message, helics_error* err);

/**
 * Subscribe an endpoint to a publication.
 *
 * @param endpoint The endpoint to use.
 * @param key The name of the publication.
 * @forcpponly
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 */
HELICS_EXPORT void helicsEndpointSubscribe(helics_endpoint endpoint, const char* key, helics_error* err);

/**
 * Check if the federate has any outstanding messages.
 *
 * @param fed The federate to check.
 *
 * @return helics_true if the federate has a message waiting, helics_false otherwise.
 */
HELICS_EXPORT helics_bool helicsFederateHasMessage(helics_federate fed);

/**
 * Check if a given endpoint has any unread messages.
 *
 * @param endpoint The endpoint to check.
 *
 * @return helics_true if the endpoint has a message, helics_false otherwise.
 */
HELICS_EXPORT helics_bool helicsEndpointHasMessage(helics_endpoint endpoint);

/**
 * Returns the number of pending receives for the specified destination endpoint.
 *
 * @param fed The federate to get the number of waiting messages from.
 */
HELICS_EXPORT int helicsFederatePendingMessages(helics_federate fed);

/**
 * Returns the number of pending receives for all endpoints of a particular federate.
 *
 * @param endpoint The endpoint to query.
 */
HELICS_EXPORT int helicsEndpointPendingMessages(helics_endpoint endpoint);

/**
 * Receive a packet from a particular endpoint.
 *
 * @deprecated This function is deprecated and will be removed in Helics 3.0.
 *             Use helicsEndpointGetMessageObject instead.
 *
 * @param[in] endpoint The identifier for the endpoint.
 *
 * @return A message object.
 */
HELICS_DEPRECATED_EXPORT helics_message helicsEndpointGetMessage(helics_endpoint endpoint);

/**
 * Receive a packet from a particular endpoint.
 *
 * @param[in] endpoint The identifier for the endpoint.
 *
 * @return A message object.
 */
HELICS_EXPORT helics_message_object helicsEndpointGetMessageObject(helics_endpoint endpoint);

/**
 * Create a new empty message object.
 *
 * @details The message is empty and isValid will return false since there is no data associated with the message yet.
 *
 * @param endpoint The endpoint object to associate the message with.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 *
 * @return A new helics_message_object.
 */
HELICS_EXPORT helics_message_object helicsEndpointCreateMessageObject(helics_endpoint endpoint, helics_error* err);

/**
 * Receive a communication message for any endpoint in the federate.
 *
 * @deprecated This function is deprecated and will be removed in Helics 3.0.
 *             Use helicsFederateGetMessageObject instead.
 *
 * @details The return order will be in order of endpoint creation.
 *          So all messages that are available for the first endpoint, then all for the second, and so on.
 *          Within a single endpoint, the messages are ordered by time, then source_id, then order of arrival.
 *
 * @return A unique_ptr to a Message object containing the message data.
 */
HELICS_DEPRECATED_EXPORT helics_message helicsFederateGetMessage(helics_federate fed);

/**
 * Receive a communication message for any endpoint in the federate.
 *
 * @details The return order will be in order of endpoint creation.
 *          So all messages that are available for the first endpoint, then all for the second, and so on.
 *          Within a single endpoint, the messages are ordered by time, then source_id, then order of arrival.
 *
 * @return A helics_message_object which references the data in the message.
 */
HELICS_EXPORT helics_message_object helicsFederateGetMessageObject(helics_federate fed);

/**
 * Create a new empty message object.
 *
 * @details The message is empty and isValid will return false since there is no data associated with the message yet.
 *
 * @param fed the federate object to associate the message with
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 *
 * @return A helics_message_object containing the message data.
 */
HELICS_EXPORT helics_message_object helicsFederateCreateMessageObject(helics_federate fed, helics_error* err);

/**
 * Clear all stored messages from a federate.
 *
 * @details This clears messages retrieved through helicsFederateGetMessage or helicsFederateGetMessageObject
 *
 * @param fed The federate to clear the message for.
 */
HELICS_EXPORT void helicsFederateClearMessages(helics_federate fed);

/**
 * Clear all message from an endpoint.
 *
 * @deprecated This function does nothing and will be removed.
 *             Use helicsFederateClearMessages to free all messages,
 *             or helicsMessageFree to clear an individual message.
 *
 * @param endpoint The endpoint object to operate on.
 */
HELICS_DEPRECATED_EXPORT void helicsEndpointClearMessages(helics_endpoint endpoint);

/**
 * Get the type specified for an endpoint.
 *
 * @param endpoint The endpoint object in question.
 *
 * @return The defined type of the endpoint.
 */
HELICS_EXPORT const char* helicsEndpointGetType(helics_endpoint endpoint);

/**
 * Get the name of an endpoint.
 *
 * @param endpoint The endpoint object in question.
 *
 * @return The name of the endpoint.
 */
HELICS_EXPORT const char* helicsEndpointGetName(helics_endpoint endpoint);

/**
 * Get the number of endpoints in a federate.
 *
 * @param fed The message federate to query.
 *
 * @return (-1) if fed was not a valid federate, otherwise returns the number of endpoints.
 */
HELICS_EXPORT int helicsFederateGetEndpointCount(helics_federate fed);

/**
 * Get the data in the info field of a filter.
 *
 * @param end The filter to query.
 *
 * @return A string with the info field string.
 */
HELICS_EXPORT const char* helicsEndpointGetInfo(helics_endpoint end);

/**
 * Set the data in the info field for a filter.
 *
 * @param endpoint The endpoint to query.
 * @param info The string to set.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsEndpointSetInfo(helics_endpoint endpoint, const char* info, helics_error* err);

/**
 * Set a handle option on an endpoint.
 *
 * @param endpoint The endpoint to modify.
 * @param option Integer code for the option to set /ref helics_handle_options.
 * @param value The value to set the option to.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsEndpointSetOption(helics_endpoint endpoint, int option, int value, helics_error* err);

/**
 * Set a handle option on an endpoint.
 *
 * @param endpoint The endpoint to modify.
 * @param option Integer code for the option to set /ref helics_handle_options.
 * @return the value of the option, for boolean options will be 0 or 1
 */
HELICS_EXPORT int helicsEndpointGetOption(helics_endpoint endpoint, int option);

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
HELICS_EXPORT const char* helicsMessageGetSource(helics_message_object message);

/**
 * Get the destination endpoint of a message.
 *
 * @param message The message object in question.
 *
 * @return A string with the destination endpoint.
 */
HELICS_EXPORT const char* helicsMessageGetDestination(helics_message_object message);

/**
 * Get the original source endpoint of a message, the source may have been modified by filters or other actions.
 *
 * @param message The message object in question.
 *
 * @return A string with the source of a message.
 */
HELICS_EXPORT const char* helicsMessageGetOriginalSource(helics_message_object message);

/**
 * Get the original destination endpoint of a message, the destination may have been modified by filters or other actions.
 *
 * @param message The message object in question.
 *
 * @return A string with the original destination of a message.
 */
HELICS_EXPORT const char* helicsMessageGetOriginalDestination(helics_message_object message);

/**
 * Get the helics time associated with a message.
 *
 * @param message The message object in question.
 *
 * @return The time associated with a message.
 */
HELICS_EXPORT helics_time helicsMessageGetTime(helics_message_object message);

/**
 * Get the payload of a message as a string.
 *
 * @param message The message object in question.
 *
 * @return A string representing the payload of a message.
 */
HELICS_EXPORT const char* helicsMessageGetString(helics_message_object message);

/**
 * Get the messageID of a message.
 *
 * @param message The message object in question.
 *
 * @return The messageID.
 */
HELICS_EXPORT int helicsMessageGetMessageID(helics_message_object message);

/**
 * Check if a flag is set on a message.
 *
 * @param message The message object in question.
 * @param flag The flag to check should be between [0,15].
 *
 * @return The flags associated with a message.
 */
HELICS_EXPORT helics_bool helicsMessageCheckFlag(helics_message_object message, int flag);

/**
 * Get the size of the data payload in bytes.
 *
 * @param message The message object in question.
 *
 * @return The size of the data payload.
 */
HELICS_EXPORT int helicsMessageGetRawDataSize(helics_message_object message);

/**
 * Get the raw data for a message object.
 *
 * @param message A message object to get the data for.
 * @forcpponly
 * @param[out] data The memory location of the data.
 * @param maxMessageLength The maximum size of information that data can hold.
 * @param[out] actualSize The actual length of data copied to data.
 * @param[in,out] err A pointer to an error object for catching errors.
 * @endforcpponly
 *
 * @beginPythonOnly
 * @return Raw string data.
 * @endPythonOnly
 */
HELICS_EXPORT void
    helicsMessageGetRawData(helics_message_object message, void* data, int maxMessageLength, int* actualSize, helics_error* err);

/**
 * Get a pointer to the raw data of a message.
 *
 * @param message A message object to get the data for.
 *
 * @return A pointer to the raw data in memory, the pointer may be NULL if the message is not a valid message.
 */
HELICS_EXPORT void* helicsMessageGetRawDataPointer(helics_message_object message);

/**
 * A check if the message contains a valid payload.
 *
 * @param message The message object in question.
 *
 * @return helics_true if the message contains a payload.
 */
HELICS_EXPORT helics_bool helicsMessageIsValid(helics_message_object message);

/**
 * Set the source of a message.
 *
 * @param message The message object in question.
 * @param src A string containing the source.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageSetSource(helics_message_object message, const char* src, helics_error* err);

/**
 * Set the destination of a message.
 *
 * @param message The message object in question.
 * @param dst A string containing the new destination.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageSetDestination(helics_message_object message, const char* dst, helics_error* err);

/**
 * Set the original source of a message.
 *
 * @param message The message object in question.
 * @param src A string containing the new original source.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageSetOriginalSource(helics_message_object message, const char* src, helics_error* err);

/**
 * Set the original destination of a message.
 *
 * @param message The message object in question.
 * @param dst A string containing the new original source.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageSetOriginalDestination(helics_message_object message, const char* dst, helics_error* err);

/**
 * Set the delivery time for a message.
 *
 * @param message The message object in question.
 * @param time The time the message should be delivered.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageSetTime(helics_message_object message, helics_time time, helics_error* err);

/**
 * Resize the data buffer for a message.
 *
 * @details The message data buffer will be resized. There are no guarantees on what is in the buffer in newly allocated space.
 *          If the allocated space is not sufficient new allocations will occur.
 *
 * @param message The message object in question.
 * @param newSize The new size in bytes of the buffer.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageResize(helics_message_object message, int newSize, helics_error* err);

/**
 * Reserve space in a buffer but don't actually resize.
 *
 * @details The message data buffer will be reserved but not resized.
 *
 * @param message The message object in question.
 * @param reserveSize The number of bytes to reserve in the message object.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageReserve(helics_message_object message, int reserveSize, helics_error* err);

/**
 * Set the message ID for the message.
 *
 * @details Normally this is not needed and the core of HELICS will adjust as needed.
 *
 * @param message The message object in question.
 * @param messageID A new message ID.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageSetMessageID(helics_message_object message, int32_t messageID, helics_error* err);

/**
 * Clear the flags of a message.
 *
 * @param message The message object in question
 */
HELICS_EXPORT void helicsMessageClearFlags(helics_message_object message);

/**
 * Set a flag on a message.
 *
 * @param message The message object in question.
 * @param flag An index of a flag to set on the message.
 * @param flagValue The desired value of the flag.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageSetFlagOption(helics_message_object message, int flag, helics_bool flagValue, helics_error* err);

/**
 * Set the data payload of a message as a string.
 *
 * @param message The message object in question.
 * @param str A string containing the message data.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageSetString(helics_message_object message, const char* str, helics_error* err);

/**
 * Set the data payload of a message as raw data.
 *
 * @param message The message object in question.
 * @param data A string containing the message data.
 * @param inputDataLength The length of the data to input.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageSetData(helics_message_object message, const void* data, int inputDataLength, helics_error* err);

/**
 * Append data to the payload.
 *
 * @param message The message object in question.
 * @param data A string containing the message data to append.
 * @param inputDataLength The length of the data to input.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageAppendData(helics_message_object message, const void* data, int inputDataLength, helics_error* err);

/**
 * Copy a message object.
 *
 * @param src_message The message object to copy from.
 * @param dst_message The message object to copy to.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageCopy(helics_message_object src_message, helics_message_object dst_message, helics_error* err);

/**
 * Clone a message object.
 *
 * @param message The message object to copy from.
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT helics_message_object helicsMessageClone(helics_message_object message, helics_error* err);

/**
 * Free a message object from memory
 * @param message The message object to copy from.
 * @details memory for message is managed so not using this function does not create memory leaks, this is an indication
 * to the system that the memory for this message is done being used and can be reused for a new message.
 * helicsFederateClearMessages() can also be used to clear up all stored messages at once
 */
HELICS_EXPORT void helicsMessageFree(helics_message_object message);

/**
 * Reset a message to empty state
 * @param message The message object to copy from.
 * @details The message after this function will be empty, with no source or destination
 * @forcpponly
 * @param[in,out] err An error object to fill out in case of an error.
 * @endforcpponly
 */
HELICS_EXPORT void helicsMessageClear(helics_message_object message, helics_error* err);

/**@}*/
#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif /*HELICS_APISHARED_MESSAGE_FEDERATE_FUNCTIONS_H_*/
