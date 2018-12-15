/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "TcpBroker.h"
#include "../../common/argParser.h"
#include "../NetworkBroker_impl.hpp"
#include "TcpComms.h"
#include "TcpCommsSS.h"
#include <iostream>

namespace helics
{
template class NetworkBroker<tcp::TcpComms, interface_type::tcp, static_cast<int> (core_type::TCP)>;
namespace tcp
{
using namespace std::string_literals;
static const ArgDescriptors extraArgs{{"connections"s, ArgDescriptor::arg_type_t::vector_string,
                                       "target link connections"s},
                                      {"no_outgoing_connections"s, ArgDescriptor::arg_type_t::flag_type,
                                       "disable outgoing connections"s}};

TcpBrokerSS::TcpBrokerSS (bool rootBroker) noexcept : NetworkBroker (rootBroker) {}

TcpBrokerSS::TcpBrokerSS (const std::string &broker_name) : NetworkBroker (broker_name) {}

void TcpBrokerSS::displayHelp (bool local_only)
{
    std::cout << " Help for TCP SS Broker only arguments: \n";
    variable_map vm;
    const char *const argV[] = {"", "--help"};
    argumentParser (2, argV, vm, extraArgs);
    NetworkBroker::displayHelp (local_only);
}

void TcpBrokerSS::initializeFromArgs (int argc, const char *const *argv)
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
            if (vm.count("no_outgoing_connections") > 0)
            {
                no_outgoing_connections = true;
            }
        }
        lock.unlock ();
        NetworkBroker::initializeFromArgs (argc, argv);
    }
}

bool TcpBrokerSS::brokerConnect ()
{
    std::unique_lock<std::mutex> lock (dataMutex);
    if (!connections.empty ())
    {
        comms->addConnections (connections);
    }
    if (no_outgoing_connections)
    {
        comms->setFlag("allow_outgoing",false);
    }
    lock.unlock ();
    return NetworkBroker::brokerConnect ();
}

}  // namespace tcp
}  // namespace helics
