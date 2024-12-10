# ~~~
# Copyright (c) 2017-2024, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~

cmake_minimum_required(VERSION 3.22...3.31)

# Install dependencies using vcpkg if VCPKG_ROOT is set and no CMake Toolchain file is given vcpkg
# installation on a system doesn't set VCPKG_ROOT, so setting it should be like an opt-in for users
option(HELICS_DISABLE_VCPKG "Force CMake to ignore VCPKG_ROOT even if it is set" OFF)
mark_as_advanced(HELICS_DISABLE_VCPKG)
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE AND NOT HELICS_DISABLE_VCPKG)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

message(STATUS "Generating with CMAKE version ${CMAKE_VERSION}")
project(HELICS VERSION 3.6.0)

# -----------------------------------------------------------------------------
# HELICS Version number
# -----------------------------------------------------------------------------
set(HELICS_VERSION_BUILD)
# use ISO date YYYY-MM-DD
set(HELICS_DATE "2024-12-09")

set(HELICS_VERSION_UNDERSCORE
    "${HELICS_VERSION_MAJOR}_${HELICS_VERSION_MINOR}_${HELICS_VERSION_PATCH}"
)
if(HELICS_VERSION_BUILD)
    set(HELICS_VERSION "${HELICS_VERSION}-${HELICS_VERSION_BUILD}")
    set(HELICS_VERSION_UNDERSCORE "${HELICS_VERSION_UNDERSCORE}-${HELICS_VERSION_BUILD}")
endif()

set(HELICS_COMPILER_VERSION
    "${CMAKE_GENERATOR} ${CMAKE_GENERATOR_PLATFORM} ${CMAKE_SYSTEM}:${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}"
)
message(STATUS "SYSTEM INFO -> ${HELICS_COMPILER_VERSION}")

# -----------------------------------------------------------------------------
# set the module path and include some common macros
# -----------------------------------------------------------------------------

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/config/cmake/"
                          "${PROJECT_SOURCE_DIR}/ThirdParty/cmake/"
    )
else()
    set(${PROJECT_NAME}_ORIGINAL_MODULE_PATH ${CMAKE_MODULE_PATH})
    set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/config/cmake/"
                          "${PROJECT_SOURCE_DIR}/ThirdParty/cmake/" ${CMAKE_MODULE_PATH}
    )

endif()

include(commonBuildPaths)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
else()
    get_property(helics-use-folders GLOBAL PROPERTY USE_FOLDERS)
endif()

if(NOT HELICS_DISABLE_GIT_OPERATIONS)
    # generate a version description based on git tags
    include(version_describe)
    git_version_describe(${HELICS_SOURCE_DIR} HELICS_VERSION_DESCRIPTION)
endif()

if(HELICS_VERSION_DESCRIPTION)
    message(STATUS "helics version tag description is ${HELICS_VERSION_DESCRIPTION}")
    string(TIMESTAMP current_date "%Y-%m-%d")
    set(HELICS_VERSION_STRING "${HELICS_VERSION}-${HELICS_VERSION_DESCRIPTION} (${current_date})")
else()
    set(HELICS_VERSION_STRING "${HELICS_VERSION} (${HELICS_DATE})")
endif()

include(extraMacros)
include(CMakeDependentOption)
include(copy_key_files)
include(CTest)

option(
    HELICS_ENABLE_PYTHON_BUILD_SCRIPTS
    "HELICS build systems can use python to automatically generate some files.  This is not required but may be useful if you are a developer and frequently modifying some files"
    OFF
)
mark_as_advanced(HELICS_ENABLE_PYTHON_BUILD_SCRIPTS)

if(HELICS_ENABLE_PYTHON_BUILD_SCRIPTS)
    find_package(Python COMPONENTS Interpreter)
endif()

# allow BOOST library inclusion to be turned off completely; this option will disable the IPC core
option(HELICS_DISABLE_BOOST OFF "disable all references to the Boost C++ libraries")
mark_as_advanced(HELICS_DISABLE_BOOST)

# allow ASIO library inclusion to be turned off completely; this option will disable the UDP and TCP
# core as well as the timeout detection and real-time features of HELICS
option(HELICS_DISABLE_ASIO OFF "disable all references to the ASIO C++ libraries")
mark_as_advanced(HELICS_DISABLE_ASIO)

