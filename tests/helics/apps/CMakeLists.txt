#
# Copyright © 2017-2019, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved. 
# 
# SPDX-License-Identifier: BSD-3-Clause
#

set(
  helics_apps_test_sources
  helics_apps_tests.cpp
  multi_player_tests.cpp
  multi_tests.cpp
  PlayerTests.cpp
  RecorderTests.cpp
  exeTestHelper.cpp
  combo_tests.cpp
  EchoTests.cpp
  TracerTests.cpp
  SourceTests.cpp
  CloneTests.cpp
  BrokerServerTests.cpp
  exeTestHelper.h
)

add_executable(helics_apps-tests ${helics_apps_test_sources})

set_target_properties(helics_apps-tests PROPERTIES FOLDER tests)

target_link_libraries(helics_apps-tests helics_apps helics_boost_test_base)

target_include_directories(helics_apps-tests PRIVATE ${PROJECT_SOURCE_DIR}/src)

target_compile_definitions(
  helics_apps-tests PRIVATE -DTEST_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/test_files/\"
)
target_compile_definitions(
  helics_apps-tests PRIVATE -DHELICS_BUILD_LOC=\"${HELICS_BINARY_DIR}/src/helics/\"
)
target_compile_definitions(
  helics_apps-tests PRIVATE
  -DHELICS_INSTALL_LOC=\"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}\"
)

if(DISABLE_SYSTEM_CALL_TESTS)
  target_compile_definitions(helics_apps-tests PRIVATE -DDISABLE_SYSTEM_CALL_TESTS=1)
endif()

add_test(NAME helics_apps-tests COMMAND helics_apps-tests --report_level=short)
set_property(TEST helics_apps-tests PROPERTY LABELS HelicsApps Coverage Daily)

# Tests for Continuous Integration builds
add_test(
  NAME helics_apps-ci-tests
  COMMAND helics_apps-tests --run_test=@ci --report_level=short --log_level=test_suite
)
set_property(TEST helics_apps-ci-tests PROPERTY LABELS Continuous)

copy_key_files_to_target_location(helics_apps-tests)
