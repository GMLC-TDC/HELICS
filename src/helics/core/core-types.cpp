/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "core-exceptions.h"
#include "core-types.h"
#include "helics/helics-config.h"
#include <cctype>
#include <map>
#include <algorithm>

namespace helics
{
    std::string helicsTypeString(core_type type)
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
            return "";
        }
    }

    static const std::map<std::string, core_type> coreTypes
    {
        {"default",core_type::DEFAULT },
        {"def",core_type::DEFAULT},
        { "mpi",core_type::MPI },
        {"message_passing_interface", core_type::MPI },
        { "0mq",core_type::ZMQ },
        { "zmq",core_type::ZMQ },
        { "zeromq",core_type::ZMQ },
        {"interprocess", core_type::INTERPROCESS},
        { "ipc", core_type::INTERPROCESS},
        { "interproc", core_type::INTERPROCESS},
        { "tcp", core_type::TCP },
        { "tcpip", core_type::TCP },
        { "udp", core_type::UDP },
        { "test", core_type::TEST },
        { "local", core_type::TEST },
        { "inprocess", core_type::TEST },
        { "test1", core_type::TEST }
    };

    core_type coreTypeFromString(std::string type)
    {
        std::transform(type.cbegin(), type.cend(), type.begin(), ::tolower);
        if (type.empty())
        {
            return core_type::DEFAULT;
        }
        auto fnd = coreTypes.find(type);
        if (fnd != coreTypes.end())
        {
            return fnd->second;
        }
        if (type.compare(0, 3, "zmq") == 0)
        {
            return core_type::ZMQ;
        }
        if  (type.compare(0, 3, "ipc") == 0)
        {
            return core_type::INTERPROCESS;
        }
        if (type.compare(0, 4, "test") == 0)
        {
            return core_type::TEST;
        }
        if (type.compare(0, 3, "tcp") == 0)
        {
            return core_type::TCP;
        }
        if (type.compare(0, 3, "udp") == 0)
        {
            return core_type::UDP;
        }
        if (type.compare(0, 3, "mpi") == 0)
        {
            return core_type::MPI;
        }
        throw (std::invalid_argument("unrecognized core type"));
    }


    bool isAvailable(core_type type)
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
        default:
            break;
        }

        return available;
    }

} //namespace helics