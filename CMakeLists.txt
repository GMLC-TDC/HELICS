##############################################################################
#Copyright © 2017-2018,
#Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
#All rights reserved. See LICENSE file and DISCLAIMER for more details.
##############################################################################

cmake_minimum_required (VERSION 3.4)
cmake_policy(VERSION 3.4)

project (HELICS VERSION 1.2.1)

#-----------------------------------------------------------------------------
# HELICS Version number
#-----------------------------------------------------------------------------
set (HELICS_VERSION_BUILD )
set (HELICS_DATE "06-30-18")

set (HELICS_VERSION_UNDERSCORE "${HELICS_VERSION_MAJOR}_${HELICS_VERSION_MINOR}_${HELICS_VERSION_PATCH}")
if(HELICS_VERSION_BUILD)
	set (HELICS_VERSION "${HELICS_VERSION}-${HELICS_VERSION_BUILD}")
	set (HELICS_VERSION_UNDERSCORE "${HELICS_VERSION_UNDERSCORE}-${HELICS_VERSION_BUILD}")
endif()
set (HELICS_VERSION_STRING "${HELICS_VERSION} (${HELICS_DATE})")

#-----------------------------------------------------------------------------
# set the module path and include some common macros
#-----------------------------------------------------------------------------
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/config/cmake/")
include(extraMacros)
include(CMakeDependentOption)
#-----------------------------------------------------------------------------
# set the install path to a local directory
#-----------------------------------------------------------------------------
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	 if (WIN32)
		if (MSYS)
			set (CMAKE_INSTALL_PREFIX "/usr/local/helics_${HELICS_VERSION_UNDERSCORE}/" CACHE PATH "default install path" FORCE )
		else(MSYS)
			set (CMAKE_INSTALL_PREFIX "C:/local/helics_${HELICS_VERSION_UNDERSCORE}/" CACHE PATH "default install path" FORCE )
		endif(MSYS)
	endif(WIN32)
endif()

SHOW_VARIABLE(AUTOBUILD_INSTALL_PATH PATH
  "location to install the autobuild libraries and Headers" "${PROJECT_BINARY_DIR}/libs")

mark_as_advanced(AUTOBUILD_INSTALL_PATH)

file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/logs)

option(BUILD_HELICS_TESTS "Enable the test Executables to be built" ON)
# enable testing
if (BUILD_HELICS_TESTS)
	enable_testing ()
endif(BUILD_HELICS_TESTS)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(BUILD_HELICS_EXAMPLES "Enable the example Executables to be built" ON)

option(BUILD_PYTHON_INTERFACE "Build Python extension" OFF)
option(BUILD_PYTHON2_INTERFACE "Build Python2.7 extension(Requires swig and will not build if \"PYTHON_INTERFACE\" is active)" OFF)
option(BUILD_MATLAB_INTERFACE "Build Matlab Extension" OFF)
option(BUILD_OCTAVE_INTERFACE "Build Octave extension (very experimental)" OFF)
option(BUILD_JAVA_INTERFACE "Build Java extension" OFF)

if (BUILD_PYTHON_INTERFACE OR BUILD_PYTHON2_INTERFACE OR BUILD_MATLAB_INTERFACE OR BUILD_JAVA_INTERFACE OR BUILD_OCTAVE_INTERFACE)
	set(INTERFACE_BUILD ON)
else()
	set(INTERFACE_BUILD OFF)
endif()

cmake_dependent_option(BUILD_C_SHARED_LIB "Build the Shared Libraries with a C interface" ON "NOT INTERFACE_BUILD" ON)
option(DISABLE_SWIG "disable the use of the swig executable to generate interface code and use repository code instead" OFF)
option(BUILD_CXX_SHARED_LIB "Build a Shared Libraries of the CXX interface" OFF)


if (INTERFACE_BUILD OR BUILD_C_SHARED_LIB OR BUILD_CXX_SHARED_LIB)
	set(BUILD_SHARED_LIBS ON)
endif()

cmake_dependent_option(USE_POSITION_INDEPENDENT_CODE "Build the libraries with Position independent code Useful if only building the static library and it will be used later in a shared library" OFF "NOT BUILD_SHARED_LIBS" ON)

