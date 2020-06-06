/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC. See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../CommsInterface.hpp"
#include "gmlc/containers/BlockingQueue.hpp"
#include "helics/helics-config.h"

#include <atomic>
#include <future>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace helics {
namespace mpi {
    /** implementation for the communication interface that uses MPI to communicate*/
    class MpiComms final: public CommsInterface {
      public:
        /** default constructor*/
        MpiComms();
        /** destructor*/
        ~MpiComms();

      private:
        std::atomic<bool> shutdown{false};
        virtual void queue_rx_function() override;  //!< the functional loop for the receive queue
        virtual void queue_tx_function() override;  //!< the loop for transmitting data

        /** process an incoming message
    return code for required action 0=NONE, -1 TERMINATE*/
        int processIncomingMessage(ActionMessage& cmd);

        /** queue for pending incoming messages*/
        gmlc::containers::BlockingQueue<ActionMessage> rxMessageQueue;
        /** queue for pending outgoing messages*/
        gmlc::containers::BlockingQueue<std::pair<std::pair<int, int>, std::vector<char>>>
            txMessageQueue;

        std::atomic<bool> hasBroker{false};
        virtual void closeReceiver() override;  //!< function to instruct the receiver loop to close

      public:
        void setBrokerAddress(const std::string& address);

        std::string getAddress() { return localTargetAddress; }
        gmlc::containers::BlockingQueue<ActionMessage>& getRxMessageQueue()
        {
            return rxMessageQueue;
        }
        gmlc::containers::BlockingQueue<std::pair<std::pair<int, int>, std::vector<char>>>&
            getTxMessageQueue()
        {
            return txMessageQueue;
        }
    };

}  // namespace mpi
}  // namespace helics
