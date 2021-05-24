/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "Federate.hpp"

#include "../common/GuardedTypes.hpp"
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

Federate::Federate(const std::string& fedName, const FederateInfo& fi): name(fedName)
{
    if (name.empty()) {
        name = fi.defName;
    }
    if (fi.coreName.empty()) {
        if (!fi.forceNewCore) {
            coreObject = CoreFactory::findJoinableCoreOfType(fi.coreType);
        }
        if (!coreObject) {
            if (!name.empty()) {
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
    fedID = coreObject->registerFederate(name, fi);
    nameSegmentSeparator = fi.separator;
    strictConfigChecking = fi.checkFlagProperty(helics_flag_strict_config_checking, true);
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
    name(fedName)
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
    if (name.empty()) {
        name = fi.defName;
    }
    fedID = coreObject->registerFederate(name, fi);
    nameSegmentSeparator = fi.separator;
    strictConfigChecking = fi.checkFlagProperty(helics_flag_strict_config_checking, true);
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
    asyncCallInfo = std::move(fed.asyncCallInfo);
    fManager = std::move(fed.fManager);
    name = std::move(fed.name);
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
    fManager = std::move(fed.fManager);
    name = std::move(fed.name);
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
        case modes::startup:
            try {
                coreObject->enterInitializingMode(fedID);
                currentMode = modes::initializing;
                currentTime = coreObject->getCurrentTime(fedID);
                startupToInitializeStateTransition();
            }
            catch (const HelicsException&) {
                currentMode = modes::error;
                throw;
            }
            break;
        case modes::pending_init:
            enterInitializingModeComplete();
            break;
        case modes::initializing:
            break;
        default:
            throw(InvalidFunctionCall("cannot transition from current mode to initializing mode"));
    }
}

void Federate::enterInitializingModeAsync()
{
    auto cm = currentMode.load();
    if (cm == modes::startup) {
        auto asyncInfo = asyncCallInfo->lock();
        if (currentMode.compare_exchange_strong(cm, modes::pending_init)) {
            asyncInfo->initFuture = std::async(std::launch::async, [this]() {
                coreObject->enterInitializingMode(fedID);
            });
        }
    } else if (cm == modes::pending_init) {
        return;
    } else if (cm != modes::initializing) {
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
        case modes::pending_init:
            return (asyncInfo->initFuture.wait_for(wait_delay) == ready);
        case modes::pending_exec:
            return (asyncInfo->execFuture.wait_for(wait_delay) == ready);
        case modes::pending_time:
            return (asyncInfo->timeRequestFuture.wait_for(wait_delay) == ready);
        case modes::pending_iterative_time:
            return (asyncInfo->timeRequestIterativeFuture.wait_for(wait_delay) == ready);
        case modes::pending_finalize:
            return (asyncInfo->finalizeFuture.wait_for(wait_delay) == ready);
        default:
            return false;
    }
}

void Federate::enterInitializingModeComplete()
{
    switch (currentMode.load()) {
        case modes::pending_init: {
            auto asyncInfo = asyncCallInfo->lock();
            try {
                asyncInfo->initFuture.get();
            }
            catch (const std::exception&) {
                currentMode = modes::error;
                throw;
            }
            currentMode = modes::initializing;
            currentTime = coreObject->getCurrentTime(fedID);
            startupToInitializeStateTransition();
        } break;
        case modes::initializing:
            break;
        case modes::startup:
            enterInitializingMode();
            break;
        default:
            throw(InvalidFunctionCall(
                "cannot call Initialization Complete function without first calling "
                "enterInitializingModeAsync function or being in startup mode"));
    }
}

iteration_result Federate::enterExecutingMode(iteration_request iterate)
{
    iteration_result res = iteration_result::next_step;
    switch (currentMode) {
        case modes::startup:
        case modes::pending_init:
            enterInitializingMode();
            FALLTHROUGH
            /* FALLTHROUGH */
        case modes::initializing: {
            res = coreObject->enterExecutingMode(fedID, iterate);
            switch (res) {
                case iteration_result::next_step:
                    currentMode = modes::executing;
                    currentTime = timeZero;
                    initializeToExecuteStateTransition(res);
                    break;
                case iteration_result::iterating:
                    currentMode = modes::initializing;
                    currentTime = initializationTime;
                    initializeToExecuteStateTransition(res);
                    break;
                case iteration_result::error:
                    // LCOV_EXCL_START
                    currentMode = modes::error;
                    break;
                    // LCOV_EXCL_STOP
                case iteration_result::halted:
                    currentMode = modes::finished;
                    break;
            }
            break;
        }
        case modes::pending_exec:
            return enterExecutingModeComplete();
        case modes::executing:
            // already in this state --> do nothing
            break;
        case modes::pending_time:
            requestTimeComplete();
            break;
        case modes::pending_iterative_time: {
            auto result = requestTimeIterativeComplete();
            return (result.state == iteration_result::iterating) ? iteration_result::next_step :
                                                                   result.state;
        }
        default:
            throw(InvalidFunctionCall("cannot transition from current state to execution state"));
            break;
    }
    return res;
}

void Federate::enterExecutingModeAsync(iteration_request iterate)
{
    switch (currentMode) {
        case modes::startup: {
            auto eExecFunc = [this, iterate]() {
                coreObject->enterInitializingMode(fedID);
                currentTime = coreObject->getCurrentTime(fedID);
                startupToInitializeStateTransition();
                return coreObject->enterExecutingMode(fedID, iterate);
            };
            auto asyncInfo = asyncCallInfo->lock();
            currentMode = modes::pending_exec;
            asyncInfo->execFuture = std::async(std::launch::async, eExecFunc);
        } break;
        case modes::pending_init:
            enterInitializingModeComplete();
            FALLTHROUGH
            /* FALLTHROUGH */
        case modes::initializing: {
            auto eExecFunc = [this, iterate]() {
                return coreObject->enterExecutingMode(fedID, iterate);
            };
            auto asyncInfo = asyncCallInfo->lock();
            currentMode = modes::pending_exec;
            asyncInfo->execFuture = std::async(std::launch::async, eExecFunc);
        } break;
        case modes::pending_exec:
        case modes::executing:
        case modes::pending_time:
        case modes::pending_iterative_time:
            // we are already in or executing a function that would achieve this request
            break;
        default:
            throw(InvalidFunctionCall("cannot transition from current state to execution state"));
            break;
    }
}

iteration_result Federate::enterExecutingModeComplete()
{
    switch (currentMode.load()) {
        case modes::pending_exec: {
            auto asyncInfo = asyncCallInfo->lock();
            try {
                auto res = asyncInfo->execFuture.get();
                switch (res) {
                    case iteration_result::next_step:
                        currentMode = modes::executing;
                        currentTime = timeZero;
                        initializeToExecuteStateTransition(iteration_result::next_step);
                        break;
                    case iteration_result::iterating:
                        currentMode = modes::initializing;
                        currentTime = initializationTime;
                        initializeToExecuteStateTransition(iteration_result::iterating);
                        break;
                    case iteration_result::error:
                        // LCOV_EXCL_START
                        currentMode = modes::error;
                        break;
                        // LCOV_EXCL_STOP
                    case iteration_result::halted:
                        currentMode = modes::finished;
                        break;
                }

                return res;
            }
            catch (const std::exception&) {
                currentMode = modes::error;
                throw;
            }
        }
        default:
            return enterExecutingMode();
    }
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
    const std::function<void(int, const std::string&, const std::string&)>& logFunction)
{
    coreObject->setLoggingCallback(fedID, logFunction);
}

void Federate::setFlagOption(int flag, bool flagValue)
{
    coreObject->setFlagOption(fedID, flag, flagValue);
}

bool Federate::getFlagOption(int flag) const
{
    return coreObject->getFlagOption(fedID, flag);
}
void Federate::finalize()
{  // since finalize is called in the destructor we can't allow any potential virtual function calls
    switch (currentMode) {
        case modes::startup:
            break;
        case modes::pending_init: {
            auto asyncInfo = asyncCallInfo->lock();
            try {
                asyncInfo->initFuture.get();
            }
            catch (const std::exception&) {
                currentMode = modes::error;
                throw;
            }
        } break;
        case modes::initializing:
            break;
        case modes::pending_exec:
            asyncCallInfo->lock()->execFuture.get();
            break;
        case modes::pending_time:
            asyncCallInfo->lock()->timeRequestFuture.get();
            break;
        case modes::executing:
        case modes::finished:
            break;
        case modes::pending_iterative_time:
            asyncCallInfo->lock()
                ->timeRequestIterativeFuture.get();  // I don't care about the return any more
            break;
        case modes::finalize:
        case modes::error:
            return;
            // do nothing
        case modes::pending_finalize:
            finalizeComplete();
            return;
        default:
            throw(InvalidFunctionCall("cannot call finalize in present state"));  // LCOV_EXCL_LINE
    }
    coreObject->finalize(fedID);
    if (fManager) {
        fManager->closeAllFilters();
    }
    currentMode = modes::finalize;
}

void Federate::finalizeAsync()
{
    switch (currentMode) {
        case modes::pending_init:
            enterInitializingModeComplete();
            break;
        case modes::pending_exec:
            enterExecutingModeComplete();
            break;
        case modes::pending_time:
            requestTimeComplete();
            break;
        case modes::pending_iterative_time:
            requestTimeIterativeComplete();
            break;
        case modes::finalize:
        case modes::error:
        case modes::pending_finalize:
            return;
            // do nothing
        default:
            break;
    }
    auto finalizeFunc = [this]() { return coreObject->finalize(fedID); };
    auto asyncInfo = asyncCallInfo->lock();
    currentMode = modes::pending_finalize;
    asyncInfo->finalizeFuture = std::async(std::launch::async, finalizeFunc);
}

/** complete the asynchronous terminate pair*/
void Federate::finalizeComplete()
{
    if (currentMode == modes::pending_finalize) {
        auto asyncInfo = asyncCallInfo->lock();
        asyncInfo->finalizeFuture.get();
        currentMode = modes::finalize;
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
        case modes::pending_init:
            enterInitializingModeComplete();
            break;
        case modes::pending_exec:
            enterExecutingModeComplete();
            break;
        case modes::pending_time:
            requestTimeComplete();
            break;
        case modes::pending_iterative_time:
            requestTimeIterativeComplete();
            break;
        case modes::pending_finalize:
            finalizeComplete();
            break;
        default:
            break;
    }
}

void Federate::error(int errorcode)
{
    localError(errorcode);
}

void Federate::error(int errorcode, const std::string& message)
{
    localError(errorcode, message);
}

void Federate::localError(int errorcode)
{
    std::string errorString = "local error " + std::to_string(errorcode) + " in federate " + name;
    localError(errorcode, errorString);
}

void Federate::globalError(int errorcode)
{
    std::string errorString = "global error " + std::to_string(errorcode) + " in federate " + name;
    globalError(errorcode, errorString);
}

void Federate::localError(int errorcode, const std::string& message)
{
    if (!coreObject) {
        throw(InvalidFunctionCall(
            "cannot generate a federation error on uninitialized or disconnected Federate"));
    }
    completeOperation();
    currentMode = modes::error;
    coreObject->localError(fedID, errorcode, message);
}

void Federate::globalError(int errorcode, const std::string& message)
{
    if (!coreObject) {
        throw(InvalidFunctionCall(
            "cannot generate a federation error on uninitialized or disconnected Federate"));
    }
    completeOperation();
    currentMode = modes::error;
    coreObject->globalError(fedID, errorcode, message);
}

Time Federate::requestTime(Time nextInternalTimeStep)
{
    switch (currentMode) {
        case modes::executing:
            try {
                auto newTime = coreObject->timeRequest(fedID, nextInternalTimeStep);
                Time oldTime = currentTime;
                currentTime = newTime;
                updateTime(newTime, oldTime);
                if (newTime == Time::maxVal()) {
                    currentMode = modes::finished;
                }
                return newTime;
            }
            catch (const FunctionExecutionFailure&) {
                currentMode = modes::error;
                throw;
            }
            break;
        case modes::finalize:
        case modes::finished:
            return Time::maxVal();
        default:
            break;
    }

    throw(InvalidFunctionCall("cannot call request time in present state"));
}

iteration_time Federate::requestTimeIterative(Time nextInternalTimeStep, iteration_request iterate)
{
    if (currentMode == modes::executing) {
        auto iterativeTime = coreObject->requestTimeIterative(fedID, nextInternalTimeStep, iterate);
        Time oldTime = currentTime;
        switch (iterativeTime.state) {
            case iteration_result::next_step:
                currentTime = iterativeTime.grantedTime;
                FALLTHROUGH
                /* FALLTHROUGH */
            case iteration_result::iterating:
                updateTime(currentTime, oldTime);
                break;
            case iteration_result::halted:
                currentTime = iterativeTime.grantedTime;
                updateTime(currentTime, oldTime);
                currentMode = modes::finished;
                break;
            case iteration_result::error:
                // LCOV_EXCL_START
                currentMode = modes::error;
                break;
                // LCOV_EXCL_STOP
        }
        return iterativeTime;
    }
    if (currentMode == modes::finalize || currentMode == modes::finished) {
        return {Time::maxVal(), iteration_result::halted};
    }
    throw(InvalidFunctionCall("cannot call request time in present state"));
}

void Federate::requestTimeAsync(Time nextInternalTimeStep)
{
    auto exp = modes::executing;
    if (currentMode.compare_exchange_strong(exp, modes::pending_time)) {
        auto asyncInfo = asyncCallInfo->lock();
        asyncInfo->timeRequestFuture =
            std::async(std::launch::async, [this, nextInternalTimeStep]() {
                return coreObject->timeRequest(fedID, nextInternalTimeStep);
            });
    } else {
        throw(InvalidFunctionCall("cannot call request time in present state"));
    }
}

void Federate::requestTimeIterativeAsync(Time nextInternalTimeStep, iteration_request iterate)
{
    auto exp = modes::executing;
    if (currentMode.compare_exchange_strong(exp, modes::pending_iterative_time)) {
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
    auto exp = modes::pending_time;
    if (currentMode.compare_exchange_strong(exp, modes::executing)) {
        auto asyncInfo = asyncCallInfo->lock();
        auto newTime = asyncInfo->timeRequestFuture.get();
        asyncInfo.unlock();  // remove the lock;
        Time oldTime = currentTime;
        currentTime = newTime;
        updateTime(newTime, oldTime);
        return newTime;
    }
    throw(InvalidFunctionCall(
        "cannot call finalize requestTime without first calling requestTimeIterative function"));
}

/** finalize the time advancement request
@return the granted time step*/
iteration_time Federate::requestTimeIterativeComplete()
{
    auto asyncInfo = asyncCallInfo->lock();
    auto exp = modes::pending_iterative_time;
    if (currentMode.compare_exchange_strong(exp, modes::executing)) {
        auto iterativeTime = asyncInfo->timeRequestIterativeFuture.get();
        Time oldTime = currentTime;
        switch (iterativeTime.state) {
            case iteration_result::next_step:
                currentTime = iterativeTime.grantedTime;
                FALLTHROUGH
                /* FALLTHROUGH */
            case iteration_result::iterating:
                updateTime(currentTime, oldTime);
                break;
            case iteration_result::halted:
                currentTime = iterativeTime.grantedTime;
                updateTime(currentTime, oldTime);
                currentMode = modes::finished;
                break;
            case iteration_result::error:
                // LCOV_EXCL_START
                currentMode = modes::error;
                break;
                // LCOV_EXCL_STOP
        }
        return iterativeTime;
    }
    throw(InvalidFunctionCall(
        "cannot call finalize requestTimeIterative without first calling requestTimeIterativeAsync function"));
}

void Federate::updateTime(Time /*newTime*/, Time /*oldTime*/)
{
    // child classes would likely implement this
}

void Federate::startupToInitializeStateTransition()
{
    // child classes may do something with this
}
void Federate::initializeToExecuteStateTransition(iteration_result /*unused*/)
{
    // child classes may do something with this
}

void Federate::registerInterfaces(const std::string& configString)
{
    registerFilterInterfaces(configString);
}

void Federate::registerFilterInterfaces(const std::string& configString)
{
    if (hasTomlExtension(configString)) {
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
                              filter_types operation,
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
        return (global) ? make_cloning_filter(GLOBAL, operation, fed, name) :
                          make_cloning_filter(operation, fed, name);
    }
    return (global) ? make_filter(GLOBAL, operation, fed, name) : make_filter(operation, fed, name);
}

const std::string emptyStr;

template<class Inp>
static void loadOptions(Federate* fed, const Inp& data, Filter& filt)
{
    addTargets(data, "flags", [&filt](const std::string& target) {
        if (target.front() != '-') {
            filt.setOption(getOptionIndex(target), true);
        } else {
            filt.setOption(getOptionIndex(target.substr(2)), false);
        }
    });
    processOptions(
        data,
        [](const std::string& option) { return getOptionIndex(option); },
        [](const std::string& value) { return getOptionValue(value); },
        [&filt](int32_t option, int32_t value) { filt.setOption(option, value); });

    auto info = getOrDefault(data, "info", emptyStr);
    if (!info.empty()) {
        fed->setInfo(filt.getHandle(), info);
    }
    auto asrc = [&filt](const std::string& target) { filt.addSourceTarget(target); };
    auto adest = [&filt](const std::string& target) { filt.addDestinationTarget(target); };
    addTargets(data, "targets", asrc);
    addTargets(data, "sourcetargets", asrc);
    addTargets(data, "desttargets", adest);
    addTargets(data, "sourceTargets", asrc);
    addTargets(data, "destTargets", adest);
}

void Federate::registerFilterInterfacesJson(const std::string& jsonString)
{
    auto doc = loadJson(jsonString);

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
                    logMessage(helics_log_level_error,
                               "input and output types may only be specified for custom filters");
                    throw(InvalidParameter(
                        "input and output types may only be specified for custom filters"));
                }
                logMessage(helics_log_level_warning,
                           "input and output types may only be specified for custom filters");
                continue;
            }
            if (!useTypes) {
                if (opType == filter_types::unrecognized) {
                    if (strictConfigChecking) {
                        std::string emessage =
                            fmt::format("unrecognized filter operation:{}", operation);
                        logMessage(helics_log_level_error, emessage);

                        throw(InvalidParameter(emessage));
                    }
                    logMessage(helics_log_level_warning,
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
                                    helics_log_level_error,
                                    R"(filter properties require "name" and "value" fields)");

                                throw(InvalidParameter(
                                    R"(filter properties require "name" and "value" fields)"));
                            }
                            logMessage(helics_log_level_warning,
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
                            logMessage(helics_log_level_error,
                                       R"(filter properties require "name" and "value" fields)");

                            throw(InvalidParameter(
                                R"(filter properties require "name" and "value" fields)"));
                        }
                        logMessage(helics_log_level_warning,
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
}

void Federate::registerFilterInterfacesToml(const std::string& tomlString)
{
    toml::value doc;
    try {
        doc = loadToml(tomlString);
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
                    logMessage(helics_log_level_error,
                               "input and output types may only be specified for custom filters");
                    throw(InvalidParameter(
                        "input and output types may only be specified for custom filters"));
                }
                logMessage(helics_log_level_warning,
                           "input and output types may only be specified for custom filters");
                continue;
            }
            if (!useTypes) {
                if (opType == filter_types::unrecognized) {
                    auto emessage = fmt::format("unrecognized filter operation:{}", operation);
                    if (strictConfigChecking) {
                        logMessage(helics_log_level_error, emessage);

                        throw(InvalidParameter(emessage));
                    }
                    logMessage(helics_log_level_warning, emessage);
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
                                    helics_log_level_error,
                                    R"(filter properties require "name" and "value" fields)");

                                throw(InvalidParameter(
                                    R"(filter properties require "name" and "value" fields)"));
                            }
                            logMessage(helics_log_level_warning,
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
                            logMessage(helics_log_level_error,
                                       R"(filter properties require "name" and "value" fields)");

                            throw(InvalidParameter(
                                R"(filter properties require "name" and "value" fields)"));
                        }
                        logMessage(helics_log_level_warning,
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
std::string Federate::query(const std::string& queryStr, helics_sequencing_mode mode)
{
    std::string res;
    if (queryStr == "name") {
        res = getName();
    } else if (queryStr == "corename") {
        if (coreObject) {
            res = coreObject->getIdentifier();
        } else {
            res = "#disconnected";
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
            res = "#disconnected";
        }
    }
    return res;
}

std::string Federate::query(const std::string& target,
                            const std::string& queryStr,
                            helics_sequencing_mode mode)
{
    std::string res;
    if ((target.empty()) || (target == "federate") || (target == getName())) {
        res = query(queryStr);
    } else {
        if (coreObject) {
            res = coreObject->query(target, queryStr, mode);
        } else {
            res = "#disconnected";
        }
    }
    return res;
}

query_id_t Federate::queryAsync(const std::string& target,
                                const std::string& queryStr,
                                helics_sequencing_mode mode)
{
    auto queryFut = std::async(std::launch::async, [this, target, queryStr, mode]() {
        return coreObject->query(target, queryStr, mode);
    });
    auto asyncInfo = asyncCallInfo->lock();
    int cnt = asyncInfo->queryCounter++;

    asyncInfo->inFlightQueries.emplace(cnt, std::move(queryFut));
    return query_id_t(cnt);
}

query_id_t Federate::queryAsync(const std::string& queryStr, helics_sequencing_mode mode)
{
    auto queryFut =
        std::async(std::launch::async, [this, queryStr, mode]() { return query(queryStr, mode); });
    auto asyncInfo = asyncCallInfo->lock();
    int cnt = asyncInfo->queryCounter++;

    asyncInfo->inFlightQueries.emplace(cnt, std::move(queryFut));
    return query_id_t(cnt);
}

std::string Federate::queryComplete(query_id_t queryIndex)  // NOLINT
{
    auto asyncInfo = asyncCallInfo->lock();
    auto fnd = asyncInfo->inFlightQueries.find(queryIndex.value());
    if (fnd != asyncInfo->inFlightQueries.end()) {
        return fnd->second.get();
    }
    return {"#invalid"};
}

void Federate::setQueryCallback(const std::function<std::string(const std::string&)>& queryFunction)
{
    if (coreObject) {
        coreObject->setQueryCallback(fedID, queryFunction);
    } else {
        throw(InvalidFunctionCall(
            " setQueryCallback cannot be called on uninitialized federate or after finalize call"));
    }
}

bool Federate::isQueryCompleted(query_id_t queryIndex) const  // NOLINT
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
    if (coreObject) {
        coreObject->setGlobal(valueName, value);
    } else {
        throw(InvalidFunctionCall(
            " setGlobal cannot be called on uninitialized federate or after finalize call"));
    }
}

void Federate::addDependency(const std::string& fedName)
{
    if (coreObject) {
        coreObject->addDependency(fedID, fedName);
    } else {
        throw(InvalidFunctionCall(
            "addDependency cannot be called on uninitialized federate or after finalize call"));
    }
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

void Federate::addSourceTarget(const Filter& filt, const std::string& targetEndpoint)
{
    if (coreObject) {
        coreObject->addSourceTarget(filt.getHandle(), targetEndpoint);
    } else {
        throw(InvalidFunctionCall(
            "add source target cannot be called on uninitialized federate or after finalize call"));
    }
}

void Federate::addDestinationTarget(const Filter& filt, const std::string& targetEndpoint)
{
    if (coreObject) {
        coreObject->addDestinationTarget(filt.getHandle(), targetEndpoint);
    } else {
        throw(InvalidFunctionCall(
            "add destination target cannot be called on uninitialized federate or after finalize call"));
    }
}

const std::string& Federate::getInterfaceName(interface_handle handle) const
{
    return (coreObject) ? (coreObject->getHandleName(handle)) : emptyStr;
}

const std::string& Federate::getInjectionType(interface_handle handle) const
{
    return (coreObject) ? (coreObject->getInjectionType(handle)) : emptyStr;
}

const std::string& Federate::getExtractionType(interface_handle handle) const
{
    return (coreObject) ? (coreObject->getExtractionType(handle)) : emptyStr;
}

const std::string& Federate::getInjectionUnits(interface_handle handle) const
{
    return (coreObject) ? (coreObject->getInjectionUnits(handle)) : emptyStr;
}

const std::string& Federate::getExtractionUnits(interface_handle handle) const
{
    return (coreObject) ? (coreObject->getExtractionUnits(handle)) : emptyStr;
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

void Federate::setInterfaceOption(interface_handle handle, int32_t option, int32_t option_value)
{
    if (coreObject) {
        coreObject->setHandleOption(handle, option, option_value);
    } else {
        throw(InvalidFunctionCall(
            "setInterfaceOption cannot be called on uninitialized federate or after finalize call"));
    }
}

/** get the current value for an interface option*/
int32_t Federate::getInterfaceOption(interface_handle handle, int32_t option)
{
    return (coreObject) ? coreObject->getHandleOption(handle, option) : 0;
}

void Federate::closeInterface(interface_handle handle)
{
    if (coreObject) {
        coreObject->closeHandle(handle);
    }
    // well if there is no core object it already is closed
}

void Federate::setInfo(interface_handle handle, const std::string& info)
{
    if (coreObject) {
        coreObject->setInterfaceInfo(handle, info);
    } else {
        throw(
            InvalidFunctionCall("cannot call set info on uninitialized or disconnected federate"));
    }
}

std::string const& Federate::getInfo(interface_handle handle)
{
    return (coreObject) ? coreObject->getInterfaceInfo(handle) : emptyStr;
}

void Federate::logMessage(int level, const std::string& message) const
{
    if (coreObject) {
        coreObject->logMessage(fedID, level, message);
    } else if (level <= helics_log_level_warning) {
        std::cerr << message << std::endl;
    } else {
        std::cout << message << std::endl;
    }
}

}  // namespace helics
