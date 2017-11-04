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

int helicsValueFederateisUpdated (helics_value_federate vfed, helics_subscription sub)
{
    helics::SubscriptionObject *subobj = reinterpret_cast<helics::SubscriptionObject *> (sub);
    helics::ValueFederate *valuefed = reinterpret_cast<helics::ValueFederate *> (vfed);
    return (int)(valuefed->isUpdated (subobj->id));
}

static inline void addSubscription(helics_value_federate fed, helics::SubscriptionObject *sub)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    fedObj->subs.push_back(sub);
}

static inline void addPublication(helics_value_federate fed, helics::PublicationObject *pub)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    fedObj->pubs.push_back(pub);
}
/* sub/pub registration */
helics_subscription
helicsRegisterSubscription (helics_value_federate fed, const char *key, const char *type, const char *units)
{
    auto htype = helics::getTypeFromString (type);
    if (htype != helics::helicsType_t::helicsInvalid)
    {
        return helicsRegisterTypeSubscription (fed, key, static_cast<int> (htype), units);
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
        sub->id = fedObj->registerOptionalSubscription (key, type, units);
        sub->rawOnly = true;
        sub->fedptr = std::move (fedObj);
        addSubscription(fed, sub);
        return reinterpret_cast<helics_subscription> (sub);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete sub;
    }
    return nullptr;
}
helics_subscription
helicsRegisterTypeSubscription (helics_value_federate fed, const char *key, int type, const char *units)
{
    if ((type < 0) || (type > HELICS_VECTOR_TYPE))
    {
        if (type == HELICS_RAW_TYPE)
        {
            return helicsRegisterSubscription (fed, key, "", units);
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
        sub->subptr = std::make_unique<helics::Subscription> (fedObj.get (), key, units);
        sub->fedptr = std::move (fedObj);
        addSubscription(fed, sub);
        return reinterpret_cast<helics_subscription> (sub);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete sub;
    }
    return nullptr;
}

helics_publication
helicsRegisterPublication (helics_value_federate fed, const char *key, const char *type, const char *units)
{
    auto htype = helics::getTypeFromString (type);
    if (htype != helics::helicsType_t::helicsInvalid)
    {
        return helicsRegisterTypePublication (fed, key, static_cast<int> (htype), units);
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
        pub->id = fedObj->registerPublication (key, type, units);
        pub->rawOnly = true;
        pub->fedptr = std::move (fedObj);
        addPublication(fed, pub);
        return reinterpret_cast<helics_publication> (pub);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete pub;
    }
    return nullptr;
}
helics_publication
helicsRegisterTypePublication (helics_value_federate fed, const char *key, int type, const char *units)
{
    if ((type < 0) || (type > HELICS_VECTOR_TYPE))
    {
        if (type == HELICS_RAW_TYPE)
        {
            return helicsRegisterPublication (fed, key, "", units);
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
        pub->pubptr = std::make_unique<helics::Publication> (fedObj.get (), key,
                                                             static_cast<helics::helicsType_t> (type), units);
        pub->fedptr = std::move (fedObj);
        addPublication(fed, pub);
        return reinterpret_cast<helics_publication> (pub);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete pub;
    }
    return nullptr;
}

helics_publication
helicsRegisterGlobalPublication (helics_value_federate fed, const char *key, const char *type, const char *units)
{
    auto htype = helics::getTypeFromString (type);
    if (htype != helics::helicsType_t::helicsInvalid)
    {
        return helicsRegisterGlobalTypePublication (fed, key, static_cast<int> (htype), units);
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
        pub->id = fedObj->registerGlobalPublication (key, type, units);
        pub->rawOnly = true;
        pub->fedptr = std::move (fedObj);
        addPublication(fed, pub);
        return reinterpret_cast<helics_publication> (pub);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete pub;
    }
    return nullptr;
}

helics_publication
helicsRegisterGlobalTypePublication (helics_value_federate fed, const char *key, int type, const char *units)
{
    if ((type < 0) || (type > HELICS_VECTOR_TYPE))
    {
        if (type == HELICS_RAW_TYPE)
        {
            return helicsRegisterGlobalPublication (fed, key, "", units);
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
        pub->pubptr = std::make_unique<helics::Publication> (helics::GLOBAL, fedObj.get (), key,
                                                             static_cast<helics::helicsType_t> (type), units);
        pub->fedptr = std::move (fedObj);
        addPublication(fed, pub);
        return reinterpret_cast<helics_publication> (pub);
    }
    catch (const helics::InvalidFunctionCall &)
    {
        delete pub;
    }
    return nullptr;
}

/* getting and publishing values */
helicsStatus helicsPublish (helics_publication pub, const char *data, int len)
{
    if (pub == nullptr)
    {
        return helicsError;
    }
    auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
    if (pubObj->rawOnly)
    {
        pubObj->fedptr->publish (pubObj->id, data, len);
    }
    return helicsOK;
}

helicsStatus helicsPublishString (helics_publication pub, const char *str)
{
    if (pub == nullptr)
    {
        return helicsError;
    }
    auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
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

helicsStatus helicsPublishInteger (helics_publication pub, int64_t val)
{
    if (pub == nullptr)
    {
        return helicsError;
    }
    auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
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
helicsStatus helicsPublishDouble (helics_publication pub, double val)
{
    if (pub == nullptr)
    {
        return helicsError;
    }
    auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
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

helicsStatus helicsPublishComplex (helics_publication pub, double real, double imag)
{
    if (pub == nullptr)
    {
        return helicsError;
    }
    auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
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

helicsStatus helicsPublishVector (helics_publication pub, const double data[], int len)
{
    if (pub == nullptr)
    {
        return helicsError;
    }
    auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
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

int helicsGetValue (helics_subscription sub, char *data, int maxlen)
{
    if (sub == nullptr)
    {
        return -1;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
    if (subObj->rawOnly)
    {
        auto dv = subObj->fedptr->getValueRaw (subObj->id);
        if (maxlen > static_cast<int> (dv.size ()))
        {
            memcpy (data, dv.data (), dv.size ());
            return static_cast<int> (dv.size ());
        }
        memcpy (data, dv.data (), maxlen);
        return maxlen;
    }
    else
    {
        auto str = subObj->subptr->getValue<std::string> ();
        if (maxlen > static_cast<int> (str.size ()))
        {
            strcpy (data, str.c_str ());
            return static_cast<int> (str.size ());
        }
        memcpy (data, str.data (), maxlen);
        return maxlen;
    }
}

helicsStatus helicsGetString (helics_subscription sub, char *str, int maxlen)
{
    auto len = helicsGetValue (sub, str, maxlen);
    // make sure we have a null terminator
    if (len == maxlen)
    {
        str[maxlen - 1] = 0;
        return helicsWarning;
    }
    str[len] = 0;
    return helicsOK;
}
helicsStatus helicsGetInteger (helics_subscription sub, int64_t *val)
{
    if (sub == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
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
helicsStatus helicsGetDouble (helics_subscription sub, double *val)
{
    if (sub == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
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
helicsStatus helicsGetComplex (helics_subscription sub, double *real, double *imag)
{
    if (sub == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
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

int helicsGetVector (helics_subscription sub, double data[], int len)
{
    if (sub == nullptr)
    {
        return 0;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
    if (subObj->rawOnly)
    {
        auto V = subObj->fedptr->getValue<std::vector<double>> (subObj->id);
        std::copy (V.data (), V.data () + std::min (static_cast<int> (V.size ()), len), data);
        return std::min (static_cast<int> (V.size ()), len);
    }

    auto V = subObj->subptr->getValue<std::vector<double>> ();
    std::copy (V.data (), V.data () + std::min (static_cast<int> (V.size ()), len), data);
    return std::min (static_cast<int> (V.size ()), len);
}

helicsStatus helicsSetDefaultValue (helics_subscription sub, const char *data, int len)
{
    if (sub == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);

    subObj->fedptr->setDefaultValue (subObj->id, helics::data_view (data, len));
    return helicsOK;
}

helicsStatus helicsSetDefaultString (helics_subscription sub, const char *str)
{
    if (sub == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
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

helicsStatus helicsSetDefaultInteger (helics_subscription sub, int64_t val)
{
    if (sub == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
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
helicsStatus helicsSetDefaultDouble (helics_subscription sub, double val)
{
    if (sub == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
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
helicsStatus helicsSetDefaultComplex (helics_subscription sub, double real, double imag)
{
    if (sub == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
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

helicsStatus helicsSetDefaultVector (helics_subscription sub, const double *data, int len)
{
    if (sub == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
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

helicsStatus helicsGetSubscriptionType (helics_subscription sub, char *str, int maxlen)
{
    if (sub == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
    auto type = subObj->subptr->getType ();
    if (static_cast<int> (type.size ()) > maxlen)
    {
        strncpy (str, type.c_str (), maxlen);
        str[maxlen - 1] = 0;
    }
    else
    {
        strcpy (str, type.c_str ());
    }
    return helicsOK;
}

helicsStatus helicsGetPublicationType (helics_publication pub, char *str, int maxlen)
{
    if (pub == nullptr)
    {
        return helicsError;
    }
    auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
    auto type = pubObj->pubptr->getType ();
    if (static_cast<int> (type.size ()) > maxlen)
    {
        strncpy (str, type.c_str (), maxlen);
        str[maxlen - 1] = 0;
    }
    else
    {
        strcpy (str, type.c_str ());
    }
    return helicsOK;
}

helicsStatus helicsGetSubscriptionKey (helics_subscription sub, char *str, int maxlen)
{
    if (sub == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
    auto type = subObj->subptr->getKey ();
    if (static_cast<int> (type.size ()) > maxlen)
    {
        strncpy (str, type.c_str (), maxlen);
        str[maxlen - 1] = 0;
    }
    else
    {
        strcpy (str, type.c_str ());
    }
    return helicsOK;
}

helicsStatus helicsGetPublicationKey (helics_publication pub, char *str, int maxlen)
{
    if (pub == nullptr)
    {
        return helicsError;
    }
    auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
    auto type = pubObj->pubptr->getKey ();
    if (static_cast<int> (type.size ()) > maxlen)
    {
        strncpy (str, type.c_str (), maxlen);
        str[maxlen - 1] = 0;
    }
    else
    {
        strcpy (str, type.c_str ());
    }
    return helicsOK;
}

helicsStatus helicsGetSubscriptionUnits (helics_subscription sub, char *str, int maxlen)
{
    if (sub == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
    auto type = subObj->subptr->getUnits ();
    if (static_cast<int> (type.size ()) > maxlen)
    {
        strncpy (str, type.c_str (), maxlen);
        str[maxlen - 1] = 0;
    }
    else
    {
        strcpy (str, type.c_str ());
    }
    return helicsOK;
}

helicsStatus helicsGetPublicationUnits (helics_publication pub, char *str, int maxlen)
{
    if (pub == nullptr)
    {
        return helicsError;
    }
    auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
    auto type = pubObj->pubptr->getUnits ();
    if (static_cast<int> (type.size ()) > maxlen)
    {
        strncpy (str, type.c_str (), maxlen);
        str[maxlen - 1] = 0;
    }
    else
    {
        strcpy (str, type.c_str ());
    }
    return helicsOK;
}

int helicsIsValueUpdated (helics_subscription sub)
{
    if (sub == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
    auto val = subObj->subptr->isUpdated ();
    return (val) ? 1 : 0;
}

helics_time_t helicsGetLastUpdateTime (helics_subscription sub)
{
    if (sub == nullptr)
    {
        return helicsError;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
    auto time = subObj->subptr->getLastUpdate ();
    return time.getBaseTimeCode ();
}
