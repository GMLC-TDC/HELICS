/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "Publications.hpp"
#include "../core/core-exceptions.hpp"

namespace helics
{
Publication::Publication (ValueFederate *valueFed,
                          interface_handle id,
                          const std::string &key,
                          const std::string &type,
                          const std::string &units)
    : fed (valueFed), handle (id), key_ (key), units_ (units)
{
    pubType = getTypeFromString (type);
}

Publication::Publication (ValueFederate *valueFed,
                          const std::string &key,
                          const std::string &type,
                          const std::string &units)
{
    auto &pub = valueFed->getPublication (key);
    if (pub.isValid ())
    {
        operator= (pub);
    }
    else
    {
        operator= (valueFed->registerPublication (key, type, units));
    }
}

Publication::Publication (interface_visibility locality,
                          ValueFederate *valueFed,
                          const std::string &key,
                          const std::string &type,
                          const std::string &units)
{
    try
    {
        if (locality == interface_visibility::global)
        {
            operator= (valueFed->registerGlobalPublication (key, type, units));
        }
        else
        {
            operator= (valueFed->registerPublication (key, type, units));
        }
    }
    catch (const RegistrationFailure &e)
    {
        operator= (valueFed->getPublication (key));
        if (!isValid ())
        {
            throw (e);
        }
    }
}

void Publication::publish (double val)
{
    bool doPublish = true;
    if (changeDetectionEnabled)
    {
        if (changeDetected (prevValue, val, delta))
        {
            prevValue = val;
        }
        else
        {
            doPublish = false;
        }
    }
    if (doPublish)
    {
        auto db = typeConvert (pubType, val);
        fed->publishRaw (*this, db);
    }
}
void Publication::publishInt (int64_t val)
{
    bool doPublish = true;
    if (changeDetectionEnabled)
    {
        if (changeDetected (prevValue, val, static_cast<int64_t> (delta)))
        {
            prevValue = val;
        }
        else
        {
            doPublish = false;
        }
    }
    if (doPublish)
    {
        auto db = typeConvert (pubType, val);
        fed->publishRaw (*this, db);
    }
}

void Publication::publish (char val)
{
    switch (pubType)
    {
    case data_type::helicsBool:
        publish (!((val == '0') || (val == 'f') || (val == 0) || (val == 'F') || (val == '-')));
        break;
    case data_type::helicsString:
    case data_type::helicsNamedPoint:
        publish (std::string (1, val));
        break;
    default:
        publishInt (static_cast<int64_t> (val));
    }
}

void Publication::publish (Time val)
{
    bool doPublish = true;
    if (changeDetectionEnabled)
    {
        if (changeDetected (prevValue, val, delta))
        {
            prevValue = val.getBaseTimeCode ();
        }
        else
        {
            doPublish = false;
        }
    }
    if (doPublish)
    {
        auto db = typeConvert (pubType, val.getBaseTimeCode ());
        fed->publishRaw (*this, db);
    }
}

void Publication::publish (bool val)
{
    bool doPublish = true;
    std::string bstring = val ? "1" : "0";
    if (changeDetectionEnabled)
    {
        if (changeDetected (prevValue, bstring, delta))
        {
            prevValue = bstring;
        }
        else
        {
            doPublish = false;
        }
    }
    if (doPublish)
    {
        auto db = typeConvert (pubType, bstring);
        fed->publishRaw (*this, db);
    }
}

void Publication::publish (const char *val)
{
    bool doPublish = true;
    if (changeDetectionEnabled)
    {
        if (changeDetected (prevValue, val, delta))
        {
            prevValue = val;
        }
        else
        {
            doPublish = false;
        }
    }
    if (doPublish)
    {
        auto db = typeConvert (pubType, val);
        fed->publishRaw (*this, db);
    }
}
void Publication::publish (const std::string &val)
{
    bool doPublish = true;
    if (changeDetectionEnabled)
    {
        if (changeDetected (prevValue, val, delta))
        {
            prevValue = val;
        }
        else
        {
            doPublish = false;
        }
    }
    if (doPublish)
    {
        auto db = typeConvert (pubType, val);
        fed->publishRaw (*this, db);
    }
}
void Publication::publish (const std::vector<double> &val)
{
    bool doPublish = true;
    if (changeDetectionEnabled)
    {
        if (changeDetected (prevValue, val, delta))
        {
            prevValue = val;
        }
        else
        {
            doPublish = false;
        }
    }
    if (doPublish)
    {
        auto db = typeConvert (pubType, val);
        fed->publishRaw (*this, db);
    }
}

void Publication::publish (const std::vector<std::complex<double>> &val)
{
    bool doPublish = true;
    if (changeDetectionEnabled)
    {
        if (changeDetected (prevValue, val, delta))
        {
            prevValue = val;
        }
        else
        {
            doPublish = false;
        }
    }
    if (doPublish)
    {
        auto db = typeConvert (pubType, val);
        fed->publishRaw (*this, db);
    }
}

void Publication::publish (const double *vals, int size)
{
    bool doPublish = true;
    if (changeDetectionEnabled)
    {
        if (changeDetected (prevValue, vals, size, delta))
        {
            prevValue = std::vector<double> (vals, vals + size);
        }
        else
        {
            doPublish = false;
        }
    }
    if (doPublish)
    {
        auto db = typeConvert (pubType, vals, size);
        fed->publishRaw (*this, db);
    }
}

void Publication::publish (std::complex<double> val)
{
    bool doPublish = true;
    if (changeDetectionEnabled)
    {
        if (changeDetected (prevValue, val, delta))
        {
            prevValue = val;
        }
        else
        {
            doPublish = false;
        }
    }
    if (doPublish)
    {
        auto db = typeConvert (pubType, val);
        fed->publishRaw (*this, db);
    }
}

void Publication::publish (const named_point &np)
{
    bool doPublish = true;
    if (changeDetectionEnabled)
    {
        if (changeDetected (prevValue, np, delta))
        {
            prevValue = np;
        }
        else
        {
            doPublish = false;
        }
    }
    if (doPublish)
    {
        auto db = typeConvert (pubType, np);
        fed->publishRaw (*this, db);
    }
}

void Publication::publish (const std::string &name, double val)
{
    bool doPublish = true;
    if (changeDetectionEnabled)
    {
        named_point np (name, val);
        if (changeDetected (prevValue, np, delta))
        {
            prevValue = std::move (np);
        }
        else
        {
            doPublish = false;
        }
    }
    if (doPublish)
    {
        auto db = typeConvert (pubType, name, val);
        fed->publishRaw (*this, db);
    }
}

void Publication::publish (const char *name, double val)
{
    bool doPublish = true;
    if (changeDetectionEnabled)
    {
        named_point np (name, val);
        if (changeDetected (prevValue, np, delta))
        {
            prevValue = std::move (np);
        }
        else
        {
            doPublish = false;
        }
    }
    if (doPublish)
    {
        auto db = typeConvert (pubType, name, val);
        fed->publishRaw (*this, db);
    }
}

data_block typeConvert (data_type type, const defV &val)
{
    switch (val.index ())
    {
    case double_loc:  // double
        return typeConvert (type, mpark::get<double> (val));
    case int_loc:  // int64_t
        return typeConvert (type, mpark::get<int64_t> (val));
    case string_loc:  // string
    default:
        return typeConvert (type, mpark::get<std::string> (val));
    case complex_loc:  // complex
        return typeConvert (type, mpark::get<std::complex<double>> (val));
    case vector_loc:  // vector
        return typeConvert (type, mpark::get<std::vector<double>> (val));
    case complex_vector_loc:  // complex
        return typeConvert (type, mpark::get<std::vector<std::complex<double>>> (val));
    case named_point_loc:
        return typeConvert (type, mpark::get<named_point> (val));
    }
}

void Publication::publish (const defV &val)
{
    bool doPublish = true;
    if (changeDetectionEnabled)
    {
        if (prevValue != val)
        {
            prevValue = val;
        }
        else
        {
            doPublish = false;
        }
    }
    if (doPublish)
    {
        auto db = typeConvert (pubType, val);
        fed->publishRaw (*this, db);
    }
}
}  // namespace helics
