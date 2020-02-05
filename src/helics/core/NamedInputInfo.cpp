/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "NamedInputInfo.hpp"

#include "units/units/units.hpp"

#include <algorithm>
#include <set>

namespace helics {
std::vector<std::shared_ptr<const data_block>> NamedInputInfo::getAllData()
{
    std::vector<std::shared_ptr<const data_block>> out;
    out.reserve(current_data.size());
    for (auto& cd : current_data) {
        out.push_back(cd.data);
    }
    return out;
}

std::shared_ptr<const data_block> NamedInputInfo::getData(int index)
{
    if (isValidIndex(index, current_data)) {
        return current_data[index].data;
    }
    return nullptr;
}

std::shared_ptr<const data_block> NamedInputInfo::getData()
{
    int ind = 0;
    int mxind = -1;
    Time mxTime = Time::minVal();
    for (auto& cd : current_data) {
        if (cd.time > mxTime) {
            mxTime = cd.time;
            mxind = ind;
        }
        ++ind;
    }
    if (mxind >= 0) {
        return current_data[mxind].data;
    }
    return nullptr;
}

static auto recordComparison = [](const NamedInputInfo::dataRecord& rec1,
                                  const NamedInputInfo::dataRecord& rec2) {
    return (rec1.time < rec2.time) ?
        true :
        ((rec1.time == rec2.time) ? (rec1.iteration < rec2.iteration) : false);
};

void NamedInputInfo::addData(
    global_handle source_id,
    Time valueTime,
    unsigned int iteration,
    std::shared_ptr<const data_block> data)
{
    int index;
    bool found = false;
    for (index = 0; index < static_cast<int>(input_sources.size()); ++index) {
        if (input_sources[index] == source_id) {
            if (valueTime > deactivated[index]) {
                return;
            }
            found = true;
            break;
        }
    }
    if (!found) {
        return;
    }
    if ((data_queues[index].empty()) || (valueTime > data_queues[index].back().time)) {
        data_queues[index].emplace_back(valueTime, iteration, std::move(data));
    } else {
        dataRecord newRecord(valueTime, iteration, std::move(data));
        auto m = std::upper_bound(
            data_queues[index].begin(), data_queues[index].end(), newRecord, recordComparison);
        data_queues[index].insert(m, std::move(newRecord));
    }
}

void NamedInputInfo::addSource(
    global_handle newSource,
    const std::string& sourceName,
    const std::string& stype,
    const std::string& sunits)
{
    if (input_sources.empty()) {
        inputType = stype;
        inputUnits = sunits;
    }
    input_sources.push_back(newSource);
    source_info.emplace_back(sourceName, stype, sunits);
    data_queues.resize(input_sources.size());
    current_data.resize(input_sources.size());
    deactivated.push_back(Time::maxVal());
    has_target = true;
}

void NamedInputInfo::removeSource(global_handle sourceToRemove, Time minTime)
{
    for (size_t ii = 0; ii < input_sources.size(); ++ii) {
        if (input_sources[ii] == sourceToRemove) {
            while ((!data_queues[ii].empty()) && (data_queues[ii].back().time > minTime)) {
                data_queues[ii].pop_back();
            }
            if (minTime < deactivated[ii]) {
                deactivated[ii] = minTime;
            }
        }
        // there could be duplicate sources so we need to do the full loop
    }
}

void NamedInputInfo::removeSource(const std::string& sourceName, Time minTime)
{
    for (size_t ii = 0; ii < source_info.size(); ++ii) {
        if (std::get<0>(source_info[ii]) == sourceName) {
            while ((!data_queues[ii].empty()) && (data_queues[ii].back().time > minTime)) {
                data_queues[ii].pop_back();
            }
            if (minTime < deactivated[ii]) {
                deactivated[ii] = minTime;
            }
        }
        // there could be duplicate sources so we need to do the full loop
    }
}

void NamedInputInfo::clearFutureData()
{
    for (auto& vec : data_queues) {
        vec.clear();
    }
}

bool NamedInputInfo::updateTimeUpTo(Time newTime)
{
    int index = 0;
    bool updated = false;
    for (auto& data_queue : data_queues) {
        auto currentValue = data_queue.begin();

        auto it_final = data_queue.end();
        if (currentValue == it_final) {
            return false;
        }
        if (currentValue->time > newTime) {
            return false;
        }
        auto last = currentValue;
        ++currentValue;
        while ((currentValue != it_final) && (currentValue->time < newTime)) {
            last = currentValue;
            ++currentValue;
        }

        auto res = updateData(std::move(*last), index);
        data_queue.erase(data_queue.begin(), currentValue);
        ++index;
        if (res) {
            updated = true;
        }
    }
    return updated;
}

bool NamedInputInfo::updateTimeNextIteration(Time newTime)
{
    int index = 0;
    bool updated = false;
    for (auto& data_queue : data_queues) {
        auto currentValue = data_queue.begin();
        auto it_final = data_queue.end();
        if (currentValue == it_final) {
            return false;
        }
        if (currentValue->time > newTime) {
            return false;
        }
        auto last = currentValue;
        ++currentValue;
        while ((currentValue != it_final) && (currentValue->time < newTime)) {
            last = currentValue;
            ++currentValue;
        }
        if (currentValue != it_final) {
            if (currentValue->time == newTime) {
                auto cindex = last->iteration;
                while ((currentValue != it_final) && (currentValue->time == newTime) &&
                       (currentValue->iteration == cindex)) {
                    last = currentValue;
                    ++currentValue;
                }
            }
        }

        auto res = updateData(std::move(*last), index);
        data_queue.erase(data_queue.begin(), currentValue);
        ++index;
        if (res) {
            updated = true;
        }
    }
    return updated;
}

bool NamedInputInfo::updateTimeInclusive(Time newTime)
{
    int index = 0;
    bool updated = false;
    for (auto& data_queue : data_queues) {
        auto currentValue = data_queue.begin();
        auto it_final = data_queue.end();
        if (currentValue == it_final) {
            return false;
        }
        if (currentValue->time > newTime) {
            return false;
        }
        auto last = currentValue;
        ++currentValue;
        while ((currentValue != it_final) && (currentValue->time <= newTime)) {
            last = currentValue;
            ++currentValue;
        }

        auto res = updateData(std::move(*last), index);
        data_queue.erase(data_queue.begin(), currentValue);
        ++index;
        if (res) {
            updated = true;
        }
    }
    return updated;
}

bool NamedInputInfo::updateData(dataRecord&& update, int index)
{
    if (!only_update_on_change || !current_data[index].data) {
        current_data[index] = std::move(update);
        return true;
    }

    if (*current_data[index].data != *(update.data)) {
        current_data[index] = std::move(update);
        return true;
    }
    if (current_data[index].time ==
        update.time) { // this is for bookkeeping purposes should still return false
        current_data[index].iteration = update.iteration;
    }
    return false;
}

Time NamedInputInfo::nextValueTime() const
{
    Time nvtime = Time::maxVal();
    if (not_interruptible) {
        return nvtime;
    }
    for (const auto& q : data_queues) {
        if (!q.empty()) {
            if (q.front().time < nvtime) {
                nvtime = q.front().time;
            }
        }
    }
    return nvtime;
}

static const std::set<std::string> convertible_set{"double_vector",
                                                   "complex_vector",
                                                   "vector",
                                                   "double",
                                                   "float",
                                                   "bool",
                                                   "char",
                                                   "uchar",
                                                   "int32",
                                                   "int64",
                                                   "uint32",
                                                   "uint64",
                                                   "int16",
                                                   "string",
                                                   "complex",
                                                   "complex_f",
                                                   "named_point"};
bool checkTypeMatch(const std::string& type1, const std::string& type2, bool strict_match)
{
    if ((type1.empty()) || (type1 == type2) || (type1 == "def") || (type1 == "any") ||
        (type1 == "raw")) {
        return true;
    }
    if (strict_match) {
        return false;
    }

    if ((type2.empty()) || (type2 == "def") || (type2 == "any")) {
        return true;
    }
    if (convertible_set.find(type1) != convertible_set.end()) {
        return ((convertible_set.find(type2) != convertible_set.end()));
    }
    return (type2 == "raw");
}

bool checkUnitMatch(const std::string& unit1, const std::string& unit2, bool strict_match)
{
    if ((unit1.empty()) || (unit1 == unit2) || (unit1 == "def") || (unit1 == "any")) {
        return true;
    }

    if ((unit2.empty()) || (unit2 == "def") || (unit2 == "any")) {
        return true;
    }
    auto u1 = units::unit_from_string(unit1);
    auto u2 = units::unit_from_string(unit2);

    if (!units::is_valid(u1) || !units::is_valid(u2)) {
        return false;
    }
    if (strict_match) {
        double conv = units::quick_convert(u1, u2);
        return (!std::isnan(conv));
    }
    double conv = units::convert(u1, u2);
    return (!std::isnan(conv));
}

} // namespace helics
