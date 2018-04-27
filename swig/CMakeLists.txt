if (NOT DISABLE_SWIG)
	FIND_PACKAGE(SWIG REQUIRED 3.0)
	INCLUDE(${SWIG_USE_FILE})
endif()


INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES("${PROJECT_SOURCE_DIR}/src/helics/shared_api_library")
	

if (BUILD_PYTHON_INTERFACE)
    ADD_SUBDIRECTORY(python)
	if( BUILD_PYTHON2_INTERFACE)
		message(WARNING "PYTHON2 interface will not be built since python 3 build is active")
	endif()
elseif (BUILD_PYTHON2_INTERFACE)
    ADD_SUBDIRECTORY(python2)
endif()

if (BUILD_JAVA_INTERFACE)
    ADD_SUBDIRECTORY(java)
endif ()

if(BUILD_MATLAB_INTERFACE)
    add_subdirectory(matlab)
endif()

if (BUILD_OCTAVE_INTERFACE)
	add_subdirectory(octave)
endif()
