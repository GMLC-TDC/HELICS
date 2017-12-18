/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "../helics.hpp"
#include "helics.h"
#include "internal/api_objects.h"
#include <memory>
#include <mutex>
#include <vector>

static const std::string nullstr;

helics_source_filter helicsRegisterSourceFilter (helics_federate fed, const char *name, const char *inputType, const char *outputType)
{
    // now generate a generic subscription
    auto fedObj = getFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }
    helics::SourceFilterObject *filt = nullptr;
    try
    {
        filt = new helics::SourceFilterObject ();
        filt->filtptr =
          std::make_unique<helics::SourceFilter> (fedObj.get (), (name!=nullptr)?std::string(name): nullstr, (inputType != nullptr) ? std::string (inputType) : nullstr,
                                                  (outputType != nullptr) ? std::string (outputType) : nullstr);
        filt->fedptr = std::move (fedObj);
        return reinterpret_cast<helics_source_filter> (filt);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete filt;
    }
    return nullptr;
}

helics_destination_filter
helicsRegisterDestinationFilter (helics_federate fed, const char *name, const char *inputType, const char *outputType)
{
    // now generate a generic subscription
    auto fedObj = getFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }
    helics::DestFilterObject *filt = nullptr;
    try
    {
        filt = new helics::DestFilterObject ();
        filt->filtptr = std::make_unique<helics::DestinationFilter> (fedObj.get (), (name != nullptr) ? std::string(name) : nullstr,
                                                                     (inputType != nullptr) ? std::string (inputType) : nullstr,
                                                                     (outputType != nullptr) ? std::string (outputType) : nullstr);
        filt->fedptr = std::move (fedObj);
        return reinterpret_cast<helics_destination_filter> (filt);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete filt;
    }
    return nullptr;
}

void helicsFreeSourceFilter (helics_source_filter filter) { delete reinterpret_cast<helics::SourceFilterObject *> (filter); }

void helicsFreeDestinationFilter (helics_destination_filter filter) { delete reinterpret_cast<helics::DestFilterObject *> (filter); }
