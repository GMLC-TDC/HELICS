/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "configFileHelpers.hpp"

#include <string>
#include <type_traits>

namespace helics {
template<typename Callable>
void addTargets(const toml::value& section, std::string targetName, Callable callback)
{
    toml::value uval;
    // There should probably be a static_assert here but there isn't a nice type trait to check that
    auto targets = toml::find_or(section, targetName, uval);
    if (!targets.is_uninitialized()) {
        if (targets.is_array()) {
            auto& targetArray = targets.as_array();
            for (const auto& target : targetArray) {
                callback(target.as_string());
            }
        } else {
            callback(targets.as_string());
        }
    }
    if (targetName.back() == 's') {
        targetName.pop_back();
        std::string target;
        target = toml::find_or(section, targetName, target);
        if (!target.empty()) {
            callback(target);
        }
    }
}

template<typename Callable>
void addTargets(const Json::Value& section, std::string targetName, Callable callback)
{
    // There should probably be a static_assert here but there isn't a nice type trait to check that
    if (section.isMember(targetName)) {
        auto targets = section[targetName];
        if (targets.isArray()) {
            for (const auto& target : targets) {
                callback(target.asString());
            }
        } else {
            callback(targets.asString());
        }
    }
    if (targetName.back() == 's') {
        targetName.pop_back();
        if (section.isMember(targetName)) {
            callback(section[targetName].asString());
        }
    }
}

void processOptions(const toml::value& section,
                    const std::function<int(const std::string&)>& optionConversion,
                    const std::function<int(const std::string&)>& valueConversion,
                    const std::function<void(int, int)>& optionAction);

void processOptions(const Json::Value& section,
                    const std::function<int(const std::string&)>& optionConversion,
                    const std::function<int(const std::string&)>& valueConversion,
                    const std::function<void(int, int)>& optionAction);

}  // namespace helics
