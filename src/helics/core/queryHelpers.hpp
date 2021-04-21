/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include <string>
#include <type_traits>

template<typename X, typename Proc>
std::string generateStringVector(const X& data, Proc generator)
{
    static_assert(std::is_convertible<decltype(generator(*(data.begin()))), std::string>::value,
                  "generator output must be convertible to std::string");
    std::string ret(1, '[');
    for (auto& ele : data) {
        ret.append(generator(ele));
        ret.push_back(';');
    }
    if (ret.size() > 1) {
        ret.back() = ']';
    } else {
        ret.push_back(']');
    }
    return ret;
}

template<typename X, typename Proc, typename validator>
std::string generateStringVector_if(const X& data, Proc generator, validator valid)
{
    static_assert(std::is_convertible<decltype(generator(*(data.begin()))), std::string>::value,
                  "generator output must be convertible to std::string");
    std::string ret(1, '[');
    for (auto& ele : data) {
        if (valid(ele)) {
            ret.append(generator(ele));
            ret.push_back(';');
        }
    }
    if (ret.size() > 1) {
        ret.back() = ']';
    } else {
        ret.push_back(']');
    }
    return ret;
}
