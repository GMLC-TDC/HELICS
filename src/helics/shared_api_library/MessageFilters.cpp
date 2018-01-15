/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "../helics.hpp"
#include "../core/core-exceptions.h"
#include "helics.h"
#include "internal/api_objects.h"
#include <memory>
#include <mutex>
#include <vector>

static const std::string nullstr;

/** this is a random identifier put in place when the federate or core or broker gets created*/
static const int filterValidationIdentifier = 0xEC26'0127;

static inline void federateAddFilter (helics_federate fed, helics::FilterObject *filt)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    fedObj->filters.push_back (filt);
}

static inline void coreAddFilter (helics_core core, helics::FilterObject *filt)
{
    auto coreObj = reinterpret_cast<helics::CoreObject *> (core);
    coreObj->filters.push_back (filt);
}

helics_filter helicsFederateRegisterSourceFilter (helics_federate fed, helics_filter_type_t type, const char *target, const char *name)
{
    if (target == nullptr)
    {
        return nullptr;
    }
    // now generate a generic subscription
    auto fedObj = getFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }

    helics::FilterObject *filt = nullptr;
    try
    {
        filt = new helics::FilterObject ();
        filt->filtptr = helics::make_source_filter (static_cast<helics::defined_filter_types> (type), fedObj.get (), std::string (target),
                                                    (name != nullptr) ? std::string (name) : nullstr);
        filt->fedptr = std::move (fedObj);
        filt->type = helics::ftype::source;
        filt->valid = filterValidationIdentifier;
        federateAddFilter (fed, filt);
        return reinterpret_cast<helics_filter> (filt);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete filt;
    }
    return nullptr;
}

helics_filter helicsFederateRegisterDestinationFilter (helics_federate fed, helics_filter_type_t type, const char *target, const char *name)
{
    // now generate a generic subscription
    auto fedObj = getFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }
    helics::FilterObject *filt = nullptr;
    try
    {
        filt = new helics::FilterObject ();
        filt->filtptr = helics::make_destination_filter (static_cast<helics::defined_filter_types> (type), fedObj.get (),
                                                         std::string (target), (name != nullptr) ? std::string (name) : nullstr);
        filt->fedptr = std::move (fedObj);
        filt->type = helics::ftype::dest;
        filt->valid = filterValidationIdentifier;
        federateAddFilter (fed, filt);
        return reinterpret_cast<helics_filter> (filt);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete filt;
    }
    return nullptr;
}

helics_filter helicsCoreRegisterSourceFilter (helics_core cr, helics_filter_type_t type, const char *target, const char *name)
{
    if (target == nullptr)
    {
        return nullptr;
    }
    auto core = getCoreSharedPtr (cr);
    if (!core)
    {
        return nullptr;
    }
    helics::FilterObject *filt = nullptr;
    try
    {
        filt = new helics::FilterObject ();
        filt->filtptr = helics::make_source_filter (static_cast<helics::defined_filter_types> (type), core.get (), std::string (target),
                                                    (name != nullptr) ? std::string (name) : nullstr);
        filt->corePtr = std::move (core);
        filt->type = helics::ftype::source;
        filt->valid = filterValidationIdentifier;
        coreAddFilter (cr, filt);
        return reinterpret_cast<helics_filter> (filt);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete filt;
    }
    return nullptr;
}

helics_filter helicsCoreRegisterDestinationFilter (helics_core cr, helics_filter_type_t type, const char *target, const char *name)
{
    if (target == nullptr)
    {
        return nullptr;
    }
    auto core = getCoreSharedPtr (cr);
    if (!core)
    {
        return nullptr;
    }

    helics::FilterObject *filt = nullptr;
    try
    {
        filt = new helics::FilterObject ();
        filt->filtptr = helics::make_destination_filter (static_cast<helics::defined_filter_types> (type), core.get (),
                                                         std::string (target), (name != nullptr) ? std::string (name) : nullstr);
        filt->corePtr = std::move (core);
        filt->type = helics::ftype::dest;
        filt->valid = filterValidationIdentifier;
        coreAddFilter (cr, filt);
        return reinterpret_cast<helics_filter> (filt);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete filt;
    }
    return nullptr;
}

