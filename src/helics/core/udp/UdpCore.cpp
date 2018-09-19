/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "UdpCore.h"
#include "UdpComms.h"

namespace helics
{
namespace udp
{
UdpCore::UdpCore () noexcept {}

UdpCore::UdpCore (const std::string &core_name) : CommsBroker (core_name) {}

void UdpCore::initializeFromArgs (int argc, const char *const *argv)
{
    if (brokerState == created)
    {
        std::unique_lock<std::mutex> lock (dataMutex);
        if (brokerState == created)
        {
			netInfo.initializeFromArgs (argc, argv, "localhost");
			CommonCore::initializeFromArgs (argc, argv);
		}
    }
}

bool UdpCore::brokerConnect ()
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

std::string UdpCore::generateLocalAddressString () const
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
