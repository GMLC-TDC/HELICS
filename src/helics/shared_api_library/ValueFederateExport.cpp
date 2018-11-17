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
#include <iostream>

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
            err->error_code = helics_error_invalid_object;
            err->message = invalidInputString;
        }
        return nullptr;
    }
    auto inpObj = reinterpret_cast<helics::InputObject *> (inp);
    if (inpObj->valid != InputValidationIdentifier)
    {
        if (err != nullptr)
        {
            err->error_code = helics_error_invalid_object;
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
            err->error_code = helics_error_invalid_object;
            err->message = invalidPublicationString;
        }
        return nullptr;
    }
    auto pubObj = reinterpret_cast<helics::PublicationObject *> (pub);
    if (pubObj->valid != PublicationValidationIdentifier)
    {
        if (err != nullptr)
        {
            err->error_code = helics_error_invalid_object;
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
    fedObj->inputs.push_back (std::move (inp));
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
            err->error_code = helics_error_invalid_argument;
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
            err->error_code = helics_error_invalid_argument;
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

helics_input helicsFederateRegisterTypeInput (helics_federate fed, const char *key, const char *type, const char *units, helics_error *err)
{
    // now generate a generic subscription
    auto fedObj = getValueFedSharedPtr (fed, err);
    if (!fedObj)
    {
        return nullptr;
    }
    auto htype = (type != nullptr) ? helics::getTypeFromString (type) : helics::helics_type_t::helicsAny;
    if (htype != helics::helics_type_t::helicsInvalid)
    {
        return helicsFederateRegisterInput (fed, key, static_cast<int> (htype), units, err);
    }
    try
    {
        auto inp = std::make_unique<helics::InputObject> ();
        inp->id =
          fedObj->registerInput (key, (type == nullptr) ? nullStr : std::string (type), (units == nullptr) ? nullStr : std::string (units));
        inp->rawOnly = true;
        inp->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_input> (inp.get ());
        addInput (fed, std::move (inp));
        return ret;
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
    return nullptr;
}

helics_input helicsFederateRegisterInput (helics_federate fed, const char *key, int type, const char *units, helics_error *err)
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
            return helicsFederateRegisterTypeInput (fed, key, "", units, err);
        }
        if (type != HELICS_DATA_TYPE_ANY)
        {
            if (err != nullptr)
            {
                err->error_code = helics_error_invalid_argument;
                err->message = getMasterHolder ()->addErrorString ("unrecognized type code");
            }
            return nullptr;
        }
    }

    try
    {
        auto inp = std::make_unique<helics::InputObject> ();
        inp->inputPtr = std::make_unique<helics::Input> (fedObj.get (), key, static_cast<helics::helics_type_t> (type),
                                                         (units == nullptr) ? nullStr : std::string (units));
        inp->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_input> (inp.get ());
        addInput (fed, std::move (inp));
        return ret;
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
    return nullptr;
}

helics_input
helicsFederateRegisterGlobalTypeInput (helics_federate fed, const char *key, const char *type, const char *units, helics_error *err)
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
        return helicsFederateRegisterGlobalInput (fed, key, static_cast<int> (htype), units, err);
    }

    try
    {
        auto inp = std::make_unique<helics::InputObject> ();
        inp->id = fedObj->registerGlobalInput (key, (type == nullptr) ? nullStr : std::string (type),
                                               (units == nullptr) ? nullStr : std::string (units));
        inp->rawOnly = true;
        inp->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_input> (inp.get ());
        addInput (fed, std::move (inp));
        return ret;
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
    return nullptr;
}

