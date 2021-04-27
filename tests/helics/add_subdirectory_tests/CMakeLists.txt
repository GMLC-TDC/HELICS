# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

cmake_minimum_required(VERSION 3.4.0)

project(HELICS-find-package-test)

# Add HELICS as a subproject
add_subdirectory(../../.. helics)

# Test the CXX static library target
add_executable(static-library-test ../pkg_test_code/main.cpp)
target_link_libraries(static-library-test HELICS::application_api)

# Test the CXX shared library target
if(HELICS_BUILD_CXX_SHARED_LIB)
    add_executable(shared-library-test ../pkg_test_code/main.cpp)
    target_link_libraries(shared-library-test HELICS::helics-shared)
endif()

# Test the C and C++98 targets
if(NOT HELICS_DISABLE_C_SHARED_LIB)
    add_executable(c-shared-library-test ../pkg_test_code/main.c)
    target_link_libraries(c-shared-library-test HELICS::helicsSharedLib)

    add_executable(cpp98-shared-library-test ../pkg_test_code/main98.cpp)
    target_link_libraries(cpp98-shared-library-test HELICS::helicsCpp98)
    set_property(TARGET cpp98-shared-library-test PROPERTY CXX_STANDARD 98)
endif()

# Test the helics_app static library target
if(HELICS_BUILD_APP_LIBRARY)
    add_executable(apps-library-test ../pkg_test_code/main-apps.cpp)
    target_link_libraries(apps-library-test HELICS::apps)
endif()
