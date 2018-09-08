/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "TcpCore.h"
#include "TcpComms.h"

namespace helics
{
namespace tcp
{
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
    comms = std::make_unique<TcpComms> (netInfo);
    comms->setCallback ([this](ActionMessage &&M) { addActionMessage (std::move (M)); });
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
    std::lock_guard<std::mutex> lock (dataMutex);
    if (comms)
    {
        return comms->getAddress ();
    }
    return makePortAddress (netInfo.localInterface, netInfo.portNumber);
}

}  // namespace tcp
}  // namespace helics
