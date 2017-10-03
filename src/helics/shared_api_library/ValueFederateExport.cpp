/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "ValueFederate_c.h"
#include "application_api/Publications.hpp"
#include "application_api/Subscriptions.hpp"
#include "application_api/application_api.h"
#include "application_api/helicsTypes.hpp"
#include "core/helics-time.h"
#include "shared_api_library/internal/api_objects.h"
#include <map>
#include <memory>
#include <mutex>
#include <vector>

/* sub/pub registration */
helics_subscription
helicsRegisterSubscription (helics_value_federate fed, const char *name, const char *type, const char *units)
{
    auto htype = helics::getTypeFromString (type);
    if (htype != helics::helicsType_t::helicsInvalid)
    {
        return helicsRegisterTypeSubscription (fed, static_cast<int> (htype), name, units);
    }
    // now generate a generic subscription
    auto fedObj = getValueFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }
    helics::SubscriptionObject *sub = nullptr;
    try
    {
        sub = new helics::SubscriptionObject ();
        sub->id = fedObj->registerOptionalSubscription (name, type, units);
        sub->rawOnly = true;
        sub->fedptr = std::move (fedObj);
        return reinterpret_cast<helics_subscription> (sub);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        if (sub != nullptr)
        {
            delete sub;
        }
    }
    return nullptr;
}
helics_subscription
helicsRegisterTypeSubscription (helics_value_federate fed, int type, const char *name, const char *units)
{
    if ((type < 0) || (type > HELICS_VECTOR_TYPE))
    {
        if (type == HELICS_RAW_TYPE)
        {
            return helicsRegisterSubscription (fed, name, "", units);
        }
        return nullptr;
    }
    auto fedObj = getValueFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }

    helics::SubscriptionObject *sub = nullptr;
    try
    {
        sub = new helics::SubscriptionObject ();
        sub->subptr = std::make_unique<helics::Subscription> (fedObj.get (), name, units);
        sub->fedptr = std::move (fedObj);
        return reinterpret_cast<helics_subscription> (sub);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        if (sub != nullptr)
        {
            delete sub;
        }
    }
    return nullptr;
}

helics_publication
helicsRegisterPublication (helics_value_federate fed, const char *name, const char *type, const char *units)
{
    auto htype = helics::getTypeFromString (type);
    if (htype != helics::helicsType_t::helicsInvalid)
    {
        return helicsRegisterTypePublication (fed, static_cast<int> (htype), name, units);
    }
    // now generate a generic subscription
    auto fedObj = getValueFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }
    helics::PublicationObject *pub = nullptr;
    try
    {
        pub = new helics::PublicationObject ();
        pub->id = fedObj->registerPublication (name, type, units);
        pub->rawOnly = true;
        pub->fedptr = std::move (fedObj);
        return reinterpret_cast<helics_publication> (pub);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        if (pub != nullptr)
        {
            delete pub;
        }
    }
    return nullptr;
}
helics_publication
helicsRegisterTypePublication (helics_value_federate fed, int type, const char *name, const char *units)
{
    if ((type < 0) || (type > HELICS_VECTOR_TYPE))
    {
        if (type == HELICS_RAW_TYPE)
        {
            return helicsRegisterPublication (fed, name, "", units);
        }
        return nullptr;
    }
    auto fedObj = getValueFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }
    helics::PublicationObject *pub = nullptr;
    try
    {
        pub = new helics::PublicationObject ();
        pub->pubptr =
          std::make_unique<helics::Publication> (fedObj.get (), static_cast<helics::helicsType_t> (type), name,
                                                 units);
        pub->fedptr = std::move (fedObj);
        return reinterpret_cast<helics_publication> (pub);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        if (pub != nullptr)
        {
            delete pub;
        }
    }
    return nullptr;
}

helics_publication
helicsRegisterGlobalPublication (helics_value_federate fed, const char *name, const char *type, const char *units)
{
    auto htype = helics::getTypeFromString (type);
    if (htype != helics::helicsType_t::helicsInvalid)
    {
        return helicsRegisterGlobalTypePublication (fed, static_cast<int> (htype), name, units);
    }
    // now generate a generic subscription
    auto fedObj = getValueFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }
    helics::PublicationObject *pub = nullptr;
    try
    {
        pub = new helics::PublicationObject ();
        pub->id = fedObj->registerGlobalPublication (name, type, units);
        pub->rawOnly = true;
        pub->fedptr = std::move (fedObj);
        return reinterpret_cast<helics_publication> (pub);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        if (pub != nullptr)
        {
            delete pub;
        }
    }
    return nullptr;
}