# we want to acknowledge this flag since it is standard CMake but it can cause issues in submodules
# so we need to set other variables
if(NOT DEFINED BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
endif()
if(BUILD_SHARED_LIBS)
    set(HELICS_BUILD_CXX_SHARED_LIB ON CACHE BOOL "")
    set(HELICS_DISABLE_C_SHARED_LIB OFF CACHE BOOL "")
    set(old_build_shared ON)
    # we need this to prevent submodules from building shared libs
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
endif()

cmake_dependent_option(
    HELICS_BUILD_TESTS "Enable the HELICS test executables to be built" OFF
    "BUILD_TESTING;CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" OFF
)

cmake_dependent_option(
    HELICS_BUILD_EXAMPLES "Build HELICS examples" OFF "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME"
    OFF
)

cmake_dependent_option(
    HELICS_BUILD_BENCHMARKS "Build HELICS Benchmarks" OFF
    "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" OFF
)

cmake_dependent_advanced_option(
    HELICS_WITH_CMAKE_PACKAGE "Generate and install cmake package files" ON
    "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" OFF
)

option(HELICS_BINARY_ONLY_INSTALL "only install the helics binary executables" OFF)
mark_as_advanced(HELICS_BINARY_ONLY_INSTALL)

# Install instructions for this target
if(HELICS_WITH_CMAKE_PACKAGE)
    set(HELICS_EXPORT_COMMAND EXPORT helics-targets)
else(HELICS_WITH_CMAKE_PACKAGE)
    set(HELICS_EXPORT_COMMAND)
endif(HELICS_WITH_CMAKE_PACKAGE)

option(HELICS_DISABLE_C_SHARED_LIB OFF "turn off building of the HELICS C shared library interface")
mark_as_advanced(HELICS_DISABLE_C_SHARED_LIB)

option(HELICS_BUILD_CXX_SHARED_LIB "build the shared libraries for the HELICS CXX interface" OFF)

cmake_dependent_advanced_option(
    HELICS_USE_POSITION_INDEPENDENT_CODE
    "Build the libraries with Position independent code Useful if only building the static library and it will be used later in a shared library"
    OFF
    "HELICS_DISABLE_C_SHARED_LIB;NOT HELICS_BUILD_CXX_SHARED_LIB"
    ON
)

cmake_dependent_option(
    HELICS_GENERATE_DOXYGEN_DOC "Generate HELICS Doxygen doc target" OFF
    "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" OFF
)

if(NOT HELICS_DISABLE_C_SHARED_LIB OR HELICS_BUILD_CXX_SHARED_LIB
   OR HELICS_USE_POSITION_INDEPENDENT_CODE
)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

option(HELICS_ENABLE_LOGGING "enable normal, debug, and trace logging in HELICS" ON)

cmake_dependent_advanced_option(
    HELICS_ENABLE_TRACE_LOGGING "enable trace logging" ON "HELICS_ENABLE_LOGGING" ON
)
cmake_dependent_advanced_option(
    HELICS_ENABLE_DEBUG_LOGGING "enable debug logging" ON "HELICS_ENABLE_LOGGING" ON
)

include(commonBuildFlags)

# add a baseline library for underlying dependencies and flags
add_library(helics_base INTERFACE)

add_library(HELICS::compile_flags_target ALIAS compile_flags_target)
add_library(HELICS::build_flags_target ALIAS build_flags_target)

get_target_property(EXTRA_BUILD_FLAGS build_flags_target INTERFACE_COMPILE_OPTIONS)

# -------------------------------------------------------------
# add coverage target
# -------------------------------------------------------------

cmake_dependent_advanced_option(
    HELICS_TEST_CODE_COVERAGE "Build a target for testing code coverage" OFF "HELICS_BUILD_TESTS"
    OFF
)

if(HELICS_BUILD_TESTS AND HELICS_TEST_CODE_COVERAGE)
    if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
        include(CodeCoverage)

        set(COVERAGE_EXCLUDES
            'usr/*'
            'dependencies/*'
            'ThirdParty/*'
            'tests/*'
            'interfaces/*'
            'examples/*'
            'benchmarks/*'
            'scripts/*'
        )
        setup_target_for_coverage(
            NAME helics_coverage # New target name
            EXECUTABLE CTest # Executable in PROJECT_BINARY_DIR
        )
    else()
        message(FATAL_ERROR "CMAKE_BUILD_TYPE must be set to Coverage for testing code coverage")
    endif()
endif()

if(HELICS_GENERATE_DOXYGEN_DOC)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)

        show_variable(
            DOXYGEN_OUTPUT_DIR PATH "location to put Doxygen docs" "${HELICS_BINARY_DIR}/docs"
        )
        configure_file(${HELICS_SOURCE_DIR}/config/Doxyfile.in ${HELICS_BINARY_DIR}/Doxyfile @ONLY)
        add_custom_target(
            helics_doxygen
            ${DOXYGEN_EXECUTABLE} ${HELICS_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${DOXYGET_OUTPUT_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
        set_target_properties(helics_doxygen PROPERTIES FOLDER docs)
    endif(DOXYGEN_FOUND)
endif(HELICS_GENERATE_DOXYGEN_DOC)

# -------------------------------------------------------------
# Update git submodules
# -------------------------------------------------------------
if(NOT HELICS_DISABLE_GIT_OPERATIONS)
    include(updateGitSubmodules)
    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/fmtlib/CMakeLists.txt")
        submod_update(ThirdParty/fmtlib)
    endif()

    if(NOT HELICS_DISABLE_ASIO)
        if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/asio/asio/include/asio.hpp")
            submod_update(ThirdParty/asio)
        endif()
    endif()

    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/containers/gmlc/containers/BlockingQueue.hpp")
        submod_update(ThirdParty/containers)
    endif()

    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/concurrency/gmlc/concurrency/Barrier.hpp")
        submod_update(ThirdParty/concurrency)
    endif()

    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/utilities/gmlc/utilities/stringOps.h")
        submod_update(ThirdParty/utilities)
    endif()

    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/networking/gmlc/networking/TcpServer.h")
        submod_update(ThirdParty/networking)
    endif()

    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/units/units/units.hpp")
        submod_update(ThirdParty/units)
    endif()

    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/toml/include/toml11/value.hpp")
        submod_update(ThirdParty/toml)
    endif()

    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/spdlog/CMakeLists.txt")
        submod_update(ThirdParty/spdlog)
    endif()

    set(IGNORE_MODULES libzmq)
    if(HELICS_BUILD_TESTS)
        if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/googletest/CMakeLists.txt")
            submod_update(ThirdParty/googletest)
        endif()
    else()
        list(APPEND IGNORE_MODULES googletest)
    endif()

    if(HELICS_BUILD_BENCHMARKS)
        if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/benchmark/CMakeLists.txt")
            submod_update(ThirdParty/benchmark)
        endif()
    else()
        list(APPEND IGNORE_MODULES benchmark)
    endif()

    check_submodule_status(IGNORE_MODULES)

endif() # NOT HELICS_DISABLE_GIT_OPERATIONS
include(GNUInstallDirs)

# -------------------------------------------------------------
# BOOST  find the boost libraries
# -------------------------------------------------------------

if(NOT HELICS_DISABLE_BOOST)
    include(addBoost)
endif()

# -------------------------------------------------------------
# TOML11  add the TOML interpreter
# -------------------------------------------------------------
if(NOT TARGET toml11::toml11)
    set(toml11_BUILD_TEST OFF CACHE INTERNAL "")
    set(toml11_TEST_WITH_ASAN OFF CACHE INTERNAL "")
    set(toml11_TEST_WITH_UBSAN OFF CACHE INTERNAL "")
    set(toml11_INSTALL OFF CACHE INTERNAL "")

    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.25)
        add_subdirectory(ThirdParty/toml EXCLUDE_FROM_ALL SYSTEM)
    else()
        add_subdirectory(ThirdParty/toml EXCLUDE_FROM_ALL)
    endif()
    hide_variable(TOML11_USE_UNRELEASED_TOML_FEATURES)
    hide_variable(TOML11_PRECOMPILE)
endif()

# -------------------------------------------------------------
# options for enabling specific core communication types
# -------------------------------------------------------------

option(HELICS_ENABLE_MPI_CORE "Enable MPI networking library" OFF)
cmake_dependent_advanced_option(
    HELICS_ENABLE_TCP_CORE "Enable TCP core types" ON "NOT HELICS_DISABLE_ASIO" OFF
)
cmake_dependent_advanced_option(
    HELICS_ENABLE_UDP_CORE "Enable UDP core types" ON "NOT HELICS_DISABLE_ASIO" OFF
)
if("${CMAKE_SYSTEM_NAME}" MATCHES ".*BSD")
    set(SYSTEM_IS_BSD ON)
endif()

cmake_dependent_advanced_option(
    HELICS_ENABLE_IPC_CORE "Enable Interprocess communication types" ON
    "NOT HELICS_DISABLE_BOOST;NOT SYSTEM_IS_BSD" OFF
)
cmake_dependent_advanced_option(
    HELICS_ENABLE_TEST_CORE "Enable test inprocess core type" OFF "NOT HELICS_BUILD_TESTS" ON
)
cmake_dependent_advanced_option(
    HELICS_ENABLE_INPROC_CORE "Enable inprocess core type" ON "NOT HELICS_BUILD_BENCHMARKS" ON
)
option(HELICS_ENABLE_ZMQ_CORE "Enable ZeroMQ networking library" ON)

mark_as_advanced(HELICS_ENABLE_MPI_CORE HELICS_ENABLE_ZMQ_CORE)

# -------------------------------------------------------------
# Some specialized configurations
# -------------------------------------------------------------

show_variable(
    HELICS_BUILD_CONFIGURATION
    STRING
    "specify a build configuration to use for some specialized systems that are not otherwise detectable"
    ""
)
mark_as_advanced(HELICS_BUILD_CONFIGURATION)

if(HELICS_BUILD_CONFIGURATION STREQUAL "PI")
    target_link_libraries(helics_base INTERFACE atomic)
endif()

# Add to list of enabled vcpkg features based on core type
if(HELICS_ENABLE_ZMQ_CORE)
    list(APPEND VCPKG_MANIFEST_FEATURES "zeromq")
endif()
if(HELICS_ENABLE_IPC_CORE)
    list(APPEND VCPKG_MANIFEST_FEATURES "ipc")
endif()
if(HELICS_ENABLE_MPI_CORE)
    list(APPEND VCPKG_MANIFEST_FEATURES "mpi")
endif()

# -------------------------------------------------------------
# finding MPI
# -------------------------------------------------------------

if(HELICS_ENABLE_MPI_CORE)
    include(addMPI)
    if(MPI_C_FOUND)
        target_link_libraries(helics_base INTERFACE MPI::MPI_C)
        if(MPI_CXX_FOUND)
            target_link_libraries(helics_base INTERFACE MPI::MPI_CXX)
        endif()
    else()
        message(FATAL_ERROR "MPI not found")
    endif(MPI_C_FOUND)
endif(HELICS_ENABLE_MPI_CORE)

# -------------------------------------------------------------
# add threading support
# -------------------------------------------------------------
if(MSYS OR CYGWIN OR NOT WIN32)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()
find_package(Threads REQUIRED)

target_link_libraries(helics_base INTERFACE Threads::Threads)

if(UNIX AND NOT APPLE)
    target_link_libraries(helics_base INTERFACE rt)
endif()

target_compile_definitions(helics_base INTERFACE "CLI11_HAS_FILESYSTEM=1")
target_compile_definitions(helics_base INTERFACE "USE_STD_OPTIONAL=1")
# -------------------------------------------------------------
# add some additional libraries for MINGW and MSYS
# -------------------------------------------------------------
if(MSYS OR CYGWIN)
    target_link_libraries(helics_base INTERFACE wsock32 ws2_32 iphlpapi)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(ENABLE_IPC_CORE FALSE)
    endif()
    # setting to windows 10 for all compilation
    if(NOT HELICS_SKIP_WINNT_DEFINITION)
        if(WIN32_WINNT)
            add_compile_definitions("_WIN32_WINNT=${WIN32_WINNT}")
        else()
            add_compile_definitions(_WIN32_WINNT=0x0A00)
        endif()
    endif()
endif()

if(MINGW AND NOT UNIX_LIKE)
    target_link_libraries(helics_base INTERFACE wsock32 ws2_32 iphlpapi)

endif()

if(CYGWIN)
    target_compile_definitions(helics_base INTERFACE -D_XOPEN_SOURCE=500 -D__USE_W32_SOCKETS)
endif()

option(HELICS_ENABLE_ENCRYPTION "enable encryption support in HELICS" OFF)

# -------------------------------------------------------------
# Enable ZeroMQ
# -------------------------------------------------------------

# If ZeroMQ library is enabled try to locate it and link against it
add_library(zmq INTERFACE)
if(HELICS_ENABLE_ZMQ_CORE)
    include(addZeroMQ)

    if(NOT ZeroMQ_FOUND)
        message(FATAL_ERROR "ZeroMQ not found, needed to enable the ZMQ Core")
    endif()
    if(HELICS_USE_ZMQ_STATIC_LIBRARY)
        target_link_libraries(zmq INTERFACE libzmq-static)
        target_compile_definitions(zmq INTERFACE -DZMQ_STATIC)
    else()
        target_link_libraries(zmq INTERFACE libzmq)
    endif()
endif()
add_library(helics::zmq ALIAS zmq)

# -----------------------------------------------------------------------------
# Setup configure.h file for accessing configure options
# -----------------------------------------------------------------------------
configure_file(
    "config/helics-config.h.in"
    "${HELICS_BINARY_DIR}/helics_generated_includes/helics/helics-config.h"
)

target_include_directories(
    helics_base INTERFACE $<BUILD_INTERFACE:${HELICS_BINARY_DIR}/helics_generated_includes/>
)

# -------------------------------------------------------------
# global include directories
# -------------------------------------------------------------
target_include_directories(
    helics_base
    INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
              $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
              $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDE_DIR}>
)
# the utilities/gmlc is to account for the transfer of the header to a known location on install

target_include_directories(
    helics_base SYSTEM
    INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty/containers>
              $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty/concurrency>
              $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty/utilities>
              $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty/utilities/gmlc/utilities>
              $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty>
)

target_link_libraries(helics_base INTERFACE toml11::toml11)

if(NOT HELICS_DISABLE_BOOST)
    target_include_directories(helics_base SYSTEM INTERFACE $<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>)
endif()

# -------------------------------------------------------------
# Asio include directories
# -------------------------------------------------------------
if(NOT HELICS_DISABLE_ASIO)
    target_compile_definitions(helics_base INTERFACE "-DASIO_STANDALONE")
    target_include_directories(
        helics_base SYSTEM
        INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty/asio/asio/include>
    )
endif()

include(commonRPath)
if(NOT WIN32)
    if(HELICS_ENABLE_ZMQ_CORE AND NOT HELICS_USE_ZMQ_STATIC_LIBRARY)
        get_target_property(zmqlibfile libzmq IMPORTED_LOCATION)
        if(NOT zmqlibfile)
            get_target_property(zmqlibfile libzmq IMPORTED_LOCATION_RELEASE)
        endif()
        # message(STATUS "zmqlib file =${zmqlibfile}")
        get_filename_component(zmqdir ${zmqlibfile} DIRECTORY)
        # message(STATUS "zmqdir path =${zmqdir}")
        if(APPLE)
            # TODO(@nightlark): Make sure it works for building ZMQ from source automatically using
            # CMake
            list(APPEND CMAKE_INSTALL_RPATH ${ZeroMQ_INSTALL_PATH}/lib)
            list(APPEND CMAKE_BUILD_RPATH ${ZeroMQ_INSTALL_PATH}/lib)
        endif()
        list(APPEND CMAKE_INSTALL_RPATH "${zmqdir}")
        list(APPEND CMAKE_BUILD_RPATH "${zmqdir}")
    endif()
endif()

# -----------------------------------------------------------------------------
# create the fmt target
# -----------------------------------------------------------------------------
include(addfmt)

# --------------------------------------------------------------
# Create the target for spdlog
# -----------------------------------------------------------
include(addSpdlog)

# --------------------------------------------------------------
# Create the target for the units library
# -----------------------------------------------------------
include(addUnits)

# -----------------------------------------------------------------------------
# create utilities target
# -----------------------------------------------------------------------------
include(addUtilities)

# -----------------------------------------------------------------------------
# create networking target
# -----------------------------------------------------------------------------

include(addNetworking)

# -----------------------------------------------------------------------------
# CMAKE Subdirectories
# -----------------------------------------------------------------------------

add_subdirectory(src)

# -----------------------------------------------------------------------------
# Build the tests
# -----------------------------------------------------------------------------

if(HELICS_BUILD_TESTS AND BUILD_TESTING)
    if(NOT HELICS_ENABLE_TEST_CORE)
        message(FATAL_ERROR "TEST CORE must be enabled to build the HELICS tests")
    endif()
    mark_as_advanced(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(HELICS_BUILD_BENCHMARKS)
    if(NOT HELICS_ENABLE_INPROC_CORE)
        message(FATAL_ERROR "INPROC CORE must be enabled to build the HELICS Benchmarks")
    endif()
    add_subdirectory(benchmarks)
endif()

# -----------------------------------------------------------------------------
# Setup Examples
# -----------------------------------------------------------------------------
if(HELICS_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif(HELICS_BUILD_EXAMPLES)

# -------------------------------------------------------------
# Enable clang analysis and formatting tools
# -------------------------------------------------------------
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    cmake_dependent_advanced_option(
        HELICS_ENABLE_CLANG_TOOLS
        "if clang is found enable some custom targets for clang formatting and tidy" OFF
        "CMAKE_CXX_COMPILER_ID STREQUAL \"Clang\"" OFF
    )
    if(HELICS_ENABLE_CLANG_TOOLS)
        include(clang-cxx-dev-tools)
    endif(HELICS_ENABLE_CLANG_TOOLS)

    add_subdirectory(interfaces)

    add_subdirectory(docs)

endif(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

if(NOT HELICS_BINARY_ONLY_INSTALL)
    install(TARGETS compile_flags_target ${HELICS_EXPORT_COMMAND})
    install(TARGETS build_flags_target ${HELICS_EXPORT_COMMAND})
    #[[   if(HELICS_BUILD_CXX_SHARED_LIB)
        set(
            helics_cxx_shared_file
            "${CMAKE_SHARED_LIBRARY_PREFIX}helicscpp${CMAKE_SHARED_LIBRARY_SUFFIX}"
        )
        set(
            helics_cxx_shared_file_debug
            "${CMAKE_SHARED_LIBRARY_PREFIX}helicscpp${CMAKE_DEBUG_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}"
        )
    else(HELICS_BUILD_CXX_SHARED_LIB)
        set(helics_cxx_shared_file)
        set(helics_cxx_shared_file_debug)
    endif(HELICS_BUILD_CXX_SHARED_LIB)

    if(NOT HELICS_DISABLE_C_SHARED_LIB)
        set(
            helics_c_shared_file
            "${CMAKE_SHARED_LIBRARY_PREFIX}helics${CMAKE_SHARED_LIBRARY_SUFFIX}"
        )
        set(
            helics_c_shared_file_debug
            "${CMAKE_SHARED_LIBRARY_PREFIX}helics${CMAKE_DEBUG_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}"
        )
    else()
        set(helics_c_shared_file)
        set(helics_c_shared_file_debug)
    endif()
#]]
    if(HELICS_WITH_CMAKE_PACKAGE)

        set(HELICS_CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
            CACHE STRING "install path for HELICSConfig.cmake"
        )
        mark_as_advanced(HELICS_CMAKECONFIG_INSTALL_DIR)

        # Export targets for importing build tree with find_package
        export(EXPORT helics-targets NAMESPACE HELICS::
               FILE ${PROJECT_BINARY_DIR}/helics-targets.cmake
        )

        install(
            EXPORT helics-targets
            NAMESPACE HELICS::
            DESTINATION ${HELICS_CMAKECONFIG_INSTALL_DIR}
            COMPONENT libs
        )
    endif()
endif(NOT HELICS_BINARY_ONLY_INSTALL)

if(NOT HELICS_BINARY_ONLY_INSTALL)

    if(HELICS_WITH_CMAKE_PACKAGE)

        include(CMakePackageConfigHelpers)

        configure_package_config_file(
            config/${PROJECT_NAME}Config.cmake.in
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
            INSTALL_DESTINATION ${HELICS_CMAKECONFIG_INSTALL_DIR}
        )
        write_basic_package_version_file(
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake VERSION ${HELICS_VERSION}
            COMPATIBILITY SameMajorVersion
        )

        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
                      ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                DESTINATION ${HELICS_CMAKECONFIG_INSTALL_DIR} COMPONENT libs
        )
    endif()

endif(NOT HELICS_BINARY_ONLY_INSTALL)

# combine 3rd party license files for install package
include(combineLicenses)
set(LICENSE_LIST
    "GMLC Concurrency"
    "${PROJECT_SOURCE_DIR}/ThirdParty/concurrency/LICENSE"
    "libGuarded"
    "${PROJECT_SOURCE_DIR}/ThirdParty/concurrency/gmlc/libguarded/LICENSE"
    "GMLC Containers"
    "${PROJECT_SOURCE_DIR}/ThirdParty/containers/LICENSE"
    "cppzmq"
    "${PROJECT_SOURCE_DIR}/ThirdParty/cppzmq/LICENSE"
    "fmt"
    "${PROJECT_SOURCE_DIR}/ThirdParty/fmtlib/LICENSE"
    "units"
    "${PROJECT_SOURCE_DIR}/ThirdParty/units/LICENSE"
    "toml11"
    "${PROJECT_SOURCE_DIR}/ThirdParty/toml/LICENSE"
    "GMLC Utilities"
    "${PROJECT_SOURCE_DIR}/ThirdParty/utilities/LICENSE"
    "GMLC Networking"
    "${PROJECT_SOURCE_DIR}/ThirdParty/networking/LICENSE"
)

if(HELICS_BUILD_TESTS)
    list(APPEND LICENSE_LIST "Google Test" "${PROJECT_SOURCE_DIR}/ThirdParty/googletest/LICENSE")
endif()

if(HELICS_BUILD_BENCHMARKS)
    list(APPEND LICENSE_LIST "Google Benchmark"
         "${PROJECT_SOURCE_DIR}/ThirdParty/benchmark/LICENSE"
    )
endif()

if(EXISTS ${PROJECT_BINARY_DIR}/_deps/libzmq-src AND HELICS_ENABLE_ZMQ_CORE)
    list(APPEND LICENSE_LIST "ZeroMQ" "${PROJECT_SOURCE_DIR}/ThirdParty/libzmq/LICENSE")
endif()

if(NOT HELICS_DISABLE_ASIO)
    list(APPEND LICENSE_LIST "asio" "${PROJECT_SOURCE_DIR}/ThirdParty/asio/asio/LICENSE_1_0.txt")
endif()

combinelicenses(${PROJECT_BINARY_DIR}/THIRDPARTY_LICENSES ${LICENSE_LIST})

install(FILES LICENSE NOTICE CHANGELOG.md README.md ${PROJECT_BINARY_DIR}/THIRDPARTY_LICENSES
        DESTINATION ${CMAKE_INSTALL_DOCDIR} COMPONENT libs
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs/man/manpage_out/
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
    FILES_MATCHING
    PATTERN "*.1"
)

# -------------------------------------------------------------
# pkg-config setup and install
# -------------------------------------------------------------

if(NOT MSVC AND NOT HELICS_BINARY_ONLY_INSTALL)

    set(prefix ${CMAKE_INSTALL_PREFIX})
    set(exec_prefix ${prefix}/${CMAKE_INSTALL_BINDIR})
    set(libdir ${prefix}/${CMAKE_INSTALL_LIBDIR})
    set(includedir ${prefix}/${CMAKE_INSTALL_INCLUDEDIR})

    if(USE_LIBCXX)
        set(stdlib "-lc++ -lc++abi")
    elseif(STATIC_STANDARD_LIB STREQUAL "static")
        set(stdlib)
    else()
        set(stdlib -lstdc++)
    endif()

    if(NOT HELICS_DISABLE_C_SHARED_LIB)
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/config/packaging/helics.pc.in
            ${CMAKE_CURRENT_BINARY_DIR}/helics.pc @ONLY
        )
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/helics.pc
                DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
        )
    endif()

    if(ENABLE_ZMQ_CORE AND NOT HELICS_USE_ZMQ_STATIC_LIBRARY)
        set(zmq_pc_require libzmq)
    endif()

    if(HELICS_BUILD_CXX_SHARED_LIB)

        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/config/packaging/helicscpp.pc.in
            ${CMAKE_CURRENT_BINARY_DIR}/helicscpp.pc @ONLY
        )
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/helicscpp.pc
                DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
        )

        if(HELICS_BUILD_APP_LIBRARY)
            configure_file(
                ${CMAKE_CURRENT_SOURCE_DIR}/config/packaging/helicscpp-apps.pc.in
                ${CMAKE_CURRENT_BINARY_DIR}/helicscpp-apps.pc @ONLY
            )
            install(FILES ${CMAKE_CURRENT_BINARY_DIR}/helicscpp-apps.pc
                    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
            )
        endif()

    endif()
