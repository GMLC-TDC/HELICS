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

static const std::string nullstr;


helics_status helicsFederateInfoLoadFromArgs(helics_federate_info_t fi, int argc, const char * const *argv)
{
    if (fi == nullptr)
    {
        return helics_discard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->loadInfoFromArgs(argc, argv);
    return helics_ok;
}

helics_status helicsFederateInfoSetFederateName (helics_federate_info_t fi, const char *name)
{
    if (fi == nullptr)
    {
        return helics_invalid_object;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->name = (name != nullptr) ? std::string(name) : nullstr;
    return helics_ok;
}
helics_status helicsFederateInfoSetCoreName(helics_federate_info_t fi, const char *corename)
{
    if (fi == nullptr)
    {
        return helics_invalid_object;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
	hfi->coreName= (corename != nullptr) ? std::string(corename) : nullstr;
    
    return helics_ok;
}

helics_status helicsFederateInfoSetCoreInitString (helics_federate_info_t fi, const char *coreinit)
{
    if (fi == nullptr)
    {
        return helics_invalid_object;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
	hfi->coreInitString= (coreinit != nullptr) ? std::string(coreinit) : nullstr;
    return helics_ok;
}

helics_status helicsFederateInfoSetCoreType (helics_federate_info_t fi, int coretype)
{
    if (fi == nullptr)
    {
        return helics_invalid_object;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->coreType = static_cast<helics::core_type> (coretype);
    return helics_ok;
}

helics_status helicsFederateInfoSetCoreTypeFromString (helics_federate_info_t fi, const char *coretype)
{
    if (fi == nullptr)
    {
        return helics_invalid_object;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    try
    {
        if (coretype == nullptr)
        {
            hfi->coreType = helics::core_type::DEFAULT;
        }
        else
        {
            hfi->coreType = helics::coreTypeFromString(coretype);
        }
    }
    catch (const std::invalid_argument &ie)
    {
        return helics_error;
    }
    return helics_ok;
}

helics_status helicsFederateInfoSetFlag (helics_federate_info_t fi, int flag, int value)
{
    if (fi == nullptr)
    {
        return helics_invalid_object;
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
	case WAIT_FOR_CURRENT_TIME_UPDATE_FLAG:
		hfi->wait_for_current_time_updates = (value != 0);
		break;
    default:
        return helics_invalid_object;
    }
    return helics_ok;
}
helics_status helicsFederateInfoSetOutputDelay (helics_federate_info_t fi, helics_time_t outputDelay)
{
    if (fi == nullptr)
    {
        return helics_invalid_object;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->outputDelay = outputDelay;
    return helics_ok;
}

helics_status helicsFederateInfoSetTimeDelta (helics_federate_info_t fi, helics_time_t timeDelta)
{
    if (fi == nullptr)
    {
        return helics_invalid_object;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->timeDelta = timeDelta;
    return helics_ok;
}

helics_status helicsFederateInfoSetInputDelay (helics_federate_info_t fi, helics_time_t inputDelay)
{
    if (fi == nullptr)
    {
        return helics_invalid_object;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->inputDelay = inputDelay;
    return helics_ok;
}
helics_status helicsFederateInfoSetTimeOffset (helics_federate_info_t fi, helics_time_t timeOffset)
{
    if (fi == nullptr)
    {
        return helics_invalid_object;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->offset = timeOffset;
    return helics_ok;
}
helics_status helicsFederateInfoSetPeriod (helics_federate_info_t fi, helics_time_t period)
{
    if (fi == nullptr)
    {
        return helics_invalid_object;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
	if (period < 0)
	{
		return helics_discard;
	}
    hfi->period = period;
    return helics_ok;
}

helics_status helicsFederateInfoSetLoggingLevel (helics_federate_info_t fi, int logLevel)
{
    if (fi == nullptr)
    {
        return helics_invalid_object;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->logLevel = logLevel;
    return helics_ok;
}

helics_status helicsFederateInfoSetMaxIterations (helics_federate_info_t fi, int maxIterations)
{
    if (fi == nullptr)
    {
        return helics_invalid_object;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->maxIterations = maxIterations;
    return helics_ok;
}

helics_core helicsCreateCore (const char *type, const char *name, const char *initString)
{

    helics::core_type ct;
    try
    {
        ct = (type != nullptr) ? helics::coreTypeFromString(type) : helics::core_type::DEFAULT;
    }
    catch (const std::invalid_argument &ie)
    {
        return nullptr;
    }
    auto *core = new helics::CoreObject;
    core->index = getMasterHolder()->addCore(core);
    core->coreptr = helics::CoreFactory::FindOrCreate (ct, (name!=nullptr)?std::string(name):nullstr, (initString!=nullptr)?std::string(initString):nullstr);
    return reinterpret_cast<helics_core> (core);
}

helics_core helicsCreateCoreFromArgs (const char *type, const char *name, int argc, const char *const *argv)
{
    helics::core_type ct;
    try
    {
        ct = (type != nullptr) ? helics::coreTypeFromString(type) : helics::core_type::DEFAULT;
    }
    catch (const std::invalid_argument &ie)
    {
        return nullptr;
    }
    auto *core = new helics::CoreObject;
    core->index = getMasterHolder ()->addCore (core);
    core->coreptr = helics::CoreFactory::FindOrCreate (ct, (name != nullptr) ? std::string(name) : nullstr, argc, argv);
    return reinterpret_cast<helics_core> (core);
}

helics_broker helicsCreateBroker (const char *type, const char *name, const char *initString)
{
    helics::core_type ct;
    try
    {
        ct = (type != nullptr) ? helics::coreTypeFromString(type) : helics::core_type::DEFAULT;
    }
    catch (const std::invalid_argument &ie)
    {
        return nullptr;
    }
    auto broker = new helics::BrokerObject;
    broker->index = getMasterHolder ()->addBroker (broker);
    broker->brokerptr = helics::BrokerFactory::create (ct, (name != nullptr) ? std::string(name) : nullstr, (initString != nullptr) ? std::string(initString) : nullstr);
    return reinterpret_cast<helics_broker> (broker);
}

helics_broker helicsCreateBrokerFromArgs (const char *type, const char *name, int argc, const char *const *argv)
{
    helics::core_type ct;
    try
    {
        ct = (type != nullptr) ? helics::coreTypeFromString(type) : helics::core_type::DEFAULT;
    }
    catch (const std::invalid_argument &ie)
    {
        return nullptr;
    }
    auto *broker = new helics::BrokerObject;
    broker->index = getMasterHolder ()->addBroker (broker);
    broker->brokerptr = helics::BrokerFactory::create (ct, (name != nullptr) ? std::string(name) : nullstr, argc, argv);
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


helics_status helicsBrokerGetIdentifier(helics_broker broker, char *identifier, int maxlen)
{
    if (broker == nullptr)
    {
        return helics_error;
    }
    auto brokerObj = reinterpret_cast<helics::BrokerObject *> (broker);
    if (brokerObj->brokerptr)
    {
        auto &ident = brokerObj->brokerptr->getIdentifier();
        if (static_cast<int> (ident.size()) > maxlen)
        {
            strncpy(identifier, ident.c_str(), maxlen);
            identifier[maxlen - 1] = 0;
        }
        else
        {
            strcpy(identifier, ident.c_str());
        }
        return helics_ok;
    }
    return helics_error;
}


helics_status helicsCoreGetIdentifier(helics_core core, char *identifier, int maxlen)
{
    if (core == nullptr)
    {
        return helics_error;
    }
    auto coreObj = reinterpret_cast<helics::CoreObject *> (core);
    if (coreObj->coreptr)
    {
        auto &ident = coreObj->coreptr->getIdentifier();
        if (static_cast<int> (ident.size()) > maxlen)
        {
            strncpy(identifier, ident.c_str(), maxlen);
            identifier[maxlen - 1] = 0;
        }
        else
        {
            strcpy(identifier, ident.c_str());
        }
        return helics_ok;
    }
    return helics_error;
}


helics_status helicsBrokerGetAddress(helics_broker broker, char *address, int maxlen)
{
    if (broker == nullptr)
    {
        return helics_error;
    }
    auto brokerObj = reinterpret_cast<helics::BrokerObject *> (broker);
    if (brokerObj->brokerptr)
    {
        auto ident = brokerObj->brokerptr->getAddress();
        if (static_cast<int> (ident.size()) > maxlen)
        {
            strncpy(address, ident.c_str(), maxlen);
            address[maxlen - 1] = 0;
        }
        else
        {
            strcpy(address, ident.c_str());
        }
        return helics_ok;
    }
    return helics_error;
}



helics_status helicsCoreDisconnect(helics_core core)
{
    if (core == nullptr)
    {
        return helics_error;
    }
    auto coreObj = reinterpret_cast<helics::CoreObject *> (core);
    if (coreObj->coreptr)
    {
        coreObj->coreptr->disconnect();
        return helics_ok;
    }
    return helics_error;
}


helics_status helicsBrokerDisconnect(helics_broker broker)
{
    if (broker == nullptr)
    {
        return helics_error;
    }
    auto brokerObj = reinterpret_cast<helics::BrokerObject *> (broker);
    if (brokerObj->brokerptr)
    {
        brokerObj->brokerptr->disconnect();
        return helics_ok;
    }
    return helics_error;
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
    queryObj->query = (query != nullptr) ? std::string(query) : std::string();
    queryObj->target = (target!=nullptr)?std::string(target):std::string();
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


helics_status helicsExecuteQueryAsync(helics_federate fed, helics_query query)
{
    if (fed == nullptr)
    {
        return helics_invalid_object;
    }
    if (query == nullptr)
    {
        return helics_invalid_object;
    }
    auto fedObj = getFedSharedPtr(fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }

    auto queryObj = reinterpret_cast<helics::queryObject *> (query);
    if (queryObj->target.empty())
    {
        queryObj->asyncIndexCode=fedObj->queryAsync(queryObj->query);
        
    }
    else
    {
        queryObj->asyncIndexCode = fedObj->queryAsync(queryObj->target,queryObj->query);
    }
    queryObj->activeAsync = true;
    queryObj->activeFed = fedObj;
    return helics_ok;
}

const char *helicsExecuteQueryComplete(helics_query query)
{

    if (query == nullptr)
    {
        return nullptr;
    }
   
    auto queryObj = reinterpret_cast<helics::queryObject *> (query);
    if (queryObj->asyncIndexCode != helics::invalid_id_value)
    {
        queryObj->response = queryObj->activeFed->queryComplete(queryObj->asyncIndexCode);
   }
    queryObj->activeAsync = false;
    queryObj->activeFed = nullptr;
    queryObj->asyncIndexCode = helics::invalid_id_value;
    return queryObj->response.c_str();
}


HELICS_Export int isQueryCompleted(helics_query query)
{

    if (query == nullptr)
    {
        return 0;
    }

    auto queryObj = reinterpret_cast<helics::queryObject *> (query);
    if (queryObj->asyncIndexCode != helics::invalid_id_value)
    {
        auto res = queryObj->activeFed->isQueryCompleted(queryObj->asyncIndexCode);
        return (res) ? 1 : 0;
    }
    return 0;
}

void helicsFreeQuery (helics_query query) { delete reinterpret_cast<helics::queryObject *> (query); }