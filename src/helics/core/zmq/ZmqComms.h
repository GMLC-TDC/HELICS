/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_IPC_COMMS_
#define _HELICS_IPC_COMMS_
#pragma once

#include "helics/core/CommsInterface.h"
#include <atomic>
#include <set>

namespace zmq
{
class message_t;
class socket_t;
}
namespace helics {

/** generate a string with a full address based on an interface string and port number
@details,  how things get merged depend on what interface is used some use port number some do not

@param[in] interface a string with an interface description i.e tcp://127.0.0.1 
@param portNumber the number of the port to use
@return a string with the merged address
*/
std::string makePortAddress(const std::string &interface, int portNumber);

/** extract a port number and interface string from an address number
@details,  if there is no port number it default to -1 this is true if none was listed
or the interface doesn't use port numbers

@param[in] address a string with an network location description i.e tcp://127.0.0.1:34
@return a pair with a string and int with the interface name and port number
*/
std::pair<std::string, int> extractInterfaceandPort(const std::string &address);

/** implementation for the communication interface that uses zmq messages to communicate*/
class ZmqComms:public CommsInterface {

public:
	/** default constructor*/
	ZmqComms() = default;
	ZmqComms(const std::string &brokerTarget, const std::string &localTarget);
	/** destructor*/
	~ZmqComms();
	/** set the portnumbers for the local ports*/
	void setBrokerPorts(int reqPort, int pushPort=-1);
	void setPortNumbers(int repPort, int pullPort=-1);
	void setAutomaticPortStartPort(int startingPort);
	void setReplyCallback(std::function<ActionMessage(ActionMessage &&)> callback);
private:
	int brokerReqPort = -1;
	int brokerPushPort = -1;
	std::atomic<int> repPortNumber{ -1 };
	std::atomic<int> pullPortNumber{ -1 };

	std::set<int> usedPortNumbers;
	int openPortStart = -1;
	std::atomic<bool> hasBroker{ false };
	std::function<ActionMessage(ActionMessage &&)> replyCallback;
	virtual void queue_rx_function() override;	//!< the functional loop for the receive queue
	virtual void queue_tx_function() override;  //!< the loop for transmitting data
	virtual void closeTransmitter() override; //!< function to instruct the transmitter loop to close
	virtual void closeReceiver() override;  //!< function to instruct the receiver loop to close
	/** find two open ports for a subBroker*/
	std::pair<int, int> findOpenPorts();  
	/** process an incoming message
	return code for required action 0=NONE, -1 TERMINATE*/
	int processIncomingMessage(zmq::message_t &msg);

	int replyToIncomingMessage(zmq::message_t &msg, zmq::socket_t &rep);

	int initializeBrokerConnections(zmq::socket_t &controlSocket);
public:
	/** get the port number of the comms object to send requests to*/
	int getRequestPort() const { return repPortNumber; };
	/** get the port number of the comms object to push message to*/
	int getPushPort() const { return pullPortNumber; };

	std::string getPushAddress() const;
	std::string getRequestAddress() const;
};


} // namespace helics

#endif /* _HELICS_IPC_COMMS_ */

