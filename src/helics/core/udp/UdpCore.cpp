/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "UdpCore.h"
#include "UdpComms.h"

namespace helics
{

UdpCore::UdpCore () noexcept {}

UdpCore::~UdpCore () = default;

UdpCore::UdpCore (const std::string &core_name) : CommsBroker (core_name) {}

void UdpCore::initializeFromArgs (int argc, const char *const *argv)
{
    
    if (brokerState == created)
    {
        netInfo.initializeFromArgs(argc, argv, "localhost");
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

}  // namespace helics
