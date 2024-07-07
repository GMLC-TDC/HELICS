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

void processOptions(const Json::Value& section,
                    const std::function<int(const std::string&)>& optionConversion,
                    const std::function<int(const std::string&)>& valueConversion,
                    const std::function<void(int, int)>& optionAction)
{
    auto stop = section.end();
    for (auto sIterator = section.begin(); sIterator != stop; ++sIterator) {
        if (sIterator->isArray() || sIterator->isObject()) {
            continue;
        }
        const int32_t index = optionConversion(sIterator.name());
        if (index >= 0) {
            int32_t val = -1;
            if (sIterator->isBool()) {
                val = sIterator->asBool() ? 1 : 0;
            } else if (sIterator->isInt64()) {
                val = sIterator->asInt64();
            } else {
                val = valueConversion(sIterator->asString());
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

void loadTags(const Json::Value& section,
              const std::function<void(std::string_view, std::string_view)>& tagAction)
{
    if (section.isMember("tags")) {
        auto tagValue = section["tags"];
        if (tagValue.isArray()) {
            for (auto& tagPair : tagValue) {
                if (tagPair.isObject()) {
                    auto pairValues = getTagPair(tagPair);
                    if (!pairValues.first.empty()) {
                        tagAction(pairValues.first, pairValues.second);
                    }
                } else if (tagPair.isArray()) {
                    if (tagPair.size() > 1) {
                        tagAction(tagPair[0].asString(), tagPair[1].asString());
                    } else {
                        tagAction(tagPair[0].asString(), "1");
                    }
                } else if (tagPair.isString()) {
                    tagAction(tagPair.asString(), "1");
                }
            }
        } else if (tagValue.isObject()) {
            auto tagPair = getTagPair(tagValue);
            if (!tagPair.first.empty()) {
                tagAction(tagPair.first, tagPair.second);
            } else if (tagValue.isObject()) {
                auto names = tagValue.getMemberNames();
                for (auto& name : names) {
                    tagAction(name,
                              (tagValue[name].isString()) ?
                                  tagValue[name].asString() :
                                  fileops::generateJsonString(tagValue[name]));
                }
            }
        } else if (tagValue.isString()) {
            tagAction("tags", tagValue.asString());
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
