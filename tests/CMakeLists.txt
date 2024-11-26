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

    add_subdirectory(helics)

    if(NOT HELICS_SWIG_GENERATE_INTERFACE_FILES_ONLY)

        if(HELICS_BUILD_JAVA_INTERFACE)
            add_subdirectory(java)
        endif()

        if(HELICS_BUILD_CSHARP_INTERFACE)
            # add_subdirectory(csharp)
        endif()

    endif(NOT HELICS_SWIG_GENERATE_INTERFACE_FILES_ONLY)

else()
    cmake_minimum_required(VERSION 3.22...3.31)

    set(HELICS_BUILD_TESTS ON CACHE INTERNAL "")
    set(HELICS_BUILD_JAVA_INTERFACE OFF CACHE INTERNAL "")
    set(HELICS_BUILD_CSHARP_INTERFACE OFF CACHE INTERNAL "")
    set(HELICS_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(HELICS_BUILD_BENCHMARKS OFF CACHE INTERNAL "")
    # now call into the root directory
    add_subdirectory(.. ${CMAKE_CURRENT_BINARY_DIR}/root/)
endif()
