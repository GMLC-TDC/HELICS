/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See the top-level NOTICE for
additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "../core/BrokerFactory.hpp"
#include "../core/CoreFactory.hpp"
#include "../core/core-exceptions.hpp"
#include "../core/coreTypeOperations.hpp"
#include "../core/helicsVersion.hpp"
#include "../helics.hpp"
#include "../helics_enums.h"
#include "api-data.h"
#include "helics/helics-config.h"
#include "helicsApps.h"
#include "helicsCore.h"
#include "internal/api_objects.h"

#include <algorithm>
#include <csignal>
#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#ifdef HELICS_ENABLE_ZMQ_CORE
#    include "../network/zmq/ZmqContextManager.h"
#endif

const char* helicsGetVersion(void)
{
    return helics::versionString;
}

const char* helicsGetBuildFlags(void)
{
    return helics::buildFlags;
}

const char* helicsGetCompilerVersion(void)
{
    return helics::compiler;
}

const char* helicsGetSystemInfo(void)
{
    static const std::string systemInfo{helics::core::systemInfo()};
    return systemInfo.c_str();
}

static constexpr const char* nullstrPtr = "";

// NOLINTNEXTLINE
const std::string gHelicsEmptyStr;

HelicsError helicsErrorInitialize(void)
{
    HelicsError err;
    err.error_code = 0;
    err.message = nullstrPtr;
    return err;
}

/** clear an error object*/
void helicsErrorClear(HelicsError* err)
{
    if (err != nullptr) {
        err->error_code = 0;
        err->message = nullstrPtr;
    }
}

namespace {
void signalHandler(int /*signum*/)
{
    helicsAbort(HELICS_ERROR_USER_ABORT, "user abort");
    // add a sleep to give the abort a chance to propagate to other federates
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << std::flush;
    exit(HELICS_ERROR_USER_ABORT);
}

void signalHandlerNoExit(int /*signum*/)
{
    helicsAbort(HELICS_ERROR_USER_ABORT, "user abort");
    // add a sleep to give the abort a chance to propagate to other federates
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << std::flush;
}

void signalHandlerThreaded(int signum)
{
    std::thread sigthread(signalHandler, signum);
    sigthread.detach();
}

void signalHandlerThreadedNoExit(int signum)
{
    std::thread sigthread(signalHandlerNoExit, signum);
    sigthread.detach();
}

}  // namespace
void helicsLoadSignalHandler()
{
    static_cast<void>(signal(SIGINT, signalHandler));
}

void helicsLoadThreadedSignalHandler()
{
    static_cast<void>(signal(SIGINT, signalHandlerThreaded));
}

void helicsClearSignalHandler()
{
    static_cast<void>(signal(SIGINT, SIG_DFL));
}

namespace {
HelicsBool (*keyHandler)(int) = nullptr;

void signalHandlerCallback(int signum)
{
    HelicsBool runDefaultSignalHandler{HELICS_TRUE};
    if (keyHandler != nullptr) {
        runDefaultSignalHandler = keyHandler(signum);
    }
    if (runDefaultSignalHandler != HELICS_FALSE) {
        signalHandler(signum);
    }
}

void signalHandlerCallbackNoExit(int signum)
{
    HelicsBool runDefaultSignalHandler{HELICS_TRUE};
    if (keyHandler != nullptr) {
        runDefaultSignalHandler = keyHandler(signum);
    }
    if (runDefaultSignalHandler != HELICS_FALSE) {
        signalHandlerNoExit(signum);
    }
}

void signalHandlerThreadedCallback(int signum)
{
    HelicsBool runDefaultSignalHandler{HELICS_TRUE};
    if (keyHandler != nullptr) {
        runDefaultSignalHandler = keyHandler(signum);
    }
    if (runDefaultSignalHandler != HELICS_FALSE) {
        signalHandlerThreaded(signum);
    }
}

void signalHandlerThreadedCallbackNoExit(int signum)
{
    HelicsBool runDefaultSignalHandler{HELICS_TRUE};
    if (keyHandler != nullptr) {
        runDefaultSignalHandler = keyHandler(signum);
    }
    if (runDefaultSignalHandler != HELICS_FALSE) {
        signalHandlerThreadedNoExit(signum);
    }
}
}  // namespace

void helicsLoadSignalHandlerCallback(HelicsBool (*handler)(int), HelicsBool useSeparateThread)
{
    keyHandler = handler;
    if (handler != nullptr) {
        if (useSeparateThread != HELICS_FALSE) {
            static_cast<void>(signal(SIGINT, signalHandlerThreadedCallback));
        } else {
            static_cast<void>(signal(SIGINT, signalHandlerCallback));
        }

    } else {
        if (useSeparateThread != HELICS_FALSE) {
            helicsLoadThreadedSignalHandler();
        } else {
            helicsLoadSignalHandler();
        }
    }
}

void helicsLoadSignalHandlerCallbackNoExit(HelicsBool (*handler)(int), HelicsBool useSeparateThread)
{
    keyHandler = handler;
    if (handler != nullptr) {
        if (useSeparateThread != HELICS_FALSE) {
            static_cast<void>(signal(SIGINT, signalHandlerThreadedCallbackNoExit));
        } else {
            static_cast<void>(signal(SIGINT, signalHandlerCallbackNoExit));
        }

    } else {
        if (useSeparateThread != HELICS_FALSE) {
            static_cast<void>(signal(SIGINT, signalHandlerThreadedNoExit));
        } else {
            static_cast<void>(signal(SIGINT, signalHandlerNoExit));
        }
    }
}

