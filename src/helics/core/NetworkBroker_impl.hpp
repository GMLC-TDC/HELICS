/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../core/core-types.hpp"
#include "NetworkBroker.hpp"
namespace helics
{
constexpr const char *tstr[] = {"default", "ZeroMQ", "MPI",   "TEST",   "IPC",      "interprocess",
                                "TCP",     "UDP",    "undef", "nng",    "ZMQ_test", "TCPSS",
                                "undef",   "undef",  "http",  "unknown"};

constexpr const char *tcodeStr (int tcode) { return ((tcode >= 0) && (tcode < 15)) ? tstr[tcode] : tstr[15]; }

constexpr const char *defInterface[] = {"127.0.0.1", "127.0.0.1", "tcp://127.0.0.1", "_ipc_broker", ""};

template <class COMMS, interface_type baseline, int tcode>
NetworkBroker<COMMS, baseline, tcode>::NetworkBroker (bool rootBroker) noexcept
    : CommsBroker<COMMS, CoreBroker> (rootBroker)
{
    netInfo.server_mode = NetworkBrokerData::server_mode_options::server_default_active;
}

template <class COMMS, interface_type baseline, int tcode>
NetworkBroker<COMMS, baseline, tcode>::NetworkBroker (const std::string &broker_name)
    : CommsBroker<COMMS, CoreBroker> (broker_name)
{
    netInfo.server_mode = NetworkBrokerData::server_mode_options::server_default_active;
}

template <class COMMS, interface_type baseline, int tcode>
void NetworkBroker<COMMS, baseline, tcode>::displayHelp (bool local_only)
{
    std::cout << " Help for " << tcodeStr (tcode) << " Broker: \n";
    NetworkBrokerData::displayHelp ();
    if (!local_only)
    {
        CoreBroker::displayHelp ();
    }
}

template <class COMMS, interface_type baseline, int tcode>
void NetworkBroker<COMMS, baseline, tcode>::initializeFromArgs (int argc, const char *const *argv)
{
    if (BrokerBase::brokerState == BrokerBase::created)
    {
        std::lock_guard<std::mutex> lock (dataMutex);
        if (BrokerBase::brokerState == BrokerBase::created)
        {
            netInfo.initializeFromArgs (argc, argv, defInterface[static_cast<int> (baseline)]);
            CoreBroker::initializeFromArgs (argc, argv);
        }
    }
}

template <class COMMS, interface_type baseline, int tcode>
bool NetworkBroker<COMMS, baseline, tcode>::brokerConnect ()
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (netInfo.brokerAddress.empty ())
    {
        CoreBroker::setAsRoot ();
    }
    CommsBroker<COMMS, CoreBroker>::comms->setName (CoreBroker::getIdentifier ());
    CommsBroker<COMMS, CoreBroker>::comms->loadNetworkInfo (netInfo);
    CommsBroker<COMMS, CoreBroker>::comms->setTimeout (std::chrono::milliseconds (BrokerBase::networkTimeout));

    auto res = CommsBroker<COMMS, CoreBroker>::comms->connect ();
    if (res)
    {
        if (netInfo.portNumber < 0)
        {
            netInfo.portNumber = CommsBroker<COMMS, CoreBroker>::comms->getPort ();
        }
    }
    return res;
}

template <class COMMS, interface_type baseline, int tcode>
std::string NetworkBroker<COMMS, baseline, tcode>::generateLocalAddressString () const
{
    std::string add;
    if (CommsBroker<COMMS, CoreBroker>::comms->isConnected ())
    {
        add = CommsBroker<COMMS, CoreBroker>::comms->getAddress ();
    }
    else
    {
        std::lock_guard<std::mutex> lock (dataMutex);
        switch (baseline)
        {
        case interface_type::tcp:
        case interface_type::ip:
        case interface_type::udp:
          if (!netInfo.localInterface.empty () && (netInfo.localInterface.back () == '*')) {
              add = makePortAddress (netInfo.localInterface.substr (0, netInfo.localInterface.size () - 1),
                                     netInfo.portNumber);
          } else {
              add = makePortAddress (netInfo.localInterface, netInfo.portNumber);
          } break;
        case interface_type::inproc:
        case interface_type::ipc:
        default:
            if (!netInfo.localInterface.empty ())
            {
                add = netInfo.localInterface;
            }
            else
            {
                add = CoreBroker::getIdentifier();
            }
            break;
        }
    }
    return add;
}

}  // namespace helics
