# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2024, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(system_test_sources
    helics_system_tests.cpp
    TimingTests.cpp
    iterationTests.cpp
    ErrorTests.cpp
    TimingTests2.cpp
    QueryTests.cpp
    heat-transfer-tests.cpp
    flagTests.cpp
    updateTests.cpp
    networkTests.cpp
    commandInterfaceTests.cpp
    profilingTests.cpp
    dynamicFederationTests.cpp
    messageSizeTests.cpp
    ../application_api/testFixtures.cpp
    ../application_api/testFixtures.hpp
    ../apps/exeTestHelper.cpp
    ../apps/exeTestHelper.h
)

if(NOT HELICS_DISABLE_ASIO)
    list(APPEND system_test_sources federateRealTimeTests.cpp brokerTimeoutTests.cpp)
endif()

add_executable(system-tests ${system_test_sources} ${system_test_headers})
target_link_libraries(system-tests helics_application_api helics_test_base)

set_target_properties(system-tests PROPERTIES FOLDER tests)

target_compile_definitions(system-tests PRIVATE "-DHELICS_BROKER_LOCATION=\"${HELICS_BROKER_LOC}\"")
target_compile_definitions(
    system-tests PRIVATE "-DHELICS_INSTALL_LOCATION=\"${CMAKE_INSTALL_PREFIX}\""
)
target_compile_definitions(system-tests PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")
target_compile_definitions(
    system-tests PRIVATE "-DTEST_BIN_DIR=\"${CMAKE_CURRENT_BINARY_DIR}/../test_files/\""
)

add_test(NAME system-tests COMMAND system-tests)
set_property(TEST system-tests PROPERTY LABELS System Coverage Daily TSAN)

# Tests for Continuous Integration builds
add_test(NAME system-ci-tests COMMAND system-tests --gtest_filter=-*ci_skip*:*nosan*)
set_property(TEST system-ci-tests PROPERTY LABELS SystemCI Continuous Valgrind)
