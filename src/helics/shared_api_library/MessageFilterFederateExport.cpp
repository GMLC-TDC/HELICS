/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "application_api/Endpoints.hpp"
#include "application_api/application_api.h"
#include "core/helics-time.h"
#include "helics.h"
#include "shared_api_library/internal/api_objects.h"
#include <memory>
#include <mutex>
#include <vector>

helics_source_filter helicsRegisterSourceFilter (helics_message_filter_federate fedID,
                                                 const char *name,
                                                 const char *inputType,
                                                 const char *outputType)
{
    return nullptr;
}

helics_destination_filter helicsRegisterDestinationFilter (helics_message_filter_federate fedID,
                                                           const char *name,
                                                           const char *inputType,
                                                           const char *outputType)
{
    return nullptr;
}

int helicsFederateHasMessageToFilter (helics_message_filter_federate fedID) { return false; }
int helicsFilterHasMessage (helics_message_filter_federate id) { return false; }

int helicsFederateFilterReceiveCount (helics_message_filter_federate fedID) { return 0; }

int helicsFilterReceiveCount (helics_source_filter filter) { return 0; }

message_t helicsFilterGetMessage (helics_source_filter filter) { return message_t (); }

message_t helicsFilterFederateGetMessage (helics_message_filter_federate fedID) { return message_t (); }