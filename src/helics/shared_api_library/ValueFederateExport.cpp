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

// random integer for validation purposes of inputs
static const int InputValidationIdentifier = 0x3456'E052;

// random integer for validation purposes of publications
static const int PublicationValidationIdentifier = 0x97B1'00A5;

static const char *invalidInputString = "The given input object does not point to a valid object";

static const char *invalidPublicationString = "The given publication object does not point to a valid object";

static helics::InputObject *verifyInput (helics_input inp, helics_error *err)
{
    HELICS_ERROR_CHECK (err, nullptr);
    if (inp == nullptr)
    {
        if (err != nullptr)
        {
            err->error_code = helics_invalid_object;
            err->message = invalidInputString;
        }
        return nullptr;
    }
    auto inpObj = reinterpret_cast<helics::InputObject *> (inp);
    if (inpObj->valid == InputValidationIdentifier)
    {
        if (err != nullptr)
        {
            err->error_code = helics_invalid_object;
            err->message = invalidInputString;
        }
        return nullptr;
    }
    return inpObj;
}

static helics::PublicationObject *verifyPublication (helics_publication pub, helics_error *err)
{
    HELICS_ERROR_CHECK (err, nullptr);
    if (pub == nullptr)
    {
        if (err != nullptr)
        {
            err->error_code = helics_invalid_object;
            err->message = invalidPublicationString;
        }
        return nullptr;
    }
    auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
    if (pubObj->valid == PublicationValidationIdentifier)
    {
        if (err != nullptr)
        {
            err->error_code = helics_invalid_object;
            err->message = invalidInputString;
        }
        return nullptr;
    }
    return pubObj;
}

static inline void addInput (helics_federate fed, std::unique_ptr<helics::InputObject> inp)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    inp->valid = InputValidationIdentifier;
    fedObj->subs.push_back (std::move (inp));
}

static inline void addPublication (helics_federate fed, std::unique_ptr<helics::PublicationObject> pub)
{
    auto fedObj = reinterpret_cast<helics::FedObject *> (fed);
    pub->valid = PublicationValidationIdentifier;
    fedObj->pubs.push_back (std::move (pub));
}

const std::string nullStr;

