/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../../common/BlockingQueue.hpp"
#include "../CommsInterface.hpp"
#include <atomic>
#include <set>
#include <string>

#if (BOOST_VERSION_LEVEL >= 2)
namespace boost
{
namespace asio
{
class io_context;
using io_service = io_context;
}  // namespace asio
}  // namespace boost
#else
namespace boost
{
namespace asio
{
class io_service;
}
}  // namespace boost
#endif

namespace boost
{
namespace system
{
class error_code;
}
}  // namespace boost

namespace helics
{
namespace tcp
{
class TcpConnection;

/** implementation for the communication interface that uses TCP messages to communicate*/
class TcpComms final : public CommsInterface
{
  public:
    /** default constructor*/
    TcpComms () noexcept;
    TcpComms (const std::string &brokerTarget,
              const std::string &localTarget,
              interface_networks targetNetwork = interface_networks::local);
    TcpComms (const NetworkBrokerData &netInfo);
    /** destructor*/
    ~TcpComms ();
    /** set the port numbers for the local ports*/
    void setBrokerPort (int brokerPortNumber);
    void setPortNumber (int localPortNumber);
    void setAutomaticPortStartPort (int startingPort);

  private:
    int brokerPort = -1;
    bool autoPortNumber = true;
    bool reuse_address = false;
    std::atomic<int> PortNumber{-1};
    std::set<int> usedPortNumbers;
    int openPortStart = -1;
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
    BlockingQueue<ActionMessage> rxMessageQueue;

    void txReceive (const char *data, size_t bytes_received, const std::string &errorMessage);

    void txPriorityReceive (std::shared_ptr<TcpConnection> connection,
                            const char *data,
                            size_t bytes_received,
                            const boost::system::error_code &error);
    /** callback function for receiving data asynchronously from the socket
    @param connection pointer to the connection
    @param data the pointer to the data
    @param bytes_received the length of the received data
    @return a the number of bytes used by the function
    */
    size_t dataReceive (std::shared_ptr<TcpConnection> connection, const char *data, size_t bytes_received);

    bool commErrorHandler (std::shared_ptr<TcpConnection> connection, const boost::system::error_code &error);
    //  bool errorHandle()
  public:
    /** get the port number of the comms object to push message to*/
    int getPort () const { return PortNumber; };

    std::string getAddress () const;
};

}  // namespace tcp
}  // namespace helics
