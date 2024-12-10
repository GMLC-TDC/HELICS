# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2024, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(c_shared_library_test_sources
    ctestFixtures.cpp
    shared-library-tests.cpp
    test-value-federate1.cpp
    test-value-federate2.cpp
    FilterTests.cpp
    TranslatorTests.cpp
    test-message-federate.cpp
    SystemTests.cpp
    TimingTests.cpp
    evilInputTests.cpp
    iterationTests.cpp
    QueryTests.cpp
    DataAPITests.cpp
    badInputTests.cpp
    comboConfigTests.cpp
    loggingTests.cpp
    PropertyIndexTests.cpp
    commandInterfaceTests.cpp
    miscApiTests.cpp
    CallbackFederateTests.cpp
)

if(TARGET HELICS::apps)
    list(APPEND c_shared_library_test_sources appTests.cpp)
endif()

set(cpp_shared_library_test_sources
    cpptestFixtures.cpp
    cppshared-library-tests.cpp
    # test-value-federate1_cpp.cpp
    test-value-federate2_cpp.cpp
    # FilterTests_cpp.cpp
    test-message-federate_cpp.cpp
    test-combo-federate_cpp.cpp
    timingTests_cpp.cpp
    QueryTests_cpp.cpp
    MultiInputTests.cpp
)

add_executable(
    shared-library-tests ${c_shared_library_test_sources} ctestFixtures.hpp ../coreTypeLists.hpp
)

add_executable(
    shared-library-tests-cpp ${cpp_shared_library_test_sources} cpptestFixtures.hpp
                             ../coreTypeLists.hpp
)

target_link_libraries(shared-library-tests helics_test_base helics build_flags_target)
target_link_libraries(shared-library-tests-cpp helics_test_base helicsCpp98 build_flags_target)

set_target_properties(shared-library-tests shared-library-tests-cpp PROPERTIES FOLDER tests)

target_compile_definitions(
    shared-library-tests PRIVATE "-DHELICS_BROKER_LOCATION=\"${HELICS_BROKER_LOC}\""
)
target_compile_definitions(
    shared-library-tests PRIVATE "-DHELICS_INSTALL_LOCATION=\"${CMAKE_INSTALL_PREFIX}\""
)

target_compile_definitions(
    shared-library-tests PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../test_files/\""
)

if(TARGET HELICS::apps)
    target_compile_definitions(
        shared-library-tests
        PRIVATE "-DAPP_TEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../apps/test_files/\""
    )
endif()

add_test(NAME shared-library-tests COMMAND shared-library-tests)

target_compile_definitions(
    shared-library-tests-cpp PRIVATE "-DHELICS_BROKER_LOCATION=\"${HELICS_BROKER_LOC}\""
)
target_compile_definitions(
    shared-library-tests-cpp PRIVATE "-DHELICS_INSTALL_LOCATION=\"${CMAKE_INSTALL_PREFIX}\""
)

target_compile_definitions(
    shared-library-tests-cpp PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../test_files/\""
)

add_test(NAME shared-library-tests-cpp COMMAND shared-library-tests-cpp)

set_property(TEST shared-library-tests PROPERTY LABELS SharedLibrary Coverage Daily)

set_property(TEST shared-library-tests-cpp PROPERTY LABELS SharedLibraryCpp Coverage Daily)

# Tests used for Continuous Integration builds
add_test(NAME shared-library-ci-tests COMMAND shared-library-tests
                                              --gtest_filter=-*ci_skip*:*nosan*
)
add_test(NAME shared-library-ci-tests-cpp COMMAND shared-library-tests-cpp
                                                  --gtest_filter=-*ci_skip*:*nosan*
)

set_property(TEST shared-library-ci-tests PROPERTY LABELS SharedLibraryCI Continuous)
set_property(TEST shared-library-ci-tests-cpp PROPERTY LABELS SharedLibraryCppCI Continuous)
