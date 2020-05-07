# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

cmake_minimum_required(VERSION 3.4.0)

project(HELICS-find-package-test)

include(CTest)

# Test the HELICS CMake package config
find_package(HELICS 2.3 REQUIRED)

# add some tests inside of the CMake
if(NOT TARGET HELICS::helics_broker)
    message(FATAL_ERROR "unable to locate HELICS_BROKER target")
endif()

# Test the CXX shared library target
if(HELICS_BUILD_CXX_SHARED_LIB)
    add_executable(shared-library-test-exe ../pkg_test_code/main.cpp)
    target_link_libraries(shared-library-test-exe HELICS::helics-shared)

    add_test(NAME shared-library-test COMMAND shared-library-test-exe)
    set_property(TEST shared-library-test PROPERTY PASS_REGULAR_EXPRESSION "${HELICS_VERSION}")

    # Test the helics_app shared library target
    if(HELICS_BUILD_APP_LIBRARY)
        add_executable(apps-library-test-exe ../pkg_test_code/main-apps.cpp)
        target_link_libraries(apps-library-test-exe PUBLIC HELICS::helics-apps-shared)

        add_test(NAME apps-library-test COMMAND apps-library-test-exe)
        set_property(TEST apps-library-test PROPERTY PASS_REGULAR_EXPRESSION "${HELICS_VERSION}")
    endif()

endif()

# Test the C and C++98 targets
if(NOT HELICS_DISABLE_C_SHARED_LIB)
    add_executable(c-shared-library-test-exe ../pkg_test_code/main.c)
    target_link_libraries(c-shared-library-test-exe HELICS::helicsSharedLib)

    add_test(NAME c-shared-library-test COMMAND c-shared-library-test-exe)
    set_property(TEST c-shared-library-test PROPERTY PASS_REGULAR_EXPRESSION "${HELICS_VERSION}")

    add_executable(cpp98-shared-library-test-exe ../pkg_test_code/main98.cpp)
    target_link_libraries(cpp98-shared-library-test-exe HELICS::helicsCpp98)
    set_property(TARGET cpp98-shared-library-test-exe PROPERTY CXX_STANDARD 98)

    add_test(NAME cpp98-shared-library-test COMMAND cpp98-shared-library-test-exe)
    set_property(
        TEST cpp98-shared-library-test PROPERTY PASS_REGULAR_EXPRESSION "${HELICS_VERSION}"
    )

endif()
