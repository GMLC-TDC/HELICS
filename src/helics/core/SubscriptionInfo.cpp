/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "SubscriptionInfo.h"

#include <algorithm>

namespace helics
{
std::shared_ptr<const data_block> SubscriptionInfo::getData () { return current_data; }

void SubscriptionInfo::addData (Time valueTime, std::shared_ptr<const data_block> data)
{
    if (data_queue.empty ())
    {
        data_queue.emplace_back (valueTime, std::move (data));
    }
    else
    {
        auto m = std::upper_bound (data_queue.begin (), data_queue.end (), valueTime,
                                   [](auto &time, auto &tm) { return (time < tm.first); });
        data_queue.emplace (m, valueTime, std::move (data));
    }
}

bool SubscriptionInfo::updateTime (Time newTime)
{
    auto currentValue = data_queue.begin ();
    auto last = currentValue;
    auto it_final = data_queue.end ();
    if (currentValue == it_final)
    {
        return false;
    }
    while ((currentValue != it_final) && (currentValue->first <= newTime))
    {
        last = currentValue;
        ++currentValue;
    }
    if (currentValue != last)
    {
        current_data = std::move (last->second);
        data_queue.erase (data_queue.begin (), currentValue);
        return true;
    }
    return false;
}

Time SubscriptionInfo::nextValueTime () const
{
    if (data_queue.empty ())
    {
        return Time::maxVal ();
    }
    return data_queue.front ().first;
}
} //namespace helics
