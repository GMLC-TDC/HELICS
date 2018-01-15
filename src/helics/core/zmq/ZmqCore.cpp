/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "ZmqCore.h"
#include "ZmqComms.h"

namespace helics
{
ZmqCore::ZmqCore () noexcept {}

ZmqCore::~ZmqCore () = default;

ZmqCore::ZmqCore (const std::string &core_name) : CommsBroker (core_name) {}

void ZmqCore::initializeFromArgs (int argc, const char *const *argv)
{
    if (brokerState == created)
    {
        netInfo.initializeFromArgs (argc, argv, "tcp://127.0.0.1");
        CommonCore::initializeFromArgs (argc, argv);
    }
}

bool ZmqCore::brokerConnect ()
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (netInfo.brokerAddress.empty ())  // cores require a broker
    {
        netInfo.brokerAddress = "tcp://127.0.0.1";
    }
    comms = std::make_unique<ZmqComms> (netInfo);
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

std::string ZmqCore::getAddress () const
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (comms)
    {
        return comms->getAddress ();
    }
    if (netInfo.localInterface == "tcp://*")
    {
        return makePortAddress ("tcp://127.0.0.1", netInfo.portNumber);
    }
    else
    {
        return makePortAddress (netInfo.localInterface, netInfo.portNumber);
    }
}

}  // namespace helics
