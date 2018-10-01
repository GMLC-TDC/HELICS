/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "TcpCore.h"
#include "TcpComms.h"
#include "TcpCommsSS.h"
#include "../../common/argParser.h"
#include "../NetworkCore_impl.hpp"

namespace helics
{
template class NetworkCore<tcp::TcpComms, NetworkBrokerData::interface_type::tcp>;
namespace tcp
{

TcpCoreSS::TcpCoreSS () noexcept {}

TcpCoreSS::TcpCoreSS (const std::string &core_name) : NetworkCore (core_name) {}


using namespace std::string_literals;
static const ArgDescriptors extraArgs{
{ "connections"s, ArgDescriptor::arg_type_t::vector_string,"target link connections"s } };

void TcpCoreSS::initializeFromArgs (int argc, const char *const *argv)
{
    if (brokerState == created)
    {
        std::unique_lock<std::mutex> lock(dataMutex);
        if (brokerState == created)
        {
            variable_map vm;
            argumentParser(argc, argv, vm, extraArgs);
            if (vm.count("connections") > 0)
            {
                connections = vm["connections"].as<std::vector<std::string>>();
            }
        }
        lock.unlock();
        NetworkCore::initializeFromArgs(argc, argv);
    }
}

bool TcpCoreSS::brokerConnect ()
{
    std::unique_lock<std::mutex> lock (dataMutex);
	if (!connections.empty())
	{
		comms->addConnections(connections);
	}
    lock.unlock();
    return NetworkCore::brokerConnect();
}

}  // namespace tcp
}  // namespace helics
