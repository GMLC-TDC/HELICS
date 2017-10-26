cmake_minimum_required (VERSION 3.4)
project (HELICS)

#-----------------------------------------------------------------------------
# HELICS Version number
#-----------------------------------------------------------------------------
set (HELICS_VERSION_MAJOR 0)
set (HELICS_VERSION_MINOR 3)
set (HELICS_VERSION_PATCH 0)

set (HELICS_DATE "10-16-17")

OPTION(BUILD_HELICS_TESTS "Enable the test Executables to be built" ON)
# enable testing
if (BUILD_HELICS_TESTS)
enable_testing ()
endif(BUILD_HELICS_TESTS)

OPTION(BUILD_HELICS_EXAMPLES "Enable the test Executables to be built" OFF)

OPTION(BUILD_PYTHON "Build Python extension" OFF)

OPTION(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)
SET(LIB_TYPE STATIC)
IF(BUILD_SHARED_LIBS)
    SET(LIB_TYPE SHARED)
    SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
ENDIF(BUILD_SHARED_LIBS)

#-----------------------------------------------------------------------------
# General project wide configuration
#-----------------------------------------------------------------------------
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

include_directories ("${PROJECT_BINARY_DIR}/include")
include_directories ("${PROJECT_SOURCE_DIR}/src")
include_directories ("${PROJECT_SOURCE_DIR}/ThirdParty")
include_directories ("${PROJECT_SOURCE_DIR}/src/helics")
include_directories ("${PROJECT_BINARY_DIR}/libs/include")

# -------------------------------------------------------------
# MACRO definitions
# -------------------------------------------------------------

# Macros to hide/show cached variables.
# These two macros can be used to "hide" or "show" in the
# list of cached variables various variables and/or options 
# that depend on other options.
# Note that once a variable is modified, it will preserve its
# value (hiding it merely makes it internal)

MACRO(HIDE_VARIABLE var)
  IF(DEFINED ${var})
    SET(${var} "${${var}}" CACHE INTERNAL "")
  ENDIF(DEFINED ${var})
ENDMACRO(HIDE_VARIABLE)

MACRO(SHOW_VARIABLE var type doc default)
  IF(DEFINED ${var})
    SET(${var} "${${var}}" CACHE "${type}" "${doc}" FORCE)
  ELSE(DEFINED ${var})
    SET(${var} "${default}" CACHE "${type}" "${doc}")
  ENDIF(DEFINED ${var})
ENDMACRO(SHOW_VARIABLE)




# Prohibit in-source build
IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In-source build is not supported. Please, use an empty directory for building the project.")
ENDIF()

# -------------------------------------------------------------
# Setup compiler options and configurations
# -------------------------------------------------------------
message(STATUS "setting up for ${CMAKE_CXX_COMPILER_ID}")
IF(UNIX)
  # Since default builds of boost library under Unix don't use
  # CMake, turn off using CMake build and find include/libs the
  # regular way.
  set(Boost_NO_BOOST_CMAKE ON)

  set(Boost_USE_MULTITHREADED      OFF)   # Needed if MT libraries not built
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wall>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-pedantic>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wextra>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wshadow>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wstrict-aliasing>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wunreachable-code>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-parameter>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wundef>)
ELSE(UNIX)
  IF(MINGW)
 add_compile_options(-Wall -pedantic)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wextra>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wshadow>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wstrict-aliasing>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wunreachable-code>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-unused-parameter>)
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wundef>)
  ELSE(MINGW)
  set(Boost_USE_STATIC_LIBS ON)
# -------------------------------------------------------------
# Extra definitions for visual studio
# -------------------------------------------------------------
IF(MSVC)
  ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
  add_compile_options(/bigobj)
  add_compile_options(-W4  /EHsc /wd4065 /wd4101 /wd4102 /wd4244 /wd4297 /wd4355 /wd4800 /wd4484)
ENDIF(MSVC)
  ENDIF(MINGW)
ENDIF(UNIX)

# -------------------------------------------------------------
# Get some configuration for C++17 as that becomes available
# -------------------------------------------------------------
#message(STATUS ${CMAKE_CXX_FLAGS})
include(configGenerator)

IF (VERSION_OPTION)
	add_compile_options(${VERSION_OPTION})
ELSE ()
set(CMAKE_CXX_STANDARD 14)
ENDIF ()


option (HELICS_GENERATE_DOXYGEN_DOC "Generate Doxygen doc target" OFF)

IF (HELICS_GENERATE_DOXYGEN_DOC)
find_package(Doxygen)
if(DOXYGEN_FOUND)

	SHOW_VARIABLE(DOXYGEN_OUTPUT_DIR PATH "location to put Doxygen docs" "${CMAKE_CURRENT_SOURCE_DIR}/docs")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(doc
	${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs}
	COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)
