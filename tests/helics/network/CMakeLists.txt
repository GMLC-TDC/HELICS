# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
# Copyright (c) 2017-2021, Battelle Memorial Institute; Lawrence Livermore
# National Security, LLC; Alliance for Sustainable Energy, LLC.
# See the top-level NOTICE for additional details.
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

set(betwork_test_headers)

set(network_test_sources network-tests.cpp networkInfoTests.cpp TestCore-tests.cpp)

if(ENABLE_ZMQ_CORE)
    list(APPEND network_test_sources ZeromqCore-tests.cpp ZeromqSSCore-tests.cpp)
endif()

if(ENABLE_TCP_CORE)
    list(APPEND network_test_sources TcpCore-tests.cpp TcpSSCore-tests.cpp)
endif()

if(ENABLE_UDP_CORE)
    list(APPEND network_test_sources UdpCore-tests.cpp)
endif()

if(ENABLE_IPC_CORE)
    list(APPEND network_test_sources IPCcore_tests.cpp)
endif()

if(ENABLE_MPI_CORE)
    list(APPEND network_test_sources MpiCore-tests.cpp)
endif()

add_executable(network-tests ${network_test_sources} ${network_test_headers})
target_link_libraries(network-tests helics_network helics_test_base helics::zmq)

target_include_directories(network-tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_compile_definitions(network-tests PRIVATE BOOST_DATE_TIME_NO_LIB)
set_target_properties(network-tests PROPERTIES FOLDER tests)

add_test(NAME network-tests COMMAND network-tests)
set_property(TEST network-tests PROPERTY LABELS Network Coverage Daily)

# Tests for Continuous Integration builds
add_test(NAME network-ci-tests COMMAND network-tests --gtest_filter=-*ci_skip*)
set_property(TEST network-ci-tests PROPERTY LABELS NetworkCI Continuous)
# set_property(TEST network-ci-tests PROPERTY LABELS DebugTest)
