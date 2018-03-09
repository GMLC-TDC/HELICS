/*
Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
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

