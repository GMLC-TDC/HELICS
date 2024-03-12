# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2024, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(common_test_headers testFixtures.hpp ../coreTypeLists.hpp)

set(common_test_sources testFixtures.cpp applicationApiTests.cpp)

set(valueFederate_test_sources
    ValueFederateKeyTests.cpp
    ValueFederateTestTemplates.hpp
    PrimaryTypeConversionTests.cpp
    ValueConverterTests.cpp
    subPubObjectTests.cpp
    ValueFederateAdditionalTests.cpp
    ValueFederateSingleTransfer.cpp
    ValueFederateDualTransfer.cpp
    helicsTypeTests.cpp
    MultiInputTests.cpp
    DynamicValueFederateTests.cpp
)

set(messageFederate_test_sources MessageFederateKeyTests.cpp MessageFederateAdditionalTests.cpp
                                 CombinationFederateTests.cpp EndpointLinkingTests.cpp
)

set(filterTranslator_test_sources FilterTests.cpp TranslatorTests.cpp FilterAdditionalTests.cpp)

set(application_api_test_sources
    CallbackFederateTests.cpp
    data_viewTests.cpp
    FederateTests.cpp
    GrantTimeoutTests.cpp
    LoggingTests.cpp
    FederateInfoTests.cpp
)

if(HELICS_ENABLE_ZMQ_CORE)
    list(APPEND application_api_test_sources zmqSSTests.cpp)
endif()

add_executable(
    applicationApiTests ${application_api_test_sources} ${common_test_headers}
                        ${common_test_sources}
)

add_executable(
    valueFederateTests ${valueFederate_test_sources} ${common_test_headers} ${common_test_sources}
)

add_executable(
    messageFederateTests ${messageFederate_test_sources} ${common_test_headers}
                         ${common_test_sources}
)

add_executable(
    filterTranslatorTests ${filterTranslator_test_sources} ${common_test_headers}
                          ${common_test_sources}
)

set(testExecutables applicationApiTests valueFederateTests messageFederateTests
                    filterTranslatorTests
)

foreach(testTarget IN LISTS testExecutables)
    target_link_libraries(${testTarget} PUBLIC helics_application_api helics_test_base gmock)

    set_target_properties(${testTarget} PROPERTIES FOLDER tests)

    target_compile_definitions(
        ${testTarget} PRIVATE "-DHELICS_BROKER_LOCATION=\"${HELICS_BROKER_LOC}\""
    )
    target_compile_definitions(
        ${testTarget} PRIVATE "-DHELICS_INSTALL_LOCATION=\"${CMAKE_INSTALL_PREFIX}\""
    )
    target_compile_definitions(
        ${testTarget} PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../test_files/\""
    )

    add_test(NAME ${testTarget} COMMAND ${testTarget})
    set_property(TEST ${testTarget} PROPERTY LABELS ApplicationApi Coverage Daily)

    # Tests for Continuous Integration builds
    add_test(NAME ${testTarget}-ci COMMAND ${testTarget} --gtest_filter=-*ci_skip*:*nosan*)
    set_property(TEST ${testTarget}-ci PROPERTY LABELS ApplicationApiCI Continuous)

endforeach()

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
    applicationApiTests PRIVATE "-DLONG_TEST_NAME=\"${LONG_TEST_PATH}/${LONG_FILE_NAME}\""
)

# shared cxx library tests
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
