# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

cmake_conditional_option(HELICS_ENABLE_EXTRA_COMPILER_WARNINGS
       "disable compiler warning for ${CMAKE_PROJECT_NAME} build" "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME")

option(HELICS_ENABLE_ERROR_ON_WARNINGS
       "generate a compiler error for any warning encountered" OFF)

mark_as_advanced(HELICS_ENABLE_EXTRA_COMPILER_WARNINGS)
mark_as_advanced(HELICS_ENABLE_ERROR_ON_WARNINGS)

# -------------------------------------------------------------
# Setup compiler options and configurations
# -------------------------------------------------------------
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
   message(STATUS "setting up for ${CMAKE_CXX_COMPILER_ID} on ${CMAKE_SYSTEM}")
endif()

if(NOT TARGET compile_flags_target)
    add_library(compile_flags_target INTERFACE)
endif()

target_compile_options(
    compile_flags_target
    INTERFACE
        $<$<CXX_COMPILER_ID:MSVC>:$<$<BOOL:${HELICS_ENABLE_ERROR_ON_WARNINGS}>:/WX>>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:$<$<BOOL:${HELICS_ENABLE_ERROR_ON_WARNINGS}>:-Werror>>
)

if(HELICS_ENABLE_EXTRA_COMPILER_WARNINGS)
    target_compile_options(
        compile_flags_target
        INTERFACE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -pedantic>
    )
    target_compile_options(
        compile_flags_target
        INTERFACE
            $<$<COMPILE_LANGUAGE:CXX>:$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wextra
            -Wshadow
            -Wstrict-aliasing=1
            -Wunreachable-code
            -Woverloaded-virtual
            -Wundef>>
    )

    target_compile_options(
        compile_flags_target
        INTERFACE $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:Clang>:-Wcast-align>>
    )
    target_compile_options(
        compile_flags_target
        INTERFACE $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:GNU>:-Wcast-align>>
    )
    # target_compile_options(compile_flags_target INTERFACE
    # $<$<COMPILE_LANGUAGE:CXX>:-Wredundant-decls>)
    # target_compile_options(compile_flags_target INTERFACE
    # $<$<COMPILE_LANGUAGE:CXX>:-Wstrict-overflow=5>)

    target_compile_options(
        compile_flags_target
        INTERFACE $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:GNU>:-Wlogical-op>>
    )
    # this option produces a number of warnings in third party libraries
    # target_compile_options(compile_flags_target INTERFACE
    # $<$<COMPILE_LANGUAGE:CXX>:$<$<CXX_COMPILER_ID:GNU>:-Wold-style-cast>>) this
    # options produces lots of warning but is useful for checking every once in a while
    # with Clang, GCC warning notices with this aren't as useful
    # target_compile_options(compile_flags_target INTERFACE
    # $<$<COMPILE_LANGUAGE:CXX>:-Wpadded>) add some gnu specific options if the compiler
    # is newer
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # this option produces a number of warnings in third party libraries but useful
        # for checking for any internal usages
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0)
            target_compile_options(
                compile_flags_target
                INTERFACE
                    $<$<COMPILE_LANGUAGE:CXX>:-Wduplicated-cond -Wnull-dereference>
            )
        endif()
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0)
            target_compile_options(
                compile_flags_target
                INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wimplicit-fallthrough=2>
            )
        endif()
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.9)
            target_compile_options(
                compile_flags_target
                INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wclass-memaccess>
            )
        endif()
    endif()
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0)
            target_compile_options(
                compile_flags_target
                INTERFACE -Wdocumentation -Wno-documentation-deprecated-sync
            )
        endif()
    endif()
endif(HELICS_ENABLE_EXTRA_COMPILER_WARNINGS)

# -------------------------------------------------------------
# Extra definitions for visual studio
# -------------------------------------------------------------
if(MSVC)

    target_compile_options(
        compile_flags_target
        INTERFACE -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS
    )
    # these next two should be global
    add_compile_options(/MP /EHsc)
    if(HELICS_ENABLE_EXTRA_COMPILER_WARNINGS)
        target_compile_options(compile_flags_target INTERFACE /W4 /sdl /wd4244)
    endif(HELICS_ENABLE_EXTRA_COMPILER_WARNINGS)
    target_compile_options(compile_flags_target INTERFACE -D_WIN32_WINNT=0x0601)
else(MSVC)
    option(USE_LIBCXX "Use Libc++ vs as opposed to the default" OFF)
    mark_as_advanced(USE_LIBCXX)
    # this is a global option on all parts
    if(USE_LIBCXX)
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>)
        link_libraries("-stdlib=libc++")
        link_libraries("c++abi")
    endif(USE_LIBCXX)
endif()

# -------------------------------------------------------------
# Check and set latest CXX Standard supported by compiler
# -------------------------------------------------------------
include(CheckLatestCXXStandardOption)
