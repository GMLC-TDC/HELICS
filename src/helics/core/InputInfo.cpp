/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "InputInfo.hpp"

#include "units/units/units.hpp"

#include <algorithm>
#include <memory>
#include <set>
#include <string>
#include <utility>

namespace helics {
const std::vector<std::shared_ptr<const data_block>>& InputInfo::getAllData() const
{
    return current_data;
}

static const std::shared_ptr<const data_block> NullData{nullptr};

const std::shared_ptr<const data_block>& InputInfo::getData(int index) const
{
    if (isValidIndex(index, current_data)) {
        return current_data[index];
    }
    return NullData;
}

/** return true if index1 has higher priority than index2*/
static bool priorityCheck(int32_t index1, int32_t index2, const std::vector<int32_t>& priorities)
{
    for (auto priority = priorities.rbegin(); priority != priorities.rend(); ++priority) {
        if (*priority == index1) {
            return true;
        }
        if (*priority == index2) {
            return false;
        }
    }
    return false;
}

const std::shared_ptr<const data_block>& InputInfo::getData(uint32_t* inputIndex) const
{
    int ind{0};
    int mxind{-1};
    Time mxTime{Time::minVal()};
    for (const auto& cd : current_data_time) {
        if (cd.first > mxTime) {
            mxTime = cd.first;
            mxind = ind;
        } else if (cd.first == mxTime) {
            if (priorityCheck(ind, mxind, priority_sources)) {
                mxind = ind;
            }
        }
        ++ind;
    }
    if (mxind >= 0) {
        if (inputIndex != nullptr) {
            *inputIndex = mxind;
        }
        return current_data[mxind];
    }
    if (inputIndex != nullptr) {
        *inputIndex = 0;
    }
    return NullData;
}

static auto recordComparison = [](const InputInfo::dataRecord& rec1,
                                  const InputInfo::dataRecord& rec2) {
    return (rec1.time < rec2.time) ?
        true :
        ((rec1.time == rec2.time) ? (rec1.iteration < rec2.iteration) : false);
};

void InputInfo::addData(global_handle source_id,
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
        auto m = std::upper_bound(data_queues[index].begin(),
                                  data_queues[index].end(),
                                  newRecord,
                                  recordComparison);
        data_queues[index].insert(m, std::move(newRecord));
    }
}

bool InputInfo::addSource(global_handle newSource,
                          const std::string& sourceName,
                          const std::string& stype,
                          const std::string& sunits)
{
    for (const auto& is : input_sources) {
        if (is == newSource) {
            return false;
        }
    }
    // clear this since it isn't well defined what the units are once a new source is added
    inputUnits.clear();
    inputType.clear();

    input_sources.push_back(newSource);
    source_info.emplace_back(sourceName, stype, sunits);
    data_queues.resize(input_sources.size());
    current_data.resize(input_sources.size());
    current_data_time.resize(input_sources.size(), {Time::minVal(), 0});
    deactivated.push_back(Time::maxVal());
    has_target = true;
    return true;
}

void InputInfo::removeSource(global_handle sourceToRemove, Time minTime)
{
    // the inputUnits and type are not determined anymore since the source list has changed
    inputUnits.clear();
    inputType.clear();
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

void InputInfo::removeSource(const std::string& sourceName, Time minTime)
{
    inputUnits.clear();
    inputType.clear();
    for (size_t ii = 0; ii < source_info.size(); ++ii) {
        if (source_info[ii].key == sourceName) {
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

void InputInfo::clearFutureData()
{
    for (auto& vec : data_queues) {
        vec.clear();
    }
}

const std::string& InputInfo::getInjectionType() const
{
    if (inputType.empty()) {
        if (!source_info.empty()) {
            bool allTheSame{true};
            for (const auto& src : source_info) {
                if (src.type != source_info.front().type) {
                    allTheSame = false;
                    break;
                }
            }
            if (allTheSame) {
                inputType = source_info.front().type;
            } else {
                inputType.push_back('[');
                for (const auto& src : source_info) {
                    inputType.push_back('"');
                    inputType.append(src.type);
                    inputType.push_back('"');
                    inputType.push_back(',');
                }
                inputType.back() = ']';
            }
        }
    }
    return inputType;
}

const std::string& InputInfo::getSourceName(global_handle source) const
{
    static const std::string empty{};
    size_t ii{0};
    while (ii < input_sources.size()) {
        if (source == input_sources[ii]) {
            return source_info[ii].key;
        }
    }
    return empty;
}

const std::string& InputInfo::getInjectionUnits() const
{
    if (inputUnits.empty()) {
        if (!source_info.empty()) {
            bool allTheSame{true};
            for (const auto& src : source_info) {
                if (src.units != source_info.front().units) {
                    allTheSame = false;
                    break;
                }
            }
            if (allTheSame) {
                inputUnits = source_info.front().units;
            } else {
                inputUnits.push_back('[');
                for (const auto& src : source_info) {
                    inputUnits.push_back('"');
                    inputUnits.append(src.units);
                    inputUnits.push_back('"');
                    inputUnits.push_back(',');
                }
                inputUnits.back() = ']';
            }
        }
    }
    return inputUnits;
}

bool InputInfo::updateTimeUpTo(Time newTime)
{
    int index{0};
    bool updated{false};
    for (auto& data_queue : data_queues) {
        auto currentValue = data_queue.begin();

        auto it_final = data_queue.end();
        if (currentValue == it_final) {
            ++index;
            continue;
        }
        if (currentValue->time >= newTime) {
            ++index;
            continue;
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

bool InputInfo::updateTimeNextIteration(Time newTime)
{
    int index{0};
    bool updated{false};
    for (auto& data_queue : data_queues) {
        auto currentValue = data_queue.begin();
        auto it_final = data_queue.end();
        if (currentValue == it_final) {
            ++index;
            continue;
        }
        if (currentValue->time > newTime) {
            ++index;
            continue;
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

bool InputInfo::updateTimeInclusive(Time newTime)
{
    int index = 0;
    bool updated = false;
    for (auto& data_queue : data_queues) {
        auto currentValue = data_queue.begin();
        auto it_final = data_queue.end();
        if (currentValue == it_final) {
            ++index;
            continue;
        }
        if (currentValue->time > newTime) {
            ++index;
            continue;
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

bool InputInfo::updateData(dataRecord&& update, int index)
{
    if (!only_update_on_change || !current_data[index]) {
        current_data[index] = std::move(update.data);
        current_data_time[index] = {update.time, update.iteration};
        return true;
    }

    if (*current_data[index] != *(update.data)) {
        current_data[index] = std::move(update.data);
        current_data_time[index] = {update.time, update.iteration};
        return true;
    }
    if (current_data_time[index].first == update.time) {
        // this is for bookkeeping purposes should still return false
        current_data_time[index].second = update.iteration;
    }
    return false;
}

Time InputInfo::nextValueTime() const
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

}  // namespace helics
