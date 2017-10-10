/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_ZEROMQ_CORE_
#define _HELICS_ZEROMQ_CORE_
#pragma once

#include "core/CommonCore.h"

namespace helics {

class ZmqComms;
/** implementation for the core that uses zmq messages to communicate*/
class ZmqCore final: public CommonCore {

public:
	/** default constructor*/
  ZmqCore() noexcept;
  ZmqCore(const std::string &core_name);
  ~ZmqCore();
  virtual void InitializeFromArgs (int argc, char *argv[]) override;
         
  virtual void transmit(int route_id, const ActionMessage &cmd) override;
  virtual void addRoute(int route_id, const std::string &routeInfo) override;
public:
	virtual std::string getAddress() const override;
private:
	
	std::string brokerAddress;	//!< the protocol string for the broker location
	std::string localInterface; //!< the interface to use for the local receive ports
	int repPortNumber=-1;	//!< the port number for the reply port
	int pullPortNumber=-1;	//!< the port number for the pull port
	int brokerReqPort=-1;  //!< the port number to use for the broker priority request port
	int brokerPushPort=-1;  //!< the port number to use for the broker regular push port

	virtual bool brokerConnect() override;
	virtual void brokerDisconnect() override;

	std::unique_ptr<ZmqComms> comms; //!< object controlling the actual comm work
 
};


} // namespace helics
 
#endif /* _HELICS_ZEROMQ_CORE_ */
