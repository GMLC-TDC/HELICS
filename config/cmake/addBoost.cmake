# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2024, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

show_variable(BOOST_INSTALL_PATH PATH "Boost root directory" "${BOOST_INSTALL_PATH}")

mark_as_advanced(BOOST_INSTALL_PATH)

if(WIN32 AND NOT UNIX_LIKE)

    if(MSVC_VERSION GREATER_EQUAL 1930)
        if(CMAKE_SIZE_OF_VOID_P EQUAL 4)
            set(BOOST_MSVC_LIB_PATH lib32-msvc-14.3)
        else()
            set(BOOST_MSVC_LIB_PATH lib64-msvc-14.3)
        endif()
    else()
        if(CMAKE_SIZE_OF_VOID_P EQUAL 4)
            set(BOOST_MSVC_LIB_PATH lib32-msvc-14.2)
        else()
            set(BOOST_MSVC_LIB_PATH lib64-msvc-14.2)
        endif()
    endif()
    set(boost_versions
        boost_1_86_0
        boost_1_85_0
        boost_1_84_0
        boost_1_83_0
        boost_1_82_0
        boost_1_81_0
        boost_1_80_0
        boost_1_79_0
        boost_1_78_0
        boost_1_77_0
        boost_1_76_0
        boost_1_75_0
        boost_1_74_0
        boost_1_73_0
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

    # create an empty list
    list(APPEND boost_paths "")
    mark_as_advanced(BOOST_INSTALL_PATH)
    foreach(boostver ${boost_versions})
        foreach(dir ${poss_prefixes})
            if(IS_DIRECTORY ${dir}/${boostver})
                if(EXISTS ${dir}/${boostver}/boost/version.hpp)
                    list(APPEND boost_paths ${dir}/${boostver})
                endif()
            endif()
        endforeach()
    endforeach()
    message(STATUS "${boost_paths}")
    find_path(
        BOOST_TEST_PATH
        NAMES boost/version.hpp
        HINTS ENV BOOST_INSTALL_PATH
        PATHS ${BOOST_INSTALL_PATH} ${boost_paths}
    )
    if(BOOST_TEST_PATH)
        if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.30)
            find_path(
                BOOST_CMAKE_PATH
                NAMES BoostConfig.cmake
                PATHS ${BOOST_TEST_PATH}/${BOOST_MSVC_LIB_PATH}/cmake
                PATH_SUFFIXES
                    Boost-1.86.0
                    Boost-1.85.0
                    Boost-1.84.0
                    Boost-1.83.0
                    Boost-1.82.0
                    Boost-1.81.0
                    Boost-1.80.0
                    Boost-1.79.0
                    Boost-1.78.0
                    Boost-1.77.0
                    Boost-1.76.0
                    Boost-1.75.0
                    Boost-1.74.0
                    Boost-1.73.0
            )
            set(Boost_ROOT ${BOOST_CMAKE_PATH})
        else()
            set(Boost_ROOT ${BOOST_TEST_PATH})
        endif()
    endif(BOOST_TEST_PATH)
endif()
# Minimum version of Boost required for building a project

if(NOT Boost_ROOT)
    message(STATUS "Boost root not found ${Boost_ROOT}")
    if(BOOST_INSTALL_PATH)
        set(Boost_ROOT "${BOOST_INSTALL_PATH}")
    elseif($ENV{BOOST_INSTALL_PATH})
        set(Boost_ROOT "$ENV{BOOST_INSTALL_PATH}")
    else()
        set(Boost_ROOT "$ENV{BOOST_ROOT}")
    endif()
endif()

if(NOT BOOST_ROOT)
    set(BOOST_ROOT ${Boost_ROOT})
    message(STATUS "setting BOOST_ROOT ${BOOST_ROOT}")
endif()

hide_variable(BOOST_TEST_PATH)

if(NOT BOOST_REQUIRED_LIBRARIES)
    set(BOOST_REQUIRED_LIBRARIES)
endif()

set(BOOST_MINIMUM_VERSION 1.73)

if(BOOST_REQUIRED_LIBRARIES)
    find_package(Boost ${BOOST_MINIMUM_VERSION} QUIET COMPONENTS ${BOOST_REQUIRED_LIBRARIES})
else()
    find_package(Boost ${BOOST_MINIMUM_VERSION} QUIET)
endif()

if(NOT Boost_FOUND)
    message(STATUS "in boost not found looking in ${Boost_ROOT} and ${BOOST_ROOT}")
    find_path(Boost_INCLUDE_DIR NAMES boost/version.hpp boost/config.hpp PATHS ${BOOST_ROOT}
                                                                               ${Boost_ROOT}
    )
    message(STATUS "boost Include dir = ${Boost_INCLUDE_DIR}")
    if(Boost_INCLUDE_DIR)

        set(version_file ${Boost_INCLUDE_DIR}/boost/version.hpp)
        if(EXISTS "${version_file}")
            file(STRINGS "${version_file}" contents REGEX "#define BOOST_(LIB_)?VERSION ")
            if(contents MATCHES "#define BOOST_VERSION ([0-9]+)")
                set(Boost_VERSION_MACRO "${CMAKE_MATCH_1}")
            endif()
            if(contents MATCHES "#define BOOST_LIB_VERSION \"([0-9_]+)\"")
                set(Boost_LIB_VERSION "${CMAKE_MATCH_1}")
            endif()
            math(EXPR Boost_VERSION_MAJOR "${Boost_VERSION_MACRO} / 100000")
            math(EXPR Boost_VERSION_MINOR "${Boost_VERSION_MACRO} / 100 % 1000")
            math(EXPR Boost_VERSION_PATCH "${Boost_VERSION_MACRO} % 100")

            message(STATUS "Boost VERSION ${Boost_VERSION_MACRO}")
        endif()
        if(Boost_VERSION_MINOR GREATER_EQUAL ${BOOST_MINIMUM_VERSION})
            add_library(Boost::headers INTERFACE IMPORTED)
            set_target_properties(
                Boost::headers PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES
                                          "${Boost_INCLUDE_DIR}"
            )
            add_library(Boost::boost INTERFACE IMPORTED)
            set_target_properties(Boost::boost PROPERTIES INTERFACE_LINK_LIBRARIES Boost::headers)

            set(Boost_FOUND ON)
            message(STATUS "Setting boost found to true")
        endif()

    endif()
endif()
if(NOT Boost_FOUND)
    message(FATAL_ERROR " Unable to find BOOST library")
endif()
# Minimum version of Boost required for building test suite
set(BOOST_VERSION_LEVEL ${Boost_MINOR_VERSION})
