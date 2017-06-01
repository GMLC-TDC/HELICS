/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil;  eval: (c-set-offset 'innamespace 0); -*- */
/*
 * LLNS Copyright Start
 * Copyright (c) 2017, Lawrence Livermore National Security
 * This work was performed under the auspices of the U.S. Department
 * of Energy by Lawrence Livermore National Laboratory in part under
 * Contract W-7405-Eng-48 and in part under Contract DE-AC52-07NA27344.
 * Produced at the Lawrence Livermore National Laboratory.
 * All rights reserved.
 * For details, see the LICENSE file.
 * LLNS Copyright End
 */

#include "helics.h"
#include "core/helics-time.h"
#include "application_api/application_api.h"
#include <memory>
#include <mutex>
#include <vector>

const char *helicsGetVersion (void) { return HelicsVersion; }

static std::vector<std::shared_ptr<helics::Federate>> federates;

enum vtype
{
	valueFed,
	messageFed,
	filterFed,
	combinFed,
};

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
std::shared_ptr<helics::Federate> getFed(helics_federate_id_t fedID)
{
	if (fedID < federates.size())
	{
		std::lock_guard<std::mutex> lock(helicsLock);
		return federates[fedID];
	}
	return nullptr;
}

std::shared_ptr<helics::ValueFederate> getValueFed(helics_federate_id_t fedID)
{
	if (fedID < federates.size())
	{
		std::lock_guard<std::mutex> lock(helicsLock);
		if ((fedTypes[fedID] == valueFed) || (fedTypes[fedID] == combinFed))
		{
			return std::dynamic_pointer_cast<helics::ValueFederate>(federates[fedID]);
		}
		
	}
	return nullptr;
}

std::shared_ptr<helics::MessageFederate> getMessageFed(helics_federate_id_t fedID)
{
	if (fedID < federates.size())
	{
		std::lock_guard<std::mutex> lock(helicsLock);
		if (fedTypes[fedID] != valueFed)
		{
			return std::dynamic_pointer_cast<helics::MessageFederate>(federates[fedID]);
		}
	}
	return nullptr;
}

std::shared_ptr<helics::MessageFilterFederate> getFilterFed(helics_federate_id_t fedID)
{
	if (fedID < federates.size())
	{
		std::lock_guard<std::mutex> lock(helicsLock);
		if (fedTypes[fedID] == filterFed)
		{
			return std::dynamic_pointer_cast<helics::MessageFilterFederate>(federates[fedID]);
		}
	}
	return nullptr;
}

/* Creation and destruction of Federates */
helics_federate_id_t helicsCreateValueFederate(federate_info_t *fi)
{
	return invalid_federate_id;
 }

helics_federate_id_t helicsCreateValueFederateFromFile(const char *file)
{
	auto fed = std::make_shared<helics::ValueFederate>(file);
	std::lock_guard<std::mutex> lock(helicsLock);
	auto id = federates.size();
	federates.push_back(std::move(fed));
	fedTypes.push_back(valueFed);
	return static_cast<helics_federate_id_t>(id);
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
 }

helicsStatus helicsFinalize(helics_federate_id_t fedID)
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
helics_subscription_id_t helicsRegisterSubscription(helics_federate_id_t fedID, const char *name, const char *type, const char *units)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	try
	{
		auto subid=fed->registerRequiredSubscription(name, type, units);
		return subid.value();
	}
	catch (const helics::InvalidFunctionCall &)
	{
		return invalid_subscription_id;
	}
	
 }
helics_subscription_id_t helicsRegisterDoubleSubscription(helics_federate_id_t fedID,const char *name, const char *units)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return invalid_subscription_id;
 }
helics_subscription_id_t helicsRegisterStringSubscription(helics_federate_id_t fedID, const char *name, const char *units)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return invalid_subscription_id;
 }

helics_publication_id_t  helicsRegisterPublication(helics_federate_id_t fedID, const char *name, const char *type, const char *units)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return invalid_publication_id;
 }
helics_publication_id_t  helicsRegisterDoublePublication(helics_federate_id_t fedID, const char *name, const char *units)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return invalid_publication_id;
 }
helics_publication_id_t  helicsRegisterStringPublication(helics_federate_id_t fedID, const char *name, const char *units)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return invalid_publication_id;
 }
helics_publication_id_t  helicsRegisterGlobalPublication(helics_federate_id_t fedID, const char *name, const char *type, const char *units)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return invalid_publication_id;
 }
helics_publication_id_t  helicsRegisterGlobalDoublePublication(helics_federate_id_t fedID, const char *name, const char *units)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return invalid_publication_id;
 }
helics_publication_id_t  helicsRegisterGlobalStringPublication(helics_federate_id_t fedID, const char *name, const char *units)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return invalid_publication_id;
 }
/* initialization, execution, and time requests */
helicsStatus helicsEnterInitializationMode(helics_federate_id_t fedID)
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

helicsStatus helicsEnterExecutionMode(helics_federate_id_t fedID)
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

helics_time_t helicsRequestTime(helics_federate_id_t fedID, helics_time_t requestTime)
{
	auto fed = getFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return requestTime;
 }

/* getting and publishing values */
helicsStatus helicsPublish(helics_federate_id_t fedID, helics_publication_id_t pubID, const char *data, uint64_t len)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return helicsOK;
 }

helicsStatus helicsPublishString(helics_federate_id_t fedID, helics_publication_id_t pubID, const char *str)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return helicsOK;
 }

helicsStatus helicsPublishDouble(helics_federate_id_t fedID, helics_publication_id_t pubID, double val)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return helicsOK;
 }

uint64_t helicsGetValue(helics_federate_id_t fedID, helics_publication_id_t pubID, char *data, uint64_t maxlen)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return 0;
 }

helicsStatus helicsGetString(helics_federate_id_t fedID, helics_publication_id_t pubID, char *str, uint64_t maxlen)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return helicsOK;
 }

helicsStatus helicsGetDouble(helics_federate_id_t fedID, helics_publication_id_t pubID, double *val)
{
	auto fed = getValueFed(fedID);
	if (!fed)
	{
		return helicsDiscard;
	}
	return helicsOK;
 }
