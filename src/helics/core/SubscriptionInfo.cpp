/*

Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "SubscriptionInfo.hpp"

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
        if (!only_update_on_change)
        {
            current_data = std::move (last->second);
            data_queue.erase (data_queue.begin (), currentValue);
            return true;
        }

        if (*current_data != *(last->second))
        {
            current_data = std::move (last->second);
            data_queue.erase (data_queue.begin (), currentValue);
            return true;
        }

        data_queue.erase (data_queue.begin (), currentValue);
        return false;
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
}  // namespace helics

