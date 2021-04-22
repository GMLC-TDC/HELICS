# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

cmake_dependent_option(
    BUILD_PYTHON_INTERFACE "Build Python extension" OFF
    "NOT BUILD_PYTHON2_INTERFACE;NOT HELICS_DISABLE_C_SHARED_LIB" OFF
)

cmake_dependent_option(
    BUILD_PYTHON2_INTERFACE
    "Build Python2.7 extension(Requires swig and will not build if \"BUILD_PYTHON_INTERFACE\" is active)"
    OFF
    "NOT BUILD_PYTHON_INTERFACE;NOT HELICS_DISABLE_C_SHARED_LIB"
    OFF
)
cmake_dependent_option(
    BUILD_MATLAB_INTERFACE "Build Matlab Extension" OFF "NOT HELICS_DISABLE_C_SHARED_LIB" OFF
)
cmake_dependent_option(
    BUILD_OCTAVE_INTERFACE "Build Octave extension" OFF "NOT HELICS_DISABLE_C_SHARED_LIB" OFF
)
cmake_dependent_option(
    BUILD_JAVA_INTERFACE "Build Java extension" OFF "NOT HELICS_DISABLE_C_SHARED_LIB" OFF
)

# cmake projects are only available for csharp from 3.8 onward
cmake_dependent_option(
    BUILD_CSHARP_INTERFACE "Build C# extension" OFF
    "NOT HELICS_DISABLE_C_SHARED_LIB;CMAKE_VERSION VERSION_GREATER 3.8" OFF
)

# these are purposely hidden options since they will only be used for very select purposes
option(HELICS_SWIG_GENERATE_INTERFACE_FILES_ONLY
       "Use Swig to generate the interface files but not build" OFF
)
hide_variable(HELICS_SWIG_GENERATE_INTERFACE_FILES_ONLY)

option(HELICS_OVERWRITE_INTERFACE_FILES
       "have cmake automatically overwrite the interface build files" OFF
)
hide_variable(HELICS_OVERWRITE_INTERFACE_FILES)

if(BUILD_CSHARP_INTERFACE
   OR BUILD_PYTHON2_INTERFACE
   OR BUILD_OCTAVE_INTERFACE
   OR SWIG_GENERATE_INTERFACE_FILES_ONLY
)
    set(swig_required ON)
else()
    set(swig_required OFF)
endif()

# cmake-format: off
# at some point in the future (HELICS 3) switch everything to use
# find_package(Python3 ...)
cmake_dependent_advanced_option(
    HELICS_USE_NEW_PYTHON_FIND
    "Use the FindPython Routine only available from cmake 3.12 and later"
    OFF
    "CMAKE_VERSION VERSION_GREATER 3.12;NOT HELICS_SWIG_GENERATE_INTERFACE_FILES_ONLY;BUILD_PYTHON_INTERFACE OR BUILD_PYTHON2_INTERFACE"
    OFF
)

cmake_dependent_option(
    HELICS_ENABLE_SWIG
    "use swig to generate the interface files"
    OFF
    "BUILD_PYTHON_INTERFACE OR BUILD_MATLAB_INTERFACE OR BUILD_JAVA_INTERFACE;NOT swig_required"
    ${swig_required}
)
# cmake-format: on

if(HELICS_ENABLE_SWIG OR swig_required)
    if(WIN32 AND NOT MSYS)
        if(NOT CMAKE_PROGRAM_PATH)
            set(CMAKE_PROGRAM_PATH
                "C:/ProgramData/chocolatey/bin"
                "C:/local/swigwin-4.0.2"
                "C:/local/swigwin-4.0.1"
                "C:/local/swigwin-4.0.0"
                "C:/local/swigwin-3.0.12"
                "C:/local/swigwin-3.0.11"
                "C:/local/swigwin-3.0.10"
            )
        else()
            set(CMAKE_PROGRAM_PATH
                "${CMAKE_PROGRAM_PATH}"
                "C:/ProgramData/chocolatey/bin"
                "C:/local/swigwin-4.0.2"
                "C:/local/swigwin-4.0.1"
                "C:/local/swigwin-4.0.0"
                "C:/local/swigwin-3.0.12"
                "C:/local/swigwin-3.0.11"
                "C:/local/swigwin-3.0.10"
            )
        endif()
    endif()
    find_package(SWIG 3 REQUIRED)
    include(${SWIG_USE_FILE})
endif()

# add base swig interface files as install option, regardless of current interfaces built
if(NOT HELICS_BINARY_ONLY_INSTALL AND NOT HELICS_DISABLE_C_SHARED_LIB)
    install(FILES helics.i DESTINATION ${CMAKE_INSTALL_DATADIR}/helics/swig COMPONENT swig)
endif()

# cmake-format: off
# add a helper variable for standalone interface swig builds
set(HELICS_SWIG_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" "${HELICS_SOURCE_DIR}/src/helics/shared_api_library" "${HELICS_BINARY_DIR}/src/helics/shared_api_library")
# cmake-format: on

# TODO: remove this after all of the interfaces work as standalone builds
include_directories(${HELICS_SWIG_INCLUDE_DIRS})

if(BUILD_PYTHON_INTERFACE OR BUILD_PYTHON2_INTERFACE)
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
    add_subdirectory(csharp)
endif()
