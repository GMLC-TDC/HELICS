/*

Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "ZmqCore.h"
#include "ZmqComms.h"

namespace helics
{
namespace zeromq {
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

} // namespace zeromq
}  // namespace helics

