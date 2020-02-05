/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

/** @file
@brief functions related the message federates for the C api
*/

#ifndef HELICS_APISHARED_MESSAGE_FEDERATE_FUNCTIONS_H_
#define HELICS_APISHARED_MESSAGE_FEDERATE_FUNCTIONS_H_

#include "helics.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MESSAGE FEDERATE calls*/

/** create an endpoint
    @details the endpoint becomes part of the federate and is destroyed when the federate is freed so there are no separate free functions
    for endpoints
    @param fed the federate object in which to create an endpoint must have been create with helicsCreateMessageFederate or
    helicsCreateCombinationFederate
    @param name the identifier for the endpoint,  this will be prepended with the federate name for the global identifier
    @param type a string describing the expected type of the publication may be NULL
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    @return an object containing the endpoint, nullptr on failure
    */
HELICS_EXPORT helics_endpoint helicsFederateRegisterEndpoint(helics_federate fed, const char* name, const char* type, helics_error* err);

/** create an endpoint
    @details the endpoint becomes part of the federate and is destroyed when the federate is freed so there are no separate free functions
    for endpoints
    @param fed the federate object in which to create an endpoint must have been create with helicsCreateMessageFederate or
    helicsCreateCombinationFederate
    @param name the identifier for the endpoint,  the given name is the global identifier
    @param type a string describing the expected type of the publication may be NULL
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    @return an object containing the endpoint, nullptr on failure
    */
HELICS_EXPORT helics_endpoint
    helicsFederateRegisterGlobalEndpoint(helics_federate fed, const char* name, const char* type, helics_error* err);

/** get an endpoint object from a name
    @param fed the message federate object to use to get the endpoint
    @param name the name of the endpoint
    @forcpponly
    @param[in,out] err the error object to complete if there is an error
    @endforcpponly
    @return a helics_endpoint object, the object will not be valid and err will contain an error code if no endpoint with the specified
    name exists
    */
HELICS_EXPORT helics_endpoint helicsFederateGetEndpoint(helics_federate fed, const char* name, helics_error* err);
/** get an endpoint by its index typically already created via registerInterfaces file or something of that nature

    @param fed the federate object in which to create a publication
    @param index the index of the publication to get
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    @return a helics_endpoint, which will be NULL if an invalid index
    */
HELICS_EXPORT helics_endpoint helicsFederateGetEndpointByIndex(helics_federate fed, int index, helics_error* err);

/** set the default destination for an endpoint if no other endpoint is given
    @param endpoint the endpoint to set the destination for
    @param dest a string naming the desired default endpoint
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    */
HELICS_EXPORT void helicsEndpointSetDefaultDestination(helics_endpoint endpoint, const char* dest, helics_error* err);

/** get the default destination for an endpoint
    @param endpoint the endpoint to set the destination for
    @return a string with the default destination
    */
HELICS_EXPORT const char* helicsEndpointGetDefaultDestination(helics_endpoint endpoint);

/** send a message to the specified destination
    @param endpoint the endpoint to send the data from
    @param dest the target destination (nullptr to use the default destination)
    @param data the data to send
    @param inputDataLength the length of the data to send
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    */
HELICS_EXPORT void
    helicsEndpointSendMessageRaw(helics_endpoint endpoint, const char* dest, const void* data, int inputDataLength, helics_error* err);

/** send a message at a specific time to the specified destination
    @param endpoint the endpoint to send the data from
    @param dest the target destination (nullptr to use the default destination
    @param data the data to send
    @param inputDataLength the length of the data to send
    @param time the time the message should be sent
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    */
HELICS_EXPORT void helicsEndpointSendEventRaw(
    helics_endpoint endpoint,
    const char* dest,
    const void* data,
    int inputDataLength,
    helics_time time,
    helics_error* err);

/** send a message object from a specific endpoint
    @param endpoint the endpoint to send the data from
    @param message the actual message to send
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    */
HELICS_EXPORT void helicsEndpointSendMessage(helics_endpoint endpoint, helics_message* message, helics_error* err);
/** send a message object from a specific endpoint
    @param endpoint the endpoint to send the data from
    @param message the actual message to send
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    */
HELICS_EXPORT void helicsEndpointSendMessageObject(helics_endpoint endpoint, helics_message_object message, helics_error* err);

