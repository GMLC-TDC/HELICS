
/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/

#ifndef HELICS_API_OBJECTS_H_
#define HELICS_API_OBJECTS_H_
#pragma once

#include <memory>
#include "shared_api_library/api-data.h"

namespace helics
{
	class Core;
	class Federate;
	class CoreBroker;

	enum vtype
	{
		valueFed,
		messageFed,
		filterFed,
		combinFed,
	};


	class BrokerObject
	{
	public:
		std::shared_ptr<CoreBroker> brokerptr;
	};

	class coreObject
	{
	public:
		std::shared_ptr<Core> coreptr;
	};
	
	class FedObject
	{
	public:
		vtype type;
		std::shared_ptr<Federate> fedptr;
	};
}

#endif