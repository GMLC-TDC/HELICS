/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "gmlc/networking/addressOperations.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics {
/** define the network access*/
enum class InterfaceNetworks : char {
    LOCAL = 0,  //!< just open local ports
    IPV4 = 4,  //!< use external ipv4 ports
    IPV6 = 6,  //!< use external ipv6 ports
    ALL = 10,  //!< use all external ports
};

class helicsCLI11App;

/** helper class designed to contain the common elements between networking brokers and cores
 */
class NetworkBrokerData {
  public:
    enum class ServerModeOptions : char {
        UNSPECIFIED = 0,
        SERVER_DEFAULT_ACTIVE = 1,
        SERVER_DEFAULT_DEACTIVATED = 2,
        SERVER_ACTIVE = 3,
        SERVER_DEACTIVATED = 4,
    };

    using InterfaceTypes = gmlc::networking::InterfaceTypes;

    std::string brokerName;  //!< the identifier for the broker
    std::string brokerAddress;  //!< the address or domain name of the broker
    std::string localInterface;  //!< the interface to use for the local connection
    std::string brokerInitString;  //!< a string containing arguments for the broker initialization
    std::string connectionAddress;  //!< the address for connecting
    int portNumber{-1};  //!< the port number for the local interface
    int brokerPort{-1};  //!< the port number to use for the main broker interface
    int connectionPort{-1};  //!< the port number for connecting

    int portStart{-1};  //!< the starting port for automatic port definitions
    int maxMessageSize{16 * 256};  //!< maximum message size
    int maxMessageCount{256};  //!< maximum message count
    int maxRetries{5};  //!< the maximum number of retries to establish a network connection
    InterfaceNetworks interfaceNetwork{InterfaceNetworks::LOCAL};
    bool reuse_address{false};  //!< allow reuse of binding address
    bool use_os_port{false};  //!< specify that any automatic port allocation should use operating
                              //!< system allocation
    bool autobroker{false};  //!< flag for specifying an automatic broker generation
    bool appendNameToAddress{
        false};  //!< flag indicating that the name should be appended to the address
    bool noAckConnection{false};  //!< flag indicating that a connection ack message is not required
                                  //!< for broker connections
    bool useJsonSerialization{false};  //!< for message serialization use JSON
    bool observer{false};  //!< specify that the network connection is used for observation only
    ServerModeOptions server_mode{ServerModeOptions::UNSPECIFIED};  //!< setup a server mode
  public:
    NetworkBrokerData() = default;
    /** constructor from the allowed type*/
    explicit NetworkBrokerData(InterfaceTypes type): allowedType(type) {}

    /** generate a command line argument parser for the network broker data
     @param localAddress a predefined string containing the desired local only address
    */
    std::shared_ptr<helicsCLI11App> commandLineParser(const std::string& localAddress,
                                                      bool enableConfig = true);
    /** set the desired interface type
     */
    void setInterfaceType(InterfaceTypes type) { allowedType = type; }

  private:
    /** do some checking on the brokerAddress*/
    void checkAndUpdateBrokerAddress(const std::string& localAddress);
    InterfaceTypes allowedType = InterfaceTypes::IP;
};

/** create a combined address list with choices in a rough order of priority based on if they appear
in both lists, followed by the high priority addresses, and low priority addresses last

@param high addresses that should be considered before low addresses
@param low addresses that should be considered last
@return a vector of strings of ip addresses ordered in roughly the priority they should be used
 */
std::vector<std::string> prioritizeExternalAddresses(std::vector<std::string> high,
                                                     std::vector<std::string> low);

/** get the external ipv4 address of the current computer
 */
std::string getLocalExternalAddressV4();

/** get the external ipv4 Ethernet address of the current computer that best matches the listed
 * server*/
std::string getLocalExternalAddress(const std::string& server);

/** get the external ipv4 Ethernet address of the current computer that best matches the listed
 * server*/
std::string getLocalExternalAddressV4(const std::string& server);

/** get the external ipv4 address of the current computer
 */
std::string getLocalExternalAddressV6();

/** get the external ipv4 Ethernet address of the current computer that best matches the listed
 * server*/
std::string getLocalExternalAddressV6(const std::string& server);

/** generate an interface that matches a defined server or network specification
 */
std::string generateMatchingInterfaceAddress(const std::string& server,
                                             InterfaceNetworks network = InterfaceNetworks::LOCAL);
}  // namespace helics
