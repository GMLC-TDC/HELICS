/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef MPI_BROKER_H_
#define MPI_BROKER_H_
#pragma once

#include "../CoreBroker.h"
#include "../CommsBroker.hpp"

namespace helics
{

class MpiComms;

class MpiBroker final :public CommsBroker<MpiComms,CoreBroker>
{
public:
	/** default constructor*/
	MpiBroker(bool isRoot_ = false) noexcept;
	MpiBroker(const std::string &broker_name);

	void initializeFromArgs(int argc, const char * const *argv) override;

	/**destructor*/
	virtual ~MpiBroker();

	virtual std::string getAddress() const override;
	static void displayHelp(bool local_only = false);
private:
	virtual bool brokerConnect() override;

private:
};
}
#endif

