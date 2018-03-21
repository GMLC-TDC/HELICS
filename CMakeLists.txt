##############################################################################
#Copyright (C) 2017-2018, Battelle Memorial Institute
#All rights reserved.

#This software was co-developed by Pacific Northwest National Laboratory, 
#operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory,
#operated by the Alliance for Sustainable Energy, LLC; 
#and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
##############################################################################

cmake_minimum_required (VERSION 3.4)
project (HELICS)

#-----------------------------------------------------------------------------
# HELICS Version number
#-----------------------------------------------------------------------------
set (HELICS_VERSION_MAJOR 1)
set (HELICS_VERSION_MINOR 0)
set (HELICS_VERSION_PATCH 0)
set (HELICS_VERSION_BUILD beta.2)
set (HELICS_DATE "03-20-18")
set (HELICS_VERSION "${HELICS_VERSION_MAJOR}.${HELICS_VERSION_MINOR}.${HELICS_VERSION_PATCH}-${HELICS_VERSION_BUILD}")
set (HELICS_VERSION_STRING "${HELICS_VERSION} ${HELICS_DATE}")

#-----------------------------------------------------------------------------
# set the install path to a local directory
#-----------------------------------------------------------------------------
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	 if (WIN32)
		if (MSVC)
			set (CMAKE_INSTALL_PREFIX "C:/local/helics_${HELICS_VERSION_MAJOR}_${HELICS_VERSION_MINOR}_${HELICS_VERSION_PATCH}/" CACHE PATH "default install path" FORCE )
		endif(MSVC)
	endif(WIN32)
endif()

OPTION(BUILD_HELICS_TESTS "Enable the test Executables to be built" ON)
# enable testing
if (BUILD_HELICS_TESTS)
	enable_testing ()
endif(BUILD_HELICS_TESTS)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

OPTION(BUILD_HELICS_EXAMPLES "Enable the example Executables to be built" ON)

OPTION(BUILD_PYTHON_INTERFACE "Build Python extension" OFF)
OPTION(BUILD_MATLAB_INTERFACE "Build Matlab Extension" OFF)
OPTION(BUILD_OCTAVE_INTERFACE "Build Octave extension" OFF)
OPTION(BUILD_JAVA_INTERFACE "Build Java extension" OFF)

OPTION(DISABLE_SWIG "Disable the use of swig to generate interface code and use repo code" OFF)

OPTION(BUILD_C_SHARED_LIB "Build the Shared Libraries with a C interface" ON)
IF(NOT MSVC)
	OPTION(BUILD_CXX_SHARED_LIB "Build a Shared Libraries of the CXX interface gcc/clang only" OFF)
ENDIF()

if (BUILD_PYTHON_INTERFACE OR BUILD_MATLAB_INTERFACE OR BUILD_JAVA_INTERFACE OR BUILD_OCTAVE_INTERFACE OR BUILD_C_SHARED_LIB OR BUILD_CXX_SHARED_LIB)
	set(BUILD_SHARED_LIBS ON)
else()
	OPTION(USE_POSITION_INDEPENDENT_CODE "Build the libraries with Position independent code Useful if only building the static library and it will be used later in a shared library" OFF)
endif()


if (BUILD_PYTHON_INTERFACE OR BUILD_MATLAB_INTERFACE OR BUILD_JAVA_INTERFACE OR BUILD_OCTAVE_INTERFACE OR BUILD_C_SHARED_LIB)
	set(BUILT_C_SHARED_LIB ON)
endif()

IF(BUILD_SHARED_LIBS OR USE_POSITION_INDEPENDENT_CODE)
    SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
ENDIF()




#-----------------------------------------------------------------------------
# General project wide configuration
#-----------------------------------------------------------------------------

if (NOT CMAKE_DEBUG_POSTFIX)
	set(CMAKE_DEBUG_POSTFIX d)
endif()

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/config/cmake/")

include_directories ("${PROJECT_SOURCE_DIR}/ThirdParty")
include_directories ("${PROJECT_BINARY_DIR}/libs/include")

include(extraMacros)

# Prohibit in-source build
IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(FATAL_ERROR "In-source build is not supported. Please, use an empty directory for building the project.")
ENDIF()

include(compiler_flags)

IF(UNIX)
  # Since default builds of boost library under Unix don't use
  # CMake, turn off using CMake build and find include/libs the
  # regular way.
  set(Boost_NO_BOOST_CMAKE ON)
  set(Boost_USE_MULTITHREADED      OFF)   # Needed if MT libraries not built
   option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" OFF)
