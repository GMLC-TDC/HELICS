/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../NetworkCommsInterface.hpp"
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
class UdpComms final : public NetworkCommsInterface
{
  public:
    /** default constructor*/
    UdpComms ();
    /** destructor*/
    ~UdpComms ();

    virtual void loadNetworkInfo(const NetworkBrokerData &netInfo) override;
  private:
    virtual int getDefaultBrokerPort() const override;
    virtual void queue_rx_function () override;  //!< the functional loop for the receive queue
    virtual void queue_tx_function () override;  //!< the loop for transmitting data
    virtual void closeReceiver () override;  //!< function to instruct the receiver loop to close

    // promise and future for communicating port number from tx_thread to rx_thread
    std::promise<int> promisePort;
    std::future<int> futurePort;

  public:
};

} // namespace udp
}  // namespace helics

