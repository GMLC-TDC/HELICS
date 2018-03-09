/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../CommsInterface.hpp"
#include <atomic>

namespace helics
{
namespace ipc
{
/** implementation for the core that uses boost interprocess messages to communicate*/
class IpcComms final : public CommsInterface
{
  public:
    /** default constructor*/
    IpcComms () = default;
    IpcComms (const std::string &brokerTarget, const std::string &localTarget);
    /** destructor*/
    ~IpcComms ();

  private:
    std::atomic<int> ipcbackchannel{0};  //!< a back channel message system if the primary is not working

    virtual void queue_rx_function () override;  //!< the functional loop for the receive queue
    virtual void queue_tx_function () override;  //!< the loop for transmitting data
    virtual void closeReceiver () override;  //!< function to instruct the receiver loop to close

  private:
};

#define IPC_BACKCHANNEL_TRY_RESET 2
#define IPC_BACKCHANNEL_DISCONNECT 4

}  // namespace ipc
}  // namespace helics

