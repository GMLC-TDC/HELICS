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


	HELICS_Export helics_federate_id_t helicsCreateMessageFilterFederate(federate_info_t *fi);
	HELICS_Export helics_federate_id_t helicsCreateMessageFilterFederateFromFile(const char *);
	//MESSAGE FEDERATE calls


#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif /* HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_*/