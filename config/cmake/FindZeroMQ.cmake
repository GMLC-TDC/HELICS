##############################################################################
# Copyright © 2017-2018,
# Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################
# Find the ZeroMQ includes and library
#
# This module defines
# ZeroMQ_INCLUDE_DIR, where to find zmq.h
# ZeroMQ_LIBRARY, the library needed to use ZeroMQ
# ZeroMQ_FOUND, if false, you cannot build anything that requires ZeroMQ.
# ZeroMQ_SHARED_LIB the shared library that needs to be associated with the executable
set(ZeroMQ_FOUND 0)

  set(ZeroMQ_REGISTRY_PATH
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\ZeroMQ (x64);DisplayIcon]"
    )

# this is to deal with something weird when specifying the install path from an external source

if ("${ZeroMQ_INSTALL_PATH}" STREQUAL "")
set(ZeroMQ_PATH2 "")
else()
STRING(REPLACE "?" "" ZeroMQ_PATH2 ${ZeroMQ_INSTALL_PATH})
endif()

find_path(ZeroMQ_ROOT_DIR
  NAMES
    include/zmq.h
  HINTS
    ${ZeroMQ_REGISTRY_PATH}
	${ZeroMQ_INSTALL_PATH}
	${ZeroMQ_INCLUDE_PATH}
	${ZeroMQ_PATH2}
  PATHS
    /usr
    /usr/local

)
find_path(ZeroMQ_INCLUDE_DIR zmq.h ${ZeroMQ_ROOT_DIR}/include)
if (MSVC)
  # Read registry key holding version
    get_filename_component(ZeroMQ_NAME "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\ZeroMQ (x64);DisplayVersion]" NAME)

  # Replace dots with underscores
  string(REGEX REPLACE "\\." "_" ZeroMQ_NAME ${ZeroMQ_NAME})
  # Get Visual studio version number


  #message(STATUS "toolset =${CMAKE_VS_PLATFORM_TOOLSET}")

  if (${ZeroMQ_NAME} MATCHES "registry") # if key was not found, the string "registry" is returned
    set(_ZeroMQ_VERSIONS "4_2_5" "4_2_3" "4_2_2" "4_2_1" "4_2_0" "4_1_5" "4_1_4" "4_0_4" "4_0_3" "4_0_2" "4_0_1" "4_0_0")
    set(ZeroMQ_LIBRARY_NAME)

		foreach(ver ${_ZeroMQ_VERSIONS})
				list(APPEND ZeroMQ_LIBRARY_NAME "libzmq-${CMAKE_VS_PLATFORM_TOOLSET}-mt-${ver}")
		endforeach()
  else()
    # Format ZeroMQ library file name

		foreach(vs ${_VS_VERSIONS})
			set(ZeroMQ_LIBRARY_NAME "libzmq-v${CMAKE_VS_PLATFORM_TOOLSET}-mt-${ZeroMQ_NAME}")
		endforeach()
  endif()
endif()
find_library(ZeroMQ_LIBRARY
  NAMES
    zmq
	libzmq
    ${ZeroMQ_LIBRARY_NAME}
HINTS
	"${ZeroMQ_ROOT_DIR}/lib"
	"${ZeroMQ_INSTALL_PATH}/lib"
	${ZeroMQ_PATH2}/lib
	"${ZeroMQ_LIBRARY_PATH}"
  PATHS
    /lib
    /usr/lib
    /usr/local/lib

)
if (ZeroMQ_INCLUDE_DIR AND ZeroMQ_LIBRARY AND NOT ZeroMQ_LIBRARY-NOTFOUND)
  set(ZeroMQ_FOUND 1)
  message(STATUS "Found ZeroMQ library: ${ZeroMQ_LIBRARY}")
  message(STATUS "Found ZeroMQ headers: ${ZeroMQ_INCLUDE_DIR}")
  #this is static because we are pointing to the library for the linker, not the shared object
  add_library(libzmq STATIC IMPORTED)
	set_target_properties(libzmq PROPERTIES IMPORTED_LOCATION "${ZeroMQ_LIBRARY}")
	set_target_properties(libzmq PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${ZeroMQ_INCLUDE_DIR}")
else()
IF(NOT ZeroMQ_FIND_QUIETLY)
  message(SEND_ERROR "Could not find ZeroMQ libraries/headers! Please install ZeroMQ with libraries and headers")
  ENDIF(NOT ZeroMQ_FIND_QUIETLY)
endif()
# show the ZeroMQ_INCLUDE_DIR and ZeroMQ_LIBRARY variables only in the advanced view
mark_as_advanced(ZeroMQ_ROOT_DIR ZeroMQ_INCLUDE_DIR ZeroMQ_LIBRARY ZeroMQ_FOUND)


