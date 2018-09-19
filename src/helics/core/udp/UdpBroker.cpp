/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "UdpBroker.h"
#include "UdpComms.h"

namespace helics
{
namespace udp
{
UdpBroker::UdpBroker (bool rootBroker) noexcept : CommsBroker (rootBroker) {}

UdpBroker::UdpBroker (const std::string &broker_name) : CommsBroker (broker_name) {}

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
    if (brokerState == created)
    {
        std::unique_lock<std::mutex> lock (dataMutex);
        if (brokerState == created)
        {
			netInfo.initializeFromArgs (argc, argv, "localhost");
			CoreBroker::initializeFromArgs (argc, argv);
		}
    }
}

bool UdpBroker::brokerConnect ()
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (netInfo.brokerAddress.empty ())
    {
        setAsRoot ();
    }
    comms->loadNetworkInfo (netInfo);
    comms->setName (getIdentifier ());
    comms->setTimeout (networkTimeout);
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

std::string UdpBroker::generateLocalAddressString () const
{
    
    if (comms->isConnected())
    {
        return comms->getAddress ();
    }
    std::lock_guard<std::mutex> lock (dataMutex);
    return makePortAddress (netInfo.localInterface, netInfo.portNumber);
}
}  // namespace udp
}  // namespace helics
