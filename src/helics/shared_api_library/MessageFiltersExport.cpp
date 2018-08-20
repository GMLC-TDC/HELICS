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

static const char *invalidFilterString = "The given filter object is not valid";

/** this is a random identifier put in place when the federate or core or broker gets created*/
static const int filterValidationIdentifier = 0xEC26'0127;

static helics::FilterObject *getFilterObj(helics_filter filt, helics_error *err)
{
    HELICS_ERROR_CHECK (err, nullptr);
    if (filt == nullptr)
    {
        if (err != nullptr)
        {
            err->error_code = helics_invalid_object;
            err->message = invalidFilterString;
        }
        return nullptr;
    }
    auto fObj = reinterpret_cast<helics::FilterObject *> (filt);
    if (fObj->valid != filterValidationIdentifier)
    {
        if (err != nullptr)
        {
            err->error_code = helics_invalid_object;
            err->message = invalidFilterString;
        }
        return nullptr;
    }
    return fObj;
}

static inline void federateAddFilter (helics_federate fed, std::unique_ptr<helics::FilterObject >filt)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    filt->valid = filterValidationIdentifier;
    fedObj->filters.push_back (std::move(filt));
}

static inline void coreAddFilter (helics_core core, std::unique_ptr<helics::FilterObject >filt)
{
    auto coreObj = reinterpret_cast<helics::CoreObject *> (core);
    filt->valid = filterValidationIdentifier;
    coreObj->filters.push_back (std::move(filt));
}

