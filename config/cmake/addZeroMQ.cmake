# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# file to include ZMQ
option(${PROJECT_NAME}_USE_SYSTEM_ZEROMQ_ONLY
       "only search for system zeromq libraries, bypass local build options" OFF)

mark_as_advanced(${PROJECT_NAME}_USE_SYSTEM_ZEROMQ_ONLY)

if(MSVC)
    cmake_dependent_option(
        ${PROJECT_NAME}_ZMQ_SUBPROJECT
        "enable ZMQ to automatically download and include as a subproject"
        ON
        "NOT ${PROJECT_NAME}_USE_SYSTEM_ZEROMQ_ONLY"
        OFF
    )
else()
    cmake_dependent_option(
        ${PROJECT_NAME}_ZMQ_SUBPROJECT
        "enable ZMQ to automatically download and include as a subproject"
        OFF
        "NOT ${PROJECT_NAME}_USE_SYSTEM_ZEROMQ_ONLY"
        OFF
    )
endif()
cmake_dependent_option(
    ${PROJECT_NAME}_ZMQ_FORCE_SUBPROJECT
    "force ZMQ to automatically download and include as a subproject"
    OFF
    "NOT ${PROJECT_NAME}_USE_SYSTEM_ZEROMQ_ONLY"
    OFF
)

mark_as_advanced(${PROJECT_NAME}_USE_SYSTEM_ZEROMQ_ONLY)
mark_as_advanced(${PROJECT_NAME}_ZMQ_SUBPROJECT)
mark_as_advanced(${PROJECT_NAME}_ZMQ_FORCE_SUBPROJECT)

# Add option after seeing if only the static ZMQ library is available
# Using a macro so the option() command appears near other option() commands
macro(add_zmq_static_lib_option)
    if(ZeroMQ_STATIC_LIBRARY AND NOT ZeroMQ_LIBRARY)
        option(${PROJECT_NAME}_USE_ZMQ_STATIC_LIBRARY "use the ZMQ static library" ON)
    else()
        option(${PROJECT_NAME}_USE_ZMQ_STATIC_LIBRARY "use the ZMQ static library" OFF)
    endif()
    mark_as_advanced(${PROJECT_NAME}_USE_ZMQ_STATIC_LIBRARY)
endmacro()

# flag that zeromq headers are required
set(ZeroMQ_REQUIRE_HEADERS ON)

if(${PROJECT_NAME}_USE_SYSTEM_ZEROMQ_ONLY)
    find_package(ZeroMQ)
    set(${PROJECT_NAME}_ZMQ_LOCAL_BUILD OFF CACHE INTERNAL "")
elseif(${PROJECT_NAME}_ZMQ_FORCE_SUBPROJECT)
    include(addlibzmq)
else()

    show_variable(ZeroMQ_INSTALL_PATH PATH "path to the zmq libraries" "")

    mark_as_advanced(ZeroMQ_INSTALL_PATH)

    set(ZMQ_CMAKE_SUFFIXES cmake/ZeroMQ cmake CMake/ZeroMQ lib/cmake)

    if(WIN32 AND NOT MSYS)
        find_package(
            ZeroMQ
            QUIET
            HINTS
            ${ZeroMQ_INSTALL_PATH}
            $ENV{ZeroMQ_INSTALL_PATH}
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
        set(ZeroMQ_FIND_QUIETLY ON)
        find_package(ZeroMQ)
        if(NOT ZeroMQ_FOUND)
            if(${PROJECT_NAME}_ZMQ_SUBPROJECT)
                include(addlibzmq)
                hide_variable(ZeroMQ_DEBUG_LIBRARY)
                hide_variable(ZeroMQ_LIBRARY)
                hide_variable(ZeroMQ_ROOT_DIR)
                hide_variable(ZeroMQ_STATIC_LIBRARY)
                hide_variable(ZeroMQ_INCLUDE_DIR)
            else()
                show_variable(ZeroMQ_DEBUG_LIBRARY FILEPATH
                              "path to the ZeroMQ debug library" "")
                show_variable(ZeroMQ_LIBRARY FILEPATH "path to the ZeroMQ library" "")
                show_variable(ZeroMQ_ROOT_DIR PATH "path to the ZeroMQ root directory"
                              "")
                if(${PROJECT_NAME}_USE_ZMQ_STATIC_LIBRARY)
                    show_variable(ZeroMQ_STATIC_LIBRARY FILEPATH
                                  "path to the ZeroMQ static library" "")
                endif()
                show_variable(ZeroMQ_INCLUDE_DIR PATH
                              "path to the ZeroMQ include directory" "")
            endif()
        else()
            set(${PROJECT_NAME}_ZMQ_LOCAL_BUILD OFF CACHE INTERNAL "")
        endif()
    endif()

endif() # ${PROJECT_NAME}_USE_SYSTEM_ZEROMQ_ONLY
hide_variable(ZeroMQ_DIR)

add_zmq_static_lib_option()

if(WIN32 AND NOT MSYS AND NOT CYGWIN)
    if(TARGET libzmq)
        install(
            FILES $<TARGET_FILE:libzmq>
            DESTINATION ${CMAKE_INSTALL_BINDIR}
            COMPONENT Runtime
        )
    endif()
endif()
