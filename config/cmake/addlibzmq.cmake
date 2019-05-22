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
if(NOT CMAKE_VERSION VERSION_LESS 3.11)
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

  set(ZMQ_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(ENABLE_CURVE OFF CACHE BOOL "" FORCE)
  set(ENABLE_DRAFTS OFF CACHE BOOL "" FORCE)
  set(LIBZMQ_PEDANTIC OFF CACHE BOOL "" FORCE)
  set(WITH_PERF_TOOL OFF CACHE BOOL "" FORCE)
  set(BUILD_STATIC ${zmq_static_build} CACHE BOOL "" FORCE)
  set(BUILD_SHARED ${zmq_shared_build} CACHE BOOL "" FORCE)

  # Bring the populated content into the build
  add_subdirectory(${${lcName}_SOURCE_DIR} ${${lcName}_BINARY_DIR})
  set(ZeroMQ_FOUND TRUE)

  set_target_properties(clang-format clang-format-check clang-format-diff PROPERTIES FOLDER "Extern/zmq_clang_format")

  if(ZMQ_USE_STATIC_LIBRARY)
  set_target_properties(libzmq-static
    PROPERTIES FOLDER "Extern")
	if (MSVC)
	  target_compile_options(libzmq-static PRIVATE "/w")
	else()
		target_compile_options(libzmq-static PRIVATE "-fPIC")
	endif()
    else()
        set_target_properties(libzmq
    PROPERTIES FOLDER "Extern")
	if (MSVC)
	  target_compile_options(libzmq PRIVATE "/w")
	endif()
    endif()

  
endif()
else()
include(buildlibZMQ)
            build_libzmq()
            find_package(
                ZeroMQ
                HINTS
                ${ZeroMQ_INSTALL_PATH}
                $ENV{ZeroMQ_INSTALL_PATH}
                ${AUTOBUILD_INSTALL_PATH}
                PATH_SUFFIXES
                ${ZMQ_CMAKE_SUFFIXES}
            )
endif()