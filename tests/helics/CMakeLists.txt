# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# add a baseline library for underlying dependencies and flags for test executables

include(AddGoogletest)

# add a baseline library for underlying dependencies and flags for test executables
add_library(helics_test_base INTERFACE)
target_link_libraries(helics_test_base INTERFACE gtest gtest_main gmock)
target_link_libraries(helics_test_base INTERFACE compile_flags_target)

add_subdirectory(core)
add_subdirectory(common)
add_subdirectory(system_tests)

if(HELICS_BUILD_APP_LIBRARY)
    add_subdirectory(apps)
endif()

add_subdirectory(application_api)

if(NOT HELICS_DISABLE_C_SHARED_LIB)
    add_subdirectory(shared_library)
endif()

if(HELICS_INSTALL_PACKAGE_TESTS)
    set(HELICS_PACKAGE_SEARCH_LOC)
else()
    set(HELICS_PACKAGE_SEARCH_LOC "-DHELICS_DIR=${PROJECT_BINARY_DIR}")
endif()

if(NOT MSVC)
    set(package_test_command --test-command "${CMAKE_CTEST_COMMAND}")
else() # don't try to run the tests on MSVC since that would require copying the dll's
       # and doing some other setup that isn't that important to run on all OS
    set(package_test_command)
endif()

# Tests for other CMake projects including and using HELICS using find_package
add_test(
    find-package-tests
    ${CMAKE_CTEST_COMMAND}
    -C
    ${CMAKE_BUILD_TYPE}
    --build-and-test
    "${CMAKE_CURRENT_SOURCE_DIR}/find_package_tests"
    "${CMAKE_CURRENT_BINARY_DIR}/find_package_tests"
    --build-generator
    "${CMAKE_GENERATOR}"
    --build-generator-platform
    "${CMAKE_GENERATOR_PLATFORM}"
    --build-options
    ${HELICS_PACKAGE_SEARCH_LOC}
    "-DHELICS_BUILD_CXX_SHARED_LIB=${HELICS_BUILD_CXX_SHARED_LIB}"
    "-DHELICS_DISABLE_C_SHARED_LIB=${HELICS_DISABLE_C_SHARED_LIB}"
    "-DHELICS_BUILD_APP_LIBRARY=${HELICS_BUILD_APP_LIBRARY}"
    "-DHELICS_VERSION=${HELICS_VERSION}"
    "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
    "-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}"
    ${package_test_command}
)
set_property(TEST find-package-tests PROPERTY LABELS Packaging PackagingFast)

cmake_dependent_advanced_option(
    HELICS_ENABLE_SUBPROJECT_TESTS
    "Enable tests of using HELICS as a subproject, potentially a slow tests" OFF
    "HELICS_BUILD_TESTS" OFF
)

# Tests for adding HELICS as a subproject using add_subdirectory
if(HELICS_ENABLE_SUBPROJECT_TESTS)
    add_test(
        add-subdirectory-tests
        ${CMAKE_CTEST_COMMAND}
        -C
        ${CMAKE_BUILD_TYPE}
        --build-and-test
        "${CMAKE_CURRENT_SOURCE_DIR}/add_subdirectory_tests"
        "${CMAKE_CURRENT_BINARY_DIR}/add_subdirectory_tests"
        --build-generator
        "${CMAKE_GENERATOR}"
        --build-generator-platform
        "${CMAKE_GENERATOR_PLATFORM}"
        --build-options
        "-DHELICS_BUILD_CXX_SHARED_LIB=${HELICS_BUILD_CXX_SHARED_LIB}"
        "-DHELICS_DISABLE_C_SHARED_LIB=${HELICS_DISABLE_C_SHARED_LIB}"
        "-DHELICS_BUILD_APP_LIBRARY=${HELICS_BUILD_APP_LIBRARY}"
        "-DHELICS_VERSION=${HELICS_VERSION}"
        "-DZeroMQ_INSTALL_PATH=${ZeroMQ_INSTALL_PATH}"
        "-DBOOST_INSTALL_PATH=${BOOST_INSTALL_PATH}"
        "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
        "-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}"
    )
    set_property(TEST add-subdirectory-tests PROPERTY LABELS Packaging)
endif()

if(NOT MSVC AND HELICS_INSTALL_PACKAGE_TESTS)
    if(NOT CMAKE_VERSION VERSION_LESS 3.6.0)
        # Tests for other CMake projects using the package_config files
        add_test(
            package-config-tests
            ${CMAKE_CTEST_COMMAND}
            -C
            ${CMAKE_BUILD_TYPE}
            --build-and-test
            "${CMAKE_CURRENT_SOURCE_DIR}/package_config_tests"
            "${CMAKE_CURRENT_BINARY_DIR}/package_config_tests"
            --build-generator
            "${CMAKE_GENERATOR}"
            --build-generator-platform
            "${CMAKE_GENERATOR_PLATFORM}"
            --build-options
            "-DHELICS_BUILD_CXX_SHARED_LIB=${HELICS_BUILD_CXX_SHARED_LIB}"
            "-DHELICS_DISABLE_C_SHARED_LIB=${HELICS_DISABLE_C_SHARED_LIB}"
            "-DHELICS_BUILD_APP_LIBRARY=${HELICS_BUILD_APP_LIBRARY}"
            "-DHELICS_VERSION=${HELICS_VERSION}"
            "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
            "-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}"
            ${package_test_command}
        )
        set_property(TEST package-config-tests PROPERTY LABELS Packaging)
    endif()
endif()
