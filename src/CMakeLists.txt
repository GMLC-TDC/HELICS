# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    add_subdirectory(helics)
else()
    cmake_minimum_required(VERSION 3.4)
    set(HELICS_BUILD_TESTS
        OFF
        CACHE INTERNAL ""
    )
    set(BUILD_PYTHON_INTERFACE
        OFF
        CACHE INTERNAL ""
    )
    set(BUILD_PYTHON2_INTERFACE
        OFF
        CACHE INTERNAL ""
    )
    set(BUILD_MATLAB_INTERFACE
        OFF
        CACHE INTERNAL ""
    )
    set(BUILD_JAVA_INTERFACE
        OFF
        CACHE INTERNAL ""
    )
    set(BUILD_CSHARP_INTERFACE
        OFF
        CACHE INTERNAL ""
    )
    set(HELICS_BUILD_EXAMPLES
        OFF
        CACHE INTERNAL ""
    )
    set(HELICS_BUILD_BENCHMARKS
        OFF
        CACHE INTERNAL ""
    )
    # now call into the root directory
    add_subdirectory(.. ${CMAKE_CURRENT_BINARY_DIR}/root/)
endif()
