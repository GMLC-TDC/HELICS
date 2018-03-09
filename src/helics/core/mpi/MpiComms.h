/*
Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../../common/BlockingQueue.hpp"
#include "../CommsInterface.hpp"
#include "helics/helics-config.h"
#include <atomic>
#include <future>
#include <mutex>
#include <set>
#include <string>
#include <vector>

namespace helics
{
namespace mpi
{
/** implementation for the communication interface that uses MPI to communicate*/
class MpiComms final : public CommsInterface
{
  public:
    /** default constructor*/
    MpiComms ();
    MpiComms (const std::string &brokerAddress);
    /** destructor*/
    ~MpiComms ();

  private:
    std::string brokerAddress;  //!< the mpi rank:tag of the broker
    std::string commAddress;  //!< the mpi rank:tag of this comm object

    std::atomic<bool> shutdown;

    virtual void queue_rx_function () override;  //!< the functional loop for the receive queue
    virtual void queue_tx_function () override;  //!< the loop for transmitting data

    /** process an incoming message
    return code for required action 0=NONE, -1 TERMINATE*/
    int processIncomingMessage (ActionMessage &cmd);

    /** queue for pending incoming messages*/
    BlockingQueue<ActionMessage> rxMessageQueue;
    /** queue for pending outgoing messages*/
    BlockingQueue<std::pair<std::string, std::vector<char>>> txMessageQueue;

    std::atomic<bool> hasBroker{false};
    virtual void closeReceiver () override;  //!< function to instruct the receiver loop to close

    void setBrokerAddress (const std::string &address) { brokerAddress = address; }

  public:
    std::string getAddress () { return commAddress; }
    BlockingQueue<ActionMessage> &getRxMessageQueue () { return rxMessageQueue; }
    BlockingQueue<std::pair<std::string, std::vector<char>>> &getTxMessageQueue () { return txMessageQueue; }
};

} // namespace mpi
}  // namespace helics

