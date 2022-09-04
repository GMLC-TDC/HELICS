# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2022, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

show_variable(BOOST_INSTALL_PATH PATH "Boost root directory" "${BOOST_INSTALL_PATH}")

mark_as_advanced(BOOST_INSTALL_PATH)

if(WIN32 AND NOT UNIX_LIKE)

    set(
        boost_versions
        boost_1_80_0
        boost_1_79_0
        boost_1_78_0
        boost_1_77_0
        boost_1_76_0
        boost_1_75_0
        boost_1_74_0
        boost_1_73_0
        boost_1_72_0
        boost_1_71_0
        boost_1_70_0
        boost_1_69_0
        boost_1_68_0
        boost_1_67_0
    )

    set(
        poss_prefixes
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

    find_path(
        BOOST_TEST_PATH
        NAMES boost/version.hpp
        HINTS ENV BOOST_INSTALL_PATH
        PATHS ${BOOST_INSTALL_PATH} ${boost_paths}
    )

    if(BOOST_TEST_PATH)
        set(BOOST_ROOT ${BOOST_TEST_PATH})
    endif(BOOST_TEST_PATH)
else()
    if(NOT BOOST_ROOT)
        if(BOOST_INSTALL_PATH)
            set(BOOST_ROOT "${BOOST_INSTALL_PATH}")
        elseif($ENV{BOOST_INSTALL_PATH})
            set(BOOST_ROOT "$ENV{BOOST_INSTALL_PATH}")
        else()
            set(BOOST_ROOT "$ENV{BOOST_ROOT}")
        endif()
    endif()
endif()

hide_variable(BOOST_TEST_PATH)

if(NOT BOOST_REQUIRED_LIBRARIES)
    set(BOOST_REQUIRED_LIBRARIES)
endif()

# Minimum version of Boost required for building a project
set(BOOST_MINIMUM_VERSION 1.67)

if(BOOST_REQUIRED_LIBRARIES)
    find_package(
        Boost ${BOOST_MINIMUM_VERSION}
        COMPONENTS ${BOOST_REQUIRED_LIBRARIES}
        REQUIRED
    )
else()
    find_package(Boost ${BOOST_MINIMUM_VERSION})
endif()
# Minimum version of Boost required for building test suite
set(BOOST_VERSION_LEVEL ${Boost_MINOR_VERSION})

# message(STATUS "Using Boost include files : ${Boost_INCLUDE_DIR} |")

# message(STATUS "Using Boost libraries in : ${Boost_LIBRARY_DIRS} |")

# message(STATUS "Using Boost libraries : ${Boost_LIBRARIES} |")
set(modifier,"")
foreach(loop_var ${Boost_LIBRARIES})
    if(loop_var STREQUAL "debug")
        list(INSERT modifier 0 ${loop_var})
    elseif(loop_var STREQUAL "optimized")
        list(INSERT modifier 0 ${loop_var})
    else()
        # message("Boost_LIBRARIES ${loop_var}")
        if(loop_var MATCHES "unit_test")
            list(APPEND Boost_LIBRARIES_test ${modifier} ${loop_var})
        else()
            list(APPEND Boost_LIBRARIES_core ${modifier} ${loop_var})
        endif()
        if("${modifier}" STREQUAL "debug")
            if(loop_var MATCHES "unit_test")
                list(APPEND Boost_LIBRARIES_test_debug ${loop_var})
            else()
                list(APPEND Boost_LIBRARIES_core_debug ${loop_var})
            endif()
        else()
            if(loop_var MATCHES "unit_test")
                list(APPEND Boost_LIBRARIES_test_release ${loop_var})
            else()
                list(APPEND Boost_LIBRARIES_core_release ${loop_var})
            endif()
        endif()
        list(LENGTH modifier modifier_size)
        if(modifier_size GREATER 0)
            list(REMOVE_AT modifier -1)
        endif()
    endif()
endforeach(loop_var)

#
# Add boost targets to use
#

if(${Boost_USE_STATIC_LIBS})
    if(NOT TARGET Boostlibs::core)
        add_library(Boostlibs::core STATIC IMPORTED)
    endif()
    if(NOT TARGET Boostlibs::test)
        add_library(Boostlibs::test STATIC IMPORTED)
    endif()
else()
    if(NOT TARGET Boostlibs::core)
        add_library(Boostlibs::core UNKNOWN IMPORTED)
    endif()
    if(NOT TARGET Boostlibs::test)
        add_library(Boostlibs::test UNKNOWN IMPORTED)
    endif()
    # if(MINGW) set_property(TARGET Boostlibs::core PROPERTY
    # INTERFACE_COMPILE_DEFINTIONS BOOST_USE_WINDOWS_H) endif()
endif()

list(LENGTH Boost_LIBRARIES_core_debug core_debug_size)
list(LENGTH Boost_LIBRARIES_core_release core_release_size)

math(EXPR rng "${core_release_size} - 1")

if(core_debug_size EQUAL 0)
    list(GET Boost_LIBRARIES_core_release 0 first_lib)
    set_target_properties(Boostlibs::core PROPERTIES IMPORTED_LOCATION ${first_lib})

    foreach(item RANGE 1 ${rng})
        list(GET Boost_LIBRARIES_core_release ${item} next_lib)
        string(RANDOM LENGTH 7 rand_name)
        if(Boost_USE_STATIC_LIBS)
            add_library(Boostlibs::${rand_name} STATIC IMPORTED)
        else()
            add_library(Boostlibs::${rand_name} UNKNOWN IMPORTED)
        endif()
        set_target_properties(
            Boostlibs::${rand_name}
            PROPERTIES IMPORTED_LOCATION ${next_lib}
        )
        list(APPEND boost_core_deps Boostlibs::${rand_name})
    endforeach()
else()
    list(GET Boost_LIBRARIES_core_release 0 first_lib_r)
    list(GET Boost_LIBRARIES_core_debug 0 first_lib_d)
    set_target_properties(
        Boostlibs::core
        PROPERTIES
            IMPORTED_LOCATION_DEBUG ${first_lib_d} IMPORTED_LOCATION_RELEASE
            ${first_lib_r}
    )

    foreach(item RANGE 1 ${rng})
        list(GET Boost_LIBRARIES_core_release ${item} next_lib_r)
        list(GET Boost_LIBRARIES_core_debug ${item} next_lib_d)
        string(RANDOM LENGTH 7 rand_name)
        if(Boost_USE_STATIC_LIBS)
            add_library(Boostlibs::${rand_name} STATIC IMPORTED)
        else()
            add_library(Boostlibs::${rand_name} UNKNOWN IMPORTED)
        endif()
        set_target_properties(
            Boostlibs::${rand_name}
            PROPERTIES
                IMPORTED_LOCATION_DEBUG ${next_lib_d} IMPORTED_LOCATION_RELEASE
                ${next_lib_r}
        )
        list(APPEND boost_core_deps Boostlibs::${rand_name})
    endforeach()
endif()

if(Boost_INCLUDE_DIR)
    set_target_properties(
        Boostlibs::core Boostlibs::test
        PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR}
    )
endif()
if(BOOST_REQUIRED_LIBRARIES)
    set_target_properties(
        Boostlibs::core
        PROPERTIES INTERFACE_LINK_LIBRARIES "${boost_core_deps}"
    )

    if(Boost_LIBRARIES_test_debug)
        set_target_properties(
            Boostlibs::test
            PROPERTIES
                IMPORTED_LOCATION_DEBUG "${Boost_LIBRARIES_test_debug}"
                IMPORTED_LOCATION_RELEASE "${Boost_LIBRARIES_test_release}"
        )
    else()
        set_target_properties(
            Boostlibs::test
            PROPERTIES IMPORTED_LOCATION "${Boost_LIBRARIES_test_release}"
        )
    endif()
endif()
# message(STATUS "Using Boost core debug libraries : ${Boost_LIBRARIES_core_debug}")
# message(STATUS "Using Boost core release libraries : ${Boost_LIBRARIES_core_release}")
