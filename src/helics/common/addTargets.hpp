/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. 
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <string>
#include <type_traits>

namespace helics
{
template <typename Callable>
void addTargets (const toml::Value &section, std::string targetName, Callable callback)
{
    // There should probably be a static_assert here but there isn't a nice type trait to check that
    auto targets = section.find (targetName);
    if (targets != nullptr)
    {
        if (targets->is<toml::Array> ())
        {
            auto &targetArray = targets->as<toml::Array> ();
            for (const auto &target : targetArray)
            {
                callback (target.as<std::string> ());
            }
        }
        else
        {
            callback (targets->as<std::string> ());
        }
    }
    if (targetName.back () == 's')
    {
        targetName.pop_back ();
        auto target = section.find (targetName);
        if (target != nullptr)
        {
            callback (target->as<std::string> ());
        }
    }
}

template <typename Callable>
void addTargets (const Json::Value &section, std::string targetName, Callable callback)
{
    // There should probably be a static_assert here but there isn't a nice type trait to check that
    if (section.isMember (targetName))
    {
        auto targets = section[targetName];
        if (targets.isArray ())
        {
            for (const auto &target : targets)
            {
                callback (target.asString ());
            }
        }
        else
        {
            callback (targets.asString ());
        }
    }
    if (targetName.back () == 's')
    {
        targetName.pop_back ();
        if (section.isMember (targetName))
        {
            callback (section[targetName].asString ());
        }
    }
}

}  // namespace helics
