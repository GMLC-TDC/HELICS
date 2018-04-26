/*

Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#ifndef ZMQ_BROKER_H_
#define ZMQ_BROKER_H_
#pragma once

#include "../CoreBroker.hpp"
#include "../CommsBroker.hpp"
#include "../NetworkBrokerData.hpp"

namespace helics
{
namespace zeromq {
class ZmqComms;

class ZmqBroker final:public CommsBroker<ZmqComms,CoreBroker>
{
public:
	/** default constructor*/
	ZmqBroker(bool rootBroker = false) noexcept;
	ZmqBroker(const std::string &broker_name);

	void initializeFromArgs(int argc, const char * const *argv) override;

	virtual std::string getAddress() const override;
	static void displayHelp(bool local_only = false);
private:
	virtual bool brokerConnect() override;

    NetworkBrokerData netInfo{ NetworkBrokerData::interface_type::tcp }; //!< container for the network connection information

};

} // namespace zeromq
} // namespace helics
#endif