ELSE(UNIX)
  IF(MSYS)
	option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" OFF)
  ELSE(MSYS)
  #this would most likely be MSVC
   option (USE_BOOST_STATIC_LIBS "Build using boost static Libraries" ON)
  ENDIF(MSYS)
ENDIF(UNIX)

IF (USE_BOOST_STATIC_LIBS)
  set(Boost_USE_STATIC_LIBS ON)
  set(BOOST_STATIC ON)
ENDIF ()

# -------------------------------------------------------------
# add coverage target
# -------------------------------------------------------------
IF(BUILD_HELICS_TESTS)
 option (TEST_CODE_COVERAGE "Build a target for testing code coverage" OFF)
 if (TEST_CODE_COVERAGE)
 include(CodeCoverage)

 set(COVERAGE_EXCLUDES 'ThirdParty/*' 'tests/*' 'swig/*' 'examples/*')
 add_compile_options(${COVERAGE_COMPILER_FLAGS})
  SETUP_TARGET_FOR_COVERAGE(
     NAME helics_coverage                    # New target name
     EXECUTABLE CTest # Executable in PROJECT_BINARY_DIR
	)

 endif(TEST_CODE_COVERAGE)
 endif(BUILD_HELICS_TESTS)


# -------------------------------------------------------------
# Get some configuration for C++17 as that becomes available
# -------------------------------------------------------------
#message(STATUS ${CMAKE_CXX_FLAGS})
set(CONFIGURE_TARGET_LOCATION ${PROJECT_BINARY_DIR}/libs/include/helics/)
include(configGenerator)

option (HELICS_GENERATE_DOXYGEN_DOC "Generate Doxygen doc target" OFF)

