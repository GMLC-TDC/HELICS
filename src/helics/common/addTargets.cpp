/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "addTargets.hpp"

#include "JsonProcessingFunctions.hpp"
#include "TomlProcessingFunctions.hpp"

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace helics {

using fileops::JsonAsString;

void processOptions(const toml::value& section,
                    const std::function<int(const std::string&)>& optionConversion,
                    const std::function<int(const std::string&)>& valueConversion,
                    const std::function<void(int, int)>& optionAction)
{
    const auto& table = section.as_table();
    for (const auto& telement : table) {
        if (telement.second.is_array() || telement.second.is_table()) {
            continue;
        }
        const int32_t index = optionConversion(telement.first);
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
    for (const auto& [key, value] : section.items()) {
        if (value.is_array() || value.is_object()) {
            continue;
        }
        const int32_t index = optionConversion(key);
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
static std::pair<std::string, std::string> getTagPair(const TV& tagValue)
{
    const std::string name = fileops::getName(tagValue);
    if (name.empty()) {
    } else {
        const std::string val =
            fileops::getOrDefault(tagValue, std::string("value"), std::string_view{});
        return std::make_pair(name, val);
    }

    return std::make_pair(std::string{}, std::string{});
}

void loadTags(const nlohmann::json& section,
              const std::function<void(std::string_view, std::string_view)>& tagAction)
{
    if (section.contains("tags")) {
        auto tagValue = section["tags"];
        if (tagValue.is_array()) {
            for (auto& tagPair : tagValue) {
                auto pairValues = getTagPair(tagPair);
                if (!pairValues.first.empty()) {
                    tagAction(pairValues.first, pairValues.second);
                } else if (tagPair.is_array()) {
                    if (tagPair.size() > 1) {
                        tagAction(JsonAsString(tagPair[0]), JsonAsString(tagPair[1]));
                    } else {
                        tagAction(JsonAsString(tagPair[0]), "1");
                    }
                } else if (tagPair.is_string()) {
                    tagAction(tagPair.get<std::string>(), "1");
                }
            }
        } else if (tagValue.is_object()) {
            auto tagPair = getTagPair(tagValue);
            if (!tagPair.first.empty()) {
                tagAction(tagPair.first, tagPair.second);
            } else if (tagValue.is_object()) {
                for (const auto& item : tagValue.items()) {
                    tagAction(item.key(), JsonAsString(item.value()));
                }
            }
        } else if (tagValue.is_string()) {
            tagAction("tags", tagValue.get<std::string>());
        }
    }
}

void loadTags(const toml::value& section,
              const std::function<void(std::string_view, std::string_view)>& tagAction)
{
    if (section.contains("tags")) {
        auto tagValue = section.at("tags");
        if (tagValue.is_array()) {
            for (std::size_t ii = 0; ii < tagValue.size(); ++ii) {
                auto tagPair = getTagPair(tagValue[ii]);
                if (!tagPair.first.empty()) {
                    tagAction(tagPair.first, tagPair.second);
                }
            }
        } else {
            auto tagPair = getTagPair(tagValue);
            if (!tagPair.first.empty()) {
                tagAction(tagPair.first, tagPair.second);
            } else if (tagValue.is_table()) {
                for (auto& values : tagValue.as_table()) {
                    tagAction(values.first, fileops::tomlAsString(values.second));
                }
            }
        }
    }
}

}  // namespace helics
