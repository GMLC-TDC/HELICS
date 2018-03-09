/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.
*/

#include "ZmqBroker.h"
#include "ZmqComms.h"

namespace helics
{
namespace zeromq {
ZmqBroker::ZmqBroker (bool rootBroker) noexcept : CommsBroker (rootBroker) {}

ZmqBroker::ZmqBroker (const std::string &broker_name) : CommsBroker (broker_name) {}

ZmqBroker::~ZmqBroker () = default;

void ZmqBroker::displayHelp (bool local_only)
{
    std::cout << " Help for Zero MQ Broker: \n";
    NetworkBrokerData::displayHelp ();
    if (!local_only)
    {
        CoreBroker::displayHelp ();
    }
}

void ZmqBroker::initializeFromArgs (int argc, const char *const *argv)
{
    if (brokerState == broker_state_t::created)
    {
        netInfo.initializeFromArgs (argc, argv, "tcp://127.0.0.1");
        CoreBroker::initializeFromArgs (argc, argv);
    }
}

bool ZmqBroker::brokerConnect ()
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (netInfo.brokerAddress.empty ())
    {
        setAsRoot ();
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

std::string ZmqBroker::getAddress () const
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

