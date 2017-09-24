/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_
#define HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "helics.h"


	HELICS_Export helics_message_filter_federate_id_t helicsCreateMessageFilterFederate(federate_info_t *fi);
	HELICS_Export helics_message_filter_federate_id_t helicsCreateMessageFilterFederateFromFile(const char *);
	//MESSAGE FEDERATE calls

	HELICS_Export helics_source_filter_id_t helicsRegisterSourceFilter(helics_message_filter_federate_id_t fedID, const char *name, const char *inputType, const char *outputType);

	HELICS_Export helics_source_filter_id_t helicsRegisterDestinationFilter(helics_message_filter_federate_id_t fedID, const char *name, const char *inputType, const char *outputType);
	/** check if the federate has any outstanding messages*/
	HELICS_Export bool helicsFederateHasMessageToFilter(helics_message_filter_federate_id_t fedID);
	/* check if a given endpoint has any unread messages*/
	HELICS_Export bool helicsFilterHasMessage(helics_message_filter_federate_id_t id);

	/**
	* Returns the number of pending receives for the specified destination endpoint.
	*/
	HELICS_Export  uint64_t helicsFederateFilterReceiveCount(helics_message_filter_federate_id_t fedID);
	/**
	* Returns the number of pending receives for all endpoints of particular federate.
	*/
	HELICS_Export uint64_t helicsFilterReceiveCount(helics_source_filter_id_t filter);

	/** receive a packet from a particular endpoint
	@param[in] endpoint the identifier for the endpoint
	@return a message object*/
	message_t * helicsFilterGetMessage(helics_source_filter_id_t filter);
	/** receive a communication message for any endpoint in the federate
	@details the return order will be in order of endpoint creation then order of arrival
	all messages for the first endpoint, then all for the second, and so on
	@return a unique_ptr to a Message object containing the message data*/
	message_t * helicsFilterFederateGetMessage(helics_message_filter_federate_id_t fedID);

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif /* HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_*/