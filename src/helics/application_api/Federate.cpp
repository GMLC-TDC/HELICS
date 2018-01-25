/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "Federate.hpp"
#include "Filters.hpp"
#include "../core/BrokerFactory.hpp"
#include "../core/CoreFactory.hpp"
#include "../core/core-exceptions.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../core/Core.hpp"
#include "AsyncFedCallInfo.hpp"
#include "helics/helics-config.h"

#include <cassert>
#include <iostream>

namespace helics
{
void cleanupHelicsLibrary ()
{
    BrokerFactory::cleanUpBrokers (200);
    CoreFactory::cleanUpCores (200);
}

Federate::Federate (const FederateInfo &fi) : FedInfo (fi)
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
    }
    // this call will throw an error on failure
    fedID = coreObject->registerFederate (fi.name, fi);

    currentTime = coreObject->getCurrentTime (fedID);
    asyncCallInfo = std::make_unique<AsyncFedCallInfo> ();
}

Federate::Federate (std::shared_ptr<Core> core, const FederateInfo &fi)
    : coreObject (std::move (core)), FedInfo (fi)
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
    if (fedID == helics::invalid_fed_id)
    {
        state = op_states::error;
        return;
    }
    currentTime = coreObject->getCurrentTime (fedID);
    asyncCallInfo = std::make_unique<AsyncFedCallInfo> ();
}

Federate::Federate (const std::string &jsonString) : Federate (LoadFederateInfo (jsonString))
{
    asyncCallInfo = std::make_unique<AsyncFedCallInfo> ();
    registerFilterInterfaces(jsonString);
   
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
    if (state == op_states::startup)
    {
        coreObject->enterInitializingState (fedID);
        state = op_states::initialization;
        currentTime = coreObject->getCurrentTime (fedID);
        startupToInitializeStateTransition ();
    }
    else if (state == op_states::pending_init)
    {
        enterInitializationStateComplete ();
    }
    else if (state != op_states::initialization)  // if we are already in initialization do nothing
    {
        throw (InvalidFunctionCall ("cannot transition from current state to initialization state"));
    }
}

