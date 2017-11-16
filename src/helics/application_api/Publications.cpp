/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "Publications.hpp"

namespace helics
{
void Publication::publish (double val) const
{
    bool doPublish = true;
    if (changeDetectionEnabled)
    {
        if (changeDetected (prevValue, val,delta))
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
        if (changeDetected (prevValue, val,static_cast<int64_t>(delta)))
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
        if (changeDetected (prevValue, val,delta))
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
        if (changeDetected (prevValue, val,delta))
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
        if (changeDetected (prevValue, val,delta))
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
        if (changeDetected (prevValue, val,delta))
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
        if (changeDetected (prevValue,vals, size,delta))
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
        if (changeDetected (prevValue,val,delta))
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
