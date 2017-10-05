/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "application_api/application_api.h"
#include "core/BrokerFactory.h"
#include "core/CoreFactory.h"
#include "core/helics-time.h"
#include "helics.h"
#include "internal/api_objects.h"
#include <memory>
#include <mutex>
#include <vector>

#include "helics/config.h"

static const std::string versionStr (std::to_string (HELICS_VERSION_MAJOR) + "." +
                                     std::to_string (HELICS_VERSION_MINOR) + "." +
                                     std::to_string (HELICS_VERSION_PATCH) + " (" + HELICS_DATE + ")");

const char *helicsGetVersion (void) { return versionStr.c_str (); }

helics_time_t helicsTimeFromDouble (double time)
{
    helics::Time val (time);
    return val.getBaseTimeCode ();
}

double doubleFromHelicsTime (helics_time_t time)
{
    helics::Time val;
    val.setBaseTimeCode (time);
    return static_cast<double> (val);
}

helics_federate_info_t createFederateInfoObject ()
{
    auto *fi = new helics::FederateInfo;
    return reinterpret_cast<void *> (fi);
}

void freeFederateInfoObject (helics_federate_info_t fi) { delete reinterpret_cast<helics::FederateInfo *> (fi); }

helicsStatus FederateInfoSetFederateName (helics_federate_info_t fi, const char *name)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->name = name;
    return helicsOK;
}
helicsStatus FederateInfoSetCoreName (helics_federate_info_t fi, const char *corename)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->coreName = corename;
    return helicsOK;
}
helicsStatus FederateInfoSetCoreInitString (helics_federate_info_t fi, const char *coreinit)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->coreInitString = coreinit;
    return helicsOK;
}

helicsStatus FederateInfoSetCoreType (helics_federate_info_t fi, int coretype)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->coreType = static_cast<helics::core_type> (coretype);
    return helicsOK;
}

helicsStatus FederateInfoSetCoreTypeFromString (helics_federate_info_t fi, const char *coretype)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    try
    {
        hfi->coreType = helics::coreTypeFromString (coretype);
    }
    catch (const std::invalid_argument &ie)
    {
        return helicsError;
    }
    return helicsOK;
}
helicsStatus FederateInfoSetFlag (helics_federate_info_t fi, int flag, int value)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
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
    case SOURCE_ONLY_FLAG:
        hfi->sourceOnly = (value != 0);
        break;
    default:
        return helicsDiscard;
    }
    return helicsOK;
}
helicsStatus FederateInfoSetLookahead (helics_federate_info_t fi, helics_time_t lookahead)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->lookAhead = helics::Time (lookahead, timeUnits::ns);
    return helicsOK;
}

helicsStatus FederateInfoSetTimeDelta (helics_federate_info_t fi, helics_time_t timeDelta)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->timeDelta = helics::Time (timeDelta, timeUnits::ns);
    return helicsOK;
}

helicsStatus FederateInfoSetImpactWindow (helics_federate_info_t fi, helics_time_t impactWindow)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->impactWindow = helics::Time (impactWindow, timeUnits::ns);
    return helicsOK;
}
helicsStatus FederateInfoSetTimeOffset (helics_federate_info_t fi, helics_time_t timeOffset)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->offset = helics::Time (timeOffset, timeUnits::ns);
    return helicsOK;
}
helicsStatus FederateInfoSetPeriod (helics_federate_info_t fi, helics_time_t period)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->period = helics::Time (period, timeUnits::ns);
    return helicsOK;
}

helicsStatus FederateInfoSetLoggingLevel (helics_federate_info_t fi, int logLevel)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->logLevel = logLevel;
    return helicsOK;
}

helicsStatus FederateInfoSetMaxIterations (helics_federate_info_t fi, int max_iterations)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->max_iterations = max_iterations;
    return helicsOK;
}

helics_core helicsCreateCore (const char *type, const char *name, const char *initString)
{
    auto *core = new helics::coreObject;
    core->coreptr = helics::CoreFactory::FindOrCreate (helics::coreTypeFromString (type), name, initString);
    return reinterpret_cast<helics_core> (core);
}

helics_core helicsCreateCoreFromArgs (const char *type, const char *name, int argc, char *argv[])
{
    auto *core = new helics::coreObject;
    core->coreptr = helics::CoreFactory::FindOrCreate (helics::coreTypeFromString (type), name, argc, argv);
    return reinterpret_cast<helics_core> (core);
}

helics_broker helicsCreateBroker (const char *type, const char *name, const char *initString)
{
    auto broker = new helics::BrokerObject;
    helics::core_type ctype = helics::coreTypeFromString(type);
    broker->brokerptr = helics::BrokerFactory::create (ctype, name, initString);
    return reinterpret_cast<helics_broker> (broker);
}

helics_broker helicsCreateBrokerFromArgs (const char *type, const char *name, int argc, char *argv[])
{
    auto *broker = new helics::BrokerObject;
    broker->brokerptr = helics::BrokerFactory::create (helics::coreTypeFromString (type),name, argc, argv);
    return reinterpret_cast<helics_broker> (broker);
}

int helicsBrokerIsConnected(helics_broker broker)
{
	if (broker == nullptr)
	{
		return 0;
	}
	auto brokerObj = reinterpret_cast<helics::BrokerObject*>(broker);
	if (brokerObj->brokerptr)
	{
		return (brokerObj->brokerptr->isConnected()) ? 1 : 0;
  }
	return 0;
}

int helicsCoreIsConnected(helics_core core)
{
	if (core == nullptr)
	{
		return 0;
	}
	auto coreObj = reinterpret_cast<helics::coreObject*>(core);
	if (coreObj->coreptr)
	{
		return (coreObj->coreptr->isConnected()) ? 1 : 0;
	}
	return 0;
}

void helicsFreeCore(helics_core core)
{
	delete reinterpret_cast<helics::coreObject *>(core);
}

void helicsFreeBroker(helics_broker broker)
{
	delete reinterpret_cast<helics::BrokerObject *>(broker);
}