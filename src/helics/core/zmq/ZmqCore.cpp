/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "ZmqCore.h"

#include "../core-data.h"
#include "../core.h"
#include "../helics-time.h"
#include "ZmqComms.h"
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
  {"brokerpushport"s, "int"s, "port number for the broker primary push port"s},
  {"pullport"s, "int"s, "port number for the primary receive port"s},
  {"repport"s, "int"s, "port number for the priority receive port"s},
  {"port"s, "int"s, "port number for the broker's priority port"s},
};

ZmqCore::ZmqCore () noexcept {}

ZmqCore::~ZmqCore () = default;

ZmqCore::ZmqCore (const std::string &core_name) : CommsBroker (core_name) {}

void ZmqCore::initializeFromArgs (int argc, const char *const *argv)
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
                brokerReqPort = brkprt.second;
            }
            else
            {
                auto brkprt = extractInterfaceandPort (addr.substr (0, sc));
                brokerAddress = brkprt.first;
                brokerReqPort = brkprt.second;
                brkprt = extractInterfaceandPort (addr.substr (sc + 1));
                if (brkprt.first != brokerAddress)
                {
                    // TODO::Print a message?
                }
                brokerPushPort = brkprt.second;
            }
            if ((brokerAddress == "tcp://*") || (brokerAddress == "tcp"))
            {  // the broker address can't use a wild card
                brokerAddress = "tcp://127.0.0.1";
            }
        }
        if (vm.count ("local_interface") > 0)
        {
            auto localprt = extractInterfaceandPort (vm["local_interface"].as<std::string> ());
            localInterface = localprt.first;
            repPortNumber = localprt.second;
        }
        else
        {
            localInterface = "tcp://127.0.0.1";
        }
        if (vm.count ("port") > 0)
        {
            brokerReqPort = vm["port"].as<int> ();
        }
        if (vm.count ("brokerport") > 0)
        {
            brokerReqPort = vm["brokerport"].as<int> ();
        }
        if (vm.count ("brokerpushport") > 0)
        {
            brokerPushPort = vm["brokerpushport"].as<int> ();
        }
        if (vm.count ("pullport") > 0)
        {
            pullPortNumber = vm["pullport"].as<int> ();
        }
        if (vm.count ("repport") > 0)
        {
            repPortNumber = vm["repport"].as<int> ();
            if (pullPortNumber < 0)
            {
                if (repPortNumber > 0)
                {
                    pullPortNumber = repPortNumber + 1;
                }
            }
        }

        CommonCore::initializeFromArgs (argc, argv);
    }
}

bool ZmqCore::brokerConnect ()
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (brokerAddress.empty ())  // cores require a broker
    {
        brokerAddress = "tcp://127.0.0.1";
    }
    comms = std::make_unique<ZmqComms> (localInterface, brokerAddress);
    comms->setCallback ([this](ActionMessage M) { addActionMessage (std::move (M)); });
    comms->setName (getIdentifier ());
    if ((repPortNumber > 0) || (pullPortNumber > 0))
    {
        comms->setPortNumbers (repPortNumber, pullPortNumber);
    }
    if ((brokerReqPort > 0) || (brokerPushPort > 0))
    {
        comms->setBrokerPorts (brokerReqPort, brokerPushPort);
    }

    auto res = comms->connect ();
    if (res)
    {
        if (repPortNumber < 0)
        {
            repPortNumber = comms->getRequestPort ();
        }
        if (pullPortNumber < 0)
        {
            pullPortNumber = comms->getPushPort ();
        }
    }
    return res;
}

std::string ZmqCore::getAddress () const
{
    std::lock_guard<std::mutex> lock (dataMutex);
    if (comms)
    {
        return comms->getRequestAddress () + ";" + comms->getPushAddress ();
    }
    return makePortAddress (localInterface, repPortNumber);
}

}  // namespace helics
