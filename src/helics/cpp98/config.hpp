/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_CONFIG_HPP_
#define HELICS_CPP98_CONFIG_HPP_
#pragma once

//  Detect whether the compiler supports C++11 rvalue references.
#if (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 2)) &&               \
     defined(__GXX_EXPERIMENTAL_CXX0X__))
#    define HELICS_HAS_RVALUE_REFS
#elif defined(__clang__)
#    if __has_feature(cxx_rvalue_references)
#        define HELICS_HAS_RVALUE_REFS
#    endif
#elif defined(_MSC_VER) && (_MSC_VER >= 1900)
#    define HELICS_HAS_RVALUE_REFS
#elif defined(_MSC_VER) && (_MSC_VER >= 1600)
#    define HELICS_HAS_RVALUE_REFS
#endif

//  Detect whether the compiler supports C++11.
#if __cplusplus > 199711L
#    define HELICS_THROWS_EXCEPTION noexcept(false)
#    define HELICS_NOTHROW noexcept
#    define HELICS_NULL_POINTER nullptr
#    define HELICS_HAS_FUNCTIONAL 1
#elif defined(_MSC_VER) && (_MSC_VER >= 1900)
#    define HELICS_THROWS_EXCEPTION noexcept(false)
#    define HELICS_NOTHROW noexcept
#    define HELICS_NULL_POINTER nullptr
#    define HELICS_HAS_FUNCTIONAL 1
#else
#    define HELICS_THROWS_EXCEPTION throw(HelicsException)
#    define HELICS_NOTHROW throw()
#    define HELICS_NULL_POINTER NULL
#    define HELICS_HAS_FUNCTIONAL 0
#endif

#define HELICS_IGNORE_ERROR HELICS_NULL_POINTER
#endif
