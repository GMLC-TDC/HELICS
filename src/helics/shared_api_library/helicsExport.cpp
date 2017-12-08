/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "../common/logger.h"
#include "../core/BrokerFactory.h"
#include "../core/CoreFactory.h"
#include "../helics.hpp"
#include "helics.h"
#include "internal/api_objects.h"
#include <atomic>
#include <future>
#include <memory>
#include <mutex>
#include <vector>

#include "helics/helics-config.h"
#if HELICS_HAVE_ZEROMQ > 0
#include "../common/cppzmq/zmq.hpp"
#include "../common/zmqContextManager.h"
#endif

static const std::string versionStr (std::to_string (HELICS_VERSION_MAJOR) + "." + std::to_string (HELICS_VERSION_MINOR) + "." +
                                     std::to_string (HELICS_VERSION_PATCH) + " (" + HELICS_DATE + ")");

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
    case UNINTERRUPTIBLE_FLAG:
        hfi->uninterruptible = (value != 0);
        break;
    case ONLY_TRANSMIT_ON_CHANGE_FLAG:
        hfi->only_transmit_on_change = (value != 0);
        break;
    case ONLY_UPDATE_ON_CHANGE_FLAG:
        hfi->only_update_on_change = (value != 0);
        break;
    case SOURCE_ONLY_FLAG:
        hfi->source_only = (value != 0);
        break;
    case ROLLBACK_FLAG:
        hfi->rollback = (value != 0);
        break;
    case FORWARD_COMPUTE_FLAG:
        hfi->forwardCompute = (value != 0);
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
    auto *core = new helics::CoreObject;
    core->index = getMasterHolder ()->addCore (core);
    auto ct = helics::coreTypeFromString (type);
    core->coreptr = helics::CoreFactory::FindOrCreate (ct, name, initString);
    return reinterpret_cast<helics_core> (core);
}

helics_core helicsCreateCoreFromArgs (const char *type, const char *name, int argc, const char *const *argv)
{
    auto *core = new helics::CoreObject;
    core->index = getMasterHolder ()->addCore (core);
    auto ct = helics::coreTypeFromString (type);
    core->coreptr = helics::CoreFactory::FindOrCreate (ct, name, argc, argv);
    return reinterpret_cast<helics_core> (core);
}

helics_broker helicsCreateBroker (const char *type, const char *name, const char *initString)
{
    auto broker = new helics::BrokerObject;
    broker->index = getMasterHolder ()->addBroker (broker);
    auto ct = helics::coreTypeFromString (type);
    broker->brokerptr = helics::BrokerFactory::create (ct, name, initString);
    return reinterpret_cast<helics_broker> (broker);
}

helics_broker helicsCreateBrokerFromArgs (const char *type, const char *name, int argc, const char *const *argv)
{
    auto *broker = new helics::BrokerObject;
    broker->index = getMasterHolder ()->addBroker (broker);
    auto ct = helics::coreTypeFromString (type);
    broker->brokerptr = helics::BrokerFactory::create (ct, name, argc, argv);
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
    auto coreObj = reinterpret_cast<helics::CoreObject *> (core);
    if (coreObj->coreptr)
    {
        return (coreObj->coreptr->isConnected ()) ? 1 : 0;
    }
    return 0;
}

void helicsFreeCore (helics_core core)
{
    auto *coreObj = reinterpret_cast<helics::CoreObject *> (core);
    if (coreObj != nullptr)
    {
        getMasterHolder ()->clearCore (coreObj->index);
        delete coreObj;
    }
    helics::CoreFactory::cleanUpCores ();
}

void helicsFreeBroker (helics_broker broker)
{
    auto *brokerObj = reinterpret_cast<helics::BrokerObject *> (broker);
    if (brokerObj != nullptr)
    {
        getMasterHolder ()->clearBroker (brokerObj->index);
        delete brokerObj;
    }
    helics::BrokerFactory::cleanUpBrokers ();
}

void helicsFreeFederate (helics_federate fed)
{
    auto *fedObj = reinterpret_cast<helics::FedObject *> (fed);
    if (fedObj != nullptr)
    {
        getMasterHolder ()->clearFed (fedObj->index);
        delete fedObj;
    }

    helics::CoreFactory::cleanUpCores ();
}

helics::FedObject::~FedObject ()
{
    for (auto sub : subs)
    {
        delete sub;
    }
    for (auto pub : pubs)
    {
        delete pub;
    }
    for (auto ept : epts)
    {
        delete ept;
    }
    fedptr = nullptr;
}

void helicsCloseLibrary ()
{
    clearAllObjects ();
    auto ret = std::async (std::launch::async, []() { helics::CoreFactory::cleanUpCores (2000); });
    helics::BrokerFactory::cleanUpBrokers (2000);
    ret.get ();
#if HELICS_HAVE_ZEROMQ > 0
    if (zmqContextManager::setContextToLeakOnDelete ())
    {
        zmqContextManager::getContext ().close ();
    }

#endif
    helics::loggerManager::closeLogger ();
}

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