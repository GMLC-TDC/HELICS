/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_ZEROMQ_CORE_
#define _HELICS_ZEROMQ_CORE_
#pragma once

#include "core/CommonCore.h"

#define DEFAULT_BROKER_PORT 23405

namespace helics {

class ZmqComms;
/** implementation for the core that uses zmq messages to communicate*/
class ZmqCore : public CommonCore {

public:
	/** default constructor*/
  ZmqCore() noexcept;
  ZmqCore(const std::string &core_name);
  ~ZmqCore();
  virtual void initializeFromArgs (int argc, char *argv[]) override;
         
  virtual void transmit(int route_id, const ActionMessage &cmd) override;
  virtual void addRoute(int route_id, const std::string &routeInfo) override;
public:
	virtual std::string getAddress() const override;
private:
	std::string pullSocketAddress;
	std::string brokerAddress;
	int repPortNumber;
	int pullPortNumber;
	std::string brokerRepAddress;

	virtual bool brokerConnect() override;
	virtual void brokerDisconnect() override;

	std::unique_ptr<ZmqComms> comms;
 
};


} // namespace helics
 
#endif /* _HELICS_ZEROMQ_CORE_ */
