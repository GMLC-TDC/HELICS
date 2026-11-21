# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2026, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Energy Innovation LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

show_variable(BOOST_INSTALL_PATH PATH "Boost root directory" "${BOOST_INSTALL_PATH}")
mark_as_advanced(BOOST_INSTALL_PATH)

set(BOOST_MINIMUM_VERSION 1.83)
set(boost_roots
    ${Boost_ROOT}
    $ENV{Boost_ROOT}
    ${BOOST_ROOT}
    $ENV{BOOST_ROOT}
    ${BOOST_INSTALL_PATH}
    $ENV{BOOST_INSTALL_PATH}
)
list(FILTER boost_roots EXCLUDE REGEX "(^$|-NOTFOUND$)")

if(WIN32 AND NOT UNIX_LIKE AND NOT boost_roots)
    set(boost_versions
        boost_1_92_0
        boost_1_91_0
        boost_1_90_0
        boost_1_89_0
        boost_1_88_0
        boost_1_87_0
        boost_1_86_0
        boost_1_85_0
        boost_1_84_0
        boost_1_83_0
    )

    set(poss_prefixes
        C:
        C:/local
        C:/boost
        C:/local/boost
        C:/Libraries
        "C:/Program Files/boost"
        C:/ProgramData/chocolatey/lib
        D:
        D:/local
        D:/boost
        D:/local/boost
        D:/Libraries
    )

    foreach(boostver IN LISTS boost_versions)
        foreach(dir IN LISTS poss_prefixes)
            if(EXISTS "${dir}/${boostver}/boost/version.hpp")
                list(APPEND boost_roots "${dir}/${boostver}")
            endif()
        endforeach()
    endforeach()
endif()
list(REMOVE_DUPLICATES boost_roots)

set(boost_package_hints)
foreach(boost_root IN LISTS boost_roots)
    list(APPEND boost_package_hints "${boost_root}")
    if(MSVC AND NOT UNIX_LIKE)
        math(EXPR boost_arch "${CMAKE_SIZEOF_VOID_P} * 8")
        if(NOT BOOST_REQUIRED_LIBRARIES)
            # The MSVC binary packages under the same root share their headers.
            file(GLOB boost_msvc_paths "${boost_root}/lib${boost_arch}-msvc-*/cmake")
            list(APPEND boost_package_hints ${boost_msvc_paths})
        elseif(MSVC_VERSION GREATER_EQUAL 1930)
            list(APPEND boost_package_hints "${boost_root}/lib${boost_arch}-msvc-14.3/cmake")
        else()
            list(APPEND boost_package_hints "${boost_root}/lib${boost_arch}-msvc-14.2/cmake")
        endif()
    endif()
endforeach()

if(NOT Boost_NO_BOOST_CMAKE)
    find_package(
        Boost ${BOOST_MINIMUM_VERSION} QUIET CONFIG COMPONENTS ${BOOST_REQUIRED_LIBRARIES} HINTS
                                                               ${boost_package_hints}
    )
endif()

if(NOT Boost_FOUND AND CMAKE_VERSION VERSION_LESS 3.30)
    if(NOT BOOST_ROOT AND boost_roots)
        list(GET boost_roots 0 BOOST_ROOT)
    endif()
    find_package(Boost ${BOOST_MINIMUM_VERSION} QUIET MODULE COMPONENTS ${BOOST_REQUIRED_LIBRARIES})
endif()

if(NOT Boost_FOUND AND NOT BOOST_REQUIRED_LIBRARIES)
    find_path(
        Boost_INCLUDE_DIR
        NAMES boost/version.hpp
        HINTS ${boost_roots}
        PATH_SUFFIXES include
    )
    if(EXISTS "${Boost_INCLUDE_DIR}/boost/version.hpp" AND EXISTS
                                                           "${Boost_INCLUDE_DIR}/boost/config.hpp"
    )
        file(STRINGS "${Boost_INCLUDE_DIR}/boost/version.hpp" boost_version_line
             REGEX "^#define BOOST_VERSION [0-9]+"
        )
        if(boost_version_line MATCHES "^#define BOOST_VERSION ([0-9]+)")
            set(Boost_VERSION_MACRO "${CMAKE_MATCH_1}")
            math(EXPR Boost_VERSION_MAJOR "${Boost_VERSION_MACRO} / 100000")
            math(EXPR Boost_VERSION_MINOR "${Boost_VERSION_MACRO} / 100 % 1000")
            math(EXPR Boost_VERSION_PATCH "${Boost_VERSION_MACRO} % 100")
            set(Boost_VERSION
                "${Boost_VERSION_MAJOR}.${Boost_VERSION_MINOR}.${Boost_VERSION_PATCH}"
            )

            if(Boost_VERSION VERSION_GREATER_EQUAL BOOST_MINIMUM_VERSION)
                if(NOT TARGET Boost::headers)
                    add_library(Boost::headers INTERFACE IMPORTED)
                    target_include_directories(
                        Boost::headers SYSTEM INTERFACE "${Boost_INCLUDE_DIR}"
                    )
                endif()
                if(NOT TARGET Boost::boost)
                    add_library(Boost::boost INTERFACE IMPORTED)
                    target_link_libraries(Boost::boost INTERFACE Boost::headers)
                endif()
                set(Boost_INCLUDE_DIRS "${Boost_INCLUDE_DIR}")
                set(Boost_VERSION_STRING "${Boost_VERSION}")
                set(Boost_MINOR_VERSION "${Boost_VERSION_MINOR}")
                set(Boost_FOUND ON)
            endif()
        endif()
    endif()
endif()

if(NOT Boost_FOUND)
    message(
        FATAL_ERROR
            "Could not find Boost ${BOOST_MINIMUM_VERSION} or newer. "
            "Set BOOST_INSTALL_PATH to the Boost installation directory or Boost_DIR to the "
            "directory containing BoostConfig.cmake."
    )
endif()

set(Boost_INCLUDE_DIR "${Boost_INCLUDE_DIRS}")
message(STATUS "Found Boost ${Boost_VERSION_STRING}: ${Boost_INCLUDE_DIRS}")

# Minimum version of Boost required for building test suite
set(BOOST_VERSION_LEVEL ${Boost_MINOR_VERSION})
