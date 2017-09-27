/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics.h"
#include "core/helics-time.h"
#include "internal/api_objects.h"
#include "application_api/application_api.h"
#include <memory>
#include <mutex>
#include <vector>

const char *helicsGetVersion (void) { return HelicsVersion; }

/** this is a random identifier put in place when the federate gets created*/
static const int validationIdentifier = 0x2352188;  
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


helics::Federate *getFed(helics_federate fed)
{
	auto fedObj = reinterpret_cast<helics::FedObject *>(fed);
	if (fedObj->valid == validationIdentifier)
	{
		return fedObj->fedptr.get();
	}
	return nullptr;
}

helics::ValueFederate *getValueFed(helics_value_federate fed)
{
	auto fedObj = reinterpret_cast<helics::FedObject *>(fed);
	if (fedObj->valid == validationIdentifier)
	{
		if ((fedObj->type == helics::vtype::valueFed) || (fedObj->type == helics::vtype::combinFed))
		{
			return dynamic_cast<helics::ValueFederate *>(fedObj->fedptr.get());
		}
	}
	return nullptr;

}

helics::MessageFederate * getMessageFed(helics_message_federate fed)
{
	auto fedObj = reinterpret_cast<helics::FedObject *>(fed);
	if (fedObj->valid == validationIdentifier)
	{
		if ((fedObj->type == helics::vtype::messageFed) || (fedObj->type == helics::vtype::combinFed) || (fedObj->type == helics::vtype::filterFed))
		{
			return dynamic_cast<helics::MessageFederate *>(fedObj->fedptr.get());
		}
	}
	return nullptr;
}

helics::MessageFilterFederate * getFilterFed(helics_message_filter_federate fed)
{
	auto fedObj = reinterpret_cast<helics::FedObject *>(fed);
	if (fedObj->valid == validationIdentifier)
	{
		if (fedObj->type == helics::vtype::filterFed)
		{
			return dynamic_cast<helics::MessageFilterFederate *>(fedObj->fedptr.get());
		}
	}
	return nullptr;
}

static helics::FederateInfo generateInfo(const federate_info_t *fi)
{
	helics::FederateInfo fedInfo;
	fedInfo.name = fi->name;
	fedInfo.coreInitString = fi->coreInitString;
	fedInfo.coreType = fi->coreType;
	fedInfo.logLevel = fi->logLevel;
	fedInfo.forwardCompute = fi->forwardCompute;
	fedInfo.impactWindow.setBaseTimeCode(fi->impactWindow);
	fedInfo.timeDelta.setBaseTimeCode(fi->timeDelta);
	fedInfo.lookAhead.setBaseTimeCode(fi->lookAhead);
	fedInfo.period.setBaseTimeCode(fi->period);
	fedInfo.offset.setBaseTimeCode(fi->offset);
	fedInfo.uninterruptible = fi->uninterruptible;
	fedInfo.timeAgnostic = fi->timeAgnostic;
	fedInfo.rollback = fi->rollback;
	fedInfo.max_iterations = fi->max_iterations;
	return fedInfo;
}
/* Creation and destruction of Federates */
helics_federate helicsCreateValueFederate(const federate_info_t *fi)
{
	helics::FedObject *fed = new helics::FedObject;
	fed->fedptr = std::make_shared<helics::ValueFederate>(generateInfo(fi));
	fed->type = helics::vtype::valueFed;
	fed->valid = validationIdentifier;
	return reinterpret_cast<void *>(fed);
 }

helics_federate helicsCreateValueFederateFromFile(const char *file)
{
	helics::FedObject *fed = new helics::FedObject;
	fed->fedptr = std::make_shared<helics::ValueFederate>(file);
	fed->type = helics::vtype::valueFed;
	fed->valid = validationIdentifier;
	return reinterpret_cast<void *>(fed);
 }

/* Creation and destruction of Federates */
helics_federate helicsCreateMessageFederate(const federate_info_t *fi)
{
	helics::FedObject *fed = new helics::FedObject;
	fed->fedptr = std::make_shared<helics::MessageFederate>(generateInfo(fi));
	fed->type = helics::vtype::messageFed;
	fed->valid = validationIdentifier;
	return reinterpret_cast<void *>(fed);
}

