/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "ValueFederate_c.h"
#include "core/helics-time.h"
#include "shared_api_library/internal/api_objects.h"
#include "application_api/application_api.h"
#include "application_api/Subscriptions.hpp"
#include "application_api/Publications.hpp"
#include <memory>
#include <mutex>
#include <vector>
#include <map>


/* sub/pub registration */
helics_subscription helicsRegisterSubscription(helics_value_federate fed, const char *name,  const char *type, const char *units)
{
	auto htype = helics::getTypeFromString(type);
	if (htype != helics::helicsType_t::helicsInvalid)
	{
		return helicsRegisterTypeSubscription(fed, static_cast<int>(htype), name, units);
	}
	//now generate a generic subscription
	auto fedObj = getValueFedSharedPtr(fed);
	if (!fedObj)
	{
		return nullptr;
	}
	helics::SubscriptionObject *sub = nullptr;
	try
	{
		
		sub = new helics::SubscriptionObject();
		sub->id = fedObj->registerOptionalSubscription( name ,type, units);
		sub->rawOnly = true;
		sub->fedptr = std::move(fedObj);
		return reinterpret_cast<helics_subscription>(sub);
	}
	catch (const helics::InvalidFunctionCall &)
	{
		if (sub != nullptr)
		{
			delete sub;
		}
	}
	return nullptr;

}
helics_subscription helicsRegisterTypeSubscription(helics_value_federate fed,int type, const char *name, const char *units)
{
	if ((type < 0) || (type > HELICS_VECTOR_TYPE))
	{
		if (type == HELICS_RAW_TYPE)
		{
			return helicsRegisterSubscription(fed, name, "", units);
		}
		return nullptr;
	}
	auto fedObj = getValueFedSharedPtr(fed);
	if (!fedObj)
	{
		return nullptr;
	}
	
	helics::SubscriptionObject *sub = nullptr;
	try
	{
		sub = new helics::SubscriptionObject();
		sub->subptr = std::make_shared<helics::Subscription>(fedObj.get(), name, units);
		sub->fedptr = std::move(fedObj);
		return reinterpret_cast<helics_subscription>(sub);
	}
	catch (const helics::InvalidFunctionCall &)
	{
		if (sub != nullptr)
		{
			delete sub;
		}
	}
	return nullptr;
}

helics_publication  helicsRegisterPublication(helics_value_federate fed, const char *name, const char *type, const char *units)
{
	auto htype = helics::getTypeFromString(type);
	if (htype != helics::helicsType_t::helicsInvalid)
	{
		return helicsRegisterTypePublication(fed, static_cast<int>(htype), name, units);
	}
	//now generate a generic subscription
	auto fedObj = getValueFedSharedPtr(fed);
	if (!fedObj)
	{
		return nullptr;
	}
	helics::PublicationObject *pub = nullptr;
	try
	{

		pub = new helics::PublicationObject();
		pub->id = fedObj->registerPublication(name, type, units);
		pub->rawOnly = true;
		pub->fedptr = std::move(fedObj);
		return reinterpret_cast<helics_publication>(pub);
	}
	catch (const helics::InvalidFunctionCall &)
	{
		if (pub != nullptr)
		{
			delete pub;
		}
	}
	return nullptr;
}
helics_publication  helicsRegisterTypePublication(helics_value_federate fed, int type, const char *name, const char *units)
{
	if ((type < 0) || (type > HELICS_VECTOR_TYPE))
	{
		if (type == HELICS_RAW_TYPE)
		{
			return helicsRegisterPublication(fed, name, "", units);
		}
		return nullptr;
	}
	auto fedObj = getValueFedSharedPtr(fed);
	if (!fedObj)
	{
		return nullptr;
	}
	helics::PublicationObject *pub = nullptr;
	try
	{
		pub = new helics::PublicationObject();
		pub->pubptr = std::make_shared<helics::Publication>(fedObj.get(), static_cast<helics::helicsType_t>(type), name, units);
		pub->fedptr = std::move(fedObj);
		return reinterpret_cast<helics_publication>(pub);
	}
	catch (const helics::InvalidFunctionCall &)
	{
		if (pub != nullptr)
		{
			delete pub;
		}
	}
	return nullptr;
}

