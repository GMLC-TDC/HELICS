/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "Federate.h"
#include "../core/BrokerFactory.h"
#include "../core/CoreFactory.h"
#include "../core/core-exceptions.h"

#include "../core/core.h"
#include "asyncFedCallInfo.h"
#include "helics/helics-config.h"


#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>

namespace helics
{
std::string getHelicsVersionString ()
{
    std::string vstr = std::to_string (HELICS_VERSION_MAJOR);
    vstr.push_back ('.');
    vstr.append (std::to_string (HELICS_VERSION_MINOR));
    vstr.push_back ('.');
    vstr.append (std::to_string (HELICS_VERSION_PATCH));
    vstr.push_back (' ');
    vstr.push_back ('(');
    vstr += HELICS_DATE;
    vstr.push_back (')');
    return vstr;
}

int getHelicsVersionMajor () { return HELICS_VERSION_MAJOR; }

int getHelicsVersionMinor () { return HELICS_VERSION_MINOR; }
int getHelicsVersionPatch () { return HELICS_VERSION_PATCH; }

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
        if (!coreObject->isOpenToNewFederates())
        {
            coreObject = nullptr;
            CoreFactory::cleanUpCores(200);
            coreObject = CoreFactory::FindOrCreate(fi.coreType, fi.coreName, fi.coreInitString);
            if (!coreObject->isOpenToNewFederates())
            {
                throw(registrationFailure("Unable to connect to specified core: core is not open to new Federates"));
    }
        }
    }
    if (!coreObject)
    {
        throw(registrationFailure("Unable to connect to specified core: unable to create specified core"));
    }
    /** make sure the core is connected */
    if (!coreObject->isConnected ())
    {
        coreObject->connect ();
    }
    //this call will throw an error on failure
    fedID = coreObject->registerFederate (fi.name, fi);
   
    currentTime = coreObject->getCurrentTime (fedID);
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
}

Federate::Federate (const std::string &jsonString) : Federate (LoadFederateInfo (jsonString)) {}

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
    if (asyncCallInfo)
    {
        asyncCallInfo = std::move (fed.asyncCallInfo);
    }
}

Federate &Federate::operator= (Federate &&fed) noexcept
{
    auto tstate = fed.state.load ();
    state.store (tstate);
    fedID = fed.fedID;
    coreObject = std::move (fed.coreObject);
    currentTime = fed.currentTime;
    FedInfo = std::move (fed.FedInfo);
    if (asyncCallInfo)
    {
        asyncCallInfo = std::move (fed.asyncCallInfo);
    }
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
        StartupToInitializeStateTransition ();
    }
    else if (state == op_states::pending_init)
    {
        enterInitializationStateComplete ();
    }
    else if (state != op_states::initialization)  // if we are already in initialization do nothing
    {
        throw (InvalidStateTransition ("cannot transition from current state to initialization state"));
    }
}

