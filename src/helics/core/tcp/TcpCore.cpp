/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "TcpCore.h"
#include "../../common/argParser.h"
#include "../NetworkCore_impl.hpp"
#include "TcpComms.h"
#include "TcpCommsSS.h"

namespace helics
{
template class NetworkCore<tcp::TcpComms, interface_type::tcp>;
namespace tcp
{
TcpCoreSS::TcpCoreSS () noexcept {}

TcpCoreSS::TcpCoreSS (const std::string &core_name) : NetworkCore (core_name) {}

using namespace std::string_literals;
static const ArgDescriptors extraArgs{{"connections"s, ArgDescriptor::arg_type_t::vector_string,
                                       "target link connections"s},
                                      {"no_outgoing_connections"s, ArgDescriptor::arg_type_t::flag_type,
                                       "disable outgoing connections"s}};

void TcpCoreSS::initializeFromArgs (int argc, const char *const *argv)
{
    if (brokerState == created)
    {
        std::unique_lock<std::mutex> lock (dataMutex);
        if (brokerState == created)
        {
            variable_map vm;
            argumentParser (argc, argv, vm, extraArgs);
            if (vm.count ("connections") > 0)
            {
                connections = vm["connections"].as<std::vector<std::string>> ();
            }
            if (vm.count ("no_outgoing_connections") > 0)
            {
                no_outgoing_connections = true;
            }
        }
        lock.unlock ();
        NetworkCore::initializeFromArgs (argc, argv);
    }
}

bool TcpCoreSS::brokerConnect ()
{
    std::unique_lock<std::mutex> lock (dataMutex);
    if (!connections.empty ())
    {
        comms->addConnections (connections);
    }
    if (no_outgoing_connections)
    {
        comms->setFlag ("allow_outgoing", false);
    }
    lock.unlock ();
    return NetworkCore::brokerConnect ();
}

}  // namespace tcp
}  // namespace helics
