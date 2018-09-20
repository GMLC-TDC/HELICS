/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "NetworkBroker.hpp"
#include "../core/core-types.hpp"
namespace helics
{

constexpr const char * tcodeStr(int tcode)
{
    constexpr const char* tstr[] = { "default","ZeroMQ","MPI","TEST","Interprocess","interprocess","TCP", "UDP","undef","nng","ZMQ_test","TCPSS","undef","undef","http","unknown" };
    return ((tcode >= 0) && (tcode < 15)) ? tstr[tcode] : tstr[15];
}

template <class COMMS, NetworkBrokerData::interface_type baseline,int tcode>
NetworkBroker<COMMS, baseline,tcode>::NetworkBroker(bool rootBroker) noexcept : CommsBroker<COMMS, CoreBroker>(rootBroker) {}

template <class COMMS, NetworkBrokerData::interface_type baseline, int tcode>
NetworkBroker<COMMS, baseline,tcode>::NetworkBroker(const std::string &broker_name) : CommsBroker<COMMS, CoreBroker>(broker_name) {}

template <class COMMS, NetworkBrokerData::interface_type baseline, int tcode>
void NetworkBroker<COMMS, baseline,tcode>::displayHelp(bool local_only)
{
    std::cout << " Help for "<<tcodeStr(tcode)<<" Broker: \n";
    NetworkBrokerData::displayHelp();
    if (!local_only)
    {
        CoreBroker::displayHelp();
    }
}

template <class COMMS, NetworkBrokerData::interface_type baseline, int tcode>
void NetworkBroker<COMMS, baseline,tcode>::initializeFromArgs(int argc, const char *const *argv)
{
    if (brokerState == created)
    {
        std::lock_guard<std::mutex> lock(dataMutex);
        if (brokerState == created)
        {
            netInfo.initializeFromArgs(argc, argv, "tcp://127.0.0.1");
            CoreBroker::initializeFromArgs(argc, argv);
        }
    }
}

template <class COMMS, NetworkBrokerData::interface_type baseline, int tcode>
bool NetworkBroker<COMMS, baseline,tcode>::brokerConnect()
{
    std::lock_guard<std::mutex> lock(dataMutex);
    if (netInfo.brokerAddress.empty())
    {
        setAsRoot();
    }
    // zmqContextManager::startContext();
    comms->loadNetworkInfo(netInfo);
    comms->setName(getIdentifier());
    comms->setTimeout(networkTimeout);
    // comms->setMessageSize(maxMessageSize, maxMessageCount);
    auto res = comms->connect();
    if (res)
    {
        if (netInfo.portNumber < 0)
        {
            netInfo.portNumber = comms->getPort();
        }
    }
    return res;
}

template <class COMMS, NetworkBrokerData::interface_type baseline, int tcode>
std::string NetworkBroker<COMMS, baseline,tcode>::generateLocalAddressString() const
{
    std::string add;
    if (comms->isConnected())
    {
        add=comms->getAddress();
    }
    else
    {

        std::lock_guard<std::mutex> lock(dataMutex);
        if (!netInfo.localInterface.empty() && (netInfo.localInterface.back() == '*'))
        {
            add=makePortAddress(netInfo.localInterface.substr(0, netInfo.localInterface.size() - 1), netInfo.portNumber);
        }
        else
        {
            add=makePortAddress(netInfo.localInterface, netInfo.portNumber);
        }
    }
    return add;
}

} //namespace helics