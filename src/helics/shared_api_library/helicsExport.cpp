/*

Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "../common/logger.h"
#include "../core/BrokerFactory.hpp"
#include "../core/CoreFactory.hpp"
#include "../helics.hpp"
#include "helics.h"
#include "internal/api_objects.h"
#include <atomic>
#include <future>
#include <memory>
#include <mutex>
#include <vector>

#include "../core/core-exceptions.hpp"

#include "../core/helicsVersion.hpp"
#include "helics/helics-config.h"
#if HELICS_HAVE_ZEROMQ > 0
#include "../common/cppzmq/zmq.hpp"
#include "../common/zmqContextManager.h"
#endif

const char *helicsGetVersion (void) { return helics::versionString; }

helics_bool_t helicsIsCoreTypeAvailable (const char *type)
{
    if (type == nullptr)
    {
        return helics_false;
    }
    auto coreType = helics::coreTypeFromString (type);
    return (helics::isCoreTypeAvailable (coreType)) ? helics_true : helics_false;
}

helics_federate_info_t helicsFederateInfoCreate ()
{
    auto *fi = new helics::FederateInfo;
    return reinterpret_cast<void *> (fi);
}

// typedef enum {

//    helics_ok = 0, /*!< the function executed successfully */
//    helics_invalid_object, /*!< indicator that the object used was not a valid object */
//    helics_invalid_argument, /*!< the parameter passed was invalid and unable to be used*/
//    helics_discard, /*!< the input was discarded and not used for some reason */
//    helics_terminated, /*!< the federate has terminated and the call cannot be completed*/
//    helics_warning, /*!< the function issued a warning of some kind */
//    helics_invalid_state_transition, /*!< error issued when an invalid state transition occurred */
//    helics_invalid_function_call, /*!< the call made was invalid in the present state of the calling object*/
//    helics_error /*!< the function produced an error */
//} helics_status;

/** this function is based on the lippencott function template
http://cppsecrets.blogspot.com/2013/12/using-lippincott-function-for.html
*/
helics_status helicsErrorHandler () noexcept
{
    try
    {
        try
        {
            if (std::exception_ptr eptr = std::current_exception ())
            {
                std::rethrow_exception (eptr);
            }
            else
            {
                return helics_error;
            }
        }
        catch (const helics::InvalidIdentifier &)
        {
            return helics_invalid_object;
        }
        catch (const helics::InvalidFunctionCall &)
        {
            return helics_invalid_function_call;
        }
        catch (const helics::InvalidParameter &)
        {
            return helics_invalid_argument;
        }
        catch (...)
        {
            return helics_error;
        }
    }
    catch (...)
    {
        return helics_error;
    }
}

void helicsFederateInfoFree (helics_federate_info_t fi) { delete reinterpret_cast<helics::FederateInfo *> (fi); }

static const std::string nullstr;

