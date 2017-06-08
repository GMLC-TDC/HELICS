/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#pragma once
#ifndef HELICS_APISHARED_FUNCTIONS_H_
#define HELICS_APISHARED_FUNCTIONS_H_



#ifdef __cplusplus
extern "C" {
#endif
#include "api-data.h"
#include <stdlib.h>


/*
  Export HELICS API functions on Windows and under GCC.
  If custom linking is desired then the HELICS_Export must be
  defined before including this file. For instance,
  it may be set to __declspec(dllimport).
*/
#if !defined(HELICS_Export)
    #if defined _WIN32 || defined __CYGWIN__
     /* Note: both gcc & MSVC on Windows support this syntax. */
        #define HELICS_Export __declspec(dllexport)
    #else
      #if __GNUC__ >= 4
        #define HELICS_Export __attribute__ ((visibility ("default")))
      #else
        #define HELICS_Export
      #endif
    #endif
#endif




/* Version number */
#define HelicsVersion "0.1"


/***************************************************
Common Functions
****************************************************/

/* Inquire version numbers of header files */

	HELICS_Export const char *helicsGetVersion();
	HELICS_Export helics_time_t helicsTimeFromDouble(double time);
	HELICS_Export double doubleFromHelicsTime(helics_time_t time);


/* Creation and destruction of Federates */
	HELICS_Export helics_federate_id_t helicsCreateValueFederate(federate_info_t *fi);
	HELICS_Export helics_federate_id_t helicsCreateValueFederateFromFile(const char *);
	HELICS_Export void helicsInitializeFederateInfo(federate_info_t *fi);

	HELICS_Export helicsStatus helicsFinalize(helics_federate_id_t fedID);
/* sub/pub registration */
	HELICS_Export helics_subscription_id_t helicsRegisterSubscription(helics_federate_id_t fedID, const char *name, const char *type, const char *units);
	HELICS_Export helics_subscription_id_t helicsRegisterDoubleSubscription(helics_federate_id_t fedID, const char *name, const char *units);
	HELICS_Export helics_subscription_id_t helicsRegisterStringSubscription(helics_federate_id_t fedID, const char *name, const char *units);

	HELICS_Export helics_publication_id_t  helicsRegisterPublication(helics_federate_id_t fedID, const char *name, const char *type, const char *units);
	HELICS_Export helics_publication_id_t  helicsRegisterDoublePublication(helics_federate_id_t fedID, const char *name, const char *units);
	HELICS_Export helics_publication_id_t  helicsRegisterStringPublication(helics_federate_id_t fedID, const char *name, const char *units);
	HELICS_Export helics_publication_id_t  helicsRegisterGlobalPublication(helics_federate_id_t fedID, const char *name, const char *type, const char *units);
	HELICS_Export helics_publication_id_t  helicsRegisterGlobalDoublePublication(helics_federate_id_t fedID, const char *name, const char *units);
	HELICS_Export helics_publication_id_t  helicsRegisterGlobalStringPublication(helics_federate_id_t fedID, const char *name, const char *units);
	/* initialization, execution, and time requests */
	HELICS_Export helicsStatus helicsEnterInitializationMode(helics_federate_id_t fedID);

	HELICS_Export helicsStatus helicsEnterExecutionMode(helics_federate_id_t fedID);

   HELICS_Export helics_time_t helicsRequestTime(helics_federate_id_t fedID, helics_time_t requestTime);
   
   /* getting and publishing values */
   HELICS_Export helicsStatus helicsPublish(helics_federate_id_t fedID, helics_publication_id_t pubID, const char *data, uint64_t len);

   HELICS_Export helicsStatus helicsPublishString(helics_federate_id_t fedID, helics_publication_id_t pubID, const char *str);

   HELICS_Export helicsStatus helicsPublishDouble(helics_federate_id_t fedID, helics_publication_id_t pubID, double val);

   HELICS_Export uint64_t helicsGetValue(helics_federate_id_t fedID, helics_publication_id_t pubID, char *data, uint64_t maxlen);

   HELICS_Export helicsStatus helicsGetString(helics_federate_id_t fedID, helics_publication_id_t pubID, char *str, uint64_t maxlen);

   HELICS_Export helicsStatus helicsGetDouble(helics_federate_id_t fedID, helics_publication_id_t pubID, double *val);

   
#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif