/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "TcpBroker.h"
#include "../../common/argParser.h"
#include "TcpComms.h"
#include "TcpCommsSS.h"

namespace helics
{
namespace tcp
{
TcpBroker::TcpBroker (bool rootBroker) noexcept : CommsBroker (rootBroker) {}

TcpBroker::TcpBroker (const std::string &broker_name) : CommsBroker (broker_name) {}

void TcpBroker::displayHelp (bool local_only)
{
    std::cout << " Help for TCP Broker: \n";

    NetworkBrokerData::displayHelp ();
    if (!local_only)
    {
        CoreBroker::displayHelp ();
    }
}

void TcpBroker::initializeFromArgs (int argc, const char *const *argv)
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

bool TcpBroker::brokerConnect ()
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

std::string TcpBroker::generateLocalAddressString () const
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (comms->isConnected())
    {
        return comms->getAddress ();
    }
    return makePortAddress (netInfo.localInterface, netInfo.portNumber);
}

using namespace std::string_literals;
static const ArgDescriptors extraArgs{{"server"s, ArgDescriptor::arg_type_t::flag_type,
                                       "specify that the Broker should be a server"s},
                                      {"connections"s, ArgDescriptor::arg_type_t::vector_string,
                                       "target link connections"s}};

TcpBrokerSS::TcpBrokerSS (bool rootBroker) noexcept : CommsBroker (rootBroker) {}

TcpBrokerSS::TcpBrokerSS (const std::string &broker_name) : CommsBroker (broker_name) {}

void TcpBrokerSS::displayHelp (bool local_only)
{
    std::cout << " Help for TCP Broker: \n";
    variable_map vm;
    const char *const argV[] = {"", "--help"};
    argumentParser (2, argV, vm, extraArgs);
    NetworkBrokerData::displayHelp ();
    if (!local_only)
    {
        CoreBroker::displayHelp ();
    }
}

void TcpBrokerSS::initializeFromArgs (int argc, const char *const *argv)
{
    if (brokerState == created)
    {
        std::unique_lock<std::mutex> lock (dataMutex);
        if (brokerState == created)
        {
            variable_map vm;
            argumentParser (argc, argv, vm, extraArgs);

            if (vm.count ("connections") > 0)
            {
                connections = vm["connections"].as<std::vector<std::string>> ();
            }
            if (vm.count ("server") > 0)
            {
                serverMode = true;
            }
            netInfo.initializeFromArgs (argc, argv, "localhost");
            CoreBroker::initializeFromArgs (argc, argv);
        }
    }
}

bool TcpBrokerSS::brokerConnect ()
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (netInfo.brokerAddress.empty ())
    {
        setAsRoot ();
    }
    comms->loadNetworkInfo (netInfo);
    comms->setName (getIdentifier ());
    comms->setServerMode (serverMode);
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

std::string TcpBrokerSS::generateLocalAddressString () const
{
    if (comms->isConnected ())
    {
        return comms->getAddress ();
    }
    std::lock_guard<std::mutex> lock (dataMutex);
    return makePortAddress (netInfo.localInterface, netInfo.portNumber);
}

}  // namespace tcp
}  // namespace helics