helics_status helicsFederateInfoLoadFromArgs (helics_federate_info_t fi, int argc, const char *const *argv)
{
    if (fi == nullptr)
    {
        return helics_discard;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    try
    {
        hfi->loadInfoFromArgs (argc, argv);
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFederateInfoSetFederateName (helics_federate_info_t fi, const char *name)
{
    if (fi == nullptr)
    {
        return helics_invalid_object;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    try
    {
        hfi->name = (name != nullptr) ? std::string (name) : nullstr;
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}
helics_status helicsFederateInfoSetCoreName (helics_federate_info_t fi, const char *corename)
{
    if (fi == nullptr)
    {
        return helics_invalid_object;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    try
    {
        hfi->coreName = (corename != nullptr) ? std::string (corename) : nullstr;

        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFederateInfoSetCoreInitString (helics_federate_info_t fi, const char *coreinit)
{
    if (fi == nullptr)
    {
        return helics_invalid_object;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    try
    {
        hfi->coreInitString = (coreinit != nullptr) ? std::string (coreinit) : nullstr;
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
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
            hfi->coreType = helics::coreTypeFromString (coretype);
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsFederateInfoSetFlag (helics_federate_info_t fi, int flag, helics_bool_t value)
{
    if (fi == nullptr)
    {
        return helics_invalid_object;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    switch (flag)
    {
    case OBSERVER_FLAG:
        hfi->observer = (value != helics_false);
        break;
    case UNINTERRUPTIBLE_FLAG:
        hfi->uninterruptible = (value != helics_false);
        break;
    case ONLY_TRANSMIT_ON_CHANGE_FLAG:
        hfi->only_transmit_on_change = (value != helics_false);
        break;
    case ONLY_UPDATE_ON_CHANGE_FLAG:
        hfi->only_update_on_change = (value != helics_false);
        break;
    case SOURCE_ONLY_FLAG:
        hfi->source_only = (value != helics_false);
        break;
    case ROLLBACK_FLAG:
        hfi->rollback = (value != helics_false);
        break;
    case FORWARD_COMPUTE_FLAG:
        hfi->forwardCompute = (value != helics_false);
        break;
    case WAIT_FOR_CURRENT_TIME_UPDATE_FLAG:
        hfi->wait_for_current_time_updates = (value != helics_false);
        break;
    case REALTIME_FLAG:
        hfi->realtime = (value != helics_false);
        break;
    default:
        return helics_invalid_argument;
    }
    return helics_ok;
}
helics_status helicsFederateInfoSetOutputDelay (helics_federate_info_t fi, helics_time_t outputDelay)
{
    if (fi == nullptr)
    {
        return helics_invalid_object;
    }
    if (outputDelay < helics_time_zero)
    {
        return helics_invalid_argument;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->outputDelay = outputDelay;
    return helics_ok;
}

helics_status helicsFederateInfoSetSeparator(helics_federate_info_t fi, char separator)
{
    if (fi == nullptr)
    {
        return helics_invalid_object;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->separator=separator;
    return helics_ok;
}

helics_status helicsFederateInfoSetTimeDelta (helics_federate_info_t fi, helics_time_t timeDelta)
{
    if (fi == nullptr)
    {
        return helics_invalid_object;
    }
    if (timeDelta < helics_time_zero)
    {
        return helics_invalid_argument;
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
    if (inputDelay < helics_time_zero)
    {
        return helics_invalid_argument;
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
    if (period < helics_time_zero)
    {
        return helics_invalid_argument;
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
    if (maxIterations < 0)
    {
        maxIterations = 10'000'000;
    }
    auto hfi = reinterpret_cast<helics::FederateInfo *> (fi);
    hfi->maxIterations = maxIterations;
    return helics_ok;
}

helics::Core *getCore (helics_core core)
{
    auto CoreObj = reinterpret_cast<helics::CoreObject *> (core);
    if (CoreObj->valid == coreValidationIdentifier)
    {
        return CoreObj->coreptr.get ();
    }
    return nullptr;
}

std::shared_ptr<helics::Core> getCoreSharedPtr (helics_core core)
{
    auto CoreObj = reinterpret_cast<helics::CoreObject *> (core);
    if (CoreObj->valid == coreValidationIdentifier)
    {
        return CoreObj->coreptr;
    }
    return nullptr;
}

helics::Broker *getBroker (helics_broker broker)
{
    auto BrokerObj = reinterpret_cast<helics::BrokerObject *> (broker);
    if (BrokerObj->valid == brokerValidationIdentifier)
    {
        return BrokerObj->brokerptr.get ();
    }
    return nullptr;
}

std::shared_ptr<helics::Broker> getBrokerSharedPtr (helics_broker broker)
{
    auto BrokerObj = reinterpret_cast<helics::BrokerObject *> (broker);
    if (BrokerObj->valid == brokerValidationIdentifier)
    {
        return BrokerObj->brokerptr;
    }
    return nullptr;
}

helics_core helicsCreateCore (const char *type, const char *name, const char *initString)
{
    helics::core_type ct;
    try
    {
        ct = (type != nullptr) ? helics::coreTypeFromString (type) : helics::core_type::DEFAULT;
    }
    catch (const std::invalid_argument &ie)
    {
        return nullptr;
    }
    auto core = std::make_unique<helics::CoreObject> ();
    core->valid = coreValidationIdentifier;
    core->coreptr = helics::CoreFactory::FindOrCreate (ct, (name != nullptr) ? std::string (name) : nullstr,
                                                       (initString != nullptr) ? std::string (initString) : nullstr);
    auto retcore = reinterpret_cast<helics_core> (core.get ());
    getMasterHolder ()->addCore (std::move (core));

    return retcore;
}

helics_core helicsCreateCoreFromArgs (const char *type, const char *name, int argc, const char *const *argv)
{
    helics::core_type ct;
    try
    {
        ct = (type != nullptr) ? helics::coreTypeFromString (type) : helics::core_type::DEFAULT;
    }
    catch (const std::invalid_argument &ie)
    {
        return nullptr;
    }
    auto core = std::make_unique<helics::CoreObject> ();

    core->valid = coreValidationIdentifier;
    core->coreptr = helics::CoreFactory::FindOrCreate (ct, (name != nullptr) ? std::string (name) : nullstr, argc, argv);
    auto retcore = reinterpret_cast<helics_core> (core.get ());
    getMasterHolder ()->addCore (std::move (core));

    return retcore;
}

helics_core helicsCoreClone (helics_core core)
{
    if (core == nullptr)
    {
        return nullptr;
    }
    auto *coreObj = reinterpret_cast<helics::CoreObject *> (core);
    auto coreClone = std::make_unique<helics::CoreObject> ();
    coreClone->valid = coreValidationIdentifier;
    coreClone->coreptr = coreObj->coreptr;
    auto retcore = reinterpret_cast<helics_core> (coreClone.get ());
    getMasterHolder ()->addCore (std::move (coreClone));

    return retcore;
}

helics_federate helicsGetFederateByName (const char *fedName)
{
    auto mob = getMasterHolder ();
    auto fed = mob->findFed (fedName);
    if (fed == nullptr)
    {
        return nullptr;
    }
    return helicsFederateClone (reinterpret_cast<helics_federate> (fed));
}
helics_broker helicsCreateBroker (const char *type, const char *name, const char *initString)
{
    helics::core_type ct;
    try
    {
        ct = (type != nullptr) ? helics::coreTypeFromString (type) : helics::core_type::DEFAULT;
    }
    catch (const std::invalid_argument &ie)
    {
        return nullptr;
    }
    auto broker = std::make_unique<helics::BrokerObject> ();
    broker->valid = brokerValidationIdentifier;
    broker->brokerptr = helics::BrokerFactory::create (ct, (name != nullptr) ? std::string (name) : nullstr,
                                                       (initString != nullptr) ? std::string (initString) : nullstr);
    auto retbroker = reinterpret_cast<helics_broker> (broker.get ());
    getMasterHolder ()->addBroker (std::move (broker));
    return retbroker;
}

helics_broker helicsCreateBrokerFromArgs (const char *type, const char *name, int argc, const char *const *argv)
{
    helics::core_type ct;
    try
    {
        ct = (type != nullptr) ? helics::coreTypeFromString (type) : helics::core_type::DEFAULT;
    }
    catch (const std::invalid_argument &ie)
    {
        return nullptr;
    }
    auto broker = std::make_unique<helics::BrokerObject> ();
    broker->valid = brokerValidationIdentifier;
    broker->brokerptr = helics::BrokerFactory::create (ct, (name != nullptr) ? std::string (name) : nullstr, argc, argv);
    auto retbroker = reinterpret_cast<helics_broker> (broker.get ());
    getMasterHolder ()->addBroker (std::move (broker));
    return retbroker;
}

helics_broker helicsBrokerClone (helics_broker broker)
{
    if (broker == nullptr)
    {
        return nullptr;
    }
    auto *brokerObj = reinterpret_cast<helics::BrokerObject *> (broker);
    auto brokerClone = std::make_unique<helics::BrokerObject> ();
    brokerClone->valid = brokerValidationIdentifier;
    brokerClone->brokerptr = brokerObj->brokerptr;
    auto retbroker = reinterpret_cast<helics_broker> (brokerClone.get ());
    getMasterHolder ()->addBroker (std::move (brokerClone));
    return retbroker;
}

helics_bool_t helicsBrokerIsConnected (helics_broker broker)
{
    if (broker == nullptr)
    {
        return helics_false;
    }
    auto brk = getBroker (broker);
    if (brk == nullptr)
    {
        return helics_false;
    }
    return (brk->isConnected ()) ? helics_true : helics_false;
}

helics_bool_t helicsCoreIsConnected (helics_core core)
{
    if (core == nullptr)
    {
        return helics_false;
    }
    auto cr = getCore (core);
    if (cr == nullptr)
    {
        return helics_false;
    }
    return (cr->isConnected ()) ? helics_true : helics_false;
}

helics_status helicsBrokerGetIdentifier (helics_broker broker, char *identifier, int maxlen)
{
    if (broker == nullptr)
    {
        return helics_invalid_object;
    }
    auto brk = getBroker (broker);
    if (brk == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        auto &ident = brk->getIdentifier ();
        if (static_cast<int> (ident.size ()) > maxlen)
        {
            strncpy (identifier, ident.c_str (), maxlen);
            identifier[maxlen - 1] = '\0';
        }
        else
        {
            strcpy (identifier, ident.c_str ());
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsCoreGetIdentifier (helics_core core, char *identifier, int maxlen)
{
    if (core == nullptr)
    {
        return helics_invalid_object;
    }
    auto cr = getCore (core);
    if (cr == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        auto &ident = cr->getIdentifier ();

        if (static_cast<int> (ident.size ()) > maxlen)
        {
            strncpy (identifier, ident.c_str (), maxlen);
            identifier[maxlen - 1] = 0;
        }
        else
        {
            strcpy (identifier, ident.c_str ());
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsBrokerGetAddress (helics_broker broker, char *address, int maxlen)
{
    if (broker == nullptr)
    {
        return helics_invalid_object;
    }
    auto brk = getBroker (broker);
    if (brk == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        auto ident = brk->getAddress ();
        if (static_cast<int> (ident.size ()) > maxlen)
        {
            strncpy (address, ident.c_str (), maxlen);
            address[maxlen - 1] = 0;
        }
        else
        {
            strcpy (address, ident.c_str ());
        }
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsCoreSetReadyToInit (helics_core core)
{
    if (core == nullptr)
    {
        return helics_invalid_object;
    }
    auto cr = getCore (core);
    if (cr == nullptr)
    {
        return helics_invalid_object;
    }
    cr->setCoreReadyToInit ();
    return helics_ok;
}

helics_status helicsCoreDisconnect (helics_core core)
{
    if (core == nullptr)
    {
        return helics_invalid_object;
    }
    auto cr = getCore (core);
    if (cr == nullptr)
    {
        return helics_invalid_object;
    }

    try
    {
        cr->disconnect ();
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

helics_status helicsBrokerDisconnect (helics_broker broker)
{
    if (broker == nullptr)
    {
        return helics_error;
    }
    auto brk = getBroker (broker);
    if (brk == nullptr)
    {
        return helics_invalid_object;
    }
    try
    {
        brk->disconnect ();
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

void helicsCoreFree (helics_core core)
{
    auto *coreObj = reinterpret_cast<helics::CoreObject *> (core);
    if (coreObj != nullptr)
    {
        getMasterHolder ()->clearCore (coreObj->index);
    }
    helics::CoreFactory::cleanUpCores ();
}

void helicsBrokerFree (helics_broker broker)
{
    auto *brokerObj = reinterpret_cast<helics::BrokerObject *> (broker);
    if (brokerObj != nullptr)
    {
        getMasterHolder ()->clearBroker (brokerObj->index);
    }
    helics::BrokerFactory::cleanUpBrokers ();
}

void helicsFederateFree (helics_federate fed)
{
    auto *fedObj = reinterpret_cast<helics::FedObject *> (fed);
    if (fedObj != nullptr)
    {
        getMasterHolder ()->clearFed (fedObj->index);
    }

    helics::CoreFactory::cleanUpCores ();
}

helics::FedObject::~FedObject ()
{
    //we want to remove the values in the arrays before deleting the fedptr
    // and we want to do it inside this function to ensure it does so in a consistent manner
    subs.clear();
    pubs.clear();
    epts.clear();
    filters.clear();
    fedptr = nullptr;
}

helics::CoreObject::~CoreObject ()
{
    filters.clear();
    coreptr = nullptr;
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
    helics::LoggerManager::closeLogger ();
    // helics::cleanupHelicsLibrary();
}

helics_query helicsCreateQuery (const char *target, const char *query)
{
    auto queryObj = new helics::queryObject;
    queryObj->query = (query != nullptr) ? std::string (query) : std::string ();
    queryObj->target = (target != nullptr) ? std::string (target) : std::string ();
    return reinterpret_cast<void *> (queryObj);
}

constexpr auto invalidStringConst = "#invalid";

const char *helicsQueryExecute (helics_query query, helics_federate fed)
{
    if (fed == nullptr)
    {
        return invalidStringConst;
    }
    if (query == nullptr)
    {
        return invalidStringConst;
    }
    auto fedObj = getFed (fed);
    if (fedObj == nullptr)
    {
        return invalidStringConst;
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

const char *helicsQueryCoreExecute(helics_query query, helics_core core)
{
    if (core == nullptr)
    {
        return invalidStringConst;
    }
    if (core == nullptr)
    {
        return invalidStringConst;
    }
    auto coreObj = getCore(core);
    if (coreObj == nullptr)
    {
        return invalidStringConst;
    }

    auto queryObj = reinterpret_cast<helics::queryObject *> (query);
    queryObj->response = coreObj->query(queryObj->target,queryObj->query);

    return queryObj->response.c_str();
}

const char *helicsQueryBrokerExecute(helics_query query, helics_broker broker)
{
    if (broker == nullptr)
    {
        return invalidStringConst;
    }
    if (broker == nullptr)
    {
        return invalidStringConst;
    }
    auto brokerObj = getCore(broker);
    if (brokerObj == nullptr)
    {
        return invalidStringConst;
    }

    auto queryObj = reinterpret_cast<helics::queryObject *> (query);
    queryObj->response = brokerObj->query(queryObj->target, queryObj->query);

    return queryObj->response.c_str();
}

helics_status helicsQueryExecuteAsync (helics_query query, helics_federate fed)
{
    if (fed == nullptr)
    {
        return helics_invalid_object;
    }
    if (query == nullptr)
    {
        return helics_invalid_object;
    }
    auto fedObj = getFedSharedPtr (fed);
    if (fedObj == nullptr)
    {
        return helics_invalid_object;
    }

    try
    {
        auto queryObj = reinterpret_cast<helics::queryObject *> (query);
        if (queryObj->target.empty ())
        {
            queryObj->asyncIndexCode = fedObj->queryAsync (queryObj->query);
        }
        else
        {
            queryObj->asyncIndexCode = fedObj->queryAsync (queryObj->target, queryObj->query);
        }
        queryObj->activeAsync = true;
        queryObj->activeFed = fedObj;
        return helics_ok;
    }
    catch (...)
    {
        return helicsErrorHandler ();
    }
}

const char *helicsQueryExecuteComplete (helics_query query)
{
    if (query == nullptr)
    {
        return invalidStringConst;
    }

    auto queryObj = reinterpret_cast<helics::queryObject *> (query);
    if (queryObj->asyncIndexCode != helics::invalid_id_value)
    {
        queryObj->response = queryObj->activeFed->queryComplete (queryObj->asyncIndexCode);
    }
    queryObj->activeAsync = false;
    queryObj->activeFed = nullptr;
    queryObj->asyncIndexCode = helics::invalid_id_value;
    return queryObj->response.c_str ();
}

helics_bool_t helicsQueryIsCompleted (helics_query query)
{
    if (query == nullptr)
    {
        return helics_false;
    }

    auto queryObj = reinterpret_cast<helics::queryObject *> (query);
    if (queryObj->asyncIndexCode != helics::invalid_id_value)
    {
        auto res = queryObj->activeFed->isQueryCompleted (queryObj->asyncIndexCode);
        return (res) ? helics_true : helics_false;
    }
    return helics_false;
}

void helicsQueryFree (helics_query query) { delete reinterpret_cast<helics::queryObject *> (query); }

void helicsCleanupHelicsLibrary ()
{
    helics::cleanupHelicsLibrary ();
    //  helics::LoggerManager::closeLogger();

    //  zmqContextManager::closeContext();
}

MasterObjectHolder::MasterObjectHolder () noexcept {}

MasterObjectHolder::~MasterObjectHolder ()
{
#if HELICS_HAVE_ZEROMQ > 0
    if (zmqContextManager::setContextToLeakOnDelete ())
    {
        zmqContextManager::getContext ().close ();
    }
#endif
    helics::LoggingCore::setFastShutdown ();
    deleteAll ();
    // std::cout << "end of master Object Holder destructor" << std::endl;
}
int MasterObjectHolder::addBroker (std::unique_ptr<helics::BrokerObject> broker)
{
    auto handle = brokers.lock ();
    auto index = static_cast<int> (handle->size ());
    broker->index = index;
    handle->push_back (std::move (broker));
    return index;
}

int MasterObjectHolder::addCore (std::unique_ptr<helics::CoreObject> core)
{
    auto handle = cores.lock ();
    auto index = static_cast<int> (handle->size ());
    core->index = index;
    handle->push_back (std::move (core));
    return index;
}

int MasterObjectHolder::addFed (std::unique_ptr<helics::FedObject> fed)
{
    auto handle = feds.lock ();
    auto index = static_cast<int> (handle->size ());
    fed->index = index;
    handle->push_back (std::move (fed));
    return index;
}

helics::FedObject *MasterObjectHolder::findFed (const std::string &fedName)
{
    auto handle = feds.lock ();
    for (auto &fed : (*handle))
    {
        if (fed->fedptr)
        {
            if (fed->fedptr->getName () == fedName)
            {
                return fed.get ();
            }
        }
    }
    return nullptr;
}

void MasterObjectHolder::clearBroker (int index)
{
    auto broker = brokers.lock ();
    if (index < static_cast<int> (broker->size ()))
    {
        (*broker)[index] = nullptr;
    }
}

void MasterObjectHolder::clearCore (int index)
{
    auto core = cores.lock ();
    if (index < static_cast<int> (core->size ()))
    {
        (*core)[index] = nullptr;
    }
}

void MasterObjectHolder::clearFed (int index)
{
    auto fed = feds.lock ();
    if (index < static_cast<int> (fed->size ()))
    {
        (*fed)[index] = nullptr;
    }
}

void MasterObjectHolder::deleteAll ()
{
    if (tripDetect.isTripped ())
    {
        return;
    }
    {
        auto brokerHandle = brokers.lock ();
        brokerHandle->clear ();
    }
    {
        auto coreHandle = cores.lock ();
        coreHandle->clear ();
    }
    auto fedHandle = feds.lock ();
    fedHandle->clear ();
}

std::shared_ptr<MasterObjectHolder> getMasterHolder ()
{
    static auto instance = std::make_shared<MasterObjectHolder> ();
    static tripwire::TripWireTrigger tripTriggerholder;
    return instance;
}

tripwire::TripWireTrigger tripTrigger;

void clearAllObjects ()
{
    auto v = getMasterHolder ();
    if (v)
    {
        v->deleteAll ();
    }
}