void Federate::enterInitializationStateAsync ()
{
    if (state == op_states::startup)
    {
        state = op_states::pending_init;
        asyncCallInfo->initFuture =
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
    switch (state)
    {
    case op_states::pending_init:
        return (asyncCallInfo->initFuture.wait_for (std::chrono::seconds (0)) == std::future_status::ready);
    case op_states::pending_exec:
        return (asyncCallInfo->execFuture.wait_for (std::chrono::seconds (0)) == std::future_status::ready);
    case op_states::pending_time:
        return (asyncCallInfo->timeRequestFuture.wait_for (std::chrono::seconds (0)) == std::future_status::ready);
    case op_states::pending_iterative_time:
        return (asyncCallInfo->timeRequestIterativeFuture.wait_for (std::chrono::seconds (0)) ==
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
        asyncCallInfo->initFuture.get ();
        state = op_states::initialization;
        currentTime = coreObject->getCurrentTime (fedID);
        startupToInitializeStateTransition ();
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

iteration_result Federate::enterExecutionState (helics_iteration_request iterate)
{
    iteration_result res = iteration_result::next_step;
    switch (state)
    {
    case op_states::startup:
    case op_states::pending_init:
        enterInitializationState ();
        FALLTHROUGH
    case op_states::initialization:
    {
        res = coreObject->enterExecutingState (fedID, iterate);
        switch (res)
        {
        case iteration_result::next_step:
            state = op_states::execution;
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

void Federate::enterExecutionStateAsync (helics_iteration_request iterate)
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
        state = op_states::pending_exec;
        asyncCallInfo->execFuture = std::async (std::launch::async, eExecFunc);
    }
    break;
    case op_states::pending_init:
        enterInitializationStateComplete ();
        FALLTHROUGH
    case op_states::initialization:
    {
        auto eExecFunc = [this, iterate]() { return coreObject->enterExecutingState (fedID, iterate); };
        state = op_states::pending_exec;
        asyncCallInfo->execFuture = std::async (std::launch::async, eExecFunc);
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
    auto res = asyncCallInfo->execFuture.get ();
    switch (res)
    {
    case iteration_result::next_step:
        state = op_states::execution;
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

iteration_time Federate::requestTimeIterative (Time nextInternalTimeStep, helics_iteration_request iterate)
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
    if (state == op_states::execution)
    {
        state = op_states::pending_time;
        asyncCallInfo->timeRequestFuture = std::async (std::launch::async, [this, nextInternalTimeStep]() {
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
void Federate::requestTimeIterativeAsync (Time nextInternalTimeStep, helics_iteration_request iterate)
{
    if (state == op_states::execution)
    {
        state = op_states::pending_iterative_time;
        asyncCallInfo->timeRequestIterativeFuture =
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
    if (state == op_states::pending_time)
    {
        auto newTime = asyncCallInfo->timeRequestFuture.get ();
        state = op_states::execution;
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
    if (state == op_states::pending_iterative_time)
    {
        auto iterativeTime = asyncCallInfo->timeRequestIterativeFuture.get ();
        state = op_states::execution;
        Time oldTime = currentTime;
        switch (iterativeTime.state)
        {
        case iteration_result::next_step:
            currentTime = iterativeTime.grantedTime;
            FALLTHROUGH
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

void Federate::registerInterfaces(const std::string &jsonString)
{
    registerFilterInterfaces(jsonString);
}

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
                    registerSourceFilter(name, target, inputType, outputType);
                }
                else if ((mode == "dest") || (mode == "destination"))
                {
                    registerDestinationFilter(name, target, inputType, outputType);
                }
                else
                {
                    std::cerr << "filter mode " << mode << " is unrecognized no filter created\n";
                }
            }
            else
            {
                std::shared_ptr<Filter> filter;
                auto type = filterTypeFromString(operation);
                if (type == defined_filter_types::unrecognized)
                {
                    std::cerr << "unrecognized filter operation:"<<operation<<'\n';
                    continue;
                }
                if (mode == "source")
                {
                    filter = make_source_filter(type, this, target, name);
                }
                else if ((mode == "dest") || (mode == "destination"))
                {
                    filter = make_destination_filter(type, this, target, name);
                }
                else if ((mode == "clone") || (mode == "cloning"))
                {
                    //TODO:: do something with the name
                    auto clonefilt=std::make_shared<CloningFilter>(this);
                    clonefilt->addDeliveryEndpoint(target);
                    filter = std::move(clonefilt); 
                }
                else
                {
                    std::cerr << "filter mode " << mode << " is unrecognized no filter created\n";
                }

                if (filt.isMember("properties"))
                {
                    auto props = filt["properties"];
                    if (props.isArray())
                    {
                        for (const auto &prop : props)
                        {
                            if ((!prop.isMember("name")) && (!prop.isMember("value")))
                            {
                                std::cerr << "properties must be specified with name and value fields\n";
                                continue;
                            }
                            if (prop["value"].isDouble())
                            {
                                filter->set(prop["name"].asString(), prop["value"].asDouble());
                            }
                            else
                            {
                                filter->setString(prop["name"].asString(), prop["value"].asString());
                            }
                        }
                    }
                    else
                    {
                        if ((!props.isMember("name")) && (!props.isMember("value")))
                        {
                            std::cerr << "properties must be specified with name and value fields\n";
                            continue;
                        }
                        if (props["value"].isDouble())
                        {
                            filter->set(props["name"].asString(), props["value"].asDouble());
                        }
                        else
                        {
                            filter->setString(props["name"].asString(), props["value"].asString());
                        }
                    }
                    
                }
                addFilterObject(std::move(filter));
            }
            
        }
    }
}


std::shared_ptr<Filter> Federate::getFilterObject(int index)
{
    if (isValidIndex(index, localFilters))
    {
        return localFilters[index];
    }
    return nullptr;
}


void Federate::addFilterObject(std::shared_ptr<Filter> obj)
{
    localFilters.push_back(std::move(obj));
}

int Federate::filterObjectCount() const
{
    return static_cast<int>(localFilters.size());
}

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
    int cnt = asyncCallInfo->queryCounter++;

    auto queryFut =
      std::async (std::launch::async, [this, target, queryStr]() { return coreObject->query (target, queryStr); });
    asyncCallInfo->inFlightQueries.emplace (cnt, std::move (queryFut));
    return cnt;
}

query_id_t Federate::queryAsync (const std::string &queryStr)
{
    int cnt = asyncCallInfo->queryCounter++;

    auto queryFut = std::async (std::launch::async, [this, queryStr]() { return query (queryStr); });
    asyncCallInfo->inFlightQueries.emplace (cnt, std::move (queryFut));
    return cnt;
}

std::string Federate::queryComplete (query_id_t queryIndex)
{
    auto fnd = asyncCallInfo->inFlightQueries.find (queryIndex.value ());
    if (fnd != asyncCallInfo->inFlightQueries.end ())
    {
        return fnd->second.get ();
    }
    return {"#invalid"};
}

bool Federate::isQueryCompleted (query_id_t queryIndex) const
{
    auto fnd = asyncCallInfo->inFlightQueries.find (queryIndex.value ());
    if (fnd != asyncCallInfo->inFlightQueries.end ())
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
    return coreObject->registerSourceFilter (filterName, sourceEndpoint, inputType, outputType);
}

filter_id_t Federate::registerDestinationFilter (const std::string &filterName,
                                                 const std::string &destEndpoint,
                                                 const std::string &inputType,
                                                 const std::string &outputType)
{
    return coreObject->registerDestinationFilter (filterName, destEndpoint, inputType, outputType);
}

std::string Federate::getFilterName (filter_id_t id) const { return coreObject->getHandleName (id.value ()); }
std::string Federate::getFilterEndpoint (filter_id_t id) const { return coreObject->getTarget (id.value ()); }

std::string Federate::getFilterInputType (filter_id_t id) const { return coreObject->getType (id.value ()); }

std::string Federate::getFilterOutputType (filter_id_t id) const { return coreObject->getType (id.value ()); }

filter_id_t Federate::getFilterId (const std::string &filterName) const
{
    auto id = coreObject->getSourceFilter (filterName);
    if (id == invalid_handle)
    {
        id = coreObject->getDestinationFilter (filterName);
    }
    return (id == invalid_handle) ? invalid_id_value : filter_id_t (id);
}

filter_id_t Federate::getSourceFilterId (const std::string &filterName) const
{
    auto id = coreObject->getSourceFilter (filterName);
    return (id == invalid_handle) ? invalid_id_value : filter_id_t (id);
}

filter_id_t Federate::getDestFilterId (const std::string &filterName) const
{
    auto id = coreObject->getDestinationFilter (filterName);
    return (id == invalid_handle) ? invalid_id_value : filter_id_t (id);
}

void Federate::setFilterOperator (filter_id_t id, std::shared_ptr<FilterOperator> mo)
{
    coreObject->setFilterOperator (id.value (), std::move (mo));
}

void Federate::setFilterOperator (const std::vector<filter_id_t> &filter_ids, std::shared_ptr<FilterOperator> mo)
{
    for (auto id : filter_ids)
    {
        coreObject->setFilterOperator (id.value (), mo);
    }
}

}  // namespace helics