endif()

# -------------------------------------------------------------
# CPack
# -------------------------------------------------------------
cmake_dependent_option(
    HELICS_ENABLE_PACKAGE_BUILD "Add projects for making packages and installers for HELICS" OFF
    "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" OFF
)

if(HELICS_ENABLE_PACKAGE_BUILD)

    # cmake-format: off
    set(CPACK_PACKAGE_NAME "Helics")
    set(CPACK_PACKAGE_VENDOR "GMLC")
    set(CPACK_PACKAGE_CONTACT "helicsdevelopers@helics.org")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Hierarchical Engine for Large-scale Infrastructure Co-Simulation (HELICS)")
    set(CPACK_PACKAGE_DESCRIPTION_FILE "${${PROJECT_NAME}_SOURCE_DIR}/docs/description.txt")
    set(CPACK_PACKAGE_VERSION ${HELICS_VERSION})
    set(CPACK_PACKAGE_VERSION_MAJOR ${HELICS_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${HELICS_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${HELICS_VERSION_PATCH})
    set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "HELICS")
    # cmake-format: on

    set(CPACK_COMPONENTS_ALL
        Runtime
        applications
        headers
        libs
        swig
        java
        csharp
        benchmarks
    )
    if(WIN32)
        set(CPACK_RESOURCE_FILE_LICENSE "${HELICS_SOURCE_DIR}\\\\LICENSE")
        set(CPACK_RESOURCE_FILE_README "${HELICS_SOURCE_DIR}\\\\README.md")
    else(WIN32)
        set(CPACK_RESOURCE_FILE_LICENSE "${HELICS_SOURCE_DIR}/LICENSE")
        set(CPACK_RESOURCE_FILE_README "${HELICS_SOURCE_DIR}/README.md")
    endif(WIN32)

    set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "Applications")
    set(CPACK_COMPONENT_LIBS_DISPLAY_NAME "Libraries")
    set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "Headers")
    set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "Runtime Libraries")
    set(CPACK_COMPONENT_BENCHMARKS_DISPLAY_NAME "Benchmarks")
    set(CPACK_COMPONENT_SWIG_DISPLAY_NAME "SWIG")
    set(CPACK_COMPONENT_JAVA_DISPLAY_NAME "Java")
    set(CPACK_COMPONENT_CSHARP_DISPLAY_NAME "C#")

    set(CPACK_COMPONENT_GROUP_INTERFACES_DISPLAY_NAME "Interfaces")
    set(CPACK_COMPONENT_GROUP_INTERFACES_DESCRIPTION "Additional language interfaces for HELICS")
    set(CPACK_COMPONENT_SWIG_GROUP interfaces)
    set(CPACK_COMPONENT_JAVA_GROUP interfaces)
    set(CPACK_COMPONENT_CSHARP_GROUP interfaces)

    set(CPACK_COMPONENT_GROUP_DEVELOPMENT_DISPLAY_NAME "Development")
    set(CPACK_COMPONENT_GROUP_DEVELOPMENT_DESCRIPTION
        "Files needed to build applications that use HELICS"
    )
    set(CPACK_COMPONENT_HEADERS_GROUP development)
    set(CPACK_COMPONENT_LIBS_GROUP development)

    set(CPACK_COMPONENT_APPLICATIONS_DESCRIPTION "Executables and helper applications for HELICS")
    set(CPACK_COMPONENT_LIBS_DESCRIPTION "Libraries for compiling and linking with HELICS")
    set(CPACK_COMPONENT_HEADERS_DESCRIPTION "Headers for linking and compiling with HELICS")
    set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "Runtime libraries for HELICS")
    set(CPACK_COMPONENT_BENCHMARKS_DESCRIPTION "Benchmark applications for HELICS")

    set(CPACK_COMPONENT_SWIG_DESCRIPTION
        "SWIG files needed for building 3rd party language interfaces (requires Development Headers)"
    )
    set(CPACK_COMPONENT_JAVA_DESCRIPTION "Java language interface")
    set(CPACK_COMPONENT_CSHARP_DESCRIPTION "C# language interface")

    set(CPACK_COMPONENT_SWIG_DEPENDS headers)
    set(CPACK_COMPONENT_LIBS_DEPENDS headers)
    set(CPACK_COMPONENT_RUNTIME_REQUIRED ON)

    set(CPACK_PACKAGE_EXECUTABLES
        "helics_broker"
        "Helics Broker"
        "helics_broker_server"
        "Helics Broker Server"
        "helics_app"
        "Helics app executable"
        "helics_recorder"
        "Helics Recorder"
        "helics_player"
        "Helics Player"
        "helics_connector"
        "Helics Connector"
    )

    if(INSTALL_SYSTEM_LIBRARIES)
        include(InstallRequiredSystemLibraries)
    endif()

    if(WIN32)
        # title at the top of the installer
        set(CPACK_NSIS_PACKAGE_NAME "HELICS v${HELICS_VERSION}")
        # name shown in the add/remove program control panel
        set(CPACK_NSIS_DISPLAY_NAME "HELICS v${HELICS_VERSION}")
        set(CPACK_PACKAGE_ICON "${HELICS_SOURCE_DIR}\\\\docs\\\\logos\\\\HELICS.ico")
        set(CPACK_NSIS_MUI_ICON "${HELICS_SOURCE_DIR}/docs/logos/HELICS.ico")
        set(CPACK_NSIS_INSTALL_ROOT "C:\\\\local")
        set(CPACK_NSIS_URL_INFO_ABOUT "https://www.github.com/GMLC-TDC/HELICS")
        set(CPACK_NSIS_HELP_LINK "https://helics.readthedocs.io/en/latest")
        set(CPACK_NSIS_CONTACT "helicsteam@helics.org")
        set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
        set(CPACK_PACKAGE_INSTALL_DIRECTORY helics_${HELICS_VERSION_UNDERSCORE})
        set(CPACK_NSIS_MODIFY_PATH ON)
        set(CPACK_NSIS_EXECUTABLES_DIRECTORY ${CMAKE_INSTALL_BINDIR})
        set(CPACK_NSIS_MENU_LINKS
            "https://www.github.com/GMLC-TDC/HELICS"
            "HELICS GitHub"
            "https://helics.readthedocs.io/en/latest"
            "HELICS Documentation"
            "https://www.helics.org"
            "HELICS Website"
            "https://gitter.im/GMLC-TDC/HELICS"
            "HELICS Gitter Chat"
            "https://www.youtube.com/channel/UCPa81c4BVXEYXt2EShTzbcg"
            "TDC YouTube channel"
        )
    else(WIN32)
        set(CPACK_PACKAGE_ICON "${HELICS_SOURCE_DIR}/docs/logos/HELICS.ico")
    endif(WIN32)

    if(APPLE)
        if("arm64" IN_LIST CMAKE_OSX_ARCHITECTURES AND "x86_64" IN_LIST CMAKE_OSX_ARCHITECTURES)
            set(CPACK_SYSTEM_NAME "macOS-universal2")
        else()
            set(CPACK_SYSTEM_NAME "macOS-${CMAKE_SYSTEM_PROCESSOR}")
        endif()
        set(CPACK_DMG_DISABLE_APPLICATIONS_SYMLINK ON)
        set(CPACK_PACKAGE_ICON "${HELICS_SOURCE_DIR}/docs/logos/HELICS.ico")
    elseif(NOT WIN32 AND NOT CYGWIN)
        set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    # for debian
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "all")
    set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")
    set(CPACK_DEBIAN_PACKAGE_NAME "helics-dev")

    set(CPACK_SOURCE_IGNORE_FILES "/Build*/;/build*/;/.git/")
    # THIS LINE MUST BE LAST IN THE CPACK SECTION
    include(CPack)