helics_publication
helicsRegisterGlobalTypePublication (helics_value_federate fed, int type, const char *name, const char *units)
{
    if ((type < 0) || (type > HELICS_VECTOR_TYPE))
    {
        if (type == HELICS_RAW_TYPE)
        {
            return helicsRegisterGlobalPublication (fed, name, "", units);
        }
        return nullptr;
    }
    auto fedObj = getValueFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }
    helics::PublicationObject *pub = nullptr;
    try
    {
        pub = new helics::PublicationObject ();
        pub->pubptr =
          std::make_unique<helics::Publication> (helics::GLOBAL, fedObj.get (),
                                                 static_cast<helics::helicsType_t> (type), name, units);
        pub->fedptr = std::move (fedObj);
        return reinterpret_cast<helics_publication> (pub);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        if (pub != nullptr)
        {
            delete pub;
        }
    }
    return nullptr;
}

/* getting and publishing values */
helicsStatus helicsPublish (helics_publication pubID, const char *data, int len)
{
    if (pubID == nullptr)
    {
        return helicsError;
    }
    auto pubObj = reinterpret_cast<helics::PublicationObject *> (pubID);
    if (pubObj->rawOnly)
    {
        pubObj->fedptr->publish (pubObj->id, data, len);
    }
    return helicsOK;
}

helicsStatus helicsPublishString (helics_publication pubID, const char *str)
{
    if (pubID == nullptr)
    {
        return helicsError;
    }
    auto pubObj = reinterpret_cast<helics::PublicationObject *> (pubID);
    if (pubObj->rawOnly)
    {
        pubObj->fedptr->publish (pubObj->id, str);
    }
    else
    {
        pubObj->pubptr->publish (str);
    }
    return helicsOK;
}

helicsStatus helicsPublishInteger (helics_publication pubID, int64_t val)
{
    if (pubID == nullptr)
    {
        return helicsError;
    }
    auto pubObj = reinterpret_cast<helics::PublicationObject *> (pubID);
    if (pubObj->rawOnly)
    {
        pubObj->fedptr->publish (pubObj->id, val);
    }
    else
    {
        pubObj->pubptr->publish (val);
    }
    return helicsOK;
}
helicsStatus helicsPublishDouble (helics_publication pubID, double val)
{
    if (pubID == nullptr)
    {
        return helicsError;
    }
    auto pubObj = reinterpret_cast<helics::PublicationObject *> (pubID);
    if (pubObj->rawOnly)
    {
        pubObj->fedptr->publish (pubObj->id, val);
    }
    else
    {
        pubObj->pubptr->publish (val);
    }
    return helicsOK;
}

helicsStatus helicsPublishComplex (helics_publication pubID, double real, double imag)
{
    if (pubID == nullptr)
    {
        return helicsError;
    }
    auto pubObj = reinterpret_cast<helics::PublicationObject *> (pubID);
    if (pubObj->rawOnly)
    {
        pubObj->fedptr->publish (pubObj->id, std::complex<double> (real, imag));
    }
    else
    {
        pubObj->pubptr->publish (std::complex<double> (real, imag));
    }
    return helicsOK;
}

helicsStatus helicsPublishVector (helics_publication pubID, const double data[], int len)
{
    if (pubID == nullptr)
    {
        return helicsError;
    }
    auto pubObj = reinterpret_cast<helics::PublicationObject *> (pubID);
    if (pubObj->rawOnly)
    {
        pubObj->fedptr->publish (pubObj->id, std::vector<double> (data, data + len));
    }
    else
    {
        pubObj->pubptr->publish (std::vector<double> (data, data + len));
    }
    return helicsOK;
}

