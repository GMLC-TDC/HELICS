/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "CommsInterface.hpp"
#include "helics/helics-config.h"

#include <map>
#include <set>
#include <string>

namespace helics {
/** implementation for the communication interface that uses ZMQ messages to communicate*/
class NetworkCommsInterface: public CommsInterface {
  private:
    class PortAllocator {
      public:
        /** get an open port for a particular host*/
        int findOpenPort(int count, const std::string& host = "localhost");
        void setStartingPortNumber(int startPort) { startingPort = startPort; }
        int getDefaultStartingPort() const { return startingPort; }
        void addUsedPort(int port);
        void addUsedPort(const std::string& host, int port);

      private:
        int startingPort = -1;
        std::map<std::string, std::set<int>> usedPort;
        std::map<std::string, int> nextPorts;
        bool isPortUsed(const std::string& host, int port) const;
    };

  public:
    /** default constructor*/
    explicit NetworkCommsInterface(interface_type type,
                                   CommsInterface::thread_generation threads =
                                       CommsInterface::thread_generation::dual) noexcept;

    /** load network information into the comms interface object*/
    virtual void loadNetworkInfo(const NetworkBrokerData& netInfo) override;
    /** set the port numbers for the local ports*/
    void setBrokerPort(int brokerPortNumber);
    /** set the local port number to use for incoming connections*/
    void setPortNumber(int localPortNumber);
    /** get the local port number to use for incoming connections*/
    int getPortNumber() const { return PortNumber.load(); }
    /** set the automatic port numbering starting port*/
    void setAutomaticPortStartPort(int startingPort);
    /** set a flag on the communication system*/
    virtual void setFlag(const std::string& flag, bool val) override;

  protected:
    int brokerPort{-1};  //!< standardized broker port to use for connection to the brokers
    std::atomic<int> PortNumber{-1};  //!< port to use for the local connection
    bool autoPortNumber{true};  //!< use an automatic port numbering based on broker responses
    bool useOsPortAllocation{false};  //!< use the operating system to allocate a port number
    bool appendNameToAddress{false};  //!< flag to append the name to the network address
    bool noAckConnection{false};  //!< flag to bypass the connection acknowledge requirement
    const interface_type networkType;
    interface_networks network{interface_networks::ipv4};
    std::atomic<bool> hasBroker{false};
    int maxRetries{5};  // the maximum number of network retries

  private:
    PortAllocator openPorts;  //!< a structure to deal with port allocations

  public:
    /** find an open port for a subBroker*/
    int findOpenPort(int count, const std::string& host);
    /** for protocol messages some require an immediate reply from the comms interface itself*/
    ActionMessage generateReplyToIncomingMessage(ActionMessage& cmd);

  public:
    /** get the port number of the comms object to push message to*/
    int getPort() const { return PortNumber; }
    /** get the network address of the comms interface*/
    std::string getAddress() const;
    /** return the default Broker port*/
    virtual int getDefaultBrokerPort() const = 0;

  protected:
    ActionMessage generatePortRequest(int cnt = 1) const;
    void loadPortDefinitions(const ActionMessage& cmd);
};

}  // namespace helics
