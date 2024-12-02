/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "configFileHelpers.hpp"

#include <cctype>
#include <string>
#include <type_traits>

namespace helics {

template<typename Callable>
bool addTargets(const toml::value& section, std::string targetName, Callable callback)
{
    bool found{false};
    toml::value uval;
    // There should probably be a static_assert here but there isn't a nice type trait to check that
    auto targets = toml::find_or(section, targetName, uval);
    if (!targets.is_empty()) {
        if (targets.is_array()) {
            auto& targetArray = targets.as_array();
            for (const auto& target : targetArray) {
                callback(static_cast<const std::string&>(target.as_string()));
            }
        } else {
            callback(static_cast<const std::string&>(targets.as_string()));
        }
        found = true;
    }
    if (targetName.back() == 's') {
        targetName.pop_back();
        std::string target;
        target = toml::find_or(section, targetName, target);
        if (!target.empty()) {
            found = true;
            callback(target);
        }
    }
    return found;
}

template<typename Callable>
bool addTargets(const nlohmann::json& section, std::string targetName, Callable callback)
{
    bool found{false};
    // There should probably be a static_assert here but there isn't a nice type trait to check that
    if (section.contains(targetName)) {
        auto targets = section[targetName];
        if (targets.is_array()) {
            for (const auto& target : targets) {
                callback(target.get<std::string>());
            }
        } else {
            callback(targets.get<std::string>());
        }
        found = true;
    }
    if (targetName.back() == 's') {
        targetName.pop_back();
        if (section.contains(targetName)) {
            callback(section[targetName].get<std::string>());
            found = true;
        }
    }
    return found;
}

template<typename Block, typename Callable>
void addTargetVariations(const Block& section,
                         const std::string& name1,
                         std::string name2,
                         Callable callback)
{
    bool found = addTargets(section, name1 + "_" + name2, callback);
    if (!found) {
        found = addTargets(section, name1 + name2, callback);
    }
    if (!found) {
        name2.front() = std::toupper(name2.front());
        addTargets(section, name1 + name2, callback);
    }
}

void processOptions(const toml::value& section,
                    const std::function<int(const std::string&)>& optionConversion,
                    const std::function<int(const std::string&)>& valueConversion,
                    const std::function<void(int, int)>& optionAction);

void processOptions(const nlohmann::json& section,
                    const std::function<int(const std::string&)>& optionConversion,
                    const std::function<int(const std::string&)>& valueConversion,
                    const std::function<void(int, int)>& optionAction);

void loadTags(const nlohmann::json& section,
              const std::function<void(std::string_view, std::string_view)>& tagAction);

void loadTags(const toml::value& section,
              const std::function<void(std::string_view, std::string_view)>& tagAction);

}  // namespace helics
