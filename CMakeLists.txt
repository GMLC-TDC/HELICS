# ~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~

cmake_minimum_required(VERSION 3.4)
cmake_policy(VERSION 3.4)

project(HELICS VERSION 2.4.0)

if(POLICY CMP0069)
    # Policy CMP0069 INTERPROCEDURAL OPTIMIZATION, just use the new policy since this
    # isn't used for old systems
    cmake_policy(SET CMP0069 NEW)
endif()

if(POLICY CMP0074)
    # Policy CMP0074 find_package uses <PackageName>_ROOT variables
    cmake_policy(SET CMP0074 NEW)
endif()

if(POLICY CMP0076)
    # Policy CMP0074 target_sources now converts relative paths to absolute
    cmake_policy(SET CMP0076 NEW)
endif()

# -----------------------------------------------------------------------------
# HELICS Version number
# -----------------------------------------------------------------------------
set(HELICS_VERSION_BUILD)
# use ISO date YYYY-MM-DD
set(HELICS_DATE "2020-02-04")

set(HELICS_VERSION_UNDERSCORE
    "${HELICS_VERSION_MAJOR}_${HELICS_VERSION_MINOR}_${HELICS_VERSION_PATCH}"
)
if(HELICS_VERSION_BUILD)
    set(HELICS_VERSION "${HELICS_VERSION}-${HELICS_VERSION_BUILD}")
    set(HELICS_VERSION_UNDERSCORE
        "${HELICS_VERSION_UNDERSCORE}-${HELICS_VERSION_BUILD}"
    )
endif()

