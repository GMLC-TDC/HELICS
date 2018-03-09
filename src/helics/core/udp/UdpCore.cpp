/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
*/
#include "UdpCore.h"
#include "UdpComms.h"

namespace helics
{
namespace udp {
UdpCore::UdpCore () noexcept {}

UdpCore::~UdpCore () = default;

UdpCore::UdpCore (const std::string &core_name) : CommsBroker (core_name) {}

void UdpCore::initializeFromArgs (int argc, const char *const *argv)
{
    if (brokerState == created)
    {
        netInfo.initializeFromArgs (argc, argv, "localhost");
        CommonCore::initializeFromArgs (argc, argv);
    }
}

bool UdpCore::brokerConnect ()
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (netInfo.brokerAddress.empty ())  // cores require a broker
    {
        netInfo.brokerAddress = "localhost";
    }
    comms = std::make_unique<UdpComms> (netInfo);
    comms->setCallback ([this](ActionMessage M) { addActionMessage (std::move (M)); });
    comms->setName (getIdentifier ());

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

std::string UdpCore::getAddress () const
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (comms)
    {
        return comms->getAddress ();
    }
    return makePortAddress (netInfo.localInterface, netInfo.portNumber);
}

} // namespace udp
}  // namespace helics

