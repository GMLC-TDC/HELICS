/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "gmlc/libguarded/atomic_guarded.hpp"
#include "gmlc/libguarded/guarded.hpp"
#include "gmlc/libguarded/guarded_opt.hpp"
#include "gmlc/libguarded/ordered_guarded.hpp"
#include "gmlc/libguarded/shared_guarded.hpp"
#include "gmlc/libguarded/shared_guarded_opt.hpp"

// #include "gmlc/libguarded/staged_guarded.hpp"

template<class T>
using guarded = gmlc::libguarded::guarded<T>;

template<class T>
using guarded_opt = gmlc::libguarded::guarded_opt<T>;

template<class T>
using atomic_guarded = gmlc::libguarded::atomic_guarded<T>;

template<class T>
using shared_guarded_m = gmlc::libguarded::shared_guarded<T, std::mutex>;

template<class T>
using shared_guarded_m_opt = gmlc::libguarded::shared_guarded_opt<T, std::mutex>;

template<class T>
using ordered_guarded_m = gmlc::libguarded::ordered_guarded<T, std::mutex>;

template<class T>
using shared_guarded = gmlc::libguarded::shared_guarded<T, std::shared_mutex>;

template<class T>
using shared_guarded_opt = gmlc::libguarded::shared_guarded_opt<T, std::shared_mutex>;

template<class T>
using ordered_guarded = gmlc::libguarded::ordered_guarded<T, std::shared_mutex>;
