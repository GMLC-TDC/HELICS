/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "../core/core-exceptions.hpp"
#include "../helics.hpp"
#include "ValueFederate.h"
#include "internal/api_objects.h"
#include <map>
#include <memory>
#include <mutex>
#include <vector>

static inline void addSubscription (helics_federate fed, std::unique_ptr<helics::SubscriptionObject> sub)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    fedObj->subs.push_back (std::move (sub));
}

static inline void addPublication (helics_federate fed, std::unique_ptr<helics::PublicationObject> pub)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    fedObj->pubs.push_back (std::move (pub));
}

const std::string nullStr;

/* sub/pub registration */
helics_subscription helicsFederateRegisterSubscription (helics_federate fed, const char *key, const char *type, const char *units)
{
    if ((type == nullptr) || (std::string (type).empty ()))
    {  // empty type should default to a regular subscription
        auto fedObj = getValueFedSharedPtr (fed);
        if (!fedObj)
        {
            return nullptr;
        }
        try
        {
            auto sub = std::make_unique<helics::SubscriptionObject> ();
            sub->subptr = std::make_unique<helics::Subscription> (fedObj, key, (units == nullptr) ? nullStr : std::string (units));
            sub->fedptr = std::move (fedObj);
            auto ret = reinterpret_cast<helics_subscription> (sub.get ());
            addSubscription (fed, std::move (sub));
            return ret;
        }
        catch (const helics::InvalidFunctionCall &)
        {
        }
        return nullptr;
    }
    auto htype = helics::getTypeFromString (type);
    if (htype != helics::helics_type_t::helicsInvalid)
    {
        return helicsFederateRegisterTypeSubscription (fed, key, static_cast<int> (htype), units);
    }
    // now generate a generic subscription if we have an unrecognized type
    auto fedObj = getValueFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }
    try
    {
        auto sub = std::make_unique<helics::SubscriptionObject> ();
        sub->id = fedObj->registerRequiredSubscription (key, type, (units == nullptr) ? nullStr : std::string (units));
        sub->rawOnly = true;
        sub->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_subscription> (sub.get ());
        addSubscription (fed, std::move (sub));
        return ret;
    }
    catch (const helics::InvalidFunctionCall &)
    {
    }
    return nullptr;
}
helics_subscription helicsFederateRegisterTypeSubscription (helics_federate fed, const char *key, int type, const char *units)
{
    if ((type < 0) || (type > HELICS_DATA_TYPE_BOOLEAN))
    {
        if (type == HELICS_DATA_TYPE_RAW)
        {
            return helicsFederateRegisterSubscription (fed, key, "", units);
        }
        return nullptr;
    }
    auto fedObj = getValueFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }

    try
    {
        auto sub = std::make_unique<helics::SubscriptionObject> ();
        sub->subptr = std::make_unique<helics::Subscription> (fedObj.get (), key, static_cast<helics::helics_type_t> (type),
                                                              (units == nullptr) ? nullStr : std::string (units));
        sub->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_subscription> (sub.get ());
        addSubscription (fed, std::move (sub));
        return ret;
    }
    catch (const helics::InvalidFunctionCall &)
    {
    }
    return nullptr;
}

/* sub/pub registration */
helics_subscription helicsFederateRegisterOptionalSubscription (helics_federate fed, const char *key, const char *type, const char *units)
{
    if ((type == nullptr) || (std::string (type).empty ()))
    {  // empty type should default to a regular subscription
        auto fedObj = getValueFedSharedPtr (fed);
        if (!fedObj)
        {
            return nullptr;
        }
        try
        {
            auto sub = std::make_unique<helics::SubscriptionObject> ();
            sub->subptr = std::make_unique<helics::Subscription> (helics::OPTIONAL, fedObj.get (), key,
                                                                  (units == nullptr) ? nullStr : std::string (units));
            sub->fedptr = std::move (fedObj);
            auto ret = reinterpret_cast<helics_subscription> (sub.get ());
            addSubscription (fed, std::move (sub));
            return ret;
        }
        catch (const helics::InvalidFunctionCall &)
        {
        }
        return nullptr;
    }
    auto htype = helics::getTypeFromString (type);
    if (htype != helics::helics_type_t::helicsInvalid)
    {
        return helicsFederateRegisterOptionalTypeSubscription (fed, key, static_cast<int> (htype), units);
    }
    // now generate a generic subscription if we have an unrecognized type
    auto fedObj = getValueFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }
    try
    {
        auto sub = std::make_unique<helics::SubscriptionObject> ();
        sub->id = fedObj->registerOptionalSubscription (key, type, (units == nullptr) ? nullStr : std::string (units));
        sub->rawOnly = true;
        sub->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_subscription> (sub.get ());
        addSubscription (fed, std::move (sub));
        return ret;
    }
    catch (const helics::InvalidFunctionCall &)
    {
    }
    return nullptr;
}