IF (HELICS_GENERATE_DOXYGEN_DOC)
find_package(Doxygen)
if(DOXYGEN_FOUND)

	SHOW_VARIABLE(DOXYGEN_OUTPUT_DIR PATH "location to put Doxygen docs" "${PROJECT_BINARY_DIR}/docs")
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(doc
	${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	WORKING_DIRECTORY ${DOXYGET_OUTPUT_DIR}
	COMMENT "Generating API documentation with Doxygen" VERBATIM
)
endif(DOXYGEN_FOUND)
endif (HELICS_GENERATE_DOXYGEN_DOC)

FILE(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/logs)




# -------------------------------------------------------------
# BOOST  find the boost libraries
# -------------------------------------------------------------
include(addBoost)

add_library(helics-static STATIC
    $<TARGET_OBJECTS:application_api>
    $<TARGET_OBJECTS:helics_core>
    $<TARGET_OBJECTS:helics_common>
)
message(STATUS "setting version option to \"${VERSION_OPTION}\"")
IF (VERSION_OPTION)
	IF (MSVC)
		target_compile_options(helics-static PUBLIC ${VERSION_OPTION})
	ELSE()
		target_compile_options(helics-static PUBLIC $<$<COMPILE_LANGUAGE:CXX>:${VERSION_OPTION}>)
	ENDIF (MSVC)
ENDIF(VERSION_OPTION)


# -------------------------------------------------------------
# Enable ZeroMQ
# -------------------------------------------------------------

OPTION(ZeroMQ_ENABLE "Enable ZeroMQ networking library" ON)
# If ZeroMQ library is enabled try to locate it and link against it

IF (ZeroMQ_ENABLE)
	include(addZeroMQ)

	if (ZeroMQ_FOUND)
		set(HELICS_HAVE_ZEROMQ TRUE)
	else(ZeroMQ_FOUND)
		set(HELICS_HAVE_ZEROMQ FALSE)
	endif(ZeroMQ_FOUND)
 ELSE(ZeroMQ_ENABLE)
	set(HELICS_HAVE_ZEROMQ FALSE)
ENDIF(ZeroMQ_ENABLE)

# -------------------------------------------------------------
# finding MPI
# -------------------------------------------------------------

OPTION(MPI_ENABLE "Enable MPI networking library" OFF)
IF (MPI_ENABLE)
	find_package(MPI)
	if (MPI_C_FOUND)
   set(HELICS_HAVE_MPI TRUE)
	else()
		set(HELICS_HAVE_MPI FALSE)
	endif (MPI_C_FOUND)
ELSE(MPI_ENABLE)
	set(HELICS_HAVE_MPI FALSE)
ENDIF(MPI_ENABLE)

# -------------------------------------------------------------
# add threading support
# -------------------------------------------------------------
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(helics-static PUBLIC Threads::Threads)

IF (UNIX AND NOT APPLE)
	target_link_libraries(helics-static PUBLIC rt)
ENDIF()

IF(MINGW)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	if (BOOST_VERSION_LEVEL GREATER 1)
	#set(DISABLE_TCP_CORE TRUE)
		set(DISABLE_TCP_CORE FALSE)
		target_link_libraries(helics-static PUBLIC wsock32 ws2_32)
	else()
		set(DISABLE_TCP_CORE TRUE)
		message(status, "disabling TCP core |${DISABLE_TCP_CORE}|")
		target_link_libraries(helics-static PUBLIC ws2_32)
	endif()

	else()
		target_link_libraries(helics-static PUBLIC wsock32 ws2_32)
	endif()

ENDIF(MINGW)

if (${ZeroMQ_FOUND})
	if (ZMQ_USE_STATIC_LIBRARY)
		set(ZeroMQ_DEPENDENCY ${ZeroMQ_STATIC_LIBRARY})
		ADD_DEFINITIONS(-DZMQ_STATIC)
	else()
		set(ZeroMQ_DEPENDENCY ${ZeroMQ_LIBRARY})
	endif()
	message(STATUS "zmq dep ${ZeroMQ_DEPENDENCY}")
	target_link_libraries(helics-static PUBLIC ${ZeroMQ_DEPENDENCY})
else()
   if (ZeroMQ_ENABLE)
   message(WARNING "ZeroMQ not found")
   endif()
endif ()

if (${MPI_C_FOUND})
   target_link_libraries(helics-static PUBLIC ${MPI_C_LIBRARIES})
endif (${MPI_C_FOUND})



link_directories(${Boost_LIBRARY_DIRS})

target_link_libraries(helics-static PUBLIC ${Boost_LIBRARIES_core})


include(GNUInstallDirs)

# -------------------------------------------------------------
# setting the RPATH
# -------------------------------------------------------------
# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}")
ENDIF("${isSystemDir}" STREQUAL "-1")

if (NOT Boost_USE_STATIC_LIBS)
list(APPEND CMAKE_INSTALL_RPATH ${Boost_LIBRARY_DIRS})
endif()

OPTION(BUILD_PLAYER "Build a helics player" ON)
OPTION(BUILD_RECORDER "Build a helics recorder" ON)

# we aren't ready for this for the whole library yet
IF(BUILD_SHARED_LIBS AND NOT BUILD_CXX_SHARED_LIB)
    IF(UNIX)
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fvisibility=hidden>)
		add_compile_options($<$<COMPILE_LANGUAGE:C>:-fvisibility=hidden>)
    ELSE(UNIX)
        IF(MINGW)
            add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fvisibility=hidden>)
			add_compile_options($<$<COMPILE_LANGUAGE:C>:-fvisibility=hidden>)
        ENDIF(MINGW)
    ENDIF(UNIX)
ENDIF()

IF(BUILD_CXX_SHARED_LIB)
	if (BUILD_C_SHARED_LIB OR BUILD_PYTHON_INTERFACE OR BUILD_JAVA_INTERFACE OR BUILD_MATLAB_INTERFACE OR BUILD_OCTAVE_INTERFACE)
		message(WARNING "Building the CXX shared library and C shared library in the same build is not advisable due to conflicting symbol visibility guidelines and is unlikely to work very well")
	endif()

	add_library(helics-shared SHARED src/empty.cpp)
	target_link_libraries(helics-shared helics-static)
	if (WIN32)
		IF(MINGW)
			set_target_properties(helics-shared PROPERTIES
			LINK_FLAGS "-Wl,--whole-archive")
		ELSE(MINGW)
			set_target_properties(helics-shared PROPERTIES
			LINK_FLAGS "/WHOLEARCHIVE")
		endif()

	elseif (APPLE)
		set_target_properties(helics-shared PROPERTIES
        LINK_FLAGS "-Wl,-all_load")
	else ()
		set_target_properties(helics-shared PROPERTIES
        LINK_FLAGS "-Wl,--whole-archive")
	endif ()

endif()

