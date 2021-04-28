# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
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
        if(BUILD_PYTHON_INTERFACE)
            add_subdirectory(python)
        endif()

        if(BUILD_JAVA_INTERFACE)
            add_subdirectory(java)
        endif()

        if(BUILD_MATLAB_INTERFACE)
            add_subdirectory(matlab)
        endif()

        if(BUILD_OCTAVE_INTERFACE)
            add_subdirectory(octave)
        endif()

        if(BUILD_CSHARP_INTERFACE)
            # add_subdirectory(csharp)
        endif()

    endif(NOT HELICS_SWIG_GENERATE_INTERFACE_FILES_ONLY)

else()
    cmake_minimum_required(VERSION 3.4)
    if(CMAKE_VERSION VERSION_LESS 3.18)
        cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
    else()
        cmake_policy(VERSION 3.18)
    endif()
    set(HELICS_BUILD_TESTS ON CACHE INTERNAL "")
    set(BUILD_PYTHON_INTERFACE OFF CACHE INTERNAL "")
    set(BUILD_PYTHON2_INTERFACE OFF CACHE INTERNAL "")
    set(BUILD_MATLAB_INTERFACE OFF CACHE INTERNAL "")
    set(BUILD_JAVA_INTERFACE OFF CACHE INTERNAL "")
    set(BUILD_CSHARP_INTERFACE OFF CACHE INTERNAL "")
    set(HELICS_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(HELICS_BUILD_BENCHMARKS OFF CACHE INTERNAL "")
    # now call into the root directory
    add_subdirectory(.. ${CMAKE_CURRENT_BINARY_DIR}/root/)
endif()
