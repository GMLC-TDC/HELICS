/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "addTargets.hpp"

#include <string>
#include <type_traits>

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
        int32_t index = optionConversion(sIterator.name());
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
}  // namespace helics
