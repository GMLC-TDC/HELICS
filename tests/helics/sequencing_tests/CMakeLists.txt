# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2024, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(sequencing_test_sources
    Sequencing1.cpp
    Sequencing2.cpp
    sequencing-tests.cpp
    sequencingHelpers.hpp
    sequencingHelpers.cpp
    ../application_api/testFixtures.cpp
    ../application_api/testFixtures.hpp
    ../coreTypeLists.hpp
)

add_executable(sequencing-tests ${sequencing_test_sources})
target_link_libraries(sequencing-tests helics_application_api helics_test_base gmock)

set_target_properties(sequencing-tests PROPERTIES FOLDER tests)

target_compile_definitions(
    sequencing-tests PRIVATE "-DHELICS_BROKER_LOCATION=\"${HELICS_BROKER_LOC}\""
)
target_compile_definitions(
    sequencing-tests PRIVATE "-DHELICS_INSTALL_LOCATION=\"${CMAKE_INSTALL_PREFIX}\""
)
target_compile_definitions(
    sequencing-tests PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../test_files/\""
)

# add_test(NAME application-api-tests COMMAND application-api-tests) set_property(TEST
# application-api-tests PROPERTY LABELS ApplicationApi Coverage Daily)

# Tests for Continuous Integration builds add_test(NAME application-api-ci-tests COMMAND
# application-api-tests --gtest_filter=-*ci_skip*) set_property(TEST application-api-ci-tests
# PROPERTY LABELS ApplicationApiCI Continuous)
