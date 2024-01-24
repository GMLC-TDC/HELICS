# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2024, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# cmake script to set up build output directories

include(ucm)

# -----------------------------------------------------------------------------
# General project wide configuration for debug postfix
# -----------------------------------------------------------------------------
if(NOT NO_DEBUG_POSFIX AND NOT CMAKE_DEBUG_POSTFIX)
    set(CMAKE_DEBUG_POSTFIX d)
endif()

if(NOT TARGET compile_flags_target)
    add_library(compile_flags_target INTERFACE)
endif()

if(NOT TARGET build_flags_target)
    add_library(build_flags_target INTERFACE)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    mark_as_advanced(BUILD_TESTING)
    include(compiler_flags)

endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if(NOT USE_LIBCXX)
        show_variable(STATIC_STANDARD_LIB STRING "Link against a static standard lib" default)
        set_property(CACHE STATIC_STANDARD_LIB PROPERTY STRINGS default static dynamic)
    else()
        hide_variable(STATIC_STANDARD_LIB)
    endif()
    if(MSVC)
        show_variable(
            ${PROJECT_NAME}_EMBEDDED_DEBUG_INFO STRING "embed debug info into lib files" default
        )
        set_property(
            CACHE ${PROJECT_NAME}_EMBEDDED_DEBUG_INFO PROPERTY STRINGS default embedded external
        )
    else()
        hide_variable(${PROJECT_NAME}_EMBEDDED_DEBUG_INFO)
    endif()
endif()

if(STATIC_STANDARD_LIB STREQUAL "default")

elseif(STATIC_STANDARD_LIB STREQUAL "static")
    ucm_set_runtime(STATIC)
elseif(STATIC_STANDARD_LIB STREQUAL "dynamic")
    ucm_set_runtime(DYNAMIC)
endif()

if(${PROJECT_NAME}_EMBEDDED_DEBUG_INFO STREQUAL "default")

elseif(${PROJECT_NAME}_EMBEDDED_DEBUG_INFO STREQUAL "external")
    ucm_set_embedded_debug(EXTERNAL)
else()
    ucm_set_embedded_debug(EMBEDDED)
endif()
