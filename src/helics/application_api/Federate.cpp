/*
Copyright (c) 2017-2023,
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
#include "ConnectorFederateManager.hpp"
#include "CoreApp.hpp"
#include "Filters.hpp"
#include "Translator.hpp"
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

Federate::Federate(std::string_view fedName, const FederateInfo& fi): mName(fedName)
{
    if (mName.empty()) {
        mName = fi.defName;
    }

    singleThreadFederate = fi.checkFlagProperty(defs::Flags::SINGLE_THREAD_FEDERATE, false);

    if (fi.coreName.empty()) {
        if (!fi.forceNewCore) {
            coreObject = CoreFactory::findJoinableCoreOfType(fi.coreType);
        }
        if (!coreObject) {
            if (!mName.empty()) {
                std::string cname =
                    fmt::format("{}_core_{}", fedName, gmlc::utilities::randomString(6));

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
    if (mName.find("${") != std::string::npos) {
        mName = coreObject->getFederateName(fedID);
    }
    nameSegmentSeparator = fi.separator;
    strictConfigChecking = fi.checkFlagProperty(defs::Flags::STRICT_CONFIG_CHECKING, true);

    useJsonSerialization = fi.useJsonSerialization;
    observerMode = fi.observer;
    mCurrentTime = coreObject->getCurrentTime(fedID);
    asyncCallInfo = std::make_unique<shared_guarded_m<AsyncFedCallInfo>>();
    cManager = std::make_unique<ConnectorFederateManager>(coreObject.get(),
                                                          this,
                                                          fedID,
                                                          singleThreadFederate);
}

Federate::Federate(std::string_view fedname, CoreApp& core, const FederateInfo& fi):
    Federate(fedname, core.getCopyofCorePointer(), fi)
{
}

Federate::Federate(std::string_view fedName,
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
    strictConfigChecking = fi.checkFlagProperty(defs::Flags::STRICT_CONFIG_CHECKING, true);
    singleThreadFederate = fi.checkFlagProperty(defs::Flags::SINGLE_THREAD_FEDERATE, false);
    mCurrentTime = coreObject->getCurrentTime(fedID);
    if (!singleThreadFederate) {
        asyncCallInfo = std::make_unique<shared_guarded_m<AsyncFedCallInfo>>();
    }
    cManager = std::make_unique<ConnectorFederateManager>(coreObject.get(),
                                                          this,
                                                          fedID,
                                                          singleThreadFederate);
}

Federate::Federate(std::string_view fedName, const std::string& configString):
    Federate(fedName, loadFederateInfo(configString))
{
    if (looksLikeFile(configString)) {
        registerFilterInterfaces(configString);
    }
}

Federate::Federate(const std::string& configString): Federate(std::string_view{}, configString) {}

Federate::Federate() noexcept
{
    // this function needs to be defined for the virtual inheritance to compile but shouldn't
    // actually be executed
}

Federate::Federate(Federate&& fed) noexcept
{
    auto tmode = fed.currentMode.load();
    currentMode.store(tmode);
    fed.currentMode.store(Modes::FINALIZE);
    fedID = fed.fedID;
    coreObject = std::move(fed.coreObject);
    fed.coreObject = CoreFactory::getEmptyCore();
    mCurrentTime = fed.mCurrentTime;
    nameSegmentSeparator = fed.nameSegmentSeparator;
    strictConfigChecking = fed.strictConfigChecking;
    observerMode = fed.observerMode;
    asyncCallInfo = std::move(fed.asyncCallInfo);
    cManager = std::move(fed.cManager);
    mName = std::move(fed.mName);
}

Federate& Federate::operator=(Federate&& fed) noexcept
{
    auto tstate = fed.currentMode.load();
    currentMode.store(tstate);
    fed.currentMode.store(Modes::FINALIZE);
    fedID = fed.fedID;
    coreObject = std::move(fed.coreObject);
    fed.coreObject = CoreFactory::getEmptyCore();
    mCurrentTime = fed.mCurrentTime;
    nameSegmentSeparator = fed.nameSegmentSeparator;
    strictConfigChecking = fed.strictConfigChecking;
    asyncCallInfo = std::move(fed.asyncCallInfo);
    observerMode = fed.observerMode;
    cManager = std::move(fed.cManager);
    mName = std::move(fed.mName);
    return *this;
}

Federate::~Federate()
{
    if (currentMode != Modes::FINALIZE) {
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
                if (coreObject->enterInitializingMode(fedID)) {
                    enteringInitializingMode(IterationResult::NEXT_STEP);
                }
            }
            catch (const HelicsException&) {
                updateFederateMode(Modes::ERROR_STATE);
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

void Federate::enteringInitializingMode(IterationResult iterating)
{
    updateFederateMode(Modes::INITIALIZING);
    mCurrentTime = coreObject->getCurrentTime(fedID);
    if (iterating == IterationResult::NEXT_STEP) {
        startupToInitializeStateTransition();
    }
    if (initializingEntryCallback) {
        initializingEntryCallback(iterating != IterationResult::NEXT_STEP);
    }
}

void Federate::enterInitializingModeAsync()
{
    if (singleThreadFederate) {
        throw(InvalidFunctionCall(
            "Async function calls and methods are not allowed for single thread federates"));
    }
    auto cm = currentMode.load();
    if (cm == Modes::STARTUP) {
        auto asyncInfo = asyncCallInfo->lock();
        if (currentMode.compare_exchange_strong(cm, Modes::PENDING_INIT)) {
            asyncInfo->initFuture = std::async(std::launch::async, [this]() {
                return coreObject->enterInitializingMode(fedID);
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
    if (singleThreadFederate) {
        return false;
    }
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
        case Modes::PENDING_ITERATIVE_INIT:
            return (asyncInfo->initIterativeFuture.wait_for(wait_delay) == ready);
        default:
            return (asyncInfo->asyncCheck) ? asyncInfo->asyncCheck() : false;
    }
}

void Federate::enterInitializingModeComplete()
{
    if (singleThreadFederate) {
        return enterInitializingMode();
    }
    switch (currentMode.load()) {
        case Modes::PENDING_INIT: {
            auto asyncInfo = asyncCallInfo->lock();
            try {
                bool res = asyncInfo->initFuture.get();
                if (res) {
                    enteringInitializingMode(IterationResult::NEXT_STEP);
                }
            }
            catch (const std::exception&) {
                updateFederateMode(Modes::ERROR_STATE);
                throw;
            }
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

void Federate::enterInitializingModeIterative()
{
    auto cm = currentMode.load();
    switch (cm) {
        case Modes::STARTUP:
            try {
                coreObject->enterInitializingMode(fedID, IterationRequest::FORCE_ITERATION);
            }
            catch (const HelicsException&) {
                updateFederateMode(Modes::ERROR_STATE);
                throw;
            }
            break;
        case Modes::PENDING_ITERATIVE_INIT:
            enterInitializingModeIterativeComplete();
            break;
        default:
            throw(InvalidFunctionCall("cannot call iterative initialization from current state"));
    }
}

void Federate::enterInitializingModeIterativeAsync()
{
    auto cm = currentMode.load();
    if (cm == Modes::STARTUP) {
        auto asyncInfo = asyncCallInfo->lock();
        if (currentMode.compare_exchange_strong(cm, Modes::PENDING_ITERATIVE_INIT)) {
            asyncInfo->initIterativeFuture = std::async(std::launch::async, [this]() {
                coreObject->enterInitializingMode(fedID, IterationRequest::FORCE_ITERATION);
            });
        }
    } else if (cm == Modes::PENDING_ITERATIVE_INIT) {
        return;
    } else {
        // everything else is an error
        throw(InvalidFunctionCall(
            "cannot request iterations in initializing mode if already past that mode"));
    }
}

void Federate::enterInitializingModeIterativeComplete()
{
    switch (currentMode.load()) {
        case Modes::PENDING_ITERATIVE_INIT: {
            auto asyncInfo = asyncCallInfo->lock();
            try {
                asyncInfo->initIterativeFuture.get();
                updateFederateMode(Modes::STARTUP);
            }
            catch (const std::exception&) {
                updateFederateMode(Modes::ERROR_STATE);
                throw;
            }
        } break;
        case Modes::STARTUP:
            // odd call since it would do nothing but not an error
            break;
        default:
            throw(InvalidFunctionCall(
                "cannot call enterInitializingModeIterativeComplete function without first calling "
                "enterInitializingModeIterativeAsync function "));
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
            auto ires = coreObject->enterExecutingMode(fedID, iterate);
            enteringExecutingMode(ires);
            res = ires.state;
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

void Federate::enteringExecutingMode(iteration_time res)
{
    switch (res.state) {
        case IterationResult::NEXT_STEP:
            updateFederateMode(Modes::EXECUTING);

            mCurrentTime = res.grantedTime;
            if (timeUpdateCallback) {
                timeUpdateCallback(mCurrentTime, false);
            }
            initializeToExecuteStateTransition(res);
            if (timeRequestReturnCallback) {
                timeRequestReturnCallback(mCurrentTime, false);
            }
            break;
        case IterationResult::ITERATING:
            mCurrentTime = initializationTime;

            enteringInitializingMode(res.state);

            initializeToExecuteStateTransition(res);
            break;
        case IterationResult::ERROR_RESULT:
            updateFederateMode(Modes::ERROR_STATE);
            break;
        case IterationResult::HALTED:
            updateFederateMode(Modes::FINISHED);
            break;
    }
}

void Federate::handleError(int errorCode, std::string_view errorString, bool noThrow)
{
    updateFederateMode(Modes::ERROR_STATE);

    if (errorHandlerCallback) {
        errorHandlerCallback(errorCode, errorString);
    } else if (!noThrow) {
        throw FederateError(errorCode, errorString);
    }
}

void Federate::enterExecutingModeAsync(IterationRequest iterate)
{
    if (singleThreadFederate) {
        throw(InvalidFunctionCall(
            "Async function calls and methods are not allowed for single thread federates"));
    }
    switch (currentMode) {
        case Modes::STARTUP: {
            auto eExecFunc = [this, iterate]() {
                coreObject->enterInitializingMode(fedID);
                mCurrentTime = coreObject->getCurrentTime(fedID);
                startupToInitializeStateTransition();
                return coreObject->enterExecutingMode(fedID, iterate);
            };
            auto asyncInfo = asyncCallInfo->lock();
            updateFederateMode(Modes::PENDING_EXEC);
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
            updateFederateMode(Modes::PENDING_EXEC);
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
    if (singleThreadFederate) {
        return enterExecutingMode();
    }
    switch (currentMode.load()) {
        case Modes::PENDING_EXEC: {
            auto asyncInfo = asyncCallInfo->lock();
            try {
                auto res = asyncInfo->execFuture.get();
                enteringExecutingMode(res);
                return res.state;
            }
            catch (const std::exception&) {
                updateFederateMode(Modes::ERROR_STATE);
                throw;
            }
        }
        default:
            return enterExecutingMode();
    }
}

void Federate::setAsyncCheck(std::function<bool()> asyncCheck)
{
    if (singleThreadFederate) {
        return;
    }
    auto asyncInfo = asyncCallInfo->lock();
    asyncInfo->asyncCheck = std::move(asyncCheck);
}

void Federate::setTag(std::string_view tag, std::string_view value)
{
    coreObject->setFederateTag(fedID, tag, value);
}

const std::string& Federate::getTag(std::string_view tag) const
{
    return coreObject->getFederateTag(fedID, tag);
}

void Federate::setProperty(int32_t option, double timeValue)
{
    if (option == defs::Properties::STOPTIME) {
        mStopTime = timeValue;
        return;
    }
    coreObject->setTimeProperty(fedID, option, timeValue);
}

void Federate::setProperty(int32_t option, Time timeValue)
{
    if (option == defs::Properties::STOPTIME) {
        mStopTime = timeValue;
        return;
    }
    coreObject->setTimeProperty(fedID, option, timeValue);
}

void Federate::setProperty(int32_t option, int32_t optionValue)
{
    coreObject->setIntegerProperty(fedID, option, static_cast<int16_t>(optionValue));
}

Time Federate::getTimeProperty(int32_t option) const
{
    if (option == defs::Properties::STOPTIME) {
        return mStopTime;
    }
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

void Federate::setInitializingEntryCallback(std::function<void(bool)> callback)
{
    if (currentMode == Modes::PENDING_INIT) {
        throw(InvalidFunctionCall(
            "cannot update initializing entry callback during an async operation"));  // LCOV_EXCL_LINE
    }
    initializingEntryCallback = std::move(callback);
}

void Federate::setExecutingEntryCallback(std::function<void()> callback)
{
    if (currentMode == Modes::PENDING_EXEC || currentMode == Modes::PENDING_INIT) {
        throw(InvalidFunctionCall(
            "cannot update executing entry callback during an async operation"));  // LCOV_EXCL_LINE
    }
    executingEntryCallback = std::move(callback);
}

void Federate::setTimeRequestEntryCallback(std::function<void(Time, Time, bool)> callback)
{
    if (currentMode == Modes::PENDING_ITERATIVE_TIME || currentMode == Modes::PENDING_TIME) {
        throw(InvalidFunctionCall(
            "cannot update time request callback during an async operation"));  // LCOV_EXCL_LINE
    }
    timeRequestEntryCallback = std::move(callback);
}

void Federate::setTimeUpdateCallback(std::function<void(Time, bool)> callback)
{
    if (currentMode == Modes::PENDING_ITERATIVE_TIME || currentMode == Modes::PENDING_TIME) {
        throw(InvalidFunctionCall(
            "cannot update time update callback during an async operation"));  // LCOV_EXCL_LINE
    }
    timeUpdateCallback = std::move(callback);
}

void Federate::setModeUpdateCallback(std::function<void(Modes, Modes)> callback)
{
    if (currentMode == Modes::PENDING_ITERATIVE_TIME || currentMode == Modes::PENDING_TIME ||
        currentMode == Modes::PENDING_EXEC || currentMode == Modes::PENDING_INIT ||
        currentMode == Modes::PENDING_FINALIZE) {
        throw(InvalidFunctionCall(
            "cannot update mode update callback during an async operation"));  // LCOV_EXCL_LINE
    }
    modeUpdateCallback = std::move(callback);
}

void Federate::setTimeRequestReturnCallback(std::function<void(Time, bool)> callback)
{
    if (currentMode == Modes::PENDING_ITERATIVE_TIME || currentMode == Modes::PENDING_TIME) {
        throw(InvalidFunctionCall(
            "cannot update time request return callback during an async operation"));  // LCOV_EXCL_LINE
    }
    timeRequestReturnCallback = std::move(callback);
}

void Federate::setCosimulationTerminatedCallback(std::function<void()> callback)
{
    if (currentMode == Modes::FINALIZE || currentMode == Modes::PENDING_FINALIZE) {
        throw(InvalidFunctionCall(
            "cannot update cosimulation termination callback during an async operation"));  // LCOV_EXCL_LINE
    }
    cosimulationTerminationCallback = std::move(callback);
}

void Federate::setErrorHandlerCallback(std::function<void(int, std::string_view)> callback)
{
    errorHandlerCallback = std::move(callback);
}

void Federate::setFlagOption(int flag, bool flagValue)
{
    if (flag == defs::Flags::OBSERVER && currentMode < Modes::INITIALIZING) {
        observerMode = flagValue;
    }
    if (flag == defs::Flags::AUTOMATED_TIME_REQUEST) {
        retriggerTimeRequest = flagValue;
        return;
    }
    coreObject->setFlagOption(fedID, flag, flagValue);
}

bool Federate::getFlagOption(int flag) const
{
    switch (flag) {
        case defs::Flags::USE_JSON_SERIALIZATION:
            return useJsonSerialization;
        case defs::Flags::AUTOMATED_TIME_REQUEST:
            return retriggerTimeRequest;
        case defs::Flags::SINGLE_THREAD_FEDERATE:
            return singleThreadFederate;
        default:
            return coreObject->getFlagOption(fedID, flag);
    }
}
void Federate::finalize()
{  // since finalize is called in the destructor we can't allow any potential virtual function calls
    switch (currentMode.load()) {
        case Modes::STARTUP:
        case Modes::INITIALIZING:
        case Modes::EXECUTING:
        case Modes::FINISHED:
            break;
        case Modes::PENDING_INIT: {
            auto asyncInfo = asyncCallInfo->lock();
            try {
                asyncInfo->initFuture.get();
            }
            catch (const std::exception&) {
                updateFederateMode(Modes::ERROR_STATE);
                throw;
            }
        } break;
        case Modes::PENDING_EXEC:
            asyncCallInfo->lock()->execFuture.get();
            break;
        case Modes::PENDING_TIME:
            asyncCallInfo->lock()->timeRequestFuture.get();
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
    if (coreObject) {
        coreObject->finalize(fedID);
    }
    finalizeOperations();
}

void Federate::finalizeAsync()
{
    if (singleThreadFederate) {
        throw(InvalidFunctionCall(
            "Async function calls and methods are not allowed for single thread federates"));
    }
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
    updateFederateMode(Modes::PENDING_FINALIZE);
    asyncInfo->finalizeFuture = std::async(std::launch::async, finalizeFunc);
}

/** complete the asynchronous terminate pair*/
void Federate::finalizeComplete()
{
    if (singleThreadFederate) {
        return finalize();
    }
    if (currentMode == Modes::PENDING_FINALIZE) {
        auto asyncInfo = asyncCallInfo->lock();
        asyncInfo->finalizeFuture.get();
        finalizeOperations();
    } else {
        finalize();
    }
}

