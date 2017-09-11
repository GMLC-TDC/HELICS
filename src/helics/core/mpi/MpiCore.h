/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_MPI_CORE_
#define _HELICS_MPI_CORE_
#pragma once

#include "core/CommonCore.h"

namespace helics {

class MpiComms;

/** implementation for the core that uses zmq messages to communicate*/
class MpiCore : public CommonCore {

public:
	/** default constructor*/
	MpiCore() noexcept;
	MpiCore(const std::string &core_name);
	/** destructor*/
	~MpiCore();
	virtual void initializeFromArgs(int argc, char *argv[]) override;

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
	std::unique_ptr<MpiComms> comms;	//!< object to handle the actual interprocess Connections
};


} // namespace helics

#endif /* _HELICS_MPI_CORE_ */
