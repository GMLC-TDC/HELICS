/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "queryFunctions.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "Federate.hpp"
#include "gmlc/utilities/string_viewOps.h"

#include <algorithm>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace helics {

std::vector<std::string> vectorizeQueryResult(std::string_view queryres)
{
    if (queryres.empty()) {
        return std::vector<std::string>();
    }

    if (queryres.front() == '[') {
        try {
            auto v = fileops::loadJsonStr(queryres);
            std::vector<std::string> strs;
            if (v.is_array()) {
                for (auto& str : v) {
                    if (str.is_string()) {
                        strs.emplace_back(str.get<std::string>());
                    } else {
                        strs.emplace_back(fileops::generateJsonString(str));
                    }
                }
            } else if (v.is_string()) {
                strs.emplace_back(v.get<std::string>());
            } else {
                strs.emplace_back(fileops::generateJsonString(v));
            }
            return strs;
        }
        catch (...) {
        }
    }
    std::vector<std::string> res;
    res.emplace_back(queryres);
    return res;
}

std::vector<int> vectorizeIndexQuery(std::string_view queryres)
{
    std::vector<int> result;
    if (queryres.empty()) {
        return result;
    }

    if (queryres.front() == '[') {
        try {
            auto v = fileops::loadJsonStr(queryres);
            if (v.is_array()) {
                for (auto& val : v) {
                    if (val.is_number_integer()) {
                        result.push_back(val.get<int>());
                    } else if (val.is_number()) {
                        result.push_back(val.get<double>());
                    } else {
                        continue;
                    }
                }
            } else if (v.is_number_integer()) {
                result.push_back(v.get<int>());
            } else if (v.is_number()) {
                result.push_back(v.get<double>());
            } else if (v.is_string()) {
                result.push_back(std::stoi(v.get<std::string>()));
            } else {
                result.push_back(std::stoi(std::string(queryres)));
            }
            return result;
        }
        catch (...) {
        }
    }
    try {
        result.push_back(std::stoi(std::string(queryres)));
    }
    catch (const std::invalid_argument&) {
    }
    return result;
}

std::vector<std::string> vectorizeAndSortQueryResult(std::string_view queryres)
{
    auto vec = vectorizeQueryResult(queryres);
    std::sort(vec.begin(), vec.end());
    return vec;
}

bool waitForInit(helics::Federate* fed, std::string_view fedName, std::chrono::milliseconds timeout)
{
    auto res = fed->query(fedName, "isinit", HELICS_SEQUENCING_MODE_ORDERED);
    std::chrono::milliseconds waitTime{0};
    const std::chrono::milliseconds delta{400};
    while (res != "true") {
        if (res.find("error") != std::string::npos) {
            return false;
        }
        std::this_thread::sleep_for(delta);
        res = fed->query(fedName, "isinit", HELICS_SEQUENCING_MODE_ORDERED);
        waitTime += delta;
        if (waitTime >= timeout) {
            return false;
        }
    }
    return true;
}

bool waitForFed(helics::Federate* fed, std::string_view fedName, std::chrono::milliseconds timeout)
{
    auto res = fed->query(fedName, "exists");
    std::chrono::milliseconds waitTime{0};
    const std::chrono::milliseconds delta{400};
    while (res != "true") {
        std::this_thread::sleep_for(delta);
        res = fed->query(fedName, "exists");
        waitTime += delta;
        if (waitTime >= timeout) {
            return false;
        }
    }
    return true;
}

std::string queryFederateSubscriptions(helics::Federate* fed, std::string_view fedName)
{
    auto res = fed->query(fedName, "subscriptions", HELICS_SEQUENCING_MODE_ORDERED);
    if (res.size() > 2 && res.find("error") == std::string::npos) {
        res = fed->query("gid_to_name", res);
    }
    return res;
}

}  // namespace helics
