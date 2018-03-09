/*
Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "Subscriptions.hpp"
#include "../core/core-exceptions.hpp"

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

