/*
Copyright (c) 2017-2025,
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

// The following sequence is derived from similar code in CLI11
#if !(defined(_MSC_VER) && __cplusplus == 199711L) && !defined(__INTEL_COMPILER)
#    if __cplusplus >= 201703L
#        define HELICS_CPP17
#    endif
#elif defined(_MSC_VER) && __cplusplus == 199711L
// MSVC sets _MSVC_LANG rather than __cplusplus (supposedly until the standard was fully
// implemented) Unless you use the /Zc:__cplusplus flag on Visual Studio 2017 15.7 Preview 3 or
// newer
#    if _MSVC_LANG > 201402L && _MSC_VER >= 1910
#        define HELICS_CPP17
#    endif
#endif

// GCC < 10 doesn't ignore this in unevaluated contexts
#if !defined(HELICS_CPP17) ||                                                                      \
    (defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER) && __GNUC__ < 10 &&     \
     __GNUC__ > 4)
#    define HELICS_NODISCARD
#else
#    define HELICS_NODISCARD [[nodiscard]]
#endif

#endif
