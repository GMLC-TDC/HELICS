/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "coreTypeOperations.hpp"

#include "core-exceptions.hpp"
#include "core-types.hpp"
#include "helics/helics-config.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <unordered_map>

namespace helics {
namespace core {
    std::string to_string(core_type type)
    {
        switch (type) {
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
            case core_type::INPROC:
                return "inproc_";
            case core_type::WEBSOCKET:
                return "websocket_";
            case core_type::NULLCORE:
                return "null_";
            default:
                return std::string();
        }
    }

    static const std::unordered_map<std::string, core_type> coreTypes{
        {"default", core_type::DEFAULT},
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
        {"inprocess", core_type::INPROC},
        {"websocket", core_type::WEBSOCKET},
        {"web", core_type::WEBSOCKET},
        {"inproc", core_type::INPROC},
        {"nng", core_type::NNG},
        {"null", core_type::NULLCORE},
        {"nullcore", core_type::NULLCORE},
        {"none", core_type::NULLCORE},
        {"http", core_type::HTTP},
        {"HTTP", core_type::HTTP},
        {"web", core_type::HTTP},
        {"test1", core_type::TEST},
        {"multi", core_type::MULTI}};

    core_type coreTypeFromString(std::string type) noexcept
    {
        if (type.empty()) {
            return core_type::DEFAULT;
        }
        auto fnd = coreTypes.find(type);
        if (fnd != coreTypes.end()) {
            return fnd->second;
        }
        std::transform(type.cbegin(), type.cend(), type.begin(), ::tolower);
        fnd = coreTypes.find(type);
        if (fnd != coreTypes.end()) {
            return fnd->second;
        }
        if ((type.front() == '=') || (type.front() == '-')) {
            return coreTypeFromString(type.substr(1));
        }
        if (type.compare(0, 4, "zmq2") == 0) {
            return core_type::ZMQ_SS;
        }
        if (type.compare(0, 3, "zmq") == 0) {
            return core_type::ZMQ;
        }
        if (type.compare(0, 3, "ipc") == 0) {
            return core_type::INTERPROCESS;
        }
        if (type.compare(0, 4, "test") == 0) {
            return core_type::TEST;
        }
        if (type.compare(0, 5, "tcpss") == 0) {
            return core_type::TCP_SS;
        }
        if (type.compare(0, 3, "tcp") == 0) {
            return core_type::TCP;
        }
        if (type.compare(0, 3, "udp") == 0) {
            return core_type::UDP;
        }
        if (type.compare(0, 4, "http") == 0) {
            return core_type::HTTP;
        }
        if (type.compare(0, 3, "mpi") == 0) {
            return core_type::MPI;
        }
        if (type.compare(0, 6, "inproc") == 0) {
            return core_type::INPROC;
        }
        if (type.compare(0, 3, "web") == 0) {
            return core_type::WEBSOCKET;
        }
        if (type.compare(0, 4, "null") == 0) {
            return core_type::NULLCORE;
        }
        return core_type::UNRECOGNIZED;
    }

#ifndef ENABLE_ZMQ_CORE
    static bool constexpr zmq_availability{false};
#else
    static bool constexpr zmq_availability{true};
#endif

#ifndef ENABLE_MPI_CORE
    static bool constexpr mpi_availability{false};
#else
    static bool constexpr mpi_availability{true};
#endif

#ifndef ENABLE_TCP_CORE
    static bool constexpr tcp_availability{false};
#else
    static bool constexpr tcp_availability{true};
#endif

#ifndef ENABLE_UDP_CORE
    static bool constexpr udp_availability{false};
#else
    static bool constexpr udp_availability{true};
#endif

#ifndef ENABLE_IPC_CORE
    static bool constexpr ipc_availability{false};
#else
    static bool constexpr ipc_availability{true};
#endif

#ifndef ENABLE_TEST_CORE
    static bool constexpr test_availability{false};
#else
    static bool constexpr test_availability{true};
#endif

#ifndef ENABLE_INPROC_CORE
    static bool constexpr inproc_availability{false};
#else
    static bool constexpr inproc_availability{true};
#endif

    bool isCoreTypeAvailable(core_type type) noexcept
    {
        bool available = false;

        switch (type) {
            case core_type::ZMQ:
            case core_type::ZMQ_SS:
                available = zmq_availability;
                break;
            case core_type::MPI:
                available = mpi_availability;
                break;
            case core_type::TEST:
                available = test_availability;
                break;
            case core_type::INTERPROCESS:
            case core_type::IPC:
                available = ipc_availability;
                break;
            case core_type::UDP:
                available = udp_availability;
                break;
            case core_type::TCP:
            case core_type::TCP_SS:
                available = tcp_availability;
                break;
            case core_type::DEFAULT:  // default should always be available
                available = true;
                break;
            case core_type::INPROC:
                available = inproc_availability;
                break;
            case core_type::HTTP:
            case core_type::WEBSOCKET:
            case core_type::NULLCORE:
                available = false;
                break;
            default:
                break;
        }

        return available;
    }

    static const std::set<std::string> global_match_strings{"any",
                                                            "all",
                                                            "data",
                                                            "string",
                                                            "block"};

    bool matchingTypes(const std::string& type1, const std::string& type2)
    {
        if (type1 == type2) {
            return true;
        }
        if ((type1.empty()) || (type2.empty())) {
            return true;
        }
        if ((type1.compare(0, 3, "def") == 0) || (type2.compare(0, 3, "def") == 0)) {
            return true;
        }
        auto res = global_match_strings.find(type1);
        if (res != global_match_strings.end()) {
            return true;
        }
        res = global_match_strings.find(type2);
        return (res != global_match_strings.end());
    }

}  // namespace core
}  // namespace helics
