/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once

#include "../NetworkCommsInterface.hpp"
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
/** implementation for the communication interface that uses ZMQ messages to communicate
 * This is ROUTER-DEALER pattern */
class ZmqCommsTest final:public NetworkCommsInterface {

public:
	/** default constructor*/
	ZmqCommsTest() noexcept;
	/** destructor*/
	~ZmqCommsTest();
    /** load network information into the comms object*/
    virtual void loadNetworkInfo (const NetworkBrokerData &netInfo) override;
	/** set the port numbers for the local ports*/

private:
    virtual int getDefaultBrokerPort () const override;
	virtual void queue_rx_function() override;	//!< the functional loop for the receive queue
	virtual void queue_tx_function() override;  //!< the loop for transmitting data
	virtual void closeReceiver() override;  //!< function to instruct the receiver loop to close
	/** process an incoming message
	return code for required action 0=NONE, -1 TERMINATE*/
	int processIncomingMessage(zmq::message_t &msg,
			std::map<std::string, std::string> &connection_info);
	/** process an incoming message
		return code for required action TRUE=close connection, FALSE=continue*/
	bool processTxControlCmd(ActionMessage cmd,
			std::map<route_id_t, std::string> &routes,
			std::map<std::string, std::string> &connection_info);
    /** process an incoming message and send and ack in response
    return code for required action 0=NONE, -1 TERMINATE*/
	int replyToIncomingMessage(zmq::message_t &msg, zmq::socket_t &sock);

	int initializeBrokerConnections(zmq::socket_t &controlSocket);
public:
    
    std::string getPushAddress() const;
};

} // namespace zeromq
} // namespace helics