void Federate::enterInitializationStateAsync ()
{
    if (state == op_states::startup)
    {
        if (!asyncCallInfo)
        {
            asyncCallInfo = std::make_unique<asyncFedCallInfo> ();
        }
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
        throw (InvalidStateTransition ("cannot transition from current state to initialization state"));
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
    if (state == op_states::pending_init)
    {
        asyncCallInfo->initFuture.get ();
        state = op_states::initialization;
        currentTime = coreObject->getCurrentTime (fedID);
        StartupToInitializeStateTransition ();
    }
    else
    {
        throw (InvalidFunctionCall ("cannot call finalize function without first calling async function"));
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
    case op_states::initialization:
    {
        res = coreObject->enterExecutingState (fedID, iterate);
        switch (res)
        {
        case iteration_result::next_step:
            state = op_states::execution;
            InitializeToExecuteStateTransition ();
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
        requestTimeComplete();
        break;
    case op_states::pending_iterative_time:  // since this isn't guaranteed to progress it shouldn't be called in
                                           // this fashion
    default:
        throw (InvalidStateTransition ("cannot transition from current state to execution state"));
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
        if (!asyncCallInfo)
        {
            asyncCallInfo = std::make_unique<asyncFedCallInfo> ();
        }

        auto eExecFunc = [this, iterate]() {
            coreObject->enterInitializingState (fedID);
            StartupToInitializeStateTransition ();
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
        if (!asyncCallInfo)
        {
            asyncCallInfo = std::make_unique<asyncFedCallInfo> ();
        }

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
        throw (InvalidStateTransition ("cannot transition from current state to execution state"));
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
        InitializeToExecuteStateTransition ();
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
        throw (InvalidParameterValue ("timeDelta must be >=0"));
    }
    coreObject->setTimeDelta (fedID, tdelta);
}

void Federate::setOutputDelay (Time outputDelay)
{
    if (outputDelay < timeZero)
    {
        throw (InvalidParameterValue ("outputDelay must be >=0"));
    }
    coreObject->setOutputDelay (fedID, outputDelay);
}

void Federate::setInputDelay (Time window)
{
    if (window < timeZero)
    {
        throw (InvalidParameterValue ("Input Delay must be >=0"));
    }
    coreObject->setInputDelay (fedID, window);
}

void Federate::setPeriod (Time period, Time offset)
{
    if (period < timeZero)
    {
        throw (InvalidParameterValue ("period must be >=0"));
    }

    coreObject->setPeriod (fedID, period);
    coreObject->setTimeOffset (fedID, offset);
}

void Federate::setLoggingLevel (int loggingLevel) { coreObject->setLoggingLevel (fedID, loggingLevel); }

void Federate::setFlag (int flag, bool flagValue)
{
    if ((flag > 10) || (flag < 0))
    {
        throw (InvalidParameterValue ("flag must be between 0 and 10"));
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
            coreObject->finalize(fedID);
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
        catch (functionExecutionFailure &fee)
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

iterationTime Federate::requestTimeIterative (Time nextInternalTimeStep, iteration_request iterate)
{
    if (state == op_states::execution)
    {
        auto iterativeTime = coreObject->requestTimeIterative (fedID, nextInternalTimeStep, iterate);
        Time oldTime = currentTime;
        switch (iterativeTime.state)
        {
        case iteration_result::next_step:
            currentTime = iterativeTime.stepTime;
            FALLTHROUGH
        case iteration_result::iterating:
            updateTime (currentTime, oldTime);
            break;
        case iteration_result::halted:
            currentTime = iterativeTime.stepTime;
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
        if (!asyncCallInfo)
        {
            asyncCallInfo = std::make_unique<asyncFedCallInfo> ();
        }
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
void Federate::requestTimeIterativeAsync (Time nextInternalTimeStep, iteration_request iterate)
{
    if (state == op_states::execution)
    {
        if (!asyncCallInfo)
        {
            asyncCallInfo = std::make_unique<asyncFedCallInfo> ();
        }
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
iterationTime Federate::requestTimeIterativeComplete ()
{
    if (state == op_states::pending_iterative_time)
    {
        auto iterativeTime = asyncCallInfo->timeRequestIterativeFuture.get ();
        state = op_states::execution;
        Time oldTime = currentTime;
        switch (iterativeTime.state)
        {
        case iteration_result::next_step:
            currentTime = iterativeTime.stepTime;
            FALLTHROUGH
        case iteration_result::iterating:
            updateTime (currentTime, oldTime);
            break;
        case iteration_result::halted:
            currentTime = iterativeTime.stepTime;
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

void Federate::StartupToInitializeStateTransition ()
{
    // child classes may do something with this
}
void Federate::InitializeToExecuteStateTransition ()
{
    // child classes may do something with this
}

void Federate::registerInterfaces (const std::string & /*jsonString*/)
{
    // child classes would implement this
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
    if (!asyncCallInfo)
    {
        asyncCallInfo = std::make_unique<asyncFedCallInfo> ();
    }
    int cnt = asyncCallInfo->queryCounter++;

    auto queryFut =
      std::async (std::launch::async, [this, target, queryStr]() { return coreObject->query (target, queryStr); });
    asyncCallInfo->inFlightQueries.emplace (cnt, std::move (queryFut));
    return cnt;
}

query_id_t Federate::queryAsync (const std::string &queryStr)
{
    if (!asyncCallInfo)
    {
        asyncCallInfo = std::make_unique<asyncFedCallInfo> ();
    }
    int cnt = asyncCallInfo->queryCounter++;

    auto queryFut = std::async (std::launch::async, [this, queryStr]() { return query (queryStr); });
    asyncCallInfo->inFlightQueries.emplace (cnt, std::move (queryFut));
    return cnt;
}

std::string Federate::queryComplete (query_id_t queryIndex)
{
    if (asyncCallInfo)
    {
        auto fnd = asyncCallInfo->inFlightQueries.find (queryIndex.value());
        if (fnd != asyncCallInfo->inFlightQueries.end ())
        {
            return fnd->second.get ();
        }
    }
    return {"#invalid"};
}

bool Federate::isQueryCompleted (query_id_t queryIndex) const
{
    if (asyncCallInfo)
    {
        auto fnd = asyncCallInfo->inFlightQueries.find (queryIndex.value());
        if (fnd != asyncCallInfo->inFlightQueries.end ())
        {
            return (fnd->second.wait_for (std::chrono::seconds (0)) == std::future_status::ready);
        }
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
