/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "../core/core-exceptions.hpp"
#include "Inputs.hpp"

namespace helics
{
Input::Input (ValueFederate *valueFed,
              const std::string &key,
              const std::string &defType,
              const std::string &units)
{
    auto &inp = valueFed->getInput (key);
    if (inp.isValid ())
    {
        operator= (inp);
    }
    else
    {
        operator= (valueFed->registerInput (key, defType, units));
    }
}

Input::Input (interface_visibility locality,
              ValueFederate *valueFed,
              const std::string &key,
              const std::string &defType,
              const std::string &units)
{
    try
    {
        if (locality == interface_visibility::global)
        {
            operator= (valueFed->registerGlobalInput (key, defType, units));
        }
        else
        {
            operator= (valueFed->registerInput (key, defType, units));
        }
    }
    catch (const RegistrationFailure &e)
    {
        operator= (valueFed->getInput (key));
        if (!isValid ())
        {
            throw (e);
        }
    }
}

void Input::handleCallback (Time time)
{
    if (!isUpdated ())
    {
        return;
    }
    switch (value_callback.index ())
    {
    case doubleLoc:
    {
        auto val = getValue<double> ();
        mpark::get<std::function<void(const double &, Time)>> (value_callback) (val, time);
    }
    break;
    case intLoc:
    {
        auto val = getValue<int64_t> ();
        mpark::get<std::function<void(const int64_t &, Time)>> (value_callback) (val, time);
    }
    break;
    case stringLoc:
    default:
    {
        auto val = getValue<std::string> ();
        mpark::get<std::function<void(const std::string &, Time)>> (value_callback) (val, time);
    }
    break;
    case complexLoc:
    {
        auto val = getValue<std::complex<double>> ();
        mpark::get<std::function<void(const std::complex<double> &, Time)>> (value_callback) (val, time);
    }
    break;
    case vectorLoc:
    {
        auto val = getValue<std::vector<double>> ();
        mpark::get<std::function<void(const std::vector<double> &, Time)>> (value_callback) (val, time);
    }
    break;
    case complexVectorLoc:
    {
        auto val = getValue<std::vector<std::complex<double>>> ();
        mpark::get<std::function<void(const std::vector<std::complex<double>> &, Time)>> (value_callback) (val,
                                                                                                           time);
    }
    break;
    case namedPointLoc:
    {
        auto val = getValue<named_point> ();
        mpark::get<std::function<void(const named_point &, Time)>> (value_callback) (val, time);
    }
    break;
    case 7:  // bool loc
    {
        auto val = getValue<bool> ();
        mpark::get<std::function<void(const bool &, Time)>> (value_callback) (val, time);
    }
    break;
    case 8:  // Time loc
    {
        auto val = getValue<Time> ();
        mpark::get<std::function<void(const Time &, Time)>> (value_callback) (val, time);
    }
    break;
    }
}

bool Input::isUpdated ()
{
    if (hasUpdate)
    {
        return true;
    }
    if (changeDetectionEnabled)
    {
        if (fed->isUpdated (*this))
        {
            auto dv = fed->getValueRaw (*this);
            if (type == data_type::helicsUnknown)
            {
                type = getTypeFromString (fed->getInjectionType (*this));
            }
            auto visitor = [&, this](auto &&arg) {
                std::remove_reference_t<decltype (arg)> newVal;
                (void)arg;  // suppress VS2015 warning
                valueExtract (dv, type, newVal);
                if (changeDetected (lastValue, newVal, delta))
                {
                    lastValue = newVal;
                    hasUpdate = true;
                }
            };
            mpark::visit (visitor, lastValue);
        }
    }
    else
    {
        hasUpdate = fed->isUpdated (*this);
    }
    return hasUpdate;
}

size_t Input::getRawSize ()
{
    isUpdated ();
    auto dv = fed->getValueRaw (*this);
    if (dv.empty ())
    {
        auto &out = getValueRef<std::string> ();
        return out.size ();
    }
    return dv.size ();
}

data_view Input::getRawValue ()
{
    hasUpdate = false;
    return fed->getValueRaw (*this);
}

size_t Input::getStringSize ()
{
    isUpdated ();
    if (hasUpdate && !changeDetectionEnabled)
    {
        if (lastValue.index () == namedPointLoc)
        {
            auto &np = getValueRef<named_point> ();
            if (np.name.empty ())
            {
                return 30;  //"#invalid" string +20
            }
            else
            {
                //+20 is just in case the converted string is actually being requested in which case the +20 is for
                // the string representation of a double
                return np.name.size () + 20;
            }
        }
        else
        {
            auto &out = getValueRef<std::string> ();
            return out.size ();
        }
    }

    if (lastValue.index () == stringLoc)
    {
        return mpark::get<std::string> (lastValue).size ();
    }
    else if (lastValue.index () == namedPointLoc)
    {
        const auto &np = mpark::get<named_point> (lastValue);

        if (np.name.empty ())
        {
            return 30;  //"~length of #invalid" string +20
        }
        else
        {
            //+20 is just in case the converted string is actually being requested in which case it the 20 accounts
            // for the string representation of a double
            return np.name.size () + 20;
        }
    }
    auto &out = getValueRef<std::string> ();
    return out.size ();
}

size_t Input::getVectorSize ()
{
    isUpdated ();
    if (hasUpdate && !changeDetectionEnabled)
    {
        auto &out = getValueRef<std::vector<double>> ();
        return out.size ();
    }
    switch (lastValue.index ())
    {
    case doubleLoc:
    case intLoc:
        return 1;
    case complexLoc:
        return 2;
    case vectorLoc:
        return mpark::get<std::vector<double>> (lastValue).size ();
    case complexVectorLoc:
        return mpark::get<std::vector<std::complex<double>>> (lastValue).size () * 2;
    default:
        break;
    }
    auto &out = getValueRef<std::vector<double>> ();
    return out.size ();
}

char Input::getValueChar ()
{
    if (fed->isUpdated (*this) || (hasUpdate && !changeDetectionEnabled))
    {
        auto dv = fed->getValueRaw (*this);
        if (type == data_type::helicsUnknown)
        {
            type = getTypeFromString (fed->getInjectionType (*this));
        }

        if ((type == data_type::helicsString) || (type == data_type::helicsAny) ||
            (type == data_type::helicsCustom))
        {
            std::string out;
            valueExtract (dv, type, out);
            if (changeDetectionEnabled)
            {
                if (changeDetected (lastValue, out, delta))
                {
                    lastValue = out;
                }
            }
            else
            {
                lastValue = out;
            }
        }
        else
        {
            int64_t out;
            valueExtract (dv, type, out);
            if (changeDetectionEnabled)
            {
                if (changeDetected (lastValue, out, delta))
                {
                    lastValue = out;
                }
            }
            else
            {
                lastValue = out;
            }
        }
    }
    char V;
    valueExtract (lastValue, V);
    hasUpdate = false;
    return V;
}

int Input::getValue (double *data, int maxsize)
{
    auto V = getValueRef<std::vector<double>> ();
    int length = std::min (static_cast<int> (V.size ()), maxsize);
    std::copy (V.data (), V.data () + length, data);
    return length;
}

int Input::getValue (char *str, int maxsize)
{
    auto &S = getValueRef<std::string> ();
    int length = std::min (static_cast<int> (S.size ()), maxsize);
    memcpy (str, S.data (), length);
    if (length == maxsize)
    {
        str[maxsize - 1] = '\0';
    }
    else
    {
        str[length] = '\0';
        ++length;
    }
    return length;
}

}  // namespace helics