helics_subscription helicsFederateRegisterOptionalTypeSubscription (helics_federate fed, const char *key, int type, const char *units)
{
    if ((type < 0) || (type > HELICS_DATA_TYPE_BOOLEAN))
    {
        if (type == HELICS_DATA_TYPE_RAW)
        {
            return helicsFederateRegisterOptionalSubscription (fed, key, "", units);
        }
        return nullptr;
    }
    auto fedObj = getValueFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }

    try
    {
        auto sub = std::make_unique<helics::SubscriptionObject> ();
        sub->subptr = std::make_unique<helics::Subscription> (fedObj.get (), key, static_cast<helics::helics_type_t> (type),
                                                              (units == nullptr) ? nullStr : std::string (units));
        sub->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_subscription> (sub.get ());
        addSubscription (fed, std::move (sub));
        return ret;
    }
    catch (const helics::InvalidFunctionCall &)
    {
    }
    return nullptr;
}

helics_publication helicsFederateRegisterPublication (helics_federate fed, const char *key, const char *type, const char *units)
{
    auto htype = (type != nullptr) ? helics::getTypeFromString (type) : helics::helics_type_t::helicsInvalid;
    if (htype != helics::helics_type_t::helicsInvalid)
    {
        return helicsFederateRegisterTypePublication (fed, key, static_cast<int> (htype), units);
    }
    // now generate a generic subscription
    auto fedObj = getValueFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }
    try
    {
        auto pub = std::make_unique<helics::PublicationObject> ();
        pub->id = fedObj->registerPublication (key, (type == nullptr) ? nullStr : std::string (type),
                                               (units == nullptr) ? nullStr : std::string (units));
        pub->rawOnly = true;
        pub->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_publication> (pub.get ());
        addPublication (fed, std::move (pub));
        return ret;
    }
    catch (const helics::InvalidFunctionCall &)
    {
    }
    return nullptr;
}
helics_publication helicsFederateRegisterTypePublication (helics_federate fed, const char *key, int type, const char *units)
{
    if ((type < 0) || (type > HELICS_DATA_TYPE_BOOLEAN))
    {
        if (type == HELICS_DATA_TYPE_RAW)
        {
            return helicsFederateRegisterPublication (fed, key, "", units);
        }
        return nullptr;
    }
    auto fedObj = getValueFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }
    try
    {
        auto pub = std::make_unique<helics::PublicationObject> ();
        pub->pubptr = std::make_unique<helics::Publication> (fedObj.get (), key, static_cast<helics::helics_type_t> (type),
                                                             (units == nullptr) ? nullStr : std::string (units));
        pub->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_publication> (pub.get ());
        addPublication (fed, std::move (pub));
        return ret;
    }
    catch (const helics::InvalidFunctionCall &)
    {
    }
    return nullptr;
}

helics_publication helicsFederateRegisterGlobalPublication (helics_federate fed, const char *key, const char *type, const char *units)
{
    auto htype = (type != nullptr) ? helics::getTypeFromString (type) : helics::helics_type_t::helicsInvalid;
    if (htype != helics::helics_type_t::helicsInvalid)
    {
        return helicsFederateRegisterGlobalTypePublication (fed, key, static_cast<int> (htype), units);
    }
    // now generate a generic subscription
    auto fedObj = getValueFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }
    try
    {
        auto pub = std::make_unique<helics::PublicationObject> ();
        pub->id = fedObj->registerGlobalPublication (key, (type == nullptr) ? nullStr : std::string (type),
                                                     (units == nullptr) ? nullStr : std::string (units));
        pub->rawOnly = true;
        pub->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_publication> (pub.get ());
        addPublication (fed, std::move (pub));
        return ret;
    }
    catch (const helics::InvalidFunctionCall &)
    {
    }
    return nullptr;
}

