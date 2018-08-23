/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#ifndef HELICS_CPP98_CONFIG_HPP_
#define HELICS_CPP98_CONFIG_HPP_
#pragma once

//  Detect whether the compiler supports C++11 rvalue references.
#if (defined(__GNUC__) && (__GNUC__ > 4 || \
      (__GNUC__ == 4 && __GNUC_MINOR__ > 2)) && \
      defined(__GXX_EXPERIMENTAL_CXX0X__))
#define HELICS_HAS_RVALUE_REFS
#elif defined(__clang__)
#if __has_feature(cxx_rvalue_references)
#define HELICS_HAS_RVALUE_REFS
#endif
#elif defined(_MSC_VER) && (_MSC_VER >= 1900)
#define HELICS_HAS_RVALUE_REFS
#elif defined(_MSC_VER) && (_MSC_VER >= 1600)
#define HELICS_HAS_RVALUE_REFS
#endif

//  Detect whether the compiler supports C++11.
#if __cplusplus > 199711L
#define THROWS_EXCEPTION noexcept (false)
#elif defined(_MSC_VER) && (_MSC_VER >= 1900)
#define THROWS_EXCEPTION noexcept (false)
#else
#define THROWS_EXCEPTION throw (HelicsException)
#endif
#endif