int helicsGetValue (helics_subscription subID, char *data, int maxlen)
{
    if (subID == nullptr)
    {
        return -1;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (subID);
    if (subObj->rawOnly)
    {
        auto dv = subObj->fedptr->getValueRaw (subObj->id);
        if (maxlen > static_cast<int> (dv.size ()))
        {
            memcpy (data, dv.data (), dv.size ());
            return static_cast<int> (dv.size ());
        }
        else
        {
            memcpy (data, dv.data (), maxlen);
            return maxlen;
        }
    }
    else
    {
        auto str = subObj->subptr->getValue<std::string> ();
        if (maxlen > static_cast<int> (str.size ()))
        {
            strcpy (data, str.c_str ());
            return static_cast<int> (str.size ());
        }
        else
        {
            memcpy (data, str.data (), maxlen);
            return maxlen;
        }
    }
}

helicsStatus helicsGetString (helics_subscription subID, char *str, int maxlen)
{
    auto len = helicsGetValue (subID, str, maxlen);
    // make sure we have a null terminator
    if (len == maxlen)
    {
        str[maxlen - 1] = 0;
        return helicsWarning;
    }
    else
    {
        str[len] = 0;
    }
    return helicsOK;
}
helicsStatus helicsGetInteger (helics_subscription subID, int64_t *val)
{
    if (subID == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (subID);
    if (subObj->rawOnly)
    {
        *val = subObj->fedptr->getValue<int64_t> (subObj->id);
    }
    else
    {
        subObj->subptr->getValue (*val);
    }
    return helicsOK;
}
helicsStatus helicsGetDouble (helics_subscription subID, double *val)
{
    if (subID == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (subID);
    if (subObj->rawOnly)
    {
        *val = subObj->fedptr->getValue<double> (subObj->id);
    }
    else
    {
        *val = subObj->subptr->getValue<double> ();
    }
    return helicsOK;
}
helicsStatus helicsGetComplex (helics_subscription subID, double *real, double *imag)
{
    if (subID == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (subID);
    if (subObj->rawOnly)
    {
        auto cval = subObj->fedptr->getValue<std::complex<double>> (subObj->id);
        *real = cval.real ();
        *imag = cval.imag ();
    }
    else
    {
        auto cval = subObj->subptr->getValue<std::complex<double>> ();
        *real = cval.real ();
        *imag = cval.imag ();
    }
    return helicsOK;
}

int helicsGetVector (helics_subscription subID, double data[], int len)
{
    if (subID == nullptr)
    {
        return 0;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (subID);
    if (subObj->rawOnly)
    {
        auto V = subObj->fedptr->getValue<std::vector<double>> (subObj->id);
        std::copy (V.data (), V.data () + std::min (static_cast<int> (V.size ()), len), data);
        return std::min (static_cast<int> (V.size ()), len);
    }
    else
    {
        auto V = subObj->subptr->getValue<std::vector<double>> ();
        std::copy (V.data (), V.data () + std::min (static_cast<int> (V.size ()), len), data);
        return std::min (static_cast<int> (V.size ()), len);
    }
}

helicsStatus helicsSetDefaultValue (helics_subscription subID, const char *data, int len)
{
    if (subID == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (subID);

    subObj->fedptr->setDefaultValue (subObj->id, helics::data_view (data, len));
    return helicsOK;
}

helicsStatus helicsSetDefaultString (helics_subscription subID, char *str)
{
    if (subID == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (subID);
    if (subObj->rawOnly)
    {
        subObj->fedptr->setDefaultValue (subObj->id, helics::data_view (str));
    }
    else
    {
        subObj->subptr->setDefault<std::string> (str);
    }
    return helicsOK;
}

helicsStatus helicsSetDefaultInteger (helics_subscription subID, int64_t val)
{
    if (subID == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (subID);
    if (subObj->rawOnly)
    {
        subObj->fedptr->setDefaultValue (subObj->id, val);
    }
    else
    {
        subObj->subptr->setDefault (val);
    }
    return helicsOK;
}
helicsStatus helicsSetDefaultDouble (helics_subscription subID, double val)
{
    if (subID == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (subID);
    if (subObj->rawOnly)
    {
        subObj->fedptr->setDefaultValue (subObj->id, val);
    }
    else
    {
        subObj->subptr->setDefault (val);
    }
    return helicsOK;
}
helicsStatus helicsSetDefaultComplex (helics_subscription subID, double real, double imag)
{
    if (subID == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (subID);
    if (subObj->rawOnly)
    {
        subObj->fedptr->setDefaultValue (subObj->id, std::complex<double> (real, imag));
    }
    else
    {
        subObj->subptr->setDefault (std::complex<double> (real, imag));
    }
    return helicsOK;
}

helicsStatus helicsSetDefaultVector (helics_subscription subID, const double *data, int len)
{
    if (subID == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (subID);
    if (subObj->rawOnly)
    {
        subObj->fedptr->setDefaultValue (subObj->id, std::vector<double> (data, data + len));
    }
    else
    {
        subObj->subptr->setDefault (std::vector<double> (data, data + len));
    }
    return helicsOK;
}
