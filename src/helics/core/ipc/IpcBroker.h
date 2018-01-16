/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef IPC_BROKER_H_
#define IPC_BROKER_H_
#pragma once

#include "../CoreBroker.hpp"
#include "../CommsBroker.hpp"

namespace helics
{

class IpcComms;

class IpcBroker final:public CommsBroker<IpcComms,CoreBroker>
{
public:
	/** default constructor*/
	IpcBroker(bool rootBroker = false) noexcept;
	IpcBroker(const std::string &broker_name);

	static void displayHelp(bool local_only = false);
	void initializeFromArgs(int argc, const char * const *argv) override;

	/**destructor*/
	virtual ~IpcBroker();

	virtual std::string getAddress() const override;
private:
	virtual bool brokerConnect() override;
private:
	std::string fileloc; //!< the name of the file that the shared memory queue is located
	std::string brokerloc; //!< the name of the shared queue of the broker
	std::string brokername; //!< the name of the broker
};
} //namespace helics
#endif
