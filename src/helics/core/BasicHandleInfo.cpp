/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "BasicHandleInfo.hpp"

namespace helics {

void BasicHandleInfo::setTag(const std::string& tag, const std::string& value)
{
    for (auto& tg : tags) {
        if (tg.first == tag) {
            tg.second = value;
            return;
        }
    }
    tags.emplace_back(tag, value);
}

static const std::string emptyStr;

const std::string& BasicHandleInfo::getTag(const std::string& tag) const
{
    for (const auto& tg : tags) {
        if (tg.first == tag) {
            return tg.second;
        }
    }
    return emptyStr;
}

}  // namespace helics
