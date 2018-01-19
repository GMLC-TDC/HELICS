/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#pragma once

#include <libguarded/ordered_guarded.hpp>
#include <libguarded/shared_guarded.hpp>

#include "helics/compiler-config.h"

#ifdef HAVE_SHARED_MUTEX
template <class T>
using shared_guarded= libguarded::shared_guarded<T, std::shared_mutex>;

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

