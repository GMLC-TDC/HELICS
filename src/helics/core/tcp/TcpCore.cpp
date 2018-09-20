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
namespace tcp
{
template class NetworkCore<TcpComms, NetworkBrokerData::interface_type::tcp>;
/*
TcpCore::TcpCore () noexcept {}

TcpCore::TcpCore (const std::string &core_name) : CommsBroker (core_name) {}

void TcpCore::initializeFromArgs (int argc, const char *const *argv)
{
    if (brokerState == created)
    {
        netInfo.initializeFromArgs (argc, argv, "localhost");

        CommonCore::initializeFromArgs (argc, argv);
    }
}

bool TcpCore::brokerConnect ()
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (netInfo.brokerAddress.empty ())  // cores require a broker
    {
        netInfo.brokerAddress = "localhost";
    }
    comms->loadNetworkInfo (netInfo);
    comms->setName (getIdentifier ());
    comms->setTimeout (networkTimeout);
    auto res = comms->connect ();
    if (res)
    {
        if (netInfo.portNumber < 0)
        {
            netInfo.portNumber = comms->getPort ();
        }
    }
    return res;
}

std::string TcpCore::generateLocalAddressString () const
{
    
    if (comms->isConnected())
    {
        return comms->getAddress ();
    }
    std::lock_guard<std::mutex> lock (dataMutex);
    return makePortAddress (netInfo.localInterface, netInfo.portNumber);
}
*/

TcpCoreSS::TcpCoreSS () noexcept {}

TcpCoreSS::TcpCoreSS (const std::string &core_name) : NetworkCore (core_name) {}


using namespace std::string_literals;
static const ArgDescriptors extraArgs{ { "server"s,ArgDescriptor::arg_type_t::flag_type, "specify that the Core should be a server"s },
{ "connector"s,ArgDescriptor::arg_type_t::flag_type, "specify that the Core should be a connector"s },
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
            if (vm.count("server") > 0)
            {
                serverMode = true;
            }
        }
        lock.unlock();
        NetworkCore::initializeFromArgs(argc, argv);
    }
}

bool TcpCoreSS::brokerConnect ()
{
    std::unique_lock<std::mutex> lock (dataMutex);
    comms->setServerMode(serverMode);
    lock.unlock();
    return NetworkCore::brokerConnect();
}

}  // namespace tcp
}  // namespace helics
