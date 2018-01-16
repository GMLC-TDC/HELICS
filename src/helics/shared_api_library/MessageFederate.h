/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_APISHARED_MESSAGE_FEDERATE_FUNCTIONS_H_
#define HELICS_APISHARED_MESSAGE_FEDERATE_FUNCTIONS_H_

#include "helics.h"

#ifdef __cplusplus
extern "C" {
#endif

// MESSAGE FEDERATE calls

HELICS_Export helics_endpoint helicsFederateRegisterEndpoint (helics_federate fed, const char *name, const char *type);

HELICS_Export helics_endpoint helicsFederateRegisterGlobalEndpoint (helics_federate fed, const char *name, const char *type);

HELICS_Export helics_status helicsEndpointSetDefaultDestination (helics_endpoint endpoint, const char *dest);
HELICS_Export helics_status helicsEndpointSendMessageRaw (helics_endpoint endpoint, const char *dest, const char *data, int len);

HELICS_Export helics_status
helicsEndpointSendEventRaw (helics_endpoint endpoint, const char *dest, const char *data, int len, helics_time_t time);

HELICS_Export helics_status helicsEndpointSendMessage (helics_endpoint endpoint, message_t *message);
/** subscribe an endpoint to a publication
@param endpoint the endpoint to use
@param key the name of the publication
@param type the type of the publication that is expected (nullptr or "" for DON'T CARE)
*/
HELICS_Export helics_status helicsEndpointSubscribe (helics_endpoint endpoint, const char *key, const char *type);
/** check if the federate has any outstanding messages*/
HELICS_Export int helicsFederateHasMessage (helics_federate fed);
/* check if a given endpoint has any unread messages*/
HELICS_Export int helicsEndpointHasMessage (helics_endpoint endpoint);

/**
 * Returns the number of pending receives for the specified destination endpoint.
 */
HELICS_Export int helicsFederateReceiveCount (helics_federate fed);
/**
 * Returns the number of pending receives for all endpoints of particular federate.
 */
HELICS_Export int helicsEndpointReceiveCount (helics_endpoint endpoint);

/** receive a packet from a particular endpoint
@param[in] endpoint the identifier for the endpoint
@return a message object*/
HELICS_Export message_t helicsEndpointGetMessage (helics_endpoint endpoint);
/** receive a communication message for any endpoint in the federate
@details the return order will be in order of endpoint creation then order of arrival
all messages for the first endpoint, then all for the second, and so on
@return a unique_ptr to a Message object containing the message data*/
HELICS_Export message_t helicsFederateGetMessage (helics_federate fed);
/** get the type specified for an endpoint
@param endpoint  the endpoint object in question
@param[out] str the location where the string is stored
@param[in] maxlen the maximum string length that can be stored in str
@return a status variable
*/
HELICS_Export helics_status helicsEndpointGetType (helics_endpoint endpoint, char *str, int maxlen);

/** get the name of an endpoint
@param endpoint  the endpoint object in question
@param[out] str the location where the string is stored
@param[in] maxlen the maximum string length that can be stored in str
@return a status variable
*/
HELICS_Export helics_status helicsEndpointGetName (helics_endpoint endpoint, char *str, int maxlen);

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif /*HELICS_APISHARED_MESSAGE_FEDERATE_FUNCTIONS_H_*/
