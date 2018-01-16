/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef UDP_BROKER_H_
#define UDP_BROKER_H_
#pragma once

#include "../CoreBroker.h"
#include "../CommsBroker.hpp"
#include "../NetworkBrokerData.h"

namespace helics
{

class UdpComms;

class UdpBroker final:public CommsBroker<UdpComms,CoreBroker>
{
public:
	/** default constructor*/
	UdpBroker(bool rootBroker = false) noexcept;
	UdpBroker(const std::string &broker_name);

	void initializeFromArgs(int argc, const char * const *argv) override;

	/**destructor*/
	virtual ~UdpBroker();

	virtual std::string getAddress() const override;
	static void displayHelp(bool local_only = false);
private:
	virtual bool brokerConnect() override;
    NetworkBrokerData netInfo{ NetworkBrokerData::interface_type::udp };  //!< structure containing the networking information

};
}
#endif