helics_publication helicsFederateRegisterGlobalTypePublication (helics_federate fed, const char *key, int type, const char *units)
{
    if ((type < 0) || (type > HELICS_DATA_TYPE_BOOLEAN))
    {
        if (type == HELICS_DATA_TYPE_RAW)
        {
            return helicsFederateRegisterGlobalPublication (fed, key, "", units);
        }
        return nullptr;
    }
    auto fedObj = getValueFedSharedPtr (fed);
    if (!fedObj)
    {
        return nullptr;
    }
    try
    {
        auto pub = std::make_unique<helics::PublicationObject> ();
        pub->pubptr = std::make_unique<helics::Publication> (helics::GLOBAL, fedObj.get (), key, static_cast<helics::helics_type_t> (type),
                                                             (units == nullptr) ? nullStr : std::string (units));
        pub->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_publication> (pub.get ());
        addPublication (fed, std::move (pub));
        return ret;
    }
    catch (const helics::InvalidFunctionCall &)
    {
    }
    return nullptr;
}

/* getting and publishing values */
helics_status helicsPublicationPublishRaw (helics_publication pub, const void *data, int datalen)
{
    if (pub == nullptr)
    {
        return helics_error;
    }
    try
    {
        auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
        if (pubObj->rawOnly)
        {
            pubObj->fedptr->publish (pubObj->id, (const char *)data, datalen);
        }
        else
        {
            pubObj->fedptr->publish (pubObj->pubptr->getID (), (const char *)data, datalen);
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsPublicationPublishString (helics_publication pub, const char *str)
{
    if (pub == nullptr)
    {
        return helics_error;
    }
    try
    {
        auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
        if (pubObj->rawOnly)
        {
            pubObj->fedptr->publish (pubObj->id, (str != nullptr) ? str : "");
        }
        else
        {
            pubObj->pubptr->publish ((str != nullptr) ? str : "");
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsPublicationPublishInteger (helics_publication pub, int64_t val)
{
    if (pub == nullptr)
    {
        return helics_error;
    }
    try
    {
        auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
        if (pubObj->rawOnly)
        {
            pubObj->fedptr->publish (pubObj->id, val);
        }
        else
        {
            pubObj->pubptr->publish (val);
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsPublicationPublishBoolean (helics_publication pub, helics_bool_t val)
{
    if (pub == nullptr)
    {
        return helics_error;
    }
    try
    {
        auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
        if (pubObj->rawOnly)
        {
            pubObj->fedptr->publish (pubObj->id, (val != helics_false) ? "0" : "1");
        }
        else
        {
            pubObj->pubptr->publish ((val != helics_false) ? true : false);
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsPublicationPublishDouble (helics_publication pub, double val)
{
    if (pub == nullptr)
    {
        return helics_error;
    }
    try
    {
        auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
        if (pubObj->rawOnly)
        {
            pubObj->fedptr->publish (pubObj->id, val);
        }
        else
        {
            pubObj->pubptr->publish (val);
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsPublicationPublishComplex (helics_publication pub, double real, double imag)
{
    if (pub == nullptr)
    {
        return helics_error;
    }
    try
    {
        auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
        if (pubObj->rawOnly)
        {
            pubObj->fedptr->publish (pubObj->id, std::complex<double> (real, imag));
        }
        else
        {
            pubObj->pubptr->publish (std::complex<double> (real, imag));
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsPublicationPublishVector (helics_publication pub, const double *vectorInput, int vectorlength)
{
    if (pub == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
        if ((vectorInput == nullptr) || (vectorlength <= 0))
        {
            if (pubObj->rawOnly)
            {
                pubObj->fedptr->publish (pubObj->id, std::vector<double> ());
            }
            else
            {
                pubObj->pubptr->publish (std::vector<double> ());
            }
        }
        else
        {
            if (pubObj->rawOnly)
            {
                pubObj->fedptr->publish (pubObj->id, std::vector<double> (vectorInput, vectorInput + vectorlength));
            }
            else
            {
                pubObj->pubptr->publish (std::vector<double> (vectorInput, vectorInput + vectorlength));
            }
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsPublicationPublishNamedPoint (helics_publication pub, const char *str, double val)
{
    if (pub == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
        if (str == nullptr)
        {
            if (pubObj->rawOnly)
            {
                pubObj->fedptr->publish (pubObj->id, helics::named_point (std::string (), val));
            }
            else
            {
                pubObj->pubptr->publish (std::string (), val);
            }
        }
        else
        {
            if (pubObj->rawOnly)
            {
                pubObj->fedptr->publish (pubObj->id, helics::named_point (str, val));
            }
            else
            {
                pubObj->pubptr->publish (str, val);
            }
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

int helicsSubscriptionGetValueSize (helics_subscription sub)
{
    if (sub == nullptr)
    {
        return helics_invalid_object;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
    if (subObj->rawOnly)
    {
        auto dv = subObj->fedptr->getValueRaw (subObj->id);
        return static_cast<int> (dv.size ());
    }
    return static_cast<int> (subObj->subptr->getRawSize());
}

helics_status helicsSubscriptionGetRawValue (helics_subscription sub, void *data, int maxDatalen, int *actualSize)
{
    if (sub == nullptr)
    {
        return helics_invalid_object;
    }
    if ((data == nullptr) || (maxDatalen < 0))
    {
        return helics_invalid_argument;
    }
    try
    {
        auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
        if (subObj->rawOnly)
        {
            auto dv = subObj->fedptr->getValueRaw (subObj->id);
            if (maxDatalen > static_cast<int> (dv.size ()))
            {
                memcpy (data, dv.data (), dv.size ());
                if (actualSize != nullptr)
                {
                    *actualSize = static_cast<int> (dv.size ());
                }

                return helics_ok;
            }
            memcpy (data, dv.data (), maxDatalen);
            if (actualSize != nullptr)
            {
                *actualSize = maxDatalen;
            }
            return helics_warning;
        }

        auto str = subObj->subptr->getValue<std::string> ();
        if (maxDatalen > static_cast<int> (str.size ()))
        {
            memcpy (data, str.data (), static_cast<int> (str.size ()));
            *actualSize = static_cast<int> (str.size ());
            return helics_ok;
        }
        memcpy (data, str.data (), maxDatalen);
        *actualSize = maxDatalen;
        return helics_warning;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsSubscriptionGetString (helics_subscription sub, char *outputString, int maxlen, int *actualLength)
{
    if (sub == nullptr)
    {
        return helics_invalid_object;
    }

    if ((outputString == nullptr) || (maxlen <= 0))
    {
        return helics_invalid_argument;
    }
    try
    {
        auto res = helicsSubscriptionGetRawValue (sub, outputString, maxlen, actualLength);
        // make sure we have a null terminator
        if (*actualLength == maxlen)
        {
            outputString[maxlen - 1] = '\0';
            return helics_warning;
        }
        outputString[*actualLength] = '\0';
        return res;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsSubscriptionGetInteger (helics_subscription sub, int64_t *val)
{
    if (sub == nullptr)
    {
        return helics_invalid_object;
    }
    if (val == nullptr)
    {
        return helics_invalid_argument;
    }
    try
    {
        auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
        if (subObj->rawOnly)
        {
            *val = subObj->fedptr->getValue<int64_t> (subObj->id);
        }
        else
        {
            subObj->subptr->getValue (*val);
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsSubscriptionGetBoolean (helics_subscription sub, helics_bool_t *val)
{
    if (sub == nullptr)
    {
        return helics_invalid_object;
    }
    if (val == nullptr)
    {
        return helics_invalid_argument;
    }
    try
    {
        auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
        bool boolval;
        if (subObj->rawOnly)
        {
            auto str = subObj->fedptr->getValue<std::string> (subObj->id);
            if (str.size () == 1)
            {
                boolval = (str[0] != '0');
            }
            else if (str.size () == 9)
            {
                auto ival = subObj->fedptr->getValue<int64_t> (subObj->id);
                boolval = (ival != 0);
            }
            else
            {
                boolval = true;
            }
        }
        else
        {
            boolval = subObj->subptr->getValue<bool> ();
        }
        *val = (boolval) ? helics_true : helics_false;
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsSubscriptionGetDouble (helics_subscription sub, double *val)
{
    if (sub == nullptr)
    {
        return helics_invalid_object;
    }
    if (val == nullptr)
    {
        return helics_invalid_argument;
    }
    try
    {
        auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
        if (subObj->rawOnly)
        {
            *val = subObj->fedptr->getValue<double> (subObj->id);
        }
        else
        {
            *val = subObj->subptr->getValue<double> ();
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsSubscriptionGetComplex (helics_subscription sub, double *real, double *imag)
{
    if (sub == nullptr)
    {
        return helics_invalid_object;
    }
    if ((real == nullptr) || (imag == nullptr))
    {
        return helics_invalid_argument;
    }
    try
    {
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
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

int helicsSubscriptionGetVectorSize (helics_subscription sub)
{
    if (sub == nullptr)
    {
        return 0;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
    if (subObj->rawOnly)
    {
        auto V = subObj->fedptr->getValue<std::vector<double>> (subObj->id);
        return static_cast<int> (V.size ());
    }

    return static_cast<int> (subObj->subptr->getVectorSize());
}

helics_status helicsSubscriptionGetVector (helics_subscription sub, double data[], int maxlen, int *actualSize)
{
    if (sub == nullptr)
    {
        return helics_invalid_object;
    }
    if ((data == nullptr) || (maxlen <= 0))
    {
        return helics_invalid_argument;
    }
    try
    {
        auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
        if (subObj->rawOnly)
        {
            auto V = subObj->fedptr->getValue<std::vector<double>> (subObj->id);
            int length = std::min (static_cast<int> (V.size ()), maxlen);
            std::copy (V.data (), V.data () + length, data);
            if (actualSize != nullptr)
            {
                *actualSize = length;
            }
            return (length < maxlen) ? helics_ok : helics_warning;
        }

        auto V = subObj->subptr->getValue<std::vector<double>> ();
        int length = std::min (static_cast<int> (V.size ()), maxlen);
        std::copy (V.data (), V.data () + length, data);
        if (actualSize != nullptr)
        {
            *actualSize = length;
        }
        return (length < maxlen) ? helics_ok : helics_warning;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status
helicsSubscriptionGetNamedPoint (helics_subscription sub, char *outputString, int maxStringlen, int *actualLength, double *val)
{
    if (sub == nullptr)
    {
        return helics_invalid_object;
    }
    if ((outputString == nullptr) || (maxStringlen <= 0))
    {
        return helics_invalid_argument;
    }
    try
    {
        auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
        helics::named_point np;
        if (subObj->rawOnly)
        {
            np = subObj->fedptr->getValue<helics::named_point> (subObj->id);
        }
        else
        {
            np = subObj->subptr->getValue<helics::named_point> ();
        }
        int length = std::min (static_cast<int> (np.name.size ()), maxStringlen);
        memcpy (outputString, np.name.data (), length);

        if (length == maxStringlen)
        {
            outputString[maxStringlen - 1] = '\0';
        }
        else
        {
            outputString[length] = '\0';
        }
        if (actualLength != nullptr)
        {
            *actualLength = length;
        }
        if (val != nullptr)
        {
            *val = np.value;
        }
        return (length < maxStringlen) ? helics_ok : helics_warning;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsSubscriptionSetDefaultRaw (helics_subscription sub, const void *data, int dataLen)
{
    if (sub == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);

        if ((data == nullptr) || (dataLen <= 0))
        {
            subObj->fedptr->setDefaultValue (subObj->id, std::string ());
        }
        else
        {
            subObj->fedptr->setDefaultValue (subObj->id, helics::data_view ((const char *)data, dataLen));
        }

        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsSubscriptionSetDefaultString (helics_subscription sub, const char *str)
{
    if (sub == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
        if (subObj->rawOnly)
        {
            subObj->fedptr->setDefaultValue (subObj->id, helics::data_view ((str == nullptr) ? str : ""));
        }
        else
        {
            subObj->subptr->setDefault<std::string> (str);
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsSubscriptionSetDefaultInteger (helics_subscription sub, int64_t val)
{
    if (sub == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
        if (subObj->rawOnly)
        {
            subObj->fedptr->setDefaultValue (subObj->id, val);
        }
        else
        {
            subObj->subptr->setDefault (val);
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsSubscriptionSetDefaultBoolean (helics_subscription sub, helics_bool_t val)
{
    if (sub == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
        if (subObj->rawOnly)
        {
            subObj->fedptr->setDefaultValue (subObj->id, helics::data_view ((val != helics_false) ? "1" : "0"));
        }
        else
        {
            subObj->subptr->setDefault ((val != helics_false) ? true : false);
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsSubscriptionSetDefaultDouble (helics_subscription sub, double val)
{
    if (sub == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
        if (subObj->rawOnly)
        {
            subObj->fedptr->setDefaultValue (subObj->id, val);
        }
        else
        {
            subObj->subptr->setDefault (val);
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}
helics_status helicsSubscriptionSetDefaultComplex (helics_subscription sub, double real, double imag)
{
    if (sub == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
        if (subObj->rawOnly)
        {
            subObj->fedptr->setDefaultValue (subObj->id, std::complex<double> (real, imag));
        }
        else
        {
            subObj->subptr->setDefault (std::complex<double> (real, imag));
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsSubscriptionSetDefaultVector (helics_subscription sub, const double *vectorInput, int vectorlength)
{
    if (sub == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
        if ((vectorInput == nullptr) || (vectorlength <= 0))
        {
            if (subObj->rawOnly)
            {
                subObj->fedptr->setDefaultValue (subObj->id, std::vector<double>{});
            }
            else
            {
                subObj->subptr->setDefault (std::vector<double>{});
            }
        }
        else
        {
            if (subObj->rawOnly)
            {
                subObj->fedptr->setDefaultValue (subObj->id, std::vector<double> (vectorInput, vectorInput + vectorlength));
            }
            else
            {
                subObj->subptr->setDefault (std::vector<double> (vectorInput, vectorInput + vectorlength));
            }
        }

        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsSubscriptionSetDefaultNamedPoint (helics_subscription sub, const char *str, double val)
{
    if (sub == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
        if (subObj->rawOnly)
        {
            subObj->fedptr->setDefaultValue (subObj->id, helics::named_point ((str != nullptr) ? str : "", val));
        }
        else
        {
            subObj->subptr->setDefault (helics::named_point ((str != nullptr) ? str : "", val));
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsSubscriptionGetType (helics_subscription sub, char *outputString, int maxlen)
{
    if (sub == nullptr)
    {
        return helics_invalid_object;
    }
    if ((outputString == nullptr) || (maxlen <= 0))
    {
        return helics_invalid_argument;
    }
    try
    {
        auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
        std::string type;
        if (subObj->rawOnly)
        {
            type = subObj->fedptr->getSubscriptionType (subObj->id);
        }
        else
        {
            type = subObj->subptr->getType ();
        }
        if (static_cast<int> (type.size ()) > maxlen)
        {
            strncpy (outputString, type.c_str (), maxlen);
            outputString[maxlen - 1] = 0;
        }
        else
        {
            strcpy (outputString, type.c_str ());
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsPublicationGetType (helics_publication pub, char *outputString, int maxlen)
{
    if (pub == nullptr)
    {
        return helics_invalid_object;
    }
    if ((outputString == nullptr) || (maxlen <= 0))
    {
        return helics_invalid_argument;
    }
    try
    {
        auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
        std::string type;
        if (pubObj->rawOnly)
        {
            type = pubObj->fedptr->getPublicationType (pubObj->id);
        }
        else
        {
            type = pubObj->pubptr->getType ();
        }
        if (static_cast<int> (type.size ()) > maxlen)
        {
            strncpy (outputString, type.c_str (), maxlen);
            outputString[maxlen - 1] = 0;
        }
        else
        {
            strcpy (outputString, type.c_str ());
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsSubscriptionGetKey (helics_subscription sub, char *outputString, int maxlen)
{
    if (sub == nullptr)
    {
        return helics_invalid_object;
    }
    if ((outputString == nullptr) || (maxlen <= 0))
    {
        return helics_invalid_argument;
    }
    try
    {
        auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
        std::string type;
        if (subObj->rawOnly)
        {
            type = subObj->fedptr->getSubscriptionKey (subObj->id);
        }
        else
        {
            type = subObj->subptr->getKey ();
        }
        if (static_cast<int> (type.size ()) > maxlen)
        {
            strncpy (outputString, type.c_str (), maxlen);
            outputString[maxlen - 1] = 0;
        }
        else
        {
            strcpy (outputString, type.c_str ());
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsPublicationGetKey (helics_publication pub, char *outputString, int maxlen)
{
    if (pub == nullptr)
    {
        return helics_invalid_object;
    }
    if ((outputString == nullptr) || (maxlen <= 0))
    {
        return helics_invalid_argument;
    }
    try
    {
        auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
        std::string type;
        if (pubObj->rawOnly)
        {
            type = pubObj->fedptr->getPublicationKey (pubObj->id);
        }
        else
        {
            type = pubObj->pubptr->getKey ();
        }
        if (static_cast<int> (type.size ()) > maxlen)
        {
            strncpy (outputString, type.c_str (), maxlen);
            outputString[maxlen - 1] = 0;
        }
        else
        {
            strcpy (outputString, type.c_str ());
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsSubscriptionGetUnits (helics_subscription sub, char *outputString, int maxlen)
{
    if (sub == nullptr)
    {
        return helics_invalid_object;
    }
    if ((outputString == nullptr) || (maxlen <= 0))
    {
        return helics_invalid_argument;
    }
    try
    {
        auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
        std::string type;
        if (subObj->rawOnly)
        {
            type = subObj->fedptr->getSubscriptionUnits (subObj->id);
        }
        else
        {
            type = subObj->subptr->getUnits ();
        }
        if (static_cast<int> (type.size ()) > maxlen)
        {
            strncpy (outputString, type.c_str (), maxlen);
            outputString[maxlen - 1] = 0;
        }
        else
        {
            strcpy (outputString, type.c_str ());
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsPublicationGetUnits (helics_publication pub, char *outputString, int maxlen)
{
    if (pub == nullptr)
    {
        return helics_invalid_object;
    }
    if ((outputString == nullptr) || (maxlen <= 0))
    {
        return helics_invalid_argument;
    }
    try
    {
        auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
        std::string type;
        if (pubObj->rawOnly)
        {
            type = pubObj->fedptr->getPublicationUnits (pubObj->id);
        }
        else
        {
            type = pubObj->pubptr->getUnits ();
        }
        if (static_cast<int> (type.size ()) > maxlen)
        {
            strncpy (outputString, type.c_str (), maxlen);
            outputString[maxlen - 1] = 0;
        }
        else
        {
            strcpy (outputString, type.c_str ());
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_bool_t helicsSubscriptionIsUpdated (helics_subscription sub)
{
    if (sub == nullptr)
    {
        return helics_false;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
    if (subObj->rawOnly)
    {
        auto val = subObj->fedptr->isUpdated (subObj->id);
        return (val) ? 1 : 0;
    }
    else
    {
        auto val = subObj->subptr->isUpdated ();
        return (val) ? helics_true : helics_false;
    }
}

helics_time_t helicsSubscriptionLastUpdateTime (helics_subscription sub)
{
    if (sub == nullptr)
    {
        return 1e-37;
    }
    auto subObj = reinterpret_cast<helics::SubscriptionObject *> (sub);
    if (subObj->rawOnly)
    {
        auto time = subObj->fedptr->getLastUpdateTime (subObj->id);
        return time.getBaseTimeCode ();
    }
    else
    {
        auto time = subObj->subptr->getLastUpdate ();
        return time.getBaseTimeCode ();
    }
}

int helicsFederateGetPublicationCount (helics_federate fed)
{
    if (fed == nullptr)
    {
        return (-1);
    }
    auto vfedObj = getValueFed (fed);
    if (vfedObj == nullptr)
    {
        auto fedObj = getFed (fed);
        // if this is not nullptr than it is a valid fed object just not a value federate object so it has 0 subscriptions
        return (fedObj != nullptr) ? 0 : (-1);
    }
    return static_cast<int> (vfedObj->getPublicationCount ());
}

int helicsFederateGetSubscriptionCount (helics_federate fed)
{
    if (fed == nullptr)
    {
        return (-1);
    }
    auto vfedObj = getValueFed (fed);
    if (vfedObj == nullptr)
    {
        auto fedObj = getFed (fed);
        // if this is not nullptr than it is a valid fed object just not a value federate object so it has 0 subscriptions
        return (fedObj != nullptr) ? 0 : (-1);
    }
    return static_cast<int> (vfedObj->getSubscriptionCount ());
}