helics_filter helicsFederateRegisterCloningFilter (helics_federate fed, const char *deliveryEndpoint)
{
    auto fedObj = getFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }

    helics::FilterObject *filt = nullptr;
    try
    {
        filt = new helics::FilterObject ();
        auto filtptr = std::make_unique<helics::CloningFilter> (fedObj.get ());
        if (deliveryEndpoint != nullptr)
        {
            filtptr->addDeliveryEndpoint (deliveryEndpoint);
        }
        filt->filtptr = std::move (filtptr);
        filt->fedptr = std::move (fedObj);
        filt->type = helics::ftype::clone;
        filt->valid = filterValidationIdentifier;
        federateAddFilter (fed, filt);
        return reinterpret_cast<helics_filter> (filt);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete filt;
    }
    return nullptr;
}

helics_filter helicsCoreRegisterCloningFilter (helics_core cr, const char *deliveryEndpoint)
{
    auto core = getCoreSharedPtr (cr);
    if (!core)
    {
        return nullptr;
    }
    helics::FilterObject *filt = nullptr;
    try
    {
        filt = new helics::FilterObject ();
        auto filtptr = std::make_unique<helics::CloningFilter> (core.get ());
        if (deliveryEndpoint != nullptr)
        {
            filtptr->addDeliveryEndpoint (deliveryEndpoint);
        }
        filt->filtptr = std::move (filtptr);
        filt->corePtr = std::move (core);
        filt->type = helics::ftype::clone;
        filt->valid = filterValidationIdentifier;
        coreAddFilter (cr, filt);
        return reinterpret_cast<helics_filter> (filt);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete filt;
    }
    return nullptr;
}

static helics::Filter *getFilter (helics_filter filt)
{
    if (filt == nullptr)
    {
        return nullptr;
    }
    auto fObj = reinterpret_cast<helics::FilterObject *> (filt);
    if (fObj->valid != filterValidationIdentifier)
    {
        return nullptr;
    }
    return fObj->filtptr.get ();
}

/** get the target of the filter*/
helics_status helicsFilterGetTarget (helics_filter filt, char *str, int maxlen)
{
    auto filter = getFilter (filt);
    if (filter == nullptr)
    {
        return helics_invalid_object;
    }
    auto target = filter->getTarget ();
    if (static_cast<int> (target.size ()) > maxlen)
    {
        strncpy (str, target.c_str (), maxlen);
        str[maxlen - 1] = 0;
        return helics_warning;
    }
    strcpy (str, target.c_str ());
    return helics_ok;
}

/** get the name of the filter*/
helics_status helicsFilterGetName (helics_filter filt, char *str, int maxlen)
{
    auto filter = getFilter (filt);
    if (filter == nullptr)
    {
        return helics_invalid_object;
    }
    auto name = filter->getTarget ();
    if (static_cast<int> (name.size ()) > maxlen)
    {
        strncpy (str, name.c_str (), maxlen);
        str[maxlen - 1] = 0;
        return helics_warning;
    }
    strcpy (str, name.c_str ());
    return helics_ok;
}

helics_status helicsFilterSet (helics_filter filt, const char *property, double val)
{
    auto filter = getFilter (filt);
    if (filter == nullptr)
    {
        return helics_invalid_object;
    }
    if (property == nullptr)
    {
        return helics_invalid_argument;
    }
    filter->set (property, val);
    return helics_ok;
}

helics_status setString (helics_filter filt, const char *property, const char *val)
{
    auto filter = getFilter (filt);
    if (filter == nullptr)
    {
        return helics_invalid_object;
    }
    if (property == nullptr)
    {
        return helics_invalid_argument;
    }
    filter->setString (property, val);
    return helics_ok;
}