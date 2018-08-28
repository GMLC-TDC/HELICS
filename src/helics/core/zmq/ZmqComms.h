/*

Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#ifndef _HELICS_ZMQ_COMMS_
#define _HELICS_ZMQ_COMMS_
#pragma once

#include "../CommsInterface.hpp"
#include <atomic>
#include <set>
#include <string>

namespace zmq
{
class message_t;
class socket_t;
}
namespace helics {
namespace zeromq {
/** implementation for the communication interface that uses ZMQ messages to communicate*/
class ZmqComms final:public CommsInterface {

public:
	/** default constructor*/
	ZmqComms() = default;
	ZmqComms(const std::string &brokerTarget, const std::string &localTarget, interface_networks targetNetwork = interface_networks::local);
    ZmqComms(const NetworkBrokerData &netInfo);
	/** destructor*/
	~ZmqComms();
	/** set the port numbers for the local ports*/
	void setBrokerPort(int brokerPort);
	void setPortNumber(int portNumber);
	void setAutomaticPortStartPort(int startingPort);
private:
	int brokerReqPort = -1;
	int brokerPushPort = -1;
	std::atomic<int> repPortNumber{ -1 };
	std::atomic<int> pullPortNumber{ -1 };

	std::set<int> usedPortNumbers;
	int openPortStart = -1;
	std::atomic<bool> hasBroker{ false };
    bool autoPortNumber = true;
	virtual void queue_rx_function() override;	//!< the functional loop for the receive queue
	virtual void queue_tx_function() override;  //!< the loop for transmitting data
	virtual void closeReceiver() override;  //!< function to instruct the receiver loop to close
	/** find two open ports for a subBroker*/
	std::pair<int, int> findOpenPorts();
	/** process an incoming message
	return code for required action 0=NONE, -1 TERMINATE*/
	int processIncomingMessage(zmq::message_t &msg);
    /** process an incoming message and send and ack in response
    return code for required action 0=NONE, -1 TERMINATE*/
	int replyToIncomingMessage(zmq::message_t &msg, zmq::socket_t &sock);

	int initializeBrokerConnections(zmq::socket_t &controlSocket);
public:
    /** get the port number of the comms object to push message to*/
    int getPort() const { return repPortNumber; };

	std::string getAddress() const;
    std::string getPushAddress() const;
};

} // namespace zeromq
} // namespace helics

#endif /* _HELICS_IPC_COMMS_ */

