/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once
#include <string>
#include <type_traits>

template <typename X, typename Proc>
std::string generateStringVector(const X & data, Proc generator)
{
    static_assert(std::is_convertible<decltype(generator(data.begin())), std::string>::value, "generator output must be convertible to std::string");
    std::string ret;
    ret.push_back('[');
    {
        for (auto &ele : *data)
        {
            ret.append(generator(ele));
            ret.push_back(';');
        }
    }
    if (ret.size() > 1)
    {
        ret.back() = ']';
    }
    else
    {
        ret.push_back(']');
    }
    return ret;
}
