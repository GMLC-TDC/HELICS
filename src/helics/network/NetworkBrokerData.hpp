/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "gmlc/networking/addressOperations.hpp"
#include "gmlc/networking/interfaceOperations.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics {

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
    gmlc::networking::InterfaceNetworks interfaceNetwork{
        gmlc::networking::InterfaceNetworks::LOCAL};
    bool reuse_address{false};  //!< allow reuse of binding address
    /// specify that any automatic port allocation should use operating system allocation
    bool use_os_port{false};
    bool autobroker{false};  //!< flag for specifying an automatic broker generation
    /// flag indicating that the name should be appended to the address
    bool appendNameToAddress{false};
    bool noAckConnection{false};  //!< flag indicating that a connection ack message is not required
                                  //!< for broker connections
    bool useJsonSerialization{false};  //!< for message serialization use JSON
    bool observer{false};  //!< specify that the network connection is used for observation only
    ServerModeOptions server_mode{ServerModeOptions::UNSPECIFIED};  //!< setup a server mode
    bool encrypted{false};  // enable encryption
    bool forceConnection{false};  // force the connection and terminate existing connections
    std::string encryptionConfig;

  public:
    NetworkBrokerData() = default;
    /** constructor from the allowed type*/
    explicit NetworkBrokerData(gmlc::networking::InterfaceTypes type): allowedType(type) {}

    /** generate a command line argument parser for the network broker data
     @param localAddress a predefined string containing the desired local only address
    */
    std::shared_ptr<helicsCLI11App> commandLineParser(std::string_view localAddress,
                                                      bool enableConfig = true);
    /** set the desired interface type
     */
    void setInterfaceType(gmlc::networking::InterfaceTypes type) { allowedType = type; }

  private:
    /** do some checking on the brokerAddress*/
    void checkAndUpdateBrokerAddress(std::string_view localAddress);
    gmlc::networking::InterfaceTypes allowedType{gmlc::networking::InterfaceTypes::IP};
};

}  // namespace helics
