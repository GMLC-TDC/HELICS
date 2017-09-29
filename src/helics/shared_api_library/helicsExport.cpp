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


helics_federate_info_t createFederateInfoObject()
{
	auto *fi = new helics::FederateInfo;
	return reinterpret_cast<void *>(fi);
}

void freeFederateInfoObject(helics_federate_info_t fi)
{
	if (fi != nullptr)
	{
		delete reinterpret_cast<helics::FederateInfo *>(fi);
	}
}

helicsStatus FederateInfoSetFederateName(helics_federate_info_t fi, const char *name)
{
	if (fi == nullptr)
	{
		return helicsDiscard;
	}
	auto hfi = reinterpret_cast<helics::FederateInfo *>(fi);
	hfi->name=name;
}
helicsStatus FederateInfoSetCoreName(helics_federate_info_t fi, const char *corename)
{
	if (fi == nullptr)
	{
		return helicsDiscard;
	}
	auto hfi = reinterpret_cast<helics::FederateInfo *>(fi);
	hfi->coreName = corename;
}
helicsStatus FederateInfoSetCoreInitString(helics_federate_info_t fi, const char *coreinit)
{
	if (fi == nullptr)
	{
		return helicsDiscard;
	}
	auto hfi = reinterpret_cast<helics::FederateInfo *>(fi);
	hfi->coreInitString = coreinit;
}
helicsStatus FederateInfoSetCoreType(helics_federate_info_t fi, const char *coretype)
{
	if (fi == nullptr)
	{
		return helicsDiscard;
	}
	auto hfi = reinterpret_cast<helics::FederateInfo *>(fi);
	hfi->coreType = coretype;
}
helicsStatus FederateInfoSetFlag(helics_federate_info_t fi, int flag, int value)
{
	if (fi == nullptr)
	{
		return helicsDiscard;
	}
	auto hfi = reinterpret_cast<helics::FederateInfo *>(fi);
	switch (flag)
	{
	case OBSERVER_FLAG:
		hfi->observer = (value != 0);
		break;
	case ROLLBACK_FLAG:
		hfi->rollback = (value != 0);
		break;
	case UNINTERRUPTIBLE_FLAG:
		hfi->uninterruptible = (value != 0);
		break;
	case FORWARD_COMPUTE_FLAG:
		hfi->forwardCompute = (value != 0);
		break;
	case TIME_AGNOSTIC_FLAG:
		hfi->timeAgnostic = (value != 0);
		break;
	default:
		return helicsDiscard;
	}
	return helicsOK;
}
helicsStatus FederateInfoSetLookahead(helics_federate_info_t fi, helics_time_t lookahead)
{
	if (fi == nullptr)
	{
		return helicsDiscard;
	}
	auto hfi = reinterpret_cast<helics::FederateInfo *>(fi);
	hfi->lookAhead = helics::Time(lookahead, timeUnits::ns);
	return helicsOK;
}

helicsStatus FederateInfoSetTimeDelta(helics_federate_info_t fi, helics_time_t timeDelta)
{
	if (fi == nullptr)
	{
		return helicsDiscard;
	}
	auto hfi = reinterpret_cast<helics::FederateInfo *>(fi);
	hfi->timeDelta = helics::Time(timeDelta, timeUnits::ns);
	return helicsOK;
}

helicsStatus FederateInfoSetImpactWindow(helics_federate_info_t fi, helics_time_t impactWindow)
{
	if (fi == nullptr)
	{
		return helicsDiscard;
	}
	auto hfi = reinterpret_cast<helics::FederateInfo *>(fi);
	hfi->impactWindow = helics::Time(impactWindow, timeUnits::ns);
	return helicsOK;
}
helicsStatus FederateInfoSetTimeOffset(helics_federate_info_t fi, helics_time_t timeOffset)
{
	if (fi == nullptr)
	{
		return helicsDiscard;
	}
	auto hfi = reinterpret_cast<helics::FederateInfo *>(fi);
	hfi->offset = helics::Time(timeOffset, timeUnits::ns);
	return helicsOK;
}
helicsStatus FederateInfoSetPeriod(helics_federate_info_t fi, helics_time_t period)
{
	if (fi == nullptr)
	{
		return helicsDiscard;
	}
	auto hfi = reinterpret_cast<helics::FederateInfo *>(fi);
	hfi->period = helics::Time(period, timeUnits::ns);
	return helicsOK;
}


helicsStatus FederateInfoSetLoggingLevel(helics_federate_info_t fi, int logLevel)
{
	if (fi == nullptr)
	{
		return helicsDiscard;
	}
	auto hfi = reinterpret_cast<helics::FederateInfo *>(fi);
	hfi->logLevel = logLevel;
	return helicsOK;
}

helicsStatus FederateInfoSetMaxIterations(helics_federate_info_t fi, int max_iterations)
{
	if (fi == nullptr)
	{
		return helicsDiscard;
	}
	auto hfi = reinterpret_cast<helics::FederateInfo *>(fi);
	hfi->max_iterations = max_iterations;
	return helicsOK;
}
