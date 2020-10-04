# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    if(CMAKE_VERSION VERSION_LESS 3.5.1)
        message(FATAL_ERROR "Google benchmarks requires CMake version 3.5.1 or higher")
    endif()
    include(AddGooglebenchmark)
    add_subdirectory(helics)
else()
    cmake_minimum_required(VERSION 3.5.1)
    if(CMAKE_VERSION VERSION_LESS 3.18)
        cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
    else()
        cmake_policy(VERSION 3.18)
    endif()
    set(HELICS_BUILD_TESTS OFF CACHE INTERNAL "")
    set(BUILD_PYTHON_INTERFACE OFF CACHE INTERNAL "")
    set(BUILD_PYTHON2_INTERFACE OFF CACHE INTERNAL "")
    set(BUILD_MATLAB_INTERFACE OFF CACHE INTERNAL "")
    set(BUILD_JAVA_INTERFACE OFF CACHE INTERNAL "")
    set(BUILD_CSHARP_INTERFACE OFF CACHE INTERNAL "")
    set(HELICS_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(HELICS_BUILD_BENCHMARKS ON CACHE INTERNAL "")
    # now call into the root directory
    add_subdirectory(.. ${CMAKE_CURRENT_BINARY_DIR}/root/)
endif()
