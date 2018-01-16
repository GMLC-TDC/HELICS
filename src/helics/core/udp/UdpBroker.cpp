/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "UdpBroker.h"
#include "UdpComms.h"

namespace helics
{
UdpBroker::UdpBroker (bool rootBroker) noexcept : CommsBroker (rootBroker) {}

UdpBroker::UdpBroker (const std::string &broker_name) : CommsBroker (broker_name) {}

UdpBroker::~UdpBroker () = default;

void UdpBroker::displayHelp (bool local_only)
{
    std::cout << " Help for UDP Broker: \n";
    NetworkBrokerData::displayHelp ();
    if (!local_only)
    {
        CoreBroker::displayHelp ();
    }
}

void UdpBroker::initializeFromArgs (int argc, const char *const *argv)
{
    if (brokerState == broker_state_t::created)
    {
        netInfo.initializeFromArgs (argc, argv, "localhost");
        CoreBroker::initializeFromArgs (argc, argv);
    }
}

bool UdpBroker::brokerConnect ()
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (netInfo.brokerAddress.empty ())
    {
        setAsRoot ();
    }
    comms = std::make_unique<UdpComms> (netInfo);
    comms->setCallback ([this](ActionMessage M) { addActionMessage (std::move (M)); });
    comms->setName (getIdentifier ());

    // comms->setMessageSize(maxMessageSize, maxMessageCount);
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

std::string UdpBroker::getAddress () const
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (comms)
    {
        return comms->getAddress ();
    }
    return makePortAddress (netInfo.localInterface, netInfo.portNumber);
}
}  // namespace helics
