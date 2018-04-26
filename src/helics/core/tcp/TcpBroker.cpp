/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "TcpBroker.h"
#include "TcpComms.h"

namespace helics
{
namespace tcp
{
TcpBroker::TcpBroker (bool rootBroker) noexcept : CommsBroker (rootBroker) {}

TcpBroker::TcpBroker (const std::string &broker_name) : CommsBroker (broker_name) {}

void TcpBroker::displayHelp (bool local_only)
{
    std::cout << " Help for TCP Broker: \n";

    NetworkBrokerData::displayHelp ();
    if (!local_only)
    {
        CoreBroker::displayHelp ();
    }
}

void TcpBroker::initializeFromArgs (int argc, const char *const *argv)
{
    if (brokerState == broker_state_t::created)
    {
        netInfo.initializeFromArgs (argc, argv, "localhost");
        CoreBroker::initializeFromArgs (argc, argv);
    }
}

bool TcpBroker::brokerConnect ()
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (netInfo.brokerAddress.empty ())
    {
        setAsRoot ();
    }
    comms = std::make_unique<TcpComms> (netInfo);
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

std::string TcpBroker::getAddress () const
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (comms)
    {
        return comms->getAddress ();
    }
    return makePortAddress (netInfo.localInterface, netInfo.portNumber);
}

}  // namespace tcp
}  // namespace helics