helics_input helicsFederateRegisterGlobalInput (helics_federate fed, const char *key, int type, const char *units, helics_error *err)
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
            return helicsFederateRegisterGlobalTypeInput (fed, key, "", units, err);
        }
        if (err != nullptr)
        {
            err->error_code = helics_error_invalid_argument;
            err->message = getMasterHolder ()->addErrorString ("unrecognized type code");
        }
        return nullptr;
    }

    try
    {
        auto inp = std::make_unique<helics::InputObject> ();
        inp->inputPtr = std::make_unique<helics::Input> (helics::GLOBAL, fedObj.get (), key, static_cast<helics::helics_type_t> (type),
                                                         (units == nullptr) ? nullStr : std::string (units));
        inp->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_input> (inp.get ());
        addInput (fed, std::move (inp));
        return ret;
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
    return nullptr;
}

static constexpr char invalidPubName[] = "the specified publication name is a not a valid publication name";

helics_publication helicsFederateGetPublication (helics_federate fed, const char *key, helics_error *err)
{
    auto fedObj = getValueFedSharedPtr (fed, err);
    if (!fedObj)
    {
        return nullptr;
    }
    try
    {
        auto id = fedObj->getPublicationId (key);
        if (!id.isValid ())
        {
            if (err != nullptr)
            {
                err->error_code = helics_error_invalid_argument;
                err->message = invalidPubName;
            }
            return nullptr;
        }
        auto pub = std::make_unique<helics::PublicationObject> ();
        pub->pubptr = std::make_unique<helics::Publication> (fedObj.get (), id.value ());
        pub->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_publication> (pub.get ());
        addPublication (fed, std::move (pub));
        return ret;
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return nullptr;
    }
}

helics_publication helicsFederateGetPublicationByIndex (helics_federate fed, int index, helics_error *err)
{
    auto fedObj = getValueFedSharedPtr (fed, err);
    if (!fedObj)
    {
        return nullptr;
    }
    try
    {
        auto pub = std::make_unique<helics::PublicationObject> ();
        pub->pubptr = std::make_unique<helics::Publication> (fedObj.get (), index);
        pub->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_publication> (pub.get ());
        addPublication (fed, std::move (pub));
        return ret;
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return nullptr;
    }
}

static constexpr char invalidInputName[] = "the specified input name is a not a recognized input";

helics_input helicsFederateGetInput (helics_federate fed, const char *key, helics_error *err)
{
    auto fedObj = getValueFedSharedPtr (fed, err);
    if (!fedObj)
    {
        return nullptr;
    }
    try
    {
        auto id = fedObj->getInputId (key);
        if (!id.isValid ())
        {
            if (err != nullptr)
            {
                err->error_code = helics_error_invalid_argument;
                err->message = invalidInputName;
            }
            return nullptr;
        }
        auto inp = std::make_unique<helics::InputObject> ();
        inp->inputPtr = std::make_unique<helics::Input> (fedObj.get (), id.value ());
        inp->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_input> (inp.get ());
        addInput (fed, std::move (inp));
        return ret;
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return nullptr;
    }
}

helics_input helicsFederateGetInputByIndex (helics_federate fed, int index, helics_error *err)
{
    auto fedObj = getValueFedSharedPtr (fed, err);
    if (!fedObj)
    {
        return nullptr;
    }
    try
    {
        auto inp = std::make_unique<helics::InputObject> ();
        inp->inputPtr = std::make_unique<helics::Input> (fedObj.get (), index);
        inp->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_input> (inp.get ());
        addInput (fed, std::move (inp));
        return ret;
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return nullptr;
    }
}

static constexpr char invalidSubKey[] = "the specified subscription key is a not a recognized key";

