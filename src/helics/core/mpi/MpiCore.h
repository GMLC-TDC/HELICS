/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_MPI_CORE_
#define _HELICS_MPI_CORE_
#pragma once

#include "../CommonCore.h"
#include "../CommsBroker.hpp"

namespace helics {

class MpiComms;

/** implementation for the core that uses zmq messages to communicate*/
class MpiCore final: public CommsBroker<MpiComms,CommonCore> {

public:
	/** default constructor*/
	MpiCore() noexcept;
	MpiCore(const std::string &core_name);
	/** destructor*/
	~MpiCore();
	virtual void initializeFromArgs(int argc, const char * const *argv) override;

public:
	virtual std::string getAddress() const override;
private:
	virtual bool brokerConnect() override;

	std::string fileloc; //!< the location of the file queue
	std::string brokerloc;	//!< the location of the broker	queue
	std::string brokername;	//!< the name of the broker
};


} // namespace helics

#endif /* _HELICS_MPI_CORE_ */
