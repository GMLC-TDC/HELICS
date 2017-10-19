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
        if (changeDetected (val))
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
        if (changeDetected (val))
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
        if (changeDetected (val))
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
        if (changeDetected (val))
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
        if (changeDetected (val))
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
        if (changeDetected (val))
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
        if (changeDetected (vals, size))
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
        if (changeDetected (val))
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

bool Publication::changeDetected (const std::string &val) const
{
    if (prevValue.which () == stringLoc)
    {
        if (val == boost::get<std::string> (prevValue))
        {
            return false;
        }
    }
    return true;
}

bool Publication::changeDetected (const std::vector<double> &val) const
{
    if (prevValue.which () == vectorLoc)
    {
        const auto &prevV = boost::get<std::vector<double>> (prevValue);
        if (val.size () == prevV.size ())
        {
            for (size_t ii = 0; ii < val.size (); ++ii)
            {
                if (std::abs (prevV[ii] - val[ii]) > delta)
                {
                    return true;
                }
            }
            return false;
        }
    }
    return true;
}

bool Publication::changeDetected (const std::vector<std::complex<double>> &val) const
{
    if (prevValue.which () == complexVectorLoc)
    {
        const auto &prevV = boost::get<std::vector<std::complex<double>>> (prevValue);
        if (val.size () == prevV.size ())
        {
            for (size_t ii = 0; ii < val.size (); ++ii)
            {
                if (std::abs (prevV[ii] - val[ii]) > delta)
                {
                    return true;
                }
            }
            return false;
        }
    }
    return true;
}

bool Publication::changeDetected (const double *vals, size_t size) const
{
    if (prevValue.which () == vectorLoc)
    {
        const auto &prevV = boost::get<std::vector<double>> (prevValue);
        if (size == prevV.size ())
        {
            for (size_t ii = 0; ii < size; ++ii)
            {
                if (std::abs (prevV[ii] - vals[ii]) > delta)
                {
                    return true;
                }
            }
            return false;
        }
    }
    return true;
}

bool Publication::changeDetected (const std::complex<double> &val) const
{
    if (prevValue.which () == complexLoc)
    {
        const auto &prevV = boost::get<std::complex<double>> (prevValue);
        if (std::abs (prevV.real () - val.real ()) > delta)
        {
            return true;
        }
        if (std::abs (prevV.imag () - val.imag ()) > delta)
        {
            return true;
        }
        return false;
    }
    return true;
}
bool Publication::changeDetected (double val) const
{
    if (prevValue.which () == doubleLoc)
    {
        if (std::abs (boost::get<double> (prevValue) - val) <= delta)
        {
            return false;
        }
    }
    return true;
}
bool Publication::changeDetected (int64_t val) const
{
    if (prevValue.which () == intLoc)
    {
        if (static_cast<double> (std::abs (boost::get<int64_t> (prevValue) - val)) <= delta)
        {
            return false;
        }
    }
    return true;
}

}  // namespace helics
