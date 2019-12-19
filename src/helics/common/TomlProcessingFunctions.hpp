/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

/** @file
@details functions related to loading and evaluating TOML files and helper functions for reading them
using the toml library
*/

#include "../core/helics-time.hpp"
#include "toml.hpp"

#include <functional>

/** load a TOML string or filename that points to a TOML file and return a
toml::Value to the root object
*/
toml::value loadToml(const std::string& tomlString);

bool hasTomlExtension(const std::string& tomlString);
/** load a TOML object in a string
 */
toml::value loadTomlStr(const std::string& tomlString);

/** read a time from a TOML value element*/
helics::Time
    loadTomlTime(const toml::value& timeElement, time_units defaultUnits = time_units::sec);

/** get a name or key from the element*/
std::string getKey(const toml::value& element);

template<class X>
inline X getOrDefault(const toml::value& element, const std::string& key, const X& defVal)
{
    return toml::find_or<X>(element, key, defVal);
}

inline std::string
getOrDefault(const toml::value& element, const std::string& key, const std::string& defVal)
{
    return toml::find_or<std::string>(element, key, defVal);
}

inline double getOrDefault(const toml::value& element, const std::string& key, double defVal)
{
    return toml::find_or<double>(element, key, defVal);
}

inline bool getOrDefault(const toml::value& element, const std::string& key, bool defVal)
{
    return toml::find_or<bool>(element, key, defVal);
}

inline int64_t getOrDefault(const toml::value& element, const std::string& key, int64_t defVal)
{
    return toml::find_or<int64_t>(element, key, defVal);
}


inline void callIfMember(
    const toml::value& element,
    const std::string& key,
    const std::function<void(const std::string&)>& call)
{
    const std::string empty;
    auto &val = toml::find_or<std::string>(element, key,empty);
    if (!val.empty())
    {
        call(val);
    }
}

inline void callIfMember(
    const toml::value& element,
    const std::string& key,
    const std::function<void(const std::string&, helics::Time)>& call)
{
    toml::value uval;
    auto val = toml::find_or(element, key, uval);
    
    if (!val.is_uninitialized())
    {
        call(key, loadTomlTime(val));
    }
}

template<class X>
inline void callIfMember(
    const toml::value& element,
    const std::string& key,
    const std::function<void(const std::string&, X)>& call)
{
    toml::value uval;
    auto val = toml::find_or(element, key, uval);
    if (!val.is_uninitialized())
    {
        call(key, toml::get<X>(val));
    }
}

inline void
    replaceIfMember(const toml::value& element, const std::string& key, helics::Time& timeVal)
{
    toml::value uval;
    auto val = toml::find_or(element, key, uval);

    if (!val.is_uninitialized())
    {
        timeVal = loadTomlTime(val);
    }
}

template<class X>
inline void replaceIfMember(const toml::value& element, const std::string& key, X& loc)
{
    toml::value uval;
    auto val = toml::find_or(element, key, uval);

    if (!val.is_uninitialized())
    {
        loc = toml::get<X>(val);
    }
}

inline bool isMember(const toml::value& element, const std::string& key)
{
    toml::value uval;
    auto val = toml::find_or(element, key, uval);

    return(!val.is_uninitialized());
}
