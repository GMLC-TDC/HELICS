/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "Federate.hpp"

#include "../common/GuardedTypes.hpp"
#include "../common/JsonGeneration.hpp"
#include "../common/addTargets.hpp"
#include "../common/configFileHelpers.hpp"
#include "../core/BrokerFactory.hpp"
#include "../core/Core.hpp"
#include "../core/CoreFactory.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/helics_definitions.hpp"
#include "../network/loadCores.hpp"
#include "AsyncFedCallInfo.hpp"
#include "ConnectorFederateManager.hpp"
#include "CoreApp.hpp"
#include "FederateInfo.hpp"
#include "Filters.hpp"
#include "PotentialInterfacesManager.hpp"
#include "Translator.hpp"
#include "gmlc/utilities/stringOps.h"
#include "helics/helics-config.h"

#include <fmt/format.h>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
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

Federate::Federate(std::string_view fedName, const FederateInfo& fedInfo): mName(fedName)
{
    if (mName.empty()) {
        mName = fedInfo.defName;
    }

    getCore(fedInfo);

    verifyCore();

    registerFederate(fedInfo);
}

Federate::Federate(std::string_view fedname, CoreApp& core, const FederateInfo& fedInfo):
    Federate(fedname, core.getCopyofCorePointer(), fedInfo)
{
}

Federate::Federate(std::string_view fedName,
                   const std::shared_ptr<Core>& core,
                   const FederateInfo& fedInfo): coreObject(core), mName(fedName)
{
    if (mName.empty()) {
        mName = fedInfo.defName;
    }
    getCore(fedInfo);
    verifyCore();

    registerFederate(fedInfo);
}

Federate::Federate(std::string_view fedName, const std::string& configString):
    Federate(fedName, loadFederateInfo(configString))
{
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
            // finalize may throw but we can't allow that
            ;
        }
        // LCOV_EXCL_STOP
    }
}

void Federate::getCore(const FederateInfo& fedInfo)
{
    singleThreadFederate = fedInfo.checkFlagProperty(defs::Flags::SINGLE_THREAD_FEDERATE, false);
    if (coreObject) {
        return;
    }

    if (fedInfo.coreName.empty()) {
        if (!fedInfo.forceNewCore) {
            coreObject = CoreFactory::findJoinableCoreOfType(fedInfo.coreType);
        }
        if (!coreObject) {
            if (!mName.empty()) {
                std::string cname =
                    fmt::format("{}_core_{}", mName, gmlc::utilities::randomString(6));
                auto loc = mName.find("${");
                if (loc != std::string::npos) {
                    cname = fmt::format("{}_core_{}",
                                        mName.substr(0, loc),
                                        gmlc::utilities::randomString(8));
                }
                try {
                    coreObject = CoreFactory::create(fedInfo.coreType,
                                                     cname,
                                                     generateFullCoreInitString(fedInfo));
                }
                catch (const helics::RegistrationFailure&) {
                    // there is a possibility of race condition here in the naming resulting a
                    // failure this catches and reverts to previous naming which is fully randomly
                    // generated
                    coreObject =
                        CoreFactory::create(fedInfo.coreType, generateFullCoreInitString(fedInfo));
                }
            } else {
                coreObject =
                    CoreFactory::create(fedInfo.coreType, generateFullCoreInitString(fedInfo));
            }
        }
    } else {
        if (!fedInfo.forceNewCore) {
            coreObject = CoreFactory::FindOrCreate(fedInfo.coreType,
                                                   fedInfo.coreName,
                                                   generateFullCoreInitString(fedInfo));
            if (!coreObject->isOpenToNewFederates()) {
                coreObject = nullptr;
                logWarningMessage("found core object is not open");
                CoreFactory::cleanUpCores(200ms);
                coreObject = CoreFactory::FindOrCreate(fedInfo.coreType,
                                                       fedInfo.coreName,
                                                       generateFullCoreInitString(fedInfo));
                if (!coreObject->isOpenToNewFederates()) {
                    throw(RegistrationFailure(
                        "Unable to connect to specified core: core is not open to new Federates"));
                }
            }
        } else {
            coreObject = CoreFactory::create(fedInfo.coreType,
                                             fedInfo.coreName,
                                             generateFullCoreInitString(fedInfo));
        }
    }
}

