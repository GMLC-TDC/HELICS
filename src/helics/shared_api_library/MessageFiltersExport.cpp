/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "../core/core-exceptions.hpp"
#include "../helics.hpp"
#include "MessageFilters.h"
#include "internal/api_objects.h"
#include <memory>
#include <mutex>

static const std::string nullstr;

/** this is a random identifier put in place when the federate or core or broker gets created*/
static const int filterValidationIdentifier = 0xEC26'0127;

static inline void federateAddFilter (helics_federate fed, std::unique_ptr<helics::FilterObject >filt)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    fedObj->filters.push_back (std::move(filt));
}

static inline void coreAddFilter (helics_core core, std::unique_ptr<helics::FilterObject >filt)
{
    auto coreObj = reinterpret_cast<helics::CoreObject *> (core);
    coreObj->filters.push_back (std::move(filt));
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

    try
    {
        auto filt = std::make_unique<helics::FilterObject> ();
        filt->filtptr = helics::make_source_filter (static_cast<helics::defined_filter_types> (type), fedObj.get (), std::string (target),
                                                    (name != nullptr) ? std::string (name) : nullstr);
        filt->fedptr = std::move (fedObj);
        filt->type = helics::ftype::source;
        filt->valid = filterValidationIdentifier;
        auto ret = reinterpret_cast<helics_filter> (filt.get());
        federateAddFilter (fed, std::move(filt));
        return ret;
    }
    catch (const helics::InvalidFunctionCall &)
    {
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
    try
    {
        auto filt = std::make_unique<helics::FilterObject>();
        filt->filtptr = helics::make_destination_filter (static_cast<helics::defined_filter_types> (type), fedObj.get (),
                                                         std::string (target), (name != nullptr) ? std::string (name) : nullstr);
        filt->fedptr = std::move (fedObj);
        filt->type = helics::ftype::dest;
        filt->valid = filterValidationIdentifier;
        auto ret = reinterpret_cast<helics_filter> (filt.get());
        federateAddFilter (fed, std::move(filt));
        return ret;
    }
    catch (const helics::InvalidFunctionCall &)
    {
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
    try
    {
        auto filt = std::make_unique<helics::FilterObject>();
        filt->filtptr = helics::make_source_filter (static_cast<helics::defined_filter_types> (type), core.get (), std::string (target),
                                                    (name != nullptr) ? std::string (name) : nullstr);
        filt->corePtr = std::move (core);
        filt->type = helics::ftype::source;
        filt->valid = filterValidationIdentifier;
        auto ret = reinterpret_cast<helics_filter> (filt.get());
        coreAddFilter (cr, std::move(filt));
        return ret;
    }
    catch (const helics::InvalidFunctionCall &)
    {
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

    try
    {
        auto filt = std::make_unique<helics::FilterObject>();
        filt->filtptr = helics::make_destination_filter (static_cast<helics::defined_filter_types> (type), core.get (),
                                                         std::string (target), (name != nullptr) ? std::string (name) : nullstr);
        filt->corePtr = std::move (core);
        filt->type = helics::ftype::dest;
        filt->valid = filterValidationIdentifier;
        auto ret = reinterpret_cast<helics_filter> (filt.get());
        coreAddFilter (cr, std::move(filt));
        return ret;
    }
    catch (const helics::InvalidFunctionCall &)
    {
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

    try
    {
        auto filt = std::make_unique<helics::FilterObject>();
        auto filtptr = std::make_unique<helics::CloningFilter> (fedObj.get ());
        if (deliveryEndpoint != nullptr)
        {
            filtptr->addDeliveryEndpoint (deliveryEndpoint);
        }
        filt->filtptr = std::move (filtptr);
        filt->fedptr = std::move (fedObj);
        filt->type = helics::ftype::clone;
        filt->valid = filterValidationIdentifier;
        auto ret = reinterpret_cast<helics_filter> (filt.get());
        federateAddFilter (fed, std::move(filt));
        return ret;
    }
    catch (const helics::InvalidFunctionCall &)
    {
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
    try
    {
        auto filt = std::make_unique<helics::FilterObject>();
        auto filtptr = std::make_unique<helics::CloningFilter> (core.get ());
        if (deliveryEndpoint != nullptr)
        {
            filtptr->addDeliveryEndpoint (deliveryEndpoint);
        }
        filt->filtptr = std::move (filtptr);
        filt->corePtr = std::move (core);
        filt->type = helics::ftype::clone;
        filt->valid = filterValidationIdentifier;
        auto ret = reinterpret_cast<helics_filter> (filt.get());
        coreAddFilter (cr, std::move(filt));
        return ret;
    }
    catch (const helics::InvalidFunctionCall &)
    {
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

static helics::CloningFilter *getCloningFilter (helics_filter filt)
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
    if (fObj->type != helics::ftype::clone)
    {
        return nullptr;
    }
    return dynamic_cast<helics::CloningFilter *> (fObj->filtptr.get ());
}

/** get the target of the filter*/
helics_status helicsFilterGetTarget (helics_filter filt, char *outputString, int maxlen)
{
    auto filter = getFilter (filt);
    if (filter == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        const auto &target = filter->getTarget ();
        if (static_cast<int> (target.size ()) > maxlen)
        {
            strncpy (outputString, target.c_str (), maxlen);
            outputString[maxlen - 1] = 0;
            return helics_warning;
        }
        strcpy (outputString, target.c_str ());
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

/** get the name of the filter*/
helics_status helicsFilterGetName (helics_filter filt, char *outputString, int maxlen)
{
    auto filter = getFilter (filt);
    if (filter == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        const auto &name = filter->getName ();
        if (static_cast<int> (name.size ()) > maxlen)
        {
            strncpy (outputString, name.c_str (), maxlen);
            outputString[maxlen - 1] = 0;
            return helics_warning;
        }
        strcpy (outputString, name.c_str ());
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
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
    try
    {
        filter->set (property, val);
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFilterSetString (helics_filter filt, const char *property, const char *val)
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
    try
    {
        filter->setString (property, val);
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFilterAddDestinationTarget (helics_filter filt, const char *dest)
{
    auto cfilt = getCloningFilter (filt);
    if (cfilt == nullptr)
    {
        return helics_invalid_object;
    }
    if (dest == nullptr)
    {
        return helics_invalid_argument;
    }
    try
    {
        cfilt->addDestinationTarget (dest);
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFilterAddSourceTarget (helics_filter filt, const char *src)
{
    auto cfilt = getCloningFilter (filt);
    if (cfilt == nullptr)
    {
        return helics_invalid_object;
    }
    if (src == nullptr)
    {
        return helics_invalid_argument;
    }
    try
    {
        cfilt->addSourceTarget (src);
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFilterAddDeliveryEndpoint (helics_filter filt, const char *delivery)
{
    auto cfilt = getCloningFilter (filt);
    if (cfilt == nullptr)
    {
        return helics_invalid_object;
    }
    if (delivery == nullptr)
    {
        return helics_invalid_argument;
    }
    try
    {
        cfilt->addDeliveryEndpoint (delivery);
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFilterRemoveDestinationTarget (helics_filter filt, const char *dest)
{
    auto cfilt = getCloningFilter (filt);
    if (cfilt == nullptr)
    {
        return helics_invalid_object;
    }
    if (dest == nullptr)
    {
        return helics_invalid_argument;
    }
    try
    {
        cfilt->removeDestinationTarget (dest);
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFilterRemoveSourceTarget (helics_filter filt, const char *source)
{
    auto cfilt = getCloningFilter (filt);
    if (cfilt == nullptr)
    {
        return helics_invalid_object;
    }
    if (source == nullptr)
    {
        return helics_invalid_argument;
    }
    try
    {
        cfilt->removeSourceTarget (source);
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFilterRemoveDeliveryEndpoint (helics_filter filt, const char *delivery)
{
    auto cfilt = getCloningFilter (filt);
    if (cfilt == nullptr)
    {
        return helics_invalid_object;
    }
    if (delivery == nullptr)
    {
        return helics_invalid_argument;
    }
    try
    {
        cfilt->removeDeliveryEndpoint (delivery);
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}
