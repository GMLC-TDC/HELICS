/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "TcpBroker.h"
#include "TcpComms.h"
#include "TcpCommsSS.h"
#include "../../common/argParser.h"
#include "../NetworkBroker_impl.hpp"

namespace helics
{
template class NetworkBroker<tcp::TcpComms, NetworkBrokerData::interface_type::tcp, 6>;
namespace tcp
{

using namespace std::string_literals;
static const ArgDescriptors extraArgs{{"server"s, ArgDescriptor::arg_type_t::flag_type,
                                       "specify that the Broker should be a server"s},
                                      {"connections"s, ArgDescriptor::arg_type_t::vector_string,
                                       "target link connections"s}};

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
            if (vm.count ("server") > 0)
            {
                serverMode = true;
            }
        }
        lock.unlock ();
        NetworkBroker::initializeFromArgs (argc, argv);
    }
}

bool TcpBrokerSS::brokerConnect ()
{
    std::unique_lock<std::mutex> lock (dataMutex);
    comms->setServerMode (serverMode);
    lock.unlock ();
    return NetworkBroker::brokerConnect ();
}

}  // namespace tcp
}  // namespace helics
