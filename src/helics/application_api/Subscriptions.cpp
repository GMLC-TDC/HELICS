/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "Subscriptions.hpp"

namespace helics
{
void Subscription::handleCallback (Time time)
{
    auto dv = fed->getValueRaw (id);
    if (type == helicsType_t::helicsInvalid)
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

bool Subscription::isUpdated() const
{
	if (changeDetectionEnabled)
	{
		return SubscriptionBase::isUpdated();
	}
	else
	{
		return SubscriptionBase::isUpdated();
	}
}
}  // namespace helics