/** subscribe an endpoint to a publication
    @param endpoint the endpoint to use
    @param key the name of the publication
    @forcpponly
    @param[in,out] err a pointer to an error object for catching errors
    @endforcpponly
    */
HELICS_EXPORT void helicsEndpointSubscribe(helics_endpoint endpoint, const char* key, helics_error* err);
/** check if the federate has any outstanding messages
    @param fed the federate to check if it has
    @return helics_true if the federate has a message waiting helics_false otherwise*/
HELICS_EXPORT helics_bool helicsFederateHasMessage(helics_federate fed);
/** check if a given endpoint has any unread messages
    @param endpoint the endpoint to check
    @return helics_true if the endpoint has a message, helics_false otherwise*/
HELICS_EXPORT helics_bool helicsEndpointHasMessage(helics_endpoint endpoint);

/**
     * Returns the number of pending receives for the specified destination endpoint.
     @param fed the federate to get the number of waiting messages
     */
HELICS_EXPORT int helicsFederatePendingMessages(helics_federate fed);
/**
     * Returns the number of pending receives for all endpoints of particular federate.
     @param endpoint the endpoint to query
     */
HELICS_EXPORT int helicsEndpointPendingMessages(helics_endpoint endpoint);

/** receive a packet from a particular endpoint
    @param[in] endpoint the identifier for the endpoint
    @return a message object*/
HELICS_EXPORT helics_message helicsEndpointGetMessage(helics_endpoint endpoint);

/** receive a packet from a particular endpoint
    @param[in] endpoint the identifier for the endpoint
    @return a message object*/
HELICS_EXPORT helics_message_object helicsEndpointGetMessageObject(helics_endpoint endpoint);

/** receive a communication message for any endpoint in the federate
    @details the return order will be in order of endpoint creation.
    So all messages that are available for the first endpoint, then all for the second, and so on
    within a single endpoint the messages are ordered by time, then source_id, then order of arrival
    @return a unique_ptr to a Message object containing the message data*/
HELICS_EXPORT helics_message helicsFederateGetMessage(helics_federate fed);

/** receive a communication message for any endpoint in the federate
     @details the return order will be in order of endpoint creation.
    So all messages that are available for the first endpoint, then all for the second, and so on
    within a single endpoint the messages are ordered by time, then source_id, then order of arrival
    @return a helics_message_object which references the data in the message*/
HELICS_EXPORT helics_message_object helicsFederateGetMessageObject(helics_federate fed);

/** create a new empty message object
    @details, the message is empty and isValid will return false since there is no data associated with the message yet.
    @return a helics_message_object containing the message data*/
HELICS_EXPORT helics_message_object helicsFederateCreateMessageObject(helics_federate fed, helics_error* err);

/** clear all stored messages from a federate
    @details this clears messages retrieved through helicsFederateGetMessage or helicsFederateGetMessageObject
    @param fed the federate to clear the message for
    */
HELICS_EXPORT void helicsFederateClearMessages(helics_federate fed);
/** clear all message from an endpoint
    @param endpoint  the endpoint object to operate on
    */
HELICS_EXPORT void helicsEndpointClearMessages(helics_endpoint endpoint);

/** get the type specified for an endpoint
    @param endpoint  the endpoint object in question
    @return the defined type of the endpoint
    */
HELICS_EXPORT const char* helicsEndpointGetType(helics_endpoint endpoint);

/** get the name of an endpoint
    @param endpoint  the endpoint object in question
    @return the name of the endpoint
    */
HELICS_EXPORT const char* helicsEndpointGetName(helics_endpoint endpoint);

/** get the number of endpoints in a federate
    @param fed the message federate to query
    @return (-1) if fed was not a valid federate otherwise returns the number of endpoints*/
HELICS_EXPORT int helicsFederateGetEndpointCount(helics_federate fed);

/** get the data in the info field of an filter
    @param end the filter to query
    @return a string with the info field string*/
