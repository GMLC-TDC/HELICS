/*

Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "SubscriptionInfo.hpp"

#include <algorithm>

namespace helics
{
std::shared_ptr<const data_block> SubscriptionInfo::getData () { return current_data.data; }

auto recordComparison = [](const SubscriptionInfo::dataRecord &rec1, const SubscriptionInfo::dataRecord &rec2) { return (rec1.time < rec2.time) ? true : ((rec1.time == rec2.time) ? (rec1.index < rec2.index) : false); };

void SubscriptionInfo::addData (Time valueTime, unsigned int index, std::shared_ptr<const data_block> data)
{
    if (data_queue.empty ())
    {
        data_queue.emplace_back (valueTime, index, std::move (data));
    }
    else
    {
        dataRecord newRecord(valueTime, index, std::move(data));
        auto m = std::upper_bound(data_queue.begin(), data_queue.end(), newRecord, recordComparison);
        data_queue.insert (m, std::move(newRecord));
    }
}

bool SubscriptionInfo::updateTimeUpTo (Time newTime)
{
    auto currentValue = data_queue.begin ();
    auto last = currentValue;
    auto it_final = data_queue.end ();
    if (currentValue == it_final)
    {
        return false;
    }
        while ((currentValue != it_final) && (currentValue->time < newTime))
        {
            last = currentValue;
            ++currentValue;
        }
        if (last != currentValue)
        {
            auto res = updateData(std::move(*last));
            data_queue.erase(data_queue.begin(), currentValue);
            return res;
    }
        return false;
        

}

bool SubscriptionInfo::updateTimeNextIteration(Time newTime)
{
    auto currentValue = data_queue.begin();
    auto last = currentValue;
    auto it_final = data_queue.end();
    if (currentValue == it_final)
    {
        return false;
    }
        while ((currentValue != it_final) && (currentValue->time < newTime))
        {
            last = currentValue;
            ++currentValue;
        }
        if (currentValue != it_final)
        {
            if (currentValue->time == newTime)
            {
                auto cindex = currentValue->index;
                last = currentValue;
                ++currentValue;
                while ((currentValue != it_final) && (currentValue->time == newTime) && (currentValue->index == cindex))
                {
                    last = currentValue;
                    ++currentValue;
                }
            }
        }
        if (last != currentValue)
        {
            auto res = updateData(std::move(*last));
            data_queue.erase(data_queue.begin(), currentValue);
            return res;
        }
        return false;

}

bool SubscriptionInfo::updateTimeInclusive(Time newTime)
{
    auto currentValue = data_queue.begin();
    auto last = currentValue;
    auto it_final = data_queue.end();
    if (currentValue == it_final)
    {
        return false;
    }
    while ((currentValue != it_final) && (currentValue->time <= newTime))
    {
        last = currentValue;
        ++currentValue;
    }
    if (last != currentValue)
    {
        auto res = updateData(std::move(*last));
        data_queue.erase(data_queue.begin(), currentValue);
        return res;
    }
    return false;
}

bool SubscriptionInfo::updateData(dataRecord &&update)
{
    if (!only_update_on_change)
    {
        current_data = std::move(update);
        return true;
    }

    if (*current_data.data != *(update.data))
    {
        current_data = std::move(update);
        return true;
    }
    else if (current_data.time == update.time)
    { //this is for bookkeeping purposes should still return false
        current_data.index = update.index;
    }
    return false;
}

Time SubscriptionInfo::nextValueTime () const
{
    if (data_queue.empty ())
    {
        return Time::maxVal ();
    }
    return data_queue.front ().time;
}
}  // namespace helics

