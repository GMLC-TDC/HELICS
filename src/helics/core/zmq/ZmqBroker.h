/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef ZMQ_BROKER_H_
#define ZMQ_BROKER_H_
#pragma once

#include "core/CoreBroker.h"

namespace helics
{

class ZmqComms;

class ZmqBroker final:public CoreBroker
{
public:
	/** default constructor*/
	ZmqBroker(bool isRoot_ = false) noexcept;
	ZmqBroker(const std::string &broker_name);

	void InitializeFromArgs(int argc, const char * const *argv) override;

	/**destructor*/
	virtual ~ZmqBroker();
	virtual void transmit(int32_t route, const ActionMessage &command) override;

	virtual void addRoute(int route_id, const std::string &routeInfo) override;

	virtual std::string getAddress() const override;
	static void displayHelp(bool local_only = false);
private:
	virtual bool brokerConnect() override;
	virtual void brokerDisconnect() override;
	

	std::string brokerAddress;	//!< the protocol string for the broker location
	std::string localInterface; //!< the interface to use for the local receive ports
	int repPortNumber=-1;	//!< the port number for the reply port
	int pullPortNumber=-1;	//!< the port number for the pull port
	int brokerReqPort=-1;  //!< the port number to use for the broker priority request port
	int brokerPushPort=-1;  //!< the port number to use for the broker regular push port
	int portStart = -1;  //!< the starting port for automatic port definitions
private:
	std::atomic<bool> initialized_{ false };  //!< atomic protecting local initialization
	std::unique_ptr<ZmqComms> comms;  //!< pointer to the comms object handling the actions connection
	mutable std::mutex dataLock;  //mutex protecting the local information
	//std::unique_ptr<ZmqConnection> zmqConn;  //!< object containing the ZmqConnection Information for Pimpl 
};
}
#endif
