#
# Copyright © 2017-2019, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved. 
# 
# SPDX-License-Identifier: BSD-3-Clause
#

set(
  c_shared_library_test_sources 
  ctestFixtures.cpp 
  shared-library-tests.cpp
  # data_viewTests.cpp
  test-value-federate1.cpp 
  test-value-federate2.cpp
  # ValueFederateTests2.cpp
  FilterTests.cpp 
  test-message-federate.cpp
  # test-message-federate_cpp.cpp
  # FederateTests.cpp
  TimingTests.cpp
  # iterationTests.cpp
  QueryTests.cpp 
  badInputTests.cpp
  comboConfigTests.cpp
)

set(
  cpp_shared_library_test_sources 
  cpptestFixtures.cpp 
  cppshared-library-tests.cpp
  # test-value-federate1_cpp.cpp
  test-value-federate2_cpp.cpp
  # FilterTests_cpp.cpp
  test-message-federate_cpp.cpp
  # FederateTests.cpp
  # TimingTests.cpp
  # iterationTests.cpp
  # subPubObjectTests.cpp
  # QueryTests.cpp
)

add_executable(shared-library-tests ${c_shared_library_test_sources} ctestFixtures.hpp ../coreTypeLists.hpp)

add_executable(
  shared-library-tests-cpp ${cpp_shared_library_test_sources} cpptestFixtures.hpp ../coreTypeLists.hpp
)

target_link_libraries(
  shared-library-tests
  helicsSharedLib
  helics_boost_test_base
)
target_link_libraries(
  shared-library-tests-cpp
  helicsSharedLib
  helics_boost_test_base
)

add_dependencies(shared-library-tests helicsSharedLib)
set_target_properties(
  shared-library-tests shared-library-tests-cpp
  PROPERTIES FOLDER tests
)

target_compile_definitions(
  shared-library-tests PRIVATE "-DHELICS_BROKER_LOCATION=\"${HELICS_BROKER_LOC}\""
)
target_compile_definitions(
  shared-library-tests PRIVATE "-DHELICS_INSTALL_LOCATION=\"${CMAKE_INSTALL_PREFIX}\""
)

target_compile_definitions(
  shared-library-tests PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../test_files/\""
)

add_test(
  NAME shared-library-tests
  COMMAND shared-library-tests --log_level=message --report_level=short
)

target_compile_definitions(
  shared-library-tests-cpp PRIVATE "-DHELICS_BROKER_LOCATION=\"${HELICS_BROKER_LOC}\""
)
target_compile_definitions(
  shared-library-tests-cpp PRIVATE
  "-DHELICS_INSTALL_LOCATION=\"${CMAKE_INSTALL_PREFIX}\""
)

target_compile_definitions(
  shared-library-tests-cpp PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/../test_files/\""
)

add_test(
  NAME shared-library-tests-cpp
  COMMAND shared-library-tests-cpp --log_level=message --report_level=short
)

set_property(TEST shared-library-tests PROPERTY LABELS SharedLibrary Coverage Daily)

set_property(TEST shared-library-tests-cpp PROPERTY LABELS SharedLibraryCpp Coverage Daily)

# Tests used for Continuous Integration builds
add_test(
  NAME shared-library-ci-tests
  COMMAND
    shared-library-tests
    --run_test=@ci
    --log_level=test_suite
    --report_level=short
)
add_test(
  NAME shared-library-ci-tests-cpp
  COMMAND
    shared-library-tests-cpp
    --run_test=@ci
    --log_level=test_suite
    --report_level=short
)

set_property(TEST shared-library-ci-tests PROPERTY LABELS Continuous)
set_property(TEST shared-library-ci-tests-cpp PROPERTY LABELS Continuous)

  copy_key_files_to_target_location(shared-library-tests)
 copy_shared_target(shared-library-tests)

