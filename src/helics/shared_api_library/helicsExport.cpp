/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics.h"
#include "core/helics-time.h"
#include "application_api/application_api.h"
#include <memory>
#include <mutex>
#include <vector>

const char *helicsGetVersion (void) { return HelicsVersion; }

static std::vector<std::shared_ptr<helics::Federate>> federates;



static std::vector<vtype> fedTypes;

static std::mutex helicsLock;  //!< lock for allowing multi-threaded access


helics_time_t helicsTimeFromDouble(double time)
{
	helics::Time val(time);
	return val.getBaseTimeCode();
 }

double doubleFromHelicsTime(helics_time_t time)
{
	helics::Time val;
	val.setBaseTimeCode(time);
	return static_cast<double>(val);
 }

/** this needs to preserve lifetime for multithreaded purposes*/
std::shared_ptr<helics::Federate> getFed(helics_federate fedID)
{
	std::lock_guard<std::mutex> lock(helicsLock);
	if (fedID < federates.size())
	{
		return federates[fedID];
	}
	return nullptr;
}

helics::ValueFederate *getValueFed(helics_federate fedID)
{
	if (fedID < federates.size())
	{
		std::lock_guard<std::mutex> lock(helicsLock);
		if ((fedTypes[fedID] == valueFed) || (fedTypes[fedID] == combinFed))
		{
			//need to use dynamic cast here due to virtual inheritance
			return dynamic_cast<helics::ValueFederate *>(federates[fedID].get());
		}
		
	}
	return nullptr;
}

helics::MessageFederate * getMessageFed(helics_federate fedID)
{
	std::lock_guard<std::mutex> lock(helicsLock);
	if (fedID < federates.size())
	{
		if (fedTypes[fedID] != valueFed)
		{
			return dynamic_cast<helics::MessageFederate *>(federates[fedID].get());
		}
	}
	return nullptr;
}

helics::MessageFilterFederate * getFilterFed(helics_federate fedID)
{
	std::lock_guard<std::mutex> lock(helicsLock);
	if (fedID < federates.size())
	{
		if (fedTypes[fedID] == filterFed)
		{
			return dynamic_cast<helics::MessageFilterFederate *>(federates[fedID].get());
		}
	}
	return nullptr;
}

/* Creation and destruction of Federates */
helics_federate helicsCreateValueFederate(federate_info_t *fi)
{
	return nullptr;
 }

helics_federate helicsCreateValueFederateFromFile(const char *file)
{
	auto fed = std::make_shared<helics::ValueFederate>(file);
	std::lock_guard<std::mutex> lock(helicsLock);
	auto id = federates.size();
	federates.push_back(std::move(fed));
	fedTypes.push_back(valueFed);
	return static_cast<helics_federate>(id);
 }

void helicsInitializeFederateInfo(federate_info_t *fi)
{
	fi->coreInitString = "";
	fi->coreName = "";
	fi->coreType = "test";
	fi->forwardCompute = false;
	fi->impactWindow = 0;
	fi->timeAgnostic = false;
	fi->interruptible = true;
	fi->lookAhead = 0;
	fi->obeserver = false;
	fi->timeAgnostic = false;
	fi->timeDelta = 0;
	fi->period = 0;
	fi->offset = 0;
 }

helicsStatus helicsFinalize(helics_federate fedID)
{
	auto fed = getFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	fed->finalize();
	
	return helicsOK;
 }

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
		auto subid=fed->registerRequiredSubscription(name, type, units);
		return nullptr;
	}
	catch (const helics::InvalidFunctionCall &)
	{
		return nullptr;
	}
	
 }
helics_subscription helicsRegisterDoubleSubscription(helics_federate fedID,const char *name, const char *units)
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
/* initialization, execution, and time requests */
helicsStatus helicsEnterInitializationMode(helics_federate fedID)
{
	auto fed=getFed(fedID);
	try
	{
		fed->enterInitializationState();
		return helicsOK;
	}
	catch (helics::InvalidStateTransition &)
	{
		return helicsError;
	}
 }

helicsStatus helicsEnterExecutionMode(helics_federate fedID)
{
	auto fed = getFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	try
	{
		fed->enterExecutionState();
		return helicsOK;
	}
	catch (helics::InvalidStateTransition &)
	{
		return helicsError;
	}
 }

helics_time_t helicsRequestTime(helics_federate fedID, helics_time_t requestTime)
{
	auto fed = getFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return requestTime;
 }

/* getting and publishing values */
helicsStatus helicsPublish(helics_federate fedID, helics_publication pubID, const char *data, uint64_t len)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return helicsOK;
 }

helicsStatus helicsPublishString(helics_federate fedID, helics_publication pubID, const char *str)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return helicsOK;
 }

helicsStatus helicsPublishDouble(helics_federate fedID, helics_publication pubID, double val)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return helicsOK;
 }

uint64_t helicsGetValue(helics_federate fedID, helics_publication pubID, char *data, uint64_t maxlen)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return 0;
 }

helicsStatus helicsGetString(helics_federate fedID, helics_publication pubID, char *str, uint64_t maxlen)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return helicsOK;
 }

helicsStatus helicsGetDouble(helics_federate fedID, helics_publication pubID, double *val)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return helicsOK;
 }