/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../CommsInterface.hpp"

#include <atomic>
#include <string>

namespace helics {
namespace ipc {
    /** implementation for the core that uses boost interprocess messages to communicate*/
    class IpcComms final: public CommsInterface {
      public:
        /** default constructor*/
        IpcComms();
        /** destructor*/
        ~IpcComms();

        virtual void loadNetworkInfo(const NetworkBrokerData& netInfo) override;

      private:
        std::atomic<int> ipcbackchannel{
            0};  //!< a back channel message system if the primary is not working
        virtual void queue_rx_function() override;  //!< the functional loop for the receive queue
        virtual void queue_tx_function() override;  //!< the loop for transmitting data
        virtual void closeReceiver() override;  //!< function to instruct the receiver loop to close

      public:
        /** get the port number of the comms object to push message to*/
        int getPort() const { return -1; }

        std::string getAddress() const;
    };

#define IPC_BACKCHANNEL_TRY_RESET 2
#define IPC_BACKCHANNEL_DISCONNECT 4

}  // namespace ipc
}  // namespace helics
