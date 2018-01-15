/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#include "NetworkBrokerData.h"
#include "../common/argParser.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/asio/ip/tcp.hpp>

using namespace std::string_literals;

namespace helics
{
static const ArgDescriptors extraArgs{{"interface"s, "string"s,
                                       "the local interface to use for the receive ports"s},
                                      {"broker,b"s, "string"s, "identifier for the broker"s},
                                      {"broker_address", "string"s, "location of the broker i.e network address"},
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

void NetworkBrokerData::initializeFromArgs (int argc, const char *const *argv, const std::string &localAddress)
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
        checkAndUpdateBrokerAddress (localAddress);
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
        checkAndUpdateBrokerAddress (localAddress);
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

    // check for port ambiguity
    if ((!brokerAddress.empty ()) && (brokerPort == -1))
    {
        if ((localInterface.empty ()) && (portNumber != -1))
        {
            std::swap (brokerPort, portNumber);
        }
    }
}

void NetworkBrokerData::checkAndUpdateBrokerAddress (const std::string &localAddress)
{
    switch (allowedType)
    {
    case interface_type::tcp:
        if ((brokerAddress == "tcp://*") || (brokerAddress == "*") || (brokerAddress == "tcp"))
        {  // the broker address can't use a wild card
            brokerAddress = localAddress;
        }
        break;
    case interface_type::udp:
        if ((brokerAddress == "udp://*") || (brokerAddress == "*") || (brokerAddress == "udp"))
        {  // the broker address can't use a wild card
            brokerAddress = localAddress;
        }
        break;
    case interface_type::both:
        if ((brokerAddress == "udp://*") || (brokerAddress == "udp"))
        {  // the broker address can't use a wild card
            brokerAddress = std::string ("udp://") + localAddress;
        }
        else if ((brokerAddress == "tcp://*") || (brokerAddress == "tcp"))
        {  // the broker address can't use a wild card
            brokerAddress = std::string ("tcp://") + localAddress;
        }
        else if (brokerAddress == "*")
        {
            brokerAddress = localAddress;
        }
        break;
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

std::string getLocalExternalAddressV4 ()
{
    boost::asio::io_service io_service;

    boost::asio::ip::tcp::resolver resolver (io_service);
    boost::asio::ip::tcp::resolver::query query (boost::asio::ip::tcp::v4 (), boost::asio::ip::host_name (), "");
    boost::asio::ip::tcp::resolver::iterator it = resolver.resolve (query);
    boost::asio::ip::tcp::endpoint endpoint = *it;

    return endpoint.address ().to_string ();
}

std::string getLocalExternalAddressV4 (const std::string & /*server*/)
{
    boost::asio::io_service io_service;

    boost::asio::ip::tcp::resolver resolver (io_service);
    boost::asio::ip::tcp::resolver::query query (boost::asio::ip::tcp::v4 (), boost::asio::ip::host_name (), "");
    boost::asio::ip::tcp::resolver::iterator it = resolver.resolve (query);
    boost::asio::ip::tcp::endpoint endpoint = *it;

    return endpoint.address ().to_string ();
}

}  // namespace helics