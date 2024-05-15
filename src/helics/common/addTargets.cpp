/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "addTargets.hpp"

#include "JsonProcessingFunctions.hpp"
#include "TomlProcessingFunctions.hpp"

#include <string>
#include <type_traits>
#include <utility>

namespace helics {

void processOptions(const toml::value& section,
                    const std::function<int(const std::string&)>& optionConversion,
                    const std::function<int(const std::string&)>& valueConversion,
                    const std::function<void(int, int)>& optionAction)
{
    const auto& t = section.as_table();
    for (const auto& telement : t) {
        if (telement.second.is_array() || telement.second.is_table()) {
            continue;
        }
        int32_t index = optionConversion(telement.first);
        if (index >= 0) {
            int32_t val = -1;
            if (telement.second.is_boolean()) {
                val = telement.second.as_boolean() ? 1 : 0;
            } else if (telement.second.is_integer()) {
                val = telement.second.as_integer();
            } else {
                val = valueConversion(telement.second.as_string());
            }
            optionAction(index, val);
        }
    }
}

void processOptions(const nlohmann::json& section,
                    const std::function<int(const std::string&)>& optionConversion,
                    const std::function<int(const std::string&)>& valueConversion,
                    const std::function<void(int, int)>& optionAction)
{
    auto stop = section.end();
    for (const auto& [key, value] : section.items()) {
        if (value.is_array() || value.is_object()) {
            continue;
        }
        int32_t index = optionConversion(key);
        if (index >= 0) {
            int32_t val = -1;
            if (value.is_boolean()) {
                val = value.get<bool>() ? 1 : 0;
            } else if (value.is_number_integer()) {
                val = value.get<int32_t>();
            } else {
                val = valueConversion(value.get<std::string>());
            }
            optionAction(index, val);
        }
    }
}

template<typename TV>
static std::pair<std::string, std::string> getTagPair(const TV& tv)
{
    std::string name = fileops::getName(tv);
    if (name.empty()) {
    } else {
        std::string val = fileops::getOrDefault(tv, std::string("value"), std::string_view{});
        return std::make_pair(name, val);
    }

    return std::make_pair(std::string{}, std::string{});
}

void loadTags(const nlohmann::json& section,
              const std::function<void(std::string_view, std::string_view)>& tagAction)
{
    if (section.contains("tags")) {
        auto tv = section["tags"];
        if (tv.is_array()) {
            for (auto& tp : tv) {
                auto pv = getTagPair(tp);
                if (!pv.first.empty()) {
                    tagAction(pv.first, pv.second);
                }
            }
        } else {
            auto pv = getTagPair(tv);
            if (!pv.first.empty()) {
                tagAction(pv.first, pv.second);
            } else if (tv.is_object()) {
                for (auto& item : tv.items()) {
                    tagAction(item.key(),
                              (item.value().is_string()) ?
                                  item.value().get<std::string>() :
                                  fileops::generateJsonString(item.value()));
                }
            }
        }
    }
}

void loadTags(const toml::value& section,
              const std::function<void(std::string_view, std::string_view)>& tagAction)
{
    if (section.contains("tags")) {
        auto tv = section.at("tags");
        if (tv.is_array()) {
            for (std::size_t ii = 0; ii < tv.size(); ++ii) {
                auto pv = getTagPair(tv[ii]);
                if (!pv.first.empty()) {
                    tagAction(pv.first, pv.second);
                }
            }
        } else {
            auto pv = getTagPair(tv);
            if (!pv.first.empty()) {
                tagAction(pv.first, pv.second);
            } else if (tv.is_table()) {
                for (auto& values : tv.as_table()) {
                    tagAction(values.first, fileops::tomlAsString(values.second));
                }
            }
        }
    }
}

}  // namespace helics
