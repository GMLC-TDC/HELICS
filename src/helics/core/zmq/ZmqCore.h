/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_ZEROMQ_CORE_
#define _HELICS_ZEROMQ_CORE_
#pragma once

#include "../CommonCore.h"
#include "../CommsBroker.hpp"
#include "../NetworkBrokerData.h"
namespace helics {

class ZmqComms;
/** implementation for the core that uses zmq messages to communicate*/
class ZmqCore final: public CommsBroker<ZmqComms,CommonCore> {

public:
	/** default constructor*/
  ZmqCore() noexcept;
  /** construct from with a core name*/
  ZmqCore(const std::string &core_name);
  /** destructor*/
  ~ZmqCore();
  virtual void initializeFromArgs (int argc, const char * const *argv) override;
         
public:
	virtual std::string getAddress() const override;
private:
	
    NetworkBrokerData netInfo{ NetworkBrokerData::interface_type::tcp }; //!< container for the network connection information

	virtual bool brokerConnect() override;
 
};


} // namespace helics
 
#endif /* _HELICS_ZEROMQ_CORE_ */