HELICS_EXPORT const char* helicsEndpointGetInfo(helics_endpoint end);

/** set the data in the info field for an filter
    @param end the endpoint to query
    @param info the string to set
    @forcpponly
    @param[in,out] err an error object to fill out in case of an error
    @endforcpponly
    */
HELICS_EXPORT void helicsEndpointSetInfo(helics_endpoint end, const char* info, helics_error* err);

/** set a handle option on an endpoint
    @param end the endpoint to modify
    @param option integer code for the option to set /ref helics_handle_options
    @param value the value to set the option
    @forcpponly
    @param[in,out] err an error object to fill out in case of an error
    @endforcpponly
    */
HELICS_EXPORT void helicsEndpointSetOption(helics_endpoint end, int option, helics_bool value, helics_error* err);
/** set a handle option on an endpoint
    @param end the endpoint to modify
    @param option integer code for the option to set /ref helics_handle_options
    */
HELICS_EXPORT helics_bool helicsEndpointGetOption(helics_endpoint end, int option);

/**
    * \defgroup message operation functions
    @details functions for working with helics message envelopes
    * @{
    */
/** get the source endpoint of a message
    @param message the message object in question
    @return a string with the source endpoint
    */
HELICS_EXPORT const char* helicsMessageGetSource(helics_message_object message);
/** get the destination endpoint of a message
    @param message the message object in question
    @return a string with the destination endpoint
    */
HELICS_EXPORT const char* helicsMessageGetDestination(helics_message_object message);
/** get the original source endpoint of a message, the source may have modified by filters or other actions
    @param message the message object in question
    @return a string with the source of a message
    */
HELICS_EXPORT const char* helicsMessageGetOriginalSource(helics_message_object message);
/** get the original destination endpoint of a message, the destination may have been modified by filters or other actions
    @param message the message object in question
    @return a string with the original destination of a message
    */
HELICS_EXPORT const char* helicsMessageGetOriginalDestination(helics_message_object message);
/** get the helics time associated with a message
    @param message the message object in question
    @return the time associated with a message
    */
HELICS_EXPORT helics_time helicsMessageGetTime(helics_message_object message);
/** get the payload of a message as a string
    @param message the message object in question
    @return a string representing the payload of a message
    */
HELICS_EXPORT const char* helicsMessageGetString(helics_message_object message);
/** get the messageID of a message
    @param message the message object in question
    @return the messageID
    */
HELICS_EXPORT int helicsMessageGetMessageID(helics_message_object message);
/** check if a flag is set on a message
    @param message the message object in question
    @param flag the flag to check should be between [0,15]
    @return the flags associated with a message
    */
HELICS_EXPORT helics_bool helicsMessageCheckFlag(helics_message_object message, int flag);
/** get the size of the data payload in bytes
    @param message the message object in question
    @return the size of the data payload
    */
HELICS_EXPORT int helicsMessageGetRawDataSize(helics_message_object message);

/** get the raw data for a message object
    @param message a message object to get the data for
    @param[out] data the memory location of the data
    @param maxMessagelen the maximum size of information that data can hold
    @param[out] actualSize  the actual length of data copied to data
    @forcpponly
    @param[in,out] err a pointer to an error object for catching erro
    @endforcpponly
    */
HELICS_EXPORT void
    helicsMessageGetRawData(helics_message_object message, void* data, int maxMessagelen, int* actualSize, helics_error* err);

/** get a pointer to the raw data of a message
    @param message a message object to get the data for
    @return a pointer to the raw data in memory, the pointer may be NULL if the message is not a valid message
    */
HELICS_EXPORT void* helicsMessageGetRawDataPointer(helics_message_object message);
/** a check if the message contains a valid payload
    @param message the message object in question
    @return true if the message contains a payload
    */
HELICS_EXPORT helics_bool helicsMessageIsValid(helics_message_object message);
/** set the source of a message
    @param message the message object in question
    @param src a string containing the source
    @forcpponly
    @param[in,out] err an error object to fill out in case of an error
    @endforcpponly
    */
