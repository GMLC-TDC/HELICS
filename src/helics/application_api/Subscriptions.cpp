/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
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
    switch (value_callback.index ())
    {
    case doubleLoc:
    {
        double val;
        valueExtract (dv, type, val);
        mpark::get<std::function<void(const double &, Time)>> (value_callback) (val, time);
    }
    break;
    case intLoc:
    {
        int64_t val;
        valueExtract (dv, type, val);
        mpark::get<std::function<void(const int64_t &, Time)>> (value_callback) (val, time);
    }
    break;
    case stringLoc:
    default:
    {
        std::string val;
        valueExtract (dv, type, val);
        mpark::get<std::function<void(const std::string &, Time)>> (value_callback) (val, time);
    }
    break;
    case complexLoc:
    {
        std::complex<double> val;
        valueExtract (dv, type, val);
        mpark::get<std::function<void(const std::complex<double> &, Time)>> (value_callback) (val, time);
    }
    break;
    case vectorLoc:
    {
        std::vector<double> val;
        valueExtract (dv, type, val);
        mpark::get<std::function<void(const std::vector<double> &, Time)>> (value_callback) (val, time);
    }
    break;
    case complexVectorLoc:
    {
        std::vector<std::complex<double>> val;
        valueExtract (dv, type, val);
        mpark::get<std::function<void(const std::vector<std::complex<double>> &, Time)>> (value_callback) (val,
                                                                                                           time);
    }
    break;
    }
}

bool Subscription::isUpdated () const
{
    if (hasUpdate)
    {
        return true;
    }
    if (changeDetectionEnabled)
    {
        if (SubscriptionBase::isUpdated ())
        {
            if (type == helics_type_t::helicsInvalid)
            {
                return true;
            }
            auto dv = fed->getValueRaw(id);
            auto visitor = [&](auto &&arg) -> bool {
                std::remove_const_t<std::remove_reference_t<decltype (arg)>> newVal;
                (void)arg;  //suppress VS2015 warning
                valueExtract (dv, type, newVal);
                return (changeDetected(lastValue, newVal, delta));
            };
            return mpark::visit (visitor, lastValue);
        }
        return false;
    }
    else
    {
        return SubscriptionBase::isUpdated ();
    }
}

bool Subscription::getAndCheckForUpdate ()
{
    if (hasUpdate)
    {
        return true;
    }
    if (changeDetectionEnabled)
    {
        if (SubscriptionBase::isUpdated ())
        {
            auto dv = fed->getValueRaw (id);
            if (type == helics_type_t::helicsInvalid)
            {
                type = getTypeFromString (fed->getPublicationType (id));
            }
            if (type != helics_type_t::helicsInvalid)
            {
                auto visitor = [&](auto &&arg) {
                    std::remove_reference_t<decltype (arg)> newVal;
                    (void)arg;  //suppress VS2015 warning
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
    }
    else
    {
        hasUpdate = SubscriptionBase::isUpdated ();
    }

    return hasUpdate;
}

size_t Subscription::getRawSize () 
{ 
    getAndCheckForUpdate();
    auto dv=fed->getValueRaw(id);
    if (dv.empty())
    {
        auto out = getValue<std::string>();
        return out.size();
    }
    return dv.size();
}

size_t Subscription::getStringSize () 
{ 
    getAndCheckForUpdate();
    if (hasUpdate && !changeDetectionEnabled)
    {
        auto out = getValue<std::string>();
        return out.size();
    }

    if (lastValue.index() == stringLoc)
    {
        return mpark::get<std::string>(lastValue).size();
    }
    else if (lastValue.index() == namedPointLoc)
    {
        const auto &np= mpark::get<named_point>(lastValue);
        return np.name.size();
    }
    auto out = getValue<std::string>();
    return out.size();
}

size_t Subscription::getVectorSize () 
{ 
    getAndCheckForUpdate();
    if (hasUpdate && !changeDetectionEnabled)
    {
        auto out = getValue<std::vector<double>>();
        return out.size();
    }
    switch (lastValue.index())
    {
    case doubleLoc:
    case intLoc:
        return 1;
    case complexLoc:
        return 2;
    case vectorLoc:
        return mpark::get<std::vector<double>>(lastValue).size();
    case complexVectorLoc:
        return mpark::get<std::vector<std::complex<double>>>(lastValue).size() * 2;
    default:
        break;
    }
    auto out = getValue<std::vector<double>>();
    return out.size();
}

}  // namespace helics