helics_input helicsFederateGetSubscription (helics_federate fed, const char *key, helics_error *err)
{
    auto fedObj = getValueFedSharedPtr (fed, err);
    if (!fedObj)
    {
        return nullptr;
    }
    try
    {
        auto id = fedObj->getSubscriptionId (key);
        if (!id.isValid ())
        {
            err->error_code = helics_error_invalid_argument;
            err->message = invalidSubKey;
            return nullptr;
        }
        auto inp = std::make_unique<helics::InputObject> ();
        inp->inputPtr = std::make_unique<helics::Subscription> (fedObj.get (), id.value ());
        inp->fedptr = std::move (fedObj);
        auto ret = reinterpret_cast<helics_input> (inp.get ());
        addInput (fed, std::move (inp));
        return ret;
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return nullptr;
    }
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

static constexpr char invalidTargetString[] = "target is not valid";
void helicsPublicationAddTarget (helics_publication pub, const char *target, helics_error *err)
{
    auto pubObj = verifyPublication (pub, err);
    if (pubObj == nullptr)
    {
        return;
    }
    if (target == nullptr)
    {
        if (err != nullptr)
        {
            err->error_code = helics_error_invalid_argument;
            err->message = invalidTargetString;
        }
        return;
    }
    if (pubObj->rawOnly)
    {
        pubObj->fedptr->addTarget (pubObj->id, target);
    }
    else
    {
        pubObj->pubptr->addTarget (target);
    }
}

void helicsInputAddTarget (helics_input ipt, const char *target, helics_error *err)
{
    auto inpObj = verifyInput (ipt, err);
    if (inpObj == nullptr)
    {
        return;
    }
    if (target == nullptr)
    {
        if (err != nullptr)
        {
            err->error_code = helics_error_invalid_argument;
            err->message = invalidTargetString;
        }
        return;
    }
    if (inpObj->rawOnly)
    {
        inpObj->fedptr->addTarget (inpObj->id, target);
    }
    else
    {
        inpObj->inputPtr->addTarget (target);
    }
}

int helicsInputGetRawValueSize (helics_input inp)
{
    auto inpObj = verifyInput (inp, nullptr);
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

void helicsInputGetRawValue (helics_input inp, void *data, int maxDatalen, int *actualSize, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (actualSize != nullptr)
    {  // for initialization
        *actualSize = 0;
    }
    if (inpObj == nullptr)
    {
        return;
    }
    if (!checkOutArgString (static_cast<char *> (data), maxDatalen, err))
    {
        return;
    }
    try
    {
        if (inpObj->rawOnly)
        {
            auto dv = inpObj->fedptr->getValueRaw (inpObj->id);
            if (maxDatalen > static_cast<int> (dv.size ()))
            {
                memcpy (data, dv.data (), dv.size ());
                if (actualSize != nullptr)
                {
                    *actualSize = static_cast<int> (dv.size ());
                }
                return;
            }
            memcpy (data, dv.data (), maxDatalen);
            if (actualSize != nullptr)
            {
                *actualSize = maxDatalen;
            }
            return;
        }

        auto str = inpObj->inputPtr->getValue<std::string> ();
        if (maxDatalen > static_cast<int> (str.size ()))
        {
            memcpy (data, str.data (), static_cast<int> (str.size ()));
            if (actualSize != nullptr)
            {
                *actualSize = static_cast<int> (str.size ());
            }
        }
        else
        {
            memcpy (data, str.data (), maxDatalen);
            if (actualSize != nullptr)
            {
                *actualSize = maxDatalen;
            }
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsInputGetString (helics_input inp, char *outputString, int maxlen, int *actualLength, helics_error *err)
{
    if (actualLength != nullptr)
    {  // for initialization
        *actualLength = 0;
    }
    auto inpObj = verifyInput (inp, err);
    if (inpObj == nullptr)
    {
        return;
    }

    if (!checkOutArgString (outputString, maxlen, err))
    {
        return;
    }
    try
    {
        if (inpObj->rawOnly)
        {
            int length;

            helicsInputGetRawValue (inp, outputString, maxlen, &length, err);
            // make sure we have a null terminator
            if (length == maxlen)
            {
                outputString[maxlen - 1] = '\0';
                if (actualLength != nullptr)
                {  // for initialization
                    *actualLength = maxlen;
                }
            }
            else
            {
                outputString[length] = '\0';
                if (actualLength != nullptr)
                {
                    *actualLength = length;
                }
            }
        }
        else
        {
            int length = inpObj->inputPtr->getValue (outputString, maxlen);
            if (actualLength != nullptr)
            {  // for initialization
                *actualLength = length;
            }
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
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
        // no errors here the caller just didn't want any values for some reason
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

helics_complex helicsInputGetComplexObject (helics_input inp, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);

    if (inpObj == nullptr)
    {
        // time invalid is just an invalid double
        return {helics_time_invalid, 0.0};
    }

    try
    {
        if (inpObj->rawOnly)
        {
            auto cval = inpObj->fedptr->getValue<std::complex<double>> (inpObj->id);
            return {cval.real (), cval.imag ()};
        }
        else
        {
            auto cval = inpObj->inputPtr->getValue<std::complex<double>> ();
            return {cval.real (), cval.imag ()};
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
        return {helics_time_invalid, 0.0};
    }
}

int helicsInputGetVectorSize (helics_input inp)
{
    auto inpObj = verifyInput (inp, nullptr);
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
        return 0;
    }
}

int helicsInputGetStringSize (helics_input inp)
{
    auto inpObj = verifyInput (inp, nullptr);
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
        return 0;
    }
}

void helicsInputGetVector (helics_input inp, double data[], int maxlen, int *actualSize, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (actualSize != nullptr)
    {
        *actualSize = 0;
    }
    if (inpObj == nullptr)
    {
        return;
    }
    if ((data == nullptr) || (maxlen <= 0))
    {
        return;
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
        }
        else
        {
            int length = inpObj->inputPtr->getValue (data, maxlen);
            if (actualSize != nullptr)
            {
                *actualSize = length;
            }
            std::cout<<"vector outputC ("<<length<<")[";
	for (int ii=0;ii<length;++ii)
	{

        std::cout<<data[ii]<<',';
	}
	
  std::cout<<"]"<<std::endl;
        }
    }
    catch (...)
    {
        helicsErrorHandler (err);
    }
}

void helicsInputGetNamedPoint (helics_input inp, char *outputString, int maxlen, int *actualLength, double *val, helics_error *err)
{
    auto inpObj = verifyInput (inp, err);
    if (actualLength != nullptr)
    {
        *actualLength = 0;
    }
    if (inpObj == nullptr)
    {
        return;
    }
    if (!checkOutArgString (outputString, maxlen, err))
    {
        return;
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
            if (actualLength != nullptr)
            {
                *actualLength = maxlen;
            }
        }
        else
        {
            outputString[length] = '\0';
            if (actualLength != nullptr)
            {
                *actualLength = length+1;
            }
        }

        if (val != nullptr)
        {
            *val = np.value;
        }
        
        return;
    }
    catch (...)
    {
        helicsErrorHandler (err);
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

const char *helicsInputGetType (helics_input inp)
{
    auto inpObj = verifyInput (inp, nullptr);
    if (inpObj == nullptr)
    {
        return nullStr.c_str ();
    }

    try
    {
        if (inpObj->rawOnly)
        {
            const std::string &type = inpObj->fedptr->getInputType (inpObj->id);
            return type.c_str ();
        }
        else
        {
            const std::string &type = inpObj->inputPtr->getType ();
            return type.c_str ();
        }
    }
    catch (...)
    {
        return nullStr.c_str ();
    }
}

const char *helicsPublicationGetType (helics_publication pub)
{
    auto pubObj = verifyPublication (pub, nullptr);
    if (pubObj == nullptr)
    {
        return nullStr.c_str ();
    }

    try
    {
        if (pubObj->rawOnly)
        {
            const std::string &type = pubObj->fedptr->getPublicationType (pubObj->id);
            return type.c_str ();
        }
        else
        {
            const std::string &type = pubObj->pubptr->getType ();
            return type.c_str ();
        }
    }
    catch (...)
    {
        return nullStr.c_str ();
    }
}

const char *helicsInputGetKey (helics_input inp)
{
    auto inpObj = verifyInput (inp, nullptr);
    if (inpObj == nullptr)
    {
        return nullStr.c_str ();
    }

    try
    {
        if (inpObj->rawOnly)
        {
            const std::string &key = inpObj->fedptr->getInputKey (inpObj->id);
            return key.c_str ();
        }
        else
        {
            const std::string &key = inpObj->inputPtr->getName ();
            return key.c_str ();
        }
    }
    catch (...)
    {
        return nullStr.c_str ();
    }
}

const char *helicsSubscriptionGetKey (helics_input sub)
{
    auto inpObj = verifyInput (sub, nullptr);
    if (inpObj == nullptr)
    {
        return nullStr.c_str ();
    }

    try
    {
        if (inpObj->rawOnly)
        {
            return nullStr.c_str ();
        }
        else
        {
            auto subPtr = dynamic_cast<helics::Subscription *> (inpObj->inputPtr.get ());
            if (subPtr != nullptr)
            {
                const std::string &key = subPtr->getTarget ();
                return key.c_str ();
            }
            
            return nullStr.c_str ();
        }
    }
    catch (...)
    {
        return nullStr.c_str ();
    }
}

const char *helicsPublicationGetKey (helics_publication pub)
{
    auto pubObj = verifyPublication (pub, nullptr);
    if (pubObj == nullptr)
    {
        return nullStr.c_str ();
    }
    try
    {
        if (pubObj->rawOnly)
        {
            const std::string &key = pubObj->fedptr->getPublicationKey (pubObj->id);
            return key.c_str ();
        }
        else
        {
            const std::string &key = pubObj->pubptr->getKey ();
            return key.c_str ();
        }
    }
    catch (...)
    {
        return nullStr.c_str ();
    }
}

const char *helicsInputGetUnits (helics_input inp)
{
    auto inpObj = verifyInput (inp, nullptr);
    if (inpObj == nullptr)
    {
        return nullStr.c_str ();
    }
    try
    {
        if (inpObj->rawOnly)
        {
            const std::string &units = inpObj->fedptr->getInputUnits (inpObj->id);
            return units.c_str ();
        }
        else
        {
            const std::string &units = inpObj->inputPtr->getUnits ();
            return units.c_str ();
        }
    }
    catch (...)
    {
        return nullStr.c_str ();
    }
}

const char *helicsPublicationGetUnits (helics_publication pub)
{
    auto pubObj = verifyPublication (pub, nullptr);
    if (pubObj == nullptr)
    {
        return nullStr.c_str ();
    }
    try
    {
        if (pubObj->rawOnly)
        {
            const std::string &units = pubObj->fedptr->getPublicationUnits (pubObj->id);
            return units.c_str ();
        }
        else
        {
            const std::string &units = pubObj->pubptr->getUnits ();
            return units.c_str ();
        }
    }
    catch (...)
    {
        return nullStr.c_str ();
    }
}

helics_bool_t helicsInputIsUpdated (helics_input inp)
{
    auto inpObj = verifyInput (inp, nullptr);
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

helics_time_t helicsInputLastUpdateTime (helics_input inp)
{
    auto inpObj = verifyInput (inp, nullptr);
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
        return helics_time_invalid;
    }
}

int helicsFederateGetPublicationCount (helics_federate fed)
{
    // this call should be with a nullptr since it can fail and still be a successful call
    auto vfedObj = getValueFed (fed, nullptr);
    if (vfedObj == nullptr)
    {
        return 0;
    }
    return static_cast<int> (vfedObj->getPublicationCount ());
}

int helicsFederateGetInputCount (helics_federate fed)
{
    // this call should be with a nullptr since it can fail and still be a successful call
    auto vfedObj = getValueFed (fed, nullptr);
    if (vfedObj == nullptr)
    {
        return 0;
    }
    return static_cast<int> (vfedObj->getInputCount ());
}