helics_federate helicsCreateMessageFederateFromFile(const char *file)
{
	helics::FedObject *fed = new helics::FedObject;
	fed->fedptr = std::make_shared<helics::MessageFederate>(file);
	fed->type = helics::vtype::messageFed;
	fed->valid = validationIdentifier;
	return reinterpret_cast<void *>(fed);
}

/* Creation and destruction of Federates */
helics_federate helicsCreateMessageFilterFederate(const federate_info_t *fi)
{
	helics::FedObject *fed = new helics::FedObject;
	fed->fedptr = std::make_shared<helics::MessageFilterFederate>(generateInfo(fi));
	fed->type = helics::vtype::filterFed;
	fed->valid = validationIdentifier;
	return reinterpret_cast<void *>(fed);
}

helics_federate helicsCreateMessageFilterFederateFromFile(const char *file)
{
	helics::FedObject *fed = new helics::FedObject;
	fed->fedptr = std::make_shared<helics::MessageFilterFederate>(file);
	fed->type = helics::vtype::filterFed;
	fed->valid = validationIdentifier;
	return reinterpret_cast<void *>(fed);
}

/* Creation and destruction of Federates */
helics_federate helicsCreateCombinationFederate(const federate_info_t *fi)
{
	helics::FedObject *fed = new helics::FedObject;
	fed->fedptr = std::make_shared<helics::CombinationFederate>(generateInfo(fi));
	fed->type = helics::vtype::combinFed;
	fed->valid = validationIdentifier;
	return reinterpret_cast<void *>(fed);
}

helics_federate helicsCreateCombinationFederateFromFile(const char *file)
{
	helics::FedObject *fed = new helics::FedObject;
	fed->fedptr = std::make_shared<helics::CombinationFederate>(file);
	fed->type = helics::vtype::combinFed;
	fed->valid = validationIdentifier;
	return reinterpret_cast<void *>(fed);
}

void helicsInitializeFederateInfo(federate_info_t *fi)
{
	fi->coreInitString = "";
	fi->coreName = "";
	fi->coreType = "test";
	fi->forwardCompute = false;
	fi->impactWindow = 0;
	fi->timeAgnostic = false;
	fi->uninterruptible = true;
	fi->lookAhead = 0;
	fi->obeserver = false;
	fi->timeAgnostic = false;
	fi->timeDelta = 0;
	fi->period = 0;
	fi->offset = 0;
	fi->logLevel = 2;
	fi->max_iterations = 10;
 }

helicsStatus helicsFinalize(helics_federate fedID)
{
	auto fed = getFed(fedID);
	if (fed==nullptr)
	{
		return helicsDiscard;
	}
	fed->finalize();
	
	return helicsOK;
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

static helics::convergence_state getConvergenceState(convergence_status conv)
{
	switch (conv)
	{
	case converged:
		return helics::convergence_state::complete;
	case nonconverged:
		return helics::convergence_state::nonconverged;
	case error:
	default:
		return helics::convergence_state::error;
	case halted:
		return helics::convergence_state::halted;
		
	}
}

static convergence_status getConvergenceStatus(helics::convergence_state convState)
{
	switch (convState)
	{
	case helics::convergence_state::complete:
		return converged;
	case helics::convergence_state::nonconverged:
	case helics::convergence_state::continue_processing:
		return nonconverged;
	case helics::convergence_state::error:
	default:
		return error;
	case helics::convergence_state::halted:
		return halted;
	}
}
helicsStatus helicsEnterExecutionModeIterative(helics_federate fed, convergence_status converged, convergence_status *outConverged)
{
	auto fedObj = getFed(fed);
	if (fedObj==nullptr)
	{
		return helicsDiscard;
	}
	try
	{
		auto val = fedObj->enterExecutionState(getConvergenceState(converged));
		if (outConverged != nullptr)
		{
			*outConverged = getConvergenceStatus(val);
		}
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