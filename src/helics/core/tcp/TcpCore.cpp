/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "TcpCore.h"

#include "../core-data.h"
#include "../core.h"
#include "../helics-time.h"
#include "TcpComms.h"
#include "helics/helics-config.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <sstream>

#include "../argParser.h"

namespace helics
{
using namespace std::string_literals;
static const argDescriptors extraArgs{
  {"local_interface"s, "string"s, "the local interface to use for the receive ports"s},
  {"brokerport"s, "int"s, "port number for the broker priority port"s},
  {"localport"s, "int"s, "port number for the local receive socket"s},
  {"port"s, "int"s, "port number for the broker's priority port"s},
};

TcpCore::TcpCore () noexcept {}

TcpCore::~TcpCore () = default;

TcpCore::TcpCore (const std::string &core_name) : CommsBroker (core_name) {}

void TcpCore::initializeFromArgs (int argc, const char *const *argv)
{
    namespace po = boost::program_options;
    if (brokerState == created)
    {
        po::variables_map vm;
        argumentParser (argc, argv, vm, extraArgs);

        if (vm.count ("broker_address") > 0)
        {
            auto addr = vm["broker_address"].as<std::string> ();
            auto sc = addr.find_first_of (';', 7);
            if (sc == std::string::npos)
            {
                auto brkprt = extractInterfaceandPort (addr);
                brokerAddress = brkprt.first;
                brokerPortNumber = brkprt.second;
            }
            else
            {
                auto brkprt = extractInterfaceandPort (addr.substr (0, sc));
                brokerAddress = brkprt.first;
                brokerPortNumber = brkprt.second;
                brkprt = extractInterfaceandPort (addr.substr (sc + 1));
                if (brkprt.first != brokerAddress)
                {
                    // TODO::Print a message?
                }
            }
            if ((brokerAddress == "*") || (brokerAddress == "Tcp"))
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
        else
        {
            localInterface = "localhost";
        }
        if (vm.count ("port") > 0)
        {
            PortNumber = vm["port"].as<int> ();
        }
        if (vm.count ("brokerport") > 0)
        {
            brokerPortNumber = vm["brokerport"].as<int> ();
        }
        if (vm.count ("localport") > 0)
        {
            PortNumber = vm["pullport"].as<int> ();
        }

        CommonCore::initializeFromArgs (argc, argv);
    }
}

bool TcpCore::brokerConnect ()
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (brokerAddress.empty ())  // cores require a broker
    {
        brokerAddress = "localhost";
    }
    comms = std::make_unique<TcpComms> (localInterface, brokerAddress);
    comms->setCallback ([this](ActionMessage M) { addActionMessage (std::move (M)); });
    comms->setName (getIdentifier ());
    if (PortNumber > 0)
    {
        comms->setPortNumber (PortNumber);
    }
    if (brokerPortNumber > 0)
    {
        comms->setBrokerPort (brokerPortNumber);
    }

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

std::string TcpCore::getAddress () const
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (comms)
    {
        return comms->getAddress ();
    }
    return makePortAddress (localInterface, PortNumber);
}

}  // namespace helics
