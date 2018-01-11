/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "NetworkBrokerData.h"
#include "argParser.h"

using namespace std::string_literals;

namespace helics
{
static const argDescriptors extraArgs{{"interface"s, "string"s,
                                       "the local interface to use for the receive ports"s},
                                      {"brokerport"s, "int"s, "port number for the broker priority port"s},
                                      {"localport"s, "int"s, "port number for the local receive port"s},
                                      {"port"s, "int"s, "port number for the broker's port"s},
                                      {"portstart"s, "int"s, "starting port for automatic port definitions"s}};

void NetworkBrokerData::displayHelp ()
{
    const char *const argV[] = {"", "--help"};
    boost::program_options::variables_map vm;
    argumentParser (2, argV, vm, extraArgs);
}

void NetworkBrokerData::initializeFromArgs (int argc, const char *const *argv)
{
    namespace po = boost::program_options;
    po::variables_map vm;
    argumentParser (argc, argv, vm, extraArgs);

    if (vm.count ("broker_address") > 0)
    {
        auto addr = vm["broker_address"].as<std::string> ();
        auto sc = addr.find_first_of (';', 1);
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
        if ((brokerAddress == "tcp://*") || (brokerAddress == "*") || (brokerAddress == "tcp"))
        {  // the broker address can't use a wild card
            brokerAddress = "localhost";
        }
    }
    else if (vm.count ("broker") > 0)
    {
        auto addr = vm["broker"].as<std::string> ();
        auto sc = addr.find_first_of (';', 1);  // min address is tcp://* 7 characters
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
        if ((brokerAddress == "tcp://*") || (brokerAddress == "*") || (brokerAddress == "tcp"))
        {  // the broker address can't use a wild card
            brokerAddress = "localhost";
        }
    }
    if (vm.count ("interface") > 0)
    {
        auto localprt = extractInterfaceandPort (vm["interface"].as<std::string> ());
        localInterface = localprt.first;
        portNumber = localprt.second;
    }
    if (vm.count ("port") > 0)
    {  // there is some ambiguity of what port could mean this is dealt with later
        portNumber = vm["port"].as<int> ();
    }
    if (vm.count ("brokerport") > 0)
    {
        brokerPort = vm["brokerport"].as<int> ();
    }
    if (vm.count ("localport") > 0)
    {
        portNumber = vm["localport"].as<int> ();
    }
    if (vm.count ("portstart") > 0)
    {
        portStart = vm["portstart"].as<int> ();
    }

    // check the port ambiguity
    if ((!brokerAddress.empty ()) && (brokerPort == -1))
    {
        if ((localInterface.empty ()) && (portNumber != -1))
        {
            std::swap (brokerPort, portNumber);
        }
    }
}

std::string makePortAddress (const std::string &networkInterface, int portNumber)
{
    std::string newAddress = networkInterface;
    newAddress.push_back (':');
    newAddress.append (std::to_string (portNumber));
    return newAddress;
}

std::pair<std::string, int> extractInterfaceandPort (const std::string &address)
{
    std::pair<std::string, int> ret;
    auto lastColon = address.find_last_of (':');
    if (lastColon == std::string::npos)
    {
        ret = std::make_pair (address, -1);
    }
    else
    {
        try
        {
            if (address[lastColon + 1] != '/')
            {
                auto val = std::stoi (address.substr (lastColon + 1));
                ret.first = address.substr (0, lastColon);
                ret.second = val;
            }
            else
            {
                ret = std::make_pair (address, -1);
            }
        }
        catch (const std::invalid_argument &)
        {
            ret = std::make_pair (address, -1);
        }
    }

    return ret;
}

std::pair<std::string, std::string> extractInterfaceandPortString (const std::string &address)
{
    auto lastColon = address.find_last_of (':');
    return std::make_pair (address.substr (0, lastColon), address.substr (lastColon + 1));
}

}  // namespace helics