#
# Copyright © 2017-2019, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause
#

cmake_minimum_required(VERSION 3.4)
cmake_policy(VERSION 3.4)

project(HELICS VERSION 2.2.0)

# -----------------------------------------------------------------------------
# HELICS Version number
# -----------------------------------------------------------------------------
set(HELICS_VERSION_BUILD)
set(HELICS_DATE "07-26-19")

set(
    HELICS_VERSION_UNDERSCORE
    "${HELICS_VERSION_MAJOR}_${HELICS_VERSION_MINOR}_${HELICS_VERSION_PATCH}"
)
if(HELICS_VERSION_BUILD)
    set(HELICS_VERSION "${HELICS_VERSION}-${HELICS_VERSION_BUILD}")
    set(
        HELICS_VERSION_UNDERSCORE
        "${HELICS_VERSION_UNDERSCORE}-${HELICS_VERSION_BUILD}"
    )
endif()
set(HELICS_VERSION_STRING "${HELICS_VERSION} (${HELICS_DATE})")

# -----------------------------------------------------------------------------
# set the module path and include some common macros
# -----------------------------------------------------------------------------


if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	set(HELICS_ROOT_PROJ ON)
else()
    set(HELICS_ROOT_PROJ OFF)
endif()

if (HELICS_ROOT_PROJ)
set(CMAKE_MODULE_PATH
     ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/config/cmake/"
)
else()
	set(ORIGINAL_MODULE_PATH ${CMAKE_MODULE_PATH})
	set(CMAKE_MODULE_PATH
    "${PROJECT_SOURCE_DIR}/config/cmake/" ${CMAKE_MODULE_PATH}
)
endif()
include(extraMacros)
include(CMakeDependentOption)
include(copy_key_files)
# include(BuildType)
# -----------------------------------------------------------------------------
# set the install path to a local directory
# -----------------------------------------------------------------------------
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(WIN32)
        if(MSYS)
            set(
                CMAKE_INSTALL_PREFIX
                "/usr/local/helics_${HELICS_VERSION_UNDERSCORE}/"
                CACHE PATH "default install path"
                FORCE
            )
        else(MSYS)
            set(
                CMAKE_INSTALL_PREFIX
                "C:/local/helics_${HELICS_VERSION_UNDERSCORE}/"
                CACHE PATH "default install path"
                FORCE
            )
        endif(MSYS)
    endif(WIN32)
endif()

if (HELICS_ROOT_PROJ)
if (DEFINED BUILD_TESTING)
option(BUILD_HELICS_TESTS "Enable the test executables to be built" ${BUILD_TESTING})
else()
option(BUILD_HELICS_TESTS "Enable the test executables to be built" ON)
endif()

option(BUILD_HELICS_BOOST_TESTS "Enable the test executables built using boost unit tests" ON)
option(BUILD_HELICS_EXAMPLES "Enable the example executables to be built" ON)

mark_as_advanced(BUILD_HELICS_BOOST_TESTS)

else()
endif()

option(HELICS_WITH_CMAKE_PACKAGE "Generate and install cmake package files" ${HELICS_ROOT_PROJ})
mark_as_advanced(HELICS_WITH_CMAKE_PACKAGE)


# Install instructions for this target
if(HELICS_WITH_CMAKE_PACKAGE)
    set(HELICS_EXPORT_COMMAND EXPORT helics-targets)
else(HELICS_WITH_CMAKE_PACKAGE)
    set(HELICS_EXPORT_COMMAND)
endif(HELICS_WITH_CMAKE_PACKAGE)

# enable testing
if(BUILD_HELICS_TESTS)
    include(CTest)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(
    DISABLE_C_SHARED_LIB OFF
    "turn off building of the C shared interface"
)
mark_as_advanced(DISABLE_C_SHARED_LIB)

option(BUILD_SHARED_LIBS "install shared libraries for the CXX interface" OFF)

if(NOT DISABLE_C_SHARED_LIB OR BUILD_SHARED_LIBS)
    set(BUILD_HELICS_SHARED_LIBS ON)
endif()

cmake_dependent_option(
    USE_POSITION_INDEPENDENT_CODE
    "Build the libraries with Position independent code Useful if only building the static library and it will be used later in a shared library"
    OFF
    "NOT BUILD_HELICS_SHARED_LIBS"
    ON
)

