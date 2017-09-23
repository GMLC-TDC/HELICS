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


	HELICS_Export helics_message_federate_id_t helicsCreateMessageFederate(federate_info_t *fi);
	HELICS_Export helics_message_federate_id_t helicsCreateMessageFederateFromFile(const char *);
	//MESSAGE FEDERATE calls

	HELICS_Export helics_endpoint_id_t helicsRegisterEndpoint(helics_message_federate_id_t fedID, const char *name, const char *type, const char *units);

	HELICS_Export helics_endpoint_id_t helicsRegisterGlobalEndpoint(helics_message_federate_id_t fedID, const char *name, const char *type, const char *units);

	HELICS_Export helicsStatus helicsSendMessageRaw(helics_message_federate_id_t fedID, helics_endpoint_id_t endpoint, const char *dest, const char *data, uint64_t len);

	HELICS_Export helicsStatus helicsSendEventRaw(helics_message_federate_id_t fedID, helics_endpoint_id_t endpoint, const char *dest, const char *data, uint64_t len, helics_time_t time);

	HELICS_Export helicsStatus helicsSendMessage(helics_message_federate_id_t fedID, helics_endpoint_id_t endpoint, message_t *message);

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif /*HELICS_APISHARED_MESSAGE_FEDERATE_FUNCTIONS_H_*/
