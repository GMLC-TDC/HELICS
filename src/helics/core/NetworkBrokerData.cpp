/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "NetworkBrokerData.hpp"
#include "../common/argParser.h"
#include "BrokerFactory.hpp"

#include "../common/AsioServiceManager.h"
#include <boost/asio/ip/host_name.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <iostream>

using namespace std::string_literals;

namespace helics
{
static const ArgDescriptors extraArgs{
  {"interface"s, "the local interface to use for the receive ports"s},
  {"local_interface"s, "the local interface to use for the receive ports"s},
  {"broker,b"s,
   "identifier for the broker, this is either the name or network address use --broker_address or --brokername to explicitly set the network address or name the search for the broker is first by name"s},
  {"broker_address"s, "location of the broker i.e network address"s},
  {"network_retries"s, ArgDescriptor::arg_type_t::int_type, "the maximum number of network retries"s},
  {"brokername"s, "the name of the broker"s},
  {"brokerinit"s, "the initialization string for the broker"s},
  {"max_size"s, ArgDescriptor::arg_type_t::int_type, "maximum message buffer size (16*1024)"s},
  {"max_count"s, ArgDescriptor::arg_type_t::int_type, "max queue size for the message (256)"s},
  {"local"s, ArgDescriptor::arg_type_t::flag_type, "use local interface(default)"s},
  {"ipv4"s, ArgDescriptor::arg_type_t::flag_type, "use external ipv4 addresses"s},
  {"ipv6"s, ArgDescriptor::arg_type_t::flag_type, "use external ipv6 addresses"s},
  {"server"s, ArgDescriptor::arg_type_t::flag_type, "specify that the network connection should be a server"s},
  {"os_port"s, ArgDescriptor::arg_type_t::flag_type,
   "specify that the ports should be allocated by the host operating system"s},
  {"autobroker"s, ArgDescriptor::arg_type_t::flag_type,
   "allow a broker to be automatically created if one is not available"s},
  {"client"s, ArgDescriptor::arg_type_t::flag_type, "specify that the network connection should be a client"s},
  {"reuse_address"s, ArgDescriptor::arg_type_t::flag_type, "allow the server to reuse a bound address"s},
  {"external"s, ArgDescriptor::arg_type_t::flag_type, "use all external interfaces"s},
  {"brokerport"s, ArgDescriptor::arg_type_t::int_type, "port number for the broker priority port"s},
  {"localport"s, "port number for the local receive port"s},
  {"port"s, ArgDescriptor::arg_type_t::int_type, "port number for the broker's port"s},
  {"portstart"s, ArgDescriptor::arg_type_t::int_type, "starting port for automatic port definitions"s}};

void NetworkBrokerData::displayHelp ()
{
    const char *const argV[] = {"", "--help"};
    variable_map vm;
    argumentParser (2, argV, vm, extraArgs);
}

void NetworkBrokerData::initializeFromArgs (int argc, const char *const *argv, const std::string &localAddress)
{
    variable_map vm;
    argumentParser (argc, argv, vm, extraArgs);
    if (vm.count ("local") > 0)
    {
        interfaceNetwork = interface_networks::local;
    }
    else if (vm.count ("ipv4") > 0)
    {
        interfaceNetwork = interface_networks::ipv4;
    }
    else if (vm.count ("ipv6") > 0)
    {
        interfaceNetwork = interface_networks::ipv6;
    }
    else if (vm.count ("external") > 0)
    {
        interfaceNetwork = interface_networks::all;
    }
    if (vm.count ("broker_address") > 0)
    {
        auto addr = vm["broker_address"].as<std::string> ();
        auto sc = addr.find_first_of (';', 1);
        if (sc == std::string::npos)
        {
            auto brkprt = extractInterfaceandPort (addr);
            brokerAddress = brkprt.first;
            brokerPort = brkprt.second;
        }
        else
        {
            auto brkprt = extractInterfaceandPort (addr.substr (0, sc));
            brokerAddress = brkprt.first;
            brokerPort = brkprt.second;
            brkprt = extractInterfaceandPort (addr.substr (sc + 1));
            if (brkprt.first != brokerAddress)
            {
                std::cerr << "it is not recommended to specify multiple addresses from different servers"
                          << std::endl;
            }
        }
        checkAndUpdateBrokerAddress (localAddress);
    }

    if (vm.count ("reuse_address") > 0)
    {
        reuse_address = true;
    }
    if (vm.count ("broker") > 0)
    {
        auto addr = vm["broker"].as<std::string> ();
        auto brkr = BrokerFactory::findBroker (addr);
        if (brkr)
        {
            addr = brkr->getAddress ();
        }
        if (brokerAddress.empty ())
        {
            auto sc = addr.find_first_of (';', 1);
            if (sc == std::string::npos)
            {
                auto brkprt = extractInterfaceandPort (addr);
                brokerAddress = brkprt.first;
                brokerPort = brkprt.second;
            }
            else
            {
                auto brkprt = extractInterfaceandPort (addr.substr (0, sc));
                brokerAddress = brkprt.first;
                brokerPort = brkprt.second;
                brkprt = extractInterfaceandPort (addr.substr (sc + 1));
                if (brkprt.first != brokerAddress)
                {
                    std::cerr << "it is not recommended to specify multiple addresses from different servers"
                              << std::endl;
                }
            }
            checkAndUpdateBrokerAddress (localAddress);
        }
        else
        {
            brokerName = addr;
        }
    }
    if (vm.count ("brokername") > 0)
    {
        brokerName = vm["brokername"].as<std::string> ();
    }
    if (vm.count ("max_size") > 0)
    {
        auto bsize = vm["max_size"].as<int> ();
        if (bsize > 0)
        {
            maxMessageSize = bsize;
        }
    }
    if (vm.count ("max_count") > 0)
    {
        auto msize = vm["max_count"].as<int> ();
        if (msize > 0)
        {
            maxMessageSize = msize;
        }
    }
    if (vm.count ("network_retries") > 0)
    {
        maxRetries = vm["network_retries"].as<int> ();
    }
    if (vm.count ("os_port") > 0)
    {
        use_os_port = true;
    }
    if (vm.count ("autobroker") > 0)
    {
        autobroker = true;
    }
    if (vm.count ("brokerinit") > 0)
    {
        brokerInitString = vm["brokerinit"].as<std::string> ();
    }
    if (vm.count ("server") > 0)
    {
        switch (server_mode)
        {
        case server_mode_options::unspecified:
        case server_mode_options::server_default_active:
        case server_mode_options::server_default_deactivated:
            server_mode = server_mode_options::server_active;
            break;
        default:
            break;
        }
    }
    if (vm.count ("client") > 0)
    {
        switch (server_mode)
        {
        case server_mode_options::unspecified:
        case server_mode_options::server_default_active:
        case server_mode_options::server_default_deactivated:
            server_mode = server_mode_options::server_deactivated;
            break;
        default:
            break;
        }
    }
    if (vm.count ("interface") > 0)
    {
        auto localprt = extractInterfaceandPort (vm["interface"].as<std::string> ());
        localInterface = localprt.first;
        // this may get overridden later
        portNumber = localprt.second;
    }
    if (vm.count ("local_interface") > 0)
    {
        auto localprt = extractInterfaceandPort (vm["local_interface"].as<std::string> ());
        localInterface = localprt.first;
        // this may get overridden later
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
        auto pstring = vm["localport"].as<std::string> ();
        if (pstring == "os")
        {
            use_os_port = true;
        }
        else if (pstring == "auto")
        {
            portNumber = -1;
        }
        else
        {
            try
            {
                portNumber = std::stoi (pstring);
            }
            catch (...)
            {
                std::cerr << "failed to convert " << pstring << " to a valid port number";
            }
        }
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
    case interface_type::ip:
        if ((brokerAddress == "udp://*") || (brokerAddress == "udp"))
        {  // the broker address can't use a wild card
            if (localAddress.compare (3, 3, "://") == 0)
            {
                brokerAddress = std::string ("udp://") + localAddress.substr (6);
            }
            else
            {
                brokerAddress = std::string ("udp://") + localAddress;
            }
        }
        else if ((brokerAddress == "tcp://*") || (brokerAddress == "tcp"))
        {  // the broker address can't use a wild card
            if (localAddress.compare (3, 3, "://") == 0)
            {
                brokerAddress = std::string ("tcp://") + localAddress.substr (6);
            }
            else
            {
                brokerAddress = std::string ("tcp://") + localAddress;
            }
        }
        else if (brokerAddress == "*")
        {
            brokerAddress = localAddress;
        }
        break;
    case interface_type::ipc:
    case interface_type::inproc:
        if ((brokerAddress.empty ()) && (!localAddress.empty ()))
        {
            brokerAddress = localAddress;
        }
    }
}

std::string makePortAddress (const std::string &networkInterface, int portNumber)
{
    std::string newAddress = networkInterface;
    if (portNumber >= 0)
    {
        newAddress.push_back (':');
        newAddress.append (std::to_string (portNumber));
    }
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
            if ((address.size () > lastColon + 1) && (address[lastColon + 1] != '/'))
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

std::string stripProtocol (const std::string &networkAddress)
{
    auto loc = networkAddress.find ("://");
    if (loc != std::string::npos)
    {
        return networkAddress.substr (loc + 3);
    }
    return networkAddress;
}

void removeProtocol (std::string &networkAddress)
{
    auto loc = networkAddress.find ("://");
    if (loc != std::string::npos)
    {
        networkAddress.erase (0, loc + 3);
    }
}

std::string addProtocol (const std::string &networkAddress, interface_type interfaceT)
{
    if (networkAddress.find ("://") == std::string::npos)
    {
        switch (interfaceT)
        {
        case interface_type::ip:
        case interface_type::tcp:
            return std::string ("tcp://") + networkAddress;
        case interface_type::ipc:
            return std::string ("ipc://") + networkAddress;
        case interface_type::udp:
            return std::string ("udp://") + networkAddress;
        case interface_type::inproc:
            return std::string ("inproc://") + networkAddress;
        }
    }
    return networkAddress;
}

void insertProtocol (std::string &networkAddress, interface_type interfaceT)
{
    if (networkAddress.find ("://") == std::string::npos)
    {
        switch (interfaceT)
        {
        case interface_type::ip:
        case interface_type::tcp:
            networkAddress.insert (0, "tcp://");
            break;
        case interface_type::ipc:
            networkAddress.insert (0, "ipc://");
            break;
        case interface_type::udp:
            networkAddress.insert (0, "udp://");
            break;
        case interface_type::inproc:
            networkAddress.insert (0, "inproc://");
            break;
        }
    }
}

bool isipv6 (const std::string &address)
{
    auto cntcolon = std::count (address.begin (), address.end (), ':');
    if (cntcolon > 2)
    {
        return true;
    }

    auto brkcnt = address.find_first_of ('[');
    if (brkcnt != std::string::npos)
    {
        return true;
    }
    if (address.compare (0, 2, "::") == 0)
    {
        return true;
    }
    return false;
}

template <class InputIt1, class InputIt2>
auto matchcount (InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2)
{
    int cnt = 0;
    while (first1 != last1 && first2 != last2 && *first1 == *first2)
    {
        ++first1, ++first2, ++cnt;
    }
    return cnt;
}

std::string getLocalExternalAddressV4 ()
{
    auto srv = AsioServiceManager::getServicePointer ();

    boost::asio::ip::tcp::resolver resolver (srv->getBaseService ());
    boost::asio::ip::tcp::resolver::query query (boost::asio::ip::tcp::v4 (), boost::asio::ip::host_name (), "");
    boost::asio::ip::tcp::resolver::iterator it = resolver.resolve (query);
    boost::asio::ip::tcp::endpoint endpoint = *it;

    return endpoint.address ().to_string ();
}

std::string getLocalExternalAddressV4 (const std::string &server)
{
    auto srv = AsioServiceManager::getServicePointer ();

    boost::asio::ip::tcp::resolver resolver (srv->getBaseService ());

    boost::asio::ip::tcp::resolver::query query_server (boost::asio::ip::tcp::v4 (), server, "");
    boost::system::error_code ec;
    boost::asio::ip::tcp::resolver::iterator it_server = resolver.resolve (query_server, ec);
    if (ec)
    {
        return getLocalExternalAddressV4 ();
    }
    boost::asio::ip::tcp::endpoint servep = *it_server;

    boost::asio::ip::tcp::resolver::iterator end;

    auto sstring = (it_server == end) ? server : servep.address ().to_string ();

    boost::asio::ip::tcp::resolver::query query (boost::asio::ip::tcp::v4 (), boost::asio::ip::host_name (), "");
    boost::asio::ip::tcp::resolver::iterator it = resolver.resolve (query);
    boost::asio::ip::tcp::endpoint endpoint = *it;
    int cnt = 0;
    std::string def = endpoint.address ().to_string ();
    cnt = matchcount (sstring.begin (), sstring.end (), def.begin (), def.end ());
    ++it;
    while (it != end)
    {
        boost::asio::ip::tcp::endpoint ept = *it;
        std::string ndef = ept.address ().to_string ();
        auto mcnt = matchcount (sstring.begin (), sstring.end (), ndef.begin (), ndef.end ());
        if ((mcnt > cnt) && (mcnt >= 7))
        {
            def = ndef;
            cnt = mcnt;
        }
        ++it;
    }
    return def;
}

std::string getLocalExternalAddressV6 ()
{
    auto srv = AsioServiceManager::getServicePointer ();

    boost::asio::ip::tcp::resolver resolver (srv->getBaseService ());
    boost::asio::ip::tcp::resolver::query query (boost::asio::ip::tcp::v6 (), boost::asio::ip::host_name (), "");
    boost::asio::ip::tcp::resolver::iterator it = resolver.resolve (query);
    boost::asio::ip::tcp::endpoint endpoint = *it;

    return endpoint.address ().to_string ();
}

std::string getLocalExternalAddressV6 (const std::string &server)
{
    auto srv = AsioServiceManager::getServicePointer ();

    boost::asio::ip::tcp::resolver resolver (srv->getBaseService ());

    boost::asio::ip::tcp::resolver::query query_server (boost::asio::ip::tcp::v6 (), server, "");
    boost::asio::ip::tcp::resolver::iterator it_server = resolver.resolve (query_server);
    boost::asio::ip::tcp::endpoint servep = *it_server;
    boost::asio::ip::tcp::resolver::iterator end;

    auto sstring = (it_server == end) ? server : servep.address ().to_string ();

    boost::asio::ip::tcp::resolver::query query (boost::asio::ip::tcp::v6 (), boost::asio::ip::host_name (), "");
    boost::asio::ip::tcp::resolver::iterator it = resolver.resolve (query);
    boost::asio::ip::tcp::endpoint endpoint = *it;

    if (it == end)
    {
        return std::string ();
    }
    int cnt = 0;
    std::string def = endpoint.address ().to_string ();
    cnt = matchcount (sstring.begin (), sstring.end (), def.begin (), def.end ());
    ++it;
    while (it != end)
    {
        boost::asio::ip::tcp::endpoint ept = *it;
        std::string ndef = ept.address ().to_string ();
        auto mcnt = matchcount (sstring.begin (), sstring.end (), ndef.begin (), ndef.end ());
        if ((mcnt > cnt) && (mcnt >= 9))
        {
            def = ndef;
            cnt = mcnt;
        }
        ++it;
    }
    return def;
}

std::string getLocalExternalAddress (const std::string &server)
{
    if (isipv6 (server))
    {
        return getLocalExternalAddressV6 (server);
    }
    return getLocalExternalAddressV4 (server);
}

std::string generateMatchingInterfaceAddress (const std::string &server, interface_networks network)
{
    std::string newInterface;
    switch (network)
    {
    case interface_networks::local:
        if (server.empty ())
        {
            newInterface = "tcp://127.0.0.1";
        }
        else
        {
            newInterface = getLocalExternalAddress (server);
        }
        break;
    case interface_networks::ipv4:
        if (server.empty ())
        {
            newInterface = "tcp://*";
        }
        else
        {
            newInterface = getLocalExternalAddressV4 (server);
        }
        break;
    case interface_networks::ipv6:
        if (server.empty ())
        {
            newInterface = "tcp://*";
        }
        else
        {
            newInterface = getLocalExternalAddressV6 (server);
        }
        break;
    case interface_networks::all:
        if (server.empty ())
        {
            newInterface = "tcp://*";
        }
        else
        {
            newInterface = getLocalExternalAddress (server);
        }
        break;
    }
    return newInterface;
}

}  // namespace helics
