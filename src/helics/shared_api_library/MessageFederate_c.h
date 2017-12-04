/*
Copyright (C) 2017, Battelle Memorial Institute
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

HELICS_Export helics_message_federate helicsCreateMessageFederate (const helics_federate_info_t fi);
HELICS_Export helics_message_federate helicsCreateMessageFederateFromFile (const char *fileName);
// MESSAGE FEDERATE calls

HELICS_Export helics_endpoint helicsRegisterEndpoint (helics_message_federate fed, const char *name, const char *type);

HELICS_Export helics_endpoint helicsRegisterGlobalEndpoint (helics_message_federate fed, const char *name, const char *type);

HELICS_Export helicsStatus helicsSetDefaultDestination (helics_endpoint endpoint, const char *dest);
HELICS_Export helicsStatus helicsSendMessageRaw (helics_endpoint endpoint, const char *dest, const char *data, int len);

HELICS_Export helicsStatus helicsSendEventRaw (helics_endpoint endpoint, const char *dest, const char *data, int len, helics_time_t time);

HELICS_Export helicsStatus helicsSendMessage (helics_endpoint endpoint, message_t *message);

HELICS_Export helicsStatus helicsSubscribe (helics_endpoint endpoint, const char *name, const char *type);
/** check if the federate has any outstanding messages*/
HELICS_Export int helicsFederateHasMessage (helics_message_federate fed);
/* check if a given endpoint has any unread messages*/
HELICS_Export int helicsEndpointHasMessage (helics_endpoint endpoint);

/**
 * Returns the number of pending receives for the specified destination endpoint.
 */
HELICS_Export int helicsFederateReceiveCount (helics_message_federate fed);
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
HELICS_Export message_t helicsFederateGetMessage (helics_message_federate fed);

HELICS_Export helicsStatus helicsGetEndpointType (helics_endpoint endpoint, char *str, int maxlen);

HELICS_Export helicsStatus helicsGetEndpointName (helics_endpoint endpoint, char *str, int maxlen);

#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif /*HELICS_APISHARED_MESSAGE_FEDERATE_FUNCTIONS_H_*/
