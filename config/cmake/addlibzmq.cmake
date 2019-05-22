#
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details. 
#All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

#
# This file is used to add libzmq to a project
#

if(ZMQ_USE_STATIC_LIBRARY)
        set(zmq_static_build ON)
        set(zmq_shared_build OFF)
    else()
        set(zmq_static_build OFF)
        set(zmq_shared_build ON)
    endif()

include(FetchContent)

FetchContent_Declare(
  lzmq
  GIT_REPOSITORY https://github.com/zeromq/libzmq.git
  GIT_TAG        v4.3.0
)

FetchContent_GetProperties(lzmq)
string(TOLOWER "lzmq" lcName)
if(NOT ${lcName}_POPULATED)
  # Fetch the content using previously declared details
  FetchContent_Populate(lzmq)

  # Set custom variables, policies, etc.
  # ...

  set(ZMQ_BUILD_TESTS OFF)
  set(ENABLE_CURVE OFF)
  set(ENABLE_DRAFTS OFF)
  set(LIBZMQ_PEDANTIC OFF)
  set(WITH_PERF_TOOL OFF)

  # Bring the populated content into the build
  add_subdirectory(${${lcName}_SOURCE_DIR} ${${lcName}_BINARY_DIR})
endif()