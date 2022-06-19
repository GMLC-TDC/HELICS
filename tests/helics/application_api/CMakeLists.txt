# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2022, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(application_api_test_headers testFixtures.hpp ValueFederateTestTemplates.hpp
                                 ../coreTypeLists.hpp
)

set(application_api_test_sources
    MessageFederateKeyTests.cpp
    ValueFederateKeyTests.cpp
    FilterTests.cpp
    TranslatorTests.cpp
    testFixtures.cpp
    PrimaryTypeConversionTests.cpp
    FilterAdditionalTests.cpp
    ValueConverterTests.cpp
    application-api-tests.cpp
    data_viewTests.cpp
    MessageFederateAdditionalTests.cpp
    FederateTests.cpp
    subPubObjectTests.cpp
    GrantTimeoutTests.cpp
    ValueFederateAdditionalTests.cpp
    CombinationFederateTests.cpp
    ValueFederateSingleTransfer.cpp
    ValueFederateDualTransfer.cpp
    helicsTypeTests.cpp
    LoggingTests.cpp
    FederateInfoTests.cpp
    MultiInputTests.cpp
)

if(HEILCS_ENABLE_ZMQ_CORE)
    list(APPEND application-api-tests zmqSSTests.cpp)
endif()

add_executable(
    application-api-tests ${application_api_test_sources} ${application_api_test_headers}
)
target_link_libraries(application-api-tests helics_application_api helics_test_base gmock)

set_target_properties(application-api-tests PROPERTIES FOLDER tests)

target_compile_definitions(
    application-api-tests PRIVATE "-DHELICS_BROKER_LOCATION=\"${HELICS_BROKER_LOC}\""
)
target_compile_definitions(
    application-api-tests PRIVATE "-DHELICS_INSTALL_LOCATION=\"${CMAKE_INSTALL_PREFIX}\""
)
target_compile_definitions(
    application-api-tests PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../test_files/\""
)

# generate a file for the extra long file name tests
set(LONG_TEST_PATH
    ${CMAKE_CURRENT_BINARY_DIR}/test_files/extra_files_testing_for_very_long_file_paths
)
set(LONG_FILE_NAME
    this_is_a_ridiculously_long_file_name_to_test_some_odd_conditions_in_the_toml_parser_so_we_need_a_long_file_name.toml
)
file(MAKE_DIRECTORY ${LONG_TEST_PATH})

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../test_files/example_filters.toml
    ${LONG_TEST_PATH}/${LONG_FILE_NAME} COPYONLY
)

target_compile_definitions(
    application-api-tests PRIVATE "-DLONG_TEST_NAME=\"${LONG_TEST_PATH}/${LONG_FILE_NAME}\""
)

add_test(NAME application-api-tests COMMAND application-api-tests)
set_property(TEST application-api-tests PROPERTY LABELS ApplicationApi Coverage Daily)

# Tests for Continuous Integration builds
add_test(NAME application-api-ci-tests COMMAND application-api-tests
                                               --gtest_filter=-*ci_skip*:*nosan*
)
set_property(TEST application-api-ci-tests PROPERTY LABELS ApplicationApiCI Continuous)

if(HELICS_BUILD_CXX_SHARED_LIB)
    set(shared_cxx_test_sources MessageFederateKeyTests.cpp ValueFederateKeyTests.cpp
                                FilterTests.cpp testFixtures_shared.cpp testFixtures_shared.hpp
    )

    add_executable(shared_cxx-tests ${shared_cxx_test_sources})

    target_link_libraries(shared_cxx-tests helics_test_base HELICS::helicscpp)

    set_target_properties(shared_cxx-tests PROPERTIES FOLDER tests)

    add_test(NAME shared_cxx-tests COMMAND shared_cxx-tests)
    set_property(TEST shared_cxx-tests PROPERTY LABELS ApplicationApi Coverage Daily)

    target_compile_definitions(
        shared_cxx-tests PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../test_files/\""
    )

endif()
