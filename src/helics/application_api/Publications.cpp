/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "Publications.hpp"
#include "../core/core-exceptions.hpp"

namespace helics
{
PublicationBase::PublicationBase (ValueFederate *valueFed, int pubIndex) : fed (valueFed)
{
    auto cnt = fed->getPublicationCount ();
    if ((pubIndex >= cnt) || (cnt < 0))
    {
        throw (helics::InvalidParameter ("no subscription with the specified index"));
    }
    id = static_cast<publication_id_t> (pubIndex);
    key_ = fed->getPublicationKey (id);

    type_ = fed->getPublicationType (id);
    units_ = fed->getPublicationUnits (id);
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

data_block typeConvert (helics_type_t type, const defV &val)
{
    switch (val.which ())
    {
    case doubleLoc:  // double
        return typeConvert (type, boost::get<double> (val));
    case intLoc:  // int64_t
        return typeConvert (type, boost::get<int64_t> (val));
    case stringLoc:  // string
    default:
        return typeConvert (type, boost::get<std::string> (val));
    case complexLoc:  // complex
        return typeConvert (type, boost::get<std::complex<double>> (val));
    case vectorLoc:  // vector
        return typeConvert (type, boost::get<std::vector<double>> (val));
    case complexVectorLoc:  // complex
        return typeConvert (type, boost::get<std::vector<std::complex<double>>> (val));
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

