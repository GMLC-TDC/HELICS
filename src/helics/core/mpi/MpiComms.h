/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#pragma once

#include "../CommsInterface.hpp"
#include "../../common/BlockingQueue.hpp"
#include <atomic>
#include <set>
#include <string>
#include <future>
#include <vector>
#include <mutex>
#include "helics/helics-config.h"

#include <mpi.h>

namespace helics {

/** implementation for the communication interface that uses MPI to communicate*/
class MpiComms final:public CommsInterface {

public:
	/** default constructor*/
	MpiComms();
    MpiComms(const int &brokerRank);
	/** destructor*/
	~MpiComms();
private:
	int brokerRank = -1; //!< the mpi rank of the broker
    static int commRank; //!< the mpi rank of this comm object
    int commTag; //!< the mpi tag of this comm object

    static std::mutex mpiSerialMutex;
    static std::atomic<bool> mpiCommsExists;

    std::atomic<bool> shutdown = false;
    
    virtual void queue_rx_function() override;	//!< the functional loop for the receive queue
    virtual void queue_tx_function() override;  //!< the loop for transmitting data

    bool initMPI();

    /** process an incoming message
    return code for required action 0=NONE, -1 TERMINATE*/
    int processIncomingMessage(ActionMessage &cmd);
    ActionMessage generateReplyToIncomingMessage(ActionMessage &cmd);

    /** queue for pending incoming messages*/
    BlockingQueue<ActionMessage> rxMessageQueue;
    /** queue for pending outgoing messages*/
    BlockingQueue<ActionMessage> txMessageQueue;

    void serializeSendMPI(std::vector<char> message, int dest, int tag, MPI_Comm comm);
    std::vector<char> serializeReceiveMPI(int src, int tag, MPI_Comm comm);

	std::atomic<bool> hasBroker{ false };
    virtual void closeReceiver() override;  //!< function to instruct the receiver loop to close

    void setBrokerRank (const int &rank) { brokerRank = rank; }
public:
	static std::string getAddress();
};


} // namespace helics