endif(HELICS_ENABLE_PACKAGE_BUILD)

if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # restore the original module path
    set(CMAKE_MODULE_PATH ${${PROJECT_NAME}_ORIGINAL_MODULE_PATH})
    if(old_build_shared)
        set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
    endif()

    if(HELICS_HIDE_CMAKE_VARIABLES)
        hide_variable(HELICS_BINARY_ONLY_INSTALL)
        hide_variable(HELICS_BUILD_APP_LIBRARY)
        hide_variable(HELICS_BUILD_CONFIGURATION)
        hide_variable(HELICS_BUILD_CXX_SHARED_LIB)
        hide_variable(HELICS_DISABLE_ASIO)
        hide_variable(HELICS_DISABLE_BOOST)
        hide_variable(HELICS_DISABLE_C_SHARED_LIB)
        hide_variable(HELICS_DISABLE_WEBSERVER)
        hide_variable(HELICS_DISABLE_VCPKG)
        hide_variable(HELICS_ENABLE_DEBUG_LOGGING)
        hide_variable(HELICS_ENABLE_ENCRYPTION)
        hide_variable(HELICS_ENABLE_INPROC_CORE)
        hide_variable(HELICS_ENABLE_LOGGING)
        hide_variable(HELICS_ENABLE_IPC_CORE)
        hide_variable(HELICS_ENABLE_MPI_CORE)
        hide_variable(HELICS_ENABLE_TRACE_LOGGING)
        hide_variable(HELICS_ENABLE_PYTHON_BUILD_SCRIPTS)
        hide_variable(HELICS_ENABLE_TEST_CORE)
        hide_variable(HELICS_ENABLE_ZMQ_CORE)
        hide_variable(HELICS_ENABLE_UDP_CORE)
        hide_variable(HELICS_ENABLE_SUBMODULE_UPDATE)
        hide_variable(HELICS_ENABLE_TCP_CORE)
        hide_variable(HELICS_ENABLE_MPI_CORE)

        hide_variable(HELICS_USE_EXTERNAL_FMT)
        hide_variable(HELICS_USE_EXTERNAL_JSONCPP)
        hide_variable(HELICS_USE_EXTERNAL_SPDLOG)
        hide_variable(HELICS_USE_EXTERNAL_UNITS)
        hide_variable(HELICS_USE_POSITION_INDEPENDENT_CODE)
        hide_variable(HELICS_USE_SYSTEM_ZEROMQ_ONLY)
        hide_variable(HELICS_USE_ZMQ_STATIC_LIBRARY)
        hide_variable(HELICS_ZMQ_FORCE_SUBPROJECT)
        hide_variable(HELICS_ZMQ_SUBPROJECT)
    endif()
endif()

# -------------------------------------------------------------
# Git Hooks
# -------------------------------------------------------------
if(NOT HELICS_DISABLE_GIT_OPERATIONS)
    if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
        include(addGitHooks)
    endif()
endif()