helics_filter helicsFederateRegisterFilter (helics_federate fed, helics_filter_type_t type, const char *name, helics_error *err)
{
    // now generate a generic subscription
    auto fedObj = getFedSharedPtr (fed,err);
    if (!fedObj)
    {
        return nullptr;
    }

    try
    {
        auto filt = std::make_unique<helics::FilterObject> ();
        filt->filtptr = helics::make_filter (static_cast<helics::defined_filter_types> (type), fedObj.get (),
                                                    (name != nullptr) ? std::string (name) : nullstr);
        filt->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_filter> (filt.get());
        federateAddFilter (fed, std::move(filt));
        return ret;
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
    return nullptr;
}

helics_filter helicsCoreRegisterFilter (helics_core cr, helics_filter_type_t type, const char *name, helics_error *err)
{
    auto core = getCoreSharedPtr (cr,err);
    if (!core)
    {
        return nullptr;
    }
    try
    {
        auto filt = std::make_unique<helics::FilterObject>();
        filt->filtptr = helics::make_filter (static_cast<helics::defined_filter_types> (type), core.get (),
                                                    (name != nullptr) ? std::string (name) : nullstr);
        filt->corePtr = std::move (core);
        auto ret = reinterpret_cast<helics_filter> (filt.get());
        coreAddFilter (cr, std::move(filt));
        return ret;
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
    return nullptr;
}

helics_filter helicsFederateRegisterCloningFilter (helics_federate fed, const char *deliveryEndpoint, helics_error *err)
{
    auto fedObj = getFedSharedPtr (fed,err);
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
        filt->cloning = true;
        auto ret = reinterpret_cast<helics_filter> (filt.get());
        federateAddFilter (fed, std::move(filt));
        return ret;
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
    return nullptr;
}

helics_filter helicsCoreRegisterCloningFilter (helics_core cr, const char *deliveryEndpoint, helics_error *err)
{
    auto core = getCoreSharedPtr (cr,err);
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
        filt->cloning = true;
        auto ret = reinterpret_cast<helics_filter> (filt.get());
        coreAddFilter (cr, std::move(filt));
        return ret;
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
    return nullptr;
}

static helics::Filter *getFilter (helics_filter filt, helics_error *err)
{
    auto fObj = getFilterObj(filt,err);
    if (fObj == nullptr)
    {
        return nullptr;
    }
    return fObj->filtptr.get ();
}

static helics::CloningFilter *getCloningFilter (helics_filter filt, helics_error *err)
{
    static const char *nonCloningFilterString = "filter must be a cloning filter";
    auto fObj = getFilterObj(filt,err);
    if (fObj == nullptr)
    {
        return nullptr;
    }
    if (!fObj->cloning)
    {
        if (err != nullptr)
        {
            err->error_code = helics_invalid_object;
            err->message = nonCloningFilterString;
        }
        return nullptr;
    }
    return dynamic_cast<helics::CloningFilter *> (fObj->filtptr.get ());
}

/** get the target of the filter*/
int helicsFilterGetTarget (helics_filter filt, char *outputString, int maxlen, helics_error *err)
{
    auto filter = getFilter (filt,err);
    if (filter == nullptr)
    {
        return (-1);
    }
    if (!checkOutArgString (outputString, maxlen, err))
    {
        return (-1);
    }
    try
    {
        const auto &target = filter->getTarget ();
        if (static_cast<int> (target.size ()) > maxlen)
        {
            strncpy (outputString, target.c_str (), maxlen);
            outputString[maxlen - 1] = 0;
            return maxlen;
        }
        strcpy (outputString, target.c_str ());
        return static_cast<int>(target.size());
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return (-1);
    }
}

/** get the name of the filter*/
int helicsFilterGetName (helics_filter filt, char *outputString, int maxlen, helics_error *err)
{
    auto filter = getFilter (filt,err);
    if (filter == nullptr)
    {
        return (-1);
    }
    if (!checkOutArgString (outputString, maxlen, err))
    {
        return (-1);
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
        helicsErrorHandler (err);
    }
}

static const char *invalidPropertyString = "the specified property is invalid";

void helicsFilterSet (helics_filter filt, const char *prop, double val, helics_error *err)
{
    auto filter = getFilter (filt,err);
    if (filter == nullptr)
    {
        return;
    }
    if (prop == nullptr)
    {
		if (err != nullptr)
		{
            err->error_code = helics_invalid_argument;
            err->message = invalidPropertyString;
		}
        return;
    }
    try
    {
        filter->set (prop, val);
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsFilterSetString (helics_filter filt, const char *prop, const char *val, helics_error *err)
{
    auto filter = getFilter (filt,err);
    if (filter == nullptr)
    {
        return;
    }
    if (prop == nullptr)
    {
        if (err != nullptr)
        {
            err->error_code = helics_invalid_argument;
            err->message = invalidPropertyString;
        }
        return;
    }
    try
    {
        filter->setString (prop, val);
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsFilterAddDestinationTarget (helics_filter filt, const char *dest, helics_error *err)
{
    static constexpr char *invalidDestinationString = "the specified destination is not valid";
    auto cfilt = getFilter (filt,err);
    if (cfilt == nullptr)
    {
        return;
    }
    if (dest == nullptr)
    {
        if (err != nullptr)
        {
            err->error_code = helics_invalid_argument;
            err->message = invalidDestinationString;
        }
        return;
    }
    try
    {
        cfilt->addDestinationTarget (dest);
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsFilterAddSourceTarget (helics_filter filt, const char *src, helics_error *err)
{
    static constexpr char *invalidSourceString = "the specified Source is not valid";
    auto cfilt = getFilter (filt,err);
    if (cfilt == nullptr)
    {
        return;
    }
    if (src == nullptr)
    {
        if (err != nullptr)
        {
            err->error_code = helics_invalid_argument;
            err->message = invalidSourceString;
        }
        return;
    }
    try
    {
        cfilt->addSourceTarget (src);
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

static constexpr char *invalidDeliveryString = "the specified Delivery address is not valid";

void helicsFilterAddDeliveryEndpoint (helics_filter filt, const char *delivery, helics_error *err)
{
    auto cfilt = getCloningFilter (filt,err);
    if (cfilt == nullptr)
    {
        return;
    }
    if (delivery == nullptr)
    {
        if (err != nullptr)
        {
            err->error_code = helics_invalid_argument;
            err->message = invalidDeliveryString;
        }
        return;
    }
    try
    {
        cfilt->addDeliveryEndpoint (delivery);
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsFilterRemoveTarget (helics_filter filt, const char *target, helics_error *err)
{
    static constexpr char *invalidTargetString = "The specified Target address is not valid";

    auto cfilt = getFilter (filt,err);
    if (cfilt == nullptr)
    {
        return;
    }
    if (target == nullptr)
    {
        if (err != nullptr)
        {
            err->error_code = helics_invalid_argument;
            err->message = invalidTargetString;
        }
        return;
    }
    try
    {
        cfilt->removeTarget (target);
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsFilterRemoveDeliveryEndpoint (helics_filter filt, const char *delivery, helics_error *err)
{
    auto cfilt = getCloningFilter (filt,err);
    if (cfilt == nullptr)
    {
        return;
    }
    if (delivery == nullptr)
    {
        if (err != nullptr)
        {
            err->error_code = helics_invalid_argument;
            err->message = invalidDeliveryString;
        }
        return;
    }
    try
    {
        cfilt->removeDeliveryEndpoint (delivery);
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}