set(HELICS_COMPILER_VERSION "${CMAKE_GENERATOR} ${CMAKE_GENERATOR_PLATFORM} ${CMAKE_SYSTEM}:${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "SYSTEM INFO -> ${HELICS_COMPILER_VERSION}")

# -----------------------------------------------------------------------------
# set the module path and include some common macros
# -----------------------------------------------------------------------------

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/config/cmake/")
else()
    set(ORIGINAL_MODULE_PATH ${CMAKE_MODULE_PATH})
    set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/config/cmake/" ${CMAKE_MODULE_PATH})
    get_property(helics-use-folders GLOBAL PROPERTY USE_FOLDERS)
endif()

# generate a version description based on git tags
include(version_describe)
git_version_describe(${HELICS_SOURCE_DIR} HELICS_VERSION_DESCRIPTION)

if(HELICS_VERSION_DESCRIPTION)
    message(STATUS "helics version tag description is ${HELICS_VERSION_DESCRIPTION}")
    string(TIMESTAMP current_date "%Y-%m-%d")
    set(HELICS_VERSION_STRING
        "${HELICS_VERSION}-${HELICS_VERSION_DESCRIPTION} (${current_date})"
    )
else()
    set(HELICS_VERSION_STRING "${HELICS_VERSION} (${HELICS_DATE})")
endif()

include(extraMacros)
include(CMakeDependentOption)
include(copy_key_files)
include(CTest)
include(ucm)

# include(BuildType)
# -----------------------------------------------------------------------------
# set the install path to a local directory
# -----------------------------------------------------------------------------
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(WIN32)
        if(MSVC)
            set(CMAKE_INSTALL_PREFIX
                "C:/local/helics_${HELICS_VERSION_UNDERSCORE}/"
                CACHE PATH "default install path" FORCE
            )
        elseif(MINGW AND NOT MSYS)
            set(CMAKE_INSTALL_PREFIX
                "C:/local/helics_${HELICS_VERSION_UNDERSCORE}/"
                CACHE PATH "default install path" FORCE
            )
        elseif(MSYS)
            # use CMAKE_OBJCOPY here since it is somewhat less likely to be overwridden
            # by users rather than the compiler
            get_filename_component(path_bin ${CMAKE_OBJCOPY} DIRECTORY)
            get_filename_component(path_install ${path_bin} DIRECTORY)
            set(CMAKE_INSTALL_PREFIX
                ${path_install}
                CACHE PATH "default install path" FORCE
            )
        endif(MSVC)
    endif(WIN32)
endif()

if(MSYS
   OR CYGWIN
   OR UNIX
   OR APPLE
)
    set(UNIX_LIKE TRUE)
endif()

# allow BOOST library inclusion to be turned off completely; this option will disable the IPC
# core
option(HELICS_DISABLE_BOOST OFF "disable all references to the Boost C++ libraries")
mark_as_advanced(HELICS_DISABLE_BOOST)

# allow ASIO library inclusion to be turned off completely; this option will disable the UDP and
# TCP core as well as the timeout detection and real-time features of HELICS
option(HELICS_DISABLE_ASIO OFF "disable all references to the ASIO C++ libraries")
mark_as_advanced(HELICS_DISABLE_ASIO)

# we want to acknowledge this flag since it is standard CMake but it can cause issues in
# submodules so we need to set other variables
if(NOT DEFINED BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS
        OFF
        CACHE INTERNAL ""
    )
endif()
if(BUILD_SHARED_LIBS)
    set(HELICS_BUILD_CXX_SHARED_LIB
        ON
        CACHE BOOL ""
    )
    set(HELICS_DISABLE_C_SHARED_LIB
        OFF
        CACHE BOOL ""
    )
    set(old_build_shared ON)
    # we need this to prevent submodules from building shared libs
    set(BUILD_SHARED_LIBS
        OFF
        CACHE BOOL "" FORCE
    )
endif()

cmake_dependent_option(
    HELICS_BUILD_TESTS "Enable the HELICS test executables to be built" OFF
    "BUILD_TESTING;CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" OFF
)

cmake_dependent_option(
    HELICS_BUILD_EXAMPLES "Build HELICS examples" OFF
    "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" OFF
)

cmake_dependent_option(
    HELICS_BUILD_BENCHMARKS "Build HELICS Benchmarks" OFF
    "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME;CMAKE_VERSION VERSION_GREATER 3.5" OFF
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

option(HELICS_DISABLE_C_SHARED_LIB OFF
       "turn off building of the HELICS C shared library interface"
)
mark_as_advanced(HELICS_DISABLE_C_SHARED_LIB)

option(HELICS_BUILD_CXX_SHARED_LIB
       "build the shared libraries for the HELICS CXX interface" OFF
)

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

if(NOT HELICS_DISABLE_C_SHARED_LIB
   OR HELICS_BUILD_CXX_SHARED_LIB
   OR HELICS_USE_POSITION_INDEPENDENT_CODE
)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

# -----------------------------------------------------------------------------
# General project wide configuration for debug postfix
# -----------------------------------------------------------------------------
if(NOT NO_DEBUG_POSFIX AND NOT CMAKE_DEBUG_POSTFIX)
    set(CMAKE_DEBUG_POSTFIX d)
endif()

# add a baseline library for underlying dependencies and flags
add_library(helics_base INTERFACE)

# Prohibit in-source build
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(
        FATAL_ERROR
            "In-source build is not supported. Please, use an empty directory for building the project."
    )
endif()

if(NOT TARGET compile_flags_target)
    add_library(compile_flags_target INTERFACE)
endif()

if(NOT TARGET build_flags_target)
    add_library(build_flags_target INTERFACE)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    mark_as_advanced(BUILD_TESTING)
    include(compiler_flags)

endif()

add_library(HELICS::compile_flags_target ALIAS compile_flags_target)
add_library(HELICS::build_flags_target ALIAS build_flags_target)

get_target_property(EXTRA_BUILD_FLAGS build_flags_target INTERFACE_COMPILE_OPTIONS)
set(HELICS_BUILD_FLAGS_OUTPUT "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} ${CMAKE_STATIC_LINKER_FLAGS} ${EXTRA_BUILD_FLAGS} ")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    if(NOT USE_LIBCXX)
        show_variable(
            STATIC_STANDARD_LIB STRING "Link against a static standard lib" default
        )
        set_property(CACHE STATIC_STANDARD_LIB PROPERTY STRINGS default static dynamic)
    else()
        hide_variable(STATIC_STANDARD_LIB)
    endif()
    if(MSVC)
        show_variable(
            HELICS_EMBEDDED_DEBUG_INFO STRING "embed debug info into lib files" default
        )
        set_property(
            CACHE HELICS_EMBEDDED_DEBUG_INFO PROPERTY STRINGS default embedded external
        )
    else()
        hide_variable(HELICS_EMBEDDED_DEBUG_INFO)
    endif()
endif()

if(STATIC_STANDARD_LIB STREQUAL "default")

elseif(STATIC_STANDARD_LIB STREQUAL "static")
    ucm_set_runtime(STATIC)
elseif(STATIC_STANDARD_LIB STREQUAL "dynamic")
    ucm_set_runtime(DYNAMIC)
endif()

if(HELICS_EMBEDDED_DEBUG_INFO STREQUAL "default")

elseif(HELICS_EMBEDDED_DEBUG_INFO STREQUAL "external")
    ucm_set_embedded_debug(EXTERNAL)
else()
    ucm_set_embedded_debug(EMBEDDED)
endif()

# -------------------------------------------------------------
# add coverage target
# -------------------------------------------------------------

cmake_dependent_advanced_option(
    HELICS_TEST_CODE_COVERAGE "Build a target for testing code coverage" OFF
    "HELICS_BUILD_TESTS" OFF
)

if(HELICS_BUILD_TESTS AND HELICS_TEST_CODE_COVERAGE)
    if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
        include(CodeCoverage)

        set(COVERAGE_EXCLUDES 'usr/*' 'dependencies/*' 'ThirdParty/*' 'tests/*'
                              'interfaces/*' 'examples/*' 'benchmarks/*' 'scripts/*'
        )
        setup_target_for_coverage(
            NAME helics_coverage # New target name
            EXECUTABLE CTest # Executable in PROJECT_BINARY_DIR
        )
    else()
        message(
            FATAL_ERROR
                "CMAKE_BUILD_TYPE must be set to Coverage for testing code coverage"
        )
    endif()
endif()

# -------------------------------------------------------------
# Get some configuration for C++17 as that becomes available
# -------------------------------------------------------------
# message(STATUS ${CMAKE_CXX_FLAGS})
set(CONFIGURE_TARGET_LOCATION ${HELICS_BINARY_DIR}/helics_generated_includes/helics/)
include(configGenerator)

if(HELICS_GENERATE_DOXYGEN_DOC)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)

        show_variable(
            DOXYGEN_OUTPUT_DIR PATH "location to put Doxygen docs"
            "${HELICS_BINARY_DIR}/docs"
        )
        configure_file(
            ${HELICS_SOURCE_DIR}/config/Doxyfile.in ${HELICS_BINARY_DIR}/Doxyfile @ONLY
        )
        add_custom_target(
            helics_doxygen ${DOXYGEN_EXECUTABLE} ${HELICS_BINARY_DIR}/Doxyfile
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
include(updateGitSubmodules)
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/fmtlib/CMakeLists.txt")
    submod_update(ThirdParty/fmtlib)
endif()

if(NOT HELICS_DISABLE_ASIO)
    if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/asio/asio/include/asio.hpp")
        submod_update(ThirdParty/asio)
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/jsoncpp/CMakeLists.txt")
    submod_update(ThirdParty/jsoncpp)
endif()

if(NOT EXISTS
   "${PROJECT_SOURCE_DIR}/ThirdParty/containers/gmlc/containers/BlockingQueue.hpp"
)
    submod_update(ThirdParty/containers)
endif()

if(NOT EXISTS
   "${PROJECT_SOURCE_DIR}/ThirdParty/concurrency/gmlc/concurrency/Barrier.hpp"
)
    submod_update(ThirdParty/concurrency)
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/utilities/gmlc/utilities/stringOps.h")
    submod_update(ThirdParty/utilities)
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/units/units/units.hpp")
    submod_update(ThirdParty/units)
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/ThirdParty/toml11/toml/value.hpp")
    submod_update(ThirdParty/toml11)
endif()

check_submodule_status()

include(GNUInstallDirs)

# -------------------------------------------------------------
# BOOST  find the boost libraries
# -------------------------------------------------------------

if(NOT HELICS_DISABLE_BOOST)
    include(addBoost)
endif()

# -------------------------------------------------------------
# TOML11  add the toml interpreter
# -------------------------------------------------------------

set(toml11_BUILD_TEST OFF CACHE INTERNAL "")
set(toml11_TEST_WITH_ASAN OFF CACHE INTERNAL "")
set(toml11_TEST_WITH_UBSAN OFF CACHE INTERNAL "")
add_subdirectory(ThirdParty/toml11 EXCLUDE_FROM_ALL)

# -------------------------------------------------------------
# options for enabling specific core communication types
# -------------------------------------------------------------

option(ENABLE_MPI_CORE "Enable MPI networking library" OFF)
cmake_dependent_advanced_option(
    ENABLE_TCP_CORE "Enable TCP core types" ON "NOT HELICS_DISABLE_ASIO" OFF
)
cmake_dependent_advanced_option(
    ENABLE_UDP_CORE "Enable UDP core types" ON "NOT HELICS_DISABLE_ASIO" OFF
)
cmake_dependent_advanced_option(
    ENABLE_IPC_CORE "Enable Interprocess communication types" ON
    "NOT HELICS_DISABLE_BOOST" OFF
)
cmake_dependent_advanced_option(
    ENABLE_TEST_CORE "Enable test inprocess core type" OFF "NOT HELICS_BUILD_TESTS" ON
)
cmake_dependent_advanced_option(
    ENABLE_INPROC_CORE "Enable inprocess core type" ON "NOT HELICS_BUILD_BENCHMARKS" ON
)
option(ENABLE_ZMQ_CORE "Enable ZeroMQ networking library" ON)

mark_as_advanced(ENABLE_MPI_CORE ENABLE_ZMQ_CORE)

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
elseif(MSYS OR CYGWIN)
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
# add some additional libraries for MINGW and MSYS
# -------------------------------------------------------------
if(MSYS OR CYGWIN)
    target_link_libraries(helics_base INTERFACE wsock32 ws2_32 iphlpapi)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        set(ENABLE_IPC_CORE FALSE)
    endif()
endif()

if(MINGW AND NOT UNIX_LIKE)
    target_link_libraries(helics_base INTERFACE wsock32 ws2_32 iphlpapi)
endif()

if(CYGWIN)
    target_compile_definitions(
        helics_base INTERFACE -D_XOPEN_SOURCE=500 -D__USE_W32_SOCKETS
        -D_WIN32_WINNT=0x0601
    )
endif()
# -------------------------------------------------------------
# Enable ZeroMQ
# -------------------------------------------------------------

# If ZeroMQ library is enabled try to locate it and link against it

if(ENABLE_ZMQ_CORE)
    include(addZeroMQ)

    if(NOT ZeroMQ_FOUND)
        message(FATAL_ERROR "ZeroMQ not found, needed to enable the ZMQ Core")
    endif()

    if(HELICS_USE_ZMQ_STATIC_LIBRARY)
        set(ZeroMQ_DEPENDENCY libzmq-static)
        target_compile_definitions(helics_base INTERFACE -DZMQ_STATIC)
    else()
        set(ZeroMQ_DEPENDENCY libzmq)
    endif()
    # message(STATUS "zmq dep ${ZeroMQ_DEPENDENCY}")
    target_link_libraries(helics_base INTERFACE ${ZeroMQ_DEPENDENCY})
endif()

# -----------------------------------------------------------------------------
# Setup configure.h file for accessing configure options
# -----------------------------------------------------------------------------
configure_file(
    "config/helics-config.h.in"
    "${HELICS_BINARY_DIR}/helics_generated_includes/helics/helics-config.h"
)

target_include_directories(
    helics_base
    INTERFACE $<BUILD_INTERFACE:${HELICS_BINARY_DIR}/helics_generated_includes/>
)

# -------------------------------------------------------------
# global include directories
# -------------------------------------------------------------
target_include_directories(
    helics_base
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty/containers>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty/concurrency>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty/netif>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty/utilities>
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty/utilities/gmlc/utilities>
)
# the utilities/gmlc is to account for the transfer of the header to a known location on
# install

target_include_directories(
    helics_base SYSTEM INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/ThirdParty>
)

target_link_libraries(helics_base INTERFACE toml11::toml11)

if(NOT HELICS_DISABLE_BOOST)
    target_include_directories(
        helics_base SYSTEM INTERFACE $<BUILD_INTERFACE:${Boost_INCLUDE_DIR}>
    )
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
# -------------------------------------------------------------
# setting the RPATH
# -------------------------------------------------------------
if(NOT DEFINED CMAKE_MACOSX_RPATH)
    set(CMAKE_MACOSX_RPATH ON)
endif()

# add the automatically determined parts of the RPATH which point to directories outside
# the build tree to the install RPATH
if(NOT DEFINED CMAKE_INSTALL_RPATH_USE_LINK_PATH)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

# Add the local directory to the rpath
if(NOT APPLE)
    list(APPEND CMAKE_INSTALL_RPATH $ORIGIN)
    list(APPEND CMAKE_INSTALL_RPATH
                "\$ORIGIN/../${CMAKE_INSTALL_LIBDIR}"
    )
else()
    list(APPEND CMAKE_INSTALL_RPATH "@loader_path")
    list(APPEND CMAKE_INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
    list(APPEND CMAKE_INSTALL_RPATH "@executable_path")
    list(APPEND CMAKE_INSTALL_RPATH "@executable_path/../${CMAKE_INSTALL_LIBDIR}")
endif()


# the RPATH to be used when installing, but only if it's not a system directory
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
          "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}" isSystemDir
)
if(isSystemDir STREQUAL "-1")
    list(APPEND CMAKE_INSTALL_RPATH
                "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}"
    )
endif()

list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES
          "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}" isSystemDir
)
if(isSystemDir STREQUAL "-1")
    list(APPEND CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
endif()

if(NOT WIN32)
    if(ENABLE_ZMQ_CORE AND NOT HELICS_USE_ZMQ_STATIC_LIBRARY)
        get_target_property(zmqlibfile libzmq IMPORTED_LOCATION)
        if(NOT zmqlibfile)
            get_target_property(zmqlibfile libzmq IMPORTED_LOCATION_RELEASE)
        endif()
        # message(STATUS "zmqlib file =${zmqlibfile}")
        get_filename_component(zmqdir ${zmqlibfile} DIRECTORY)
        # message(STATUS "zmqdir path =${zmqdir}")
        if(APPLE)
            # TODO: Make sure it works for building ZMQ from source automatically using
            # CMake
            list(APPEND CMAKE_INSTALL_RPATH ${ZeroMQ_INSTALL_PATH}/lib)
            list(APPEND CMAKE_BUILD_RPATH ${ZeroMQ_INSTALL_PATH}/lib)
        endif()
        list(APPEND CMAKE_INSTALL_RPATH "${zmqdir}")
        list(APPEND CMAKE_BUILD_RPATH "${zmqdir}")
    endif()
endif()

option(HELICS_ENABLE_LOGGING "enable normal, debug, and trace logging in HELICS" ON)

cmake_dependent_advanced_option(
    HELICS_ENABLE_TRACE_LOGGING "enable trace logging" ON "HELICS_ENABLE_LOGGING" ON
)
cmake_dependent_advanced_option(
    HELICS_ENABLE_DEBUG_LOGGING "enable debug logging" ON "HELICS_ENABLE_LOGGING" ON
)

# -----------------------------------------------------------------------------
# create the fmt header only targets
# -----------------------------------------------------------------------------
include(addfmt)

# --------------------------------------------------------------
# Create the target for jsoncpp
# -----------------------------------------------------------
include(addJsoncpp)
add_library(HELICS::jsoncpp_lib ALIAS jsoncpp_lib)

# --------------------------------------------------------------
# Create the target for unitslib
# -----------------------------------------------------------
include(addUnits)
add_library(HELICS::units ALIAS units-static)

# -----------------------------------------------------------------------------
# create utilities target
# -----------------------------------------------------------------------------

set(GMLC_UTILITIES_INSTALL
    OFF
    CACHE INTERNAL ""
)
add_subdirectory(ThirdParty/utilities)

hide_variable(GMLC_UTILITIES_GENERATE_DOXYGEN_DOC)
hide_variable(GMLC_UTILITIES_INCLUDE_BOOST)
hide_variable(GMLC_UTILITIES_USE_BOOST_SPIRIT)
hide_variable(GMLC_UTILITIES_WITH_CMAKE_PACKAGE)
hide_variable(GMLC_UTILITIES_OBJECT_LIB)
hide_variable(GMLC_UTILITIES_STATIC_LIB)

add_library(HELICS::utilities ALIAS gmlc_utilities)
set_target_properties(gmlc_utilities PROPERTIES FOLDER Extern)
if(NOT HELICS_DISABLE_BOOST)
    target_include_directories(
        gmlc_utilities SYSTEM PRIVATE $<BUILD_INTERFACE:${Boost_INCLUDE_DIRS}>
    )
endif()

# -----------------------------------------------------------------------------
# CMAKE Subdirectories
# -----------------------------------------------------------------------------

add_subdirectory(src)

# -----------------------------------------------------------------------------
# Build the tests
# -----------------------------------------------------------------------------

if(HELICS_BUILD_TESTS AND BUILD_TESTING)
    if(NOT ENABLE_TEST_CORE)
        message(FATAL_ERROR "TEST CORE must be enabled to build the HELICS tests")
    endif()
    mark_as_advanced(BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(HELICS_BUILD_BENCHMARKS)
    if(NOT ENABLE_INPROC_CORE)
        message(
            FATAL_ERROR "INPROC CORE must be enabled to build the HELICS Benchmarks"
        )
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
        "if clang is found enable some custom targets for clang formatting and tidy"
        OFF "CMAKE_CXX_COMPILER_ID STREQUAL \"Clang\"" OFF
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
            "${CMAKE_SHARED_LIBRARY_PREFIX}helics-shared${CMAKE_SHARED_LIBRARY_SUFFIX}"
        )
        set(
            helics_cxx_shared_file_debug
            "${CMAKE_SHARED_LIBRARY_PREFIX}helics-shared${CMAKE_DEBUG_POSTFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}"
        )
    else(HELICS_BUILD_CXX_SHARED_LIB)
        set(helics_cxx_shared_file)
        set(helics_cxx_shared_file_debug)
    endif(HELICS_BUILD_CXX_SHARED_LIB)

    if(NOT HELICS_DISABLE_C_SHARED_LIB)
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
#]]
    if(HELICS_WITH_CMAKE_PACKAGE)

        set(HELICS_CMAKECONFIG_INSTALL_DIR
            "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
            CACHE STRING "install path for HELICSConfig.cmake"
        )
        mark_as_advanced(HELICS_CMAKECONFIG_INSTALL_DIR)

        # Export targets for importing build tree with find_package
        export(
            EXPORT helics-targets
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
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
            VERSION ${HELICS_VERSION}
            COMPATIBILITY SameMajorVersion
        )

        install(
            FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
                  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
            DESTINATION ${HELICS_CMAKECONFIG_INSTALL_DIR}
            COMPONENT libs
        )
    endif()

endif(NOT HELICS_BINARY_ONLY_INSTALL)

# combine 3rd party license files for install package
include(combineLicenses)
set(LICENSE_LIST
    "cereal"
    "${PROJECT_SOURCE_DIR}/ThirdParty/helics/external/cereal/LICENSE"
    "GMLC Concurrency"
    "${PROJECT_SOURCE_DIR}/ThirdParty/concurrency/LICENSE"
    "libGuarded"
    "${PROJECT_SOURCE_DIR}/ThirdParty/concurrency/gmlc/libguarded/LICENSE"
    "GMLC Containers"
    "${PROJECT_SOURCE_DIR}/ThirdParty/containers/LICENSE"
    "cppzmq"
    "${PROJECT_SOURCE_DIR}/ThirdParty/cppzmq/LICENSE"
    "fmt"
    "${PROJECT_SOURCE_DIR}/ThirdParty/fmtlib/LICENSE.rst"
    "units"
    "${PROJECT_SOURCE_DIR}/ThirdParty/units/LICENSE"
    "JsonCpp"
    "${PROJECT_SOURCE_DIR}/ThirdParty/jsoncpp/LICENSE"
    "toml11"
    "${PROJECT_SOURCE_DIR}/ThirdParty/toml11/LICENSE"
    "GMLC Utilities"
    "${PROJECT_SOURCE_DIR}/ThirdParty/utilities/LICENSE"
)

if(NOT CMAKE_VERSION VERSION_LESS 3.11)

    if(HELICS_BUILD_TESTS)
        list(APPEND LICENSE_LIST "Google Test"
                    "${PROJECT_BINARY_DIR}/_deps/googletest-src/LICENSE"
        )
    endif()

    if(HELICS_BUILD_BENCHMARKS)
        list(APPEND LICENSE_LIST "Google Benchmark"
                    "${PROJECT_BINARY_DIR}/_deps/gbenchmark-src/LICENSE"
        )
    endif()

    if(EXISTS ${PROJECT_BINARY_DIR}/_deps/libzmq-src AND ENABLE_ZMQ_CORE)
        list(APPEND LICENSE_LIST "ZeroMQ"
                    "${PROJECT_BINARY_DIR}/_deps/libzmq-src/COPYING"
        )
    endif()

else()
    if(HELICS_BUILD_TESTS)
        list(APPEND LICENSE_LIST "Google Test"
                    "${PROJECT_BINARY_DIR}/_deps/googletest/LICENSE"
        )
    endif()

    if(HELICS_BUILD_BENCHMARKS)
        list(APPEND LICENSE_LIST "Google Benchmark"
                    "${PROJECT_BINARY_DIR}/_deps/gbenchmark/LICENSE"
        )
    endif()

    if(EXISTS ${PROJECT_BINARY_DIR}/_deps/libzmq AND ENABLE_ZMQ_CORE)
        list(APPEND LICENSE_LIST "ZeroMQ" "${PROJECT_BINARY_DIR}/_deps/libzmq/COPYING")
    endif()
endif()

if(NOT HELICS_DISABLE_ASIO)
    list(APPEND LICENSE_LIST "asio"
                "${PROJECT_SOURCE_DIR}/ThirdParty/asio/asio/LICENSE_1_0.txt"
    )
endif()

combinelicenses(${PROJECT_BINARY_DIR}/THIRDPARTY_LICENSES ${LICENSE_LIST})

install(
    FILES LICENSE NOTICE CHANGELOG.md README.md
          ${PROJECT_BINARY_DIR}/THIRDPARTY_LICENSES
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    COMPONENT libs
)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs/man/manpage_out/
    DESTINATION ${CMAKE_INSTALL_MANDIR}/man1
    FILES_MATCHING PATTERN "*.1"
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
            ${CMAKE_CURRENT_SOURCE_DIR}/config/packaging/helicsSharedLib.pc.in
            ${CMAKE_CURRENT_BINARY_DIR}/helicsSharedLib.pc @ONLY
        )
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/helicsSharedLib.pc
                DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
        )
    endif()

    if(ENABLE_ZMQ_CORE AND NOT HELICS_USE_ZMQ_STATIC_LIBRARY)
        set(zmq_pc_require libzmq)
    endif()

    if(HELICS_BUILD_CXX_SHARED_LIB)

        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/config/packaging/helics-shared.pc.in
            ${CMAKE_CURRENT_BINARY_DIR}/helics-shared.pc @ONLY
        )
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/helics-shared.pc
                DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
        )

        if(HELICS_BUILD_APP_LIBRARY)
            configure_file(
                ${CMAKE_CURRENT_SOURCE_DIR}/config/packaging/helics-apps-shared.pc.in
                ${CMAKE_CURRENT_BINARY_DIR}/helics-apps-shared.pc @ONLY
            )
            install(FILES ${CMAKE_CURRENT_BINARY_DIR}/helics-apps-shared.pc
                    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
            )
        endif()

    endif()
