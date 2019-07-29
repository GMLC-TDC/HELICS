#
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details. 
#All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

# file to include ZMQ
option(
    USE_SYSTEM_ZEROMQ_ONLY
    "only search for system zeromq libraries, bypass autobuild option" OFF
)

mark_as_advanced(USE_SYSTEM_ZEROMQ_ONLY)

if (WIN32 AND NOT MSYS)
cmake_dependent_option(
                        ZMQ_SUBPROJECT
                        "enable ZMQ to automatically download and include as a subproject" ON "NOT USE_SYSTEM_ZEROMQ_ONLY" OFF
                    )
		else()
			cmake_dependent_option(
                        ZMQ_SUBPROJECT
                        "enable ZMQ to automatically download and include as a subproject" OFF "NOT USE_SYSTEM_ZEROMQ_ONLY" OFF
                    )
					endif()
cmake_dependent_option(
                        ZMQ_FORCE_SUBPROJECT
                        "force ZMQ to automatically download and include as a subproject" OFF "NOT USE_SYSTEM_ZEROMQ_ONLY" OFF
                    )

mark_as_advanced(USE_SYSTEM_ZEROMQ_ONLY)
mark_as_advanced(ZMQ_SUBPROJECT)
mark_as_advanced(ZMQ_FORCE_SUBPROJECT)

option(
    ZMQ_USE_STATIC_LIBRARY
    "use the ZMQ static library"
    OFF
)

mark_as_advanced(ZMQ_USE_STATIC_LIBRARY)

#flag that zeromq headers are required
set(ZeroMQ_REQUIRE_HEADERS ON)

if(USE_SYSTEM_ZEROMQ_ONLY)
    find_package(ZeroMQ)
elseif (ZMQ_FORCE_SUBPROJECT)
    include(addlibzmq)
else()
	
    show_variable(
        ZeroMQ_INSTALL_PATH
        PATH
        "path to the zmq libraries"
        ""
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
           if(ZMQ_SUBPROJECT)
              include(addlibzmq)
           endif()
		endif()
    endif()

endif() # USE_SYSTEM_ZEROMQ_ONLY
hide_variable(ZeroMQ_DIR)

if (WIN32)
if (TARGET libzmq)
     install(FILES $<TARGET_FILE:libzmq> DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Runtime)
   endif()
endif()