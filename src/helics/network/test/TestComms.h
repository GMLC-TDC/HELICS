/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../CommsInterface.hpp"
#include "helics/helics-config.h"

#include <future>
#include <set>
#include <string>

namespace helics {
namespace testcore {
    /** implementation for the communication interface that uses ZMQ messages to communicate*/
    class TestComms final: public CommsInterface {
      public:
        /** default constructor*/
        TestComms();
        /** destructor*/
        ~TestComms();

        virtual void loadNetworkInfo(const NetworkBrokerData& netInfo) override;

      private:
        virtual void queue_rx_function() override;  //!< the functional loop for the receive queue
        virtual void queue_tx_function() override;  //!< the loop for transmitting data
      public:
        /** user function in the test comms to force the communication to stop immediately for
         * testing purposes*/
        void haltComms();
        /** pause the comms*/
        void pauseComms(bool paused = true);
        /** allow a specific number of messages to proceed*/
        void allowMessages(int messageCount);
        /** return a dummy port number*/
        int getPort() const { return -1; }

        std::string getAddress() const;
    };

}  // namespace testcore
}  // namespace helics
