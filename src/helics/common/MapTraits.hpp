/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

#pragma once
#include <type_traits>
#include <string>

/** type trait to check if a type is easily hashable
@details this is not necessarily all hashable types, just those which are easily known
so arithmetic types, pointers, and strings others may be added if needed
*/
template<typename X>
struct is_easily_hashable
{
    static const bool value = std::is_scalar<X>::value;
};

/** type overload for std::string*/
template<>
struct is_easily_hashable<std::string>
{
    static const bool value = true;
};

/** type overload for std::wstring*/
template<>
struct is_easily_hashable<std::wstring>
{
    static const bool value = true;
};