if(BUILD_SHARED_LIBS OR USE_POSITION_INDEPENDENT_CODE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()


#-----------------------------------------------------------------------------
# General project wide configuration
#-----------------------------------------------------------------------------

if (NOT CMAKE_DEBUG_POSTFIX)
	set(CMAKE_DEBUG_POSTFIX d)
endif()

#add a baseline library for underlying dependencies and flags
add_library(helics_base INTERFACE)
add_library(helics_base_includes INTERFACE)

target_link_libraries(helics_base INTERFACE helics_base_includes)
# Prohibit in-source build
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
	message(FATAL_ERROR "In-source build is not supported. Please, use an empty directory for building the project.")
endif()

include(compiler_flags)
message(STATUS "setting version build options to \"${VERSION_OPTION}\"")
if (VERSION_OPTION)
	if (MSVC)
		target_compile_options(helics_base_includes INTERFACE ${VERSION_OPTION})
	else()
		target_compile_options(helics_base_includes INTERFACE $<$<COMPILE_LANGUAGE:CXX>:${VERSION_OPTION}>)
	endif (MSVC)
endif(VERSION_OPTION)

cmake_dependent_option(STATIC_STANDARD_LIB "Link against a static standard lib" OFF "NOT USE_LIBCXX" OFF)
include(ucm)
mark_as_advanced(STATIC_STANDARD_LIB)
if (STATIC_STANDARD_LIB)
	ucm_set_runtime(STATIC)
else()
	ucm_set_runtime(DYNAMIC)
endif()

# -------------------------------------------------------------
# add coverage target
# -------------------------------------------------------------
if(BUILD_HELICS_TESTS)
	cmake_dependent_option (TEST_CODE_COVERAGE "Build a target for testing code coverage" OFF "BUILD_HELICS_TESTS" OFF)
	if (TEST_CODE_COVERAGE)
		include(CodeCoverage)

		set(COVERAGE_EXCLUDES 'ThirdParty/*' 'tests/*' 'swig/*' 'examples/*' 'build*/*')
		add_compile_options(${COVERAGE_COMPILER_FLAGS})
		SETUP_TARGET_FOR_COVERAGE(
			NAME helics_coverage                    # New target name
			EXECUTABLE CTest # Executable in PROJECT_BINARY_DIR
		)

	endif(TEST_CODE_COVERAGE)
 endif(BUILD_HELICS_TESTS)
mark_as_advanced(TEST_CODE_COVERAGE)

# -------------------------------------------------------------
# Get some configuration for C++17 as that becomes available
# -------------------------------------------------------------
#message(STATUS ${CMAKE_CXX_FLAGS})
set(CONFIGURE_TARGET_LOCATION ${AUTOBUILD_INSTALL_PATH}/include/helics/)
include(configGenerator)

option (GENERATE_DOXYGEN_DOC "Generate Doxygen doc target" OFF)

if (GENERATE_DOXYGEN_DOC)
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
endif (GENERATE_DOXYGEN_DOC)



# -------------------------------------------------------------
# BOOST  find the boost libraries
# -------------------------------------------------------------
set(BOOST_REQUIRED_LIBRARIES program_options unit_test_framework filesystem system date_time timer chrono)
include(addBoost)

target_link_libraries(helics_base INTERFACE Boostlibs::core)
# -------------------------------------------------------------
# finding MPI
# -------------------------------------------------------------

option(MPI_ENABLE "Enable MPI networking library" OFF)
if (MPI_ENABLE)
	include(addMPI)
	if (MPI_C_FOUND)
		set(HELICS_HAVE_MPI TRUE)
		target_link_libraries(helics_base INTERFACE MPI::MPI_C)
	else()
		set(HELICS_HAVE_MPI FALSE)
	endif (MPI_C_FOUND)
else(MPI_ENABLE)
	set(HELICS_HAVE_MPI FALSE)
endif(MPI_ENABLE)

# -------------------------------------------------------------
# add threading support
# -------------------------------------------------------------
if (NOT WIN32)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
elseif (MSYS)
	set(THREADS_PREFER_PTHREAD_FLAG ON)
else()
	set(CMAKE_USE_WIN32_THREADS_INIT ON)
endif()
find_package(Threads REQUIRED)

target_link_libraries(helics_base INTERFACE Threads::Threads)

if (UNIX AND NOT APPLE)
	target_link_libraries(helics_base INTERFACE rt)
endif()

# -------------------------------------------------------------
# check for some things with TCP core and boost
# -------------------------------------------------------------
if(MSYS)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		if (BOOST_VERSION_LEVEL GREATER 1)
			#set(DISABLE_TCP_CORE TRUE)
			set(DISABLE_TCP_CORE FALSE)
			target_link_libraries(helics_base INTERFACE wsock32 ws2_32)
		else()
			set(DISABLE_TCP_CORE TRUE)
			message(STATUS, "disabling TCP core |${DISABLE_TCP_CORE}|")
			target_link_libraries(helics_base INTERFACE ws2_32)
		endif()

	else()
		target_link_libraries(helics_base INTERFACE wsock32 ws2_32)
	endif()

endif(MSYS)

# -------------------------------------------------------------
# Enable ZeroMQ
# -------------------------------------------------------------

### just check some intenal options
if (BUILD_DEBUG_ONLY AND BUILD_RELEASE_ONLY)
message(fatal "BOTH BUILD_DEBUG_ONLY AND BUILD_RELEASE_ONLY cannot be specified together otherwise nothing works")
endif()

option(ZeroMQ_ENABLE "Enable ZeroMQ networking library" ON)
mark_as_advanced(ZeroMQ_ENABLE)
# If ZeroMQ library is enabled try to locate it and link against it

if (ZeroMQ_ENABLE)
	include(addZeroMQ)

	if (ZeroMQ_FOUND)
		set(HELICS_HAVE_ZEROMQ TRUE)
		if (ZMQ_USE_STATIC_LIBRARY)
			set(ZeroMQ_DEPENDENCY libzmq-static)
			target_compile_definitions(helics_base_includes INTERFACE -DZMQ_STATIC)
		else()
			set(ZeroMQ_DEPENDENCY libzmq)
		endif()
		#message(STATUS "zmq dep ${ZeroMQ_DEPENDENCY}")
		target_link_libraries(helics_base INTERFACE ${ZeroMQ_DEPENDENCY})
	else(ZeroMQ_FOUND)
		set(HELICS_HAVE_ZEROMQ FALSE)
	endif(ZeroMQ_FOUND)
 else(ZeroMQ_ENABLE)
	set(HELICS_HAVE_ZEROMQ FALSE)
	if (ZeroMQ_ENABLE)
		message(WARNING "ZeroMQ not found")
   endif()
endif(ZeroMQ_ENABLE)


# -------------------------------------------------------------
# global include directories
# -------------------------------------------------------------
target_include_directories(helics_base_includes INTERFACE
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
	$<BUILD_INTERFACE:${AUTOBUILD_INSTALL_PATH}/include>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_include_directories(helics_base_includes SYSTEM INTERFACE
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty>
	$<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>
)

include(GNUInstallDirs)

# -------------------------------------------------------------
# setting the RPATH
# -------------------------------------------------------------
# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_BUILD_RPATH "${AUTOBUILD_INSTALL_PATH}/bin;${AUTOBUILD_INSTALL_PATH}/lib;${AUTOBUILD_INSTALL_PATH}/lib64")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# the RPATH to be used when installing, but only if it's not a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
   list(APPEND CMAKE_INSTALL_RPATH CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}")
endif("${isSystemDir}" STREQUAL "-1")

list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    list(APPEND CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
endif("${isSystemDir}" STREQUAL "-1")

if (NOT Boost_USE_STATIC_LIBS)
	list(APPEND CMAKE_INSTALL_RPATH ${Boost_LIBRARY_DIRS})
	list(APPEND CMAKE_BUILD_RPATH ${Boost_LIBRARY_DIRS})
endif()

option(BUILD_BROKER "Build a helics Broker executable" ON)
option(BUILD_APPS "Build helics applications to run various apps, including player, recorder, and helics_apps" ON)


FILE(GLOB KEY_LIBRARY_FILES  ${AUTOBUILD_INSTALL_PATH}/bin/*)
#message(STATUS "key files ${KEY_LIBRARY_FILES}")

option(DISABLE_LOGGING "disable all normal, debug, and trace logging in HELICS" OFF)

cmake_dependent_option(DISABLE_TRACE_LOGGING "disable trace logging" OFF "NOT DISABLE_LOGGING" OFF)
cmake_dependent_option(DISABLE_DEBUG_LOGGING "disable debug logging" OFF "NOT DISABLE_LOGGING" OFF)
mark_as_advanced(DISABLE_TRACE_LOGGING)
mark_as_advanced(DISABLE_DEBUG_LOGGING)

#-----------------------------------------------------------------------------
# create the fmt header only targets
#-----------------------------------------------------------------------------
set(FMT_TEST OFF)
set(FMT_ASSUME_CPP14_SUPPORT ON)
set(FMT_SILENT ON)
#get the FMT header only library
add_subdirectory(ThirdParty/fmt EXCLUDE_FROM_ALL)
HIDE_VARIABLE(FMT_DOC)
HIDE_VARIABLE(FMT_INSTALL)
HIDE_VARIABLE(FMT_PEDANTIC)
HIDE_VARIABLE(FMT_TEST)
HIDE_VARIABLE(FMT_USE_CPP14)

#-----------------------------------------------------------------------------
# CMAKE Subdirectories
#-----------------------------------------------------------------------------

add_subdirectory (src)

#-----------------------------------------------------------------------------
# Setup CTEST environment
#-----------------------------------------------------------------------------
if(BUILD_HELICS_TESTS)
	include (CTest)
	mark_as_advanced(BUILD_TESTING)
	add_subdirectory (tests)
endif(BUILD_HELICS_TESTS)

#-----------------------------------------------------------------------------
# Setup Examples
#-----------------------------------------------------------------------------
if (BUILD_HELICS_EXAMPLES)
	add_subdirectory (examples)
endif(BUILD_HELICS_EXAMPLES)

install(FILES ${KEY_LIBRARY_FILES}
	DESTINATION ${CMAKE_INSTALL_BINDIR}
	COMPONENT runtime)

install(DIRECTORY ${AUTOBUILD_INSTALL_PATH}/lib/ DESTINATION ${CMAKE_INSTALL_LIBDIR}/ OPTIONAL)
install(DIRECTORY ${AUTOBUILD_INSTALL_PATH}/lib64/ DESTINATION ${CMAKE_INSTALL_LIBDIR}/ OPTIONAL)

if (HELICS_HAVE_ZEROMQ)
	if (WIN32 AND NOT MSYS)
		install(FILES $<TARGET_LINKER_FILE:libzmq> DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libs)
	endif()
endif()

# -------------------------------------------------------------
# Enable clang analysis and formatting tools
# -------------------------------------------------------------

option(ENABLE_CLANG_TOOLS "if clang is found enable some custom targets for clang formatting and tidy" OFF)
mark_as_advanced(ENABLE_CLANG_TOOLS)
if (ENABLE_CLANG_TOOLS)
	include(clang-cxx-dev-tools)
endif(ENABLE_CLANG_TOOLS)

if (INTERFACE_BUILD)
	add_subdirectory(swig)
endif()


add_subdirectory(docs)


install (TARGETS helics_base EXPORT helics-targets)
install (TARGETS helics_base_includes EXPORT helics-targets)

set(helics_static_file "${CMAKE_STATIC_LIBRARY_PREFIX}helics-static${CMAKE_STATIC_LIBRARY_SUFFIX}")
set(helics_static_file_debug "${CMAKE_STATIC_LIBRARY_PREFIX}helics-static${CMAKE_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}")
if(BUILD_CXX_SHARED_LIB)
	set(helics_cxx_shared_file "${CMAKE_SHARED_LIBRARY_PREFIX}helics-shared${CMAKE_SHARED_LIBRARY_SUFFIX}")
	set(helics_cxx_shared_file_debug "${CMAKE_SHARED_LIBRARY_PREFIX}helics-shared${CMAKE_DEBUG_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
else (BUILD_CXX_SHARED_LIB)
	set(helics_cxx_shared_file )
	set(helics_cxx_shared_file_debug )
endif(BUILD_CXX_SHARED_LIB)

if(BUILD_C_SHARED_LIB OR INTERFACE_BUILD)
	set(helics_c_shared_file "${CMAKE_SHARED_LIBRARY_PREFIX}helicsSharedLib${CMAKE_SHARED_LIBRARY_SUFFIX}")
	set(helics_c_shared_file_debug "${CMAKE_SHARED_LIBRARY_PREFIX}helicsSharedLib${CMAKE_DEBUG_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
else()
	set(helics_c_shared_file )
	set(helics_c_shared_file_debug )
endif()

set(HELICS_CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE STRING "install path for HELICSConfig.cmake")
mark_as_advanced(HELICS_CMAKECONFIG_INSTALL_DIR)

install (EXPORT helics-targets
	NAMESPACE HELICS::
	DESTINATION ${HELICS_CMAKECONFIG_INSTALL_DIR}
	COMPONENT libs
	)

#-----------------------------------------------------------------------------
# Setup configure.h file for accessing configure options
#-----------------------------------------------------------------------------
configure_file (
  "config/helics-config.h.in"
  "${AUTOBUILD_INSTALL_PATH}/include/helics/helics-config.h"
  )
  install(FILES ${AUTOBUILD_INSTALL_PATH}/include/helics/helics-config.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics
		COMPONENT headers)
  install(FILES ${AUTOBUILD_INSTALL_PATH}/include/helics/compiler-config.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics
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
                                 VERSION ${HELICS_VERSION}
                                 COMPATIBILITY SameMajorVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
              DESTINATION ${HELICS_CMAKECONFIG_INSTALL_DIR}
			  COMPONENT libs)
#install required additional cmake files
install(FILES config/cmake/addMPI.cmake
			  config/cmake/FindZeroMQ.cmake
			  config/cmake/addBoost.cmake
			  config/cmake/extraMacros.cmake
			  DESTINATION ${HELICS_CMAKECONFIG_INSTALL_DIR}
			  COMPONENT libs)

# -------------------------------------------------------------
# CPack
# -------------------------------------------------------------
option(ENABLE_PACKAGE_BUILD "Add projects for making packages and installers for HELICS" OFF)

if(ENABLE_PACKAGE_BUILD)
set(CPACK_PACKAGE_NAME "Helics")
set(CPACK_PACKAGE_VENDOR "GMLC")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "HELICS")
set(CPACK_PACKAGE_VERSION "${HELICS_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR ${HELICS_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${HELICS_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${HELICS_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "HELICS")

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
 set(CPACK_COMPONENT_RUNTIME_REQUIRED ON)

 set(CPACK_PACKAGE_EXECUTABLES "helics_broker" "Helics Broker" "helics_app" "Helics app executable" "helics_recorder" "Helics recorder" "helics_player" "Helics Player")
 if (WIN32)
	set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}\\\\docs\\\\img\\\\HELICS.ico")
	set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/docs/img/HELICS.ico")
	set(CPACK_NSIS_INSTALL_ROOT "C:\\\\local\\\\")
	set(CPACK_NSIS_URL_INFO_ABOUT "https://www.github.com/GMLC-TDC/Helics-src")
	set(CPACK_NSIS_HELP_LINK "https://gmlc-tdc.github.io/HELICS-src/")
	set(CPACK_NSIS_CONTACT "helicsteam@helics.org")
	set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
	set(CPACK_NSIS_MODIFY_PATH ON)
	set(CPACK_NSIS_EXECUTABLES_DIRECTORY ${CMAKE_INSTALL_BINDIR})
set(CPACK_NSIS_MENU_LINKS
	"https://www.github.com/GMLC-TDC/Helics-src" "HELICS Github"
	"https://gmlc-tdc.github.io/HELICS-src/" "Helics Documentation"
	"https://www.helics.org" "Helics Web page"
	"https://www.youtube.com/channel/UCPa81c4BVXEYXt2EShTzbcg" "TDC YouTube channel")
else (WIN32)
	set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/docs/img/HELICS.ico")
endif(WIN32)

if (APPLE)
    set(CPACK_BUNDLE_NAME "libhelics")
    configure_file("${PROJECT_SOURCE_DIR}/config/Info.plist.in" "${CMAKE_CURRENT_BINARY_DIR}/Info.plist" @ONLY)
    set(CPACK_BUNDLE_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist")
    set(CPACK_BUNDLE_ICON "${PROJECT_SOURCE_DIR}/docs/img/HELICS.ico")
endif (APPLE)

set(CPACK_SOURCE_IGNORE_FILES  "/Build*/;/build*/;/.git/")
#THIS LINE MUST BE LAST
include(CPack)
endif(ENABLE_PACKAGE_BUILD)
