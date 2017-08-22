/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef IPC_BROKER_H_
#define IPC_BROKER_H_
#pragma once

#include "core/CoreBroker.h"
#include <boost/interprocess/ipc/message_queue.hpp>

namespace helics
{

class IpcBroker :public CoreBroker
{
public:
	/** default constructor*/
	IpcBroker(bool isRoot_ = false) noexcept;
	IpcBroker(const std::string &broker_name);

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

	std::unique_ptr<boost::interprocess::message_queue> rxQueue;
	std::unique_ptr<boost::interprocess::message_queue> brokerQueue; //send queue for the higher level broker

	std::map<int, std::unique_ptr<boost::interprocess::message_queue>> routes;

	std::thread queue_watcher;
	void queue_rx_function();
};
}
#endif
