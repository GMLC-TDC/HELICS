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
#include "../common/TomlProcessingFunctions.hpp"
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

Federate::Federate (const std::string &fedName, const FederateInfo &fi) : name (fedName)
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
	if (name.empty())
	{
        name = fi.defName;
	}
    // this call will throw an error on failure
    fedID = coreObject->registerFederate (name, fi);
    separator_ = fi.separator;
    currentTime = coreObject->getCurrentTime (fedID);
    asyncCallInfo = std::make_unique<shared_guarded_m<AsyncFedCallInfo>> ();
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
    if (name.empty ())
    {
        name = fi.defName;
    }
    fedID = coreObject->registerFederate (name, fi);
    if (!fedID.isValid())
    {
        state = op_states::error;
        return;
    }
    separator_ = fi.separator;
    currentTime = coreObject->getCurrentTime (fedID);
    asyncCallInfo = std::make_unique<shared_guarded_m<AsyncFedCallInfo>> ();
}

Federate::Federate (const std::string &configString) : Federate (std::string(),loadFederateInfo (configString))
{
    registerFilterInterfaces (configString);
}

Federate::Federate (const std::string &name, const std::string &configString)
    : Federate (name,loadFederateInfo (configString))
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

void Federate::enterInitializingMode ()
{
    auto currentState = state.load();
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
    auto asyncInfo = asyncCallInfo->lock();
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

void Federate::enterInitializingModeComplete ()
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
        auto asyncInfo = asyncCallInfo->lock();
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

iteration_result Federate::enterExecutingModeComplete ()
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

void Federate::setTimeProperty (int32_t option, Time timeValue)
{
    coreObject->setTimeProperty (fedID, option,timeValue);
}


void Federate::setIntegerProperty (int32_t option, int32_t optionValue) { coreObject->setIntegerProperty (fedID,option,  optionValue); }

void Federate::setLoggingCallback (
  const std::function<void(int, const std::string &, const std::string &)> &logFunction)
{
    coreObject->setLoggingCallback (fedID, logFunction);
}

void Federate::setFlagOption (int flag, bool flagValue)
{
    coreObject->setFlagOption (fedID, flag, flagValue);
}

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
        asyncInfo.unlock(); //remove the lock;
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

void Federate::registerFilterInterfacesJson (const std::string &jsonString)
{
    auto doc = loadJson (jsonString);

    if (doc.isMember ("filters"))
    {
        for (const auto &filt : doc["filters"])
        {


            std::string key = jsonGetOrDefault(filt, "name", std::string());
            std::string target = jsonGetOrDefault(filt, "target", std::string());
            std::string inputType = jsonGetOrDefault(filt, "inputType", std::string());
            std::string outputType = jsonGetOrDefault(filt, "outputType", std::string());
            bool useTypes = !((inputType.empty()) && (outputType.empty()));
            std::string mode = jsonGetOrDefault(filt, "mode", "source");

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
                 //   registerSourceFilter (name, target, inputType, outputType);
                }
                else if ((mode == "dest") || (mode == "destination"))
                {
                  //  registerDestinationFilter (name, target, inputType, outputType);
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
                  //  filter = make_source_filter (type, this, target, name);
                }
                else if ((mode == "dest") || (mode == "destination"))
                {
                  //  filter = make_destination_filter (type, this, target, name);
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
                                std::cerr << "properties must be specified with \"name\" and \"value\" fields\n";
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
                            std::cerr << "properties must be specified with \"name\" and \"value\" fields\n";
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
            std::string key= tomlGetOrDefault (filt, "name", std::string ());
			std::string target = tomlGetOrDefault (filt, "target", std::string ());
            std::string inputType = tomlGetOrDefault (filt, "inputType", std::string ());
            std::string outputType = tomlGetOrDefault (filt, "outputType", std::string ());
            bool useTypes = !((inputType.empty()) && (outputType.empty()));

            std::string mode =tomlGetOrDefault (filt, "mode", std::string ("source"));
          
            std::string operation ("custom");
            auto op = filt.find ("operation");
            if (op!=nullptr)
            {
                operation = op->as<std::string>();
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
                   // registerSourceFilter (name, target, inputType, outputType);
                }
                else if ((mode == "dest") || (mode == "destination"))
                {
                  //  registerDestinationFilter (name, target, inputType, outputType);
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
                    filter = make_filter (type, this, name);
                    filter->addSourceTarget (target);
                }
                else if ((mode == "dest") || (mode == "destination"))
                {
                    filter = make_filter (type, this, name);
                    filter->addDestinationTarget (target);
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
                auto props = filt.find ("properties");
                if (props!=nullptr)
                {
                    if (props->is<toml::Array> ())
                    {
						auto &propArray = props->as<toml::Array> ();
                        for (const auto &prop : propArray)
                        {
                            auto propname = prop.find ("name");
                            auto propval = prop.find ("value");

                            if ((propname == nullptr) || (propval==nullptr))
                            {
                                std::cerr << "properties must be specified with \"name\" and \"value\" fields\n";
                                continue;
                            }
                            if (propval->isNumber ())
                            {
                                filter->set (propname->as<std::string> (), propval->as<double> ());
                            }
                            else
                            {
                                filter->setString (propname->as<std::string> (), propval->as<std::string> ());
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
                            filter->set (propname->as<std::string> (), propval->as<double> ());
                        }
                        else
                        {
                            filter->setString (propname->as<std::string> (), propval->as<std::string> ());
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
    return coreObject->query (getName(), queryStr);
}

std::string Federate::query (const std::string &target, const std::string &queryStr)
{
	if ((target.empty())||(target=="federate"))
	{
        return query (queryStr);
	}
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

filter_id_t Federate::registerFilter (const std::string &filterName,
                                            const std::string &inputType,
                                            const std::string &outputType)
{
    return filter_id_t(coreObject->registerFilter (filterName, inputType, outputType));
}

filter_id_t Federate::registerCloningFilter (const std::string &filterName,
                                                   const std::string &inputType,
                                                   const std::string &outputType)
{
    return filter_id_t(coreObject->registerCloningFilter (filterName, inputType, outputType));
}

void Federate::addSourceTarget (filter_id_t id, const std::string &targetEndpoint)
{
    coreObject->addSourceTarget (interface_handle(id.value()), targetEndpoint);
}

void Federate::addDestinationTarget (filter_id_t id, const std::string &targetEndpoint)
{
    coreObject->addDestinationTarget (interface_handle(id.value()), targetEndpoint);
}

std::string Federate::getFilterName (filter_id_t id) const { return coreObject->getHandleName (interface_handle(id.value ())); }

std::string Federate::getFilterInputType (filter_id_t id) const { return coreObject->getType (interface_handle(id.value ())); }

std::string Federate::getFilterOutputType (filter_id_t id) const { return coreObject->getType (interface_handle(id.value ())); }

filter_id_t Federate::getFilterId (const std::string &filterName) const
{
    auto id = coreObject->getFilter (filterName);
    return (id.isValid()) ? filter_id_t(id):invalid_id_value ;
}

void Federate::setFilterOperator (filter_id_t id, std::shared_ptr<FilterOperator> mo)
{
    coreObject->setFilterOperator (interface_handle(id.value ()), std::move (mo));
}

void Federate::setFilterOperator (const std::vector<filter_id_t> &filter_ids, std::shared_ptr<FilterOperator> mo)
{
    for (auto id : filter_ids)
    {
        coreObject->setFilterOperator (interface_handle(id.value ()), mo);
    }
}

}  // namespace helics
