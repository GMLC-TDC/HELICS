# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2020, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(core_test_headers)

set(
    core_test_sources
    core-tests.cpp
    InfoClass-tests.cpp
    FederateState-tests.cpp
    ActionMessage-tests.cpp
    BrokerClassTests.cpp
    CoreFactory-tests.cpp
    data-block-tests.cpp
    ForwardingTimeCoordinatorTests.cpp
    TimeCoordinatorTests.cpp
    networkInfoTests.cpp
	InprocCore-Tests.cpp
)

if(ENABLE_TEST_CORE)
    list(APPEND core_test_sources TestCore-tests.cpp)
endif()

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
    list(APPEND core_test_sources IPCcore_tests.cpp)
endif()

if(ENABLE_MPI_CORE)
    list(APPEND core_test_sources MpiCore-tests.cpp)
endif()

if (NOT HELICS_DISABLE_ASIO)
    list(APPEND core_test_sources MessageTimerTests.cpp)
endif()

add_executable(core-tests ${core_test_sources} ${core_test_headers})
target_link_libraries(core-tests helics_core helics_test_base)

target_include_directories(core-tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_compile_definitions(core-tests PRIVATE BOOST_DATE_TIME_NO_LIB)
set_target_properties(core-tests PROPERTIES FOLDER tests)

add_test(NAME core-tests COMMAND core-tests)
set_property(TEST core-tests PROPERTY LABELS Core Coverage Daily)

# Tests for Continuous Integration builds
add_test(NAME core-ci-tests COMMAND core-tests --gtest_filter=-*ci_skip*)
set_property(TEST core-ci-tests PROPERTY LABELS CoreCI Continuous)
# set_property(TEST core-ci-tests PROPERTY LABELS DebugTest)
copy_key_files_to_target_location(core-tests)
