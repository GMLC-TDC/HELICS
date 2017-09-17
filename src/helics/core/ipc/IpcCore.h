/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_IPC_CORE_
#define _HELICS_IPC_CORE_
#pragma once

#include "core/CommonCore.h"

namespace helics {

class IpcComms;

/** implementation for the core that uses zmq messages to communicate*/
class IpcCore : public CommonCore {

public:
	/** default constructor*/
  IpcCore() noexcept;
  IpcCore(const std::string &core_name);
  /** destructor*/
  ~IpcCore();
  virtual void InitializeFromArgs (int argc, char *argv[]) override;
         
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
	std::unique_ptr<IpcComms> comms;	//!< object to handle the actual interprocess Connections
};


} // namespace helics
 
#endif /* _HELICS_ZEROMQ_CORE_ */
