# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)

add_subdirectory(helics)

if(NOT SWIG_GENERATE_INTERFACE_FILES_ONLY)
if(BUILD_PYTHON_INTERFACE)
  add_subdirectory(python)
endif()

if(BUILD_JAVA_INTERFACE)
  add_subdirectory(java)
endif()

if(BUILD_MATLAB_INTERFACE)
  add_subdirectory(matlab)
endif()

if(BUILD_OCTAVE_INTERFACE)
   add_subdirectory(octave)
endif()

if(BUILD_CSHARP_INTERFACE)
   #add_subdirectory(csharp)
endif()

endif(NOT SWIG_GENERATE_INTERFACE_FILES_ONLY)

else()
	cmake_minimum_required(VERSION 3.4)
	set(HELICS_BUILD_TESTS ON CACHE BOOL "")
	set(BUILD_PYTHON_INTERFACE OFF CACHE BOOL "")
	set(BUILD_PYTHON2_INTERFACE OFF CACHE BOOL "")
	set(BUILD_MATLAB_INTERFACE OFF CACHE BOOL "")
	set(BUILD_JAVA_INTERFACE OFF CACHE BOOL "")
	set(BUILD_CSHARP_INTERFACE OFF CACHE BOOL "")
	set(HELICS_BUILD_EXAMPLES OFF CACHE BOOL "")
	set(HELICS_BUILD_BENCHMARKS OFF CACHE BOOL "")
	#now call into the root directory
	add_subdirectory(.. ${CMAKE_CURRENT_BINARY_DIR}/root/)
endif()

