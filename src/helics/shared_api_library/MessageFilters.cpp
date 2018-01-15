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

helics_source_filter helicsFederateRegisterSourceFilter (helics_federate fed, const char *name, const char *inputType, const char *outputType)
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
helicsFederateRegisterDestinationFilter (helics_federate fed, const char *name, const char *inputType, const char *outputType)
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


helics_source_filter helicsCoreRegisterSourceFilter(helics_core cr, const char *name, const char *inputType, const char *outputType)
{
    auto core = getCoreSharedPtr(cr);
    if (!core)
    {
        return nullptr;
    }
    helics::SourceFilterObject *filt = nullptr;
    try
    {
        filt = new helics::SourceFilterObject();
        filt->filtptr =
            std::make_unique<helics::SourceFilter>(core.get(), (name != nullptr) ? std::string(name) : nullstr, (inputType != nullptr) ? std::string(inputType) : nullstr,
            (outputType != nullptr) ? std::string(outputType) : nullstr);
        filt->corePtr = std::move(core);
        return reinterpret_cast<helics_source_filter> (filt);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete filt;
    }
    return nullptr;
}

helics_destination_filter
helicsCoreRegisterDestinationFilter(helics_core cr, const char *name, const char *inputType, const char *outputType)
{
    auto core = getCoreSharedPtr(cr);
    if (!core)
    {
        return nullptr;
    }
    helics::DestFilterObject *filt = nullptr;
    try
    {
        filt = new helics::DestFilterObject();
        filt->filtptr = std::make_unique<helics::DestinationFilter>(core.get(), (name != nullptr) ? std::string(name) : nullstr,
            (inputType != nullptr) ? std::string(inputType) : nullstr,
            (outputType != nullptr) ? std::string(outputType) : nullstr);
        filt->corePtr = std::move(core);
        return reinterpret_cast<helics_destination_filter> (filt);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete filt;
    }
    return nullptr;
}


helics_cloning_filter
helicsFederateRegisterCloningFilter(helics_federate fed, const char *deliveryEndpoint)
{
    auto fedObj = getFedSharedPtr(fed);
    if (!fedObj)
    {
        return nullptr;
    }

    helics::CloningFilterObject *filt = nullptr;
    try
    {
        filt = new helics::CloningFilterObject();
        filt->filtptr = std::make_unique<helics::CloningFilter>(fedObj.get());
        if (deliveryEndpoint != nullptr)
        {
            filt->filtptr->addDeliveryEndpoint(deliveryEndpoint);
        }

        filt->fedptr = std::move(fedObj);
        return reinterpret_cast<helics_cloning_filter> (filt);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete filt;
    }
    return nullptr;
}

helics_cloning_filter
helicsCoreRegisterCloningFilter(helics_core cr, const char *deliveryEndpoint)
{

    auto core = getCoreSharedPtr(cr);
    if (!core)
    {
        return nullptr;
    }
    helics::CloningFilterObject *filt = nullptr;
    try
    {
        filt = new helics::CloningFilterObject();
        filt->filtptr = std::make_unique<helics::CloningFilter>(core.get());
        if (deliveryEndpoint != nullptr)
        {
            filt->filtptr->addDeliveryEndpoint(deliveryEndpoint);
        }

        filt->corePtr = std::move(core);
        return reinterpret_cast<helics_cloning_filter> (filt);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete filt;
    }
    return nullptr;
}

void helicsSourceFilterFree (helics_source_filter filter) { delete reinterpret_cast<helics::SourceFilterObject *> (filter); }

void helicsDestinationFilterFree (helics_destination_filter filter) { delete reinterpret_cast<helics::DestFilterObject *> (filter); }

void helicsCloningFilterFree(helics_cloning_filter filter) { delete reinterpret_cast<helics::CloningFilterObject *> (filter); }