endif()

# -------------------------------------------------------------
# CPack
# -------------------------------------------------------------
cmake_dependent_option(
    HELICS_ENABLE_PACKAGE_BUILD
    "Add projects for making packages and installers for HELICS" OFF
    "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" OFF
)

if(HELICS_ENABLE_PACKAGE_BUILD)

    if(CMAKE_VERSION VERSION_LESS 3.13)
        if(HELICS_ZMQ_SUBPROJECT OR HELICS_ZMQ_FORCE_SUBPROJECT)
            if(ENABLE_ZMQ_CORE)
                message(
                    FATAL_ERROR
                        "CMAKE 3.13 or higher is required to package a subproject of ZeroMQ with HELICS"
                )
            endif()
        endif()
    endif()
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
        set(CPACK_RESOURCE_FILE_README "${HELICS_SOURCE_DIR}\\\\README.md")
    else(WIN32)
        set(CPACK_RESOURCE_FILE_LICENSE "${HELICS_SOURCE_DIR}/LICENSE")
        set(CPACK_RESOURCE_FILE_README "${HELICS_SOURCE_DIR}/README.md")
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

    set(CPACK_COMPONENT_APPLICATION_DESCRIPTION
        "Executables and helper applications for HELICS"
    )
    set(CPACK_COMPONENT_LIBS_DESCRIPTION
        "Libraries for compiling and linking with HELICS"
    )
    set(CPACK_COMPONENT_HEADERS_DESCRIPTION
        "Headers for linking and compiling with HELICS"
    )
    set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "Runtime libraries for HELICS")
    set(CPACK_COMPONENT_CEREAL_DESCRIPTION
        "cereal C++11 serialization library [no support for other language interfaces]"
    )
    set(CPACK_COMPONENT_SWIG_DESCRIPTION
        "Basic swig interface files for HELICS for building 3rd party language interfaces"
    )
    set(CPACK_COMPONENT_GROUP_INTERFACES_DESCRIPTION
        "additional language interfaces for HELICS"
    )

    set(CPACK_COMPONENT_LIBS_DEPENDS headers)
    set(CPACK_COMPONENT_RUNTIME_REQUIRED ON)

    set(CPACK_PACKAGE_EXECUTABLES
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
        set(CPACK_PACKAGE_ICON "${HELICS_SOURCE_DIR}\\\\docs\\\\img\\\\HELICS.ico")
        set(CPACK_NSIS_MUI_ICON "${HELICS_SOURCE_DIR}/docs/img/HELICS.ico")
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
        set(CPACK_SYSTEM_NAME "macOS-${CMAKE_SYSTEM_PROCESSOR}")
        set(CPACK_DMG_DISABLE_APPLICATIONS_SYMLINK ON)
        set(CPACK_PACKAGE_ICON "${HELICS_SOURCE_DIR}/docs/img/HELICS.ico")
    elseif(NOT WIN32 AND NOT CYGWIN)
        set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
    endif()

	#for debian
	set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "all")
    set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")
    set(CPACK_DEBIAN_PACKAGE_NAME "helics-dev")

    set(CPACK_SOURCE_IGNORE_FILES "/Build*/;/build*/;/.git/")
    # THIS LINE MUST BE LAST IN THE CPACK SECTION
    include(CPack)
endif(HELICS_ENABLE_PACKAGE_BUILD)

if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # restore the original module path
    set(CMAKE_MODULE_PATH ${ORIGINAL_MODULE_PATH})
    if(old_build_shared)
        set(BUILD_SHARED_LIBS
            ON
            CACHE BOOL "" FORCE
        )
    endif()
endif()

# -------------------------------------------------------------
# Git Hooks
# -------------------------------------------------------------
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(addGitHooks)
endif ()
