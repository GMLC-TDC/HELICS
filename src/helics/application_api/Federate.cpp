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

#include "../common/GuardedTypes.hpp"
#include "../common/JsonProcessingFunctions.hpp"
#include "../common/TomlProcessingFunctions.hpp"
#include "../core/Core.hpp"
#include "AsyncFedCallInfo.hpp"
#include "helics/helics-config.h"

#include "FilterFederateManager.hpp"

#include <cassert>
#include <iostream>

namespace helics
{
using namespace std::literals::chrono_literals;
void cleanupHelicsLibrary ()
{
    std::cerr << "cleaing brokers 1" << std::endl;
    auto sz=BrokerFactory::cleanUpBrokers (100ms);
    std::cerr << sz<<" brokers remaining now cleaning cores" << std::endl;
    sz=CoreFactory::cleanUpCores (200ms);
    std::cerr << sz<<" core remain now cleaning brokers 2" << std::endl;
    sz=BrokerFactory::cleanUpBrokers (100ms);
    std::cerr << sz<< " broker remain cleaning finished" << std::endl;
}

Federate::Federate (const std::string &fedName, const FederateInfo &fi) : name (fedName)
{
    if (fi.coreName.empty ())
    {
        coreObject = CoreFactory::findJoinableCoreOfType (fi.coreType);
        if (!coreObject)
        {
            coreObject = CoreFactory::create (fi.coreType, generateFullCoreInitString(fi));
        }
    }
    else
    {
        coreObject = CoreFactory::FindOrCreate (fi.coreType, fi.coreName, generateFullCoreInitString (fi));
        if (!coreObject->isOpenToNewFederates ())
        {
            coreObject = nullptr;
            CoreFactory::cleanUpCores (200ms);
            coreObject = CoreFactory::FindOrCreate (fi.coreType, fi.coreName, generateFullCoreInitString (fi));
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
    if (name.empty ())
    {
        name = fi.defName;
    }
    // this call will throw an error on failure
    fedID = coreObject->registerFederate (name, fi);
    separator_ = fi.separator;
    currentTime = coreObject->getCurrentTime (fedID);
    asyncCallInfo = std::make_unique<shared_guarded_m<AsyncFedCallInfo>> ();
    fManager = std::make_unique<FilterFederateManager> (coreObject.get (), this, fedID);
}

Federate::Federate (const std::string &fedName, const std::shared_ptr<Core> &core, const FederateInfo &fi)
    : coreObject (core), name (fedName)
{
    if (!coreObject)
    {
        if (fi.coreName.empty ())
        {
            coreObject = CoreFactory::findJoinableCoreOfType (fi.coreType);
            if (!coreObject)
            {
                coreObject = CoreFactory::create (fi.coreType, generateFullCoreInitString (fi));
            }
        }
        else
        {
            coreObject = CoreFactory::FindOrCreate (fi.coreType, fi.coreName, generateFullCoreInitString (fi));
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
    if (name.empty ())
    {
        name = fi.defName;
    }
    fedID = coreObject->registerFederate (name, fi);
    if (!fedID.isValid ())
    {
        state = op_states::error;
        return;
    }
    separator_ = fi.separator;
    currentTime = coreObject->getCurrentTime (fedID);
    asyncCallInfo = std::make_unique<shared_guarded_m<AsyncFedCallInfo>> ();
    fManager = std::make_unique<FilterFederateManager> (coreObject.get (), this, fedID);
}

Federate::Federate (const std::string &configString) : Federate (std::string{}, loadFederateInfo (configString))
{
    registerFilterInterfaces (configString);
}

Federate::Federate (const std::string &fedName, const std::string &configString)
    : Federate (fedName, loadFederateInfo (configString))
{
    registerFilterInterfaces (configString);
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
    separator_ = fed.separator_;
    asyncCallInfo = std::move (fed.asyncCallInfo);
    fManager = std::move (fed.fManager);
    name = std::move (fed.name);
}

Federate &Federate::operator= (Federate &&fed) noexcept
{
    auto tstate = fed.state.load ();
    state.store (tstate);
    fedID = fed.fedID;
    coreObject = std::move (fed.coreObject);
    currentTime = fed.currentTime;
    separator_ = fed.separator_;
    asyncCallInfo = std::move (fed.asyncCallInfo);
    fManager = std::move (fed.fManager);
    name = std::move (fed.name);
    return *this;
}

Federate::~Federate ()
{
    if (coreObject)
    {
        finalize ();
    }
}

void Federate::enterInitializingMode ()
{
    auto currentState = state.load ();
    if (currentState == op_states::startup)
    {
        coreObject->enterInitializingMode (fedID);
        state = op_states::initialization;
        currentTime = coreObject->getCurrentTime (fedID);
        startupToInitializeStateTransition ();
    }
    else if (currentState == op_states::pending_init)
    {
        enterInitializingModeComplete ();
    }
    else if (currentState != op_states::initialization)  // if we are already in initialization do nothing
    {
        throw (InvalidFunctionCall ("cannot transition from current state to initialization state"));
    }
}

void Federate::enterInitializingModeAsync ()
{
    auto asyncInfo = asyncCallInfo->lock ();
    if (state == op_states::startup)
    {
        state = op_states::pending_init;
        asyncInfo->initFuture =
          std::async (std::launch::async, [this]() { coreObject->enterInitializingMode (fedID); });
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
    auto asyncInfo = asyncCallInfo->lock_shared ();
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

void Federate::enterInitializingModeComplete ()
{
    switch (state)
    {
    case op_states::pending_init:
    {
        auto asyncInfo = asyncCallInfo->lock ();
        asyncInfo->initFuture.get ();
        state = op_states::initialization;
        currentTime = coreObject->getCurrentTime (fedID);
        startupToInitializeStateTransition ();
    }
    break;
    case op_states::initialization:
        break;
    case op_states::startup:
        enterInitializingMode ();
        break;
    default:
        throw (InvalidFunctionCall ("cannot call Initialization Complete function without first calling "
                                    "enterInitializingModeAsync function"));
    }
}

iteration_result Federate::enterExecutingMode (iteration_request iterate)
{
    iteration_result res = iteration_result::next_step;
    switch (state)
    {
    case op_states::startup:
    case op_states::pending_init:
        enterInitializingMode ();
        FALLTHROUGH
        /* FALLTHROUGH */
    case op_states::initialization:
    {
        res = coreObject->enterExecutingMode (fedID, iterate);
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
        return enterExecutingModeComplete ();
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

void Federate::enterExecutingModeAsync (iteration_request iterate)
{
    switch (state)
    {
    case op_states::startup:
    {
        auto eExecFunc = [this, iterate]() {
            coreObject->enterInitializingMode (fedID);
            startupToInitializeStateTransition ();
            return coreObject->enterExecutingMode (fedID, iterate);
        };
        auto asyncInfo = asyncCallInfo->lock ();
        state = op_states::pending_exec;
        asyncInfo->execFuture = std::async (std::launch::async, eExecFunc);
    }
    break;
    case op_states::pending_init:
        enterInitializingModeComplete ();
        FALLTHROUGH
        /* FALLTHROUGH */
    case op_states::initialization:
    {
        auto eExecFunc = [this, iterate]() { return coreObject->enterExecutingMode (fedID, iterate); };
        auto asyncInfo = asyncCallInfo->lock ();
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

iteration_result Federate::enterExecutingModeComplete ()
{
    if (state != op_states::pending_exec)
    {
        throw (InvalidFunctionCall ("cannot call finalize function without first calling async function"));
    }
    auto asyncInfo = asyncCallInfo->lock ();
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

void Federate::setTimeProperty (int32_t option, Time timeValue)
{
    coreObject->setTimeProperty (fedID, option, timeValue);
}

void Federate::setIntegerProperty (int32_t option, int32_t optionValue)
{
    coreObject->setIntegerProperty (fedID, option, optionValue);
}

Time Federate::getTimeProperty (int32_t option) { return coreObject->getTimeProperty (fedID, option); }

int32_t Federate::getIntegerProperty (int32_t option) { return coreObject->getIntegerProperty (fedID, option); }

void Federate::setLoggingCallback (
  const std::function<void(int, const std::string &, const std::string &)> &logFunction)
{
    coreObject->setLoggingCallback (fedID, logFunction);
}

void Federate::setFlagOption (int flag, bool flagValue) { coreObject->setFlagOption (fedID, flag, flagValue); }

bool Federate::getFlagOption (int flag) { return coreObject->getFlagOption (fedID, flag); }
void Federate::finalize ()
{
    switch (state)
    {
    case op_states::startup:
        break;
    case op_states::pending_init:
        enterInitializingModeComplete ();
        break;
    case op_states::initialization:
        break;
    case op_states::pending_exec:
        enterExecutingModeComplete ();
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
    std::string errorString = "error " + std::to_string (errorcode) + " in federate " + name;
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
            if (newTime == Time::maxVal ())
            {
                state = op_states::finalize;
            }
            return newTime;
        }
        catch (const FunctionExecutionFailure &fee)
        {
            state = op_states::error;
            throw;
        }
    }
    else if (state == op_states::finalize)
    {
        return Time::maxVal ();
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
    else if (state == op_states::finalize)
    {
        return iteration_time (Time::maxVal (), iteration_result::halted);
    }
    else
    {
        throw (InvalidFunctionCall ("cannot call request time in present state"));
    }
}

void Federate::requestTimeAsync (Time nextInternalTimeStep)
{
    auto exp = op_states::execution;
    if (state.compare_exchange_strong (exp, op_states::pending_time))
    {
        auto asyncInfo = asyncCallInfo->lock ();
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
    if (state.compare_exchange_strong (exp, op_states::pending_iterative_time))
    {
        auto asyncInfo = asyncCallInfo->lock ();
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
    if (state.compare_exchange_strong (exp, op_states::execution))
    {
        auto asyncInfo = asyncCallInfo->lock ();
        auto newTime = asyncInfo->timeRequestFuture.get ();
        asyncInfo.unlock ();  // remove the lock;
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
    auto asyncInfo = asyncCallInfo->lock ();
    auto exp = op_states::pending_iterative_time;
    if (state.compare_exchange_strong (exp, op_states::execution))
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

void Federate::registerInterfaces (const std::string &configString) { registerFilterInterfaces (configString); }

void Federate::registerFilterInterfaces (const std::string &configString)
{
    if (hasTomlExtension (configString))
    {
        registerFilterInterfacesToml (configString);
    }
    else
    {
        registerFilterInterfacesJson (configString);
    }
}

static Filter &generateFilter (Federate *fed,
                               bool global,
                               bool cloning,
                               const std::string &name,
                               defined_filter_types operation,
                               const std::string &inputType,
                               const std::string &outputType)
{
    bool useTypes = !((inputType.empty ()) && (outputType.empty ()));
    if (useTypes)
    {
        if (cloning)
        {
            return (global) ? fed->registerGlobalCloningFilter (name, inputType, outputType) :
                              fed->registerCloningFilter (name, inputType, outputType);
        }
        else
        {
            return (global) ? fed->registerGlobalFilter (name, inputType, outputType) :
                              fed->registerFilter (name, inputType, outputType);
        }
    }
    else
    {
        if (cloning)
        {
            return (global) ? make_cloning_filter (GLOBAL, operation, fed, name) :
                              make_cloning_filter (operation, fed, name);
        }
        else
        {
            return (global) ? make_filter (GLOBAL, operation, fed, name) : make_filter (operation, fed, name);
        }
    }
}

void Federate::registerFilterInterfacesJson (const std::string &jsonString)
{
    auto doc = loadJson (jsonString);

    if (doc.isMember ("filters"))
    {
        for (const auto &filt : doc["filters"])
        {
            std::string key = jsonGetOrDefault (filt, "name", std::string ());
            std::string inputType = jsonGetOrDefault (filt, "inputType", std::string ());
            std::string outputType = jsonGetOrDefault (filt, "outputType", std::string ());
            bool cloningflag = jsonGetOrDefault (filt, "cloning", false);
            bool useTypes = !((inputType.empty ()) && (outputType.empty ()));

            std::string operation = jsonGetOrDefault (filt, "operation", std::string ("custom"));

            auto opType = filterTypeFromString (operation);
            if ((useTypes) && (operation != "custom"))
            {
                std::cerr << "input and output types may only be specified for custom filters\n";
                continue;
            }
            if (!useTypes)
            {
                if (opType == defined_filter_types::unrecognized)
                {
                    std::cerr << "unrecognized filter operation:" << operation << '\n';
                    continue;
                }
            }
            auto &filter = generateFilter (this, false, cloningflag, key, opType, inputType, outputType);

            if (filt.isMember ("targets"))
            {
                auto targets = filt["targets"];
                if (targets.isArray ())
                {
                    for (const auto &target : targets)
                    {
                        filter.addSourceTarget (target.asString ());
                    }
                }
                else
                {
                    filter.addSourceTarget (targets.asString ());
                }
            }

            if (filt.isMember ("sourcetargets"))
            {
                auto targets = filt["targets"];
                if (targets.isArray ())
                {
                    for (const auto &target : targets)
                    {
                        filter.addSourceTarget (target.asString ());
                    }
                }
                else
                {
                    filter.addSourceTarget (targets.asString ());
                }
            }

            if (filt.isMember ("desttargets"))
            {
                auto targets = filt["targets"];
                if (targets.isArray ())
                {
                    for (const auto &target : targets)
                    {
                        filter.addDestinationTarget (target.asString ());
                    }
                }
                else
                {
                    filter.addDestinationTarget (targets.asString ());
                }
            }
            if (cloningflag)
            {
                if (filt.isMember ("delivery"))
                {
                    auto targets = filt["targets"];
                    if (targets.isArray ())
                    {
                        for (const auto &target : targets)
                        {
                            static_cast<CloningFilter &> (filter).addDeliveryEndpoint (target.asString ());
                        }
                    }
                    else
                    {
                        static_cast<CloningFilter &> (filter).addDeliveryEndpoint (targets.asString ());
                    }
                }
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
                            std::cerr << "properties must be specified with \"name\" and \"value\" fields\n";
                            continue;
                        }
                        if (prop["value"].isDouble ())
                        {
                            filter.set (prop["name"].asString (), prop["value"].asDouble ());
                        }
                        else
                        {
                            filter.setString (prop["name"].asString (), prop["value"].asString ());
                        }
                    }
                }
                else
                {
                    if ((!props.isMember ("name")) && (!props.isMember ("value")))
                    {
                        std::cerr << "properties must be specified with \"name\" and \"value\" fields\n";
                        continue;
                    }
                    if (props["value"].isDouble ())
                    {
                        filter.set (props["name"].asString (), props["value"].asDouble ());
                    }
                    else
                    {
                        filter.setString (props["name"].asString (), props["value"].asString ());
                    }
                }
            }
        }
    }
}

void Federate::registerFilterInterfacesToml (const std::string &tomlString)
{
    toml::Value doc;
    try
    {
        doc = loadToml (tomlString);
    }
    catch (const std::invalid_argument &ia)
    {
        throw (helics::InvalidParameter (ia.what ()));
    }

    auto filts = doc.find ("filters");
    if (filts != nullptr)
    {
        auto &filtArray = filts->as<toml::Array> ();
        for (const auto &filt : filtArray)
        {
            std::string key = tomlGetOrDefault (filt, "name", std::string ());
            bool cloningflag = tomlGetOrDefault (filt, "cloning", false);
            std::string inputType = tomlGetOrDefault (filt, "inputType", std::string ());
            std::string outputType = tomlGetOrDefault (filt, "outputType", std::string ());
            bool useTypes = !((inputType.empty ()) && (outputType.empty ()));

            std::string operation = tomlGetOrDefault (filt, "operation", std::string ("custom"));

            auto opType = filterTypeFromString (operation);
            if ((useTypes) && (operation != "custom"))
            {
                std::cerr << "input and output types may only be specified for custom filters\n";
                continue;
            }
            if (!useTypes)
            {
                if (opType == defined_filter_types::unrecognized)
                {
                    std::cerr << "unrecognized filter operation:" << operation << '\n';
                    continue;
                }
            }
            auto &filter = generateFilter (this, false, cloningflag, key, opType, inputType, outputType);

            auto targets = filt.find ("targets");
            if (targets != nullptr)
            {
                if (targets->is<toml::Array> ())
                {
                    auto &targetArray = targets->as<toml::Array> ();
                    for (const auto &target : targetArray)
                    {
                        filter.addSourceTarget (target.as<std::string> ());
                    }
                }
                else
                {
                    filter.addSourceTarget (targets->as<std::string> ());
                }
            }

            targets = filt.find ("sourcetargets");
            if (targets != nullptr)
            {
                if (targets->is<toml::Array> ())
                {
                    auto &targetArray = targets->as<toml::Array> ();
                    for (const auto &target : targetArray)
                    {
                        filter.addSourceTarget (target.as<std::string> ());
                    }
                }
                else
                {
                    filter.addSourceTarget (targets->as<std::string> ());
                }
            }
            targets = filt.find ("desttargets");
            if (targets != nullptr)
            {
                if (targets->is<toml::Array> ())
                {
                    auto &targetArray = targets->as<toml::Array> ();
                    for (const auto &target : targetArray)
                    {
                        filter.addDestinationTarget (target.as<std::string> ());
                    }
                }
                else
                {
                    filter.addDestinationTarget (targets->as<std::string> ());
                }
            }
            if (cloningflag)
            {
                targets = filt.find ("delivery");
                if (targets != nullptr)
                {
                    if (targets->is<toml::Array> ())
                    {
                        auto &targetArray = targets->as<toml::Array> ();
                        for (const auto &target : targetArray)
                        {
                            static_cast<CloningFilter &> (filter).addDeliveryEndpoint (target.as<std::string> ());
                        }
                    }
                    else
                    {
                        static_cast<CloningFilter &> (filter).addDeliveryEndpoint (targets->as<std::string> ());
                    }
                }
            }
            auto props = filt.find ("properties");
            if (props != nullptr)
            {
                if (props->is<toml::Array> ())
                {
                    auto &propArray = props->as<toml::Array> ();
                    for (const auto &prop : propArray)
                    {
                        auto propname = prop.find ("name");
                        auto propval = prop.find ("value");

                        if ((propname == nullptr) || (propval == nullptr))
                        {
                            std::cerr << "properties must be specified with \"name\" and \"value\" fields\n";
                            continue;
                        }
                        if (propval->isNumber ())
                        {
                            filter.set (propname->as<std::string> (), propval->as<double> ());
                        }
                        else
                        {
                            filter.setString (propname->as<std::string> (), propval->as<std::string> ());
                        }
                    }
                }
                else
                {
                    auto propname = props->find ("name");
                    auto propval = props->find ("value");

                    if ((propname == nullptr) || (propval == nullptr))
                    {
                        std::cerr << "properties must be specified with \"name\" and \"value\" fields\n";
                        continue;
                    }
                    if (propval->isNumber ())
                    {
                        filter.set (propname->as<std::string> (), propval->as<double> ());
                    }
                    else
                    {
                        filter.setString (propname->as<std::string> (), propval->as<std::string> ());
                    }
                }
            }
        }
    }
}

Filter &Federate::getFilter (int index) { return fManager->getFilter (index); }

const Filter &Federate::getFilter (int index) const { return fManager->getFilter (index); }

int Federate::filterCount () const { return fManager->getFilterCount (); }

std::string Federate::localQuery (const std::string & /*queryStr*/) const { return std::string (); }

std::string Federate::query (const std::string &queryStr)
{
    std::string res;
    if (queryStr == "name")
    {
        res = getName ();
    }
    else
    {
        res = localQuery (queryStr);
    }
    if (res.empty ())
    {
        res = coreObject->query (getName (), queryStr);
    }
    return res;
}

std::string Federate::query (const std::string &target, const std::string &queryStr)
{
    std::string res;
    if ((target.empty ()) || (target == "federate") || (target == getName ()))
    {
        res = query (queryStr);
    }
    else
    {
        res = coreObject->query (target, queryStr);
    }
    return res;
}

query_id_t Federate::queryAsync (const std::string &target, const std::string &queryStr)
{
    auto queryFut =
      std::async (std::launch::async, [this, target, queryStr]() { return coreObject->query (target, queryStr); });
    auto asyncInfo = asyncCallInfo->lock ();
    int cnt = asyncInfo->queryCounter++;

    asyncInfo->inFlightQueries.emplace (cnt, std::move (queryFut));
    return query_id_t (cnt);
}

query_id_t Federate::queryAsync (const std::string &queryStr)
{
    auto queryFut = std::async (std::launch::async, [this, queryStr]() { return query (queryStr); });
    auto asyncInfo = asyncCallInfo->lock ();
    int cnt = asyncInfo->queryCounter++;

    asyncInfo->inFlightQueries.emplace (cnt, std::move (queryFut));
    return query_id_t (cnt);
}

std::string Federate::queryComplete (query_id_t queryIndex)
{
    auto asyncInfo = asyncCallInfo->lock ();
    auto fnd = asyncInfo->inFlightQueries.find (queryIndex.value ());
    if (fnd != asyncInfo->inFlightQueries.end ())
    {
        return fnd->second.get ();
    }
    return {"#invalid"};
}

bool Federate::isQueryCompleted (query_id_t queryIndex) const
{
    auto asyncInfo = asyncCallInfo->lock ();
    auto fnd = asyncInfo->inFlightQueries.find (queryIndex.value ());
    if (fnd != asyncInfo->inFlightQueries.end ())
    {
        return (fnd->second.wait_for (std::chrono::seconds (0)) == std::future_status::ready);
    }
    return false;
}

Filter &Federate::registerFilter (const std::string &filterName,
                                  const std::string &inputType,
                                  const std::string &outputType)
{
    return fManager->registerFilter ((!filterName.empty ()) ? (getName () + separator_ + filterName) : filterName,
                                     inputType, outputType);
}

CloningFilter &Federate::registerCloningFilter (const std::string &filterName,
                                                const std::string &inputType,
                                                const std::string &outputType)
{
    return fManager->registerCloningFilter ((!filterName.empty ()) ? (getName () + separator_ + filterName) :
                                                                     filterName,
                                            inputType, outputType);
}

Filter &Federate::registerGlobalFilter (const std::string &filterName,
                                        const std::string &inputType,
                                        const std::string &outputType)
{
    return fManager->registerFilter (filterName, inputType, outputType);
}

CloningFilter &Federate::registerGlobalCloningFilter (const std::string &filterName,
                                                      const std::string &inputType,
                                                      const std::string &outputType)
{
    return fManager->registerCloningFilter (filterName, inputType, outputType);
}

void Federate::addSourceTarget (const Filter &filt, const std::string &targetEndpoint)
{
    coreObject->addSourceTarget (filt.getHandle (), targetEndpoint);
}

void Federate::addDestinationTarget (const Filter &filt, const std::string &targetEndpoint)
{
    coreObject->addDestinationTarget (filt.getHandle (), targetEndpoint);
}

const std::string &Federate::getFilterName (const Filter &filt) const { return filt.getName (); }

const std::string &Federate::getFilterInputType (const Filter &filt) const
{
    return coreObject->getType (filt.getHandle ());
}

const std::string &Federate::getFilterOutputType (const Filter &filt) const
{
    return coreObject->getType (filt.getHandle ());
}

const Filter &Federate::getFilter (const std::string &filterName) const
{
    auto &filt = fManager->getFilter (filterName);
    if (!filt.isValid ())
    {
        auto &filt2 = fManager->getFilter (getName () + separator_ + filterName);
        return filt2;
    }
    return filt;
}

Filter &Federate::getFilter (const std::string &filterName)
{
    auto &filt = fManager->getFilter (filterName);
    if (!filt.isValid ())
    {
        auto &filt2 = fManager->getFilter (getName () + separator_ + filterName);
        return filt2;
    }
    return filt;
}

void Federate::setFilterOperator (const Filter &filt, std::shared_ptr<FilterOperator> mo)
{
    coreObject->setFilterOperator (filt.getHandle (), std::move (mo));
}

void Federate::setFilterOption (const Filter &filt, int32_t option, bool option_value)
{
    coreObject->setHandleOption (filt.getHandle (), option, option_value);
}

}  // namespace helics
