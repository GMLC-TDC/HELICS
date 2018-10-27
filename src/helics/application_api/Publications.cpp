/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "../core/core-exceptions.hpp"
#include "Publications.hpp"

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

data_block typeConvert (helics_type_t type, const defV &val)
{
    switch (val.index ())
    {
    case doubleLoc:  // double
        return typeConvert (type, mpark::get<double> (val));
    case intLoc:  // int64_t
        return typeConvert (type, mpark::get<int64_t> (val));
    case stringLoc:  // string
    default:
        return typeConvert (type, mpark::get<std::string> (val));
    case complexLoc:  // complex
        return typeConvert (type, mpark::get<std::complex<double>> (val));
    case vectorLoc:  // vector
        return typeConvert (type, mpark::get<std::vector<double>> (val));
    case complexVectorLoc:  // complex
        return typeConvert (type, mpark::get<std::vector<std::complex<double>>> (val));
    case namedPointLoc:
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
