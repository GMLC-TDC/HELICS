/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

/** @file
@details functions related to loading and evaluating TOML files and helper functions for reading them
using the toml library
*/

#include "toml/toml.h"

#include "../core/helics-time.hpp"
#include <functional>

/** load a TOML string or filename that points to a TOML file and return a
toml::Value to the root object
*/
toml::Value loadToml (const std::string &tomlString);

bool hasTomlExtension (const std::string &tomlString);
/** load a TOML object in a string
 */
toml::Value loadTomlStr (const std::string &tomlString);

/** read a time from a TOML value element*/
helics::Time loadTomlTime (const toml::Value &timeElement, time_units defaultUnits = time_units::sec);

/** get a name or key from the element*/
std::string getKey (const toml::Value &element);

template <class X>
inline X getOrDefault (const toml::Value &element, const std::string &key, const X &defVal)
{
    auto val = element.find (key);
    return (val != nullptr) ? val->as<X> () : defVal;
}

inline void callIfMember (const toml::Value &element,
                          const std::string &key,
                          const std::function<void(const std::string &)> &call)
{
    auto val = element.find (key);
    if (val != nullptr)
    {
        call (val->as<std::string> ());
    }
}

inline void callIfMember (const toml::Value &element,
                          const std::string &key,
                          const std::function<void(const std::string &, helics::Time)> &call)
{
    auto val = element.find (key);
    if (val != nullptr)
    {
        call (key, loadTomlTime (*val));
    }
}

template <class X>
inline void callIfMember (const toml::Value &element,
                          const std::string &key,
                          const std::function<void(const std::string &, X)> &call)
{
    auto val = element.find (key);
    if (val != nullptr)
    {
        call (key, val->as<X> ());
    }
}

inline void replaceIfMember (const toml::Value &element, const std::string &key, helics::Time &timeVal)
{
    auto val = element.find (key);
    if (val != nullptr)
    {
        timeVal = loadTomlTime (*val);
    }
}

template <class X>
inline void replaceIfMember (const toml::Value &element, const std::string &key, X &loc)
{
    auto val = element.find (key);
    if (val != nullptr)
    {
        loc = val->as<X> ();
    }
}

inline bool isMember (const toml::Value &element, const std::string &key)
{
    return (element.find (key) != nullptr);
}
