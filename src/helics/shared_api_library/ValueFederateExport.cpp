/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics.h"
#include "core/helics-time.h"
#include "shared_api_library/internal/api_objects.h"
#include "application_api/application_api.h"
#include <memory>
#include <mutex>
#include <vector>

/* sub/pub registration */
helics_subscription helicsRegisterSubscription(helics_federate fedID, const char *name, const char *type, const char *units)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return nullptr;
	}
	try
	{
		auto subid = fed->registerRequiredSubscription(name, type, units);
		return nullptr;
	}
	catch (const helics::InvalidFunctionCall &)
	{
		return nullptr;
	}

}
helics_subscription helicsRegisterDoubleSubscription(helics_federate fedID, const char *name, const char *units)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return nullptr;
	}
	return nullptr;
}
helics_subscription helicsRegisterStringSubscription(helics_federate fedID, const char *name, const char *units)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return nullptr;
	}
	return nullptr;
}

helics_publication  helicsRegisterPublication(helics_federate fedID, const char *name, const char *type, const char *units)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return nullptr;
	}
	return nullptr;
}
helics_publication  helicsRegisterDoublePublication(helics_federate fedID, const char *name, const char *units)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return nullptr;
	}
	return nullptr;
}
helics_publication  helicsRegisterStringPublication(helics_federate fedID, const char *name, const char *units)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return nullptr;
	}
	return nullptr;
}
helics_publication  helicsRegisterGlobalPublication(helics_federate fedID, const char *name, const char *type, const char *units)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return nullptr;
	}
	return nullptr;
}
helics_publication  helicsRegisterGlobalDoublePublication(helics_federate fedID, const char *name, const char *units)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return nullptr;
	}
	return nullptr;
}
helics_publication  helicsRegisterGlobalStringPublication(helics_federate fedID, const char *name, const char *units)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return nullptr;
	}
	return nullptr;
}
