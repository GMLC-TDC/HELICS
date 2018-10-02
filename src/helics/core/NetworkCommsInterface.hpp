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
  public:
    /** default constructor*/
	  NetworkCommsInterface();
    /** destructor*/
    ~NetworkCommsInterface();
    /** load network information into the comms object*/
    virtual void loadNetworkInfo (const NetworkBrokerData &netInfo) override;
    /** set the port numbers for the local ports*/
    void setBrokerPort (int brokerPortNumber);
    void setPortNumber (int localPortNumber);
    void setAutomaticPortStartPort (int startingPort);

  private:
    int brokerPort = -1;
    std::atomic<int> PortNumber{-1};
    std::set<int> usedPortNumbers;
    int openPortStart = -1;
    bool autoPortNumber = true;
	interface_networks network = interface_networks::ipv4;
    std::atomic<bool> hasBroker{false};

    /** find an open port for a subBroker*/
    int findOpenPort ();
	/** for protocol messages some require an immediate reply from the comms interface itself*/
    ActionMessage generateReplyToIncomingMessage (ActionMessage &cmd);
    // promise and future for communicating port number from tx_thread to rx_thread

  public:
    /** get the port number of the comms object to push message to*/
    int getPort () const { return PortNumber; };

    std::string getAddress () const;
    virtual int getDefaultBrokerPort () const = 0;
};

}  // namespace helics

