/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "core-types.hpp"
#include "core-exceptions.hpp"
#include "helics/helics-config.h"
#include <algorithm>
#include <cctype>
#include <map>
#include <unordered_map>

namespace helics
{
std::string to_string (core_type type)
{
    switch (type)
    {
    case core_type::MPI:
        return "mpi_";
    case core_type::TEST:
        return "test_";
    case core_type::ZMQ:
        return "zmq_";
    case core_type::ZMQ_SS:
        return "zmqss_";
    case core_type::INTERPROCESS:
    case core_type::IPC:
        return "ipc_";
    case core_type::TCP:
        return "tcp_";
    case core_type::TCP_SS:
        return "tcpss_";
    case core_type::HTTP:
        return "http_";
    case core_type::UDP:
        return "udp_";
    case core_type::NNG:
        return "nng_";
    default:
        return std::string ();
    }
}

static const std::unordered_map<std::string, core_type> coreTypes{{"default", core_type::DEFAULT},
                                                                  {"def", core_type::DEFAULT},
                                                                  {"mpi", core_type::MPI},
                                                                  {"message_passing_interface", core_type::MPI},
                                                                  {"MPI", core_type::MPI},
                                                                  {"ZMQ", core_type::ZMQ},
                                                                  {"0mq", core_type::ZMQ},
                                                                  {"zmq", core_type::ZMQ},
                                                                  {"zeromq", core_type::ZMQ},
                                                                  {"zeromq_ss", core_type::ZMQ_SS},
                                                                  {"zmq_ss", core_type::ZMQ_SS},
																  {"ZMQ_SS", core_type::ZMQ_SS},
                                                                  {"zeromq2", core_type::ZMQ_SS},
                                                                  {"zmq2", core_type::ZMQ_SS},
                                                                  {"ZMQ2", core_type::ZMQ_SS},
                                                                  {"interprocess", core_type::INTERPROCESS},
                                                                  {"ZeroMQ", core_type::ZMQ},
																  {"ZeroMQ2", core_type::ZMQ_SS},
                                                                  {"ipc", core_type::INTERPROCESS},
                                                                  {"interproc", core_type::INTERPROCESS},
                                                                  {"IPC", core_type::INTERPROCESS},
                                                                  {"tcp", core_type::TCP},
                                                                  {"tcpip", core_type::TCP},
                                                                  {"TCP", core_type::TCP},
                                                                  {"TCPIP", core_type::TCP},
                                                                  {"tcpss", core_type::TCP_SS},
                                                                  {"tcpipss", core_type::TCP_SS},
                                                                  {"TCPSS", core_type::TCP_SS},
                                                                  {"TCPIPSS", core_type::TCP_SS},
                                                                  {"tcp_ss", core_type::TCP_SS},
                                                                  {"tcpip_ss", core_type::TCP_SS},
                                                                  {"TCP_SS", core_type::TCP_SS},
                                                                  {"TCPIP_SS", core_type::TCP_SS},
                                                                  {"single_socket", core_type::TCP_SS},
                                                                  {"single socket", core_type::TCP_SS},
                                                                  {"ss", core_type::TCP_SS},
                                                                  {"udp", core_type::UDP},
                                                                  {"test", core_type::TEST},
                                                                  {"UDP", core_type::UDP},
                                                                  {"local", core_type::TEST},
                                                                  {"inprocess", core_type::TEST},
                                                                  {"http", core_type::HTTP},
                                                                  {"HTTP", core_type::HTTP},
                                                                  {"web", core_type::HTTP},
                                                                  {"test1", core_type::TEST}};

core_type coreTypeFromString (std::string type) noexcept
{
    if (type.empty ())
    {
        return core_type::DEFAULT;
    }
    auto fnd = coreTypes.find (type);
    if (fnd != coreTypes.end ())
    {
        return fnd->second;
    }
    std::transform (type.cbegin (), type.cend (), type.begin (), ::tolower);
    fnd = coreTypes.find (type);
    if (fnd != coreTypes.end ())
    {
        return fnd->second;
    }
    if ((type.front() == '=') || (type.front() == '-'))
    {
        return coreTypeFromString(type.substr(1));
    }
    if (type.compare (0, 4, "zmq2") == 0)
    {
        return core_type::ZMQ_SS;
    }
    if (type.compare (0, 3, "zmq") == 0)
    {
        return core_type::ZMQ;
    }
    if (type.compare (0, 3, "ipc") == 0)
    {
        return core_type::INTERPROCESS;
    }
    if (type.compare (0, 4, "test") == 0)
    {
        return core_type::TEST;
    }
    if (type.compare (0, 5, "tcpss") == 0)
    {
        return core_type::TCP_SS;
    }
    if (type.compare (0, 3, "tcp") == 0)
    {
        return core_type::TCP;
    }
    if (type.compare (0, 3, "udp") == 0)
    {
        return core_type::UDP;
    }
    if (type.compare (0, 4, "http") == 0)
    {
        return core_type::HTTP;
    }
    if (type.compare (0, 3, "mpi") == 0)
    {
        return core_type::MPI;
    }
    return core_type::UNRECOGNIZED;
}

#ifdef DISABLE_TCP_CORE
#define TCP_AVAILABILITY false
#else
#define TCP_AVAILABILITY true
#endif

#ifdef DISABLE_UDP_CORE
#define UDP_AVAILABILITY false
#else
#define UDP_AVAILABILITY true
#endif

#ifdef DISABLE_IPC_CORE
#define IPC_AVAILABILITY false
#else
#define IPC_AVAILABILITY true
#endif

#ifdef DISABLE_TEST_CORE
#define TEST_AVAILABILITY false
#else
#define TEST_AVAILABILITY true
#endif

bool isCoreTypeAvailable (core_type type) noexcept
{
    bool available = false;

    switch (type)
    {
    case core_type::ZMQ:
#if HELICS_HAVE_ZEROMQ
        available = true;
#endif
        break;
    case core_type::ZMQ_SS:
#if HELICS_HAVE_ZEROMQ
        available = false;
#endif
        break;
    case core_type::MPI:
        available = (HELICS_HAVE_MPI != 0);
        break;
    case core_type::TEST:
        available = TEST_AVAILABILITY;
        break;
    case core_type::INTERPROCESS:
    case core_type::IPC:
        available = IPC_AVAILABILITY;
        break;
    case core_type::UDP:
        available = UDP_AVAILABILITY;
        break;
    case core_type::TCP:
        available = TCP_AVAILABILITY;
        break;
    case core_type::TCP_SS:
        available = TCP_AVAILABILITY;
        break;
    case core_type::DEFAULT:  // default should always be available
        available = true;
        break;
    case core_type::HTTP:
        available = false;
        break;
    default:
        break;
    }

    return available;
}

}  // namespace helics