HELICS_EXPORT void helicsMessageSetSource(helics_message_object message, const char* src, helics_error* err);
/** set the destination of a message
    @param message the message object in question
    @param dest a string containing the new destination
    @forcpponly
    @param[in,out] err an error object to fill out in case of an error
    @endforcpponly
    */
HELICS_EXPORT void helicsMessageSetDestination(helics_message_object message, const char* dest, helics_error* err);
/** set the original source of a message
    @param message the message object in question
    @param src a string containing the new original source
    @forcpponly
    @param[in,out] err an error object to fill out in case of an error
    @endforcpponly
    */
HELICS_EXPORT void helicsMessageSetOriginalSource(helics_message_object message, const char* src, helics_error* err);
/** set the original destination of a message
    @param message the message object in question
    @param dest a string containing the new original source
    @forcpponly
    @param[in,out] err an error object to fill out in case of an error
    @endforcpponly
    */
HELICS_EXPORT void helicsMessageSetOriginalDestination(helics_message_object message, const char* dest, helics_error* err);
/** set the delivery time for a message
    @param message the message object in question
    @param time the time the message should be delivered
    @forcpponly
    @param[in,out] err an error object to fill out in case of an error
    @endforcpponly
    */
HELICS_EXPORT void helicsMessageSetTime(helics_message_object message, helics_time time, helics_error* err);
/** resize the data buffer for a message
    @details the message data buffer will be resized there is no guarantees on what is in the buffer in newly allocated space
    if the allocated space is not sufficient new allocations will occur
    @param message the message object in question
    @param newSize the new size in bytes of the buffer
    @forcpponly
    @param[in,out] err an error object to fill out in case of an error
    @endforcpponly
    */
HELICS_EXPORT void helicsMessageResize(helics_message_object message, int newSize, helics_error* err);
/** reserve space in a buffer but don't actually resize
    @details the message data buffer will be reserved but not resized
    @param message the message object in question
    @param reserveSize the number of bytes to reserve in the message object
    @forcpponly
    @param[in,out] err an error object to fill out in case of an error
    @endforcpponly
    */
HELICS_EXPORT void helicsMessageReserve(helics_message_object message, int reserveSize, helics_error* err);
/** set the message ID for the message
    @details normally this is not needed and the core of HELICS will adjust as needed
    @param message the message object in question
    @param messageID a new message ID
    @forcpponly
    @param[in,out] err an error object to fill out in case of an error
    @endforcpponly
    */
HELICS_EXPORT void helicsMessageSetMessageID(helics_message_object message, int32_t messageID, helics_error* err);
/** clear the flags of a message
    @param message the message object in question*/
HELICS_EXPORT void helicsMessageClearFlags(helics_message_object message);
/** set a flag on a message
    @param message the message object in question
    @param flag an index of a flag to set on the message
    @param flagValue the desired value of the flag
    @forcpponly
    @param[in,out] err an error object to fill out in case of an error
    @endforcpponly
    */
HELICS_EXPORT void helicsMessageSetFlagOption(helics_message_object message, int flag, helics_bool flagValue, helics_error* err);
/** set the data payload of a message as a string
    @param message the message object in question
    @param str a string containing the message data
    @forcpponly
    @param[in,out] err an error object to fill out in case of an error
    @endforcpponly
    */
HELICS_EXPORT void helicsMessageSetString(helics_message_object message, const char* str, helics_error* err);
/** set the data payload of a message as raw data
    @param message the message object in question
    @param data a string containing the message data
    @param inputDataLength  the length of the data to input
    @forcpponly
    @param[in,out] err an error object to fill out in case of an error
    @endforcpponly
    */
HELICS_EXPORT void helicsMessageSetData(helics_message_object message, const void* data, int inputDataLength, helics_error* err);
/** append data to the payload
    @param message the message object in question
    @param data a string containing the message data to append
    @param inputDataLength  the length of the data to input
    @forcpponly
    @param[in,out] err an error object to fill out in case of an error
    @endforcpponly
    */
HELICS_EXPORT void helicsMessageAppendData(helics_message_object message, const void* data, int inputDataLength, helics_error* err);

/**@}*/
#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif /*HELICS_APISHARED_MESSAGE_FEDERATE_FUNCTIONS_H_*/
