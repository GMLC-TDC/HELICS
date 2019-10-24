# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2019, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

cmake_minimum_required(VERSION 3.4.0)

project(HELICS-find-package-test)

# Test the HELICS CMake package config
find_package(HELICS 2.3 REQUIRED)

# add some tests inside of the CMake
if(NOT TARGET HELICS::helics_broker)
    message(FATAL_ERROR "unable to locate HELICS_BROKER target")
endif()

# Test the CXX shared library target
if(HELICS_BUILD_CXX_SHARED_LIB)
    add_executable(shared-library-test main.cpp)
    target_link_libraries(shared-library-test HELICS::helics-shared)
endif()

# Test the C and C++98 targets
if(NOT HELICS_DISABLE_C_SHARED_LIB)
    add_executable(c-shared-library-test main.c)
    target_link_libraries(c-shared-library-test HELICS::helicsSharedLib)

    add_executable(cpp98-shared-library-test main98.cpp)
    target_link_libraries(cpp98-shared-library-test HELICS::helicsCpp98)
    set_property(TARGET cpp98-shared-library-test PROPERTY CXX_STANDARD 98)
endif()

