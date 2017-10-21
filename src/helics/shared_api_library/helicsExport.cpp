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
#include <atomic>
#include <memory>
#include <mutex>
#include <vector>

#include "helics/config.h"
#if HELICS_HAVE_ZEROMQ > 0
#include "helics/common/zmqContextManager.h"
#endif

static const std::string versionStr (std::to_string (HELICS_VERSION_MAJOR) + "." +
                                     std::to_string (HELICS_VERSION_MINOR) + "." +
                                     std::to_string (HELICS_VERSION_PATCH) + " (" + HELICS_DATE + ")");

#if HELICS_HAVE_ZEROMQ > 0
static std::atomic<bool> zmqInUse{false};
#endif
const char *helicsGetVersion (void) { return versionStr.c_str (); }

helics_federate_info_t helicsFederateInfoCreate ()
{
    auto *fi = new helics::FederateInfo;
    return reinterpret_cast<void *> (fi);
}

void helicsFederateInfoFree (helics_federate_info_t fi) { delete reinterpret_cast<helics::FederateInfo *> (fi); }

helicsStatus helicsFederateInfoSetFederateName (helics_federate_info_t fi, const char *name)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->name = name;
    return helicsOK;
}
helicsStatus helicsFederateInfoSetCoreName (helics_federate_info_t fi, const char *corename)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->coreName = corename;
    return helicsOK;
}
helicsStatus helicsFederateInfoSetCoreInitString (helics_federate_info_t fi, const char *coreinit)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->coreInitString = coreinit;
    return helicsOK;
}

helicsStatus helicsFederateInfoSetCoreType (helics_federate_info_t fi, int coretype)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->coreType = static_cast<helics::core_type> (coretype);
#if HELICS_HAVE_ZEROMQ > 0
    if (hfi->coreType == helics::core_type::ZMQ)
    {
        if (!zmqInUse)
        {
            zmqContextManager::setContextToLeakOnDelete ();
        }
    }
#endif
    return helicsOK;
}

helicsStatus helicsFederateInfoSetCoreTypeFromString (helics_federate_info_t fi, const char *coretype)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    try
    {
        hfi->coreType = helics::coreTypeFromString (coretype);
#if HELICS_HAVE_ZEROMQ > 0
        if (hfi->coreType == helics::core_type::ZMQ)
        {
            if (!zmqInUse)
            {
                zmqContextManager::setContextToLeakOnDelete ();
            }
        }
#endif
    }
    catch (const std::invalid_argument &ie)
    {
        return helicsError;
    }
    return helicsOK;
}
helicsStatus helicsFederateInfoSetFlag (helics_federate_info_t fi, int flag, int value)
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
helicsStatus helicsFederateInfoSetLookahead (helics_federate_info_t fi, helics_time_t lookahead)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->lookAhead = lookahead;
    return helicsOK;
}

helicsStatus helicsFederateInfoSetTimeDelta (helics_federate_info_t fi, helics_time_t timeDelta)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->timeDelta = timeDelta;
    return helicsOK;
}

helicsStatus helicsFederateInfoSetImpactWindow (helics_federate_info_t fi, helics_time_t impactWindow)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->impactWindow = impactWindow;
    return helicsOK;
}
helicsStatus helicsFederateInfoSetTimeOffset (helics_federate_info_t fi, helics_time_t timeOffset)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->offset = timeOffset;
    return helicsOK;
}
helicsStatus helicsFederateInfoSetPeriod (helics_federate_info_t fi, helics_time_t period)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->period = period;
    return helicsOK;
}

helicsStatus helicsFederateInfoSetLoggingLevel (helics_federate_info_t fi, int logLevel)
{
    if (fi == nullptr)
    {
        return helicsDiscard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->logLevel = logLevel;
    return helicsOK;
}

helicsStatus helicsFederateInfoSetMaxIterations (helics_federate_info_t fi, int max_iterations)
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
    auto ct = helics::coreTypeFromString (type);
    core->coreptr = helics::CoreFactory::FindOrCreate (ct, name, initString);
#if HELICS_HAVE_ZEROMQ > 0
    if (ct == helics::core_type::ZMQ)
    {
        if (!zmqInUse)
        {
            zmqContextManager::setContextToLeakOnDelete ();
        }
    }
#endif
    return reinterpret_cast<helics_core> (core);
}

