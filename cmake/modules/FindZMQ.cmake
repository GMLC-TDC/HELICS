##############################################################################
#Copyright (C) 2017, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################
# Find the ZMQ includes and library
# 
# This module defines
# ZMQ_INCLUDE_DIR, where to find zmq.h
# ZMQ_LIBRARY, the library needed to use ZMQ
# ZMQ_FOUND, if false, you cannot build anything that requires ZMQ.
# ZMQ_SHARED_LIB the shared library that needs to be associated with the executable
set(ZMQ_FOUND 0)

  set(ZMQ_REGISTRY_PATH
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\ZeroMQ (x64);DisplayIcon]"
    )

# this is to deal with something weird when specifying the install path from an external source

if ("${ZMQ_INSTALL_PATH}" STREQUAL "")
set(ZMQ_PATH2 "")
else()
STRING(REPLACE "?" "" ZMQ_PATH2 ${ZMQ_INSTALL_PATH})
endif()

find_path(ZMQ_ROOT_DIR
  NAMES
    include/zmq.h
  HINTS
    ${ZMQ_REGISTRY_PATH}
	${ZMQ_INSTALL_PATH}
	${ZMQ_INCLUDE_PATH}
	${ZMQ_PATH2}
  PATHS
    /usr
    /usr/local
	
)
find_path(ZMQ_INCLUDE_DIR zmq.h ${ZMQ_ROOT_DIR}/include)
if (MSVC)
  # Read registry key holding version
    get_filename_component(ZMQ_NAME "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\ZeroMQ (x64);DisplayVersion]" NAME)
 
  # Replace dots with underscores
  string(REGEX REPLACE "\\." "_" ZMQ_NAME ${ZMQ_NAME})
  # Get Visual studio version number
  
 
  #message(STATUS "toolset =${CMAKE_VS_PLATFORM_TOOLSET}")

  if (${ZMQ_NAME} MATCHES "registry") # if key was not found, the string "registry" is returned
    set(_ZMQ_VERSIONS "4_2_2" "4_2_1" "4_2_0" "4_1_5" "4_1_4" "4_0_4" "4_0_3" "4_0_2" "4_0_1" "4_0_0")
    set(ZMQ_LIBRARY_NAME)
	
		foreach(ver ${_ZMQ_VERSIONS})
				list(APPEND ZMQ_LIBRARY_NAME "libzmq-${CMAKE_VS_PLATFORM_TOOLSET}-mt-${ver}")
		endforeach()
  else()
    # Format ZMQ library file name
	
		foreach(vs ${_VS_VERSIONS})
			set(ZMQ_LIBRARY_NAME "libzmq-v${CMAKE_VS_PLATFORM_TOOLSET}-mt-${ZMQ_NAME}")
		endforeach()
  endif()
endif()
find_library(ZMQ_LIBRARY
  NAMES
    zmq
	libzmq
    ${ZMQ_LIBRARY_NAME}
HINTS
	"${ZMQ_ROOT_DIR}/lib"
	"${ZMQ_INSTALL_PATH}/lib"
	${ZMQ_PATH2}/lib
	"${ZMQ_LIBRARY_PATH}"
  PATHS
    /lib
    /usr/lib
    /usr/local/lib
    
)
if (ZMQ_INCLUDE_DIR AND ZMQ_LIBRARY AND NOT ZMQ_LIBRARY-NOTFOUND)
  set(ZMQ_FOUND 1)
  message(STATUS "Found ZMQ library: ${ZMQ_LIBRARY}")
  message(STATUS "Found ZMQ headers: ${ZMQ_INCLUDE_DIR}")
else()
IF(NOT ZMQ_FIND_QUIETLY)
  message(SEND_ERROR "Could not find ZMQ libraries/headers! Please install ZMQ with libraries and headers")
  ENDIF(NOT ZMQ_FIND_QUIETLY)
endif()
# show the ZMQ_INCLUDE_DIR and ZMQ_LIBRARY variables only in the advanced view
mark_as_advanced(ZMQ_ROOT_DIR ZMQ_INCLUDE_DIR ZMQ_LIBRARY ZMQ_FOUND)
