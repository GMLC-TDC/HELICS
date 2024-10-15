/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "NetworkBrokerData.hpp"

#include "helics/core/BrokerFactory.hpp"
#include "helics/core/helicsCLI11.hpp"
#include "helics/core/helicsCLI11JsonConfig.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

namespace helics {
std::shared_ptr<helicsCLI11App> NetworkBrokerData::commandLineParser(std::string_view localAddress,
                                                                     bool enableConfig)
{
    auto nbparser = std::make_shared<helicsCLI11App>(
        "Network connection information \n(arguments allow '_' characters in the names and ignore them)");
    if (enableConfig) {
        auto* fmtr = addJsonConfig(nbparser.get());
        fmtr->maxLayers(0);
        fmtr->promoteSection("helics");
    }
    nbparser->option_defaults()->ignore_underscore()->ignore_case();

    auto* interfaceFlag = nbparser
                              ->add_flag("--local{0},--ipv4{4},--ipv6{6},--all{10},--external{10}",
                                         interfaceNetwork,
                                         "specify external interface to use, default is --local")
                              ->disable_flag_override();
    nbparser
        ->add_option("--network_connectivity",
                     interfaceNetwork,
                     "specify the connectivity of the network interface")
        ->transform(CLI::CheckedTransformer(
            {{"local", "0"}, {"ipv4", "4"}, {"ipv6", "6"}, {"external", "10"}, {"all", "10"}}))
        ->excludes(interfaceFlag);
    nbparser
        ->add_option_function<std::string>(
            "--broker_address",
            [this, localAddress](const std::string& addr) {
                auto brkprt = gmlc::networking::extractInterfaceAndPort(addr);
                brokerAddress = brkprt.first;
                brokerPort = brkprt.second;
                checkAndUpdateBrokerAddress(localAddress);
            },
            "location of the broker i.e. network address")
        ->envname("HELICS_BROKER_ADDRESS");
    nbparser->add_flag("--reuse_address",
                       reuse_address,
                       "allow the server to reuse a bound address, mostly useful for tcp cores");
    nbparser
        ->add_flag(
            "--noackconnect",
            noAckConnection,
            "specify that a connection_ack message is not required to be connected with a broker")
        ->ignore_underscore();

    nbparser->add_flag(
        "--force",
        forceConnection,
        "if set to true the broker will on an unsuccessful connection as root broker attempt to terminate any existing brokers and reconnect");
    nbparser->add_option_function<std::string>(
        "--broker",
        [this, localAddress](std::string addr) {
            auto brkr = BrokerFactory::findBroker(addr);
            if (brkr) {
                addr = brkr->getAddress();
            }
            if (brokerAddress.empty()) {
                auto brkprt = gmlc::networking::extractInterfaceAndPort(addr);
                brokerAddress = brkprt.first;
                brokerPort = brkprt.second;
                checkAndUpdateBrokerAddress(localAddress);
            } else {
                brokerName = addr;
            }
        },
        "identifier for the broker, this is either the name or network address use --broker_address or --brokername "
        "to explicitly set the network address or name the search for the broker is first by name");

    nbparser->add_option("--brokername", brokerName, "the name of the broker");
    nbparser->add_option("--maxsize", maxMessageSize, "The message buffer size")
        ->capture_default_str()
        ->check(CLI::PositiveNumber);
    nbparser
        ->add_option("--maxcount",
                     maxMessageCount,
                     "The maximum number of message to have in a queue")
        ->capture_default_str()
        ->check(CLI::PositiveNumber);
    nbparser->add_option("--networkretries", maxRetries, "the maximum number of network retries")
        ->capture_default_str();
    nbparser->add_flag("--useosport",
                       use_os_port,
                       "specify that the ports should be allocated by the host operating system");
    nbparser->add_flag("--autobroker",
                       autobroker,
                       "allow a broker to be automatically created if one is not available");
    nbparser->add_option("--brokerinitstring",
                         brokerInitString,
                         "the initialization string for the broker");
    nbparser
        ->add_option("--brokerinit", brokerInitString, "the initialization string for the broker")
        ->envname("HELICS_BROKER_INIT");
    nbparser
        ->add_flag_function(
            "--client{0},--server{1}",
            [this](int64_t val) {
                switch (server_mode) {
                    case ServerModeOptions::UNSPECIFIED:
                    case ServerModeOptions::SERVER_DEFAULT_ACTIVE:
                    case ServerModeOptions::SERVER_DEFAULT_DEACTIVATED:
                        server_mode = (val > 0) ? ServerModeOptions::SERVER_ACTIVE :
                                                  ServerModeOptions::SERVER_DEACTIVATED;
                        break;
                    default:
                        break;
                }
            },
            "specify that the network connection should be a server or client")
        ->disable_flag_override();
    nbparser->add_option_function<std::string>(
        "--local_interface",
        [this](const std::string& addr) {
            auto localprt = gmlc::networking::extractInterfaceAndPort(addr);
            localInterface = localprt.first;
            // this may get overridden later
            portNumber = localprt.second;
        },
        "the local interface to use for the receive ports");
    nbparser->add_option("--port,-p", portNumber, "port number to use")
        ->transform(CLI::Transformer({{"auto", "-1"}}, CLI::ignore_case));
    nbparser
        ->add_option("--brokerport",
                     brokerPort,
                     "the port number to use to connect with the broker")
        ->envname("HELICS_BROKER_PORT");

    nbparser
        ->add_option("--connectionport",
                     connectionPort,
                     "the port number to use to connect a co-simulation")
        ->envname("HELICS_CONNECTION_PORT");
    nbparser
        ->add_option("--connectionaddress",
                     connectionAddress,
                     "the network address to use to connect a co-simulation")
        ->envname("HELICS_CONNECTION_ADDRESS");
    nbparser
        ->add_option_function<int>(
            "--localport",
            [this](int port) {
                if (port == -999) {
                    use_os_port = true;
                } else {
                    portNumber = port;
                }
            },
            "port number for the local receive port")
        ->transform(CLI::Transformer({{"auto", "-1"}, {"os", "-999"}}, CLI::ignore_case))
        ->envname("HELICS_LOCAL_PORT");
    nbparser->add_option("--portstart", portStart, "starting port for automatic port definitions");

    auto* encrypt_group = nbparser->add_option_group("encryption", "options related to encryption");
    encrypt_group->add_flag("--encrypted", encrypted, "enable encryption on the network")
        ->envname("HELICS_ENCRYPTION");
    encrypt_group
        ->add_option("--encryption_config",
                     encryptionConfig,
                     "set the configuration file for encryption options")
        ->envname("HELICS_ENCRYPTION_CONFIG");

    nbparser->add_callback([this]() {
        if ((!brokerAddress.empty()) && (brokerPort == -1)) {
            if ((localInterface.empty()) && (portNumber != -1)) {
                std::swap(brokerPort, portNumber);
            }
        }
    });

    return nbparser;
}

void NetworkBrokerData::checkAndUpdateBrokerAddress(std::string_view localAddress)
{
    using gmlc::networking::InterfaceTypes;
    switch (allowedType) {
        case InterfaceTypes::TCP:
            if ((brokerAddress == "tcp://*") || (brokerAddress == "*") ||
                (brokerAddress == "tcp")) {  // the broker address can't use a wild card
                brokerAddress = localAddress;
            }
            break;
        case InterfaceTypes::UDP:
            if ((brokerAddress == "udp://*") || (brokerAddress == "*") ||
                (brokerAddress == "udp")) {  // the broker address can't use a wild card
                brokerAddress = localAddress;
            }
            break;
        case InterfaceTypes::IP:
            if ((brokerAddress == "udp://*") ||
                (brokerAddress == "udp")) {  // the broker address can't use a wild card
                brokerAddress = std::string("udp://");
                if (localAddress.compare(3, 3, "://") == 0) {
                    brokerAddress.append(localAddress.substr(6));
                } else {
                    brokerAddress.append(localAddress);
                }
            } else if ((brokerAddress == "tcp://*") ||
                       (brokerAddress == "tcp")) {  // the broker address can't use a wild card
                brokerAddress = std::string("tcp://");
                if (localAddress.compare(3, 3, "://") == 0) {
                    brokerAddress.append(localAddress.substr(6));
                } else {
                    brokerAddress.append(localAddress);
                }
            } else if (brokerAddress == "*") {
                brokerAddress = localAddress;
            }
            break;
        case InterfaceTypes::IPC:
        case InterfaceTypes::INPROC:
        default:
            if ((brokerAddress.empty()) && (!localAddress.empty())) {
                brokerAddress = localAddress;
            }
            break;
    }
}

}  // namespace helics
