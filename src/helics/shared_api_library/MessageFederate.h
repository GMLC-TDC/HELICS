/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
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
@return an object containing the endpoint, nullptr on failure
*/
HELICS_EXPORT helics_endpoint helicsFederateRegisterEndpoint (helics_federate fed, const char *name, const char *type);

/** create an endpoint
@details the endpoint becomes part of the federate and is destroyed when the federate is freed so there are no separate free functions
for endpoints
@param fed the federate object in which to create an endpoint must have been create with helicsCreateMessageFederate or
helicsCreateCombinationFederate
@param name the identifier for the endpoint,  the given name is the global identifier
@param type a string describing the expected type of the publication may be NULL
@return an object containing the endpoint, nullptr on failure
*/
HELICS_EXPORT helics_endpoint helicsFederateRegisterGlobalEndpoint (helics_federate fed, const char *name, const char *type);

/** set the default destination for an endpoint if no other endpoint is given
@param endpoint the endpoint to set the destination for
@param dest a string naming the desired default endpoint
*/
HELICS_EXPORT void helicsEndpointSetDefaultDestination (helics_endpoint endpoint, const char *dest);

/** send a message to the specified destination
@param endpoint the endpoint to send the data from
@param dest the target destination (nullptr to use the default destination
@param data the data to send
@param len the length of the data to send
@return helics_ok if everything worked
*/
HELICS_EXPORT void helicsEndpointSendMessageRaw (helics_endpoint endpoint,
                                                          const char *dest,
                                                          const void *data,
                                                          int inputDataLength);

/** send a message at a specific time to the specified destination
@param endpoint the endpoint to send the data from
@param dest the target destination (nullptr to use the default destination
@param data the data to send
@param len the length of the data to send
@param time the time the message should be sent
@return helics_ok if everything worked
*/
HELICS_EXPORT void
helicsEndpointSendEventRaw (helics_endpoint endpoint, const char *dest, const void *data, int inputDataLength, helics_time_t time);

/** send a message object from a specific endpoint
@param endpoint the endpoint to send the data from
@param message the actual message to send
@return helics_ok if everything worked
*/
HELICS_EXPORT void helicsEndpointSendMessage (helics_endpoint endpoint, message_t *message);

/** subscribe an endpoint to a publication
@param endpoint the endpoint to use
@param key the name of the publication
*/
HELICS_EXPORT void helicsEndpointSubscribe (helics_endpoint endpoint, const char *key);
/** check if the federate has any outstanding messages
@param fed the federate to check if it has
@return helics_true if the federate has a message waiting false otherwise*/
HELICS_EXPORT helics_bool_t helicsFederateHasMessage (helics_federate fed);
/** check if a given endpoint has any unread messages
@param endpoint the endpoint to check
@return helics_true if the endpoint has a message, false otherwise*/
HELICS_EXPORT helics_bool_t helicsEndpointHasMessage (helics_endpoint endpoint);

/**
 * Returns the number of pending receives for the specified destination endpoint.
 @param fed the federate to get the number of waiting messages
 */
HELICS_EXPORT int helicsFederatePendingMessages (helics_federate fed);
/**
 * Returns the number of pending receives for all endpoints of particular federate.
 @param endpoint the endpoint to query
 */
HELICS_EXPORT int helicsEndpointPendingMessages (helics_endpoint endpoint);

/** receive a packet from a particular endpoint
@param[in] endpoint the identifier for the endpoint
@return a message object*/
HELICS_EXPORT message_t helicsEndpointGetMessage (helics_endpoint endpoint);

/** receive a communication message for any endpoint in the federate
@details the return order will be in order of endpoint creation then order of arrival
all messages for the first endpoint, then all for the second, and so on
within a single endpoint the messages are ordered by time, then source_id, then order of arrival
@return a unique_ptr to a Message object containing the message data*/
HELICS_EXPORT message_t helicsFederateGetMessage (helics_federate fed);

/** get the type specified for an endpoint
@param endpoint  the endpoint object in question
@param[out] outputString the location where the string is stored
@param[in] maxlen the maximum string length that can be stored in str
@return a status variable
*/
HELICS_EXPORT void helicsEndpointGetType (helics_endpoint endpoint, char *outputString, int maxlen);

/** get the name of an endpoint
@param endpoint  the endpoint object in question
@param[out] outputString the location where the string is stored
@param[in] maxlen the maximum string length that can be stored in str
@return a status variable
*/
HELICS_EXPORT void helicsEndpointGetName (helics_endpoint endpoint, char *outputString, int maxlen);

/** get the number of endpoints in a federate
@return (-1) if fed was not a valid federate otherwise returns the number of subscriptions*/
HELICS_EXPORT int helicsFederateGetEndpointCount (helics_federate fed);

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif /*HELICS_APISHARED_MESSAGE_FEDERATE_FUNCTIONS_H_*/
