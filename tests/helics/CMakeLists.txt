#
# Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause
#

# add a baseline library for underlying dependencies and flags for test executables
if (BUILD_BOOST_TESTS)
add_library(helics_boost_test_base INTERFACE)
target_link_libraries(helics_boost_test_base INTERFACE Boostlibs::test)
target_link_libraries(helics_boost_test_base INTERFACE helics_base_includes compile_flags_target)
endif()

include(AddGoogletest)
add_library(helics_google_test_base INTERFACE)
target_link_libraries(helics_google_test_base INTERFACE gtest gtest_main gmock)
target_link_libraries(helics_google_test_base INTERFACE helics_base_includes compile_flags_target)

add_subdirectory(core)
add_subdirectory(common)
add_subdirectory(system_tests)
add_subdirectory(apps)

if (BUILD_BOOST_TESTS)

add_subdirectory(application_api)

if(NOT DISABLE_C_SHARED_LIB)
    add_subdirectory(shared_library)
endif()

add_subdirectory(performance_tests)
endif()

# Tests for other CMake projects including and using HELICS using find_package
add_test(find-package-tests ${CMAKE_CTEST_COMMAND} -C ${CMAKE_BUILD_TYPE}
                            --build-and-test "${CMAKE_CURRENT_SOURCE_DIR}/find_package_tests" "${CMAKE_CURRENT_BINARY_DIR}/find_package_tests"
                            --build-generator "${CMAKE_GENERATOR}"
                            --build-generator-platform "${CMAKE_GENERATOR_PLATFORM}"
                            --build-options
                            "-DHELICS_DIR=${PROJECT_BINARY_DIR}"
                            "-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}"
                            "-DDISABLE_C_SHARED_LIB=${DISABLE_C_SHARED_LIB}"
                            "-DBUILD_APPS=${BUILD_APPS}"
                            "-DZeroMQ_INSTALL_PATH=${ZeroMQ_INSTALL_PATH}"
                            "-DBOOST_INSTALL_PATH=${BOOST_INSTALL_PATH}"
                            "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
                            "-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}"
)
set_property(TEST find-package-tests PROPERTY LABELS Packaging PackagingFast)

option(
    ENABLE_SLOW_PACKAGING_TESTS
    "Enable the packaging tests that may take a long time to run" OFF
)
mark_as_advanced(ENABLE_SLOW_PACKAGING_TESTS)

# Tests for adding HELICS as a subproject using add_subdirectory
if (ENABLE_SLOW_PACKAGING_TESTS)
    add_test(add-subdirectory-tests ${CMAKE_CTEST_COMMAND} -C ${CMAKE_BUILD_TYPE}
                                --build-and-test "${CMAKE_CURRENT_SOURCE_DIR}/add_subdirectory_tests" "${CMAKE_CURRENT_BINARY_DIR}/add_subdirectory_tests"
                                --build-generator "${CMAKE_GENERATOR}"
                                --build-generator-platform "${CMAKE_GENERATOR_PLATFORM}"
                                --build-options
                                "-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}"
                                "-DDISABLE_C_SHARED_LIB=${DISABLE_C_SHARED_LIB}"
                                "-DBUILD_APPS=${BUILD_APPS}"
                                "-DBUILD_HELICS_TESTS=OFF"
                                "-DBUILD_HELICS_EXAMPLES=OFF"
                                "-DZeroMQ_INSTALL_PATH=${ZeroMQ_INSTALL_PATH}"
                                "-DBOOST_INSTALL_PATH=${BOOST_INSTALL_PATH}"
                                "-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}"
                                "-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}"
    )
    set_property(TEST add-subdirectory-tests PROPERTY LABELS Packaging)
endif()
