/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_IPC_CORE_
#define _HELICS_IPC_CORE_
#pragma once

#include "core/CommonCore.h"

#include <boost/interprocess/ipc/message_queue.hpp>

namespace helics {

/** implementation for the core that uses zmq messages to communicate*/
class IpcCore : public CommonCore {

public:
	/** default constructor*/
  IpcCore()=default;
  IpcCore(const std::string &core_name);
  /** destructor*/
  ~IpcCore();
  virtual void initializeFromArgs (int argc, char *argv[]) override;
         
  virtual void transmit(int route_id, const ActionMessage &cmd) override;
  virtual void addRoute(int route_id, const std::string &routeInfo) override;
public:
	virtual std::string getAddress() const override;
private:
	virtual bool brokerConnect() override;
	virtual void brokerDisconnect() override;
 
	std::string fileloc; //!< the location of the file queue
	std::string brokerloc;	//!< the location of the broker	queue
	std::string brokername;	//!< the name of the broker

	std::unique_ptr<boost::interprocess::message_queue> rxQueue; //!< the receive queue
	std::unique_ptr<boost::interprocess::message_queue> brokerQueue;	//!< the queue of the broker

	std::map<int, std::unique_ptr<boost::interprocess::message_queue>> routes; //!< table of the routes to other brokers

	std::thread queue_watcher; //!< thread monitoring the receive queue
	void queue_rx_function();	//!< the functional loop for the receive queue
};


} // namespace helics
 
#endif /* _HELICS_ZEROMQ_CORE_ */
