/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "helics/helics-config.h"

#ifdef ENABLE_ZMQ_CORE
#    define ZMQTEST "zmq",
#    define ZMQTEST2 "zmq_2",
#    define ZMQTEST3 "zmq_3",
#    define ZMQTEST4 "zmq_4",

#    define ZMQSSTEST "zmq_ss",
#    define ZMQSSTEST2 "zmq_ss_2",
#else
#    define ZMQTEST
#    define ZMQTEST2
#    define ZMQTEST3
#    define ZMQTEST4

#    define ZMQSSTEST
#    define ZMQSSTEST2
#endif

#ifdef ENABLE_TCP_CORE
#    define TCPTEST "tcp",
#    define TCPTEST2 "tcp_2",
#    define TCPTEST3 "tcp_3",
#    define TCPTEST4 "tcp_4",

#    define TCPSSTEST "tcpss",
#    define TCPSSTEST2 "tcpss_2",
#else
#    define TCPTEST
#    define TCPTEST2
#    define TCPTEST3
#    define TCPTEST4

#    define TCPSSTEST
#    define TCPSSTEST2
#endif

#ifdef ENABLE_IPC_CORE
#    define IPCTEST "ipc",
#    define IPCTEST2 "ipc_2",

#else
#    define IPCTEST
#    define IPCTEST2
#endif

#ifdef ENABLE_UDP_CORE
#    define UDPTEST "udp",
#    define UDPTEST2 "udp_2",
#    define UDPTEST3 "udp_3",
#    define UDPTEST4 "udp_4",

#else
#    define UDPTEST
#    define UDPTEST2
#    define UDPTEST3
#    define UDPTEST4

#endif

#ifdef ENABLE_INPROC_CORE
#    define INPROCTEST "inproc",
#    define INPROCTEST2 "inproc_2",
#    define INPROCTEST3 "inproc_3",
#    define INPROCTEST4 "inproc_4",
#else  // if the INPROC core is turned off then just use the test core since it is required to be
       // available for tests
#    define INPROCTEST "test"
#    define INPROCTEST2 "test_2"
#    define INPROCTEST3 "test_3"
#    define INPROCTEST4 "test_4"
#endif

#ifdef ENABLE_ZMQ_CORE
constexpr const char* ztypes[] = {ZMQTEST ZMQSSTEST ZMQTEST2 ZMQTEST3 ZMQSSTEST2 ZMQTEST4};
#endif

constexpr const char* core_types[] =
    {"test", ZMQTEST3 IPCTEST2 TCPTEST INPROCTEST2 ZMQTEST UDPTEST TCPSSTEST ZMQSSTEST ZMQTEST2};

constexpr const char* core_types_2[] = {IPCTEST2 TCPTEST2 ZMQSSTEST2 "test_2",
                                        TCPSSTEST2 ZMQTEST2 UDPTEST2};

constexpr const char* core_types_simple[] = {
    INPROCTEST TCPSSTEST ZMQSSTEST IPCTEST TCPTEST ZMQTEST UDPTEST};
constexpr const char* core_types_single[] = {INPROCTEST TCPSSTEST IPCTEST TCPTEST ZMQTEST UDPTEST
                                             "test_3",
                                             ZMQTEST3 TCPTEST3 ZMQSSTEST UDPTEST3};
constexpr const char* core_types_all[] = {
    INPROCTEST TCPSSTEST ZMQSSTEST IPCTEST2 TCPTEST INPROCTEST2 ZMQTEST UDPTEST TCPSSTEST2 "test_3",
    ZMQTEST3 IPCTEST ZMQTEST2 UDPTEST2 ZMQSSTEST2 TCPTEST2 UDPTEST3 TCPTEST3 INPROCTEST4 ZMQTEST4
        TCPTEST4 UDPTEST4};

constexpr const char* core_types_extended[] = {
    IPCTEST ZMQTEST2 UDPTEST2 TCPTEST2 UDPTEST3 ZMQSSTEST2 TCPTEST3 INPROCTEST4 ZMQTEST4 TCPTEST4
        UDPTEST4};

constexpr const char* defaultNamePrefix = "fed";
