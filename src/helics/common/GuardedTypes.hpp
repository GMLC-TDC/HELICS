/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "helics/compiler-config.h"
#ifndef HAVE_SHARED_TIMED_MUTEX
#    ifndef HAVE_SHARED_MUTEX
#        define LIBGUARDED_NO_DEFAULT 1
#    endif
#endif
#include "gmlc/libguarded/atomic_guarded.hpp"
#include "gmlc/libguarded/guarded.hpp"
#include "gmlc/libguarded/ordered_guarded.hpp"
#include "gmlc/libguarded/shared_guarded.hpp"
//#include "gmlc/libguarded/staged_guarded.hpp"

template<class T>
using guarded = gmlc::libguarded::guarded<T>;

template<class T>
using atomic_guarded = gmlc::libguarded::atomic_guarded<T>;

template<class T>
using shared_guarded_m = gmlc::libguarded::shared_guarded<T, std::mutex>;

template<class T>
using ordered_guarded_m = gmlc::libguarded::ordered_guarded<T, std::mutex>;

#ifdef HAVE_SHARED_MUTEX
template<class T>
using shared_guarded = gmlc::libguarded::shared_guarded<T, std::shared_mutex>;

template<class T>
using ordered_guarded = gmlc::libguarded::ordered_guarded<T, std::shared_mutex>;
#else
#    ifdef HAVE_SHARED_TIMED_MUTEX
template<class T>
using shared_guarded = gmlc::libguarded::shared_guarded<T, std::shared_timed_mutex>;

template<class T>
using ordered_guarded = gmlc::libguarded::ordered_guarded<T, std::shared_timed_mutex>;
#    else
#        include <mutex>
template<class T>
using shared_guarded = gmlc::libguarded::shared_guarded<T, std::mutex>;

template<class T>
using ordered_guarded = gmlc::libguarded::ordered_guarded<T, std::mutex>;
#    endif
#endif
