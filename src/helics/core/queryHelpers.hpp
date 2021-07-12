/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "../common/JsonGeneration.hpp"

#include <string>
#include <type_traits>

namespace helics {
class HandleManager;
class GlobalFederateId;
class FederateState;

// enumeration of subqueries that cascade and need multiple levels of processing
enum Subqueries : std::uint16_t {
    GENERAL_QUERY = 0,
    FEDERATE_MAP = 1,
    CURRENT_TIME_MAP = 2,
    DEPENDENCY_GRAPH = 3,
    DATA_FLOW_GRAPH = 4,
    VERSION_ALL = 5,
    GLOBAL_STATE = 6,
    GLOBAL_TIME_DEBUGGING = 7,
    GLOBAL_FLUSH = 8,
    GLOBAL_STATUS = 9
};

}  // namespace helics

template<typename X, typename Proc>
std::string generateStringVector(const X& data, Proc generator)
{
    static_assert(std::is_convertible<decltype(generator(*(data.begin()))), std::string>::value,
                  "generator output must be convertible to std::string");
    std::string ret{"["};
    for (auto& ele : data) {
        ret.append(helics::generateJsonQuotedString(generator(ele)));
        ret.push_back(',');
    }
    if (ret.size() > 1) {
        ret.back() = ']';
    } else {
        ret.push_back(']');
    }
    return ret;
}

template<typename X, typename Proc, typename validator>
std::string generateStringVector_if(const X& data, Proc generator, validator valid)
{
    static_assert(std::is_convertible<decltype(generator(*(data.begin()))), std::string>::value,
                  "generator output must be convertible to std::string");
    std::string ret{"["};
    for (auto& ele : data) {
        if (valid(ele)) {
            ret.append(helics::generateJsonQuotedString(generator(ele)));
            ret.push_back(',');
        }
    }
    if (ret.size() > 1) {
        ret.back() = ']';
    } else {
        ret.push_back(']');
    }
    return ret;
}

namespace helics {
void generateInterfaceConfig(Json::Value& iblock,
                             const helics::HandleManager& hm,
                             const helics::GlobalFederateId& fed);

Json::Value generateInterfaceConfig(const helics::HandleManager& hm,
                                    const helics::GlobalFederateId& fed);

void addFederateTags(Json::Value& v, const helics::FederateState* fed);
}  // namespace helics
