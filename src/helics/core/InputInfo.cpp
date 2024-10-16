/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "InputInfo.hpp"

#include "../common/JsonGeneration.hpp"
#include "helics_definitions.hpp"
#include "units/units.hpp"

#include <algorithm>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace helics {
const std::vector<std::shared_ptr<const SmallBuffer>>& InputInfo::getAllData() const
{
    return current_data;
}

static const std::shared_ptr<const SmallBuffer> NullData{nullptr};

const std::shared_ptr<const SmallBuffer>& InputInfo::getData(int index) const
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

const std::shared_ptr<const SmallBuffer>& InputInfo::getData(uint32_t* inputIndex) const
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

bool InputInfo::addData(GlobalHandle source_id,
                        Time valueTime,
                        unsigned int iteration,
                        std::shared_ptr<const SmallBuffer> data)
{
    if (!data) {
        return false;
    }
    int index;
    bool found = false;
    for (index = 0; index < static_cast<int>(input_sources.size()); ++index) {
        if (input_sources[index] == source_id) {
            if (valueTime > deactivated[index]) {
                return false;
            }
            found = true;
            break;
        }
    }
    if (!found) {
        return false;
    }
    if (data_queues[index].empty()) {
        if (current_data[index]) {
            if (minTimeGap > timeZero) {
                if ((valueTime - current_data_time[index].first) < minTimeGap) {
                    return false;
                }
            }
            if (only_update_on_change) {
                if (*current_data[index] == *data) {
                    return false;
                }
            }
        }
        data_queues[index].emplace_back(valueTime, iteration, std::move(data));
    } else if (valueTime > data_queues[index].back().time) {
        if (minTimeGap > timeZero) {
            if ((valueTime - data_queues[index].back().time) < minTimeGap) {
                return false;
            }
        }
        if (only_update_on_change) {
            if (*data_queues[index].back().data == *data) {
                return false;
            }
        }
        data_queues[index].emplace_back(valueTime, iteration, std::move(data));
    } else {
        dataRecord newRecord(valueTime, iteration, std::move(data));
        auto m = std::upper_bound(data_queues[index].begin(),
                                  data_queues[index].end(),
                                  newRecord,
                                  recordComparison);
        if (m != data_queues[index].begin()) {
            auto prev = m;
            --prev;
            if (minTimeGap > timeZero) {
                if ((valueTime - prev->time) < minTimeGap) {
                    return false;
                }
            }
            if (only_update_on_change) {
                if (*prev->data == *newRecord.data) {
                    return false;
                }
            }
        }
        data_queues[index].insert(m, std::move(newRecord));
    }
    return true;
}

bool InputInfo::addSource(GlobalHandle newSource,
                          std::string_view sourceName,
                          std::string_view stype,
                          std::string_view sunits)
{
    int index{0};
    for (const auto& is : input_sources) {
        if (is == newSource) {
            if (deactivated[index] < Time::maxVal()) {
                // this is a reconnection
                deactivated[index] = Time::maxVal();
                source_info[index].units = sunits;
                source_info[index].type = sunits;
                return true;
            }
            return false;
        }
        ++index;
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

void InputInfo::disconnectFederate(GlobalFederateId fedToDisconnect, Time minTime)
{
    // the inputUnits and type are not determined anymore since the source list has changed
    inputUnits.clear();
    inputType.clear();
    for (size_t ii = 0; ii < input_sources.size(); ++ii) {
        if (input_sources[ii].fed_id == fedToDisconnect) {
            if (minTime < deactivated[ii]) {
                deactivated[ii] = minTime;
            }
        }
        // there could be duplicate sources so we need to do the full loop
    }
}

void InputInfo::removeSource(GlobalHandle sourceToRemove, Time minTime)
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

void InputInfo::removeSource(std::string_view sourceName, Time minTime)
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
                    inputType.append(generateJsonQuotedString(src.type));
                    inputType.push_back(',');
                }
                inputType.back() = ']';
            }
        }
    }
    return inputType;
}

const std::string& InputInfo::getSourceName(GlobalHandle source) const
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
                    inputUnits.append(generateJsonQuotedString(src.units));
                    inputUnits.push_back(',');
                }
                inputUnits.back() = ']';
            }
        }
    }
    return inputUnits;
}