HelicsBool helicsIsCoreTypeAvailable(const char* type)
{
    if (type == nullptr) {
        return HELICS_FALSE;
    }
    auto coreType = helics::core::coreTypeFromString(type);
    return (helics::core::isCoreTypeAvailable(coreType)) ? HELICS_TRUE : HELICS_FALSE;
}

// typedef enum {

//    HELICS_OK = 0, /*!< the function executed successfully */
//    HELICS_ERROR_INVALID_OBJECT, /*!< indicator that the object used was not a valid object */
//    HELICS_ERROR_INVALID_ARGUMENT, /*!< the parameter passed was invalid and unable to be used*/
//    HELICS_DISCARD, /*!< the input was discarded and not used for some reason */
//    HELICS_TERMINATED, /*!< the federate has terminated and the call cannot be completed*/
//    HELICS_WARNING, /*!< the function issued a warning of some kind */
//    HELICS_INVALID_STATE_TRANSITION, /*!< error issued when an invalid state transition occurred */
//    HELICS_INVALID_FUNCTION_CALL, /*!< the call made was invalid in the present state of the calling object*/
//    HELICS_ERROR /*!< the function produced an error */
//} void;

/** this function is based on the lippencott function template
http://cppsecrets.blogspot.com/2013/12/using-lippincott-function-for.html
*/
static constexpr char unknown_err_string[] = "unknown error";

void helicsErrorHandler(HelicsError* err) noexcept
{
    if (err == nullptr) {
        return;
    }
    try {
        try {
            // this is intended to be a single '='
            if (const std::exception_ptr eptr = std::current_exception()) {
                std::rethrow_exception(eptr);
            } else {
                // LCOV_EXCL_START
                err->error_code = HELICS_ERROR_EXTERNAL_TYPE;
                err->message = unknown_err_string;
                // LCOV_EXCL_STOP
            }
        }
        catch (const helics::InvalidFunctionCall& ifc) {
            err->error_code = HELICS_ERROR_INVALID_FUNCTION_CALL;
            err->message = getMasterHolder()->addErrorString(ifc.what());
        }
        catch (const helics::InvalidParameter& ip) {
            err->error_code = HELICS_ERROR_INVALID_ARGUMENT;
            err->message = getMasterHolder()->addErrorString(ip.what());
        }
        catch (const helics::RegistrationFailure& rf) {
            err->error_code = HELICS_ERROR_REGISTRATION_FAILURE;
            err->message = getMasterHolder()->addErrorString(rf.what());
        }
        catch (const helics::ConnectionFailure& cf) {
            err->error_code = HELICS_ERROR_CONNECTION_FAILURE;
            err->message = getMasterHolder()->addErrorString(cf.what());
        }
        // LCOV_EXCL_START
        catch (const helics::InvalidIdentifier& iid) {
            err->error_code = HELICS_ERROR_INVALID_OBJECT;
            err->message = getMasterHolder()->addErrorString(iid.what());
        }
        catch (const helics::HelicsSystemFailure& ht) {
            err->error_code = HELICS_ERROR_SYSTEM_FAILURE;
            err->message = getMasterHolder()->addErrorString(ht.what());
        }
        // LCOV_EXCL_STOP
        catch (const helics::HelicsException& he) {
            err->error_code = HELICS_ERROR_OTHER;
            err->message = getMasterHolder()->addErrorString(he.what());
        }
        catch (const std::exception& exc) {
            err->error_code = HELICS_ERROR_EXTERNAL_TYPE;
            err->message = getMasterHolder()->addErrorString(exc.what());
        }
        // LCOV_EXCL_START
        catch (...) {
            err->error_code = HELICS_ERROR_EXTERNAL_TYPE;
            err->message = unknown_err_string;
        }
        // LCOV_EXCL_STOP
    }
    // LCOV_EXCL_START
    catch (...) {
        err->error_code = HELICS_ERROR_EXTERNAL_TYPE;
        err->message = unknown_err_string;
    }
    // LCOV_EXCL_STOP
}

static constexpr char invalidCoreString[] = "core object is not valid";
static constexpr char invalidBrokerString[] = "broker object is not valid";

namespace helics {
CoreObject* getCoreObject(HelicsCore core, HelicsError* err) noexcept
{
    if ((err != nullptr) && (err->error_code != 0)) {
        return nullptr;
    }
    if (core == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidCoreString);
        return nullptr;
    }
    auto* coreObj = reinterpret_cast<helics::CoreObject*>(core);
    if (coreObj->valid == gCoreValidationIdentifier) {
        return coreObj;
    }
    assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidCoreString);
    return nullptr;
}

BrokerObject* getBrokerObject(HelicsBroker broker, HelicsError* err) noexcept
{
    if ((err != nullptr) && (err->error_code != 0)) {
        return nullptr;
    }
    if (broker == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidBrokerString);
        return nullptr;
    }
    auto* brokerObj = reinterpret_cast<helics::BrokerObject*>(broker);
    if (brokerObj->valid == gBrokerValidationIdentifier) {
        return brokerObj;
    }
    assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidBrokerString);
    return nullptr;
}

}  // namespace helics
helics::Core* getCore(HelicsCore core, HelicsError* err)
{
    auto* coreObj = helics::getCoreObject(core, err);
    if (coreObj == nullptr) {
        return nullptr;
    }
    return coreObj->coreptr.get();
}

std::shared_ptr<helics::Core> getCoreSharedPtr(HelicsCore core, HelicsError* err)
{
    auto* coreObj = helics::getCoreObject(core, err);
    if (coreObj == nullptr) {
        return nullptr;
    }
    return coreObj->coreptr;
}