helics_publication  helicsRegisterGlobalPublication(helics_value_federate fed, const char *name, const char *type, const char *units)
{
	auto htype = helics::getTypeFromString(type);
	if (htype != helics::helicsType_t::helicsInvalid)
	{
		return helicsRegisterGlobalTypePublication(fed, static_cast<int>(htype), name, units);
	}
	//now generate a generic subscription
	auto fedObj = getValueFedSharedPtr(fed);
	if (!fedObj)
	{
		return nullptr;
	}
	helics::PublicationObject *pub = nullptr;
	try
	{

		pub = new helics::PublicationObject();
		pub->id = fedObj->registerGlobalPublication(name, type, units);
		pub->rawOnly = true;
		pub->fedptr = std::move(fedObj);
		return reinterpret_cast<helics_publication>(pub);
	}
	catch (const helics::InvalidFunctionCall &)
	{
		if (pub != nullptr)
		{
			delete pub;
		}
	}
	return nullptr;
}

helics_publication  helicsRegisterGlobalTypePublication(helics_value_federate fed, int type, const char *name, const char *units)
{
	if ((type < 0) || (type > HELICS_VECTOR_TYPE))
	{
		if (type == HELICS_RAW_TYPE)
		{
			return helicsRegisterGlobalPublication(fed, name, "", units);
		}
		return nullptr;
	}
	auto fedObj = getValueFedSharedPtr(fed);
	if (!fedObj)
	{
		return nullptr;
	}
	helics::PublicationObject *pub = nullptr;
	try
	{
		pub = new helics::PublicationObject();
		pub->pubptr = std::make_shared<helics::Publication>(helics::GLOBAL,fedObj.get(), static_cast<helics::helicsType_t>(type),name, units);
		pub->fedptr = std::move(fedObj);
		return reinterpret_cast<helics_publication>(pub);
	}
	catch (const helics::InvalidFunctionCall &)
	{
		if (pub != nullptr)
		{
			delete pub;
		}
	}
	return nullptr;
}


/* getting and publishing values */
helicsStatus helicsPublish(helics_publication pubID, const char *data, uint64_t len)
{
	if (pubID == nullptr)
	{
		return helicsError;
	}
	auto pubObj = reinterpret_cast<helics::PublicationObject *>(pubID);
	if (pubObj->rawOnly)
	{
		pubObj->fedptr->publish(pubObj->id, data, len);
	}
	return helicsOK;
}
helicsStatus helicsPublishString(helics_publication pubID, const char *str)
{
	if (pubID == nullptr)
	{
		return helicsError;
	}
	auto pubObj = reinterpret_cast<helics::PublicationObject *>(pubID);
	if (pubObj->rawOnly)
	{
		pubObj->fedptr->publish(pubObj->id, str);
	}
	else
	{
		pubObj->pubptr->publish(str);
	}
	return helicsOK;
}
helicsStatus helicsPublishInteger(helics_publication pubID, int64_t val)
{
	return helicsError;
}
helicsStatus helicsPublishDouble(helics_publication pubID, double val)
{
	return helicsError;
}
helicsStatus helicsPublishComplex(helics_publication pubID, double real, double imag)
{
	return helicsError;
}
helicsStatus helicsPublishVector(helics_publication pubID, const double data[], uint64_t len)
{
	return helicsError;
}

uint64_t helicsGetValue(helics_subscription pubID, char *data, uint64_t maxlen)
{
	return helicsError;
}
helicsStatus helicsGetString(helics_subscription pubID, char *str, uint64_t maxlen)
{
	return helicsError;
}
helicsStatus helicsGetInteger(helics_subscription pubID, int64_t *val)
{
	return helicsError;
}
helicsStatus helicsGetDouble(helics_subscription pubID, double *val)
{
	return helicsError;
}
helicsStatus helicsGetComplex(helics_subscription pubID, double *real, double *imag)
{
	return helicsError;
}
helicsStatus helicsGetVector(helics_subscription pubID, double data[], uint64_t len)
{
	return helicsError;
}

uint64_t helicsSetDefaultValue(helics_subscription pubID, char *data, uint64_t maxlen)
{
	return helicsError;
}
helicsStatus helicsSetDefaultString(helics_subscription pubID, char *str, uint64_t maxlen)
{
	return helicsError;
}
helicsStatus helicsSetDefaultInteger(helics_subscription pubID, int64_t *val)
{
	return helicsError;
}
helicsStatus helicsSetDefaultDouble(helics_subscription pubID, double *val)
{
	return helicsError;
}
helicsStatus helicsSetDefaultComplex(helics_subscription pubID, double *real, double *imag)
{
	return helicsError;
}
helicsStatus helicsSetDefaultVector(helics_subscription pubID, double *data, uint64_t len)
{
	return helicsError;
}