FILE(GLOB KEY_LIBRARY_FILES  ${PROJECT_BINARY_DIR}/libs/bin/*)
message(STATUS "key files ${KEY_LIBRARY_FILES}")

OPTION(DISABLE_LOGGING "disable all normal, debug, and trace logging in HELICS" OFF)
if (NOT DISABLE_LOGGING)
OPTION(DISABLE_TRACE_LOGGING "disable trace logging" OFF)
OPTION(DISABLE_DEBUG_LOGGING "disable debug logging" OFF)
endif()


#-----------------------------------------------------------------------------
# CMAKE Subdirectories
#-----------------------------------------------------------------------------

add_subdirectory (src)

#-----------------------------------------------------------------------------
# Setup CTEST environment
#-----------------------------------------------------------------------------
IF(BUILD_HELICS_TESTS)
include (CTest)
add_subdirectory (tests)
ENDIF(BUILD_HELICS_TESTS)

if (BUILD_HELICS_EXAMPLES)
	add_subdirectory (examples)
endif(BUILD_HELICS_EXAMPLES)

INSTALL(FILES ${KEY_LIBRARY_FILES} 
	DESTINATION ${CMAKE_INSTALL_BINDIR}
	COMPONENT runtime)
# -------------------------------------------------------------
# Enable clang analysis and formatting tools
# -------------------------------------------------------------

OPTION(ENABLE_CLANG_TOOLS "if clang is found enable some custom targets for clang formatting and tidy" OFF)

if (ENABLE_CLANG_TOOLS)
include(clang-cxx-dev-tools)
endif(ENABLE_CLANG_TOOLS)

if (BUILD_PYTHON_INTERFACE OR BUILD_MATLAB_INTERFACE OR BUILD_JAVA_INTERFACE OR BUILD_OCTAVE_INTERFACE)
add_subdirectory(swig)
endif()


add_subdirectory(docs)

set(HELICS_CMAKECONFIG_INSTALL_DIR "cmake/${PROJECT_NAME}" CACHE STRING "install path for HELICSConfig.cmake")

install (TARGETS helics-static EXPORT helics-targets
	DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libs)

	set(helics_static_file "${CMAKE_STATIC_LIBRARY_PREFIX}helics-static${CMAKE_STATIC_LIBRARY_SUFFIX}")
	set(helics_static_file_debug "${CMAKE_STATIC_LIBRARY_PREFIX}helics-static${CMAKE_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
IF(BUILD_CXX_SHARED_LIB)
install (TARGETS helics-shared EXPORT helics-targets
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
	
	INSTALL(FILES $<TARGET_LINKER_FILE:helics-shared> DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libs)
	INSTALL(FILES $<TARGET_FILE:helics-shared> DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT libs)

	set(helics_cxx_shared_file "${CMAKE_SHARED_LIBRARY_PREFIX}helics-shared${CMAKE_SHARED_LIBRARY_SUFFIX}")
	set(helics_cxx_shared_file_debug "${CMAKE_SHARED_LIBRARY_PREFIX}helics-shared${CMAKE_DEBUG_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
ELSE (BUILD_CXX_SHARED_LIB)
	set(helics_cxx_shared_file )
	set(helics_cxx_shared_file_debug )
ENDIF(BUILD_CXX_SHARED_LIB)

IF(BUILD_C_SHARED_LIB OR BUILD_PYTHON_INTERFACE OR BUILD_MATLAB_INTERFACE OR BUILD_JAVA_INTERFACE OR BUILD_OCTAVE_INTERFACE)
	set(helics_c_shared_file "${CMAKE_SHARED_LIBRARY_PREFIX}helicsSharedLib${CMAKE_SHARED_LIBRARY_SUFFIX}")
	set(helics_c_shared_file_debug "${CMAKE_SHARED_LIBRARY_PREFIX}helicsSharedLib${CMAKE_DEBUG_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
ELSE()
	set(helics_c_shared_file )
	set(helics_c_shared_file_debug )
ENDIF()


install (EXPORT helics-targets 
	NAMESPACE Helics::
	DESTINATION ${HELICS_CMAKECONFIG_INSTALL_DIR}
	COMPONENT libs
	)

set(helics_dependencies ${Boost_LIBRARIES_core})

IF (UNIX AND NOT APPLE)
	list(APPEND helics_dependencies rt)
ENDIF()

IF(MINGW)
	list(APPEND helics_dependencies wsock32 ws2_32)
ENDIF(MINGW)

if (ZeroMQ_FOUND)
	list(APPEND helics_dependencies ${ZeroMQ_DEPENDENCY})
endif(ZeroMQ_FOUND)



#-----------------------------------------------------------------------------
# Setup configure.h file for accessing configure options
#-----------------------------------------------------------------------------
configure_file (
  "config/helics-config.h.in"
  "${PROJECT_BINARY_DIR}/libs/include/helics/helics-config.h"
  )
  install(FILES ${PROJECT_BINARY_DIR}/libs/include/helics/helics-config.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics
		COMPONENT headers)
  install(FILES ${PROJECT_BINARY_DIR}/libs/include/helics/compiler-config.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics
		COMPONENT headers)
  install(DIRECTORY ThirdParty/cereal DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
		COMPONENT headers)

install(DIRECTORY ThirdParty/helics_includes DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
		COMPONENT headers)



include(CMakePackageConfigHelpers)


configure_package_config_file(config/${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${HELICS_CMAKECONFIG_INSTALL_DIR})
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${HELICS_VERSION_MAJOR}.${HELICS_VERSION_MINOR}.${HELICS_VERSION_PATCH}.${HELICS_VERSION_BUILD}
                                 COMPATIBILITY AnyNewerVersion)

configure_file(config/${PROJECT_NAME}Import.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Import.cmake" @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
			   ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Import.cmake
              DESTINATION ${HELICS_CMAKECONFIG_INSTALL_DIR}
			  COMPONENT libs)

if (CMAKE_INSTALL_PREFIX)
    message(STATUS "Using CMAKE_INSTALL_PREFIX: " ${CMAKE_INSTALL_PREFIX})
endif()

# -------------------------------------------------------------
# CPack 
# -------------------------------------------------------------

set(CPACK_PACKAGE_NAME "Helics")
set(CPACK_PACKAGE_VENDOR "GMLC")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "HELICS")
set(CPACK_PACKAGE_VERSION "${HELICS_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR ${HELICS_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${HELICS_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${HELICS_VERSION_PATCH})

set(CPACK_COMPONENTS_ALL applications headers libs runtime matlab python java octave)
if (WIN32)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}\\\\LICENSE")
else(WIN32)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
endif(WIN32)

set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Application")
 set(CPACK_COMPONENT_LIBS_DISPLAY_NAME "Libraries")
 set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "Headers")
 set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "Runtime Libraries")

  
 set(CPACK_COMPONENT_MATLAB_GROUP interfaces)
 set(CPACK_COMPONENT_JAVA_GROUP interfaces)
 set(CPACK_COMPONENT_OCTAVE_GROUP interfaces)
 set(CPACK_COMPONENT_PYTHON_GROUP interfaces)
 
 set(CPACK_COMPONENT_APPLICATION_DESCRIPTION "Executables and helper applications for HELICS")
 set(CPACK_COMPONENT_LIBS_DESCRIPTION "Libraries for compiling and linking with HELICS")
 set(CPACK_COMPONENT_HEADERS_DESCRIPTION "Headers for linking and compiling with HELICS")
 set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "Runtime libraries for HELICS")
 
 set(CPACK_COMPONENT_GROUP_INTERFACES_DESCRIPTION "additional language interfaces for HELICS")
 
 set(CPACK_COMPONENT_LIBS_DEPENDS headers)
 SET(CPACK_COMPONENT_RUNTIME_REQUIRED ON)
 
 set(CPACK_PACKAGE_EXECUTABLES "helics_broker" "Helics Broker")
 if (WIN32)
	set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}\\\\docs\\\\img\\\\HELICS.ico")
	set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/docs/img/HELICS.ico")
	set(CPACK_NSIS_INSTALL_ROOT "C:\\\\local\\\\")
	set(CPACK_NSIS_URL_INFO_ABOUT "https://www.github.com/GMLC-TDC/Helics-src")
set(CPACK_NSIS_MENU_LINKS 
	"https://www.github.com/GMLC-TDC/Helics-src" "HELICS Github"
	"https://gmlc-tdc.github.io/HELICS-src/" "Helics Documentation"
	"https://www.helics.org" "Helics Web page"
	"https://www.youtube.com/channel/UCPa81c4BVXEYXt2EShTzbcg" "TDC YouTube channel")
else (WIN32)
	set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/docs/img/HELICS.ico")
endif(WIN32)


set(CPACK_SOURCE_IGNORE_FILES  "/Build*/;/build*/;/.git/")
#THIS LINE MUST BE LAST
include(CPack)
