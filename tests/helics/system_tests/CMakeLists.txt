#
# Copyright © 2017-2019, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved. 
# 
# SPDX-License-Identifier: BSD-3-Clause
#

set(
  system_test_sources
  helics_system_tests.cpp
  TimingTests.cpp
  iterationTests.cpp
  ErrorTests.cpp
  federateRealTimeTests.cpp
  TimingTests2.cpp
  QueryTests.cpp
  heat-transfer-tests.cpp
  flagTests.cpp
  updateTests.cpp
  ../application_api/testFixtures.cpp
  ../application_api/testFixtures.hpp
)

add_executable(system-tests ${system_test_sources} ${system_test_headers})
target_link_libraries(system-tests helics-static helics_boost_test_base)

set_target_properties(system-tests PROPERTIES FOLDER tests)

target_compile_definitions(
  system-tests PRIVATE "-DHELICS_BROKER_LOCATION=\"${HELICS_BROKER_LOC}\""
)
target_compile_definitions(
  system-tests PRIVATE "-DHELICS_INSTALL_LOCATION=\"${CMAKE_INSTALL_PREFIX}\""
)
target_compile_definitions(
  system-tests PRIVATE "-DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\""
)

add_test(
  NAME system-tests
  COMMAND system-tests --log_level=message --report_level=short
)
set_property(TEST system-tests PROPERTY LABELS System Coverage Daily)

# Tests for Continuous Integration builds
add_test(
  NAME system-ci-tests
  COMMAND
    system-tests
    --run_test=@ci
    --log_level=message
    --report_level=short
)
set_property(TEST system-ci-tests PROPERTY LABELS Continuous Valgrind)

copy_key_files_to_target_location(system-tests)
