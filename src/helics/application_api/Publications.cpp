/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "Publications.hpp"
#include "../core/core-exceptions.hpp"

namespace helics
{
PublicationBase::PublicationBase(ValueFederate *valueFed,
    const std::string &key,
    const std::string &type,
    const std::string &units)
    : fed(valueFed), key_(key), type_(type), units_(units)
{
    try
    {
        id = fed->registerPublication(key_, type_, units_);
    }
    catch (const RegistrationFailure &)
    {
        id = fed->getPublicationId(key_);
        loadFromId();
    }
}

PublicationBase::PublicationBase(interface_visibility locality,
    ValueFederate *valueFed,
    const std::string &key,
    const std::string &type,
    const std::string &units)
    : fed(valueFed), key_(key), type_(type), units_(units)
{
    try
    {
        if (locality == GLOBAL)
        {
            id = fed->registerGlobalPublication(key, type, units);
        }
        else
        {
            id = fed->registerPublication(key, type, units);
        }
    }
    catch (const RegistrationFailure &)
    {
        id = fed->getPublicationId(key_);
        loadFromId();
    }
}

PublicationBase::PublicationBase (ValueFederate *valueFed, int pubIndex) : fed (valueFed)
{
    auto cnt = fed->getPublicationCount ();
    if ((pubIndex >= cnt) || (cnt < 0))
    {
        throw (helics::InvalidParameter ("no publication with the specified index"));
    }
    id = static_cast<publication_id_t> (pubIndex);
    loadFromId();
}

void PublicationBase::loadFromId()
{
    key_ = fed->getPublicationKey(id);

    type_ = fed->getPublicationType(id);
    units_ = fed->getPublicationUnits(id);
}

void Publication::publish (double val) const
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
        fed->publish (id, db);
    }
}
void Publication::publish (int64_t val) const
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
        fed->publish (id, db);
    }
}
void Publication::publish (bool val) const
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
        fed->publish (id, db);
    }
}

void Publication::publish (const char *val) const
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
        fed->publish (id, db);
    }
}
void Publication::publish (const std::string &val) const
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
        fed->publish (id, db);
    }
}
void Publication::publish (const std::vector<double> &val) const
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
        fed->publish (id, db);
    }
}

void Publication::publish (const std::vector<std::complex<double>> &val) const
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
        fed->publish (id, db);
    }
}

void Publication::publish (const double *vals, int size) const
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
        fed->publish (id, db);
    }
}

void Publication::publish (std::complex<double> val) const
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
        fed->publish (id, db);
    }
}

void Publication::publish(const named_point &np) const
{
    bool doPublish = true;
    if (changeDetectionEnabled)
    {
        if (changeDetected(prevValue, np, delta))
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
        auto db = typeConvert(pubType,np);
        fed->publish(id, db);
    }
}

void Publication::publish(const std::string &name,double val) const
{
    bool doPublish = true;
    if (changeDetectionEnabled)
    {
        named_point np(name, val);
        if (changeDetected(prevValue, np, delta))
        {
            prevValue = std::move(np);
        }
        else
        {
            doPublish = false;
        }
    }
    if (doPublish)
    {
        auto db = typeConvert(pubType, name,val);
        fed->publish(id, db);
    }
}

void Publication::publish(const char *name, double val) const
{
    bool doPublish = true;
    if (changeDetectionEnabled)
    {
        named_point np(name, val);
        if (changeDetected(prevValue, np, delta))
        {
            prevValue = std::move(np);
        }
        else
        {
            doPublish = false;
        }
    }
    if (doPublish)
    {
        auto db = typeConvert(pubType, name,val);
        fed->publish(id, db);
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
        return typeConvert(type, mpark::get<named_point>(val));
    }
}

void Publication::publish (const defV &val) const
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
        fed->publish (id, db);
    }
}
}  // namespace helics
