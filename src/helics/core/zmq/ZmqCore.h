/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once
#include "../NetworkCore.hpp"
namespace helics {
namespace zeromq {
class ZmqComms;
/** implementation for the core that uses zmq messages to communicate*/
class ZmqCore final: public NetworkCore<ZmqComms, interface_type::tcp> {

public:
	/** default constructor*/
  ZmqCore() noexcept;
  /** construct from with a core name*/
  ZmqCore(const std::string &core_name);

private:
	virtual bool brokerConnect() override;

};

} // namespace zeromq
} // namespace helics