helics::Broker* getBroker(HelicsBroker broker, HelicsError* err)
{
    auto* brokerObj = helics::getBrokerObject(broker, err);
    if (brokerObj == nullptr) {
        return nullptr;
    }
    return brokerObj->brokerptr.get();
}

HelicsCore helicsCreateCore(const char* type, const char* name, const char* initString, HelicsError* err)
{
    if ((err != nullptr) && (err->error_code != 0)) {
        return nullptr;
    }

    const helics::CoreType coretype = (type != nullptr) ? helics::core::coreTypeFromString(type) : helics::CoreType::DEFAULT;

    if (coretype == helics::CoreType::UNRECOGNIZED) {
        if (err != nullptr) {
            err->error_code = HELICS_ERROR_INVALID_ARGUMENT;
            err->message = getMasterHolder()->addErrorString(std::string("core type ") + type + " is not recognized");
        }
        return nullptr;
    }
    try {
        auto core = std::make_unique<helics::CoreObject>();
        core->valid = gCoreValidationIdentifier;
        auto nstring = AS_STRING_VIEW(name);
        if (nstring.empty()) {
            core->coreptr = helics::CoreFactory::create(coretype, AS_STRING_VIEW(initString));
        } else {
            core->coreptr = helics::CoreFactory::FindOrCreate(coretype, nstring, AS_STRING_VIEW(initString));
        }

        auto* retcore = reinterpret_cast<HelicsCore>(core.get());
        getMasterHolder()->addCore(std::move(core));
        return retcore;
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
}

HelicsCore helicsCreateCoreFromArgs(const char* type, const char* name, int argc, const char* const* argv, HelicsError* err)
{
    if ((err != nullptr) && (err->error_code != 0)) {
        return nullptr;
    }
    const helics::CoreType coretype = (type != nullptr) ? helics::core::coreTypeFromString(type) : helics::CoreType::DEFAULT;

    if (coretype == helics::CoreType::UNRECOGNIZED) {
        if (err != nullptr) {
            err->error_code = HELICS_ERROR_INVALID_ARGUMENT;
            err->message = getMasterHolder()->addErrorString(std::string("core type ") + type + " is not recognized");
        }
        return nullptr;
    }
    auto core = std::make_unique<helics::CoreObject>();
    try {
        core->valid = gCoreValidationIdentifier;
        std::vector<std::string> args;
        args.reserve(static_cast<size_t>(argc) - 1);
        for (int ii = argc - 1; ii > 0; ii--) {
            args.emplace_back(argv[ii]);
        }
        core->coreptr = helics::CoreFactory::FindOrCreate(coretype, AS_STRING_VIEW(name), args);

        auto* retcore = reinterpret_cast<HelicsCore>(core.get());
        getMasterHolder()->addCore(std::move(core));

        return retcore;
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
}

HelicsCore helicsCoreClone(HelicsCore core, HelicsError* err)
{
    auto* coreObj = helics::getCoreObject(core, err);
    if (coreObj == nullptr) {
        return nullptr;
    }
    try {
        auto coreClone = std::make_unique<helics::CoreObject>();
        coreClone->valid = gCoreValidationIdentifier;
        coreClone->coreptr = coreObj->coreptr;
        auto* retcore = reinterpret_cast<HelicsCore>(coreClone.get());
        getMasterHolder()->addCore(std::move(coreClone));

        return retcore;
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
}

HelicsBool helicsCoreIsValid(HelicsCore core)
{
    auto* coreObj = helics::getCoreObject(core, nullptr);
    if (coreObj == nullptr) {
        return HELICS_FALSE;
    }
    return (coreObj->coreptr) ? HELICS_TRUE : HELICS_FALSE;
}

HelicsBroker helicsCreateBroker(const char* type, const char* name, const char* initString, HelicsError* err)
{
    if ((err != nullptr) && (err->error_code != 0)) {
        return nullptr;
    }
    const helics::CoreType coretype = (type != nullptr) ? helics::core::coreTypeFromString(type) : helics::CoreType::DEFAULT;

    if (coretype == helics::CoreType::UNRECOGNIZED) {
        if (err != nullptr) {
            err->error_code = HELICS_ERROR_INVALID_ARGUMENT;
            err->message = getMasterHolder()->addErrorString(std::string("core type ") + type + " is not recognized");
        }
        return nullptr;
    }
    auto broker = std::make_unique<helics::BrokerObject>();
    broker->valid = gBrokerValidationIdentifier;
    try {
        broker->brokerptr = helics::BrokerFactory::create(coretype, AS_STRING_VIEW(name), AS_STRING_VIEW(initString));
        auto* retbroker = reinterpret_cast<HelicsBroker>(broker.get());
        getMasterHolder()->addBroker(std::move(broker));
        return retbroker;
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
}

HelicsBroker helicsCreateBrokerFromArgs(const char* type, const char* name, int argc, const char* const* argv, HelicsError* err)
{
    if ((err != nullptr) && (err->error_code != 0)) {
        return nullptr;
    }
    const helics::CoreType coretype = (type != nullptr) ? helics::core::coreTypeFromString(type) : helics::CoreType::DEFAULT;

    if (coretype == helics::CoreType::UNRECOGNIZED) {
        if (err != nullptr) {
            err->error_code = HELICS_ERROR_INVALID_ARGUMENT;
            err->message = getMasterHolder()->addErrorString(std::string("core type ") + type + " is not recognized");
        }
        return nullptr;
    }
    auto broker = std::make_unique<helics::BrokerObject>();
    broker->valid = gBrokerValidationIdentifier;
    try {
        std::vector<std::string> args;
        args.reserve(static_cast<size_t>(argc) - 1);
        for (int ii = argc - 1; ii > 0; ii--) {
            args.emplace_back(argv[ii]);
        }
        broker->brokerptr = helics::BrokerFactory::create(coretype, AS_STRING_VIEW(name), args);
        auto* retbroker = reinterpret_cast<HelicsBroker>(broker.get());
        getMasterHolder()->addBroker(std::move(broker));
        return retbroker;
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
}

HelicsBroker helicsBrokerClone(HelicsBroker broker, HelicsError* err)
{
    auto* brokerObj = helics::getBrokerObject(broker, err);
    if (brokerObj == nullptr) {
        return nullptr;
    }
    auto brokerClone = std::make_unique<helics::BrokerObject>();
    brokerClone->valid = gBrokerValidationIdentifier;
    brokerClone->brokerptr = brokerObj->brokerptr;
    auto* retbroker = reinterpret_cast<HelicsBroker>(brokerClone.get());
    getMasterHolder()->addBroker(std::move(brokerClone));
    return retbroker;
}

HelicsBool helicsBrokerIsValid(HelicsBroker broker)
{
    auto* brokerObj = helics::getBrokerObject(broker, nullptr);
    if (brokerObj == nullptr) {
        return HELICS_FALSE;
    }
    return (brokerObj->brokerptr) ? HELICS_TRUE : HELICS_FALSE;
}

HelicsBool helicsBrokerIsConnected(HelicsBroker broker)
{
    auto* brk = getBroker(broker, nullptr);
    if (brk == nullptr) {
        return HELICS_FALSE;
    }
    return (brk->isConnected()) ? HELICS_TRUE : HELICS_FALSE;
}

static constexpr char invalidDataLinkString[] = "Data link arguments cannot be null";

void helicsBrokerDataLink(HelicsBroker broker, const char* source, const char* target, HelicsError* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    if ((source == nullptr) || (target == nullptr)) {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidDataLinkString);
        return;
    }
    brk->dataLink(source, target);
}

void helicsCoreDataLink(HelicsCore core, const char* source, const char* target, HelicsError* err)
{
    auto* cppcore = getCore(core, err);
    if (cppcore == nullptr) {
        return;
    }
    if ((source == nullptr) || (target == nullptr)) {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidDataLinkString);
        return;
    }
    cppcore->dataLink(source, target);
}

static constexpr char invalidGlobalString[] = "Global name cannot be null";

void helicsBrokerSetGlobal(HelicsBroker broker, const char* valueName, const char* value, HelicsError* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    if (valueName == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidGlobalString);
        return;
    }
    brk->setGlobal(valueName, AS_STRING_VIEW(value));
}

static constexpr char invalidInterfaceName[] = "Interface name cannot be empty";
static constexpr char invalidAliasName[] = "Alias cannot be empty";

void helicsCoreAddAlias(HelicsCore core, const char* interfaceName, const char* alias, HelicsError* err)
{
    auto* cppcore = getCore(core, err);
    if (cppcore == nullptr) {
        return;
    }
    if (interfaceName == nullptr || interfaceName[0] == '\0') {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidInterfaceName);
        return;
    }
    if (alias == nullptr || alias[0] == '\0') {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidAliasName);
        return;
    }
    try {
        cppcore->addAlias(interfaceName, alias);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsBrokerAddAlias(HelicsBroker broker, const char* interfaceName, const char* alias, HelicsError* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    if (interfaceName == nullptr || interfaceName[0] == '\0') {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidInterfaceName);
        return;
    }
    if (alias == nullptr || alias[0] == '\0') {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidAliasName);
        return;
    }
    try {
        brk->addAlias(interfaceName, alias);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsBrokerSendCommand(HelicsBroker broker, const char* target, const char* command, HelicsError* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    brk->sendCommand(AS_STRING_VIEW(target), AS_STRING_VIEW(command), HELICS_SEQUENCING_MODE_FAST);
}

void helicsBrokerSendOrderedCommand(HelicsBroker broker, const char* target, const char* command, HelicsError* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    brk->sendCommand(AS_STRING_VIEW(target), AS_STRING_VIEW(command), HELICS_SEQUENCING_MODE_ORDERED);
}

void helicsBrokerSetLogFile(HelicsBroker broker, const char* logFileName, HelicsError* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    brk->setLogFile(AS_STRING_VIEW(logFileName));
}

void helicsBrokerSetTimeBarrier(HelicsBroker broker, HelicsTime barrierTime, HelicsError* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    brk->setTimeBarrier(barrierTime);
}

void helicsBrokerClearTimeBarrier(HelicsBroker broker)
{
    auto* brk = getBroker(broker, nullptr);
    if (brk == nullptr) {
        return;
    }
    brk->clearTimeBarrier();
}

void helicsBrokerGlobalError(HelicsBroker broker, int errorCode, const char* errorString, HelicsError* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    brk->globalError(errorCode, AS_STRING_VIEW(errorString));
}

void helicsCoreGlobalError(HelicsCore core, int errorCode, const char* errorString, HelicsError* err)
{
    auto* cppcore = getCore(core, err);
    if (cppcore == nullptr) {
        return;
    }
    cppcore->globalError(helics::gLocalCoreId, errorCode, AS_STRING_VIEW(errorString));
}

void helicsBrokerAddSourceFilterToEndpoint(HelicsBroker broker, const char* filter, const char* endpoint, HelicsError* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    if ((filter == nullptr) || (endpoint == nullptr)) {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidDataLinkString);
        return;
    }
    brk->addSourceFilterToEndpoint(filter, endpoint);
}

void helicsBrokerAddDestinationFilterToEndpoint(HelicsBroker broker, const char* filter, const char* endpoint, HelicsError* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    if ((filter == nullptr) || (endpoint == nullptr)) {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidDataLinkString);
        return;
    }
    brk->addDestinationFilterToEndpoint(filter, endpoint);
}

