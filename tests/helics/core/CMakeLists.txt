#
# Copyright © 2017-2019, Battelle Memorial Institute; Lawrence Livermore National
# Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved. 
# 
# SPDX-License-Identifier: BSD-3-Clause
#

# -----------------------------------------------------------------------------
# Core tests using Boost
# -----------------------------------------------------------------------------

set(core_test_headers testFixtures.h)

set(
  core_test_sources
  core-tests.cpp
  InfoClass-tests.cpp
  FederateState-tests.cpp
  ActionMessage-tests.cpp
  BrokerClassTests.cpp
  CoreFactory-tests.cpp 
  MessageTimerTests.cpp
  TestCore-tests.cpp
  data-block-tests.cpp
  ForwardingTimeCoordinatorTests.cpp
  TimeCoordinatorTests.cpp
  networkInfoTests.cpp
)

if(ENABLE_ZMQ_CORE)
  list(APPEND core_test_sources ZeromqCore-tests.cpp ZeromqSSCore-tests.cpp)
endif()

if(ENABLE_TCP_CORE)
  list(APPEND core_test_sources TcpCore-tests.cpp TcpSSCore-tests.cpp)
endif()

if(ENABLE_UDP_CORE)
  list(APPEND core_test_sources UdpCore-tests.cpp)
endif()

if(ENABLE_IPC_CORE)
  list(APPEND core_test_sources  IPCcore_tests.cpp)
endif()

if(ENABLE_MPI_CORE)
  list(APPEND core_test_sources MpiCore-tests.cpp)
endif()

add_executable(core-tests ${core_test_sources} ${core_test_headers})
target_link_libraries(core-tests helics-static helics_google_test_base)

target_include_directories(core-tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_compile_definitions(
  core-tests PRIVATE BOOST_DATE_TIME_NO_LIB
)
set_target_properties(core-tests PROPERTIES FOLDER tests)

add_test(NAME core-tests COMMAND core-tests)
set_property(TEST core-tests PROPERTY LABELS Core Coverage Daily TSAN)

# Tests for Continuous Integration builds
add_test(
  NAME core-ci-tests
  COMMAND
    core-tests
    --gtest_filter=-*skip_ci*
)
set_property(TEST core-ci-tests PROPERTY LABELS Continuous)
#set_property(TEST core-ci-tests PROPERTY LABELS DebugTest)
foreach(keyfile IN LISTS KEY_LIBRARY_FILES)
  add_custom_command(
    TARGET
    core-tests
    POST_BUILD # Adds a post-build event to core tests
    COMMAND
      ${CMAKE_COMMAND}
      -E
      copy_if_different # which executes "cmake - E copy_if_different..."
      "${keyfile}" # <--this is in-file
      "$<TARGET_FILE_DIR:core-tests>/"
  ) # <--this is out- file path
endforeach(keyfile)
