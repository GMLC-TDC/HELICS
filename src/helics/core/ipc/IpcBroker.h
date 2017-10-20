/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef IPC_BROKER_H_
#define IPC_BROKER_H_
#pragma once

#include "core/CoreBroker.h"

namespace helics
{

class IpcComms;

class IpcBroker :public CoreBroker
{
public:
	/** default constructor*/
	IpcBroker(bool rootBroker = false) noexcept;
	IpcBroker(const std::string &broker_name);

	static void displayHelp(bool local_only = false);
	void InitializeFromArgs(int argc, char *argv[]) override;

	/**destructor*/
	virtual ~IpcBroker();
	virtual void transmit(int32_t route, const ActionMessage &command) override;

	virtual void addRoute(int route_id, const std::string &routeInfo) override;

	virtual std::string getAddress() const override;
private:
	virtual bool brokerConnect() override;
	virtual void brokerDisconnect() override;
private:
	std::atomic<bool> initialized_{ false };  //!< atomic protecting local initialization
	
	std::string fileloc;
	std::string brokerloc;
	std::string brokername;
	std::unique_ptr<IpcComms> comms;
	mutable std::mutex dataMutex;  //mutex protecting the other information in the ipcBroker
};
} //namespace helics
#endif
