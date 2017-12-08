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
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)
#include "json/json.h"
#pragma warning(pop)
#else
#include "json/json.h"
#endif

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
    else if (state == op_states::pendingInit)
    {
        enterInitializationStateFinalize ();
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
        state = op_states::pendingInit;
        asyncCallInfo->initFuture =
          std::async (std::launch::async, [this]() { coreObject->enterInitializingState (fedID); });
    }
    else if (state == op_states::pendingInit)
    {
        return;
    }
    else if (state != op_states::initialization)  // if we are already in initialization do nothing
    {
        throw (InvalidStateTransition ("cannot transition from current state to initialization state"));
    }
}

bool Federate::asyncOperationCompleted () const
{
    switch (state)
    {
    case op_states::pendingInit:
        return (asyncCallInfo->initFuture.wait_for (std::chrono::seconds (0)) == std::future_status::ready);
    case op_states::pendingExec:
        return (asyncCallInfo->execFuture.wait_for (std::chrono::seconds (0)) == std::future_status::ready);
    case op_states::pendingTime:
        return (asyncCallInfo->timeRequestFuture.wait_for (std::chrono::seconds (0)) == std::future_status::ready);
    case op_states::pendingIterativeTime:
        return (asyncCallInfo->timeRequestIterativeFuture.wait_for (std::chrono::seconds (0)) ==
                std::future_status::ready);
    default:
        return false;
    }
}

void Federate::enterInitializationStateFinalize ()
{
    if (state == op_states::pendingInit)
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
    case op_states::pendingInit:
        enterInitializationState ();
        FALLTHROUGH
    case op_states::initialization:
    {
        res = coreObject->enterExecutingState (fedID, iterate);
        switch (res)
        {
        case iteration_result::next_step:
            state = op_states::execution;
            InitializeToExecuteStateTransition();
            break;
        case iteration_result::iterating:
            state = op_states::initialization;
            updateTime(getCurrentTime(), getCurrentTime());
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
    case op_states::pendingExec:
        return enterExecutionStateFinalize ();
    case op_states::execution:
        // already in this state --> do nothing
        break;
    case op_states::pendingTime:
        requestTimeFinalize ();
        break;
    case op_states::pendingIterativeTime:  // since this isn't guaranteed to progress it shouldn't be called in
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
        state = op_states::pendingExec;
        asyncCallInfo->execFuture = std::async (std::launch::async, eExecFunc);
    }
    break;
    case op_states::pendingInit:
        enterInitializationStateFinalize ();
        // FALLTHROUGH
    case op_states::initialization:
    {
        if (!asyncCallInfo)
        {
            asyncCallInfo = std::make_unique<asyncFedCallInfo> ();
        }

        auto eExecFunc = [this, iterate]() { return coreObject->enterExecutingState (fedID, iterate); };
        state = op_states::pendingExec;
        asyncCallInfo->execFuture = std::async (std::launch::async, eExecFunc);
    }
    break;
    case op_states::pendingExec:
        break;
    case op_states::execution:
        // already in this state --> do nothing
        break;
    default:
        throw (InvalidStateTransition ("cannot transition from current state to execution state"));
        break;
    }
}

