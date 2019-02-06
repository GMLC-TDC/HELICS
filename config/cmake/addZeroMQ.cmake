#
#Copyright (c) 2017-2019,
#Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#

# file to include ZMQ
option(
    USE_SYSTEM_ZEROMQ_ONLY
    "only search for system zeromq libraries, bypass autobuild option" OFF
)

mark_as_advanced(USE_SYSTEM_ZEROMQ_ONLY)

cmake_dependent_option(
    ZMQ_USE_STATIC_LIBRARY
    "use the ZMQ static library"
    OFF
    "NOT USE_SYSTEM_ZEROMQ_ONLY"
    OFF
)

mark_as_advanced(ZMQ_USE_STATIC_LIBRARY)

if(USE_SYSTEM_ZEROMQ_ONLY)
    find_package(ZeroMQ)
else()

    show_variable(
        ZeroMQ_INSTALL_PATH
        PATH
        "path to the zmq libraries"
        "${AUTOBUILD_INSTALL_PATH}"
    )

    mark_as_advanced(ZeroMQ_INSTALL_PATH)

    set(ZMQ_CMAKE_SUFFIXES cmake/ZeroMQ cmake CMake/ZeroMQ lib/cmake)

    if(WIN32 AND NOT MSYS)
        find_package(
            ZeroMQ
            QUIET
            HINTS
            ${ZeroMQ_INSTALL_PATH}
            $ENV{ZeroMQ_INSTALL_PATH}
            ${AUTOBUILD_INSTALL_PATH}
            PATH_SUFFIXES
            ${ZMQ_CMAKE_SUFFIXES}
        )
    else()
        find_package(
            ZeroMQ
            QUIET
            HINTS
            ${ZeroMQ_INSTALL_PATH}
            $ENV{ZeroMQ_INSTALL_PATH}
            ${AUTOBUILD_INSTALL_PATH}
            PATH_SUFFIXES
            ${ZMQ_CMAKE_SUFFIXES}
            NO_SYSTEM_ENVIRONMENT_PATH
            NO_CMAKE_PACKAGE_REGISTRY
            NO_CMAKE_SYSTEM_PATH
            NO_CMAKE_SYSTEM_PACKAGE_REGISTRY
        )
    endif()

    if(NOT ZeroMQ_FOUND)
        # message(STATUS "initialZMQ not found")
        if(ZMQ_USE_STATIC_LIBRARY OR AUTOBUILD_ZMQ)
            include(buildlibZMQ)
            build_libzmq()
            find_package(
                ZeroMQ
                HINTS
                ${ZeroMQ_INSTALL_PATH}
                $ENV{ZeroMQ_INSTALL_PATH}
                ${AUTOBUILD_INSTALL_PATH}
                PATH_SUFFIXES
                ${ZMQ_CMAKE_SUFFIXES}
            )
        else()
            set(ZeroMQ_FIND_QUIETLY ON)
            find_package(ZeroMQ)
            if(NOT ZeroMQ_FOUND)
                if(WIN32 AND NOT MSYS)
                    option(
                        AUTOBUILD_ZMQ
                        "enable ZMQ to automatically download and build" ON
                    )
                else()
                    option(
                        AUTOBUILD_ZMQ
                        "enable ZMQ to automatically download and build" OFF
                    )
                endif()
                if(AUTOBUILD_ZMQ)
                    include(buildlibZMQ)
                    build_libzmq()
                    find_package(
                        ZeroMQ
                        HINTS
                        ${ZeroMQ_INSTALL_PATH}
                        $ENV{ZeroMQ_INSTALL_PATH}
                        ${AUTOBUILD_INSTALL_PATH}
                        PATH_SUFFIXES
                        ${ZMQ_CMAKE_SUFFIXES}
                    )
                endif(AUTOBUILD_ZMQ)
            endif()
        endif()
    endif()

endif() # USE_SYSTEM_ZEROMQ_ONLY
hide_variable(ZeroMQ_DIR)
