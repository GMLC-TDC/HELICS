/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_
#define HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_

#include "helics.h"

#ifdef __cplusplus
extern "C" {
#endif

// MESSAGE FEDERATE calls

HELICS_Export helics_source_filter helicsFederateRegisterSourceFilter (helics_federate fed,
                                                               const char *name,
                                                               const char *inputType,
                                                               const char *outputType);

HELICS_Export helics_destination_filter helicsFederateRegisterDestinationFilter (helics_federate fed,
                                                                         const char *name,
                                                                         const char *inputType,
                                                                         const char *outputType);

HELICS_Export void helicsFreeSourceFilter (helics_source_filter filter);

HELICS_Export void helicsFreeDestinationFilter (helics_destination_filter filter);
#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif /* HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_*/