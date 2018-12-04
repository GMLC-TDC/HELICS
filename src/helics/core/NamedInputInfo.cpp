/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "NamedInputInfo.hpp"

#include <algorithm>

namespace helics
{
std::vector<std::shared_ptr<const data_block>> NamedInputInfo::getAllData ()
{
    std::vector<std::shared_ptr<const data_block>> out;
    out.reserve (current_data.size ());
    for (auto &cd : current_data)
    {
        out.push_back (cd.data);
    }
    return out;
}

std::shared_ptr<const data_block> NamedInputInfo::getData(int index)
{
    if (isValidIndex(index, current_data))
    {
        return current_data[index].data;
    }
    return nullptr;
}

std::shared_ptr<const data_block> NamedInputInfo::getData()
{
    int ind = 0;
    int mxind = -1;
    Time mxTime = Time::minVal();
    for (auto &cd : current_data)
    {
        if (cd.time > mxTime)
        {
            mxTime = cd.time;
            mxind = ind;
        }
        ++ind;
    }
    if (mxind>=0)
    {
        return current_data[mxind].data;
    }
    return nullptr;
}

static auto recordComparison = [](const NamedInputInfo::dataRecord &rec1, const NamedInputInfo::dataRecord &rec2) {
    return (rec1.time < rec2.time) ? true : ((rec1.time == rec2.time) ? (rec1.iteration < rec2.iteration) : false);
};

void NamedInputInfo::addData (global_handle source_id,
                              Time valueTime,
                              unsigned int iteration,
                              std::shared_ptr<const data_block> data)
{
    int index;
    bool found = false;
    for (index = 0; index < static_cast<int> (input_sources.size ()); ++index)
    {
        if (input_sources[index] == source_id)
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        return;
    }
    if ((data_queues[index].empty ()) || (valueTime > data_queues[index].back ().time))
    {
        data_queues[index].emplace_back (valueTime, iteration, std::move (data));
    }
    else
    {
        dataRecord newRecord (valueTime, iteration, std::move (data));
        auto m =
          std::upper_bound (data_queues[index].begin (), data_queues[index].end (), newRecord, recordComparison);
        data_queues[index].insert (m, std::move (newRecord));
    }
}

void NamedInputInfo::addSource (global_handle newSource, const std::string &stype, const std::string &sunits) 
{ 
    input_sources.push_back (newSource);
    source_types.emplace_back (stype, sunits);
    data_queues.resize(input_sources.size());
    current_data.resize(input_sources.size());
}


void NamedInputInfo::removeSource(global_handle /*sourceToRemove*/)
{
	
}

bool NamedInputInfo::updateTimeUpTo (Time newTime)
{
    int index = 0;
    bool updated = false;
    for (auto &data_queue : data_queues)
    {
        auto currentValue = data_queue.begin ();

        auto it_final = data_queue.end ();
        if (currentValue == it_final)
        {
            return false;
        }
        if (currentValue->time > newTime)
        {
            return false;
        }
        auto last = currentValue;
        ++currentValue;
        while ((currentValue != it_final) && (currentValue->time < newTime))
        {
            last = currentValue;
            ++currentValue;
        }

        auto res = updateData (std::move (*last), index);
        data_queue.erase (data_queue.begin (), currentValue);
        ++index;
        if (res)
        {
            updated = true;
        }
    }
    return updated;
}

bool NamedInputInfo::updateTimeNextIteration (Time newTime)
{
    int index = 0;
    bool updated = false;
    for (auto &data_queue : data_queues)
    {
        auto currentValue = data_queue.begin ();
        auto it_final = data_queue.end ();
        if (currentValue == it_final)
        {
            return false;
        }
        if (currentValue->time > newTime)
        {
            return false;
        }
        auto last = currentValue;
        ++currentValue;
        while ((currentValue != it_final) && (currentValue->time < newTime))
        {
            last = currentValue;
            ++currentValue;
        }
        if (currentValue != it_final)
        {
            if (currentValue->time == newTime)
            {
                auto cindex = last->iteration;
                while ((currentValue != it_final) && (currentValue->time == newTime) &&
                       (currentValue->iteration == cindex))
                {
                    last = currentValue;
                    ++currentValue;
                }
            }
        }

        auto res = updateData (std::move (*last), index);
        data_queue.erase (data_queue.begin (), currentValue);
        ++index;
        if (res)
        {
            updated = true;
        }
    }
    return updated;
}

bool NamedInputInfo::updateTimeInclusive (Time newTime)
{
    int index = 0;
    bool updated = false;
    for (auto &data_queue : data_queues)
    {
        auto currentValue = data_queue.begin ();
        auto it_final = data_queue.end ();
        if (currentValue == it_final)
        {
            return false;
        }
        if (currentValue->time > newTime)
        {
            return false;
        }
        auto last = currentValue;
        ++currentValue;
        while ((currentValue != it_final) && (currentValue->time <= newTime))
        {
            last = currentValue;
            ++currentValue;
        }

        auto res = updateData (std::move (*last), index);
        data_queue.erase (data_queue.begin (), currentValue);
        ++index;
        if (res)
        {
            updated = true;
        }
    }
    return updated;
}

bool NamedInputInfo::updateData (dataRecord &&update, int index)
{
    if (!only_update_on_change)
    {
        current_data[index] = std::move (update);
        return true;
    }

    if (*current_data[index].data != *(update.data))
    {
        current_data[index] = std::move (update);
        return true;
    }
    if (current_data[index].time == update.time)
    {  // this is for bookkeeping purposes should still return false
        current_data[index].iteration = update.iteration;
    }
    return false;
}

Time NamedInputInfo::nextValueTime () const
{
    Time nvtime = Time::maxVal ();
    for (const auto &q : data_queues)
    {
        if (!q.empty ())
        {
            if (q.front ().time < nvtime)
            {
                nvtime = q.front ().time;
            }
        }
    }
    return nvtime;
}
}  // namespace helics
