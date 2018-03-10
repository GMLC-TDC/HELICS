/*

Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#ifndef _HELICS_ZEROMQ_CORE_
#define _HELICS_ZEROMQ_CORE_
#pragma once

#include "../CommonCore.hpp"
#include "../CommsBroker.hpp"
#include "../NetworkBrokerData.hpp"
namespace helics {
namespace zeromq {
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

} // namespace zeromq
} // namespace helics

#endif /* _HELICS_ZEROMQ_CORE_ */

