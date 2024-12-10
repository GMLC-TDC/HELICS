# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2024, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    include(AddGooglebenchmark)
    add_subdirectory(helics)
else()
    cmake_minimum_required(VERSION 3.22...3.31)
    set(HELICS_BUILD_TESTS OFF CACHE INTERNAL "")
    set(HELICS_BUILD_JAVA_INTERFACE OFF CACHE INTERNAL "")
    set(HELICS_BUILD_CSHARP_INTERFACE OFF CACHE INTERNAL "")
    set(HELICS_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(HELICS_BUILD_BENCHMARKS ON CACHE INTERNAL "")
    # now call into the root directory
    add_subdirectory(.. ${CMAKE_CURRENT_BINARY_DIR}/root/)
endif()
