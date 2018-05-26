/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "helics/compiler-config.h"
#ifndef HAVE_SHARED_TIMED_MUTEX
#ifndef HAVE_SHARED_MUTEX
    #define LIBGUARDED_NO_DEFAULT 1
#endif
#endif
#include <libguarded/atomic_guarded.hpp>
#include <libguarded/guarded.hpp>
#include <libguarded/ordered_guarded.hpp>
#include <libguarded/shared_guarded.hpp>
//#include <libguarded/staged_guarded.hpp>


template <class T>
using guarded = libguarded::guarded<T>;

template <class T>
using atomic_guarded = libguarded::atomic_guarded<T>;

template <class T>
using shared_guarded_m = libguarded::shared_guarded<T, std::mutex>;

template <class T>
using ordered_guarded_m = libguarded::ordered_guarded<T, std::mutex>;

#ifdef HAVE_SHARED_MUTEX
template <class T>
using shared_guarded = libguarded::shared_guarded<T, std::shared_mutex>;

template <class T>
using ordered_guarded = libguarded::ordered_guarded<T, std::shared_mutex>;
#else
#ifdef HAVE_SHARED_TIMED_MUTEX
template <class T>
using shared_guarded = libguarded::shared_guarded<T, std::shared_timed_mutex>;

template <class T>
using ordered_guarded = libguarded::ordered_guarded<T, std::shared_timed_mutex>;
#else
#include <mutex>
template <class T>
using shared_guarded = libguarded::shared_guarded<T, std::mutex>;

template <class T>
using ordered_guarded = libguarded::ordered_guarded<T, std::mutex>;
#endif
#endif
