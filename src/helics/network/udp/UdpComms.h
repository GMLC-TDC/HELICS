/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../NetworkCommsInterface.hpp"
#include "helics/helics-config.h"

#include <future>
#include <set>

class AsioContextManager;
namespace asio {
class io_context;
}  // namespace asio

namespace helics {
namespace udp {
    /** implementation for the communication interface that uses ZMQ messages to communicate*/
    class UdpComms final: public NetworkCommsInterface {
      public:
        /** default constructor*/
        UdpComms();
        /** destructor*/
        ~UdpComms();

        virtual void loadNetworkInfo(const NetworkBrokerData& netInfo) override;

      private:
        virtual int getDefaultBrokerPort() const override;
        virtual void queue_rx_function() override;  //!< the functional loop for the receive queue
        virtual void queue_tx_function() override;  //!< the loop for transmitting data
        virtual void closeReceiver() override;  //!< function to instruct the receiver loop to close

        // promise and future for communicating port number from tx_thread to rx_thread
        std::promise<int> promisePort;
        std::future<int> futurePort;

      public:
    };

}  // namespace udp
}  // namespace helics
