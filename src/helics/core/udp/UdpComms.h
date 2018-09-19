/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../CommsInterface.hpp"
#include "helics/helics-config.h"
#include <future>
#include <set>

#if (BOOST_VERSION_LEVEL >= 2)
namespace boost
{
namespace asio
{
class io_context;
using io_service = io_context;
}
}
#else
namespace boost
{
namespace asio
{
class io_service;
}
}
#endif
namespace helics
{
namespace udp {
/** implementation for the communication interface that uses ZMQ messages to communicate*/
class UdpComms final : public CommsInterface
{
  public:
    /** default constructor*/
    UdpComms ();
    /** destructor*/
    ~UdpComms ();
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
    std::atomic<bool> hasBroker{false};
    virtual void queue_rx_function () override;  //!< the functional loop for the receive queue
    virtual void queue_tx_function () override;  //!< the loop for transmitting data
    virtual void closeReceiver () override;  //!< function to instruct the receiver loop to close
    /** find an open port for a subBroker*/
    int findOpenPort ();
    /** process an incoming message
    return code for required action 0=NONE, -1 TERMINATE*/
    int processIncomingMessage (ActionMessage &cmd);
    ActionMessage generateReplyToIncomingMessage (ActionMessage &cmd);
    // promise and future for communicating port number from tx_thread to rx_thread
    std::promise<int> promisePort;
    std::future<int> futurePort;

  public:
    /** get the port number of the comms object to push message to*/
    int getPort () const { return PortNumber; };

    std::string getAddress () const;
};

} // namespace udp
}  // namespace helics

