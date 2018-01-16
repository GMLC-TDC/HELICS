/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "Subscriptions.hpp"
#include "../core/core-exceptions.h"

namespace helics
{
SubscriptionBase::SubscriptionBase (ValueFederate *valueFed, int subIndex) : fed (valueFed)
{
    auto cnt = fed->getSubscriptionCount ();
    if ((subIndex >= cnt) || (cnt < 0))
    {
        throw (helics::InvalidParameter ("no subscription with the specified index"));
    }
    id = static_cast<subscription_id_t> (subIndex);
    key_ = fed->getSubscriptionKey (id);

    type_ = fed->getSubscriptionType (id);
    units_ = fed->getSubscriptionUnits (id);
}

void Subscription::handleCallback (Time time)
{
    auto dv = fed->getValueRaw (id);
    if (type == helics_type_t::helicsInvalid)
    {
        type = getTypeFromString (fed->getPublicationType (id));
    }
    switch (value_callback.which ())
    {
    case doubleLoc:
    {
        double val;
        valueExtract (dv, type, val);
        boost::get<std::function<void(const double &, Time)>> (value_callback) (val, time);
    }
    break;
    case intLoc:
    {
        int64_t val;
        valueExtract (dv, type, val);
        boost::get<std::function<void(const int64_t &, Time)>> (value_callback) (val, time);
    }
    break;
    case stringLoc:
    default:
    {
        std::string val;
        valueExtract (dv, type, val);
        boost::get<std::function<void(const std::string &, Time)>> (value_callback) (val, time);
    }
    break;
    case complexLoc:
    {
        std::complex<double> val;
        valueExtract (dv, type, val);
        boost::get<std::function<void(const std::complex<double> &, Time)>> (value_callback) (val, time);
    }
    break;
    case vectorLoc:
    {
        std::vector<double> val;
        valueExtract (dv, type, val);
        boost::get<std::function<void(const std::vector<double> &, Time)>> (value_callback) (val, time);
    }
    break;
    case complexVectorLoc:
    {
        std::vector<std::complex<double>> val;
        valueExtract (dv, type, val);
        boost::get<std::function<void(const std::vector<std::complex<double>> &, Time)>> (value_callback) (val,
                                                                                                           time);
    }
    break;
    }
}

bool Subscription::isUpdated () const
{
    if (changeDetectionEnabled)
    {
        return SubscriptionBase::isUpdated ();
    }
    else
    {
        return SubscriptionBase::isUpdated ();
    }
}
}  // namespace helics