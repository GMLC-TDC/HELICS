#
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details. 
#All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

option(
    ENABLE_EXTRA_COMPILER_WARNINGS
    "disable compiler warning for ${CMAKE_PROJECT_NAME} build" ON
)
option(
    ENABLE_ERROR_ON_WARNINGS
    "generate a compiler error for any warning encountered" OFF
)

mark_as_advanced(ENABLE_EXTRA_COMPILER_WARNINGS)
mark_as_advanced(ENABLE_ERROR_ON_WARNINGS)

# -------------------------------------------------------------
# Setup compiler options and configurations
# -------------------------------------------------------------
message(STATUS "setting up for ${CMAKE_CXX_COMPILER_ID}")
if (NOT TARGET compile_flags_target)
add_library(compile_flags_target INTERFACE)
endif()

if(UNIX)

    if(ENABLE_ERROR_ON_WARNINGS)
        target_compile_options(compile_flags_target INTERFACE -Werror)
    endif(ENABLE_ERROR_ON_WARNINGS)

    if(ENABLE_EXTRA_COMPILER_WARNINGS)
        target_compile_options(compile_flags_target INTERFACE -Wall -pedantic)
        target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wextra>)
        target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wshadow>)
        target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wstrict-aliasing=1>)
        target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wunreachable-code>)
        target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual>)
        # target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wredundant-decls>)
        target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wundef>)
        if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
            # this produces a lot of noise on newer compilers
            # target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wstrict-overflow=5>)
            target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wcast-align>)
        endif()
        # this options produces lots of warning but is useful for checking every
        # once in a while with Clang, GCC warning notices with this aren't as
        # useful target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wpadded>) add
        # some gnu specific options if the compiler is newer
        if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
            # this option produces a number of warnings in third party libraries
            # but useful for checking for any internal usages
            # target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast>)
            target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wlogical-op>)
            if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0)
                target_compile_options(compile_flags_target INTERFACE 
                    $<$<COMPILE_LANGUAGE:CXX>:-Wduplicated-cond>
                )
                target_compile_options(compile_flags_target INTERFACE 
                    $<$<COMPILE_LANGUAGE:CXX>:-Wnull-dereference>
                )
            endif()
            if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0)
                target_compile_options(compile_flags_target INTERFACE 
                    $<$<COMPILE_LANGUAGE:CXX>:-Wimplicit-fallthrough=2>
                )
            endif()
            if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.9)
                target_compile_options(compile_flags_target INTERFACE 
                    $<$<COMPILE_LANGUAGE:CXX>:-Wclass-memaccess>
                )
            endif()
        endif()
		if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0)
                target_compile_options(compile_flags_target INTERFACE -Wdocumentation -Wno-documentation-deprecated-sync)
		    endif()
		endif()
    endif(ENABLE_EXTRA_COMPILER_WARNINGS)
    option(USE_BOOST_STATIC_LIBS "Build using boost static Libraries" OFF)
    option(USE_LIBCXX "Use Libc++ vs as opposed to the default" OFF)
    mark_as_advanced(USE_LIBCXX)
	#this is a global option on all parts
    if(USE_LIBCXX)
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>)
        link_libraries("-stdlib=libc++")
    endif(USE_LIBCXX)
else(UNIX)
    if(MINGW)
        if(MSYS)
            option(USE_LIBCXX "Use Libc++ vs as opposed to the default" OFF)
            mark_as_advanced(USE_LIBCXX)
            if(USE_LIBCXX)
                add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>)
                link_libraries("-stdlib=libc++")
				link_libraries("c++abi")
            endif(USE_LIBCXX)
        endif(MSYS)
        if(ENABLE_ERROR_ON_WARNINGS)
            target_compile_options(compile_flags_target INTERFACE -Werror)
        endif(ENABLE_ERROR_ON_WARNINGS)

        if(ENABLE_EXTRA_COMPILER_WARNINGS)
            target_compile_options(compile_flags_target INTERFACE -Wall -pedantic)
            target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wextra>)
            target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wshadow>)
            target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wstrict-aliasing=1>)
            target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wunreachable-code>)
            # target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wstrict-overflow=5>)
            target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual>)
            # target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wredundant-decls>)
            target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wcast-align>)
            target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wundef>)

            # this options produces lots of warning but is useful for checking
            # ever once in a while with Clang, GCC warning notices with this
            # aren't as useful
            # target_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wpadded>)
            if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
                # this option produces a number of warnings in third party
                # libraries but useful for checking for any internal usages
                # atarget_compile_options(compile_flags_target INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast>)
                add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wlogical-op>)
                if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0)
                    target_compile_options(compile_flags_target INTERFACE 
                        $<$<COMPILE_LANGUAGE:CXX>:-Wduplicated-cond>
                    )
                    target_compile_options(compile_flags_target INTERFACE 
                        $<$<COMPILE_LANGUAGE:CXX>:-Wnull-dereference>
                    )
                endif()
                if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0)
                    target_compile_options(compile_flags_target INTERFACE 
                        $<$<COMPILE_LANGUAGE:CXX>:-Wimplicit-fallthrough=2>
                    )
                endif()
            endif()
			if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0)
                target_compile_options(compile_flags_target INTERFACE -Wdocumentation -Wno-documentation-deprecated-sync)
		    endif()
		endif()
        endif(ENABLE_EXTRA_COMPILER_WARNINGS)
        option(USE_BOOST_STATIC_LIBS "Build using boost static Libraries" OFF)
    else(MINGW)
        option(USE_BOOST_STATIC_LIBS "Build using boost static Libraries" ON)
        # -------------------------------------------------------------
        # Extra definitions for visual studio
        # -------------------------------------------------------------
        if(MSVC)
            if(ENABLE_ERROR_ON_WARNINGS)
                target_compile_options(compile_flags_target INTERFACE /WX)
            endif(ENABLE_ERROR_ON_WARNINGS)

            target_compile_options(compile_flags_target INTERFACE -D_CRT_SECURE_NO_WARNINGS)
            target_compile_options(compile_flags_target INTERFACE -D_SCL_SECURE_NO_WARNINGS)
			# these next two should be global
            add_compile_options(/MP /EHsc)
            add_compile_options(/sdl)
            if(ENABLE_EXTRA_COMPILER_WARNINGS)
                target_compile_options(compile_flags_target INTERFACE 
                    -W4
                    /wd4065
                    /wd4101
                    /wd4102
                    /wd4244
                    /wd4297
                    /wd4355
                    /wd4800
                    /wd4484
                    /wd4702
                    /wd4996
                )
            endif(ENABLE_EXTRA_COMPILER_WARNINGS)
            add_definitions(-D_WIN32_WINNT=0x0601)
        endif(MSVC)
    endif(MINGW)
endif(UNIX)

# -------------------------------------------------------------
# Check and set latest CXX Standard supported by compiler
# -------------------------------------------------------------
include(CheckLatestCXXStandardOption)

mark_as_advanced(USE_BOOST_STATIC_LIBS)