endif (HELICS_GENERATE_DOXYGEN_DOC)
# -------------------------------------------------------------
# finding glog
# -------------------------------------------------------------
#find_package (glog)
#if (${GLOG_FOUND})
#    set(HELICS_HAVE_GLOG TRUE)
#    include_directories("${GLOG_INCLUDE_DIRS}")
#endif (${GLOG_FOUND})

# -------------------------------------------------------------
# finding ZMQ
# -------------------------------------------------------------
#find_package (ZeroMQ)
#if (${ZEROMQ_FOUND})
#   set(HELICS_HAVE_ZEROMQ TRUE)
#   include_directories ("${ZeroMQ_INCLUDE_DIR}")
#endif (${ZEROMQ_FOUND})	   

# -------------------------------------------------------------
# Enable ZMQ
# -------------------------------------------------------------

OPTION(ZMQ_ENABLE "Enable ZMQ networking library" ON)
# If ZMQ library is enabled try to locate it and link against it

IF (ZMQ_ENABLE)

SHOW_VARIABLE(ZMQ_LIBRARY_PATH PATH
  "path to the zmq libraries" "${ZMQ_LIBRARY_PATH}")
  
SHOW_VARIABLE(ZMQ_INCLUDE_PATH PATH
  "path to the zmq headers" "${ZMQ_INCLUDE_PATH}")
  set(ZMQ_FIND_QUIETLY ON)
find_package(ZMQ)

if (NOT ZMQ_FOUND)
  OPTION(AUTOBUILD_ZMQ "enable ZMQ to automatically download and build" OFF)
  IF (AUTOBUILD_ZMQ)
  include(buildlibZMQ)
  build_libzmq()
  set(ZMQ_INSTALL_PATH ${PROJECT_BINARY_DIR}/libs)
  find_package(ZMQ)
  ENDIF(AUTOBUILD_ZMQ)
  
endif(NOT ZMQ_FOUND)
if (ZMQ_FOUND)
   set(HELICS_HAVE_ZEROMQ TRUE)
   include_directories ("${ZMQ_INCLUDE_DIR}")
 endif(ZMQ_FOUND)
endif(ZMQ_ENABLE)

# -------------------------------------------------------------
# finding MPI
# -------------------------------------------------------------

OPTION(MPI_ENABLE "Enable MPI networking library" OFF)
IF (MPI_ENABLE)
find_package(MPI)
set(MPI_C_FOUND FALSE)
if (${MPI_C_FOUND})
   set(HELICS_HAVE_MPI TRUE)
   include_directories("${MPI_C_INCLUDE_PATH}")
endif (${MPI_C_FOUND})
ENDIF(MPI_ENABLE)

# -------------------------------------------------------------
# add threading support
# -------------------------------------------------------------
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
# -------------------------------------------------------------
# BOOST  find the boost libraries
# -------------------------------------------------------------

IF (MSVC)

set( boost_paths
C:/boost_1_65_1
 C:/boost_1_64_0
C:/boost_1_63_0
C:/boost_1_61_0
)
IF (IS_DIRECTORY C:/boost)
list(APPEND boost_paths
C:/boost/boost_1_65_1
 C:/boost/boost_1_64_0
C:/boost/boost_1_63_0
C:/boost/boost_1_61_0
)
ENDIF()

IF (IS_DIRECTORY C:/local)
list(APPEND boost_paths
C:/local/boost_1_65_1
 C:/local/boost_1_64_0
C:/local/boost_1_63_0
C:/local/boost_1_61_0
)
ENDIF()


IF (EXISTS D:/)
list(APPEND boost_paths
D:/boost_1_65_1
 D:/boost_1_64_0
D:/boost_1_63_0
D:/boost_1_61_0
)
IF (IS_DIRECTORY D:/boost)
list(APPEND boost_paths
D:/boost/boost_1_65_1
 D:/boost/boost_1_64_0
D:/boost/boost_1_63_0
D:/boost/boost_1_61_0
)
ENDIF()

IF (IS_DIRECTORY D:/local)
list(APPEND boost_paths
D:/local/boost_1_65_1
 D:/local/boost_1_64_0
D:/local/boost_1_63_0
D:/local/boost_1_61_0
)
ENDIF()

ENDIF()

 message(STATUS ${boost_paths})
 
find_path(BOOST_TEST_PATH 
			NAMES 			boost/version.hpp
			PATHS		${boost_paths}
		)
		
		if (BOOST_TEST_PATH)
		set(BOOST_ROOT ${BOOST_TEST_PATH})
		endif(BOOST_TEST_PATH)
ENDIF(MSVC)

SHOW_VARIABLE(BOOST_ROOT PATH "Boost root directory" "${BOOST_ROOT}")

