/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

/** @file
@brief functions related the message filters for the C api
*/

#ifndef HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_
#define HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_

#include "helics.h"

#ifdef __cplusplus
extern "C" {
#endif


/** create a source Filter on the specified federate
@details filters can be created through a federate or a core , linking through a federate allows
a few extra features of name matching to function on the federate interface but otherwise equivalent behavior
@param fed the fed to register through
@param name the name of the filter (can be nullptr)
@param inputType the input type of the filter,  used for ordering (can be nullptr)
@param outputType the output type of the filter,  used for ordering (can be nullptr)
@return a helics_source_filter object
*/
HELICS_Export helics_filter helicsFederateRegisterSourceFilter (helics_federate fed,
                                                                helics_filter_type_t type,
                                                                const char *target,
                                                                const char *name);

HELICS_Export helics_filter helicsFederateRegisterDestinationFilter (helics_federate fed,
                                                                     helics_filter_type_t type,
                                                                     const char *target,
                                                                     const char *name);

HELICS_Export helics_filter helicsFederateRegisterCloningFilter (helics_federate fed, const char *deliveryEndpoint);

HELICS_Export helics_filter helicsCoreRegisterSourceFilter (helics_core core,
                                                            helics_filter_type_t type,
                                                            const char *target,
                                                            const char *name);

HELICS_Export helics_filter helicsCoreRegisterDestinationFilter (helics_core core,
                                                                 helics_filter_type_t type,
                                                                 const char *target,
                                                                 const char *name);

HELICS_Export helics_filter helicsCoreRegisterCloningFilter (helics_core fed, const char *deliveryEndpoint);

/** get the target of the filter*/
HELICS_Export helics_status helicsFilterGetTarget (helics_filter filt, char *str, int maxlen);

/** get the name of the filter*/
HELICS_Export helics_status helicsFilterGetName (helics_filter filt, char *str, int maxlen);

HELICS_Export helics_status helicsFilterSet (helics_filter filt, const char *property, double val);

HELICS_Export helics_status setString (helics_filter filt, const char *property, const char *val);

/* the following operations only work with cloning filters*/

HELICS_Export helics_status helicsFilterAddDestinationTarget(helics_filter filt, const char *dest);
HELICS_Export helics_status helicsFilterAddSourceTarget(helics_filter filt, const char *dest);
HELICS_Export helics_status helicsFilterAddDeliveryEndpoint(helics_filter filt, const char *dest);

HELICS_Export helics_status helicsFilterRemoveDestinationTarget(helics_filter filt, const char *dest);
HELICS_Export helics_status helicsFilterRemoveSourceTarget(helics_filter filt, const char *dest);
HELICS_Export helics_status helicsFilterRemoveDeliveryEndpoint(helics_filter filt, const char *dest);
#ifdef __cplusplus
} /* end of extern "C" { */
#endif

#endif /* HELICS_APISHARED_MESSAGE_FILTER_FEDERATE_FUNCTIONS_H_*/

