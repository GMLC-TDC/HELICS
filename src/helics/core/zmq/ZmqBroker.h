/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef ZMQ_BROKER_H_
#define ZMQ_BROKER_H_
#pragma once

#include "../CoreBroker.h"
#include "../CommsBroker.hpp"
#include "../NetworkBrokerData.h"

namespace helics
{

class ZmqComms;

class ZmqBroker final:public CommsBroker<ZmqComms,CoreBroker>
{
public:
	/** default constructor*/
	ZmqBroker(bool rootBroker = false) noexcept;
	ZmqBroker(const std::string &broker_name);

	void initializeFromArgs(int argc, const char * const *argv) override;

	/**destructor*/
	virtual ~ZmqBroker();

	virtual std::string getAddress() const override;
	static void displayHelp(bool local_only = false);
private:
	virtual bool brokerConnect() override;
	
    NetworkBrokerData netInfo{ NetworkBrokerData::interface_type::tcp }; //!< container for the network connection information

};
}
#endif
