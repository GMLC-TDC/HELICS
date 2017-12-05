/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "UdpBroker.h"
#include "../../common/blocking_queue.h"
#include "helics/helics-config.h"
#include "../core-data.h"
#include "../core.h"
#include "../helics-time.h"
#include "UdpComms.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>

#include "../argParser.h"

namespace helics
{
using namespace std::string_literals;
static const argDescriptors extraArgs{{"local_interface"s, "string"s,
                                       "the local interface to use for the receive ports"s},
                                      {"brokerport"s, "int"s, "port number for the broker priority port"s},
                                      {"localport"s, "int"s, "port number for the local receive port"s},
                                      {"port"s, "int"s, "port number for the broker's port"s},
                                      {"portstart"s, "int"s, "starting port for automatic port definitions"s}};

UdpBroker::UdpBroker (bool rootBroker) noexcept : CommsBroker (rootBroker) {}

UdpBroker::UdpBroker (const std::string &broker_name) : CommsBroker (broker_name) {}

UdpBroker::~UdpBroker() = default;

void UdpBroker::displayHelp (bool local_only)
{
    std::cout << " Help for Zero MQ Broker: \n";
    namespace po = boost::program_options;
    po::variables_map vm;
    const char *const argV[] = {"", "--help"};
    argumentParser (2, argV, vm, extraArgs);
    if (!local_only)
    {
        CoreBroker::displayHelp ();
    }
}

void UdpBroker::initializeFromArgs (int argc, const char *const *argv)
{
    namespace po = boost::program_options;
    if (brokerState == broker_state_t::created)
    {
        po::variables_map vm;
        argumentParser (argc, argv, vm, extraArgs);

        if (vm.count ("broker_address") > 0)
        {
            auto addr = vm["broker_address"].as<std::string> ();
            auto sc = addr.find_first_of (';', 7);  // min address is tcp://* 7 characters
            if (sc == std::string::npos)
            {
                auto brkprt = extractInterfaceandPort (addr);
                brokerAddress = brkprt.first;
            }
            else
            {
                auto brkprt = extractInterfaceandPort (addr.substr (0, sc));
                brokerAddress = brkprt.first;
                brokerPort = brkprt.second;
                brkprt = extractInterfaceandPort (addr.substr (sc + 1));
                if (brkprt.first != brokerAddress)
                {
                    // TODO::Print a message?
                }
            }
            if ((brokerAddress == "udp://*")||(brokerAddress=="*"))
            {  // the broker address can't use a wild card
                brokerAddress = "localhost";
            }
        }
        if (vm.count ("local_interface") > 0)
        {
            auto localprt = extractInterfaceandPort (vm["local_interface"].as<std::string> ());
            localInterface = localprt.first;
            PortNumber = localprt.second;
        }
        if (vm.count ("port") > 0)
        {
            brokerPort = vm["port"].as<int> ();
        }
        if (vm.count ("brokerport") > 0)
        {
            brokerPort = vm["brokerport"].as<int> ();
        }
        if (vm.count ("localport") > 0)
        {
            PortNumber = vm["pullport"].as<int> ();
        }
        if (vm.count ("portstart") > 0)
        {
            portStart = vm["portstart"].as<int> ();
        }
        CoreBroker::initializeFromArgs (argc, argv);
    }
}

bool UdpBroker::brokerConnect ()
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (brokerAddress.empty ())
    {
        setAsRoot ();
    }
    comms = std::make_unique<UdpComms> (localInterface, brokerAddress);
    comms->setCallback ([this](ActionMessage M) { addActionMessage (std::move (M)); });
    comms->setName (getIdentifier ());
    if (PortNumber > 0)
    {
        comms->setPortNumber (PortNumber);
    }
   
    if (portStart > 0)
    {
        comms->setAutomaticPortStartPort (portStart);
    }
    // comms->setMessageSize(maxMessageSize, maxMessageCount);
    auto res = comms->connect ();
    if (res)
    {
        if (PortNumber < 0)
        {
            PortNumber = comms->getPort ();
        }
       
    }
    return res;
}

std::string UdpBroker::getAddress () const
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (comms)
    {
        return comms->getAddress ();
    }
    return makePortAddress (localInterface, PortNumber);
}
}  // namespace helics