if(BUILD_HELICS_SHARED_LIBS OR USE_POSITION_INDEPENDENT_CODE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# -----------------------------------------------------------------------------
# General project wide configuration for debug postfix
# -----------------------------------------------------------------------------
if(NOT NO_DEBUG_POSFIX)
    if(NOT CMAKE_DEBUG_POSTFIX)
        set(CMAKE_DEBUG_POSTFIX d)
    endif()
endif()

# add a baseline library for underlying dependencies and flags
add_library(helics_base INTERFACE)
add_library(helics_base_includes INTERFACE)

target_link_libraries(helics_base INTERFACE helics_base_includes)
# Prohibit in-source build
if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
    message(
        FATAL_ERROR
            "In-source build is not supported. Please, use an empty directory for building the project."
    )
endif()

if (NOT TARGET compile_flags_target)
   add_library(compile_flags_target INTERFACE)
endif()

if (HELICS_ROOT_PROJ)
	include(compiler_flags)
	message(STATUS "setting version build options to \"${CXX_STANDARD_FLAG}\"")
	if(CXX_STANDARD_FLAG)
		if(MSVC)
			target_compile_options(helics_base_includes INTERFACE ${CXX_STANDARD_FLAG})
		else()
			target_compile_options(
				helics_base_includes
				INTERFACE $<$<COMPILE_LANGUAGE:CXX>:${CXX_STANDARD_FLAG}>
			)
		endif(MSVC)
	endif(CXX_STANDARD_FLAG)

endif()

 target_compile_options(helics_base_includes INTERFACE -DENABLE_BOOST_IOSTREAMS)

 if (HELICS_ROOT_PROJ)
cmake_dependent_option(
    STATIC_STANDARD_LIB
    "Link against a static standard lib"
    OFF
    "NOT USE_LIBCXX"
    OFF
)
include(ucm)
mark_as_advanced(STATIC_STANDARD_LIB)
if(STATIC_STANDARD_LIB)
    ucm_set_runtime(STATIC)
else()
    ucm_set_runtime(DYNAMIC)
endif()

if(MSVC)
    option(EMBEDDED_DEBUG_INFO "embed debug info into obj files" OFF)
    mark_as_advanced(EMBEDDED_DEBUG_INFO)
    if(EMBEDDED_DEBUG_INFO)
        ucm_set_embedded_debug(EMBEDDED)
    else()
        ucm_set_embedded_debug(EXTERNAL)
    endif()
endif(MSVC)
endif (HELICS_ROOT_PROJ)
# -------------------------------------------------------------
# add coverage target
# -------------------------------------------------------------
if(BUILD_HELICS_TESTS)
    cmake_dependent_option(
        TEST_CODE_COVERAGE
        "Build a target for testing code coverage"
        OFF
        "BUILD_HELICS_TESTS"
        OFF
    )
    if(TEST_CODE_COVERAGE)
        if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
            include(CodeCoverage)

            set(
                COVERAGE_EXCLUDES
                'usr/*'
                'dependencies/*'
                'ThirdParty/*'
                'tests/*'
                'interfaces/*'
                'examples/*'
            )
            setup_target_for_coverage(
                NAME
                helics_coverage # New target name
                EXECUTABLE
                CTest # Executable in PROJECT_BINARY_DIR
            )
        else()
            message(
                FATAL_ERROR
                    "CMAKE_BUILD_TYPE must be set to Coverage for testing code coverage"
            )
        endif()
    endif(TEST_CODE_COVERAGE)
endif()
mark_as_advanced(TEST_CODE_COVERAGE)

# -------------------------------------------------------------
# Get some configuration for C++17 as that becomes available
# -------------------------------------------------------------
# message(STATUS ${CMAKE_CXX_FLAGS})
set(CONFIGURE_TARGET_LOCATION ${HELICS_BINARY_DIR}/include/helics/)
include(configGenerator)

option(GENERATE_DOXYGEN_DOC "Generate Doxygen doc target" OFF)

if(GENERATE_DOXYGEN_DOC)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)

        show_variable(
            DOXYGEN_OUTPUT_DIR
            PATH
            "location to put Doxygen docs"
            "${PROJECT_BINARY_DIR}/docs"
        )
        configure_file(
            ${HELICS_CURRENT_SOURCE_DIR}/config/Doxyfile.in
            ${HELICS_CURRENT_BINARY_DIR}/Doxyfile @ONLY
        )
        add_custom_target(
            doc ${DOXYGEN_EXECUTABLE} ${HELICS_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${DOXYGET_OUTPUT_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    endif(DOXYGEN_FOUND)
endif(GENERATE_DOXYGEN_DOC)

# if(MINGW) add_definitions(-DBOOST_USE_WINDOWS_H)
# add_definitions(-DWIN32_LEAN_AND_MEAN) endif()

# -------------------------------------------------------------
# Update git submodules
# -------------------------------------------------------------
include(updateGitSubmodules)
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/fmtlib/CMakeLists.txt")
	submod_update(ThirdParty/fmtlib)
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/asio/asio/include/asio.hpp")
	submod_update(ThirdParty/asio)
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/jsoncpp/CMakeLists.txt")
	submod_update(ThirdParty/jsoncpp)
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/containers/gmlc/containers/BlockingQueue.hpp")
	submod_update(ThirdParty/containers)
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/concurrency/gmlc/concurrency/TriggerVariable.hpp")
	submod_update(ThirdParty/concurrency)
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/utilities/gmlc/utilities/stringOps.h")
	submod_update(ThirdParty/utilities)
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/units/units/units.hpp")
	submod_update(ThirdParty/units)
endif()

include(GNUInstallDirs)

# -------------------------------------------------------------
# BOOST  find the boost libraries
# -------------------------------------------------------------
include(addBoost)


option(ENABLE_MPI_CORE "Enable MPI networking library" OFF)
option(ENABLE_ZMQ_CORE "Enable ZeroMQ networking library" ON)
option(ENABLE_TCP_CORE "Enable TCP core types" ON)
option(ENABLE_UDP_CORE "Enable UDP core types" ON)
option(ENABLE_IPC_CORE "Enable Interprocess communication types" ON)
option(ENABLE_TEST_CORE "Enable test inprocess core type" ON)
mark_as_advanced(ENABLE_MPI_CORE ENABLE_ZMQ_CORE ENABLE_TCP_CORE ENABLE_UDP_CORE ENABLE_TEST_CORE ENABLE_IPC_CORE)

# -------------------------------------------------------------
# finding MPI
# -------------------------------------------------------------

if(ENABLE_MPI_CORE)
    include(addMPI)
    if(MPI_C_FOUND)
        target_link_libraries(helics_base INTERFACE MPI::MPI_C)
    else()
        message(FATAL_ERROR "MPI not found")
    endif(MPI_C_FOUND)
endif(ENABLE_MPI_CORE)

# -------------------------------------------------------------
# add threading support
# -------------------------------------------------------------
if(NOT WIN32)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
elseif(MSYS)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
else()
    set(CMAKE_USE_WIN32_THREADS_INIT ON)
endif()
find_package(Threads REQUIRED)

target_link_libraries(helics_base INTERFACE Threads::Threads)

if(UNIX AND NOT APPLE)
    target_link_libraries(helics_base INTERFACE rt)
endif()

# -------------------------------------------------------------
# add some additional libraries for MSYS
# -------------------------------------------------------------
if(MSYS)
    target_link_libraries(helics_base INTERFACE wsock32 ws2_32)
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(ENABLE_IPC_CORE FALSE)
		target_compile_definitions(helics_base_includes INTERFACE -DDISABLE_BOOST_IOSTREAMS)
    endif()
endif(MSYS)

# -------------------------------------------------------------
# Enable ZeroMQ
# -------------------------------------------------------------

# just check some internal options
if(BUILD_DEBUG_ONLY AND BUILD_RELEASE_ONLY)
    message(
        fatal
        "BOTH BUILD_DEBUG_ONLY AND BUILD_RELEASE_ONLY cannot be specified together otherwise nothing works"
    )
endif()

option(ENABLE_ZMQ_CORE "Enable ZeroMQ networking library" ON)
mark_as_advanced(ENABLE_ZMQ_CORE)
# If ZeroMQ library is enabled try to locate it and link against it

if(ENABLE_ZMQ_CORE)
    include(addZeroMQ)

    if(ZeroMQ_FOUND)
        if(ZMQ_USE_STATIC_LIBRARY)
            set(ZeroMQ_DEPENDENCY libzmq-static)
            target_compile_definitions(
                helics_base_includes INTERFACE -DZMQ_STATIC
            )
        else()
            set(ZeroMQ_DEPENDENCY libzmq)
        endif()
        # message(STATUS "zmq dep ${ZeroMQ_DEPENDENCY}")
	    target_link_libraries(helics_base INTERFACE ${ZeroMQ_DEPENDENCY})
    else(ZeroMQ_FOUND)
        message(FATAL_ERROR "ZeroMQ not found")
    endif(ZeroMQ_FOUND)
endif()

# -------------------------------------------------------------
# global include directories
# -------------------------------------------------------------
target_include_directories(
    helics_base_includes
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty/containers>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty/concurrency>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty/utilities>
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty/utilities/gmlc/utilities>

)
# the utilities/gmlc is to account for the transfer of the header to a known location on install

target_include_directories(
    helics_base_includes SYSTEM
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty>
        $<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>
)

# -------------------------------------------------------------
# Asio include directories
# -------------------------------------------------------------
target_compile_definitions(helics_base_includes INTERFACE "-DASIO_STANDALONE")
target_include_directories(
    helics_base_includes SYSTEM
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty/asio/asio/include>
)

# -------------------------------------------------------------
# setting the RPATH
# -------------------------------------------------------------
# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH FALSE)

# when building, don't use the install RPATH already (but later on when
# installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

set(CMAKE_MACOSX_RPATH ON)

# add the automatically determined parts of the RPATH which point to directories
# outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
list(
    FIND
        CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
        "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}" isSystemDir
)
if("${isSystemDir}" STREQUAL "-1")
    list(
        APPEND
            CMAKE_INSTALL_RPATH CMAKE_INSTALL_RPATH
            "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}"
    )
endif()

list(
    FIND
        CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
        "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir
)
if("${isSystemDir}" STREQUAL "-1")
    list(
        APPEND
            CMAKE_INSTALL_RPATH
            "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}"
    )
endif()

# add the local directory to the rpath
list(APPEND CMAKE_INSTALL_RPATH "./")

if(NOT WIN32)
    if(ENABLE_ZMQ_CORE AND NOT ZMQ_USE_STATIC_LIBRARY)
        get_target_property(zmqlibfile libzmq IMPORTED_LOCATION)
        if(NOT zmqlibfile)
            get_target_property(zmqlibfile libzmq IMPORTED_LOCATION_RELEASE)
        endif()
        # message(STATUS "zmqlib file =${zmqlibfile}")
        get_filename_component(zmqdir ${zmqlibfile} DIRECTORY)
        # message(STATUS "zmqdir path =${zmqdir}")
        if(APPLE)
            # TODO: Make sure it works for building ZMQ from source
            # automatically using CMake
            list(APPEND CMAKE_INSTALL_RPATH ${ZeroMQ_INSTALL_PATH}/lib)
            list(APPEND CMAKE_BUILD_RPATH ${ZeroMQ_INSTALL_PATH}/lib)
        endif()
        list(APPEND CMAKE_INSTALL_RPATH "${zmqdir}")
        list(APPEND CMAKE_BUILD_RPATH "${zmqdir}")
    endif()
endif()

option(
    BUILD_APP_LIBRARY
    "Build helics application library (a static library containing objects for broker, player, recorder, and helics_apps"
    ON
)

if (HELICS_ROOT_PROJ)
option(
    BUILD_APP_EXECUTABLES
    "Build helics applications to run various apps, including broker, player, recorder, and helics_apps"
    ${BUILD_APP_LIBRARY}
)
else()
SET(BUILD_APP_EXECUTABLES OFF)
endif()

option(ENABLE_LOGGING "enable normal, debug, and trace logging in HELICS" ON)

cmake_dependent_option(
    ENABLE_TRACE_LOGGING
    "enable trace logging"
    ON
    "ENABLE_LOGGING"
    ON
)
cmake_dependent_option(
    ENABLE_DEBUG_LOGGING
    "enable debug logging"
    ON
    "ENABLE_LOGGING"
    ON
)
mark_as_advanced(ENABLE_TRACE_LOGGING)
mark_as_advanced(ENABLE_DEBUG_LOGGING)

# -----------------------------------------------------------------------------
# create the fmt header only targets
# -----------------------------------------------------------------------------
include(addfmt)

# --------------------------------------------------------------
#  Create the target for jsoncpp
# -----------------------------------------------------------
include(addJsoncpp)

if (JSONCPP_OBJLIB)
target_include_directories(
    helics_base_includes SYSTEM
    INTERFACE
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:jsoncpp_headers,INTERFACE_INCLUDE_DIRECTORIES>>
)
set_target_properties(jsoncppobj_lib PROPERTIES FOLDER Extern)
else()
target_include_directories(
    helics_base_includes SYSTEM
    INTERFACE
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:jsoncpp_lib,INTERFACE_INCLUDE_DIRECTORIES>>
)
set_target_properties(jsoncpp_lib PROPERTIES FOLDER Extern)
endif()

# --------------------------------------------------------------
#  Create the target for unitslib
# -----------------------------------------------------------
include(addUnits)

# -----------------------------------------------------------------------------
# create utilities target
# -----------------------------------------------------------------------------

set(GMLC_UTILITIES_OBJECT_LIB ON CACHE INTERNAL "")
set(GMLC_UTILITIES_STATIC_LIB OFF CACHE INTERNAL "")
if (NOT GMLC_UTILITIES_OBJECT_LIB)
	set(UTILITIES_LIBRARY_EXPORT_COMMAND ${HELICS_EXPORT_COMMAND} CACHE INTERNAL "")
endif()

add_subdirectory(ThirdParty/utilities)

mark_as_advanced(GMLC_UTILITIES_OBJECT_LIB)
mark_as_advanced(GMLC_UTILITIES_STATIC_LIB)

if (GMLC_UTILITIES_OBJECT_LIB)
   if (NOT HELICS_ROOT_PROJ)
	add_library(HELICS::utilities INTERFACE IMPORTED )
	set_target_properties(HELICS::utilities PROPERTIES
       INTERFACE_INCLUDE_DIRECTORIES $<BUILD_INTERFACE:ThirdParty/utilities/gmlc>
    )
	set_target_properties(HELICS::utilities PROPERTIES
       INTERFACE_LINK_LIBRARIES "HELICS::helics-static"
    )
   endif()
   set_target_properties(gmlc_utilities_obj PROPERTIES FOLDER Extern)
   message(STATUS "Boost include=${Boost_INCLUDE_DIRS}")
	target_include_directories(gmlc_utilities_obj SYSTEM PRIVATE $<BUILD_INTERFACE:${Boost_INCLUDE_DIRS}>)

else()
   add_library(HELICS::utilities ALIAS gmlc_utilities)
   set_target_properties(gmlc_utilities PROPERTIES FOLDER Extern)
   message(STATUS "Boost include=${Boost_INCLUDE_DIRS}")
target_include_directories(gmlc_utilities SYSTEM PRIVATE $<BUILD_INTERFACE:${Boost_INCLUDE_DIRS}>)

endif()

file(GLOB UTILITIES_HEADERS_INSTALL_H ${PROJECT_SOURCE_DIR}/ThirdParty/utilities/gmlc/utilities/*.h)
file(GLOB UTILITIES_HEADERS_INSTALL_HPP ${PROJECT_SOURCE_DIR}/ThirdParty/utilities/gmlc/utilities/*.hpp)
file(GLOB UTILITIES_HEADERS_INSTALL_EXTRA ${PROJECT_SOURCE_DIR}/ThirdParty/utilities/gmlc/utilities/extra/*.hpp)

install(
  FILES ${UTILITIES_HEADERS_INSTALL_H} ${UTILITIES_HEADERS_INSTALL_HPP}
  ${PROJECT_SOURCE_DIR}/ThirdParty/utilities/LICENSE
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics/utilities/
  COMPONENT headers
)

install(
  FILES ${UTILITIES_HEADERS_INSTALL_EXTRA}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics/utilities/extra
  COMPONENT headers
)

# -----------------------------------------------------------------------------
# CMAKE Subdirectories
# -----------------------------------------------------------------------------

add_subdirectory(src)

# -----------------------------------------------------------------------------
# Build the tests
# -----------------------------------------------------------------------------

if(BUILD_HELICS_TESTS AND BUILD_TESTING AND ENABLE_TEST_CORE)
    mark_as_advanced(BUILD_TESTING)
    add_subdirectory(tests)
endif()

# -----------------------------------------------------------------------------
# Setup Examples
# -----------------------------------------------------------------------------
if(BUILD_HELICS_EXAMPLES)
    add_subdirectory(examples)
endif(BUILD_HELICS_EXAMPLES)

if(ENABLE_ZMQ_CORE AND NOT ZMQ_USE_STATIC_LIBRARY)
    if(WIN32)
        install(
            FILES $<TARGET_LINKER_FILE:libzmq>
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
            COMPONENT libs
        )
    endif()
endif()

# -------------------------------------------------------------
# Enable clang analysis and formatting tools
# -------------------------------------------------------------
if (HELICS_ROOT_PROJ)
option(
    ENABLE_CLANG_TOOLS
    "if clang is found enable some custom targets for clang formatting and tidy"
    OFF
)
mark_as_advanced(ENABLE_CLANG_TOOLS)
if(ENABLE_CLANG_TOOLS)
    include(clang-cxx-dev-tools)
endif(ENABLE_CLANG_TOOLS)

add_subdirectory(interfaces)


# add base swig interface files as install option, regardless of current interfaces built
install(
    FILES interfaces/helics.i interfaces/output.i
    DESTINATION ${CMAKE_INSTALL_DATADIR}/helics/swig
    COMPONENT swig
)


add_subdirectory(docs)

endif(HELICS_ROOT_PROJ)

install(TARGETS helics_base ${HELICS_EXPORT_COMMAND})
install(TARGETS helics_base_includes ${HELICS_EXPORT_COMMAND})
install(TARGETS compile_flags_target ${HELICS_EXPORT_COMMAND})
set(
    helics_static_file
    "${CMAKE_STATIC_LIBRARY_PREFIX}helics-static${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
set(
    helics_static_file_debug
    "${CMAKE_STATIC_LIBRARY_PREFIX}helics-static${CMAKE_DEBUG_POSTFIX}${CMAKE_STATIC_LIBRARY_SUFFIX}"
)
if(BUILD_SHARED_LIBS)
    set(
        helics_cxx_shared_file
        "${CMAKE_SHARED_LIBRARY_PREFIX}helics-shared${CMAKE_SHARED_LIBRARY_SUFFIX}"
    )
    set(
        helics_cxx_shared_file_debug
        "${CMAKE_SHARED_LIBRARY_PREFIX}helics-shared${CMAKE_DEBUG_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}"
    )
else(BUILD_SHARED_LIBS)
    set(helics_cxx_shared_file)
    set(helics_cxx_shared_file_debug)
endif(BUILD_SHARED_LIBS)

if(BUILD_C_SHARED_LIB OR INTERFACE_BUILD)
    set(
        helics_c_shared_file
        "${CMAKE_SHARED_LIBRARY_PREFIX}helicsSharedLib${CMAKE_SHARED_LIBRARY_SUFFIX}"
    )
    set(
        helics_c_shared_file_debug
        "${CMAKE_SHARED_LIBRARY_PREFIX}helicsSharedLib${CMAKE_DEBUG_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}"
    )
else()
    set(helics_c_shared_file)
    set(helics_c_shared_file_debug)
endif()



# Copy required CMake module files to build tree for importing build tree with find_package
file(COPY config/cmake/addMPI.cmake config/cmake/FindZeroMQ.cmake DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

if (HELICS_WITH_CMAKE_PACKAGE)

set(
    HELICS_CMAKECONFIG_INSTALL_DIR
    "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
    CACHE STRING "install path for HELICSConfig.cmake"
)
mark_as_advanced(HELICS_CMAKECONFIG_INSTALL_DIR)

# Export targets for importing build tree with find_package
export(EXPORT helics-targets
    NAMESPACE HELICS::
    FILE ${PROJECT_BINARY_DIR}/helics-targets.cmake
)

install(
    EXPORT helics-targets
    NAMESPACE HELICS::
    DESTINATION ${HELICS_CMAKECONFIG_INSTALL_DIR}
    COMPONENT libs
)
endif()
# -----------------------------------------------------------------------------
# Setup configure.h file for accessing configure options
# -----------------------------------------------------------------------------
configure_file(
    "config/helics-config.h.in"
    "${HELICS_BINARY_DIR}/include/helics/helics-config.h"
)
install(
    FILES ${HELICS_BINARY_DIR}/include/helics/helics-config.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics
    COMPONENT headers
)
install(
    FILES ${HELICS_BINARY_DIR}/include/helics/compiler-config.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics
    COMPONENT headers
)

option(HELICS_INSTALL_CEREAL "Install a copy of the cereal headers used for compiling and a CMake target to include them" ${MSVC})

mark_as_advanced(HELICS_INSTALL_CEREAL)

if (HELICS_INSTALL_CEREAL)
install(
    DIRECTORY ThirdParty/cereal
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics/external/optional
    COMPONENT cereal
)
add_library(cereal INTERFACE)
add_library(HELICS::cereal ALIAS cereal)
target_include_directories(cereal
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/helics/external/optional>
)
install(TARGETS cereal ${HELICS_EXPORT_COMMAND} COMPONENT cereal)
endif()

install(
    DIRECTORY ThirdParty/helics/external
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/helics
    COMPONENT headers
)
if (HELICS_WITH_CMAKE_PACKAGE)

   include(CMakePackageConfigHelpers)

   configure_package_config_file(
    config/${PROJECT_NAME}Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    INSTALL_DESTINATION
    ${HELICS_CMAKECONFIG_INSTALL_DIR}
   )
   write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${HELICS_VERSION}
    COMPATIBILITY SameMajorVersion
   )
endif()

if (NOT HELICS_ROOT_PROJ)
	set_target_properties(helics_apps PROPERTIES FOLDER helics)
	set_target_properties(application_api helics_common helics_core helics-static PROPERTIES FOLDER helics)
endif()

if (HELICS_WITH_CMAKE_PACKAGE)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${HELICS_CMAKECONFIG_INSTALL_DIR}
    COMPONENT libs
)
# install required additional cmake files
install(
    FILES
        config/cmake/addMPI.cmake
        config/cmake/FindZeroMQ.cmake
    DESTINATION ${HELICS_CMAKECONFIG_INSTALL_DIR}
    COMPONENT libs
)
endif()

# combine 3rd party license files for install package
include(combineLicenses)
set(LICENSE_LIST
	"asio" "${PROJECT_SOURCE_DIR}/ThirdParty/asio/asio/LICENSE_1_0.txt"
	"cereal" "${PROJECT_SOURCE_DIR}/ThirdParty/cereal/LICENSE"
	"GMLC Concurrency" "${PROJECT_SOURCE_DIR}/ThirdParty/concurrency/LICENSE"
	"libGuarded" "${PROJECT_SOURCE_DIR}/ThirdParty/concurrency/gmlc/libguarded/LICENSE"
	"GMLC Containers" "${PROJECT_SOURCE_DIR}/ThirdParty/containers/LICENSE"
	"cppzmq" "${PROJECT_SOURCE_DIR}/ThirdParty/cppzmq/LICENSE"
	"fmt" "${PROJECT_SOURCE_DIR}/ThirdParty/fmtlib/LICENSE.rst"
	"units" "${PROJECT_SOURCE_DIR}/ThirdParty/units/LICENSE"
	"Google Test" "${PROJECT_SOURCE_DIR}/ThirdParty/googletest/LICENSE"
	"JsonCpp" "${PROJECT_SOURCE_DIR}/ThirdParty/jsoncpp/LICENSE"
	"tinytoml" "${PROJECT_SOURCE_DIR}/ThirdParty/toml/LICENSE"
	"GMLC Utilities" "${PROJECT_SOURCE_DIR}/ThirdParty/utilities/LICENSE"
)

if (EXISTS ${PROJECT_BINARY_DIR}/_deps/libzmq-src)
	list(APPEND LICENSE_LIST "ZeroMQ" "${PROJECT_BINARY_DIR}/_deps/libzmq-src/COPYING")
endif()

combineLicenses(${PROJECT_BINARY_DIR}/THIRDPARTY_LICENSES ${LICENSE_LIST})

install(
    FILES LICENSE NOTICE CHANGELOG.md README.md ${PROJECT_BINARY_DIR}/THIRDPARTY_LICENSES
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    COMPONENT libs
)

# -------------------------------------------------------------
# CPack
# -------------------------------------------------------------
option(
    ENABLE_PACKAGE_BUILD
    "Add projects for making packages and installers for HELICS" OFF
)

if(ENABLE_PACKAGE_BUILD)

	if(CMAKE_VERSION VERSION_LESS 3.13)
		if (ZMQ_SUBPROJECT OR ZMQ_FORCE_SUBPROJECT)
			if (ENABLE_ZMQ_CORE)
				message(FATAL_ERROR "CMAKE 3.13 or higher is required to package a subproject of ZeroMQ with HELICS")
			endif()
		endif()
	endif()
    set(CPACK_PACKAGE_NAME "Helics")
    set(CPACK_PACKAGE_VENDOR "GMLC")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "HELICS")
    set(CPACK_PACKAGE_VERSION "${HELICS_VERSION}")
    set(CPACK_PACKAGE_VERSION_MAJOR ${HELICS_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${HELICS_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${HELICS_VERSION_PATCH})
    set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "HELICS")

    set(
        CPACK_COMPONENTS_ALL
        applications
        headers
        libs
        Runtime
        matlab
        python
        java
        octave
        csharp
		swig
        cereal
    )
    if(WIN32)
        set(CPACK_RESOURCE_FILE_LICENSE "${HELICS_SOURCE_DIR}\\\\LICENSE")
    else(WIN32)
        set(CPACK_RESOURCE_FILE_LICENSE "${HELICS_SOURCE_DIR}/LICENSE")
    endif(WIN32)

    set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Application")
    set(CPACK_COMPONENT_LIBS_DISPLAY_NAME "Libraries")
    set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "Headers")
    set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "Runtime Libraries")

    set(CPACK_COMPONENT_MATLAB_GROUP interfaces)
    set(CPACK_COMPONENT_JAVA_GROUP interfaces)
    set(CPACK_COMPONENT_OCTAVE_GROUP interfaces)
    set(CPACK_COMPONENT_PYTHON_GROUP interfaces)
    set(CPACK_COMPONENT_CSHARP_GROUP interfaces)

    set(
        CPACK_COMPONENT_APPLICATION_DESCRIPTION
        "Executables and helper applications for HELICS"
    )
    set(
        CPACK_COMPONENT_LIBS_DESCRIPTION
        "Libraries for compiling and linking with HELICS"
    )
    set(
        CPACK_COMPONENT_HEADERS_DESCRIPTION
        "Headers for linking and compiling with HELICS"
    )
    set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "Runtime libraries for HELICS")
    set(CPACK_COMPONENT_CEREAL_DESCRIPTION "cereal C++11 serialization library [no support for other language interfaces]")
    set(CPACK_COMPONENT_SWIG_DESCRIPTION "Basic swig interface files for HELICS for building 3rd party language interfaces")
    set(
        CPACK_COMPONENT_GROUP_INTERFACES_DESCRIPTION
        "additional language interfaces for HELICS"
    )

    set(CPACK_COMPONENT_LIBS_DEPENDS headers)
    set(CPACK_COMPONENT_RUNTIME_REQUIRED ON)

    set(
        CPACK_PACKAGE_EXECUTABLES
        "helics_broker"
        "Helics Broker"
        "helics_app"
        "Helics app executable"
        "helics_recorder"
        "Helics recorder"
        "helics_player"
        "Helics Player"
    )

    if(INSTALL_SYSTEM_LIBRARIES)
        include(InstallRequiredSystemLibraries)
    endif()

    if(WIN32)
        set(
            CPACK_PACKAGE_ICON
            "${HELICS_SOURCE_DIR}\\\\docs\\\\img\\\\HELICS.ico"
        )
        set(CPACK_NSIS_MUI_ICON "${HELICS_SOURCE_DIR}/docs/img/HELICS.ico")
        set(CPACK_NSIS_INSTALL_ROOT "C:\\\\local")
        set(
            CPACK_NSIS_URL_INFO_ABOUT
	    "https://www.github.com/GMLC-TDC/HELICS"
        )
        set(CPACK_NSIS_HELP_LINK "https://helics.readthedocs.io/en/latest")
        set(CPACK_NSIS_CONTACT "helicsteam@helics.org")
        set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
		set(CPACK_PACKAGE_INSTALL_DIRECTORY helics_${HELICS_VERSION_UNDERSCORE})
        set(CPACK_NSIS_MODIFY_PATH ON)
        set(CPACK_NSIS_EXECUTABLES_DIRECTORY ${CMAKE_INSTALL_BINDIR})
        set(
            CPACK_NSIS_MENU_LINKS
	    "https://www.github.com/GMLC-TDC/HELICS"
            "HELICS Github"
            "https://helics.readthedocs.io/en/latest"
            "Helics Documentation"
            "https://www.helics.org"
            "Helics Web page"
            "https://www.youtube.com/channel/UCPa81c4BVXEYXt2EShTzbcg"
            "TDC YouTube channel"
        )
    else(WIN32)
        set(CPACK_PACKAGE_ICON "${HELICS_SOURCE_DIR}/docs/img/HELICS.ico")
    endif(WIN32)

    if(APPLE)
        set(CPACK_BUNDLE_NAME "libhelics")
        configure_file(
            "${HELICS_SOURCE_DIR}/config/Info.plist.in"
            "${CMAKE_CURRENT_BINARY_DIR}/Info.plist" @ONLY
        )
        set(CPACK_BUNDLE_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist")
        set(CPACK_BUNDLE_ICON "${HELICS_SOURCE_DIR}/docs/img/HELICS.ico")
    endif(APPLE)

    set(CPACK_SOURCE_IGNORE_FILES "/Build*/;/build*/;/.git/")
    # THIS LINE MUST BE LAST
    include(CPack)
endif(ENABLE_PACKAGE_BUILD)

if (NOT HELICS_ROOT_PROJ)
	#restore the original module path
	set(CMAKE_MODULE_PATH ${ORIGINAL_MODULE_PATH})
endif()