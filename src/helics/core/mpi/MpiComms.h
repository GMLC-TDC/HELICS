/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
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
