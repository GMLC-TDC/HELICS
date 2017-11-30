/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_UDP_COMMS_
#define _HELICS_UDP_COMMS_
#pragma once

#include "../CommsInterface.h"
#include <atomic>
#include <set>
#include <string>
#include <future>



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
class TcpComms final:public CommsInterface {

public:
	/** default constructor*/
	TcpComms();
	TcpComms(const std::string &brokerTarget, const std::string &localTarget);
	/** destructor*/
	~TcpComms();
	/** set the port numbers for the local ports*/
	void setBrokerPort(int brokerPortNumber);
	void setPortNumber(int localPortNumber);
	void setAutomaticPortStartPort(int startingPort);
private:
	int brokerPort = -1;
	std::atomic<int> PortNumber{ -1 };
	std::set<int> usedPortNumbers;
	int openPortStart = -1;
	std::atomic<bool> hasBroker{ false };
	virtual void queue_rx_function() override;	//!< the functional loop for the receive queue
	virtual void queue_tx_function() override;  //!< the loop for transmitting data
	virtual void closeTransmitter() override; //!< function to instruct the transmitter loop to close
	virtual void closeReceiver() override;  //!< function to instruct the receiver loop to close
	/** find an open port for a subBroker*/
	int findOpenPort();  
	/** process an incoming message
	return code for required action 0=NONE, -1 TERMINATE*/
	int processIncomingMessage(ActionMessage &cmd);
    ActionMessage generateReplyToIncomingMessage(ActionMessage &cmd);
    //promise and future for communicating port number from tx_thread to rx_thread
    std::promise<int> promisePort;
    std::future<int> futurePort;
public:
	/** get the port number of the comms object to push message to*/
	int getPort() const { return PortNumber; };

	std::string getAddress() const;
};


} // namespace helics

#endif /* _HELICS_UDP_COMMS_ */