void Federate::verifyCore()
{
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
}
/** function to register the federate with the core*/
void Federate::registerFederate(const FederateInfo& fedInfo)
{
    // this call will throw an error on failure
    fedID = coreObject->registerFederate(mName, fedInfo);
    if (mName.find("${") != std::string::npos) {
        mName = coreObject->getFederateName(fedID);
    }

    nameSegmentSeparator = fedInfo.separator;
    strictConfigChecking = fedInfo.checkFlagProperty(defs::Flags::STRICT_CONFIG_CHECKING, true);

    useJsonSerialization = fedInfo.useJsonSerialization;
    observerMode = fedInfo.observer;
    configFile = fedInfo.configString;
    mCurrentTime = coreObject->getCurrentTime(fedID);
    if (!singleThreadFederate) {
        asyncCallInfo = std::make_unique<shared_guarded_m<AsyncFedCallInfo>>();
    }
    cManager = std::make_unique<ConnectorFederateManager>(coreObject.get(),
                                                          this,
                                                          fedID,
                                                          singleThreadFederate);
    if (!configFile.empty()) {
        registerConnectorInterfaces(configFile);
    }
}

void Federate::enterInitializingMode()
{
    auto cmode = currentMode.load();
    switch (cmode) {
        case Modes::STARTUP:
            try {
                if (hasPotentialInterfaces) {
                    potentialInterfacesStartupSequence();
                }

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
    auto cmode = currentMode.load();
    if (cmode == Modes::STARTUP) {
        auto asyncInfo = asyncCallInfo->lock();
        if (currentMode.compare_exchange_strong(cmode, Modes::PENDING_INIT)) {
            asyncInfo->initFuture = std::async(std::launch::async, [this]() {
                if (hasPotentialInterfaces) {
                    potentialInterfacesStartupSequence();
                }
                return coreObject->enterInitializingMode(fedID);
            });
        }
    } else if (cmode == Modes::PENDING_INIT) {
        return;
    } else if (cmode != Modes::INITIALIZING) {
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
                if (asyncInfo->initFuture.get()) {
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
    auto cmode = currentMode.load();
    switch (cmode) {
        case Modes::STARTUP:
            try {
                if (hasPotentialInterfaces && potManager) {
                    switch (potInterfacesSequence.load()) {
                        case 0:
                            potManager->initialize();
                            coreObject->enterInitializingMode(
                                fedID, helics::IterationRequest::FORCE_ITERATION);
                            potInterfacesSequence.store(2);
                            break;
                        case 2: {
                            // respond to query
                            coreObject->enterInitializingMode(
                                fedID, helics::IterationRequest::FORCE_ITERATION);
                            // now check for commands
                            auto cmd = coreObject->getCommand(fedID);
                            if (cmd.first.empty()) {
                                std::this_thread::sleep_for(std::chrono::milliseconds(200));
                                cmd = coreObject->getCommand(fedID);
                            }
                            while (!cmd.first.empty()) {
                                potManager->processCommand(std::move(cmd));
                                cmd = coreObject->getCommand(fedID);
                            }
                            potInterfacesSequence.store(3);
                        } break;
                        default:
                            coreObject->enterInitializingMode(
                                fedID, helics::IterationRequest::FORCE_ITERATION);
                            break;
                    }
                } else {
                    coreObject->enterInitializingMode(fedID, IterationRequest::FORCE_ITERATION);
                }
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
    auto cmode = currentMode.load();
    if (cmode == Modes::STARTUP) {
        auto asyncInfo = asyncCallInfo->lock();
        if (currentMode.compare_exchange_strong(cmode, Modes::PENDING_ITERATIVE_INIT)) {
            asyncInfo->initIterativeFuture = std::async(std::launch::async, [this]() {
                coreObject->enterInitializingMode(fedID, IterationRequest::FORCE_ITERATION);
            });
        }
    } else if (cmode == Modes::PENDING_ITERATIVE_INIT) {
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
                if (hasPotentialInterfaces) {
                    potentialInterfacesStartupSequence();
                }
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
    const std::string errorString =
        "local error " + std::to_string(errorcode) + " in federate " + mName;
    localError(errorcode, errorString);
}

void Federate::globalError(int errorcode)
{
    const std::string errorString =
        "global error " + std::to_string(errorcode) + " in federate " + mName;
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

void Federate::potentialInterfacesStartupSequence()
{
    if (potManager) {
        switch (potInterfacesSequence.load()) {
            case 0:
                potManager->initialize();
                potInterfacesSequence.store(1);
                [[fallthrough]];
            case 1:
                coreObject->enterInitializingMode(fedID, helics::IterationRequest::FORCE_ITERATION);
                potInterfacesSequence.store(2);
                [[fallthrough]];
            case 2: {
                // respond to query
                coreObject->enterInitializingMode(fedID, helics::IterationRequest::FORCE_ITERATION);
                // now check for commands
                auto cmd = coreObject->getCommand(fedID);
                if (cmd.first.empty()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    cmd = coreObject->getCommand(fedID);
                }
                while (!cmd.first.empty()) {
                    potManager->processCommand(std::move(cmd));
                    cmd = coreObject->getCommand(fedID);
                }
                potInterfacesSequence.store(3);
            } break;
            default:
                break;
        }
    }
}

void Federate::updateFederateMode(Modes newMode)
{
    const Modes oldMode = currentMode.load();
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
    registerConnectorInterfaces(configString);
}

void Federate::registerConnectorInterfaces(const std::string& configString)
{
    auto hint = fileops::getConfigType(configString);
    switch (hint) {
        case fileops::ConfigType::JSON_FILE:
        case fileops::ConfigType::JSON_STRING:
            try {
                registerConnectorInterfacesJson(configString);
            }
            catch (const std::invalid_argument& e) {
                throw(helics::InvalidParameter(e.what()));
            }
            break;
        case fileops::ConfigType::TOML_FILE:
        case fileops::ConfigType::TOML_STRING:
            registerConnectorInterfacesToml(configString);
            break;
        case fileops::ConfigType::NONE:
        default:
            break;
    }
}

static Translator& generateTranslator(Federate* fed,
                                      bool global,
                                      std::string_view name,
                                      TranslatorTypes ttype,
                                      std::string_view endpointType,
                                      std::string_view units)
{
    Translator& trans = (global) ? fed->registerGlobalTranslator(name, endpointType, units) :
                                   fed->registerTranslator(name, endpointType, units);
    if (ttype != TranslatorTypes::CUSTOM) {
        trans.setTranslatorType(static_cast<std::int32_t>(ttype));
    }
    return trans;
}
static Filter& registerFilter(Federate* fed,
                              bool global,
                              bool cloning,
                              std::string_view name,
                              std::string_view inputType,
                              std::string_view outputType)
{
    const bool useTypes = !((inputType.empty()) && (outputType.empty()));
    if (useTypes) {
        if (cloning) {
            return (global) ? fed->registerGlobalCloningFilter(name, inputType, outputType) :
                              fed->registerCloningFilter(name, inputType, outputType);
        }
        return (global) ? fed->registerGlobalFilter(name, inputType, outputType) :
                          fed->registerFilter(name, inputType, outputType);
    }
    if (cloning) {
        return (global) ? fed->registerGlobalCloningFilter(name) : fed->registerCloningFilter(name);
    }
    return (global) ? fed->registerGlobalFilter(name) : fed->registerFilter(name);
}

static Filter& generateFilter(Federate* fed,
                              bool global,
                              bool cloning,
                              std::string_view name,
                              FilterTypes operation,
                              std::string_view inputType,
                              std::string_view outputType)
{
    auto& filt = registerFilter(fed, global, cloning, name, inputType, outputType);
    if (operation != FilterTypes::CUSTOM) {
        filt.setFilterType(static_cast<std::int32_t>(operation));
    }
    return filt;
}

static constexpr std::string_view emptyStr;

template<class Inp, class INTERFACE>
static void loadOptions(Federate* fed, const Inp& data, INTERFACE& iface)
{
    addTargets(data, "flags", [&iface, fed](const std::string& target) {
        auto oindex = getOptionIndex((target.front() != '-') ? target : target.substr(1));
        const int val = (target.front() != '-') ? 1 : 0;
        if (oindex == HELICS_INVALID_OPTION_INDEX) {
            fed->logWarningMessage(target + " is not a recognized flag");
            return;
        }
        iface.setOption(oindex, val);
    });
    processOptions(
        data,
        [](const std::string& option) { return getOptionIndex(option); },
        [](const std::string& value) { return getOptionValue(value); },
        [&iface](int32_t option, int32_t value) { iface.setOption(option, value); });

    auto info = fileops::getOrDefault(data, "info", emptyStr);
    if (!info.empty()) {
        iface.setInfo(info);
    }
    loadTags(data, [&iface](std::string_view tagname, std::string_view tagvalue) {
        iface.setTag(tagname, tagvalue);
    });

    addTargetVariations(data, "source", "targets", [&iface](const std::string& target) {
        iface.addSourceTarget(target);
    });
    addTargetVariations(data, "destination", "targets", [&iface](const std::string& target) {
        iface.addDestinationTarget(target);
    });
}

static void arrayPairProcess(nlohmann::json doc,
                             const std::string& key,
                             const std::function<void(std::string_view, std::string_view)>& pairOp)
{
    if (doc.contains(key)) {
        if (doc[key].is_array()) {
            for (const auto& val : doc[key]) {
                pairOp(val[0].get<std::string>(), val[1].get<std::string>());
            }
        } else {
            for (const auto& val : doc[key].items()) {
                pairOp(val.key(), val.value().get<std::string>());
            }
        }
    }
}

bool Federate::checkValidFilterType(bool useTypes,
                                    FilterTypes opType,
                                    const std::string& operation) const
{
    if ((useTypes) && (operation != "custom")) {
        if (strictConfigChecking) {
            logMessage(HELICS_LOG_LEVEL_ERROR,
                       "input and output types may only be specified for custom filters");
            throw(InvalidParameter(
                "input and output types may only be specified for custom filters"));
        }
        logMessage(HELICS_LOG_LEVEL_WARNING,
                   "input and output types may only be specified for custom filters");
        return false;
    }
    if (!useTypes) {
        if (opType == FilterTypes::UNRECOGNIZED) {
            if (strictConfigChecking) {
                const std::string emessage =
                    fmt::format("unrecognized filter operation:{}", operation);
                logMessage(HELICS_LOG_LEVEL_ERROR, emessage);

                throw(InvalidParameter(emessage));
            }
            logMessage(HELICS_LOG_LEVEL_WARNING,
                       fmt::format("unrecognized filter operation:{}", operation));
            return false;
        }
    }
    return true;
}

template<class INTERFACE>
static void
    loadPropertiesJson(Federate* fed, INTERFACE& iface, const nlohmann::json& json, bool strict)
{
    static constexpr std::string_view errorMessage =
        R"(interface properties require "name" and "value" fields)";
    if (json.contains("properties")) {
        const auto& props = json["properties"];
        if (props.is_array()) {
            for (const auto& prop : props) {
                if ((!prop.contains("name")) || (!prop.contains("value"))) {
                    if (strict) {
                        fed->logMessage(HELICS_LOG_LEVEL_ERROR, errorMessage);

                        throw(InvalidParameter(errorMessage));
                    }
                    fed->logMessage(HELICS_LOG_LEVEL_WARNING, errorMessage);
                    continue;
                }
                if (prop["value"].is_number()) {
                    iface.set(prop["name"].get<std::string>(), prop["value"].get<double>());
                } else {
                    iface.setString(prop["name"].get<std::string>(),
                                    prop["value"].get<std::string>());
                }
            }
        } else {
            if ((!props.contains("name")) || (!props.contains("value"))) {
                if (strict) {
                    fed->logMessage(HELICS_LOG_LEVEL_ERROR, errorMessage);

                    throw(InvalidParameter(errorMessage));
                }
                fed->logMessage(HELICS_LOG_LEVEL_WARNING, errorMessage);
            } else {
                if (props["value"].is_number()) {
                    iface.set(props["name"].get<std::string>(), props["value"].get<double>());
                } else {
                    iface.setString(props["name"].get<std::string>(),
                                    props["value"].get<std::string>());
                }
            }
        }
    }
}

void Federate::registerConnectorInterfacesJson(const std::string& jsonString)
{
    auto doc = fileops::loadJson(jsonString);
    registerConnectorInterfacesJsonDetail(fileops::JsonBuffer(doc));
}

void Federate::registerConnectorInterfacesJsonDetail(const fileops::JsonBuffer& jsonBuffer)
{
    using fileops::getOrDefault;
    const auto& json = jsonBuffer.json();
    bool defaultGlobal = false;
    fileops::replaceIfMember(json, "defaultglobal", defaultGlobal);

    const nlohmann::json& iface = (json.contains("interfaces")) ? json["interfaces"] : json;

    if (iface.contains("filters")) {
        for (const auto& filt : iface["filters"]) {
            const std::string key = getOrDefault(filt, "name", emptyStr);
            const std::string inputType = getOrDefault(filt, "inputType", emptyStr);
            const std::string outputType = getOrDefault(filt, "outputType", emptyStr);
            const bool cloningFlag = getOrDefault(filt, "cloning", false);
            const bool useTypes = !((inputType.empty()) && (outputType.empty()));
            const bool global = fileops::getOrDefault(filt, "global", defaultGlobal);
            const std::string operation =
                getOrDefault(filt,
                             "operation",
                             (cloningFlag) ? std::string("clone") : std::string("custom"));

            auto opType = filterTypeFromString(operation);
            if (!checkValidFilterType(useTypes, opType, operation)) {
                continue;
            }
            auto& filter =
                generateFilter(this, global, cloningFlag, key, opType, inputType, outputType);
            loadOptions(this, filt, filter);

            addTargetVariations(filt, "source", "endpoints", [&filter](const std::string& target) {
                filter.addSourceTarget(target);
            });
            addTargetVariations(filt,
                                "destination",
                                "endpoints",
                                [&filter](const std::string& target) {
                                    filter.addDestinationTarget(target);
                                });

            if (cloningFlag) {
                addTargets(filt, "delivery", [&filter](const std::string& target) {
                    static_cast<CloningFilter&>(filter).addDeliveryEndpoint(target);
                });
            }

            loadPropertiesJson(this, filter, filt, strictConfigChecking);
        }
    }
    if (iface.contains("translators")) {
        for (const auto& trans : iface["translators"]) {
            const std::string key = getOrDefault(trans, "name", emptyStr);

            std::string ttype = getOrDefault(trans, "type", std::string("custom"));
            auto opType = translatorTypeFromString(ttype);
            auto etype = fileops::getOrDefault(trans, "endpointtype", emptyStr);
            auto units = fileops::getOrDefault(trans, "unit", emptyStr);
            fileops::replaceIfMember(trans, "units", units);
            const bool global = fileops::getOrDefault(trans, "global", defaultGlobal);

            if (opType == TranslatorTypes::UNRECOGNIZED) {
                if (strictConfigChecking) {
                    const std::string emessage =
                        fmt::format("unrecognized translator type:{}", ttype);
                    logMessage(HELICS_LOG_LEVEL_ERROR, emessage);

                    throw(InvalidParameter(emessage));
                }
                logMessage(HELICS_LOG_LEVEL_WARNING,
                           fmt::format("unrecognized translator operation:{}", ttype));
                continue;
            }
            auto& translator = generateTranslator(this, global, key, opType, etype, units);
            loadOptions(this, trans, translator);

            addTargetVariations(trans,
                                "source",
                                "endpoints",
                                [&translator](const std::string& target) {
                                    translator.addSourceEndpoint(target);
                                });
            addTargetVariations(trans,
                                "destination",
                                "endpoints",
                                [&translator](const std::string& target) {
                                    translator.addDestinationEndpoint(target);
                                });
            addTargetVariations(trans,
                                "source",
                                "publications",
                                [&translator](const std::string& target) {
                                    translator.addPublication(target);
                                });
            addTargetVariations(trans,
                                "destination",
                                "inputs",
                                [&translator](const std::string& target) {
                                    translator.addInputTarget(target);
                                });
            addTargetVariations(trans,
                                "source",
                                "filters",
                                [&translator](const std::string& target) {
                                    translator.addSourceFilter(target);
                                });
            addTargetVariations(trans,
                                "destination",
                                "filters",
                                [&translator](const std::string& target) {
                                    translator.addDestinationFilter(target);
                                });
            loadPropertiesJson(this, translator, trans, strictConfigChecking);
        }
    }
    arrayPairProcess(json, "globals", [this](std::string_view key, std::string_view val) {
        setGlobal(key, val);
    });
    arrayPairProcess(json, "aliases", [this](std::string_view key, std::string_view val) {
        addAlias(key, val);
    });

    loadTags(json, [this](std::string_view tagname, std::string_view tagvalue) {
        this->setTag(tagname, tagvalue);
    });
    if (json.contains("helics")) {
        registerConnectorInterfacesJsonDetail(fileops::JsonBuffer(json["helics"]));
    }

    if (json.contains("potential_interfaces") || json.contains("potential_interface_templates")) {
        if (!potManager) {
            potManager = std::make_unique<PotentialInterfacesManager>(coreObject.get(), this);
        }
        potManager->loadPotentialInterfaces(json);
        hasPotentialInterfaces = true;
    }
}

static void arrayPairProcess(toml::value doc,
                             const std::string& key,
                             const std::function<void(std::string_view, std::string_view)>& pairOp)
{
    using fileops::isMember;
    if (isMember(doc, key)) {
        auto& info = toml::find(doc, key);
        if (info.is_array()) {
            for (auto& val : info.as_array()) {
                pairOp(static_cast<std::string_view>(val.as_array()[0].as_string()),
                       static_cast<std::string_view>(val.as_array()[1].as_string()));
            }
        } else {
            for (const auto& val : info.as_table()) {
                pairOp(val.first, static_cast<std::string_view>(val.second.as_string()));
            }
        }
    }
}

template<class INTERFACE>
static void
    loadPropertiesToml(Federate* fed, INTERFACE& iface, const toml::value& data, bool strict)
{
    static constexpr std::string_view errorMessage =
        R"(interface properties require "name" and "value" fields)";
    if (fileops::isMember(data, "properties")) {
        const auto& props = toml::find(data, "properties");
        if (props.is_array()) {
            const auto& propArray = props.as_array();
            for (const auto& prop : propArray) {
                std::string propname;
                propname = toml::find_or(prop, "name", propname);
                const toml::value uVal;
                const auto& propval = toml::find_or(prop, "value", uVal);

                if ((propname.empty()) || (propval.is_empty())) {
                    if (strict) {
                        fed->logMessage(HELICS_LOG_LEVEL_ERROR, errorMessage);

                        throw(InvalidParameter(errorMessage));
                    }
                    fed->logMessage(HELICS_LOG_LEVEL_WARNING, errorMessage);
                    continue;
                }
                if (propval.is_floating()) {
                    iface.set(propname, propval.as_floating());
                } else {
                    iface.setString(propname, static_cast<std::string_view>(propval.as_string()));
                }
            }
        } else {
            std::string propname;
            propname = toml::find_or(props, "name", propname);
            const toml::value uVal;
            const auto& propval = toml::find_or(props, "value", uVal);

            if ((propname.empty()) || (propval.is_empty())) {
                if (strict) {
                    fed->logMessage(HELICS_LOG_LEVEL_ERROR, errorMessage);

                    throw(InvalidParameter(errorMessage));
                }
                fed->logMessage(HELICS_LOG_LEVEL_WARNING, errorMessage);
            } else {
                if (propval.is_floating()) {
                    iface.set(propname, propval.as_floating());
                } else {
                    iface.setString(propname, static_cast<std::string_view>(propval.as_string()));
                }
            }
        }
    }
}

void Federate::registerConnectorInterfacesToml(const std::string& tomlString)
{
    using fileops::getOrDefault;
    using fileops::isMember;
    using fileops::replaceIfMember;

    toml::value doc;
    try {
        doc = fileops::loadToml(tomlString);
    }
    catch (const std::invalid_argument& ia) {
        throw(helics::InvalidParameter(ia.what()));
    }
    bool defaultGlobal = false;
    replaceIfMember(doc, "defaultglobal", defaultGlobal);

    if (isMember(doc, "filters")) {
        auto& filts = toml::find(doc, "filters");
        if (!filts.is_array()) {
            throw(helics::InvalidParameter("filters section in toml file must be an array"));
        }
        auto& filtArray = filts.as_array();
        for (const auto& filt : filtArray) {
            const std::string key = getOrDefault(filt, "name", emptyStr);
            const bool cloningflag = getOrDefault(filt, "cloning", false);
            const std::string inputType = getOrDefault(filt, "inputType", emptyStr);
            const std::string outputType = getOrDefault(filt, "outputType", emptyStr);
            const bool useTypes = !((inputType.empty()) && (outputType.empty()));
            const bool global = getOrDefault(filt, "global", defaultGlobal);
            const std::string operation = getOrDefault(filt, "operation", std::string("custom"));

            auto opType = filterTypeFromString(operation);
            if (!checkValidFilterType(useTypes, opType, operation)) {
                continue;
            }
            auto& filter =
                generateFilter(this, global, cloningflag, key, opType, inputType, outputType);

            loadOptions(this, filt, filter);
            addTargetVariations(filt, "source", "endpoints", [&filter](const std::string& target) {
                filter.addSourceTarget(target);
            });
            addTargetVariations(filt,
                                "destination",
                                "endpoints",
                                [&filter](const std::string& target) {
                                    filter.addDestinationTarget(target);
                                });
            if (cloningflag) {
                addTargets(filt, "delivery", [&filter](const std::string& target) {
                    static_cast<CloningFilter&>(filter).addDeliveryEndpoint(target);
                });
            }
            loadPropertiesToml(this, filter, filt, strictConfigChecking);
        }
    }
    if (isMember(doc, "translators")) {
        auto& transs = toml::find(doc, "translators");
        if (!transs.is_array()) {
            throw(helics::InvalidParameter("translators section in toml file must be an array"));
        }
        auto& transArray = transs.as_array();
        for (const auto& trans : transArray) {
            const std::string key = getOrDefault(trans, "name", emptyStr);

            std::string ttype = getOrDefault(trans, "type", std::string("custom"));
            auto opType = translatorTypeFromString(ttype);
            auto etype = fileops::getOrDefault(trans, "endpointtype", emptyStr);
            auto units = fileops::getOrDefault(trans, "unit", emptyStr);
            fileops::replaceIfMember(trans, "units", units);
            const bool global = fileops::getOrDefault(trans, "global", defaultGlobal);

            if (opType == TranslatorTypes::UNRECOGNIZED) {
                if (strictConfigChecking) {
                    const std::string emessage =
                        fmt::format("unrecognized translator type:{}", ttype);
                    logMessage(HELICS_LOG_LEVEL_ERROR, emessage);

                    throw(InvalidParameter(emessage));
                }
                logMessage(HELICS_LOG_LEVEL_WARNING,
                           fmt::format("unrecognized filter operation:{}", ttype));
                continue;
            }
            auto& translator = generateTranslator(this, global, key, opType, etype, units);
            loadOptions(this, trans, translator);

            addTargetVariations(trans,
                                "source",
                                "endpoints",
                                [&translator](const std::string& target) {
                                    translator.addSourceEndpoint(target);
                                });
            addTargetVariations(trans,
                                "destination",
                                "endpoints",
                                [&translator](const std::string& target) {
                                    translator.addDestinationEndpoint(target);
                                });
            addTargetVariations(trans,
                                "source",
                                "publications",
                                [&translator](const std::string& target) {
                                    translator.addPublication(target);
                                });
            addTargetVariations(trans,
                                "destination",
                                "inputs",
                                [&translator](const std::string& target) {
                                    translator.addInputTarget(target);
                                });
            addTargetVariations(trans,
                                "source",
                                "filters",
                                [&translator](const std::string& target) {
                                    translator.addSourceFilter(target);
                                });
            addTargetVariations(trans,
                                "destination",
                                "filters",
                                [&translator](const std::string& target) {
                                    translator.addDestinationFilter(target);
                                });
            loadPropertiesToml(this, translator, trans, strictConfigChecking);
        }
    }
    if (isMember(doc, "globals")) {
        auto& globals = toml::find(doc, "globals");
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

    arrayPairProcess(doc, "globals", [this](std::string_view key, std::string_view val) {
        setGlobal(key, val);
    });
    arrayPairProcess(doc, "aliases", [this](std::string_view key, std::string_view val) {
        addAlias(key, val);
    });

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
    coreObject->setQueryCallback(fedID, queryFunction, 1);
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
    if (hasPotentialInterfaces) {
        if (potManager->hasExtraCommands()) {
            return potManager->getCommand();
        }
    }
    return coreObject->getCommand(fedID);
}

std::pair<std::string, std::string> Federate::waitCommand()
{
    if (hasPotentialInterfaces) {
        if (potManager->hasExtraCommands()) {
            return potManager->getCommand();
        }
    }
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

void Federate::setFilterOperator(Filter& filt, std::shared_ptr<FilterOperator> filtOp)
{
    filt.setOperator(std::move(filtOp));
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

void Federate::setTranslatorOperator(Translator& trans,
                                     std::shared_ptr<TranslatorOperator> transOps)
{
    trans.setOperator(std::move(transOps));
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
        std::cerr << message << '\n';
    } else {
        std::cout << message << '\n';
    }
}

Interface::Interface(Federate* federate, InterfaceHandle hid, std::string_view actName):
    handle(hid), mName(actName)
{
    if (federate != nullptr) {
        const auto& crp = federate->getCorePointer();
        if (crp) {
            mCore = crp.get();
        }
    }
}

const std::string& Interface::getName() const
{
    return mCore->getHandleName(handle);
}

const std::string& Interface::getTarget() const
{
    return mCore->getSourceTargets(handle);
}

void Interface::addSourceTarget(std::string_view newTarget, InterfaceType hint)
{
    mCore->addSourceTarget(handle, newTarget, hint);
}

void Interface::addDestinationTarget(std::string_view newTarget, InterfaceType hint)
{
    mCore->addDestinationTarget(handle, newTarget, hint);
}

void Interface::removeTarget(std::string_view targetToRemove)
{
    mCore->removeTarget(handle, targetToRemove);
}

void Interface::addAlias(std::string_view alias)
{
    mCore->addAlias(getName(), alias);
}

const std::string& Interface::getInfo() const
{
    return mCore->getInterfaceInfo(handle);
}

void Interface::setInfo(std::string_view info)
{
    mCore->setInterfaceInfo(handle, info);
}

const std::string& Interface::getTag(std::string_view tag) const
{
    return mCore->getInterfaceTag(handle, tag);
}

void Interface::setTag(std::string_view tag, std::string_view value)
{
    mCore->setInterfaceTag(handle, tag, value);
}

void Interface::setOption(int32_t option, int32_t value)
{
    mCore->setHandleOption(handle, option, value);
}

int32_t Interface::getOption(int32_t option) const
{
    return mCore->getHandleOption(handle, option);
}

const std::string& Interface::getInjectionType() const
{
    return mCore->getInjectionType(handle);
}

const std::string& Interface::getExtractionType() const
{
    return mCore->getExtractionType(handle);
}

const std::string& Interface::getInjectionUnits() const
{
    return mCore->getInjectionUnits(handle);
}

const std::string& Interface::getExtractionUnits() const
{
    return mCore->getExtractionUnits(handle);
}

const std::string& Interface::getSourceTargets() const
{
    return mCore->getSourceTargets(handle);
}

const std::string& Interface::getDestinationTargets() const
{
    return mCore->getDestinationTargets(handle);
}

std::size_t Interface::getSourceTargetCount() const
{
    const auto& targets = getSourceTargets();
    if (targets.empty()) {
        return 0;
    }
    try {
        const nlohmann::json tvalues = fileops::loadJsonStr(targets);
        return (tvalues.is_array()) ? tvalues.size() : 1;
    }
    catch (...) {
        return 1;
    }
}
std::size_t Interface::getDestinationTargetCount() const
{
    const auto& targets = getDestinationTargets();
    if (targets.empty()) {
        return 0;
    }
    try {
        const nlohmann::json tvalues = fileops::loadJsonStr(targets);
        return (tvalues.is_array()) ? tvalues.size() : 1;
    }
    catch (...) {
        return 1;
    }
}

const std::string& Interface::getDisplayName() const
{
    return (mName.empty() ? getSourceTargets() : mName);
}

void Interface::close()
{
    mCore->closeHandle(handle);
    mCore = CoreFactory::getEmptyCorePtr();
}

void Interface::disconnectFromCore()
{
    mCore = CoreFactory::getEmptyCorePtr();
}

}  // namespace helics
