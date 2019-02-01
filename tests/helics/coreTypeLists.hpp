/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "helics/helics-config.h"

#ifdef HELICS_HAVE_ZEROMQ
#define ZMQTEST "zmq",
#define ZMQTEST2 "zmq_2",
#define ZMQTEST3 "zmq_3",
#define ZMQTEST4 "zmq_4",

#define ZMQSSTEST "zmq_ss",
#define ZMQSSTEST2 "zmq_ss_2",
#else
#define ZMQTEST
#define ZMQTEST2
#define ZMQTEST3
#define ZMQTEST4

#define ZMQSSTEST
#define ZMQSSTEST2
#endif

#ifndef DISABLE_TCP_CORE
#define TCPTEST "tcp",
#define TCPTEST2 "tcp_2",
#define TCPTEST3 "tcp_3",
#define TCPTEST4 "tcp_4",

#define TCPSSTEST "tcpss",
#define TCPSSTEST2 "tcpss_2",
#else
#define TCPTEST
#define TCPTEST2
#define TCPTEST3
#define TCPTEST4

#define TCPSSTEST
#define TCPSSTEST2
#endif

#ifndef DISABLE_IPC_CORE
#define IPCTEST "ipc",
#define IPCTEST2 "ipc_2",

#else
#define IPCTEST
#define IPCTEST2
#endif

#ifndef DISABLE_UDP_CORE
#define UDPTEST "udp",
#define UDPTEST2 "udp_2",
#define UDPTEST3 "udp_3",
#define UDPTEST4 "udp_4",

#else
#define UDPTEST
#define UDPTEST2
#define UDPTEST3
#define UDPTEST4

#endif

constexpr const char *ztypes[] = {ZMQTEST ZMQSSTEST ZMQTEST2 ZMQTEST3 ZMQSSTEST2 ZMQTEST4};

constexpr const char *core_types[] = {"test", ZMQTEST3 IPCTEST2 TCPTEST "test_2",
                                      ZMQTEST UDPTEST TCPSSTEST ZMQSSTEST "test_3"};

constexpr const char *core_types_2[] = {IPCTEST2 TCPTEST2 ZMQSSTEST2 "test_2", TCPSSTEST2 ZMQTEST2 UDPTEST2};

constexpr const char *core_types_simple[] = {"test", TCPSSTEST ZMQSSTEST IPCTEST TCPTEST ZMQTEST UDPTEST};
constexpr const char *core_types_single[] = {"test", TCPSSTEST IPCTEST TCPTEST ZMQTEST UDPTEST "test_3",
                                             ZMQTEST3 TCPTEST3 ZMQSSTEST UDPTEST3};
constexpr const char *core_types_all[] = {"test", TCPSSTEST ZMQSSTEST IPCTEST2 TCPTEST "test_2",
                                          ZMQTEST UDPTEST TCPSSTEST2 "test_3",
                                          ZMQTEST3 IPCTEST ZMQTEST2 UDPTEST2 ZMQSSTEST2 TCPTEST2 UDPTEST3 TCPTEST3
                                          "test_4",
                                          ZMQTEST4 TCPTEST4 UDPTEST4};

constexpr const char *core_types_extended[] = {IPCTEST ZMQTEST2 UDPTEST2 TCPTEST2 UDPTEST3 ZMQSSTEST2 TCPTEST3
                                               "test_4",
                                               ZMQTEST4 TCPTEST4 UDPTEST4};

constexpr const char *defaultNamePrefix = "fed";