/* inp/pub registration */
helics_input helicsFederateRegisterSubscription (helics_federate fed, const char *key, const char *units, helics_error *err)
{
    auto fedObj = getValueFedSharedPtr (fed, err);
    if (!fedObj)
    {
        return nullptr;
    }
    try
    {
        auto sub = std::make_unique<helics::InputObject> ();
        sub->inputPtr = std::make_unique<helics::Subscription> (fedObj, key, (units == nullptr) ? nullStr : std::string (units));
        sub->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_input> (sub.get ());
        addInput (fed, std::move (sub));
        return ret;
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
    return nullptr;
}

helics_publication
helicsFederateRegisterTypePublication (helics_federate fed, const char *key, const char *type, const char *units, helics_error *err)
{
    // now generate a generic subscription
    auto fedObj = getValueFedSharedPtr (fed, err);
    if (!fedObj)
    {
        return nullptr;
    }
    auto htype = (type != nullptr) ? helics::getTypeFromString (type) : helics::helics_type_t::helicsInvalid;
    if (htype != helics::helics_type_t::helicsInvalid)
    {
        return helicsFederateRegisterPublication (fed, key, static_cast<int> (htype), units, err);
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
    catch (...)
    {
        helicsErrorHandler (err);
    }
    return nullptr;
}

helics_publication helicsFederateRegisterPublication (helics_federate fed, const char *key, int type, const char *units, helics_error *err)
{
    auto fedObj = getValueFedSharedPtr (fed, err);
    if (!fedObj)
    {
        return nullptr;
    }
    if ((type < 0) || (type > HELICS_DATA_TYPE_BOOLEAN))
    {
        if (type == HELICS_DATA_TYPE_RAW)
        {
            return helicsFederateRegisterTypePublication (fed, key, "", units, err);
        }
        if (err != nullptr)
        {
            err->error_code = helics_invalid_argument;
            err->message = getMasterHolder ()->addErrorString ("unrecognized type code");
        }
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
    catch (...)
    {
        helicsErrorHandler (err);
    }
    return nullptr;
}

helics_publication
helicsFederateRegisterGlobalTypePublication (helics_federate fed, const char *key, const char *type, const char *units, helics_error *err)
{
    // now generate a generic subscription
    auto fedObj = getValueFedSharedPtr (fed, err);
    if (!fedObj)
    {
        return nullptr;
    }
    auto htype = (type != nullptr) ? helics::getTypeFromString (type) : helics::helics_type_t::helicsInvalid;
    if (htype != helics::helics_type_t::helicsInvalid)
    {
        return helicsFederateRegisterGlobalPublication (fed, key, static_cast<int> (htype), units, err);
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
    catch (...)
    {
        helicsErrorHandler (err);
    }
    return nullptr;
}

helics_publication
helicsFederateRegisterGlobalPublication (helics_federate fed, const char *key, int type, const char *units, helics_error *err)
{
    auto fedObj = getValueFedSharedPtr (fed, err);
    if (!fedObj)
    {
        return nullptr;
    }
    if ((type < 0) || (type > HELICS_DATA_TYPE_BOOLEAN))
    {
        if (type == HELICS_DATA_TYPE_RAW)
        {
            return helicsFederateRegisterGlobalTypePublication (fed, key, "", units, err);
        }
        if (err != nullptr)
        {
            err->error_code = helics_invalid_argument;
            err->message = getMasterHolder ()->addErrorString ("unrecognized type code");
        }
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
    catch (...)
    {
        helicsErrorHandler (err);
    }
    return nullptr;
}

/* getting and publishing values */
void helicsPublicationPublishRaw (helics_publication pub, const void *data, int datalen, helics_error *err)
{
    auto pubObj = verifyPublication (pub, err);
    if (pubObj == nullptr)
    {
        return;
    }
    try
    {
        if (pubObj->rawOnly)
        {
            pubObj->fedptr->publish (pubObj->id, reinterpret_cast<const char *> (data), datalen);
        }
        else
        {
            pubObj->fedptr->publish (pubObj->pubptr->getID (), reinterpret_cast<const char *> (data), datalen);
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsPublicationPublishString (helics_publication pub, const char *str, helics_error *err)
{
    auto pubObj = verifyPublication (pub, err);
    if (pubObj == nullptr)
    {
        return;
    }
    try
    {
        if (pubObj->rawOnly)
        {
            pubObj->fedptr->publish (pubObj->id, (str != nullptr) ? str : "");
        }
        else
        {
            pubObj->pubptr->publish ((str != nullptr) ? str : "");
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsPublicationPublishInteger (helics_publication pub, int64_t val, helics_error *err)
{
    auto pubObj = verifyPublication (pub, err);
    if (pubObj == nullptr)
    {
        return;
    }
    try
    {
        if (pubObj->rawOnly)
        {
            pubObj->fedptr->publish (pubObj->id, val);
        }
        else
        {
            pubObj->pubptr->publish (val);
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsPublicationPublishBoolean (helics_publication pub, helics_bool_t val, helics_error *err)
{
    auto pubObj = verifyPublication (pub, err);
    if (pubObj == nullptr)
    {
        return;
    }
    try
    {
        if (pubObj->rawOnly)
        {
            pubObj->fedptr->publish (pubObj->id, (val != helics_false) ? "0" : "1");
        }
        else
        {
            pubObj->pubptr->publish ((val != helics_false));
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsPublicationPublishDouble (helics_publication pub, double val, helics_error *err)
{
    auto pubObj = verifyPublication (pub, err);
    if (pubObj == nullptr)
    {
        return;
    }
    try
    {
        if (pubObj->rawOnly)
        {
            pubObj->fedptr->publish (pubObj->id, val);
        }
        else
        {
            pubObj->pubptr->publish (val);
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsPublicationPublishComplex (helics_publication pub, double real, double imag, helics_error *err)
{
    auto pubObj = verifyPublication (pub, err);
    if (pubObj == nullptr)
    {
        return;
    }
    try
    {
        if (pubObj->rawOnly)
        {
            pubObj->fedptr->publish (pubObj->id, std::complex<double> (real, imag));
        }
        else
        {
            pubObj->pubptr->publish (std::complex<double> (real, imag));
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsPublicationPublishVector (helics_publication pub, const double *vectorInput, int vectorlength, helics_error *err)
{
    auto pubObj = verifyPublication (pub, err);
    if (pubObj == nullptr)
    {
        return;
    }
    try
    {
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
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsPublicationPublishNamedPoint (helics_publication pub, const char *str, double val, helics_error *err)
{
    auto pubObj = verifyPublication (pub, err);
    if (pubObj == nullptr)
    {
        return;
    }
    try
    {
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
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

int helicsInputGetRawValueSize (helics_input inp, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return (-1);
    }
    if (inpObj->rawOnly)
    {
        auto dv = inpObj->fedptr->getValueRaw (inpObj->id);
        return static_cast<int> (dv.size ());
    }
    return static_cast<int> (inpObj->inputPtr->getRawSize ());
}

int helicsInputGetRawValue (helics_input inp, void *data, int maxDatalen, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return (-1);
    }
    if (!checkOutArgString (static_cast<char *>(data), maxDatalen, err))
    {
        return (-1);
    }
    try
    {
        if (inpObj->rawOnly)
        {
            auto dv = inpObj->fedptr->getValueRaw (inpObj->id);
            if (maxDatalen > static_cast<int> (dv.size ()))
            {
                memcpy (data, dv.data (), dv.size ());
                return static_cast<int> (dv.size ());
            }
            memcpy (data, dv.data (), maxDatalen);
            return maxDatalen;
        }

        auto str = inpObj->inputPtr->getValue<std::string> ();
        if (maxDatalen > static_cast<int> (str.size ()))
        {
            memcpy (data, str.data (), static_cast<int> (str.size ()));
            return static_cast<int> (str.size ());
        }
        memcpy (data, str.data (), maxDatalen);
        return maxDatalen;
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

int helicsInputGetString (helics_input inp, char *outputString, int maxlen, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return (-1);
    }

    if (!checkOutArgString (outputString, maxlen, err))
    {
        return (-1);
    }
    try
    {
        if (inpObj->rawOnly)
        {
            int length=helicsInputGetRawValue (inp, outputString, maxlen,err);
            // make sure we have a null terminator
            if (length == maxlen)
            {
                outputString[maxlen - 1] = '\0';
                return maxlen;
            }
            outputString[length] = '\0';
            return length;
        }
        return inpObj->inputPtr->getValue (outputString, maxlen);
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return (-1);
    }
}

int64_t helicsInputGetInteger (helics_input inp, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return (-101);
    }
    try
    {
        if (inpObj->rawOnly)
        {
            return inpObj->fedptr->getValue<int64_t> (inpObj->id);
        }
        return inpObj->inputPtr->getValue<int64_t> ();
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return (-101);
    }
}

helics_bool_t helicsInputGetBoolean (helics_input inp, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return helics_false;
    }
    try
    {
        bool boolval;
        if (inpObj->rawOnly)
        {
            auto str = inpObj->fedptr->getValue<std::string> (inpObj->id);
            if (str.size () == 1)
            {
                boolval = ((str[0] != '0') && (str[0] != 'f') && (str[0] != 'F'));
            }
            else if (str.size () == 9)
            {
                auto ival = inpObj->fedptr->getValue<int64_t> (inpObj->id);
                boolval = (ival != 0);
            }
            else if (str == "false")
            {
                boolval = false;
            }
            else
            {
                boolval = true;
            }
        }
        else
        {
            boolval = inpObj->inputPtr->getValue<bool> ();
        }
        return (boolval) ? helics_true : helics_false;
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return helics_false;
    }
}

double helicsInputGetDouble (helics_input inp, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return helics_time_invalid;
    }
    try
    {
        if (inpObj->rawOnly)
        {
            return inpObj->fedptr->getValue<double> (inpObj->id);
        }
        return inpObj->inputPtr->getValue<double> ();
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return helics_time_invalid;
    }
}

void helicsInputGetComplex (helics_input inp, double *real, double *imag, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return;
    }
    if ((real == nullptr) && (imag == nullptr))
    {
        // no errors here the caller just didn't want any values
        return;
    }
    try
    {
        if (inpObj->rawOnly)
        {
            auto cval = inpObj->fedptr->getValue<std::complex<double>> (inpObj->id);
            if (real != nullptr)
            {
                *real = cval.real ();
            }
            if (imag != nullptr)
            {
                *imag = cval.imag ();
            }
        }
        else
        {
            auto cval = inpObj->inputPtr->getValue<std::complex<double>> ();
            if (real != nullptr)
            {
                *real = cval.real ();
            }
            if (imag != nullptr)
            {
                *imag = cval.imag ();
            }
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

int helicsInputGetVectorSize (helics_input inp, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return 0;
    }
    try
    {
        if (inpObj->rawOnly)
        {
            auto V = inpObj->fedptr->getValue<std::vector<double>> (inpObj->id);
            return static_cast<int> (V.size ());
        }

        return static_cast<int> (inpObj->inputPtr->getVectorSize ());
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return 0;
    }
}

int helicsInputGetStringSize (helics_input inp, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return 0;
    }
    try
    {
        if (inpObj->rawOnly)
        {
            auto str = inpObj->fedptr->getValue<std::string> (inpObj->id);
            return static_cast<int> (str.size ()) + 1;
        }

        return static_cast<int> (inpObj->inputPtr->getStringSize ()) + 1;
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return 0;
    }
}

int helicsInputGetVector (helics_input inp, double data[], int maxlen, int *actualSize, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return (-1);
    }
    if ((data == nullptr) || (maxlen <= 0))
    {
        return (-1);
    }
    try
    {
        if (inpObj->rawOnly)
        {
            auto V = inpObj->fedptr->getValue<std::vector<double>> (inpObj->id);
            int length = std::min (static_cast<int> (V.size ()), maxlen);
            std::copy (V.data (), V.data () + length, data);
            if (actualSize != nullptr)
            {
                *actualSize = length;
            }
            return (length <= maxlen) ? helics_ok : helics_warning;
        }
        int length = inpObj->inputPtr->getValue (data, maxlen);
        if (actualSize != nullptr)
        {
            *actualSize = length;
        }
        return length;
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return (-1);
    }
}

int helicsInputGetNamedPoint (helics_input inp, char *outputString, int maxlen, double *val, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return (-1);
    }
    if (!checkOutArgString (outputString, maxlen, err))
    {
        return (-1);
    }
    try
    {
        helics::named_point np;
        if (inpObj->rawOnly)
        {
            np = inpObj->fedptr->getValue<helics::named_point> (inpObj->id);
        }
        else
        {
            np = inpObj->inputPtr->getValue<helics::named_point> ();
        }
        int length = std::min (static_cast<int> (np.name.size ()), maxlen);
        memcpy (outputString, np.name.data (), length);

        if (length == maxlen)
        {
            outputString[maxlen - 1] = '\0';
        }
        else
        {
            outputString[length] = '\0';
        }

        if (val != nullptr)
        {
            *val = np.value;
        }
        return length;
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return (-1);
    }
}

void helicsInputSetDefaultRaw (helics_input inp, const void *data, int dataLen, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return;
    }
    try
    {
        if ((data == nullptr) || (dataLen <= 0))
        {
            inpObj->fedptr->setDefaultValue (inpObj->id, std::string ());
        }
        else
        {
            inpObj->fedptr->setDefaultValue (inpObj->id, helics::data_view (static_cast<const char *> (data), dataLen));
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsInputSetDefaultString (helics_input inp, const char *str, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return;
    }
    try
    {
        if (inpObj->rawOnly)
        {
            inpObj->fedptr->setDefaultValue (inpObj->id, helics::data_view ((str == nullptr) ? str : ""));
        }
        else
        {
            inpObj->inputPtr->setDefault<std::string> (str);
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsInputSetDefaultInteger (helics_input inp, int64_t val, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return;
    }
    try
    {
        if (inpObj->rawOnly)
        {
            inpObj->fedptr->setDefaultValue (inpObj->id, val);
        }
        else
        {
            inpObj->inputPtr->setDefault (val);
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsInputSetDefaultBoolean (helics_input inp, helics_bool_t val, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return;
    }
    try
    {
        if (inpObj->rawOnly)
        {
            inpObj->fedptr->setDefaultValue (inpObj->id, helics::data_view ((val != helics_false) ? "1" : "0"));
        }
        else
        {
            inpObj->inputPtr->setDefault ((val != helics_false));
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsInputSetDefaultDouble (helics_input inp, double val, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return;
    }
    try
    {
        if (inpObj->rawOnly)
        {
            inpObj->fedptr->setDefaultValue (inpObj->id, val);
        }
        else
        {
            inpObj->inputPtr->setDefault (val);
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}
void helicsInputSetDefaultComplex (helics_input inp, double real, double imag, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return;
    }
    try
    {
        if (inpObj->rawOnly)
        {
            inpObj->fedptr->setDefaultValue (inpObj->id, std::complex<double> (real, imag));
        }
        else
        {
            inpObj->inputPtr->setDefault (std::complex<double> (real, imag));
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsInputSetDefaultVector (helics_input inp, const double *vectorInput, int vectorlength, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return;
    }
    try
    {
        if ((vectorInput == nullptr) || (vectorlength <= 0))
        {
            if (inpObj->rawOnly)
            {
                inpObj->fedptr->setDefaultValue (inpObj->id, std::vector<double>{});
            }
            else
            {
                inpObj->inputPtr->setDefault (std::vector<double>{});
            }
        }
        else
        {
            if (inpObj->rawOnly)
            {
                inpObj->fedptr->setDefaultValue (inpObj->id, std::vector<double> (vectorInput, vectorInput + vectorlength));
            }
            else
            {
                inpObj->inputPtr->setDefault (std::vector<double> (vectorInput, vectorInput + vectorlength));
            }
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsInputSetDefaultNamedPoint (helics_input inp, const char *str, double val, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return;
    }
    try
    {
        if (inpObj->rawOnly)
        {
            inpObj->fedptr->setDefaultValue (inpObj->id, helics::named_point ((str != nullptr) ? str : "", val));
        }
        else
        {
            inpObj->inputPtr->setDefault (helics::named_point ((str != nullptr) ? str : "", val));
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

int helicsInputGetType (helics_input inp, char *outputString, int maxlen, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return 0;
    }
    if (!checkOutArgString (outputString, maxlen, err))
    {
        return 0;
    }
    try
    {
        std::string type;
        if (inpObj->rawOnly)
        {
            type = inpObj->fedptr->getInputType (inpObj->id);
        }
        else
        {
            type = inpObj->inputPtr->getType ();
        }
        if (static_cast<int> (type.size ()) > maxlen)
        {
            strncpy (outputString, type.c_str (), maxlen);
            outputString[maxlen - 1] = 0;
            return maxlen;
        }
        else
        {
            strcpy (outputString, type.c_str ());
            return static_cast<int> (type.size ());
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return 0;
    }
}

int helicsPublicationGetType (helics_publication pub, char *outputString, int maxlen, helics_error *err)
{
    auto pubObj = verifyPublication (pub, err);
    if (pubObj == nullptr)
    {
        return (-1);
    }
    if (!checkOutArgString (outputString, maxlen, err))
    {
        return (-1);
    }
    try
    {
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
            return maxlen;
        }
        else
        {
            strcpy (outputString, type.c_str ());
            return static_cast<int> (type.size ());
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return (-1);
    }
}

int helicsInputGetKey (helics_input inp, char *outputString, int maxlen, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return (-1);
    }
    if (!checkOutArgString (outputString, maxlen, err))
    {
        return (-1);
    }
    try
    {
        std::string type;
        if (inpObj->rawOnly)
        {
            type = inpObj->fedptr->getInputKey (inpObj->id);
        }
        else
        {
            type = inpObj->inputPtr->getName ();
        }
        if (static_cast<int> (type.size ()) > maxlen)
        {
            strncpy (outputString, type.c_str (), maxlen);
            outputString[maxlen - 1] = 0;
            return maxlen;
        }
        else
        {
            strcpy (outputString, type.c_str ());
            return static_cast<int> (type.size ());
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return (-1);
    }
}

int helicsPublicationGetKey (helics_publication pub, char *outputString, int maxlen, helics_error *err)
{
    auto pubObj = verifyPublication (pub, err);
    if (pubObj == nullptr)
    {
        return (-1);
    }
    if (!checkOutArgString (outputString, maxlen, err))
    {
        return (-1);
    }
    try
    {
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
            return maxlen;
        }
        else
        {
            strcpy (outputString, type.c_str ());
            return static_cast<int> (type.size ());
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return (-1);
    }
}

int helicsInputGetUnits (helics_input inp, char *outputString, int maxlen, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return (-1);
    }
    if (!checkOutArgString (outputString, maxlen, err))
    {
        return (-1);
    }
    try
    {
        std::string type;
        if (inpObj->rawOnly)
        {
            type = inpObj->fedptr->getInputUnits (inpObj->id);
        }
        else
        {
            type = inpObj->inputPtr->getUnits ();
        }
        if (static_cast<int> (type.size ()) > maxlen)
        {
            strncpy (outputString, type.c_str (), maxlen);
            outputString[maxlen - 1] = 0;
            return maxlen;
        }
        else
        {
            strcpy (outputString, type.c_str ());
            return static_cast<int> (type.size ());
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return (-1);
    }
}

int helicsPublicationGetUnits (helics_publication pub, char *outputString, int maxlen, helics_error *err)
{
    auto pubObj = verifyPublication (pub, err);
    if (pubObj == nullptr)
    {
        return (-1);
    }
    if (!checkOutArgString (outputString, maxlen, err))
    {
        return (-1);
    }
    try
    {
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
            return maxlen;
        }
        else
        {
            strcpy (outputString, type.c_str ());
            return static_cast<int> (type.size ());
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return (-1);
    }
}

helics_bool_t helicsInputIsUpdated (helics_input inp, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return helics_false;
    }
    if (inpObj->rawOnly)
    {
        auto val = inpObj->fedptr->isUpdated (inpObj->id);
        return (val) ? helics_true : helics_false;
    }
    auto val = inpObj->inputPtr->isUpdated ();
    return (val) ? helics_true : helics_false;
}

helics_time_t helicsInputLastUpdateTime (helics_input inp, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return helics_time_invalid;
    }
    try
    {
        if (inpObj->rawOnly)
        {
            auto time = inpObj->fedptr->getLastUpdateTime (inpObj->id);
            return static_cast<helics_time_t> (time);
        }
        auto time = inpObj->inputPtr->getLastUpdate ();
        return static_cast<helics_time_t> (time);
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return helics_time_invalid;
    }
}

int helicsFederateGetPublicationCount (helics_federate fed, helics_error *err)
{
    HELICS_ERROR_CHECK (err, -1);
    // this call should be with a nullptr since it can fail and still be a successful call
    auto vfedObj = getValueFed (fed, nullptr);
    if (vfedObj == nullptr)
    {
        auto fedObj = getFed (fed, err);
        // if this is not nullptr than it is a valid fed object just not a value federate object so it has 0 subscriptions
        return (fedObj != nullptr) ? 0 : (-1);
    }
    return static_cast<int> (vfedObj->getPublicationCount ());
}

int helicsFederateGetInputCount (helics_federate fed, helics_error *err)
{
    HELICS_ERROR_CHECK (err, -1);
    // this call should be with a nullptr since it can fail and still be a successful call
    auto vfedObj = getValueFed (fed,nullptr);
    if (vfedObj == nullptr)
    {
        auto fedObj = getFed (fed, err);
        // if this is not nullptr than it is a valid fed object just not a value federate object so it has 0 subscriptions
        return (fedObj != nullptr) ? 0 : (-1);
    }
    return static_cast<int> (vfedObj->getInputCount ());
}
