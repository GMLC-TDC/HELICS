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


	HELICS_Export helics_message_filter_federate helicsCreateMessageFilterFederate(const helics_federate_info_t fi);
	HELICS_Export helics_message_filter_federate helicsCreateMessageFilterFederateFromFile(const char *filename);
	//MESSAGE FEDERATE calls

	HELICS_Export helics_source_filter helicsRegisterSourceFilter(helics_message_filter_federate fedID, const char *name, const char *inputType, const char *outputType);

	HELICS_Export helics_destination_filter helicsRegisterDestinationFilter(helics_message_filter_federate fedID, const char *name, const char *inputType, const char *outputType);
	/** check if the federate has any outstanding messages*/
	HELICS_Export int helicsFederateHasMessageToFilter(helics_message_filter_federate fedID);
	/* check if a given endpoint has any unread messages*/
	HELICS_Export int helicsFilterHasMessage(helics_message_filter_federate id);

	/**
	* Returns the number of pending receives for the specified destination endpoint.
	*/
	HELICS_Export  int helicsFederateFilterReceiveCount(helics_message_filter_federate fedID);
	/**
	* Returns the number of pending receives for all endpoints of particular federate.
	*/
	HELICS_Export int helicsFilterReceiveCount(helics_source_filter filter);

	/** receive a packet from a particular endpoint
	@param[in] endpoint the identifier for the endpoint
	@return a message object*/
	HELICS_Export message_t helicsFilterGetMessage(helics_source_filter filter);
	/** receive a communication message for any endpoint in the federate
	@details the return order will be in order of endpoint creation then order of arrival
	all messages for the first endpoint, then all for the second, and so on
	@return a unique_ptr to a Message object containing the message data*/
	HELICS_Export message_t helicsFilterFederateGetMessage(helics_message_filter_federate fedID);

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif /* HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_*/