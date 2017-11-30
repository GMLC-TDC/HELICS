/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef TCP_BROKER_H_
#define TCP_BROKER_H_
#pragma once

#include "../CoreBroker.h"
#include "../CommsBroker.hpp"

namespace helics
{

class TcpComms;

class TcpBroker final:public CommsBroker<TcpComms,CoreBroker>
{
public:
	/** default constructor*/
	TcpBroker(bool rootBroker = false) noexcept;
	TcpBroker(const std::string &broker_name);

	void InitializeFromArgs(int argc, const char * const *argv) override;

	/**destructor*/
	virtual ~TcpBroker();

	virtual std::string getAddress() const override;
	static void displayHelp(bool local_only = false);
private:
	virtual bool brokerConnect() override;
	
	std::string brokerAddress;	//!< the ip address or domain name of the broker
	std::string localInterface; //!< the interface to use for the local receive ports
	int PortNumber=-1;	//!< the port number for the reply port
	int brokerPort=-1;  //!< the port number to use for the broker priority request port
	int portStart = -1;  //!< the starting port for automatic port definitions

};
}
#endif
