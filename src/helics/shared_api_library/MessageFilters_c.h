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

HELICS_Export helics_cloning_filter helicsFederateRegisterCloningFilter(helics_federate fed,
const char *deliveryEndpoint);

HELICS_Export helics_source_filter helicsCoreRegisterSourceFilter(helics_core fed,
    const char *name,
    const char *inputType,
    const char *outputType);

HELICS_Export helics_destination_filter helicsCoreRegisterDestinationFilter(helics_core fed,
    const char *name,
    const char *inputType,
    const char *outputType);

HELICS_Export helics_cloning_filter helicsCoreRegisterCloningFilter(helics_core fed,
    const char *deliveryEndpoint);

HELICS_Export void helicsSourceFilterFree (helics_source_filter filter);

HELICS_Export void helicsDestinationFilterFree (helics_destination_filter filter);

HELICS_Export void helicsCloningFilterFree(helics_cloning_filter filter);
#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif /* HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_*/