/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "BasicHandleInfo.hpp"

#include <string>
namespace helics {

// NOLINTNEXTLINE
const std::string BasicHandleInfo::emptyString{};

void BasicHandleInfo::setTag(std::string_view tag, std::string_view value)
{
    for (auto& tg : tags) {
        if (tg.first == tag) {
            tg.second = value;
            return;
        }
    }
    tags.emplace_back(tag, value);
}

const std::string& BasicHandleInfo::getTag(std::string_view tag) const
{
    for (const auto& tg : tags) {
        if (tg.first == tag) {
            return tg.second;
        }
    }
    return emptyString;
}

}  // namespace helics