void helicsBrokerMakeConnections(HelicsBroker broker, const char* file, HelicsError* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    try {
        brk->makeConnections(AS_STRING(file));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

void helicsCoreAddSourceFilterToEndpoint(HelicsCore core, const char* filter, const char* endpoint, HelicsError* err)
{
    auto* cppcore = getCore(core, err);
    if (cppcore == nullptr) {
        return;
    }
    if ((filter == nullptr) || (endpoint == nullptr)) {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidDataLinkString);
        return;
    }
    cppcore->addSourceFilterToEndpoint(filter, endpoint);
}

void helicsCoreAddDestinationFilterToEndpoint(HelicsCore core, const char* filter, const char* endpoint, HelicsError* err)
{
    auto* cppcore = getCore(core, err);
    if (cppcore == nullptr) {
        return;
    }
    if ((filter == nullptr) || (endpoint == nullptr)) {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidDataLinkString);
        return;
    }
    cppcore->addDestinationFilterToEndpoint(filter, endpoint);
}

void helicsCoreMakeConnections(HelicsCore core, const char* file, HelicsError* err)
{
    auto* cppcore = getCore(core, err);
    if (cppcore == nullptr) {
        return;
    }
    try {
        cppcore->makeConnections(AS_STRING(file));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

HelicsBool helicsCoreIsConnected(HelicsCore core)
{
    auto* cppcore = getCore(core, nullptr);
    if (cppcore == nullptr) {
        return HELICS_FALSE;
    }
    return (cppcore->isConnected()) ? HELICS_TRUE : HELICS_FALSE;
}

void helicsCoreSetGlobal(HelicsCore core, const char* valueName, const char* value, HelicsError* err)
{
    auto* cppcore = getCore(core, err);
    if (cppcore == nullptr) {
        return;
    }
    if (valueName == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_ARGUMENT, invalidGlobalString);
        return;
    }
    try {
        cppcore->setGlobal(valueName, AS_STRING_VIEW(value));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsCoreSendCommand(HelicsCore core, const char* target, const char* command, HelicsError* err)
{
    auto* cppcore = getCore(core, err);
    if (cppcore == nullptr) {
        return;
    }
    try {
        cppcore->sendCommand(AS_STRING_VIEW(target), AS_STRING_VIEW(command), std::string_view{}, HELICS_SEQUENCING_MODE_FAST);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsCoreSendOrderedCommand(HelicsCore core, const char* target, const char* command, HelicsError* err)
{
    auto* cppcore = getCore(core, err);
    if (cppcore == nullptr) {
        return;
    }
    try {
        cppcore->sendCommand(AS_STRING_VIEW(target), AS_STRING_VIEW(command), std::string{}, HELICS_SEQUENCING_MODE_ORDERED);
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsCoreSetLogFile(HelicsCore core, const char* logFileName, HelicsError* err)
{
    auto* cppcore = getCore(core, err);
    if (cppcore == nullptr) {
        return;
    }
    try {
        cppcore->setLogFile(AS_STRING_VIEW(logFileName));
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

const char* helicsBrokerGetIdentifier(HelicsBroker broker)
{
    auto* brk = getBroker(broker, nullptr);
    if (brk == nullptr) {
        return nullstrPtr;
    }
    const auto& ident = brk->getIdentifier();
    return ident.c_str();
}

const char* helicsCoreGetIdentifier(HelicsCore core)
{
    auto* cppcore = getCore(core, nullptr);
    if (cppcore == nullptr) {
        return nullstrPtr;
    }

    const auto& ident = cppcore->getIdentifier();
    return ident.c_str();
}

const char* helicsBrokerGetAddress(HelicsBroker broker)
{
    auto* brk = getBroker(broker, nullptr);
    if (brk == nullptr) {
        return nullstrPtr;
    }

    const auto& add = brk->getAddress();
    return add.c_str();
}

const char* helicsCoreGetAddress(HelicsCore core)
{
    auto* cppcore = getCore(core, nullptr);
    if (cppcore == nullptr) {
        return nullstrPtr;
    }

    const auto& add = cppcore->getAddress();
    return add.c_str();
}

void helicsCoreSetReadyToInit(HelicsCore core, HelicsError* err)
{
    auto* cppcore = getCore(core, err);
    if (cppcore == nullptr) {
        return;
    }
    try {
        cppcore->setCoreReadyToInit();
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

HelicsBool helicsCoreConnect(HelicsCore core, HelicsError* err)
{
    auto* cppcore = getCore(core, err);
    if (cppcore == nullptr) {
        return HELICS_FALSE;
    }

    try {
        return (cppcore->connect()) ? HELICS_TRUE : HELICS_FALSE;
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return HELICS_FALSE;
    }
    // LCOV_EXCL_STOP
}

void helicsCoreDisconnect(HelicsCore core, HelicsError* err)
{
    auto* cppcore = getCore(core, err);
    if (cppcore == nullptr) {
        return;
    }

    try {
        cppcore->disconnect();
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

HelicsBool helicsBrokerWaitForDisconnect(HelicsBroker broker, int msToWait, HelicsError* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return HELICS_TRUE;
    }
    try {
        const bool res = brk->waitForDisconnect(std::chrono::milliseconds(msToWait));
        return res ? HELICS_TRUE : HELICS_FALSE;
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return HELICS_FALSE;
    }
    // LCOV_EXCL_STOP
}

HelicsBool helicsCoreWaitForDisconnect(HelicsCore core, int msToWait, HelicsError* err)
{
    auto* cppcore = getCore(core, err);
    if (cppcore == nullptr) {
        return HELICS_TRUE;
    }
    try {
        const bool res = cppcore->waitForDisconnect(std::chrono::milliseconds(msToWait));
        return res ? HELICS_TRUE : HELICS_FALSE;
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return HELICS_FALSE;
    }
    // LCOV_EXCL_STOP
}

void helicsBrokerDisconnect(HelicsBroker broker, HelicsError* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    try {
        brk->disconnect();
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsFederateDestroy(HelicsFederate fed)
{
    helicsFederateFinalize(fed, nullptr);
    helicsFederateFree(fed);
}

void helicsBrokerDestroy(HelicsBroker broker)
{
    helicsBrokerDisconnect(broker, nullptr);
    helicsBrokerFree(broker);
}

void helicsCoreDestroy(HelicsCore core)
{
    helicsCoreDisconnect(core, nullptr);
    helicsCoreFree(core);
}

void helicsCoreFree(HelicsCore core)
{
    auto* coreObj = helics::getCoreObject(core, nullptr);
    if (coreObj != nullptr) {
        coreObj->valid = 0;
        getMasterHolder()->clearCore(coreObj->index);
    }
    helics::CoreFactory::cleanUpCores();
}

void helicsBrokerFree(HelicsBroker broker)
{
    auto* brokerObj = helics::getBrokerObject(broker, nullptr);
    if (brokerObj != nullptr) {
        brokerObj->valid = 0;
        getMasterHolder()->clearBroker(brokerObj->index);
    }
    helics::BrokerFactory::cleanUpBrokers();
}

void helicsFederateFree(HelicsFederate fed)
{
    auto* fedObj = helics::getFedObject(fed, nullptr);
    if (fedObj != nullptr) {
        fedObj->valid = 0;
        getMasterHolder()->clearFed(fedObj->index);
    }

    helics::CoreFactory::cleanUpCores();
}

helics::FedObject::~FedObject()
{
    // we want to remove the values in the arrays before deleting the fedptr
    // and we want to do it inside this function to ensure it does so in a consistent manner
    messages.clear();
    inputs.clear();
    pubs.clear();
    epts.clear();
    filters.clear();
    fedptr = nullptr;
}

helics::CoreObject::~CoreObject()
{
    filters.clear();
    coreptr = nullptr;
}

void helicsCloseLibrary(void)
{
    clearAllObjects();
    helics::cleanupHelicsLibrary();
    helics::CoreFactory::cleanUpCores(std::chrono::milliseconds(2000));
    helics::BrokerFactory::cleanUpBrokers(std::chrono::milliseconds(2000));

    // helics::LoggerManager::closeLogger();
}

void helicsAbort(int errorCode, const char* message)
{
    auto holder = getMasterHolder();
    if (holder) {
        holder->abortAll(errorCode, message);
    }
}

namespace {
constexpr const char* invalidQueryString = "Query object is invalid";

constexpr int validQueryIdentifier = 0x2706'3885;

helics::QueryObject* getQueryObj(HelicsQuery query, HelicsError* err)
{
    if ((err != nullptr) && (err->error_code != 0)) {
        return nullptr;
    }
    if (query == nullptr) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidQueryString);
        return nullptr;
    }
    auto* queryPtr = reinterpret_cast<helics::QueryObject*>(query);
    if (queryPtr->valid != validQueryIdentifier) {
        assignError(err, HELICS_ERROR_INVALID_OBJECT, invalidQueryString);
        return nullptr;
    }
    return queryPtr;
}
}  // namespace
HelicsQuery helicsCreateQuery(const char* target, const char* query)
{
    auto* queryObj = new helics::QueryObject;
    queryObj->query = AS_STRING(query);
    queryObj->target = AS_STRING(target);
    queryObj->valid = validQueryIdentifier;
    return reinterpret_cast<void*>(queryObj);
}

constexpr auto invalidQueryObConst = "{{\n  \"error\":{{\n    \"code\":400,\n    \"message\":\"query object is not valid\"\n  }}\n}}";
constexpr auto invalidFedObConst = "{{\n  \"error\":{{\n    \"code\":404,\n    \"message\":\"federate object is not valid\"\n  }}\n}}";
constexpr auto queryErrorString = "{{\n  \"error\":{{\n    \"code\":500,\n    \"message\":\"Error on query Evaluation\"\n  }}\n}}";

const char* helicsQueryExecute(HelicsQuery query, HelicsFederate fed, HelicsError* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return invalidFedObConst;
    }

    auto* queryObj = getQueryObj(query, err);
    if (queryObj == nullptr) {
        return invalidQueryObConst;
    }
    try {
        if (queryObj->target.empty()) {
            queryObj->response = fedObj->query(queryObj->query, queryObj->mode);
        } else {
            queryObj->response = fedObj->query(queryObj->target, queryObj->query, queryObj->mode);
        }

        return queryObj->response.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    return queryErrorString;
    // LCOV_EXCL_START
}

const char* helicsQueryCoreExecute(HelicsQuery query, HelicsCore core, HelicsError* err)
{
    auto* coreObj = getCore(core, err);
    if (coreObj == nullptr) {
        constexpr auto invalidCoreObConst = "{{\n  \"error\":{{\n    \"code\":404,\n    \"message\":\"Core object is not valid\"\n  }}\n}}";
        return invalidCoreObConst;
    }
    auto* queryObj = getQueryObj(query, err);
    if (queryObj == nullptr) {
        return invalidQueryObConst;
    }
    try {
        queryObj->response = coreObj->query(queryObj->target, queryObj->query, queryObj->mode);
        return queryObj->response.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    return queryErrorString;
    // LCOV_EXCL_START
}

const char* helicsQueryBrokerExecute(HelicsQuery query, HelicsBroker broker, HelicsError* err)
{
    auto* brokerObj = getBroker(broker, err);
    if (brokerObj == nullptr) {
        constexpr auto invalidBrokerObConst =
            "{{\n  \"error\":{{\n    \"code\":404,\n    \"message\":\"Broker object is not valid\"\n  }}\n}}";
        return invalidBrokerObConst;
    }

    auto* queryObj = getQueryObj(query, err);
    if (queryObj == nullptr) {
        return invalidQueryObConst;
    }
    try {
        queryObj->response = brokerObj->query(queryObj->target, queryObj->query, queryObj->mode);
        return queryObj->response.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    return queryErrorString;
    // LCOV_EXCL_STOP
}

void helicsQueryExecuteAsync(HelicsQuery query, HelicsFederate fed, HelicsError* err)
{
    auto* queryObj = getQueryObj(query, err);
    if (queryObj == nullptr) {
        return;
    }
    auto fedObj = getFedSharedPtr(fed, err);
    if (fedObj == nullptr) {
        queryObj->response = invalidFedObConst;
        return;
    }
    try {
        if (queryObj->target.empty()) {
            queryObj->asyncIndexCode = fedObj->queryAsync(queryObj->query);
        } else {
            queryObj->asyncIndexCode = fedObj->queryAsync(queryObj->target, queryObj->query);
        }
        queryObj->activeAsync = true;
        queryObj->activeFed = fedObj;
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

const char* helicsQueryExecuteComplete(HelicsQuery query, HelicsError* err)
{
    auto* queryObj = getQueryObj(query, err);
    if (queryObj == nullptr) {
        return invalidQueryObConst;
    }
    try {
        if (queryObj->asyncIndexCode.isValid()) {
            queryObj->response = queryObj->activeFed->queryComplete(queryObj->asyncIndexCode);
        }
        queryObj->activeAsync = false;
        queryObj->activeFed = nullptr;
        queryObj->asyncIndexCode = helics::QueryId();
        return queryObj->response.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    return queryErrorString;
    // LCOV_EXCL_STOP
}

HelicsBool helicsQueryIsCompleted(HelicsQuery query)
{
    auto* queryObj = getQueryObj(query, nullptr);
    if (queryObj == nullptr) {
        return HELICS_FALSE;
    }
    try {
        if (queryObj->asyncIndexCode.isValid()) {
            auto res = queryObj->activeFed->isQueryCompleted(queryObj->asyncIndexCode);
            return (res) ? HELICS_TRUE : HELICS_FALSE;
        }
        return HELICS_FALSE;
    }
    // LCOV_EXCL_START
    catch (...) {
        return HELICS_FALSE;
    }
    // LCOV_EXCL_STOP
}

void helicsQuerySetTarget(HelicsQuery query, const char* target, HelicsError* err)
{
    auto* queryObj = getQueryObj(query, err);
    if (queryObj == nullptr) {
        return;
    }
    queryObj->target = AS_STRING(target);
}

void helicsQuerySetQueryString(HelicsQuery query, const char* queryString, HelicsError* err)
{
    auto* queryObj = getQueryObj(query, err);
    if (queryObj == nullptr) {
        return;
    }
    queryObj->query = AS_STRING(queryString);
}

void helicsQuerySetOrdering(HelicsQuery query, int32_t mode, HelicsError* err)
{
    auto* queryObj = getQueryObj(query, err);
    if (queryObj == nullptr) {
        return;
    }
    queryObj->mode = (mode == 0) ? HELICS_SEQUENCING_MODE_FAST : HELICS_SEQUENCING_MODE_ORDERED;
}

void helicsQueryFree(HelicsQuery query)
{
    auto* queryObj = getQueryObj(query, nullptr);
    if (queryObj == nullptr) {
        //  fprintf(stderr, "invalid query object\n");
        return;
    }
    queryObj->valid = 0;
    delete queryObj;
}

void helicsCleanupLibrary(void)
{
    helics::cleanupHelicsLibrary();
    //  helics::LoggerManager::closeLogger();

    //  ZmqContextManager::closeContext();
}

MasterObjectHolder::MasterObjectHolder() noexcept {}

MasterObjectHolder::~MasterObjectHolder()
{
#ifdef HELICS_ENABLE_ZMQ_CORE
    if (ZmqContextManager::setContextToLeakOnDelete()) {
        ZmqContextManager::closeContext();  // LCOV_EXCL_LINE
    }
#endif
    deleteAll();
    // std::cout << "end of master Object Holder destructor" << std::endl;
}
int MasterObjectHolder::addBroker(std::unique_ptr<helics::BrokerObject> broker)
{
    auto handle = brokers.lock();
    auto index = static_cast<int>(handle->size());
    broker->index = index;
    handle->push_back(std::move(broker));
    return index;
}

int MasterObjectHolder::addCore(std::unique_ptr<helics::CoreObject> core)
{
    auto handle = cores.lock();
    auto index = static_cast<int>(handle->size());
    core->index = index;
    handle->push_back(std::move(core));
    return index;
}

int MasterObjectHolder::addFed(std::unique_ptr<helics::FedObject> fed)
{
    auto handle = feds.lock();
    auto index = static_cast<int>(handle->size());
    fed->index = index;
    handle->push_back(std::move(fed));
    return index;
}

int MasterObjectHolder::addApp(std::unique_ptr<helics::AppObject> app)
{
    auto handle = apps.lock();
    auto index = static_cast<int>(handle->size());
    app->index = index;
    handle->push_back(std::move(app));
    return index;
}

helics::FedObject* MasterObjectHolder::findFed(std::string_view fedName)
{
    auto handle = feds.lock();
    for (auto& fed : (*handle)) {
        if ((fed) && (fed->fedptr)) {
            if (fed->fedptr->getName() == fedName) {
                return fed.get();
            }
        }
    }
    return nullptr;
}

helics::FedObject* MasterObjectHolder::findFed(std::string_view fedName, int validationCode)
{
    auto handle = feds.lock();
    for (auto& fed : (*handle)) {
        if ((fed) && (fed->fedptr)) {
            if (fed->valid == validationCode && fed->fedptr->getName() == fedName) {
                return fed.get();
            }
        }
    }
    return nullptr;
}

/** remove a federate object*/
bool MasterObjectHolder::removeFed(std::string_view fedName, int validationCode)
{
    auto handle = feds.lock();
    bool found{false};
    for (auto& fed : (*handle)) {
        if ((fed) && (fed->fedptr)) {
            if (fed->fedptr->getName() == fedName && fed->valid == validationCode) {
                fed->valid = 0;
                fed->fedptr.reset();
                fed.reset();
                found = true;
            }
        }
    }
    return found;
}

void MasterObjectHolder::clearBroker(int index)
{
    auto broker = brokers.lock();
    if ((index < static_cast<int>(broker->size())) && (index >= 0)) {
        (*broker)[index]->valid = 0;
        (*broker)[index] = nullptr;
        if (broker->size() > 10) {
            if (std::none_of(broker->begin(), broker->end(), [](const auto& brk) { return static_cast<bool>(brk); })) {
                broker->clear();
            }
        }
    }
}

void MasterObjectHolder::clearCore(int index)
{
    auto coreList = cores.lock();
    if ((index < static_cast<int>(coreList->size())) && (index >= 0)) {
        (*coreList)[index]->valid = 0;
        (*coreList)[index] = nullptr;
        if (coreList->size() > 10) {
            if (std::none_of(coreList->begin(), coreList->end(), [](const auto& core) { return static_cast<bool>(core); })) {
                coreList->clear();
            }
        }
    }
}

void MasterObjectHolder::clearFed(int index)
{
    auto fedList = feds.lock();
    if ((index < static_cast<int>(fedList->size())) && (index >= 0)) {
        (*fedList)[index]->valid = 0;
        (*fedList)[index] = nullptr;
        if (fedList->size() > 10) {
            if (std::none_of(fedList->begin(), fedList->end(), [](const auto& federate) { return static_cast<bool>(federate); })) {
                fedList->clear();
            }
        }
    }
}

void MasterObjectHolder::clearApp(int index)
{
    auto appList = apps.lock();
    if ((index < static_cast<int>(appList->size())) && (index >= 0)) {
        (*appList)[index]->valid = 0;
        (*appList)[index] = nullptr;
        if (appList->size() > 10) {
            if (std::none_of(appList->begin(), appList->end(), [](const auto& app) { return static_cast<bool>(app); })) {
                appList->clear();
            }
        }
    }
}

void MasterObjectHolder::abortAll(int code, std::string_view error)
{
    {
        auto fedHandle = feds.lock();
        for (auto& fed : fedHandle) {
            if ((fed) && (fed->fedptr)) {
                fed->fedptr->globalError(code, fed->fedptr->getName() + " sent abort message: '" + std::string(error) + "'");
            }
        }
    }
    helics::BrokerFactory::abortAllBrokers(code, error);
    helics::CoreFactory::abortAllCores(code, error);
}

void MasterObjectHolder::deleteAll()
{
    if (tripDetect.isTripped()) {
        return;
    }
    // brackets are for the scopes on the lock handles
    {
        auto fedHandle = feds.lock();
        for (auto& fed : fedHandle) {
            if ((fed) && (fed->fedptr)) {
                fed->valid = 0;
                fed->fedptr->finalize();
            }
        }
        fedHandle->clear();
    }
    {
        auto appHandle = apps.lock();
        for (auto& app : appHandle) {
            if ((app) && (app->app)) {
                helicsAppFinalize(reinterpret_cast<HelicsApp>(app.get()), nullptr);
                app->valid = 0;
            }
        }
        appHandle->clear();
    }
    {
        auto coreHandle = cores.lock();
        for (auto& core : coreHandle) {
            if ((core) && (core->coreptr)) {
                core->valid = 0;
                core->coreptr->disconnect();
            }
        }
        coreHandle->clear();
    }
    {
        auto brokerHandle = brokers.lock();
        for (auto& brk : brokerHandle) {
            if ((brk) && (brk->brokerptr)) {
                brk->valid = 0;
                brk->brokerptr->disconnect();
            }
        }
        brokerHandle->clear();
    }
    errorStrings.lock()->clear();
}

const char* MasterObjectHolder::addErrorString(std::string_view newError)
{
    auto estrings = errorStrings.lock();
    estrings->emplace_back(newError);
    auto& estring = estrings->back();
    return estring.c_str();
}

std::shared_ptr<MasterObjectHolder> getMasterHolder()
{
    static auto instance = std::make_shared<MasterObjectHolder>();
    static const gmlc::concurrency::TripWireTrigger tripTriggerholder;
    return instance;
}

gmlc::concurrency::TripWireTrigger tripTrigger;

void clearAllObjects()
{
    auto holder = getMasterHolder();
    if (holder) {
        holder->deleteAll();
    }
}