iteration_result Federate::enterExecutionStateFinalize ()
{
    if (state != op_states::pendingExec)
    {
        throw (InvalidFunctionCall ("cannot call finalize function without first calling async function"));
    }
    auto res = asyncCallInfo->execFuture.get ();
    switch (res)
    {
    case iteration_result::next_step:
        state = op_states::execution;
        InitializeToExecuteStateTransition();
        break;
    case iteration_result::iterating:
        state = op_states::initialization;
        updateTime(getCurrentTime(), getCurrentTime());
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

void Federate::setLookAhead (Time lookAhead)
{
    if (lookAhead < timeZero)
    {
        throw (InvalidParameterValue ("lookahead must be >=0"));
    }
    coreObject->setLookAhead (fedID, lookAhead);
}

void Federate::setImpactWindow (Time window)
{
    if (window < timeZero)
    {
        throw (InvalidParameterValue ("Impact Window must be >=0"));
    }
    coreObject->setImpactWindow (fedID, window);
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
    case op_states::pendingInit:
        enterInitializationStateFinalize ();
        break;
    case op_states::initialization:
        break;
    case op_states::pendingExec:
        enterExecutionStateFinalize ();
        break;
    case op_states::pendingTime:
        requestTimeFinalize ();
        break;
    case op_states::execution:
        break;
    case op_states::pendingIterativeTime:
        requestTimeIterativeFinalize ();  // I don't care about the return any more
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
    if (state != op_states::error)
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
            auto newTime = coreObject->timeRequest(fedID, nextInternalTimeStep);
            Time oldTime = currentTime;
            currentTime = newTime;
            updateTime(newTime, oldTime);
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
            updateTime(currentTime, oldTime);
            break;
        case iteration_result::halted:
            currentTime = iterativeTime.stepTime;
            updateTime(currentTime, oldTime);
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
        state = op_states::pendingTime;
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
        state = op_states::pendingIterativeTime;
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
Time Federate::requestTimeFinalize ()
{
    if (state == op_states::pendingTime)
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
iterationTime Federate::requestTimeIterativeFinalize ()
{
    if (state == op_states::pendingIterativeTime)
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
            updateTime(currentTime, oldTime);
            break;
        case iteration_result::halted:
            currentTime = iterativeTime.stepTime;
            updateTime(currentTime, oldTime);
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

int Federate::queryAsync (const std::string &target, const std::string &queryStr)
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

int Federate::queryAsync (const std::string &queryStr)
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

std::string Federate::queryFinalize (int queryIndex)
{
    if (asyncCallInfo)
    {
        auto fnd = asyncCallInfo->inFlightQueries.find (queryIndex);
        if (fnd != asyncCallInfo->inFlightQueries.end ())
        {
            return fnd->second.get ();
        }
    }
    return {"#invalid"};
}

bool Federate::queryCompleted (int queryIndex) const
{
    if (asyncCallInfo)
    {
        auto fnd = asyncCallInfo->inFlightQueries.find (queryIndex);
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
    if (id == invalid_Handle)
    {
        id = coreObject->getDestinationFilter (filterName);
    }
    return (id == invalid_Handle) ? invalid_id_value : filter_id_t (id);
}

filter_id_t Federate::getSourceFilterId (const std::string &filterName) const
{
    auto id = coreObject->getSourceFilter (filterName);
    return (id == invalid_Handle) ? invalid_id_value : filter_id_t (id);
}

filter_id_t Federate::getDestFilterId (const std::string &filterName) const
{
    auto id = coreObject->getDestinationFilter (filterName);
    return (id == invalid_Handle) ? invalid_id_value : filter_id_t (id);
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

FederateInfo LoadFederateInfo (const std::string &jsonString)
{
    FederateInfo fi;
    std::ifstream file (jsonString);
    Json_helics::Value doc;

    if (file.is_open ())
    {
        Json_helics::CharReaderBuilder rbuilder;
        std::string errs;
        bool ok = Json_helics::parseFromStream (rbuilder, file, &doc, &errs);
        if (!ok)
        {
            // should I throw an error here?
            return fi;
        }
    }
    else
    {
        Json_helics::CharReaderBuilder rbuilder;
        std::string errs;
        std::istringstream jstring (jsonString);
        bool ok = Json_helics::parseFromStream (rbuilder, jstring, &doc, &errs);
        if (!ok)
        {
            // should I throw an error here?
            return fi;
        }
    }

    if (doc.isMember ("name"))
    {
        fi.name = doc["name"].asString ();
    }

    if (doc.isMember ("observer"))
    {
        fi.observer = doc["observer"].asBool ();
    }
    if (doc.isMember ("rollback"))
    {
        fi.rollback = doc["rollback"].asBool ();
    }
    if (doc.isMember ("only_update_on_change"))
    {
        fi.only_update_on_change = doc["only_update_on_change"].asBool ();
    }
    if (doc.isMember ("only_transmit_on_change"))
    {
        fi.only_transmit_on_change = doc["only_transmit_on_change"].asBool ();
    }
    if (doc.isMember ("source_only"))
    {
        fi.source_only = doc["sourc_only"].asBool ();
    }
    if (doc.isMember ("uninterruptible"))
    {
        fi.uninterruptible = doc["uninterruptible"].asBool ();
    }
    if (doc.isMember ("interruptible"))  // can use either flag
    {
        fi.uninterruptible = !doc["uninterruptible"].asBool ();
    }
    if (doc.isMember ("forwardCompute"))
    {
        fi.forwardCompute = doc["forwardCompute"].asBool ();
    }
    if (doc.isMember ("coreType"))
    {
        try
        {
            fi.coreType = coreTypeFromString (doc["coreType"].asString ());
        }
        catch (const std::invalid_argument &ia)
        {
            std::cerr << "Unrecognized core type\n";
        }
    }
    if (doc.isMember ("coreName"))
    {
        fi.coreName = doc["coreName"].asString ();
    }
    if (doc.isMember ("coreInit"))
    {
        fi.coreInitString = doc["coreInit"].asString ();
    }
    if (doc.isMember ("maxiterations"))
    {
        fi.max_iterations = static_cast<int16_t> (doc["maxiterations"].asInt ());
    }
    if (doc.isMember ("period"))
    {
        if (doc["period"].isObject ())
        {
        }
        else
        {
            fi.timeDelta = doc["period"].asDouble ();
        }
    }

    if (doc.isMember ("timeDelta"))
    {
        if (doc["timeDelta"].isObject ())
        {
        }
        else
        {
            fi.timeDelta = doc["timeDelta"].asDouble ();
        }
    }

    if (doc.isMember ("lookAhead"))
    {
        if (doc["lookAhead"].isObject ())
        {
            // TODO:: something about units yet
        }
        else
        {
            fi.lookAhead = doc["lookAhead"].asDouble ();
        }
    }
    if (doc.isMember ("impactWindow"))
    {
        if (doc["impactWindow"].isObject ())
        {
            // TOOD:: something about units yet
        }
        else
        {
            fi.impactWindow = doc["impactWindow"].asDouble ();
        }
    }
    return fi;
}
}  // namespace helics