const std::string& InputInfo::getTargets() const
{
    if (sourceTargets.empty()) {
        if (!source_info.empty()) {
            if (source_info.size() == 1) {
                sourceTargets = source_info.front().key;
            } else {
                sourceTargets.push_back('[');
                for (const auto& src : source_info) {
                    sourceTargets.append(generateJsonQuotedString(src.key));
                    sourceTargets.push_back(',');
                }
                sourceTargets.back() = ']';
            }
        }
    }
    return sourceTargets;
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

void InputInfo::setProperty(int32_t option, int32_t value)
{
    bool bvalue = (value != 0);
    switch (option) {
        case defs::Options::IGNORE_INTERRUPTS:
            not_interruptible = bvalue;
            break;
        case defs::Options::HANDLE_ONLY_UPDATE_ON_CHANGE:
            only_update_on_change = bvalue;
            break;
        case defs::Options::CONNECTION_REQUIRED:
            required = bvalue;
            break;
        case defs::Options::CONNECTION_OPTIONAL:
            required = !bvalue;
            break;
        case defs::Options::SINGLE_CONNECTION_ONLY:
            required_connnections = bvalue ? 1 : 0;
            break;
        case defs::Options::MULTIPLE_CONNECTIONS_ALLOWED:
            required_connnections = bvalue ? 0 : 1;
            break;
        case defs::Options::STRICT_TYPE_CHECKING:
            strict_type_matching = bvalue;
            break;
        case defs::Options::IGNORE_UNIT_MISMATCH:
            ignore_unit_mismatch = bvalue;
            break;
        case defs::Options::CONNECTIONS:
            required_connnections = value;
            break;
        case defs::Options::INPUT_PRIORITY_LOCATION:
            priority_sources.push_back(value);
            break;
        case defs::Options::CLEAR_PRIORITY_LIST:
            priority_sources.clear();
            break;
        case defs::Options::TIME_RESTRICTED:
            minTimeGap = Time(value, time_units::ms);
            break;
        default:
            break;
    }
}

int32_t InputInfo::getProperty(int32_t option) const
{
    bool flagval = false;
    switch (option) {
        case defs::Options::IGNORE_INTERRUPTS:
            flagval = not_interruptible;
            break;
        case defs::Options::HANDLE_ONLY_UPDATE_ON_CHANGE:
            flagval = only_update_on_change;
            break;
        case defs::Options::CONNECTION_REQUIRED:
            flagval = required;
            break;
        case defs::Options::CONNECTION_OPTIONAL:
            flagval = !required;
            break;
        case defs::Options::SINGLE_CONNECTION_ONLY:
            flagval = (required_connnections == 1);
            break;
        case defs::Options::MULTIPLE_CONNECTIONS_ALLOWED:
            flagval = (required_connnections != 1);
            break;
        case defs::Options::STRICT_TYPE_CHECKING:
            flagval = strict_type_matching;
            break;
        case defs::Options::CONNECTIONS:
            return static_cast<int32_t>(input_sources.size());
        case defs::Options::INPUT_PRIORITY_LOCATION:
            return priority_sources.empty() ? -1 : priority_sources.back();
        case defs::Options::CLEAR_PRIORITY_LIST:
            flagval = priority_sources.empty();
            break;
        case defs::Options::TIME_RESTRICTED:
            return static_cast<std::int32_t>(minTimeGap.to_ms().count());
        default:
            break;
    }
    return flagval ? 1 : 0;
}

static const std::set<std::string_view> convertible_set{"double_vector",
                                                        "complex_vector",
                                                        "vector",
                                                        "double",
                                                        "float",
                                                        "bool",
                                                        "time",
                                                        "char",
                                                        "uchar",
                                                        "json",
                                                        "int32",
                                                        "int64",
                                                        "uint32",
                                                        "uint64",
                                                        "int16",
                                                        "string",
                                                        "complex",
                                                        "complex_f",
                                                        "named_point"};
bool checkTypeMatch(std::string_view type1, std::string_view type2, bool strict_match)
{
    if ((type1.empty()) || (type1 == type2) || (type1 == "def") || (type1 == "any") ||
        (type1 == "raw") || (type1 == "json")) {
        return true;
    }
    if (strict_match) {
        return false;
    }

    if ((type2.empty()) || (type2 == "def") || (type2 == "any") || (type1 == "json")) {
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
