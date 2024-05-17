/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include "../common/JsonGeneration.hpp"

#include <functional>
#include <string>
#include <string_view>
#include <type_traits>

namespace helics {
class HandleManager;
class GlobalFederateId;
class FederateState;
class InterfaceInfo;

/// enumeration of subqueries that cascade and need multiple levels of processing
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
    GLOBAL_STATUS = 9,
    BARRIERS = 11,
    UNCONNECTED_INTERFACES = 14
};

/// Enumeration of if query result is reusable
enum class QueryReuse : std::uint8_t { ENABLED = 0, DISABLED = 1 };

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
void generateInterfaceConfig(nlohmann::json& iblock,
                             const helics::HandleManager& hm,
                             const helics::GlobalFederateId& fed);

nlohmann::json generateInterfaceConfig(const helics::HandleManager& hm,
                                       const helics::GlobalFederateId& fed);

void addFederateTags(nlohmann::json& v, const helics::FederateState* fed);
/** generate results from a query related to interfaces*/
std::string
    generateInterfaceQueryResults(std::string_view request,
                                  const HandleManager& handles,
                                  const GlobalFederateId fed,
                                  const std::function<void(nlohmann::json&)>& addHeaderInfo);

std::string
    generateInterfaceQueryResults(std::string_view request,
                                  const InterfaceInfo& info,
                                  const std::function<void(nlohmann::json&)>& addHeaderInfo);
}  // namespace helics