if (${MPI_C_FOUND})
  #find_package(Boost COMPONENTS program_options unit_test_framework filesystem mpi system date_time REQUIRED)
  find_package(Boost COMPONENTS program_options unit_test_framework filesystem system date_time REQUIRED)
ELSE(${MPI_C_FOUND})
  find_package(Boost COMPONENTS program_options unit_test_framework filesystem system date_time REQUIRED)
ENDIF(${MPI_C_FOUND})

mark_as_advanced(CLEAR BOOST_ROOT)

message(STATUS "Using Boost include files : ${Boost_INCLUDE_DIR}")
message(STATUS "Using Boost libraries in : ${Boost_LIBRARY_DIRS}")
message(STATUS "Using Boost libraries : ${Boost_LIBRARIES}")
set(modifier,"")
foreach(loop_var ${Boost_LIBRARIES})
	if (${loop_var} MATCHES "debug")
		list(INSERT modifier 0 ${loop_var})
	elseif(${loop_var} MATCHES "optimized")
		list(INSERT modifier 0 ${loop_var})
	else()
		message("Boost_LIBRARIES ${loop_var}")
		if(${loop_var} MATCHES "unit_test")
			list(APPEND Boost_LIBRARIES_test ${modifier} ${loop_var})
		else()
			list(APPEND Boost_LIBRARIES_core ${modifier} ${loop_var})
		endif()
		list(LENGTH modifier modifier_size)
		if (modifier_size GREATER 0)
		list(REMOVE_AT modifier -1)
		endif()
	endif()
endforeach(loop_var)

message(STATUS "Using Boost core libraries : ${Boost_LIBRARIES_core}")
message(STATUS "Using Boost test libraries : ${Boost_LIBRARIES_test}")
list(APPEND external_library_list ${Boost_LIBRARIES_core})
list(APPEND external_link_directories ${Boost_LIBRARY_DIRS})
  
add_library(helics STATIC
    $<TARGET_OBJECTS:application_api>
    $<TARGET_OBJECTS:helics_core>
    $<TARGET_OBJECTS:helics_common>
)
target_link_libraries(helics Threads::Threads)
IF (UNIX AND NOT APPLE)
target_link_libraries(helics rt)
ENDIF()

if (${ZMQ_FOUND})
   target_link_libraries(helics ${ZMQ_LIBRARY})
else()
   if (ZMQ_ENABLE)
   message(WARNING "ZMQ not found")
   endif()
endif ()

if (${MPI_C_FOUND})
   target_link_libraries(helics ${MPI_C_LIBRARIES})
endif (${MPI_C_FOUND})


install (TARGETS helics DESTINATION lib)

# we aren't ready for this for the whole library yet
IF(BUILD_SHARED_LIBS)
    IF(UNIX)
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fvisibility=hidden>)
    ELSE(UNIX)
        IF(MINGW)
            add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fvisibility=hidden>)
        ENDIF(MINGW)
    ENDIF(UNIX)
ENDIF(BUILD_SHARED_LIBS)

FILE(GLOB KEY_LIBRARY_FILES  ${PROJECT_BINARY_DIR}/libs/bin/*)
message(STATUS "key files ${KEY_LIBRARY_FILES}")

#-----------------------------------------------------------------------------
# Setup configure.h file for accessing configure options
#-----------------------------------------------------------------------------
configure_file (
  "src/helics/config.h.in"
  "include/helics/config.h"
  )
  install(FILES ${CMAKE_BINARY_DIR}/include/helics/config.h DESTINATION include/helics)

  install(DIRECTORY ThirdParty/ DESTINATION include
	  FILES_MATCHING PATTERN "helics_includes/*.h*")
#-----------------------------------------------------------------------------
# CMAKE Subdirectories 
#-----------------------------------------------------------------------------

add_subdirectory (src)

if (BUILD_HELICS_TESTS)
add_subdirectory (tests)


#-----------------------------------------------------------------------------
# Setup CTEST environment
#-----------------------------------------------------------------------------
include (CTest)

endif(BUILD_HELICS_TESTS)

if (BUILD_HELICS_EXAMPLES)
add_subdirectory (examples)

endif(BUILD_HELICS_EXAMPLES)

INSTALL(FILES ${KEY_LIBRARY_FILES} DESTINATION bin)
# -------------------------------------------------------------
# Enable clang analysis and formatting tools
# -------------------------------------------------------------

OPTION(ENABLE_CLANG_TOOLS "if clang is found enable some custom targets for clang formatting and tidy" OFF)

if (ENABLE_CLANG_TOOLS)
include(clang-cxx-dev-tools)
endif(ENABLE_CLANG_TOOLS)

if (BUILD_PYTHON)
add_subdirectory(python)
endif(BUILD_PYTHON)

