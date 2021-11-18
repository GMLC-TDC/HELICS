/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "Federate.hpp"

#include "../common/GuardedTypes.hpp"
#include "../common/JsonGeneration.hpp"
#include "../common/addTargets.hpp"
#include "../common/configFileHelpers.hpp"
#include "../common/fmt_format.h"
#include "../core/BrokerFactory.hpp"
#include "../core/Core.hpp"
#include "../core/CoreFactory.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helics_definitions.hpp"
#include "../network/loadCores.hpp"
#include "AsyncFedCallInfo.hpp"
#include "CoreApp.hpp"
#include "FilterFederateManager.hpp"
#include "Filters.hpp"
#include "gmlc/utilities/stringOps.h"
#include "helics/helics-config.h"

#include <cassert>
#include <iostream>
#include <string>
#include <utility>

namespace helics {
// a key link that does very little yet, but forces linking to a particular file
static const auto ldcores = loadCores();

using namespace std::chrono_literals;  // NOLINT
void cleanupHelicsLibrary()
{
    BrokerFactory::cleanUpBrokers(100ms);
    CoreFactory::cleanUpCores(200ms);
    BrokerFactory::cleanUpBrokers(100ms);
}

Federate::Federate(const std::string& fedName, const FederateInfo& fi): mName(fedName)
{
    if (mName.empty()) {
        mName = fi.defName;
    }

    if (fi.coreName.empty()) {
        if (!fi.forceNewCore) {
            coreObject = CoreFactory::findJoinableCoreOfType(fi.coreType);
        }
        if (!coreObject) {
            if (!mName.empty()) {
                std::string cname = fedName + "_core_" + gmlc::utilities::randomString(6);

                try {
                    coreObject =
                        CoreFactory::create(fi.coreType, cname, generateFullCoreInitString(fi));
                }
                catch (const helics::RegistrationFailure&) {
                    // there is a possibility of race condition here in the naming resulting a
                    // failure this catches and reverts to previous naming which is fully randomly
                    // generated
                    coreObject = CoreFactory::create(fi.coreType, generateFullCoreInitString(fi));
                }
            } else {
                coreObject = CoreFactory::create(fi.coreType, generateFullCoreInitString(fi));
            }
        }
    } else {
        if (!fi.forceNewCore) {
            coreObject =
                CoreFactory::FindOrCreate(fi.coreType, fi.coreName, generateFullCoreInitString(fi));
            if (!coreObject->isOpenToNewFederates()) {
                coreObject = nullptr;
                logWarningMessage("found core object is not open");
                CoreFactory::cleanUpCores(200ms);
                coreObject = CoreFactory::FindOrCreate(fi.coreType,
                                                       fi.coreName,
                                                       generateFullCoreInitString(fi));
                if (!coreObject->isOpenToNewFederates()) {
                    throw(RegistrationFailure(
                        "Unable to connect to specified core: core is not open to new Federates"));
                }
            }
        } else {
            coreObject =
                CoreFactory::create(fi.coreType, fi.coreName, generateFullCoreInitString(fi));
        }
    }
    /** make sure the core is connected */
    if (!coreObject->isConnected()) {
        coreObject->connect();
        if (!coreObject->isConnected()) {
            if (coreObject->hasError()) {
                auto message = coreObject->getErrorMessage();
                coreObject->disconnect();
                throw(RegistrationFailure(message));
            }
            coreObject->disconnect();
            throw(RegistrationFailure("Unable to connect to broker->unable to register federate"));
        }
    }

    // this call will throw an error on failure
    fedID = coreObject->registerFederate(mName, fi);
    nameSegmentSeparator = fi.separator;
    strictConfigChecking = fi.checkFlagProperty(HELICS_FLAG_STRICT_CONFIG_CHECKING, true);
    useJsonSerialization = fi.useJsonSerialization;
    observerMode = fi.observer;
    currentTime = coreObject->getCurrentTime(fedID);
    asyncCallInfo = std::make_unique<shared_guarded_m<AsyncFedCallInfo>>();
    fManager = std::make_unique<FilterFederateManager>(coreObject.get(), this, fedID);
}

Federate::Federate(const std::string& fedname, CoreApp& core, const FederateInfo& fi):
    Federate(fedname, core.getCopyofCorePointer(), fi)
{
}

Federate::Federate(const std::string& fedName,
                   const std::shared_ptr<Core>& core,
                   const FederateInfo& fi):
    coreObject(core),
    mName(fedName)
{
    if (!coreObject) {
        if (fi.coreName.empty()) {
            coreObject = CoreFactory::findJoinableCoreOfType(fi.coreType);
            if (!coreObject) {
                coreObject = CoreFactory::create(fi.coreType, generateFullCoreInitString(fi));
            }
        } else {
            coreObject =
                CoreFactory::FindOrCreate(fi.coreType, fi.coreName, generateFullCoreInitString(fi));
        }
    }

    /** make sure the core is connected */
    if (!coreObject->isConnected()) {
        coreObject->connect();
    }
    if (mName.empty()) {
        mName = fi.defName;
    }
    fedID = coreObject->registerFederate(mName, fi);
    nameSegmentSeparator = fi.separator;
    observerMode = fi.observer;
    strictConfigChecking = fi.checkFlagProperty(HELICS_FLAG_STRICT_CONFIG_CHECKING, true);
    currentTime = coreObject->getCurrentTime(fedID);
    asyncCallInfo = std::make_unique<shared_guarded_m<AsyncFedCallInfo>>();
    fManager = std::make_unique<FilterFederateManager>(coreObject.get(), this, fedID);
}

Federate::Federate(const std::string& fedName, const std::string& configString):
    Federate(fedName, loadFederateInfo(configString))
{
    if (looksLikeFile(configString)) {
        registerFilterInterfaces(configString);
    }
}

Federate::Federate(const std::string& configString): Federate(std::string{}, configString) {}

Federate::Federate() noexcept
{
    // this function needs to be defined for the virtual inheritance to compile but shouldn't
    // actually be executed
}

Federate::Federate(Federate&& fed) noexcept
{
    auto tmode = fed.currentMode.load();
    currentMode.store(tmode);
    fedID = fed.fedID;
    coreObject = std::move(fed.coreObject);
    currentTime = fed.currentTime;
    nameSegmentSeparator = fed.nameSegmentSeparator;
    strictConfigChecking = fed.strictConfigChecking;
    observerMode = fed.observerMode;
    asyncCallInfo = std::move(fed.asyncCallInfo);
    fManager = std::move(fed.fManager);
    mName = std::move(fed.mName);
}

Federate& Federate::operator=(Federate&& fed) noexcept
{
    auto tstate = fed.currentMode.load();
    currentMode.store(tstate);
    fedID = fed.fedID;
    coreObject = std::move(fed.coreObject);
    currentTime = fed.currentTime;
    nameSegmentSeparator = fed.nameSegmentSeparator;
    strictConfigChecking = fed.strictConfigChecking;
    asyncCallInfo = std::move(fed.asyncCallInfo);
    observerMode = fed.observerMode;
    fManager = std::move(fed.fManager);
    mName = std::move(fed.mName);
    return *this;
}

Federate::~Federate()
{
    if (coreObject) {
        try {
            finalize();
        }
        // LCOV_EXCL_START
        catch (...)  // do not allow a throw inside the destructor
        {
        }
        // LCOV_EXCL_STOP
    }
}

void Federate::enterInitializingMode()
{
    auto cm = currentMode.load();
    switch (cm) {
        case Modes::STARTUP:
            try {
                coreObject->enterInitializingMode(fedID);
                currentMode = Modes::INITIALIZING;
                currentTime = coreObject->getCurrentTime(fedID);
                startupToInitializeStateTransition();
            }
            catch (const HelicsException&) {
                currentMode = Modes::ERROR_STATE;
                throw;
            }
            break;
        case Modes::PENDING_INIT:
            enterInitializingModeComplete();
            break;
        case Modes::INITIALIZING:
            break;
        default:
            throw(InvalidFunctionCall("cannot transition from current mode to initializing mode"));
    }
}

void Federate::enterInitializingModeAsync()
{
    auto cm = currentMode.load();
    if (cm == Modes::STARTUP) {
        auto asyncInfo = asyncCallInfo->lock();
        if (currentMode.compare_exchange_strong(cm, Modes::PENDING_INIT)) {
            asyncInfo->initFuture = std::async(std::launch::async, [this]() {
                coreObject->enterInitializingMode(fedID);
            });
        }
    } else if (cm == Modes::PENDING_INIT) {
        return;
    } else if (cm != Modes::INITIALIZING) {
        // if we are already in initialization do nothing
        throw(InvalidFunctionCall("cannot transition from current mode to initializing mode"));
    }
}

bool Federate::isAsyncOperationCompleted() const
{
    constexpr std::chrono::seconds wait_delay{0};
    auto ready = std::future_status::ready;

    auto asyncInfo = asyncCallInfo->lock_shared();
    switch (currentMode.load()) {
        case Modes::PENDING_INIT:
            return (asyncInfo->initFuture.wait_for(wait_delay) == ready);
        case Modes::PENDING_EXEC:
            return (asyncInfo->execFuture.wait_for(wait_delay) == ready);
        case Modes::PENDING_TIME:
            return (asyncInfo->timeRequestFuture.wait_for(wait_delay) == ready);
        case Modes::PENDING_ITERATIVE_TIME:
            return (asyncInfo->timeRequestIterativeFuture.wait_for(wait_delay) == ready);
        case Modes::PENDING_FINALIZE:
            return (asyncInfo->finalizeFuture.wait_for(wait_delay) == ready);
        default:
            return false;
    }
}

void Federate::enterInitializingModeComplete()
{
    switch (currentMode.load()) {
        case Modes::PENDING_INIT: {
            auto asyncInfo = asyncCallInfo->lock();
            try {
                asyncInfo->initFuture.get();
            }
            catch (const std::exception&) {
                currentMode = Modes::ERROR_STATE;
                throw;
            }
            currentMode = Modes::INITIALIZING;
            currentTime = coreObject->getCurrentTime(fedID);
            startupToInitializeStateTransition();
        } break;
        case Modes::INITIALIZING:
            break;
        case Modes::STARTUP:
            enterInitializingMode();
            break;
        default:
            throw(InvalidFunctionCall(
                "cannot call Initialization Complete function without first calling "
                "enterInitializingModeAsync function or being in startup mode"));
    }
}

IterationResult Federate::enterExecutingMode(IterationRequest iterate)
{
    IterationResult res = IterationResult::NEXT_STEP;
    switch (currentMode) {
        case Modes::STARTUP:
        case Modes::PENDING_INIT:
            enterInitializingMode();
            [[fallthrough]];
        case Modes::INITIALIZING: {
            res = coreObject->enterExecutingMode(fedID, iterate);
            switch (res) {
                case IterationResult::NEXT_STEP:
                    currentMode = Modes::EXECUTING;
                    if (observerMode) {
                        currentTime = coreObject->getCurrentTime(fedID);
                    } else {
                        currentTime = timeZero;
                    }

                    initializeToExecuteStateTransition(res);
                    break;
                case IterationResult::ITERATING:
                    currentMode = Modes::INITIALIZING;
                    currentTime = initializationTime;
                    initializeToExecuteStateTransition(res);
                    break;
                case IterationResult::ERROR_RESULT:
                    currentMode = Modes::ERROR_STATE;
                    break;
                case IterationResult::HALTED:
                    currentMode = Modes::FINISHED;
                    break;
            }
            break;
        }
        case Modes::PENDING_EXEC:
            return enterExecutingModeComplete();
        case Modes::EXECUTING:
            // already in this state --> do nothing
            break;
        case Modes::PENDING_TIME:
            requestTimeComplete();
            break;
        case Modes::PENDING_ITERATIVE_TIME: {
            auto result = requestTimeIterativeComplete();
            return (result.state == IterationResult::ITERATING) ? IterationResult::NEXT_STEP :
                                                                  result.state;
        }
        default:
            throw(InvalidFunctionCall("cannot transition from current state to execution state"));
            break;
    }
    return res;
}

void Federate::enterExecutingModeAsync(IterationRequest iterate)
{
    switch (currentMode) {
        case Modes::STARTUP: {
            auto eExecFunc = [this, iterate]() {
                coreObject->enterInitializingMode(fedID);
                currentTime = coreObject->getCurrentTime(fedID);
                startupToInitializeStateTransition();
                return coreObject->enterExecutingMode(fedID, iterate);
            };
            auto asyncInfo = asyncCallInfo->lock();
            currentMode = Modes::PENDING_EXEC;
            asyncInfo->execFuture = std::async(std::launch::async, eExecFunc);
        } break;
        case Modes::PENDING_INIT:
            enterInitializingModeComplete();
            [[fallthrough]];
        case Modes::INITIALIZING: {
            auto eExecFunc = [this, iterate]() {
                return coreObject->enterExecutingMode(fedID, iterate);
            };
            auto asyncInfo = asyncCallInfo->lock();
            currentMode = Modes::PENDING_EXEC;
            asyncInfo->execFuture = std::async(std::launch::async, eExecFunc);
        } break;
        case Modes::PENDING_EXEC:
        case Modes::EXECUTING:
        case Modes::PENDING_TIME:
        case Modes::PENDING_ITERATIVE_TIME:
            // we are already in or executing a function that would achieve this request
            break;
        default:
            throw(InvalidFunctionCall("cannot transition from current state to execution state"));
            break;
    }
}

IterationResult Federate::enterExecutingModeComplete()
{
    switch (currentMode.load()) {
        case Modes::PENDING_EXEC: {
            auto asyncInfo = asyncCallInfo->lock();
            try {
                auto res = asyncInfo->execFuture.get();
                switch (res) {
                    case IterationResult::NEXT_STEP:
                        currentMode = Modes::EXECUTING;
                        if (observerMode) {
                            currentTime = coreObject->getCurrentTime(fedID);
                        } else {
                            currentTime = timeZero;
                        }
                        initializeToExecuteStateTransition(IterationResult::NEXT_STEP);
                        break;
                    case IterationResult::ITERATING:
                        currentMode = Modes::INITIALIZING;
                        currentTime = initializationTime;
                        initializeToExecuteStateTransition(IterationResult::ITERATING);
                        break;
                    case IterationResult::ERROR_RESULT:
                        // LCOV_EXCL_START
                        currentMode = Modes::ERROR_STATE;
                        break;
                        // LCOV_EXCL_STOP
                    case IterationResult::HALTED:
                        currentMode = Modes::FINISHED;
                        break;
                }

                return res;
            }
            catch (const std::exception&) {
                currentMode = Modes::ERROR_STATE;
                throw;
            }
        }
        default:
            return enterExecutingMode();
    }
}

void Federate::setTag(const std::string& tag, const std::string& value)
{
    coreObject->setFederateTag(fedID, tag, value);
}

const std::string& Federate::getTag(const std::string& tag) const
{
    return coreObject->getFederateTag(fedID, tag);
}

void Federate::setProperty(int32_t option, double timeValue)
{
    coreObject->setTimeProperty(fedID, option, timeValue);
}

void Federate::setProperty(int32_t option, Time timeValue)
{
    coreObject->setTimeProperty(fedID, option, timeValue);
}

void Federate::setProperty(int32_t option, int32_t optionValue)
{
    coreObject->setIntegerProperty(fedID, option, static_cast<int16_t>(optionValue));
}

Time Federate::getTimeProperty(int32_t option) const
{
    return coreObject->getTimeProperty(fedID, option);
}

int32_t Federate::getIntegerProperty(int32_t option) const
{
    return coreObject->getIntegerProperty(fedID, option);
}

void Federate::setLoggingCallback(
    const std::function<void(int, std::string_view, std::string_view)>& logFunction)
{
    coreObject->setLoggingCallback(fedID, logFunction);
}

void Federate::setFlagOption(int flag, bool flagValue)
{
    if (flag == HELICS_FLAG_OBSERVER && currentMode < Modes::INITIALIZING) {
        observerMode = flagValue;
    }
    coreObject->setFlagOption(fedID, flag, flagValue);
}

bool Federate::getFlagOption(int flag) const
{
    if (flag == HELICS_FLAG_USE_JSON_SERIALIZATION) {
        return useJsonSerialization;
    }
    return coreObject->getFlagOption(fedID, flag);
}
void Federate::finalize()
{  // since finalize is called in the destructor we can't allow any potential virtual function calls
    switch (currentMode) {
        case Modes::STARTUP:
            break;
        case Modes::PENDING_INIT: {
            auto asyncInfo = asyncCallInfo->lock();
            try {
                asyncInfo->initFuture.get();
            }
            catch (const std::exception&) {
                currentMode = Modes::ERROR_STATE;
                throw;
            }
        } break;
        case Modes::INITIALIZING:
            break;
        case Modes::PENDING_EXEC:
            asyncCallInfo->lock()->execFuture.get();
            break;
        case Modes::PENDING_TIME:
            asyncCallInfo->lock()->timeRequestFuture.get();
            break;
        case Modes::EXECUTING:
        case Modes::FINISHED:
            break;
        case Modes::PENDING_ITERATIVE_TIME:
            asyncCallInfo->lock()
                ->timeRequestIterativeFuture.get();  // I don't care about the return any more
            break;
        case Modes::FINALIZE:
        case Modes::ERROR_STATE:
            return;
            // do nothing
        case Modes::PENDING_FINALIZE:
            finalizeComplete();
            return;
        default:
            throw(InvalidFunctionCall("cannot call finalize in present state"));  // LCOV_EXCL_LINE
    }
    coreObject->finalize(fedID);
    if (fManager) {
        fManager->closeAllFilters();
    }
    currentMode = Modes::FINALIZE;
}

void Federate::finalizeAsync()
{
    switch (currentMode) {
        case Modes::PENDING_INIT:
            enterInitializingModeComplete();
            break;
        case Modes::PENDING_EXEC:
            enterExecutingModeComplete();
            break;
        case Modes::PENDING_TIME:
            requestTimeComplete();
            break;
        case Modes::PENDING_ITERATIVE_TIME:
            requestTimeIterativeComplete();
            break;
        case Modes::FINALIZE:
        case Modes::ERROR_STATE:
        case Modes::PENDING_FINALIZE:
            return;
            // do nothing
        default:
            break;
    }
    auto finalizeFunc = [this]() { return coreObject->finalize(fedID); };
    auto asyncInfo = asyncCallInfo->lock();
    currentMode = Modes::PENDING_FINALIZE;
    asyncInfo->finalizeFuture = std::async(std::launch::async, finalizeFunc);
}

/** complete the asynchronous terminate pair*/
void Federate::finalizeComplete()
{
    if (currentMode == Modes::PENDING_FINALIZE) {
        auto asyncInfo = asyncCallInfo->lock();
        asyncInfo->finalizeFuture.get();
        currentMode = Modes::FINALIZE;
    } else {
        finalize();
    }
}

void Federate::disconnect()
{
    finalize();

    coreObject = nullptr;
}

void Federate::completeOperation()
{
    switch (currentMode.load()) {
        case Modes::PENDING_INIT:
            enterInitializingModeComplete();
            break;
        case Modes::PENDING_EXEC:
            enterExecutingModeComplete();
            break;
        case Modes::PENDING_TIME:
            requestTimeComplete();
            break;
        case Modes::PENDING_ITERATIVE_TIME:
            requestTimeIterativeComplete();
            break;
        case Modes::PENDING_FINALIZE:
            finalizeComplete();
            break;
        default:
            break;
    }
}

void Federate::localError(int errorcode)
{
    std::string errorString = "local error " + std::to_string(errorcode) + " in federate " + mName;
    localError(errorcode, errorString);
}

void Federate::globalError(int errorcode)
{
    std::string errorString = "global error " + std::to_string(errorcode) + " in federate " + mName;
    globalError(errorcode, errorString);
}

void Federate::localError(int errorcode, const std::string& message)
{
    if (!coreObject) {
        throw(InvalidFunctionCall(
            "cannot generate a federation error on uninitialized or disconnected Federate"));
    }
    completeOperation();
    currentMode = Modes::ERROR_STATE;
    coreObject->localError(fedID, errorcode, message);
}

void Federate::globalError(int errorcode, const std::string& message)
{
    if (!coreObject) {
        throw(InvalidFunctionCall(
            "cannot generate a federation error on uninitialized or disconnected Federate"));
    }
    completeOperation();
    currentMode = Modes::ERROR_STATE;
    coreObject->globalError(fedID, errorcode, message);
}

Time Federate::requestTime(Time nextInternalTimeStep)
{
    switch (currentMode) {
        case Modes::EXECUTING:
            try {
                auto newTime = coreObject->timeRequest(fedID, nextInternalTimeStep);
                Time oldTime = currentTime;
                currentTime = newTime;
                updateTime(newTime, oldTime);
                if (newTime == Time::maxVal()) {
                    currentMode = Modes::FINISHED;
                }
                return newTime;
            }
            catch (const FunctionExecutionFailure&) {
                currentMode = Modes::ERROR_STATE;
                throw;
            }
            break;
        case Modes::FINALIZE:
        case Modes::FINISHED:
            return Time::maxVal();
        default:
            break;
    }

    throw(InvalidFunctionCall("cannot call request time in present state"));
}

iteration_time Federate::requestTimeIterative(Time nextInternalTimeStep, IterationRequest iterate)
{
    if (currentMode == Modes::EXECUTING) {
        auto iterativeTime = coreObject->requestTimeIterative(fedID, nextInternalTimeStep, iterate);
        Time oldTime = currentTime;
        switch (iterativeTime.state) {
            case IterationResult::NEXT_STEP:
                currentTime = iterativeTime.grantedTime;
                [[fallthrough]];
            case IterationResult::ITERATING:
                updateTime(currentTime, oldTime);
                break;
            case IterationResult::HALTED:
                currentTime = iterativeTime.grantedTime;
                updateTime(currentTime, oldTime);
                currentMode = Modes::FINISHED;
                break;
            case IterationResult::ERROR_RESULT:
                // LCOV_EXCL_START
                currentMode = Modes::ERROR_STATE;
                break;
                // LCOV_EXCL_STOP
        }
        return iterativeTime;
    }
    if (currentMode == Modes::FINALIZE || currentMode == Modes::FINISHED) {
        return {Time::maxVal(), IterationResult::HALTED};
    }
    throw(InvalidFunctionCall("cannot call request time in present state"));
}

void Federate::requestTimeAsync(Time nextInternalTimeStep)
{
    auto exp = Modes::EXECUTING;
    if (currentMode.compare_exchange_strong(exp, Modes::PENDING_TIME)) {
        auto asyncInfo = asyncCallInfo->lock();
        asyncInfo->timeRequestFuture =
            std::async(std::launch::async, [this, nextInternalTimeStep]() {
                return coreObject->timeRequest(fedID, nextInternalTimeStep);
            });
    } else {
        throw(InvalidFunctionCall("cannot call request time in present state"));
    }
}

void Federate::requestTimeIterativeAsync(Time nextInternalTimeStep, IterationRequest iterate)
{
    auto exp = Modes::EXECUTING;
    if (currentMode.compare_exchange_strong(exp, Modes::PENDING_ITERATIVE_TIME)) {
        auto asyncInfo = asyncCallInfo->lock();
        asyncInfo->timeRequestIterativeFuture =
            std::async(std::launch::async, [this, nextInternalTimeStep, iterate]() {
                return coreObject->requestTimeIterative(fedID, nextInternalTimeStep, iterate);
            });
    } else {
        throw(InvalidFunctionCall("cannot call request time in present state"));
    }
}

Time Federate::requestTimeComplete()
{
    auto exp = Modes::PENDING_TIME;
    if (currentMode.compare_exchange_strong(exp, Modes::EXECUTING)) {
        auto asyncInfo = asyncCallInfo->lock();
        auto newTime = asyncInfo->timeRequestFuture.get();
        asyncInfo.unlock();  // remove the lock;
        Time oldTime = currentTime;
        currentTime = newTime;
        updateTime(newTime, oldTime);
        return newTime;
    }
    throw(InvalidFunctionCall(
        "cannot call requestTimeComplete without first calling requestTimeAsync function"));
}

/** finalize the time advancement request
@return the granted time step*/
iteration_time Federate::requestTimeIterativeComplete()
{
    auto asyncInfo = asyncCallInfo->lock();
    auto exp = Modes::PENDING_ITERATIVE_TIME;
    if (currentMode.compare_exchange_strong(exp, Modes::EXECUTING)) {
        auto iterativeTime = asyncInfo->timeRequestIterativeFuture.get();
        Time oldTime = currentTime;
        switch (iterativeTime.state) {
            case IterationResult::NEXT_STEP:
                currentTime = iterativeTime.grantedTime;
                [[fallthrough]];
            case IterationResult::ITERATING:
                updateTime(currentTime, oldTime);
                break;
            case IterationResult::HALTED:
                currentTime = iterativeTime.grantedTime;
                updateTime(currentTime, oldTime);
                currentMode = Modes::FINISHED;
                break;
            case IterationResult::ERROR_RESULT:
                // LCOV_EXCL_START
                currentMode = Modes::ERROR_STATE;
                break;
                // LCOV_EXCL_STOP
        }
        return iterativeTime;
    }
    throw(InvalidFunctionCall(
        "cannot call requestTimeIterativeComplete without first calling requestTimeIterativeAsync function"));
}

void Federate::updateTime(Time /*newTime*/, Time /*oldTime*/)
{
    // child classes would likely implement this
}

void Federate::startupToInitializeStateTransition()
{
    // child classes may do something with this
}
void Federate::initializeToExecuteStateTransition(IterationResult /*unused*/)
{
    // child classes may do something with this
}

void Federate::disconnectTransition()
{
    if (fManager) {
        fManager->closeAllFilters();
    }
}

void Federate::registerInterfaces(const std::string& configString)
{
    registerFilterInterfaces(configString);
}

void Federate::registerFilterInterfaces(const std::string& configString)
{
    if (fileops::hasTomlExtension(configString)) {
        registerFilterInterfacesToml(configString);
    } else {
        try {
            registerFilterInterfacesJson(configString);
        }
        catch (const std::invalid_argument& e) {
            throw(helics::InvalidParameter(e.what()));
        }
    }
}

static Filter& generateFilter(Federate* fed,
                              bool global,
                              bool cloning,
                              const std::string& name,
                              FilterTypes operation,
                              const std::string& inputType,
                              const std::string& outputType)
{
    bool useTypes = !((inputType.empty()) && (outputType.empty()));
    if (useTypes) {
        if (cloning) {
            return (global) ? fed->registerGlobalCloningFilter(name, inputType, outputType) :
                              fed->registerCloningFilter(name, inputType, outputType);
        }
        return (global) ? fed->registerGlobalFilter(name, inputType, outputType) :
                          fed->registerFilter(name, inputType, outputType);
    }
    if (cloning) {
        return (global) ? make_cloning_filter(InterfaceVisibility::GLOBAL, operation, fed, name) :
                          make_cloning_filter(operation, fed, name);
    }
    return (global) ? make_filter(InterfaceVisibility::GLOBAL, operation, fed, name) :
                      make_filter(operation, fed, name);
}

const std::string emptyStr;

template<class Inp>
static void loadOptions(Federate* fed, const Inp& data, Filter& filt)
{
    addTargets(data, "flags", [&filt, fed](const std::string& target) {
        auto oindex = getOptionIndex((target.front() != '-') ? target : target.substr(1));
        int val = (target.front() != '-') ? 1 : 0;
        if (oindex == HELICS_INVALID_OPTION_INDEX) {
            fed->logWarningMessage(target + " is not a recognized flag");
            return;
        }
        filt.setOption(oindex, val);
    });
    processOptions(
        data,
        [](const std::string& option) { return getOptionIndex(option); },
        [](const std::string& value) { return getOptionValue(value); },
        [&filt](int32_t option, int32_t value) { filt.setOption(option, value); });

    auto info = fileops::getOrDefault(data, "info", emptyStr);
    if (!info.empty()) {
        filt.setInfo(info);
    }
    loadTags(data, [&filt](const std::string& tagname, const std::string& tagvalue) {
        filt.setTag(tagname, tagvalue);
    });
    auto asrc = [&filt](const std::string& target) { filt.addSourceTarget(target); };
    auto adest = [&filt](const std::string& target) { filt.addDestinationTarget(target); };
    addTargets(data, "sourcetargets", asrc);
    addTargets(data, "sourceTargets", asrc);
    addTargets(data, "source_targets", asrc);
    addTargets(data, "destinationTargets", adest);
    addTargets(data, "destinationtargets", adest);
    addTargets(data, "destination_targets", adest);
}

void Federate::registerFilterInterfacesJson(const std::string& jsonString)
{
    using fileops::getOrDefault;
    auto doc = fileops::loadJson(jsonString);

    if (doc.isMember("filters")) {
        for (const auto& filt : doc["filters"]) {
            std::string key = getOrDefault(filt, "name", emptyStr);
            std::string inputType = getOrDefault(filt, "inputType", emptyStr);
            std::string outputType = getOrDefault(filt, "outputType", emptyStr);
            bool cloningflag = getOrDefault(filt, "cloning", false);
            bool useTypes = !((inputType.empty()) && (outputType.empty()));

            std::string operation = getOrDefault(filt, "operation", std::string("custom"));

            auto opType = filterTypeFromString(operation);
            if ((useTypes) && (operation != "custom")) {
                if (strictConfigChecking) {
                    logMessage(HELICS_LOG_LEVEL_ERROR,
                               "input and output types may only be specified for custom filters");
                    throw(InvalidParameter(
                        "input and output types may only be specified for custom filters"));
                }
                logMessage(HELICS_LOG_LEVEL_WARNING,
                           "input and output types may only be specified for custom filters");
                continue;
            }
            if (!useTypes) {
                if (opType == FilterTypes::UNRECOGNIZED) {
                    if (strictConfigChecking) {
                        std::string emessage =
                            fmt::format("unrecognized filter operation:{}", operation);
                        logMessage(HELICS_LOG_LEVEL_ERROR, emessage);

                        throw(InvalidParameter(emessage));
                    }
                    logMessage(HELICS_LOG_LEVEL_WARNING,
                               fmt::format("unrecognized filter operation:{}", operation));
                    continue;
                }
            }
            auto& filter =
                generateFilter(this, false, cloningflag, key, opType, inputType, outputType);
            loadOptions(this, filt, filter);
            if (cloningflag) {
                addTargets(filt, "delivery", [&filter](const std::string& target) {
                    static_cast<CloningFilter&>(filter).addDeliveryEndpoint(target);
                });
            }

            if (filt.isMember("properties")) {
                auto props = filt["properties"];
                if (props.isArray()) {
                    for (const auto& prop : props) {
                        if ((!prop.isMember("name")) || (!prop.isMember("value"))) {
                            if (strictConfigChecking) {
                                logMessage(
                                    HELICS_LOG_LEVEL_ERROR,
                                    R"(filter properties require "name" and "value" fields)");

                                throw(InvalidParameter(
                                    R"(filter properties require "name" and "value" fields)"));
                            }
                            logMessage(HELICS_LOG_LEVEL_WARNING,
                                       R"(filter properties require "name" and "value" fields)");
                            continue;
                        }
                        if (prop["value"].isDouble()) {
                            filter.set(prop["name"].asString(), prop["value"].asDouble());
                        } else {
                            filter.setString(prop["name"].asString(), prop["value"].asString());
                        }
                    }
                } else {
                    if ((!props.isMember("name")) || (!props.isMember("value"))) {
                        if (strictConfigChecking) {
                            logMessage(HELICS_LOG_LEVEL_ERROR,
                                       R"(filter properties require "name" and "value" fields)");

                            throw(InvalidParameter(
                                R"(filter properties require "name" and "value" fields)"));
                        }
                        logMessage(HELICS_LOG_LEVEL_WARNING,
                                   R"(filter properties require "name" and "value" fields)");
                        continue;
                    }
                    if (props["value"].isDouble()) {
                        filter.set(props["name"].asString(), props["value"].asDouble());
                    } else {
                        filter.setString(props["name"].asString(), props["value"].asString());
                    }
                }
            }
        }
    }
    if (doc.isMember("globals")) {
        if (doc["globals"].isArray()) {
            for (auto& val : doc["globals"]) {
                setGlobal(val[0].asString(), val[1].asString());
            }
        } else {
            auto members = doc["globals"].getMemberNames();
            for (auto& val : members) {
                setGlobal(val, doc["globals"][val].asString());
            }
        }
    }
    loadTags(doc, [this](const std::string& tagname, const std::string& tagvalue) {
        this->setTag(tagname, tagvalue);
    });
}

void Federate::registerFilterInterfacesToml(const std::string& tomlString)
{
    using fileops::getOrDefault;
    using fileops::isMember;

    toml::value doc;
    try {
        doc = fileops::loadToml(tomlString);
    }
    catch (const std::invalid_argument& ia) {
        throw(helics::InvalidParameter(ia.what()));
    }

    if (isMember(doc, "filters")) {
        auto filts = toml::find(doc, "filters");
        if (!filts.is_array()) {
            throw(helics::InvalidParameter("filters section in toml file must be an array"));
        }
        auto& filtArray = filts.as_array();
        for (const auto& filt : filtArray) {
            std::string key = getOrDefault(filt, "name", emptyStr);
            bool cloningflag = getOrDefault(filt, "cloning", false);
            std::string inputType = getOrDefault(filt, "inputType", emptyStr);
            std::string outputType = getOrDefault(filt, "outputType", emptyStr);
            bool useTypes = !((inputType.empty()) && (outputType.empty()));

            std::string operation = getOrDefault(filt, "operation", std::string("custom"));

            auto opType = filterTypeFromString(operation);
            if ((useTypes) && (operation != "custom")) {
                if (strictConfigChecking) {
                    logMessage(HELICS_LOG_LEVEL_ERROR,
                               "input and output types may only be specified for custom filters");
                    throw(InvalidParameter(
                        "input and output types may only be specified for custom filters"));
                }
                logMessage(HELICS_LOG_LEVEL_WARNING,
                           "input and output types may only be specified for custom filters");
                continue;
            }
            if (!useTypes) {
                if (opType == FilterTypes::UNRECOGNIZED) {
                    auto emessage = fmt::format("unrecognized filter operation:{}", operation);
                    if (strictConfigChecking) {
                        logMessage(HELICS_LOG_LEVEL_ERROR, emessage);

                        throw(InvalidParameter(emessage));
                    }
                    logMessage(HELICS_LOG_LEVEL_WARNING, emessage);
                    continue;
                }
            }
            auto& filter =
                generateFilter(this, false, cloningflag, key, opType, inputType, outputType);

            loadOptions(this, filt, filter);

            if (cloningflag) {
                addTargets(filt, "delivery", [&filter](const std::string& target) {
                    static_cast<CloningFilter&>(filter).addDeliveryEndpoint(target);
                });
            }
            if (isMember(filt, "properties")) {
                auto props = toml::find(filt, "properties");
                if (props.is_array()) {
                    auto& propArray = props.as_array();
                    for (const auto& prop : propArray) {
                        std::string propname;
                        propname = toml::find_or(prop, "name", propname);
                        toml::value uVal;
                        auto propval = toml::find_or(prop, "value", uVal);

                        if ((propname.empty()) || (propval.is_uninitialized())) {
                            if (strictConfigChecking) {
                                logMessage(
                                    HELICS_LOG_LEVEL_ERROR,
                                    R"(filter properties require "name" and "value" fields)");

                                throw(InvalidParameter(
                                    R"(filter properties require "name" and "value" fields)"));
                            }
                            logMessage(HELICS_LOG_LEVEL_WARNING,
                                       R"(filter properties require "name" and "value" fields)");
                            continue;
                        }
                        if (propval.is_floating()) {
                            filter.set(propname, propval.as_floating());
                        } else {
                            filter.setString(propname, propval.as_string());
                        }
                    }
                } else {
                    std::string propname;
                    propname = toml::find_or(props, "name", propname);
                    toml::value uVal;
                    auto propval = toml::find_or(props, "value", uVal);

                    if ((propname.empty()) || (propval.is_uninitialized())) {
                        if (strictConfigChecking) {
                            logMessage(HELICS_LOG_LEVEL_ERROR,
                                       R"(filter properties require "name" and "value" fields)");

                            throw(InvalidParameter(
                                R"(filter properties require "name" and "value" fields)"));
                        }
                        logMessage(HELICS_LOG_LEVEL_WARNING,
                                   R"(filter properties require "name" and "value" fields)");
                        continue;
                    }
                    if (propval.is_floating()) {
                        filter.set(propname, propval.as_floating());
                    } else {
                        filter.setString(propname, propval.as_string());
                    }
                }
            }
        }
    }
    if (isMember(doc, "globals")) {
        auto globals = toml::find(doc, "globals");
        if (globals.is_array()) {
            for (auto& val : globals.as_array()) {
                setGlobal(val.as_array()[0].as_string(), val.as_array()[1].as_string());
            }
        } else {
            for (const auto& val : globals.as_table()) {
                setGlobal(val.first, val.second.as_string());
            }
        }
    }
    loadTags(doc, [this](const std::string& tagname, const std::string& tagvalue) {
        this->setTag(tagname, tagvalue);
    });
}

Filter& Federate::getFilter(int index)
{
    return fManager->getFilter(index);
}

const Filter& Federate::getFilter(int index) const
{
    return fManager->getFilter(index);
}

int Federate::filterCount() const
{
    return fManager->getFilterCount();
}

std::string Federate::localQuery(const std::string& /*queryStr*/) const
{
    return std::string{};
}
std::string Federate::query(const std::string& queryStr, HelicsSequencingModes mode)
{
    std::string res;
    if (queryStr == "name") {
        res = generateJsonQuotedString(getName());
    } else if (queryStr == "corename") {
        if (coreObject) {
            res = generateJsonQuotedString(coreObject->getIdentifier());
        } else {
            res =
                generateJsonErrorResponse(JsonErrorCodes::DISCONNECTED, "Federate is disconnected");
        }
    } else if (queryStr == "time") {
        res = std::to_string(currentTime);
    } else {
        res = localQuery(queryStr);
    }
    if (res.empty()) {
        if (coreObject) {
            res = coreObject->query(getName(), queryStr, mode);
        } else {
            res =
                generateJsonErrorResponse(JsonErrorCodes::DISCONNECTED, "Federate is disconnected");
        }
    }
    return res;
}

std::string Federate::query(const std::string& target,
                            const std::string& queryStr,
                            HelicsSequencingModes mode)
{
    std::string res;
    if ((target.empty()) || (target == "federate") || (target == getName())) {
        res = query(queryStr);
    } else {
        if (coreObject) {
            res = coreObject->query(target, queryStr, mode);
        } else {
            res =
                generateJsonErrorResponse(JsonErrorCodes::DISCONNECTED, "Federate is disconnected");
        }
    }
    return res;
}

QueryId Federate::queryAsync(const std::string& target,
                             const std::string& queryStr,
                             HelicsSequencingModes mode)
{
    auto queryFut = std::async(std::launch::async, [this, target, queryStr, mode]() {
        return coreObject->query(target, queryStr, mode);
    });
    auto asyncInfo = asyncCallInfo->lock();
    int cnt = asyncInfo->queryCounter++;

    asyncInfo->inFlightQueries.emplace(cnt, std::move(queryFut));
    return QueryId(cnt);
}

QueryId Federate::queryAsync(const std::string& queryStr, HelicsSequencingModes mode)
{
    auto queryFut =
        std::async(std::launch::async, [this, queryStr, mode]() { return query(queryStr, mode); });
    auto asyncInfo = asyncCallInfo->lock();
    int cnt = asyncInfo->queryCounter++;

    asyncInfo->inFlightQueries.emplace(cnt, std::move(queryFut));
    return QueryId(cnt);
}

std::string Federate::queryComplete(QueryId queryIndex)  // NOLINT
{
    auto asyncInfo = asyncCallInfo->lock();
    auto fnd = asyncInfo->inFlightQueries.find(queryIndex.value());
    if (fnd != asyncInfo->inFlightQueries.end()) {
        return fnd->second.get();
    }
    return generateJsonErrorResponse(JsonErrorCodes::METHOD_NOT_ALLOWED,
                                     "No Async queries are available");
}

void Federate::setQueryCallback(const std::function<std::string(std::string_view)>& queryFunction)
{
    if (coreObject) {
        coreObject->setQueryCallback(fedID, queryFunction);
    } else {
        throw(InvalidFunctionCall(
            " setQueryCallback cannot be called on uninitialized federate or after finalize call"));
    }
}

bool Federate::isQueryCompleted(QueryId queryIndex) const  // NOLINT
{
    auto asyncInfo = asyncCallInfo->lock();
    auto fnd = asyncInfo->inFlightQueries.find(queryIndex.value());
    if (fnd != asyncInfo->inFlightQueries.end()) {
        return (fnd->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
    }
    return false;
}

void Federate::setGlobal(const std::string& valueName, const std::string& value)
{
    if (!coreObject) {
        throw(InvalidFunctionCall(
            " setGlobal cannot be called on uninitialized federate or after finalize call"));
    }
    coreObject->setGlobal(valueName, value);
}

void Federate::sendCommand(const std::string& target,
                           const std::string& commandStr,
                           HelicsSequencingModes mode)
{
    if (!coreObject) {
        throw(InvalidFunctionCall(
            "sendCommand cannot be called on uninitialized federate or after disconnect call"));
    }
    coreObject->sendCommand(target, commandStr, getName(), mode);
}

std::pair<std::string, std::string> Federate::getCommand()
{
    if (!coreObject) {
        throw(InvalidFunctionCall(
            "getCommand cannot be called on uninitialized federate or after disconnect call"));
    }
    return coreObject->getCommand(fedID);
}

std::pair<std::string, std::string> Federate::waitCommand()
{
    if (!coreObject) {
        throw(InvalidFunctionCall(
            "waitCommand cannot be called on uninitialized federate or after disconnect call"));
    }
    return coreObject->waitCommand(fedID);
}

void Federate::addDependency(const std::string& fedName)
{
    if (!coreObject) {
        throw(InvalidFunctionCall(
            "addDependency cannot be called on uninitialized federate or after finalize call"));
    }
    coreObject->addDependency(fedID, fedName);
}

Filter& Federate::registerFilter(const std::string& filterName,
                                 const std::string& inputType,
                                 const std::string& outputType)
{
    return fManager->registerFilter((!filterName.empty()) ?
                                        (getName() + nameSegmentSeparator + filterName) :
                                        filterName,
                                    inputType,
                                    outputType);
}

CloningFilter& Federate::registerCloningFilter(const std::string& filterName,
                                               const std::string& inputType,
                                               const std::string& outputType)
{
    return fManager->registerCloningFilter((!filterName.empty()) ?
                                               (getName() + nameSegmentSeparator + filterName) :
                                               filterName,
                                           inputType,
                                           outputType);
}

Filter& Federate::registerGlobalFilter(const std::string& filterName,
                                       const std::string& inputType,
                                       const std::string& outputType)
{
    return fManager->registerFilter(filterName, inputType, outputType);
}

CloningFilter& Federate::registerGlobalCloningFilter(const std::string& filterName,
                                                     const std::string& inputType,
                                                     const std::string& outputType)
{
    return fManager->registerCloningFilter(filterName, inputType, outputType);
}

const Filter& Federate::getFilter(const std::string& filterName) const
{
    const Filter& filt = fManager->getFilter(filterName);
    if (!filt.isValid()) {
        return fManager->getFilter(getName() + nameSegmentSeparator + filterName);
    }
    return filt;
}

Filter& Federate::getFilter(const std::string& filterName)
{
    Filter& filt = fManager->getFilter(filterName);
    if (!filt.isValid()) {
        return fManager->getFilter(getName() + nameSegmentSeparator + filterName);
    }
    return filt;
}

int Federate::getFilterCount() const
{
    return fManager->getFilterCount();
}

void Federate::setFilterOperator(const Filter& filt, std::shared_ptr<FilterOperator> op)
{
    if (coreObject) {
        coreObject->setFilterOperator(filt.getHandle(), std::move(op));
    } else {
        throw(InvalidFunctionCall(
            "set FilterOperator cannot be called on uninitialized federate or after finalize call"));
    }
}

void Federate::logMessage(int level, const std::string& message) const
{
    if (coreObject) {
        coreObject->logMessage(fedID, level, message);
    } else if (level <= HELICS_LOG_LEVEL_WARNING) {
        std::cerr << message << std::endl;
    } else {
        std::cout << message << std::endl;
    }
}

Interface::Interface(Federate* federate, InterfaceHandle id, std::string_view actName):
    handle(id), mName(actName)
{
    if (federate != nullptr) {
        const auto& crp = federate->getCorePointer();
        if (crp) {
            cr = crp.get();
        }
    }
}

const std::string& Interface::getName() const
{
    return (cr != nullptr) ? (cr->getHandleName(handle)) : emptyStr;
}

const std::string& Interface::getTarget() const
{
    return (cr != nullptr) ? cr->getSourceTargets(handle) : emptyStr;
}

void Interface::addSourceTarget(std::string_view newTarget)
{
    if (cr != nullptr) {
        cr->addSourceTarget(handle, newTarget);
    } else {
        throw(InvalidFunctionCall(
            "add source target cannot be called on uninitialized federate or after finalize call"));
    }
}

void Interface::addDestinationTarget(std::string_view newTarget)
{
    if (cr != nullptr) {
        cr->addDestinationTarget(handle, newTarget);
    } else {
        throw(InvalidFunctionCall(
            "add destination target cannot be called on a closed or uninitialized interface"));
    }
}

void Interface::removeTarget(std::string_view targetToRemove)
{
    if (cr != nullptr) {
        cr->removeTarget(handle, targetToRemove);
    } else {
        throw(InvalidFunctionCall(
            "remove target cannot be called on a closed or uninitialized interface"));
    }
}

const std::string& Interface::getInfo() const
{
    return (cr != nullptr) ? cr->getInterfaceInfo(handle) : emptyStr;
}

void Interface::setInfo(const std::string& info)
{
    if (cr != nullptr) {
        cr->setInterfaceInfo(handle, info);
    } else {
        throw(
            InvalidFunctionCall("cannot call set info on uninitialized or disconnected interface"));
    }
}

const std::string& Interface::getTag(const std::string& tag) const
{
    return (cr != nullptr) ? cr->getInterfaceTag(handle, tag) : emptyStr;
}

void Interface::setTag(const std::string& tag, const std::string& value)
{
    if (cr != nullptr) {
        cr->setInterfaceTag(handle, tag, value);
    } else {
        throw(
            InvalidFunctionCall("cannot call set tag on uninitialized or disconnected interface"));
    }
}

void Interface::setOption(int32_t option, int32_t value)
{
    if (cr != nullptr) {
        cr->setHandleOption(handle, option, value);
    } else {
        throw(InvalidFunctionCall(
            "setInterfaceOption cannot be called on uninitialized federate or after finalize call"));
    }
}

int32_t Interface::getOption(int32_t option) const
{
    return (cr != nullptr) ? cr->getHandleOption(handle, option) : 0;
}

const std::string& Interface::getInjectionType() const
{
    return (cr != nullptr) ? (cr->getInjectionType(handle)) : emptyStr;
}

const std::string& Interface::getExtractionType() const
{
    return (cr != nullptr) ? (cr->getExtractionType(handle)) : emptyStr;
}

const std::string& Interface::getInjectionUnits() const
{
    return (cr != nullptr) ? (cr->getInjectionUnits(handle)) : emptyStr;
}

const std::string& Interface::getExtractionUnits() const
{
    return (cr != nullptr) ? (cr->getExtractionUnits(handle)) : emptyStr;
}

const std::string& Interface::getSourceTargets() const
{
    return (cr != nullptr) ? (cr->getSourceTargets(handle)) : emptyStr;
}

const std::string& Interface::getDestinationTargets() const
{
    return (cr != nullptr) ? (cr->getDestinationTargets(handle)) : emptyStr;
}

const std::string& Interface::getDisplayName() const
{
    return (mName.empty() ? getTarget() : mName);
}

void Interface::close()
{
    if (cr != nullptr) {
        cr->closeHandle(handle);
        cr = nullptr;
    }
}

void Interface::disconnectFromCore()
{
    cr = nullptr;
}

}  // namespace helics
