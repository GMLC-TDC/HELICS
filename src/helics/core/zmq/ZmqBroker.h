/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef ZMQ_BROKER_H_
#define ZMQ_BROKER_H_
#pragma once

#include "core/CoreBroker.h"
#include "core/CommsBroker.hpp"

namespace helics
{

class ZmqComms;

class ZmqBroker final:public CommsBroker<ZmqComms,CoreBroker>
{
public:
	/** default constructor*/
	ZmqBroker(bool isRoot_ = false) noexcept;
	ZmqBroker(const std::string &broker_name);

	void InitializeFromArgs(int argc, const char * const *argv) override;

	/**destructor*/
	virtual ~ZmqBroker();

	virtual std::string getAddress() const override;
	static void displayHelp(bool local_only = false);
private:
	virtual bool brokerConnect() override;
	
	std::string brokerAddress;	//!< the protocol string for the broker location
	std::string localInterface; //!< the interface to use for the local receive ports
	int repPortNumber=-1;	//!< the port number for the reply port
	int pullPortNumber=-1;	//!< the port number for the pull port
	int brokerReqPort=-1;  //!< the port number to use for the broker priority request port
	int brokerPushPort=-1;  //!< the port number to use for the broker regular push port
	int portStart = -1;  //!< the starting port for automatic port definitions

};
}
#endif
