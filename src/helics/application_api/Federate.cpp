/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "Federate.hpp"
#include "../core/BrokerFactory.hpp"
#include "../core/CoreFactory.hpp"
#include "../core/core-exceptions.hpp"
#include "Filters.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../core/Core.hpp"
#include "AsyncFedCallInfo.hpp"
#include "helics/helics-config.h"
#include "../common/GuardedTypes.hpp"

#include <cassert>
#include <iostream>

namespace helics
{
void cleanupHelicsLibrary ()
{
    BrokerFactory::cleanUpBrokers (200);
    CoreFactory::cleanUpCores (200);
}

Federate::Federate (const FederateInfo &fi) : Federate (fi.name, fi) {}

Federate::Federate (const std::string &name, const FederateInfo &fi) : FedInfo (fi)
{
    if (fi.coreName.empty ())
    {
        coreObject = CoreFactory::findJoinableCoreOfType (fi.coreType);
        if (!coreObject)
        {
            coreObject = CoreFactory::create (fi.coreType, fi.coreInitString);
        }
    }
    else
    {
        coreObject = CoreFactory::FindOrCreate (fi.coreType, fi.coreName, fi.coreInitString);
        if (!coreObject->isOpenToNewFederates ())
        {
            coreObject = nullptr;
            CoreFactory::cleanUpCores (200);
            coreObject = CoreFactory::FindOrCreate (fi.coreType, fi.coreName, fi.coreInitString);
            if (!coreObject->isOpenToNewFederates ())
            {
                throw (
                  RegistrationFailure ("Unable to connect to specified core: core is not open to new Federates"));
            }
        }
    }
    if (!coreObject)
    {
        throw (RegistrationFailure ("Unable to connect to specified core: unable to create specified core"));
    }
    /** make sure the core is connected */
    if (!coreObject->isConnected ())
    {
        coreObject->connect ();
        if (!coreObject->isConnected ())
        {
            throw (RegistrationFailure ("Unable to connect to broker->unable to register federate"));
        }
    }

    // this call will throw an error on failure
    fedID = coreObject->registerFederate (name, fi);
    separator_ = fi.separator;
    currentTime = coreObject->getCurrentTime (fedID);
    asyncCallInfo = std::make_unique<shared_guarded_m<AsyncFedCallInfo>> ();
}

Federate::Federate (const std::shared_ptr<Core> &core, const FederateInfo &fi) : coreObject (core), FedInfo (fi)
{
    if (!coreObject)
    {
        if (fi.coreName.empty ())
        {
            coreObject = CoreFactory::findJoinableCoreOfType (fi.coreType);
            if (!coreObject)
            {
                coreObject = CoreFactory::create (fi.coreType, fi.coreInitString);
            }
        }
        else
        {
            coreObject = CoreFactory::FindOrCreate (fi.coreType, fi.coreName, fi.coreInitString);
        }
    }

    if (!coreObject)
    {
        state = op_states::error;
        return;
    }
    /** make sure the core is connected */
    if (!coreObject->isConnected ())
    {
        coreObject->connect ();
    }
    fedID = coreObject->registerFederate (fi.name, fi);
    if (!fedID.isValid())
    {
        state = op_states::error;
        return;
    }
    separator_ = fi.separator;
    currentTime = coreObject->getCurrentTime (fedID);
    asyncCallInfo = std::make_unique<shared_guarded_m<AsyncFedCallInfo>> ();
}

Federate::Federate (const std::string &jsonString) : Federate (loadFederateInfo (jsonString))
{
    registerFilterInterfaces (jsonString);
}

Federate::Federate (const std::string &name, const std::string &jsonString)
    : Federate (loadFederateInfo (name, jsonString))
{
    registerFilterInterfaces (jsonString);
}

Federate::Federate () noexcept
{
    // this function needs to be defined for the virtual inheritance to compile but shouldn't actually be executed
}

Federate::Federate (Federate &&fed) noexcept
{
    auto tstate = fed.state.load ();
    state.store (tstate);
    fedID = fed.fedID;
    coreObject = std::move (fed.coreObject);
    currentTime = fed.currentTime;
    FedInfo = std::move (fed.FedInfo);
    separator_ = fed.separator_;
    asyncCallInfo = std::move (fed.asyncCallInfo);
}

Federate &Federate::operator= (Federate &&fed) noexcept
{
    auto tstate = fed.state.load ();
    state.store (tstate);
    fedID = fed.fedID;
    coreObject = std::move (fed.coreObject);
    currentTime = fed.currentTime;
    FedInfo = std::move (fed.FedInfo);
    separator_ = fed.separator_;
    asyncCallInfo = std::move (fed.asyncCallInfo);
    return *this;
}

Federate::~Federate ()
{
    if (coreObject)
    {
        finalize ();
    }
}

void Federate::enterInitializationState ()
{
    auto currentState = state.load();
    if (currentState == op_states::startup)
    {
        coreObject->enterInitializingState (fedID);
        state = op_states::initialization;
        currentTime = coreObject->getCurrentTime (fedID);
        startupToInitializeStateTransition ();
    }
    else if (currentState == op_states::pending_init)
    {
        enterInitializationStateComplete ();
    }
    else if (currentState != op_states::initialization)  // if we are already in initialization do nothing
    {
        throw (InvalidFunctionCall ("cannot transition from current state to initialization state"));
    }
}

void Federate::enterInitializationStateAsync ()
{
    auto asyncInfo = asyncCallInfo->lock();
    if (state == op_states::startup)
    {
        state = op_states::pending_init;
        asyncInfo->initFuture =
          std::async (std::launch::async, [this]() { coreObject->enterInitializingState (fedID); });
    }
    else if (state == op_states::pending_init)
    {
        return;
    }
    else if (state != op_states::initialization)  // if we are already in initialization do nothing
    {
        throw (InvalidFunctionCall ("cannot transition from current state to initialization state"));
    }
}

bool Federate::isAsyncOperationCompleted () const
{
    auto asyncInfo = asyncCallInfo->lock_shared();
    switch (state)
    {
    case op_states::pending_init:
        return (asyncInfo->initFuture.wait_for (std::chrono::seconds (0)) == std::future_status::ready);
    case op_states::pending_exec:
        return (asyncInfo->execFuture.wait_for (std::chrono::seconds (0)) == std::future_status::ready);
    case op_states::pending_time:
        return (asyncInfo->timeRequestFuture.wait_for (std::chrono::seconds (0)) == std::future_status::ready);
    case op_states::pending_iterative_time:
        return (asyncInfo->timeRequestIterativeFuture.wait_for (std::chrono::seconds (0)) ==
                std::future_status::ready);
    default:
        return false;
    }
}

void Federate::enterInitializationStateComplete ()
{
    
    switch (state)
    {
    case op_states::pending_init:
    {
        auto asyncInfo = asyncCallInfo->lock();
        asyncInfo->initFuture.get ();
        state = op_states::initialization;
        currentTime = coreObject->getCurrentTime (fedID);
        startupToInitializeStateTransition ();
    }
    break;
    case op_states::initialization:
        break;
    case op_states::startup:
        enterInitializationState ();
        break;
    default:
        throw (InvalidFunctionCall ("cannot call Initialization Complete function without first calling "
                                    "enterInitializationStateAsync function"));
    }
}

iteration_result Federate::enterExecutionState (iteration_request iterate)
{
    iteration_result res = iteration_result::next_step;
    switch (state)
    {
    case op_states::startup:
    case op_states::pending_init:
        enterInitializationState ();
        FALLTHROUGH
        /* FALLTHROUGH */
    case op_states::initialization:
    {
        res = coreObject->enterExecutingState (fedID, iterate);
        switch (res)
        {
        case iteration_result::next_step:
            state = op_states::execution;
            currentTime = timeZero;
            initializeToExecuteStateTransition ();
            break;
        case iteration_result::iterating:
            state = op_states::initialization;
            updateTime (getCurrentTime (), getCurrentTime ());
            break;
        case iteration_result::error:
            state = op_states::error;
            break;
        case iteration_result::halted:
            state = op_states::finalize;
            break;
        }
        break;
    }
    case op_states::pending_exec:
        return enterExecutionStateComplete ();
    case op_states::execution:
        // already in this state --> do nothing
        break;
    case op_states::pending_time:
        requestTimeComplete ();
        break;
    case op_states::pending_iterative_time:  // since this isn't guaranteed to progress it shouldn't be called in
                                             // this fashion
    default:
        throw (InvalidFunctionCall ("cannot transition from current state to execution state"));
        break;
    }
    return res;
}

void Federate::enterExecutionStateAsync (iteration_request iterate)
{
    
    switch (state)
    {
    case op_states::startup:
    {
        auto eExecFunc = [this, iterate]() {
            coreObject->enterInitializingState (fedID);
            startupToInitializeStateTransition ();
            return coreObject->enterExecutingState (fedID, iterate);
        };
        auto asyncInfo = asyncCallInfo->lock();
        state = op_states::pending_exec;
        asyncInfo->execFuture = std::async (std::launch::async, eExecFunc);
    }
    break;
    case op_states::pending_init:
        enterInitializationStateComplete ();
        FALLTHROUGH
        /* FALLTHROUGH */
    case op_states::initialization:
    {
        auto eExecFunc = [this, iterate]() { return coreObject->enterExecutingState (fedID, iterate); };
        auto asyncInfo = asyncCallInfo->lock();
        state = op_states::pending_exec;
        asyncInfo->execFuture = std::async (std::launch::async, eExecFunc);
    }
    break;
    case op_states::pending_exec:
        break;
    case op_states::execution:
        // already in this state --> do nothing
        break;
    default:
        throw (InvalidFunctionCall ("cannot transition from current state to execution state"));
        break;
    }
}

iteration_result Federate::enterExecutionStateComplete ()
{
    
    if (state != op_states::pending_exec)
    {
        throw (InvalidFunctionCall ("cannot call finalize function without first calling async function"));
    }
    auto asyncInfo = asyncCallInfo->lock();
    auto res = asyncInfo->execFuture.get ();
    switch (res)
    {
    case iteration_result::next_step:
        state = op_states::execution;
        currentTime = timeZero;
        initializeToExecuteStateTransition ();
        break;
    case iteration_result::iterating:
        state = op_states::initialization;
        updateTime (getCurrentTime (), getCurrentTime ());
        break;
    case iteration_result::error:
        state = op_states::error;
        break;
    case iteration_result::halted:
        state = op_states::finalize;
        break;
    }

    return res;
}

void Federate::setTimeDelta (Time tdelta)
{
    if (tdelta < timeZero)
    {
        throw (InvalidParameter ("timeDelta must be >=0"));
    }
    coreObject->setTimeDelta (fedID, tdelta);
}

void Federate::setOutputDelay (Time outputDelay)
{
    if (outputDelay < timeZero)
    {
        throw (InvalidParameter ("outputDelay must be >=0"));
    }
    coreObject->setOutputDelay (fedID, outputDelay);
}

void Federate::setInputDelay (Time window)
{
    if (window < timeZero)
    {
        throw (InvalidParameter ("Input Delay must be >=0"));
    }
    coreObject->setInputDelay (fedID, window);
}

void Federate::setPeriod (Time period, Time offset)
{
    if (period < timeZero)
    {
        throw (InvalidParameter ("period must be >=0"));
    }

    coreObject->setPeriod (fedID, period);
    coreObject->setTimeOffset (fedID, offset);
}

void Federate::setLoggingLevel (int loggingLevel) { coreObject->setLoggingLevel (fedID, loggingLevel); }

void Federate::setMaxIterations (int maxIterations) { coreObject->setMaximumIterations (fedID, maxIterations); }
void Federate::setLoggingCallback (
  const std::function<void(int, const std::string &, const std::string &)> &logFunction)
{
    coreObject->setLoggingCallback (fedID, logFunction);
}

void Federate::setFlag (int flag, bool flagValue)
{
    if ((flag > 10) || (flag < 0))
    {
        throw (InvalidParameter ("flag must be between 0 and 10"));
    }
    switch (flag)
    {
    case ROLLBACK_FLAG:
        FedInfo.rollback = flagValue;
        break;
    case FORWARD_COMPUTE_FLAG:
        FedInfo.forwardCompute = flagValue;
        break;
    default:
        coreObject->setFlag (fedID, flag, flagValue);
        break;
    }
}
void Federate::finalize ()
{
    switch (state)
    {
    case op_states::startup:
        break;
    case op_states::pending_init:
        enterInitializationStateComplete ();
        break;
    case op_states::initialization:
        break;
    case op_states::pending_exec:
        enterExecutionStateComplete ();
        break;
    case op_states::pending_time:
        requestTimeComplete ();
        break;
    case op_states::execution:
        break;
    case op_states::pending_iterative_time:
        requestTimeIterativeComplete ();  // I don't care about the return any more
        break;
    case op_states::finalize:
    case op_states::error:
        return;
        // do nothing
    default:
        throw (InvalidFunctionCall ("cannot call finalize in present state"));
    }
    coreObject->finalize (fedID);
    state = op_states::finalize;
}

void Federate::disconnect ()
{
    if (coreObject)
    {
        coreObject->finalize (fedID);
    }
    state = op_states::finalize;
    coreObject = nullptr;
}

void Federate::error (int errorcode)
{
    state = op_states::error;
    std::string errorString = "error " + std::to_string (errorcode) + " in federate " + FedInfo.name;
    coreObject->logMessage (fedID, errorcode, errorString);
}

void Federate::error (int errorcode, const std::string &message)
{
    state = op_states::error;
    coreObject->logMessage (fedID, errorcode, message);
}

Time Federate::requestTime (Time nextInternalTimeStep)
{
    if (state == op_states::execution)
    {
        try
        {
            auto newTime = coreObject->timeRequest (fedID, nextInternalTimeStep);
            Time oldTime = currentTime;
            currentTime = newTime;
            updateTime (newTime, oldTime);
            return newTime;
        }
        catch (FunctionExecutionFailure &fee)
        {
            state = op_states::error;
            throw;
        }
    }
    else
    {
        throw (InvalidFunctionCall ("cannot call request time in present state"));
    }
}

iteration_time Federate::requestTimeIterative (Time nextInternalTimeStep, iteration_request iterate)
{
    if (state == op_states::execution)
    {
        auto iterativeTime = coreObject->requestTimeIterative (fedID, nextInternalTimeStep, iterate);
        Time oldTime = currentTime;
        switch (iterativeTime.state)
        {
        case iteration_result::next_step:
            currentTime = iterativeTime.grantedTime;
            FALLTHROUGH
            /* FALLTHROUGH */
        case iteration_result::iterating:
            updateTime (currentTime, oldTime);
            break;
        case iteration_result::halted:
            currentTime = iterativeTime.grantedTime;
            updateTime (currentTime, oldTime);
            state = op_states::finalize;
            break;
        case iteration_result::error:
            state = op_states::error;
            break;
        }
        return iterativeTime;
    }
    else
    {
        throw (InvalidFunctionCall ("cannot call request time in present state"));
    }
}

void Federate::requestTimeAsync (Time nextInternalTimeStep)
{
   
    auto exp = op_states::execution;
    if (state.compare_exchange_strong(exp,op_states::pending_time))
    {
        auto asyncInfo = asyncCallInfo->lock();
        asyncInfo->timeRequestFuture = std::async (std::launch::async, [this, nextInternalTimeStep]() {
            return coreObject->timeRequest (fedID, nextInternalTimeStep);
        });
    }
    else
    {
        throw (InvalidFunctionCall ("cannot call request time in present state"));
    }
}

/** request a time advancement
@param[in] the next requested time step
@return the granted time step*/
void Federate::requestTimeIterativeAsync (Time nextInternalTimeStep, iteration_request iterate)
{
   
    auto exp = op_states::execution;
    if (state.compare_exchange_strong(exp, op_states::pending_iterative_time))
    {
        auto asyncInfo = asyncCallInfo->lock();
        asyncInfo->timeRequestIterativeFuture =
          std::async (std::launch::async, [this, nextInternalTimeStep, iterate]() {
              return coreObject->requestTimeIterative (fedID, nextInternalTimeStep, iterate);
          });
    }
    else
    {
        throw (InvalidFunctionCall ("cannot call request time in present state"));
    }
}

/** request a time advancement
@param[in] the next requested time step
@return the granted time step*/
Time Federate::requestTimeComplete ()
{
    
    auto exp = op_states::pending_time;
    if (state.compare_exchange_strong(exp, op_states::execution))
    {

        auto asyncInfo = asyncCallInfo->lock();
        auto newTime = asyncInfo->timeRequestFuture.get ();
        asyncInfo = nullptr; //remove the lock;
        Time oldTime = currentTime;
        currentTime = newTime;
        updateTime (newTime, oldTime);
        return newTime;
    }
    else
    {
        throw (InvalidFunctionCall (
          "cannot call finalize requestTime without first calling requestTimeIterative function"));
    }
}

/** finalize the time advancement request
@return the granted time step*/
iteration_time Federate::requestTimeIterativeComplete ()
{
    auto asyncInfo = asyncCallInfo->lock();
    auto exp = op_states::pending_iterative_time;
    if (state.compare_exchange_strong(exp, op_states::execution))
    {
        auto iterativeTime = asyncInfo->timeRequestIterativeFuture.get ();
        Time oldTime = currentTime;
        switch (iterativeTime.state)
        {
        case iteration_result::next_step:
            currentTime = iterativeTime.grantedTime;
            FALLTHROUGH
            /* FALLTHROUGH */
        case iteration_result::iterating:
            updateTime (currentTime, oldTime);
            break;
        case iteration_result::halted:
            currentTime = iterativeTime.grantedTime;
            updateTime (currentTime, oldTime);
            state = op_states::finalize;
            break;
        case iteration_result::error:
            state = op_states::error;
            break;
        }
        return iterativeTime;
    }
    else
    {
        throw (InvalidFunctionCall (
          "cannot call finalize requestTimeIterative without first calling requestTimeIterativeAsync function"));
    }
}

void Federate::updateTime (Time /*newTime*/, Time /*oldTime*/)
{
    // child classes would likely implement this
}

void Federate::startupToInitializeStateTransition ()
{
    // child classes may do something with this
}
void Federate::initializeToExecuteStateTransition ()
{
    // child classes may do something with this
}

void Federate::registerInterfaces (const std::string &jsonString) { registerFilterInterfaces (jsonString); }

void Federate::registerFilterInterfaces (const std::string &jsonString)
{
    auto doc = loadJsonString (jsonString);

    if (doc.isMember ("filters"))
    {
        for (const auto &filt : doc["filters"])
        {
            std::string name;
            bool useTypes = false;
            if (filt.isMember ("name"))
            {
                name = filt["name"].asString ();
            }
            std::string target;
            if (filt.isMember ("target"))
            {
                target = filt["target"].asString ();
            }
            std::string inputType;
            if (filt.isMember ("inputType"))
            {
                inputType = filt["inputType"].asString ();
                useTypes = true;
            }
            std::string outputType;
            if (filt.isMember ("outputType"))
            {
                outputType = filt["outputType"].asString ();
                useTypes = true;
            }

            std::string mode = "source";
            if (filt.isMember ("mode"))
            {
                mode = filt["mode"].asString ();
            }
            std::string operation ("custom");
            if (filt.isMember ("operation"))
            {
                operation = filt["operation"].asString ();
                if ((mode == "clone") || (mode == "cloning"))
                {
                    if (operation != "clone")
                    {
                        std::cerr << "the only valid operation with cloning filter is \"clone\"\n";
                        continue;
                    }
                }
            }
            if ((useTypes) && (operation != "custom"))
            {
                std::cerr << "input and output types may only be specified for custom filters\n";
                continue;
            }
            if (useTypes)
            {
                if (mode == "source")
                {
                    registerSourceFilter (name, target, inputType, outputType);
                }
                else if ((mode == "dest") || (mode == "destination"))
                {
                    registerDestinationFilter (name, target, inputType, outputType);
                }
                else
                {
                    std::cerr << "filter mode " << mode << " is unrecognized no filter created\n";
                }
            }
            else
            {
                std::shared_ptr<Filter> filter;
                auto type = filterTypeFromString (operation);
                if (type == defined_filter_types::unrecognized)
                {
                    std::cerr << "unrecognized filter operation:" << operation << '\n';
                    continue;
                }
                if (mode == "source")
                {
                    filter = make_source_filter (type, this, target, name);
                }
                else if ((mode == "dest") || (mode == "destination"))
                {
                    filter = make_destination_filter (type, this, target, name);
                }
                else if ((mode == "clone") || (mode == "cloning"))
                {
                    // TODO:: do something with the name
                    auto clonefilt = std::make_shared<CloningFilter> (this);
                    clonefilt->addDeliveryEndpoint (target);
                    filter = std::move (clonefilt);
                }
                else
                {
                    std::cerr << "filter mode " << mode << " is unrecognized no filter created\n";
                }

                if (filt.isMember ("properties"))
                {
                    auto props = filt["properties"];
                    if (props.isArray ())
                    {
                        for (const auto &prop : props)
                        {
                            if ((!prop.isMember ("name")) && (!prop.isMember ("value")))
                            {
                                std::cerr << "properties must be specified with name and value fields\n";
                                continue;
                            }
                            if (prop["value"].isDouble ())
                            {
                                filter->set (prop["name"].asString (), prop["value"].asDouble ());
                            }
                            else
                            {
                                filter->setString (prop["name"].asString (), prop["value"].asString ());
                            }
                        }
                    }
                    else
                    {
                        if ((!props.isMember ("name")) && (!props.isMember ("value")))
                        {
                            std::cerr << "properties must be specified with name and value fields\n";
                            continue;
                        }
                        if (props["value"].isDouble ())
                        {
                            filter->set (props["name"].asString (), props["value"].asDouble ());
                        }
                        else
                        {
                            filter->setString (props["name"].asString (), props["value"].asString ());
                        }
                    }
                }
                addFilterObject (std::move (filter));
            }
        }
    }
}

std::shared_ptr<Filter> Federate::getFilterObject (int index)
{
    if (isValidIndex (index, localFilters))
    {
        return localFilters[index];
    }
    return nullptr;
}

void Federate::addFilterObject (std::shared_ptr<Filter> obj) { localFilters.push_back (std::move (obj)); }

int Federate::filterObjectCount () const { return static_cast<int> (localFilters.size ()); }

std::string Federate::query (const std::string &queryStr)
{
    if (queryStr == "name")
    {
        return getName ();
    }
    return coreObject->query ("federation", queryStr);
}

std::string Federate::query (const std::string &target, const std::string &queryStr)
{
    return coreObject->query (target, queryStr);
}

query_id_t Federate::queryAsync (const std::string &target, const std::string &queryStr)
{
    auto queryFut =
        std::async(std::launch::async, [this, target, queryStr]() { return coreObject->query(target, queryStr); });
    auto asyncInfo = asyncCallInfo->lock();
    int cnt = asyncInfo->queryCounter++;
   
    asyncInfo->inFlightQueries.emplace (cnt, std::move (queryFut));
    return cnt;
}

query_id_t Federate::queryAsync (const std::string &queryStr)
{
    auto queryFut = std::async(std::launch::async, [this, queryStr]() { return query(queryStr); });
    auto asyncInfo = asyncCallInfo->lock();
    int cnt = asyncInfo->queryCounter++;

    
    asyncInfo->inFlightQueries.emplace (cnt, std::move (queryFut));
    return cnt;
}

std::string Federate::queryComplete (query_id_t queryIndex)
{
    auto asyncInfo = asyncCallInfo->lock();
    auto fnd = asyncInfo->inFlightQueries.find (queryIndex.value ());
    if (fnd != asyncInfo->inFlightQueries.end ())
    {
        return fnd->second.get ();
    }
    return {"#invalid"};
}

bool Federate::isQueryCompleted (query_id_t queryIndex) const
{
    auto asyncInfo = asyncCallInfo->lock();
    auto fnd = asyncInfo->inFlightQueries.find (queryIndex.value ());
    if (fnd != asyncInfo->inFlightQueries.end ())
    {
        return (fnd->second.wait_for (std::chrono::seconds (0)) == std::future_status::ready);
    }
    return false;
}

filter_id_t Federate::registerSourceFilter (const std::string &filterName,
                                            const std::string &sourceEndpoint,
                                            const std::string &inputType,
                                            const std::string &outputType)
{
    return filter_id_t(coreObject->registerSourceFilter (filterName, sourceEndpoint, inputType, outputType));
}

filter_id_t Federate::registerDestinationFilter (const std::string &filterName,
                                                 const std::string &destEndpoint,
                                                 const std::string &inputType,
                                                 const std::string &outputType)
{
    return filter_id_t(coreObject->registerDestinationFilter (filterName, destEndpoint, inputType, outputType));
}

filter_id_t Federate::registerCloningSourceFilter (const std::string &filterName,
                                                   const std::string &sourceEndpoint,
                                                   const std::string &inputType,
                                                   const std::string &outputType)
{
    return filter_id_t(coreObject->registerCloningSourceFilter (filterName, sourceEndpoint, inputType, outputType));
}

filter_id_t Federate::registerCloningDestinationFilter (const std::string &filterName,
                                                        const std::string &destEndpoint,
                                                        const std::string &inputType,
                                                        const std::string &outputType)
{
    return filter_id_t(coreObject->registerCloningDestinationFilter (filterName, destEndpoint, inputType, outputType));
}

std::string Federate::getFilterName (filter_id_t id) const { return coreObject->getHandleName (handle_id_t(id.value ())); }
std::string Federate::getFilterEndpoint (filter_id_t id) const { return coreObject->getTarget (handle_id_t(id.value ())); }

std::string Federate::getFilterInputType (filter_id_t id) const { return coreObject->getType (handle_id_t(id.value ())); }

std::string Federate::getFilterOutputType (filter_id_t id) const { return coreObject->getType (handle_id_t(id.value ())); }

filter_id_t Federate::getFilterId (const std::string &filterName) const
{
    auto id = coreObject->getSourceFilter (filterName);
    if (!id.isValid())
    {
        id = coreObject->getDestinationFilter (filterName);
    }
    return (id.isValid()) ? filter_id_t(id):invalid_id_value ;
}

filter_id_t Federate::getSourceFilterId (const std::string &filterName) const
{
    auto id = coreObject->getSourceFilter (filterName);
    return (id.isValid()) ? filter_id_t(id) : invalid_id_value;
}

filter_id_t Federate::getDestFilterId (const std::string &filterName) const
{
    auto id = coreObject->getDestinationFilter (filterName);
    return (id.isValid()) ? filter_id_t(id) : invalid_id_value;
}

void Federate::setFilterOperator (filter_id_t id, std::shared_ptr<FilterOperator> mo)
{
    coreObject->setFilterOperator (handle_id_t(id.value ()), std::move (mo));
}

void Federate::setFilterOperator (const std::vector<filter_id_t> &filter_ids, std::shared_ptr<FilterOperator> mo)
{
    for (auto id : filter_ids)
    {
        coreObject->setFilterOperator (handle_id_t(id.value ()), mo);
    }
}

}  // namespace helics
