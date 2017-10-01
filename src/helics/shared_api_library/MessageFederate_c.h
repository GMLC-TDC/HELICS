/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_APISHARED_MESSAGE_FEDERATE_FUNCTIONS_H_
#define HELICS_APISHARED_MESSAGE_FEDERATE_FUNCTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "helics.h"


	HELICS_Export helics_message_federate helicsCreateMessageFederate(const federate_info_t *fi);
	HELICS_Export helics_message_federate helicsCreateMessageFederateFromFile(const char *fileName);
	//MESSAGE FEDERATE calls

	HELICS_Export helics_endpoint helicsRegisterEndpoint(helics_message_federate fedID, const char *name, const char *type);

	HELICS_Export helics_endpoint helicsRegisterGlobalEndpoint(helics_message_federate fedID, const char *name, const char *type);

	HELICS_Export helicsStatus helicsSetDefaultDestination(helics_endpoint endpoint, const char *dest);
	HELICS_Export helicsStatus helicsSendMessageRaw(helics_endpoint endpoint, const char *dest, const char *data, uint64_t len);

	HELICS_Export helicsStatus helicsSendEventRaw( helics_endpoint endpoint, const char *dest, const char *data, uint64_t len, helics_time_t time);

	HELICS_Export helicsStatus helicsSendMessage( helics_endpoint endpoint, message_t *message);

	HELICS_Export helicsStatus helicsSubscribe( helics_endpoint endpoint, const char *name, const char *type);
	/** check if the federate has any outstanding messages*/
	HELICS_Export bool helicsFederateHasMessage(helics_message_federate fedID);
	/* check if a given endpoint has any unread messages*/
	HELICS_Export bool helicsEndpointHasMessage(helics_endpoint endpoint);

	/**
	* Returns the number of pending receives for the specified destination endpoint.
	*/
	HELICS_Export  uint64_t helicsFederateReceiveCount(helics_message_federate fedID);
	/**
	* Returns the number of pending receives for all endpoints of particular federate.
	*/
	HELICS_Export uint64_t helicsEndpointReceiveCount(helics_endpoint endpoint);

	/** receive a packet from a particular endpoint
	@param[in] endpoint the identifier for the endpoint
	@return a message object*/
	message_t helicsEndpointGetMessage(helics_endpoint endpoint);
	/** receive a communication message for any endpoint in the federate
	@details the return order will be in order of endpoint creation then order of arrival
	all messages for the first endpoint, then all for the second, and so on
	@return a unique_ptr to a Message object containing the message data*/
	message_t helicsFederateGetMessage(helics_message_federate fedID);

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif /*HELICS_APISHARED_MESSAGE_FEDERATE_FUNCTIONS_H_*/
