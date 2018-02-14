/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_MPI_COMMS_
#define _HELICS_MPI_COMMS_
#pragma once

#include "../CommsInterface.hpp"
#include <atomic>
#include <set>
#include <string>
#include <future>
#include <vector>
#include <mutex>
#include "helics/helics-config.h"

#include <mpi.h>

#if (BOOST_VERSION_LEVEL >=2)
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
namespace helics {

/** generate a string with a full address based on an interface string and port number
@details,  how things get merged depend on what interface is used some use port number some do not

@param[in] interface a string with an interface description i.e 127.0.0.1 
@param portNumber the number of the port to use
@return a string with the merged address
*/
std::string makePortAddress(const std::string &networkInterface, int portNumber);

/** extract a port number and interface string from an address number
@details,  if there is no port number it default to -1 this is true if none was listed
or the interface doesn't use port numbers

@param[in] address a string with an network location description i.e 127.0.0.1:34
@return a pair with a string and int with the interface name and port number
*/
std::pair<std::string, int> extractInterfaceandPort(const std::string &address);

/** implementation for the communication interface that uses ZMQ messages to communicate*/
class MpiComms final:public CommsInterface {

public:
	/** default constructor*/
	MpiComms();
    MpiComms(const int &brokerRank);
	/** destructor*/
	~MpiComms();
private:
	int brokerRank = -1; //!< the mpi rank of the broker

    static std::mutex mpiSerialMutex;
    static bool mpiCommsExists;

    bool shutdown = false;
    
    virtual void queue_rx_function() override;	//!< the functional loop for the receive queue
    virtual void queue_tx_function() override;  //!< the loop for transmitting data

    bool initMPI();

    /** process an incoming message
    return code for required action 0=NONE, -1 TERMINATE*/
    int processIncomingMessage(ActionMessage &cmd);
    ActionMessage generateReplyToIncomingMessage(ActionMessage &cmd);

    void serializeSendMPI(std::vector<char> message, int dest, int tag, MPI_Comm comm);
    std::vector<char> serializeReceiveMPI(int src, int tag, MPI_Comm comm);

	std::atomic<bool> hasBroker{ false };
    virtual void closeReceiver() override;  //!< function to instruct the receiver loop to close

    void setBrokerRank (const int &rank) { brokerRank = rank; }
public:
	static std::string getAddress();
};


} // namespace helics

#endif /* _HELICS_MPI_COMMS_ */