helics_core helicsCreateCoreFromArgs (const char *type, const char *name, int argc, const char * const *argv)
{
    auto *core = new helics::coreObject;
    auto ct = helics::coreTypeFromString (type);
    core->coreptr = helics::CoreFactory::FindOrCreate (ct, name, argc, argv);
#if HELICS_HAVE_ZEROMQ > 0
    if (ct == helics::core_type::ZMQ)
    {
        if (!zmqInUse)
        {
            zmqContextManager::setContextToLeakOnDelete ();
        }
    }
#endif
    return reinterpret_cast<helics_core> (core);
}

helics_broker helicsCreateBroker (const char *type, const char *name, const char *initString)
{
    auto broker = new helics::BrokerObject;
    auto ct = helics::coreTypeFromString (type);
    broker->brokerptr = helics::BrokerFactory::create (ct, name, initString);
#if HELICS_HAVE_ZEROMQ > 0
    if (ct == helics::core_type::ZMQ)
    {
        if (!zmqInUse)
        {
            zmqContextManager::setContextToLeakOnDelete ();
        }
    }
#endif
    return reinterpret_cast<helics_broker> (broker);
}

helics_broker helicsCreateBrokerFromArgs (const char *type, const char *name, int argc, const char * const *argv)
{
    auto *broker = new helics::BrokerObject;
    auto ct = helics::coreTypeFromString (type);
    broker->brokerptr = helics::BrokerFactory::create (ct, name, argc, argv);
#if HELICS_HAVE_ZEROMQ > 0
    if (ct == helics::core_type::ZMQ)
    {
        if (!zmqInUse)
        {
            zmqContextManager::setContextToLeakOnDelete ();
        }
    }
#endif
    return reinterpret_cast<helics_broker> (broker);
}

int helicsBrokerIsConnected (helics_broker broker)
{
    if (broker == nullptr)
    {
        return 0;
    }
    auto brokerObj = reinterpret_cast<helics::BrokerObject *> (broker);
    if (brokerObj->brokerptr)
    {
        return (brokerObj->brokerptr->isConnected ()) ? 1 : 0;
    }
    return 0;
}

int helicsCoreIsConnected (helics_core core)
{
    if (core == nullptr)
    {
        return 0;
    }
    auto coreObj = reinterpret_cast<helics::coreObject *> (core);
    if (coreObj->coreptr)
    {
        return (coreObj->coreptr->isConnected ()) ? 1 : 0;
    }
    return 0;
}

void helicsFreeCore (helics_core core) { delete reinterpret_cast<helics::coreObject *> (core); }

void helicsFreeBroker (helics_broker broker) { delete reinterpret_cast<helics::BrokerObject *> (broker); }

helics_query helicsCreateQuery (const char *target, const char *query)
{
    auto queryObj = new helics::queryObject;
    queryObj->query = query;
    queryObj->target = target;
    return reinterpret_cast<void *> (queryObj);
}
const char *helicsExecuteQuery (helics_federate fed, helics_query query)
{
    if (fed == nullptr)
    {
        return nullptr;
    }
    if (query == nullptr)
    {
        return nullptr;
    }
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return nullptr;
    }

    auto queryObj = reinterpret_cast<helics::queryObject *> (query);
    if (queryObj->target.empty ())
    {
        queryObj->response = fedObj->query (queryObj->query);
    }
    else
    {
        queryObj->response = fedObj->query (queryObj->target, queryObj->query);
    }

    return queryObj->response.c_str ();
}

void helicsFreeQuery (helics_query query) { delete reinterpret_cast<helics::queryObject *> (query); }