void Federate::finalizeOperations()
{
    // this should not contain virtual calls
    if (cManager) {
        cManager->closeAllConnectors();
    }
    updateFederateMode(Modes::FINALIZE);
}

void Federate::processCommunication(std::chrono::milliseconds period)
{
    coreObject->processCommunications(fedID, period);
}

void Federate::disconnect()
{
    finalize();
    if (cManager) {
        cManager->disconnect();
    }
    coreObject = CoreFactory::getEmptyCore();
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

void Federate::localError(int errorcode, std::string_view message)
{
    completeOperation();
    updateFederateMode(Modes::ERROR_STATE);
    coreObject->localError(fedID, errorcode, message);
}

void Federate::globalError(int errorcode, std::string_view message)
{
    completeOperation();
    updateFederateMode(Modes::ERROR_STATE);
    coreObject->globalError(fedID, errorcode, message);
}

Time Federate::requestTime(Time nextInternalTimeStep)
{
    switch (currentMode) {
        case Modes::EXECUTING:
            try {
                Time newTime;
                do {
                    preTimeRequestOperations(nextInternalTimeStep, false);
                    newTime = coreObject->timeRequest(fedID, nextInternalTimeStep);
                    postTimeRequestOperations(newTime, false);
                } while (retriggerTimeRequest && newTime < Time::maxVal());

                return newTime;
            }
            catch (const FunctionExecutionFailure&) {
                updateFederateMode(Modes::ERROR_STATE);
                throw;
            }
            catch (const RegistrationFailure&) {
                updateFederateMode(Modes::ERROR_STATE);
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

void Federate::preTimeRequestOperations(Time nextStep, bool iterating)
{
    if (timeRequestEntryCallback) {
        timeRequestEntryCallback(mCurrentTime, nextStep, iterating);
    }
}
void Federate::postTimeRequestOperations(Time newTime, bool iterating)
{
    updateSimulationTime(newTime, mCurrentTime, iterating);

    if (timeRequestReturnCallback) {
        timeRequestReturnCallback(newTime, iterating);
    }
}

iteration_time Federate::requestTimeIterative(Time nextInternalTimeStep, IterationRequest iterate)
{
    if (currentMode == Modes::EXECUTING) {
        preTimeRequestOperations(nextInternalTimeStep, iterate != IterationRequest::NO_ITERATIONS);
        auto iterativeTime = coreObject->requestTimeIterative(fedID, nextInternalTimeStep, iterate);
        switch (iterativeTime.state) {
            case IterationResult::NEXT_STEP:
                postTimeRequestOperations(iterativeTime.grantedTime, false);
                break;
            case IterationResult::ITERATING:
                postTimeRequestOperations(iterativeTime.grantedTime, true);
                break;
            case IterationResult::HALTED:
                updateFederateMode(Modes::FINISHED);
                updateSimulationTime(iterativeTime.grantedTime, mCurrentTime, false);
                break;
            case IterationResult::ERROR_RESULT:
                // LCOV_EXCL_START
                updateFederateMode(Modes::ERROR_STATE);
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
    if (singleThreadFederate) {
        throw(InvalidFunctionCall(
            "Async function calls and methods are not allowed for single thread federates"));
    }
    auto exp = Modes::EXECUTING;
    if (currentMode.compare_exchange_strong(exp, Modes::PENDING_TIME)) {
        preTimeRequestOperations(nextInternalTimeStep, false);
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
    if (singleThreadFederate) {
        throw(InvalidFunctionCall(
            "Async function calls and methods are not allowed for single thread federates"));
    }
    auto exp = Modes::EXECUTING;
    if (currentMode.compare_exchange_strong(exp, Modes::PENDING_ITERATIVE_TIME)) {
        preTimeRequestOperations(nextInternalTimeStep, iterate != IterationRequest::NO_ITERATIONS);
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
        postTimeRequestOperations(newTime, false);
        return newTime;
    }
    throw(InvalidFunctionCall(
        "cannot call requestTimeComplete without first calling requestTimeAsync function"));
}

/** finalize the time advancement request
@return the granted time step*/
iteration_time Federate::requestTimeIterativeComplete()
{
    auto exp = Modes::PENDING_ITERATIVE_TIME;
    if (currentMode.compare_exchange_strong(exp, Modes::EXECUTING)) {
        auto asyncInfo = asyncCallInfo->lock();
        auto iterativeTime = asyncInfo->timeRequestIterativeFuture.get();
        switch (iterativeTime.state) {
            case IterationResult::NEXT_STEP:
                postTimeRequestOperations(iterativeTime.grantedTime, false);
                break;
            case IterationResult::ITERATING:
                postTimeRequestOperations(iterativeTime.grantedTime, true);
                break;
            case IterationResult::HALTED:
                updateFederateMode(Modes::FINISHED);
                updateSimulationTime(iterativeTime.grantedTime, mCurrentTime, false);
                break;
            case IterationResult::ERROR_RESULT:
                // LCOV_EXCL_START
                updateFederateMode(Modes::ERROR_STATE);
                break;
                // LCOV_EXCL_STOP
        }
        return iterativeTime;
    }
    throw(InvalidFunctionCall(
        "cannot call requestTimeIterativeComplete without first calling requestTimeIterativeAsync function"));
}

void Federate::updateFederateMode(Modes newMode)
{
    Modes oldMode = currentMode.load();
    currentMode.store(newMode);
    if (newMode == oldMode) {
        return;
    }
    if (newMode == Modes::PENDING_EXEC || newMode == Modes::PENDING_INIT ||
        newMode == Modes::PENDING_ITERATIVE_TIME || newMode == Modes::PENDING_TIME ||
        newMode == Modes::PENDING_FINALIZE) {
        return;
    }
    if (modeUpdateCallback) {
        switch (oldMode) {
            case Modes::PENDING_ITERATIVE_INIT:
                break;
            case Modes::PENDING_INIT:
                modeUpdateCallback(newMode, Modes::STARTUP);
                break;
            case Modes::PENDING_EXEC:
                if (newMode != Modes::INITIALIZING) {
                    modeUpdateCallback(newMode, Modes::INITIALIZING);
                }
                break;
            case Modes::PENDING_ITERATIVE_TIME:
            case Modes::PENDING_TIME:
                if (newMode != Modes::EXECUTING) {
                    modeUpdateCallback(newMode, Modes::EXECUTING);
                }
                break;
            case Modes::PENDING_FINALIZE:
                modeUpdateCallback(newMode, Modes::EXECUTING);
                break;
            case Modes::FINALIZE:
            case Modes::INITIALIZING:
            case Modes::EXECUTING:
            case Modes::FINISHED:
            case Modes::ERROR_STATE:
            case Modes::STARTUP:
                modeUpdateCallback(newMode, oldMode);
                break;
        }
    }
    if (executingEntryCallback) {
        if (newMode == Modes::EXECUTING &&
            (oldMode == Modes::INITIALIZING || oldMode == Modes::PENDING_EXEC)) {
            executingEntryCallback();
        }
    }
    if (newMode == Modes::FINALIZE || newMode == Modes::ERROR_STATE) {
        if (cosimulationTerminationCallback) {
            cosimulationTerminationCallback();
        }
    }
}

void Federate::updateSimulationTime(Time newTime, Time oldTime, bool iterating)
{
    mCurrentTime = newTime;
    if (timeUpdateCallback) {
        timeUpdateCallback(newTime, iterating);
    }
    updateTime(newTime, oldTime);
    if (newTime == Time::maxVal()) {
        updateFederateMode(Modes::FINISHED);
    }
}

void Federate::updateTime(Time /*newTime*/, Time /*oldTime*/)
{
    // child classes would likely implement this
}

void Federate::startupToInitializeStateTransition()
{
    // child classes may do something with this
}
void Federate::initializeToExecuteStateTransition(iteration_time /*unused*/)
{
    // child classes may do something with this
}

void Federate::disconnectTransition()
{
    if (cManager) {
        cManager->closeAllConnectors();
    }
}

void Federate::registerInterfaces(const std::string& configString)
{
    registerFilterInterfaces(configString);
}

void Federate::registerFilterInterfaces(const std::string& configString)
{
    if (fileops::hasTomlExtension(configString)) {
        registerConnectorInterfacesToml(configString);
    } else {
        try {
            registerConnectorInterfacesJson(configString);
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

static constexpr std::string_view emptyStr;

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
    loadTags(data, [&filt](std::string_view tagname, std::string_view tagvalue) {
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

void Federate::registerConnectorInterfacesJson(const std::string& jsonString)
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

    if (doc.isMember("aliases")) {
        if (doc["aliases"].isArray()) {
            for (auto& val : doc["aliases"]) {
                addAlias(val[0].asString(), val[1].asString());
            }
        } else {
            auto members = doc["aliases"].getMemberNames();
            for (auto& val : members) {
                addAlias(val, doc["aliases"][val].asString());
            }
        }
    }

    loadTags(doc, [this](std::string_view tagname, std::string_view tagvalue) {
        this->setTag(tagname, tagvalue);
    });
}

void Federate::registerConnectorInterfacesToml(const std::string& tomlString)
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
                            filter.setString(propname,
                                             static_cast<std::string_view>(propval.as_string()));
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
                        filter.setString(propname,
                                         static_cast<std::string_view>(propval.as_string()));
                    }
                }
            }
        }
    }
    if (isMember(doc, "globals")) {
        auto globals = toml::find(doc, "globals");
        if (globals.is_array()) {
            for (auto& val : globals.as_array()) {
                setGlobal(static_cast<std::string_view>(val.as_array()[0].as_string()),
                          static_cast<std::string_view>(val.as_array()[1].as_string()));
            }
        } else {
            for (const auto& val : globals.as_table()) {
                setGlobal(val.first, static_cast<std::string_view>(val.second.as_string()));
            }
        }
    }

    if (isMember(doc, "aliases")) {
        auto globals = toml::find(doc, "aliases");
        if (globals.is_array()) {
            for (auto& val : globals.as_array()) {
                addAlias(static_cast<std::string_view>(val.as_array()[0].as_string()),
                         static_cast<std::string_view>(val.as_array()[1].as_string()));
            }
        } else {
            for (const auto& val : globals.as_table()) {
                addAlias(val.first, static_cast<std::string_view>(val.second.as_string()));
            }
        }
    }
    loadTags(doc, [this](std::string_view tagname, std::string_view tagvalue) {
        this->setTag(tagname, tagvalue);
    });
}

Filter& Federate::getFilter(int index)
{
    return cManager->getFilter(index);
}

const Filter& Federate::getFilter(int index) const
{
    return cManager->getFilter(index);
}

Translator& Federate::getTranslator(int index)
{
    return cManager->getTranslator(index);
}

const Translator& Federate::getTranslator(int index) const
{
    return cManager->getTranslator(index);
}

std::string Federate::localQuery(std::string_view /*queryStr*/) const
{
    return std::string{};
}
std::string Federate::query(std::string_view queryStr, HelicsSequencingModes mode)
{
    std::string res;
    if (queryStr == "name") {
        res = generateJsonQuotedString(getName());
    } else if (queryStr == "corename") {
        res = generateJsonQuotedString(coreObject->getIdentifier());
    } else if (queryStr == "time") {
        res = std::to_string(mCurrentTime);
    } else {
        res = localQuery(queryStr);
    }
    if (res.empty()) {
        res = coreObject->query(getName(), queryStr, mode);
    }
    return res;
}

std::string
    Federate::query(std::string_view target, std::string_view queryStr, HelicsSequencingModes mode)
{
    std::string res;
    if ((target.empty()) || (target == "federate") || (target == getName())) {
        res = query(queryStr);
    } else {
        res = coreObject->query(target, queryStr, mode);
    }
    return res;
}

QueryId Federate::queryAsync(std::string_view target,
                             std::string_view queryStr,
                             HelicsSequencingModes mode)
{
    if (singleThreadFederate) {
        throw(helics::InvalidFunctionCall("No Async calls are allowed in single thread federates"));
    }
    auto queryFut = std::async(std::launch::async, [this, target, queryStr, mode]() {
        return coreObject->query(target, queryStr, mode);
    });
    auto asyncInfo = asyncCallInfo->lock();
    int cnt = asyncInfo->queryCounter++;

    asyncInfo->inFlightQueries.emplace(cnt, std::move(queryFut));
    return QueryId(cnt);
}

QueryId Federate::queryAsync(std::string_view queryStr, HelicsSequencingModes mode)
{
    if (singleThreadFederate) {
        throw(helics::InvalidFunctionCall("No Async calls are allowed in single thread federates"));
    }
    auto queryFut =
        std::async(std::launch::async, [this, queryStr, mode]() { return query(queryStr, mode); });
    auto asyncInfo = asyncCallInfo->lock();
    int cnt = asyncInfo->queryCounter++;

    asyncInfo->inFlightQueries.emplace(cnt, std::move(queryFut));
    return QueryId(cnt);
}

std::string Federate::queryComplete(QueryId queryIndex)  // NOLINT
{
    if (singleThreadFederate) {
        return generateJsonErrorResponse(
            JsonErrorCodes::METHOD_NOT_ALLOWED,
            "Async queries are not allowed when using single thread federates");
    }
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
    coreObject->setQueryCallback(fedID, queryFunction);
}

bool Federate::isQueryCompleted(QueryId queryIndex) const  // NOLINT
{
    if (singleThreadFederate) {
        return false;
    }
    auto asyncInfo = asyncCallInfo->lock();
    auto fnd = asyncInfo->inFlightQueries.find(queryIndex.value());
    if (fnd != asyncInfo->inFlightQueries.end()) {
        return (fnd->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
    }
    return false;
}

void Federate::setGlobal(std::string_view valueName, std::string_view value)
{
    coreObject->setGlobal(valueName, value);
}

void Federate::addAlias(std::string_view interfaceName, std::string_view alias)
{
    coreObject->addAlias(interfaceName, alias);
}

void Federate::sendCommand(std::string_view target,
                           std::string_view commandStr,
                           HelicsSequencingModes mode)
{
    coreObject->sendCommand(target, commandStr, getName(), mode);
}

std::pair<std::string, std::string> Federate::getCommand()
{
    return coreObject->getCommand(fedID);
}

std::pair<std::string, std::string> Federate::waitCommand()
{
    return coreObject->waitCommand(fedID);
}

void Federate::addDependency(std::string_view fedName)
{
    coreObject->addDependency(fedID, fedName);
}
std::string Federate::localNameGenerator(std::string_view addition) const
{
    if (!addition.empty()) {
        std::string localName = getName();
        localName.push_back(nameSegmentSeparator);
        localName.append(addition);
        return localName;
    }
    return std::string{};
}

Filter& Federate::registerFilter(std::string_view filterName,
                                 std::string_view inputType,
                                 std::string_view outputType)
{
    return cManager->registerFilter(localNameGenerator(filterName), inputType, outputType);
}

CloningFilter& Federate::registerCloningFilter(std::string_view filterName,
                                               std::string_view inputType,
                                               std::string_view outputType)
{
    return cManager->registerCloningFilter(localNameGenerator(filterName), inputType, outputType);
}

Filter& Federate::registerGlobalFilter(std::string_view filterName,
                                       std::string_view inputType,
                                       std::string_view outputType)
{
    return cManager->registerFilter(filterName, inputType, outputType);
}

CloningFilter& Federate::registerGlobalCloningFilter(std::string_view filterName,
                                                     std::string_view inputType,
                                                     std::string_view outputType)
{
    return cManager->registerCloningFilter(filterName, inputType, outputType);
}

Translator& Federate::registerGlobalTranslator(std::int32_t translatorType,
                                               std::string_view translatorName,
                                               std::string_view endpointType,
                                               std::string_view units)
{
    Translator& trans = cManager->registerTranslator(translatorName, endpointType, units);
    trans.setTranslatorType(translatorType);
    return trans;
}

Translator& Federate::registerTranslator(std::int32_t translatorType,
                                         std::string_view translatorName,
                                         std::string_view endpointType,
                                         std::string_view units)
{
    Translator& trans =
        cManager->registerTranslator(localNameGenerator(translatorName), endpointType, units);
    trans.setTranslatorType(translatorType);
    return trans;
}

const Filter& Federate::getFilter(std::string_view filterName) const
{
    const Filter& filt = cManager->getFilter(filterName);
    if (!filt.isValid()) {
        return cManager->getFilter(localNameGenerator(filterName));
    }
    return filt;
}

Filter& Federate::getFilter(std::string_view filterName)
{
    Filter& filt = cManager->getFilter(filterName);
    if (!filt.isValid()) {
        return cManager->getFilter(localNameGenerator(filterName));
    }
    return filt;
}

int Federate::getFilterCount() const
{
    return cManager->getFilterCount();
}

void Federate::setFilterOperator(const Filter& filt, std::shared_ptr<FilterOperator> op)
{
    coreObject->setFilterOperator(filt.getHandle(), std::move(op));
}

const Translator& Federate::getTranslator(std::string_view translatorName) const
{
    const Translator& trans = cManager->getTranslator(translatorName);
    if (!trans.isValid()) {
        return cManager->getTranslator(localNameGenerator(translatorName));
    }
    return trans;
}

Translator& Federate::getTranslator(std::string_view translatorName)
{
    Translator& trans = cManager->getTranslator(translatorName);
    if (!trans.isValid()) {
        return cManager->getTranslator(localNameGenerator(translatorName));
    }
    return trans;
}

void Federate::setTranslatorOperator(const Translator& trans,
                                     std::shared_ptr<TranslatorOperator> op)
{
    coreObject->setTranslatorOperator(trans.getHandle(), std::move(op));
}

int Federate::getTranslatorCount() const
{
    return cManager->getTranslatorCount();
}

void Federate::logMessage(int level, std::string_view message) const
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
    return cr->getHandleName(handle);
}

const std::string& Interface::getTarget() const
{
    return cr->getSourceTargets(handle);
}

void Interface::addSourceTarget(std::string_view newTarget, InterfaceType hint)
{
    cr->addSourceTarget(handle, newTarget, hint);
}

void Interface::addDestinationTarget(std::string_view newTarget, InterfaceType hint)
{
    cr->addDestinationTarget(handle, newTarget, hint);
}

void Interface::removeTarget(std::string_view targetToRemove)
{
    cr->removeTarget(handle, targetToRemove);
}

void Interface::addAlias(std::string_view alias)
{
    cr->addAlias(getName(), alias);
}

const std::string& Interface::getInfo() const
{
    return cr->getInterfaceInfo(handle);
}

void Interface::setInfo(std::string_view info)
{
    cr->setInterfaceInfo(handle, info);
}

const std::string& Interface::getTag(std::string_view tag) const
{
    return cr->getInterfaceTag(handle, tag);
}

void Interface::setTag(std::string_view tag, std::string_view value)
{
    cr->setInterfaceTag(handle, tag, value);
}

void Interface::setOption(int32_t option, int32_t value)
{
    cr->setHandleOption(handle, option, value);
}

int32_t Interface::getOption(int32_t option) const
{
    return cr->getHandleOption(handle, option);
}

const std::string& Interface::getInjectionType() const
{
    return cr->getInjectionType(handle);
}

const std::string& Interface::getExtractionType() const
{
    return cr->getExtractionType(handle);
}

const std::string& Interface::getInjectionUnits() const
{
    return cr->getInjectionUnits(handle);
}

const std::string& Interface::getExtractionUnits() const
{
    return cr->getExtractionUnits(handle);
}

const std::string& Interface::getSourceTargets() const
{
    return cr->getSourceTargets(handle);
}

const std::string& Interface::getDestinationTargets() const
{
    return cr->getDestinationTargets(handle);
}

const std::string& Interface::getDisplayName() const
{
    return (mName.empty() ? getSourceTargets() : mName);
}

void Interface::close()
{
    cr->closeHandle(handle);
    cr = CoreFactory::getEmptyCorePtr();
}

void Interface::disconnectFromCore()
{
    cr = CoreFactory::getEmptyCorePtr();
}

}  // namespace helics
