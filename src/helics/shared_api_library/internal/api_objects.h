
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
	class ValueFederate;
	class MessageFederate;
	class MessageFilterFederate;

	enum class vtype:int
	{
		genericFed,
		valueFed,
		messageFed,
		filterFed,
		combinFed,
	};

	/** object wrapping a broker for the c-api*/
	class BrokerObject
	{
	public:
		std::shared_ptr<CoreBroker> brokerptr;
	};

	/** object wrapping a core for the c-api*/
	class coreObject
	{
	public:
		std::shared_ptr<Core> coreptr;
	};
	
	/** object wrapping a federate for the c-api*/
	class FedObject
	{
	public:
		vtype type;
		int valid;
		std::shared_ptr<Federate> fedptr;
	};

	
}

helics::Federate *getFed(helics_federate fed);
helics::ValueFederate *getValueFed(helics_value_federate fed);
helics::MessageFederate *getMessageFed(helics_message_federate fed);
helics::MessageFilterFederate *getFilterFed(helics_message_filter_federate fed);

#endif