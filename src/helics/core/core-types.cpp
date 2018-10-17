/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "core-types.hpp"
#include "core-exceptions.hpp"
#include "helics/helics-config.h"
#include <algorithm>
#include <cctype>
#include <map>

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
    case core_type::INTERPROCESS:
    case core_type::IPC:
        return "ipc_";
    case core_type::TCP:
        return "tcp_";
    case core_type::UDP:
        return "udp_";
    default:
        return std::string ();
    }
}

static const std::map<std::string, core_type> coreTypes{{"default", core_type::DEFAULT},
                                                        {"def", core_type::DEFAULT},
                                                        {"mpi", core_type::MPI},
                                                        {"message_passing_interface", core_type::MPI},
                                                        {"MPI", core_type::MPI},
                                                        {"ZMQ", core_type::ZMQ},
                                                        {"0mq", core_type::ZMQ},
                                                        {"zmq", core_type::ZMQ},
                                                        {"zeromq", core_type::ZMQ},
                                                        {"interprocess", core_type::INTERPROCESS},
                                                        {"ZeroMQ", core_type::ZMQ},
                                                        {"ipc", core_type::INTERPROCESS},
                                                        {"interproc", core_type::INTERPROCESS},
                                                        {"IPC", core_type::INTERPROCESS},
                                                        {"tcp", core_type::TCP},
                                                        {"tcpip", core_type::TCP},
                                                        {"TCP", core_type::TCP},
                                                        {"TCPIP", core_type::TCP},
                                                        {"udp", core_type::UDP},
                                                        {"test", core_type::TEST},
                                                        {"UDP", core_type::UDP},
                                                        {"local", core_type::TEST},
                                                        {"inprocess", core_type::TEST},
                                                        {"test1", core_type::TEST}};

core_type coreTypeFromString (std::string type)
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
    if (type.compare (0, 3, "tcp") == 0)
    {
        return core_type::TCP;
    }
    if (type.compare (0, 3, "udp") == 0)
    {
        return core_type::UDP;
    }
    if (type.compare (0, 3, "mpi") == 0)
    {
        return core_type::MPI;
    }
    return core_type::UNRECOGNIZED;
}

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
    case core_type::MPI:
#if HELICS_HAVE_MPI
        available = true;
#endif
        break;
    case core_type::TEST:
        available = true;
        break;
    case core_type::INTERPROCESS:
    case core_type::IPC:
        available = true;
        break;
    case core_type::UDP:
        available = true;
        break;
    case core_type::TCP:
#ifdef DISABLE_TCP_CORE
        available = false;
#else
        available = true;
#endif
        break;
    case core_type::DEFAULT:  // default should always be available
        available = true;
        break;
    default:
        break;
    }

    return available;
}

}  // namespace helics
