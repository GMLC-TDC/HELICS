/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "CommsInterface.hpp"
#include "helics/helics-config.h"


namespace helics
{

/** implementation for the communication interface that uses ZMQ messages to communicate*/
class NetworkCommsInterface : public CommsInterface
{
private:

    class PortAllocator
    {
    public:
        /** get an open port for a particular host*/
        int findOpenPort(int count, const std::string &host="localhost");
        void setStartingPortNumber(int startPort) { startingPort = startPort; }
        int getDefaultStartingPort() const { return startingPort; }
        void addUsedPort(int port);
        void addUsedPort(const std::string &host, int port);
    private:
        int startingPort=-1;
        std::map<std::string, std::set<int>> usedPort;
        std::map<std::string, int> nextPorts;
        bool isPortUsed(const std::string &host, int port) const;
    };

  public:
    /** default constructor*/
	  explicit NetworkCommsInterface(interface_type type) noexcept;

    /** load network information into the comms interface object*/
    virtual void loadNetworkInfo (const NetworkBrokerData &netInfo) override;
    /** set the port numbers for the local ports*/
    void setBrokerPort (int brokerPortNumber);
    void setPortNumber (int localPortNumber);
    void setAutomaticPortStartPort (int startingPort);

  protected:
    int brokerPort = -1;
    std::atomic<int> PortNumber{-1};
    bool autoPortNumber = true;
    bool useOsPortAllocation = false;
    const interface_type networkType;
	interface_networks network = interface_networks::ipv4;
    std::atomic<bool> hasBroker{false};
private:
    PortAllocator openPorts;
public:
    /** find an open port for a subBroker*/
    int findOpenPort (int count, const std::string &host);
	/** for protocol messages some require an immediate reply from the comms interface itself*/
    ActionMessage generateReplyToIncomingMessage (ActionMessage &cmd);
    // promise and future for communicating port number from tx_thread to rx_thread

  public:
    /** get the port number of the comms object to push message to*/
    int getPort () const { return PortNumber; };

    std::string getAddress () const;
    /** return the default Broker port*/
    virtual int getDefaultBrokerPort () const = 0;
protected:
    ActionMessage generatePortRequest(int cnt=1) const;
    void loadPortDefinitions(const ActionMessage &M);
};



}  // namespace helics

