/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
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
#define HelicsVersion "0.3"


/***************************************************
Common Functions
****************************************************/

/* Inquire version numbers of header files */

	HELICS_Export const char *helicsGetVersion();
	HELICS_Export helics_time_t helicsTimeFromDouble(double time);
	HELICS_Export double doubleFromHelicsTime(helics_time_t time);

	HELICS_Export helics_core helicsCreateCore(const char *type, const char *name, const char *initString);
	HELICS_Export helics_core helicsCreateCoreFromArgs(const char *type, const char *name, int argc, char *argv[]);
/* Creation and destruction of Federates */

	HELICS_Export helics_federate helicsCreateCombinationFederate(federate_info_t *fi);
	HELICS_Export helics_federate helicsCreateCombinationFederateFromFile(const char *filename);

	HELICS_Export void helicsInitializeFederateInfo(federate_info_t *fi);

	HELICS_Export helicsStatus helicsFinalize(helics_federate fed);

	/* initialization, execution, and time requests */
	HELICS_Export helicsStatus helicsEnterInitializationMode(helics_federate fed);

	HELICS_Export helicsStatus helicsEnterExecutionMode(helics_federate fed);
	HELICS_Export helicsStatus helicsEnterExecutionModeIterative(helics_federate fed,convergence_status converged, convergence_status *outConverged);

   HELICS_Export helics_time_t helicsRequestTime(helics_federate fed, helics_time_t requestTime);
   HELICS_Export helics_iterative_time helicsRequestTimeIterative(helics_federate fed, helics_time_t requestTime, convergence_status converged);

   HELICS_Export void helics_free_federate(helics_federate fed)
   
  

   

#ifdef __cplusplus
}  /* end of extern "C" { */
#endif

#endif