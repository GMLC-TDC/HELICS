##############################################################################
#Copyright © 2017-2018,
#Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################

if (NOT DISABLE_SWIG)
	if (WIN32 AND NOT MSYS)
		if (NOT CMAKE_PROGRAM_PATH)
			set(CMAKE_PROGRAM_PATH "C:/local/")
		endif()
	endif()
	set(CMAKE_PREFIX_PATH "swigwin-3.0.12" "swigwin-3.0.11" "swigwin-3.0.10")
	find_package(SWIG REQUIRED 3.0)
	include(${SWIG_USE_FILE})
endif()


include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories("${PROJECT_SOURCE_DIR}/src/helics/shared_api_library")
include_directories("${PROJECT_BINARY_DIR}/src/helics/shared_api_library")
	

if (BUILD_PYTHON_INTERFACE)
    add_subdirectory(python)
	if( BUILD_PYTHON2_INTERFACE)
		message(WARNING "PYTHON2 interface will not be built since python 3 build is active")
	endif()
elseif (BUILD_PYTHON2_INTERFACE)
    add_subdirectory(python2)
endif()

if (BUILD_JAVA_INTERFACE)
    add_subdirectory(java)
endif ()

if(BUILD_MATLAB_INTERFACE)
    add_subdirectory(matlab)
endif()

if (BUILD_OCTAVE_INTERFACE)
	add_subdirectory(octave)
endif()
