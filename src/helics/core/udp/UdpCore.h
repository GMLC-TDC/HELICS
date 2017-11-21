/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_UDP_CORE_
#define _HELICS_UDP_CORE_
#pragma once

#include "../CommonCore.h"
#include "../CommsBroker.hpp"

namespace helics {

class UdpComms;
/** implementation for the core that uses udp messages to communicate*/
class UdpCore final: public CommsBroker<UdpComms,CommonCore> {

public:
	/** default constructor*/
  UdpCore() noexcept;
  UdpCore(const std::string &core_name);
  ~UdpCore();
  virtual void InitializeFromArgs (int argc, const char * const *argv) override;
         
public:
	virtual std::string getAddress() const override;
private:
	
	std::string brokerAddress;	//!< the protocol string for the broker location
	std::string localInterface; //!< the interface to use for the local receive ports
	int PortNumber=-1;	//!< the port number for the reply port
	int brokerPortNumber=-1;  //!< the port number to use for the broker priority request port
	virtual bool brokerConnect() override;
 
};


} // namespace helics
 
#endif /* _HELICS_UDP_CORE_ */
