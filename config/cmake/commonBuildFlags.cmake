# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2026, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Energy Innovation LLC.
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

# GNU's static runtime archives otherwise contribute globally visible symbols to Linux C++ shared
# libraries. Keep their own exported C++ APIs visible.
function(helics_hide_static_cpp_runtime target)
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
       AND STATIC_STANDARD_LIB STREQUAL "static"
    )
        target_link_options(
            ${target} PRIVATE "LINKER:--exclude-libs=libstdc++.a" "LINKER:--exclude-libs=libgcc.a"
            "LINKER:--exclude-libs=libgcc_eh.a"
        )
    endif()
endfunction()

if(${PROJECT_NAME}_EMBEDDED_DEBUG_INFO STREQUAL "default")

elseif(${PROJECT_NAME}_EMBEDDED_DEBUG_INFO STREQUAL "external")
    ucm_set_embedded_debug(EXTERNAL)
else()
    ucm_set_embedded_debug(EMBEDDED)
endif()
