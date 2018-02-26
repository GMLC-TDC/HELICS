/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "TcpCore.h"
#include "TcpComms.h"

namespace helics
{
namespace tcp {
TcpCore::TcpCore () noexcept {}

TcpCore::~TcpCore () = default;

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

std::string TcpCore::getAddress () const
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (comms)
    {
        return comms->getAddress ();
    }
    return makePortAddress (netInfo.localInterface, netInfo.portNumber);
}

} // namespace tcp
}  // namespace helics
