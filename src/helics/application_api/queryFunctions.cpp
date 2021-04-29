/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "queryFunctions.hpp"

#include "Federate.hpp"
#include "gmlc/utilities/stringOps.h"

#include <algorithm>
#include <thread>
#include <utility>

namespace helics {
std::vector<std::string> vectorizeQueryResult(std::string&& queryres)
{
    if (queryres.empty()) {
        return std::vector<std::string>();
    }
    if (queryres.front() == '[') {
        std::vector<std::string> strs = gmlc::utilities::stringOps::splitline(queryres, ';');
        strs.front() = strs.front().substr(1);  // get rid of the leading '['
        strs.back().pop_back();  // get rid of the trailing ']';
        return strs;
    }
    std::vector<std::string> res;
    res.push_back(std::move(queryres));
    return res;
}

std::vector<std::string> vectorizeQueryResult(const std::string& queryres)
{
    if (queryres.empty()) {
        return std::vector<std::string>();
    }
    if (queryres.front() == '[') {
        std::vector<std::string> strs = gmlc::utilities::stringOps::splitline(queryres, ';');
        strs.front() = strs.front().substr(1);  // get rid of the leading '['
        strs.back().pop_back();  // get rid of the trailing ']';
        return strs;
    }
    std::vector<std::string> res;
    res.push_back(queryres);
    return res;
}

std::vector<int> vectorizeIndexQuery(const std::string& queryres)
{
    std::vector<int> result;
    if (queryres.empty()) {
        return result;
    }

    if (queryres.front() == '[') {
        auto strs = vectorizeQueryResult(queryres);
        result.reserve(strs.size());
        for (auto& str : strs) {
            try {
                result.push_back(std::stoi(str));
            }
            catch (const std::invalid_argument&) {
                continue;
            }
        }
    }
    try {
        result.push_back(std::stoi(queryres));
    }
    catch (const std::invalid_argument&) {
    }
    return result;
}

std::vector<std::string> vectorizeAndSortQueryResult(const std::string& queryres)
{
    auto vec = vectorizeQueryResult(queryres);
    std::sort(vec.begin(), vec.end());
    return vec;
}

std::vector<std::string> vectorizeAndSortQueryResult(std::string&& queryres)
{
    auto vec = vectorizeQueryResult(std::move(queryres));
    std::sort(vec.begin(), vec.end());
    return vec;
}

bool waitForInit(helics::Federate* fed,
                 const std::string& fedName,
                 std::chrono::milliseconds timeout)
{
    auto res = fed->query(fedName, "isinit", helics_sequencing_mode_ordered);
    std::chrono::milliseconds waitTime{0};
    const std::chrono::milliseconds delta{400};
    while (res != "true") {
        if (res == "#invalid") {
            return false;
        }
        std::this_thread::sleep_for(delta);
        res = fed->query(fedName, "isinit", helics_sequencing_mode_ordered);
        waitTime += delta;
        if (waitTime >= timeout) {
            return false;
        }
    }
    return true;
}

bool waitForFed(helics::Federate* fed,
                const std::string& fedName,
                std::chrono::milliseconds timeout)
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

std::string queryFederateSubscriptions(helics::Federate* fed, const std::string& fedName)
{
    auto res = fed->query(fedName, "subscriptions", helics_sequencing_mode_ordered);
    if (res.size() > 2 && res != "#invalid") {
        res = fed->query("gid_to_name", res);
    }
    return res;
}

}  // namespace helics
