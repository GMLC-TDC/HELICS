/*
Copyright (c) 2017-2021,
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
#include "helics.h"
#include "helics/helics-config.h"
#include "internal/api_objects.h"

#include <atomic>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <vector>
#ifdef ENABLE_ZMQ_CORE
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

static constexpr const char* nullstrPtr = "";

const std::string emptyStr;

helics_error helicsErrorInitialize(void)
{
    helics_error err;
    err.error_code = 0;
    err.message = nullstrPtr;
    return err;
}

/** clear an error object*/
void helicsErrorClear(helics_error* err)
{
    if (err != nullptr) {
        err->error_code = 0;
        err->message = nullstrPtr;
    }
}

#include <csignal>
static void signalHandler(int /*signum*/)
{
    helicsAbort(helics_error_user_abort, "user abort");
    // add a sleep to give the abort a chance to propagate to other federates
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << std::endl;
    exit(helics_error_user_abort);
}

void helicsLoadSignalHandler()
{
    signal(SIGINT, signalHandler);
}

void helicsClearSignalHandler()
{
    signal(SIGINT, SIG_DFL);
}

static helics_bool (*keyHandler)(int) = nullptr;

static void signalHandlerCallback(int signum)
{
    helics_bool runDefaultSignalHandler{helics_true};
    if (keyHandler != nullptr) {
        runDefaultSignalHandler = keyHandler(signum);
    }
    if (runDefaultSignalHandler != helics_false) {
        signalHandler(signum);
    }
}

void helicsLoadSignalHandlerCallback(helics_bool (*handler)(int))
{
    keyHandler = handler;
    if (handler != nullptr) {
        signal(SIGINT, signalHandlerCallback);
    } else {
        helicsLoadSignalHandler();
    }
}

helics_bool helicsIsCoreTypeAvailable(const char* type)
{
    if (type == nullptr) {
        return helics_false;
    }
    auto coreType = helics::core::coreTypeFromString(type);
    return (helics::core::isCoreTypeAvailable(coreType)) ? helics_true : helics_false;
}

// typedef enum {

//    helics_ok = 0, /*!< the function executed successfully */
//    helics_error_invalid_object, /*!< indicator that the object used was not a valid object */
//    helics_error_invalid_argument, /*!< the parameter passed was invalid and unable to be used*/
//    helics_discard, /*!< the input was discarded and not used for some reason */
//    helics_terminated, /*!< the federate has terminated and the call cannot be completed*/
//    helics_warning, /*!< the function issued a warning of some kind */
//    helics_invalid_state_transition, /*!< error issued when an invalid state transition occurred */
//    helics_invalid_function_call, /*!< the call made was invalid in the present state of the calling object*/
//    helics_error /*!< the function produced an error */
//} void;

/** this function is based on the lippencott function template
http://cppsecrets.blogspot.com/2013/12/using-lippincott-function-for.html
*/
static constexpr char unknown_err_string[] = "unknown error";

void helicsErrorHandler(helics_error* err) noexcept
{
    if (err == nullptr) {
        return;
    }
    try {
        try {
            // this is intended to be a single '='
            if (std::exception_ptr eptr = std::current_exception()) {
                std::rethrow_exception(eptr);
            } else {
                // LCOV_EXCL_START
                err->error_code = helics_error_external_type;
                err->message = unknown_err_string;
                // LCOV_EXCL_STOP
            }
        }
        catch (const helics::InvalidFunctionCall& ifc) {
            err->error_code = helics_error_invalid_function_call;
            err->message = getMasterHolder()->addErrorString(ifc.what());
        }
        catch (const helics::InvalidParameter& ip) {
            err->error_code = helics_error_invalid_argument;
            err->message = getMasterHolder()->addErrorString(ip.what());
        }
        catch (const helics::RegistrationFailure& rf) {
            err->error_code = helics_error_registration_failure;
            err->message = getMasterHolder()->addErrorString(rf.what());
        }
        catch (const helics::ConnectionFailure& cf) {
            err->error_code = helics_error_connection_failure;
            err->message = getMasterHolder()->addErrorString(cf.what());
        }
        // LCOV_EXCL_START
        catch (const helics::InvalidIdentifier& iid) {
            err->error_code = helics_error_invalid_object;
            err->message = getMasterHolder()->addErrorString(iid.what());
        }
        catch (const helics::HelicsSystemFailure& ht) {
            err->error_code = helics_error_system_failure;
            err->message = getMasterHolder()->addErrorString(ht.what());
        }
        // LCOV_EXCL_STOP
        catch (const helics::HelicsException& he) {
            err->error_code = helics_error_other;
            err->message = getMasterHolder()->addErrorString(he.what());
        }
        catch (const std::exception& exc) {
            err->error_code = helics_error_external_type;
            err->message = getMasterHolder()->addErrorString(exc.what());
        }
        // LCOV_EXCL_START
        catch (...) {
            err->error_code = helics_error_external_type;
            err->message = unknown_err_string;
        }
        // LCOV_EXCL_STOP
    }
    // LCOV_EXCL_START
    catch (...) {
        err->error_code = helics_error_external_type;
        err->message = unknown_err_string;
    }
    // LCOV_EXCL_STOP
}

static constexpr char invalidCoreString[] = "core object is not valid";
static constexpr char invalidBrokerString[] = "broker object is not valid";

namespace helics {
CoreObject* getCoreObject(helics_core core, helics_error* err) noexcept
{
    if ((err != nullptr) && (err->error_code != 0)) {
        return nullptr;
    }
    if (core == nullptr) {
        assignError(err, helics_error_invalid_object, invalidCoreString);
        return nullptr;
    }
    auto* coreObj = reinterpret_cast<helics::CoreObject*>(core);
    if (coreObj->valid == coreValidationIdentifier) {
        return coreObj;
    }
    assignError(err, helics_error_invalid_object, invalidCoreString);
    return nullptr;
}

BrokerObject* getBrokerObject(helics_broker broker, helics_error* err) noexcept
{
    if ((err != nullptr) && (err->error_code != 0)) {
        return nullptr;
    }
    if (broker == nullptr) {
        assignError(err, helics_error_invalid_object, invalidBrokerString);
        return nullptr;
    }
    auto* brokerObj = reinterpret_cast<helics::BrokerObject*>(broker);
    if (brokerObj->valid == brokerValidationIdentifier) {
        return brokerObj;
    }
    assignError(err, helics_error_invalid_object, invalidBrokerString);
    return nullptr;
}

}  // namespace helics
helics::Core* getCore(helics_core core, helics_error* err)
{
    auto* coreObj = helics::getCoreObject(core, err);
    if (coreObj == nullptr) {
        return nullptr;
    }
    return coreObj->coreptr.get();
}

std::shared_ptr<helics::Core> getCoreSharedPtr(helics_core core, helics_error* err)
{
    auto* coreObj = helics::getCoreObject(core, err);
    if (coreObj == nullptr) {
        return nullptr;
    }
    return coreObj->coreptr;
}

helics::Broker* getBroker(helics_broker broker, helics_error* err)
{
    auto* brokerObj = helics::getBrokerObject(broker, err);
    if (brokerObj == nullptr) {
        return nullptr;
    }
    return brokerObj->brokerptr.get();
}

helics_core helicsCreateCore(const char* type, const char* name, const char* initString, helics_error* err)
{
    if ((err != nullptr) && (err->error_code != 0)) {
        return nullptr;
    }

    helics::core_type ct = (type != nullptr) ? helics::core::coreTypeFromString(type) : helics::core_type::DEFAULT;

    if (ct == helics::core_type::UNRECOGNIZED) {
        if (err != nullptr) {
            err->error_code = helics_error_invalid_argument;
            err->message = getMasterHolder()->addErrorString(std::string("core type ") + type + " is not recognized");
        }
        return nullptr;
    }
    try {
        auto core = std::make_unique<helics::CoreObject>();
        core->valid = coreValidationIdentifier;
        auto nstring = AS_STRING(name);
        if (nstring.empty()) {
            core->coreptr = helics::CoreFactory::create(ct, AS_STRING(initString));
        } else {
            core->coreptr = helics::CoreFactory::FindOrCreate(ct, nstring, AS_STRING(initString));
        }

        auto* retcore = reinterpret_cast<helics_core>(core.get());
        getMasterHolder()->addCore(std::move(core));
        return retcore;
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
}

helics_core helicsCreateCoreFromArgs(const char* type, const char* name, int argc, const char* const* argv, helics_error* err)
{
    if ((err != nullptr) && (err->error_code != 0)) {
        return nullptr;
    }
    helics::core_type ct = (type != nullptr) ? helics::core::coreTypeFromString(type) : helics::core_type::DEFAULT;

    if (ct == helics::core_type::UNRECOGNIZED) {
        if (err != nullptr) {
            err->error_code = helics_error_invalid_argument;
            err->message = getMasterHolder()->addErrorString(std::string("core type ") + type + " is not recognized");
        }
        return nullptr;
    }
    auto core = std::make_unique<helics::CoreObject>();
    try {
        core->valid = coreValidationIdentifier;
        std::vector<std::string> args;
        args.reserve(static_cast<size_t>(argc) - 1);
        for (int ii = argc - 1; ii > 0; ii--) {
            args.emplace_back(argv[ii]);
        }
        core->coreptr = helics::CoreFactory::FindOrCreate(ct, AS_STRING(name), args);

        auto* retcore = reinterpret_cast<helics_core>(core.get());
        getMasterHolder()->addCore(std::move(core));

        return retcore;
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
}

helics_core helicsCoreClone(helics_core core, helics_error* err)
{
    auto* coreObj = helics::getCoreObject(core, err);
    if (coreObj == nullptr) {
        return nullptr;
    }
    auto coreClone = std::make_unique<helics::CoreObject>();
    coreClone->valid = coreValidationIdentifier;
    coreClone->coreptr = coreObj->coreptr;
    auto* retcore = reinterpret_cast<helics_core>(coreClone.get());
    getMasterHolder()->addCore(std::move(coreClone));

    return retcore;
}

helics_bool helicsCoreIsValid(helics_core core)
{
    auto* coreObj = helics::getCoreObject(core, nullptr);
    if (coreObj == nullptr) {
        return helics_false;
    }
    return (coreObj->coreptr) ? helics_true : helics_false;
}

helics_federate helicsGetFederateByName(const char* fedName, helics_error* err)
{
    if ((err != nullptr) && (err->error_code != 0)) {
        return nullptr;
    }
    if (fedName == nullptr) {
        if (err != nullptr) {
            err->error_code = helics_error_invalid_argument;
            err->message = getMasterHolder()->addErrorString("fedName is empty");
        }
        return nullptr;
    }
    auto mob = getMasterHolder();
    auto* fed = mob->findFed(fedName);
    if (fed == nullptr) {
        if (err != nullptr) {
            err->error_code = helics_error_invalid_argument;
            err->message = getMasterHolder()->addErrorString(std::string(fedName) + " is not an active federate identifier");
        }
        return nullptr;
    }
    return helicsFederateClone(reinterpret_cast<helics_federate>(fed), err);
}

helics_broker helicsCreateBroker(const char* type, const char* name, const char* initString, helics_error* err)
{
    if ((err != nullptr) && (err->error_code != 0)) {
        return nullptr;
    }
    helics::core_type ct = (type != nullptr) ? helics::core::coreTypeFromString(type) : helics::core_type::DEFAULT;

    if (ct == helics::core_type::UNRECOGNIZED) {
        if (err != nullptr) {
            err->error_code = helics_error_invalid_argument;
            err->message = getMasterHolder()->addErrorString(std::string("core type ") + type + " is not recognized");
        }
        return nullptr;
    }
    auto broker = std::make_unique<helics::BrokerObject>();
    broker->valid = brokerValidationIdentifier;
    try {
        broker->brokerptr = helics::BrokerFactory::create(ct, AS_STRING(name), AS_STRING(initString));
        auto* retbroker = reinterpret_cast<helics_broker>(broker.get());
        getMasterHolder()->addBroker(std::move(broker));
        return retbroker;
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
}

helics_broker helicsCreateBrokerFromArgs(const char* type, const char* name, int argc, const char* const* argv, helics_error* err)
{
    if ((err != nullptr) && (err->error_code != 0)) {
        return nullptr;
    }
    helics::core_type ct = (type != nullptr) ? helics::core::coreTypeFromString(type) : helics::core_type::DEFAULT;

    if (ct == helics::core_type::UNRECOGNIZED) {
        if (err != nullptr) {
            err->error_code = helics_error_invalid_argument;
            err->message = getMasterHolder()->addErrorString(std::string("core type ") + type + " is not recognized");
        }
        return nullptr;
    }
    auto broker = std::make_unique<helics::BrokerObject>();
    broker->valid = brokerValidationIdentifier;
    try {
        std::vector<std::string> args;
        args.reserve(static_cast<size_t>(argc) - 1);
        for (int ii = argc - 1; ii > 0; ii--) {
            args.emplace_back(argv[ii]);
        }
        broker->brokerptr = helics::BrokerFactory::create(ct, AS_STRING(name), args);
        auto* retbroker = reinterpret_cast<helics_broker>(broker.get());
        getMasterHolder()->addBroker(std::move(broker));
        return retbroker;
    }
    catch (...) {
        helicsErrorHandler(err);
        return nullptr;
    }
}

helics_broker helicsBrokerClone(helics_broker broker, helics_error* err)
{
    auto* brokerObj = helics::getBrokerObject(broker, err);
    if (brokerObj == nullptr) {
        return nullptr;
    }
    auto brokerClone = std::make_unique<helics::BrokerObject>();
    brokerClone->valid = brokerValidationIdentifier;
    brokerClone->brokerptr = brokerObj->brokerptr;
    auto* retbroker = reinterpret_cast<helics_broker>(brokerClone.get());
    getMasterHolder()->addBroker(std::move(brokerClone));
    return retbroker;
}

helics_bool helicsBrokerIsValid(helics_broker broker)
{
    auto* brokerObj = helics::getBrokerObject(broker, nullptr);
    if (brokerObj == nullptr) {
        return helics_false;
    }
    return (brokerObj->brokerptr) ? helics_true : helics_false;
}

helics_bool helicsBrokerIsConnected(helics_broker broker)
{
    auto* brk = getBroker(broker, nullptr);
    if (brk == nullptr) {
        return helics_false;
    }
    return (brk->isConnected()) ? helics_true : helics_false;
}

static constexpr char invalidDataLinkString[] = "Data link arguments cannot be null";

void helicsBrokerDataLink(helics_broker broker, const char* source, const char* target, helics_error* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    if ((source == nullptr) || (target == nullptr)) {
        assignError(err, helics_error_invalid_argument, invalidDataLinkString);
        return;
    }
    brk->dataLink(source, target);
}

void helicsCoreDataLink(helics_core core, const char* source, const char* target, helics_error* err)
{
    auto* cr = getCore(core, err);
    if (cr == nullptr) {
        return;
    }
    if ((source == nullptr) || (target == nullptr)) {
        assignError(err, helics_error_invalid_argument, invalidDataLinkString);
        return;
    }
    cr->dataLink(source, target);
}

static constexpr char invalidGlobalString[] = "Global name cannot be null";

void helicsBrokerSetGlobal(helics_broker broker, const char* valueName, const char* value, helics_error* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    if (valueName == nullptr) {
        assignError(err, helics_error_invalid_argument, invalidGlobalString);
        return;
    }
    brk->setGlobal(valueName, AS_STRING(value));
}

void helicsBrokerSetLogFile(helics_broker broker, const char* logFileName, helics_error* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    brk->setLogFile(AS_STRING(logFileName));
}

void helicsBrokerSetTimeBarrier(helics_broker broker, helics_time barrierTime, helics_error* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    brk->setTimeBarrier(barrierTime);
}

void helicsBrokerClearTimeBarrier(helics_broker broker)
{
    auto* brk = getBroker(broker, nullptr);
    if (brk == nullptr) {
        return;
    }
    brk->clearTimeBarrier();
}

void helicsBrokerGlobalError(helics_broker broker, int errorCode, const char* errorString, helics_error* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    brk->globalError(errorCode, AS_STRING(errorString));
}

void helicsCoreGlobalError(helics_core core, int errorCode, const char* errorString, helics_error* err)
{
    auto* cr = getCore(core, err);
    if (cr == nullptr) {
        return;
    }
    cr->globalError(helics::local_core_id, errorCode, AS_STRING(errorString));
}

void helicsBrokerAddSourceFilterToEndpoint(helics_broker broker, const char* filter, const char* endpoint, helics_error* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    if ((filter == nullptr) || (endpoint == nullptr)) {
        assignError(err, helics_error_invalid_argument, invalidDataLinkString);
        return;
    }
    brk->addSourceFilterToEndpoint(filter, endpoint);
}

void helicsBrokerAddDestinationFilterToEndpoint(helics_broker broker, const char* filter, const char* endpoint, helics_error* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return;
    }
    if ((filter == nullptr) || (endpoint == nullptr)) {
        assignError(err, helics_error_invalid_argument, invalidDataLinkString);
        return;
    }
    brk->addDestinationFilterToEndpoint(filter, endpoint);
}

void helicsBrokerMakeConnections(helics_broker broker, const char* file, helics_error* err)
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

void helicsCoreAddSourceFilterToEndpoint(helics_core core, const char* filter, const char* endpoint, helics_error* err)
{
    auto* cr = getCore(core, err);
    if (cr == nullptr) {
        return;
    }
    if ((filter == nullptr) || (endpoint == nullptr)) {
        assignError(err, helics_error_invalid_argument, invalidDataLinkString);
        return;
    }
    cr->addSourceFilterToEndpoint(filter, endpoint);
}

void helicsCoreAddDestinationFilterToEndpoint(helics_core core, const char* filter, const char* endpoint, helics_error* err)
{
    auto* cr = getCore(core, err);
    if (cr == nullptr) {
        return;
    }
    if ((filter == nullptr) || (endpoint == nullptr)) {
        assignError(err, helics_error_invalid_argument, invalidDataLinkString);
        return;
    }
    cr->addDestinationFilterToEndpoint(filter, endpoint);
}

void helicsCoreMakeConnections(helics_core core, const char* file, helics_error* err)
{
    auto* cr = getCore(core, err);
    if (cr == nullptr) {
        return;
    }
    try {
        cr->makeConnections(AS_STRING(file));
    }
    catch (...) {
        helicsErrorHandler(err);
    }
}

helics_bool helicsCoreIsConnected(helics_core core)
{
    auto* cr = getCore(core, nullptr);
    if (cr == nullptr) {
        return helics_false;
    }
    return (cr->isConnected()) ? helics_true : helics_false;
}

void helicsCoreSetGlobal(helics_core core, const char* valueName, const char* value, helics_error* err)
{
    auto* cr = getCore(core, err);
    if (cr == nullptr) {
        return;
    }
    if (valueName == nullptr) {
        assignError(err, helics_error_invalid_argument, invalidGlobalString);
        return;
    }
    cr->setGlobal(valueName, AS_STRING(value));
}

void helicsCoreSetLogFile(helics_core core, const char* logFileName, helics_error* err)
{
    auto* cr = getCore(core, err);
    if (cr == nullptr) {
        return;
    }
    cr->setLogFile(AS_STRING(logFileName));
}

const char* helicsBrokerGetIdentifier(helics_broker broker)
{
    auto* brk = getBroker(broker, nullptr);
    if (brk == nullptr) {
        return nullstrPtr;
    }
    const auto& ident = brk->getIdentifier();
    return ident.c_str();
}

const char* helicsCoreGetIdentifier(helics_core core)
{
    auto* cr = getCore(core, nullptr);
    if (cr == nullptr) {
        return nullstrPtr;
    }

    const auto& ident = cr->getIdentifier();
    return ident.c_str();
}

const char* helicsBrokerGetAddress(helics_broker broker)
{
    auto* brk = getBroker(broker, nullptr);
    if (brk == nullptr) {
        return nullstrPtr;
    }

    const auto& add = brk->getAddress();
    return add.c_str();
}

const char* helicsCoreGetAddress(helics_core core)
{
    auto* cr = getCore(core, nullptr);
    if (cr == nullptr) {
        return nullstrPtr;
    }

    const auto& add = cr->getAddress();
    return add.c_str();
}

void helicsCoreSetReadyToInit(helics_core core, helics_error* err)
{
    auto* cr = getCore(core, err);
    if (cr == nullptr) {
        return;
    }
    cr->setCoreReadyToInit();
}

helics_bool helicsCoreConnect(helics_core core, helics_error* err)
{
    auto* cr = getCore(core, err);
    if (cr == nullptr) {
        return helics_false;
    }

    try {
        return (cr->connect()) ? helics_true : helics_false;
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
        return helics_false;
    }
    // LCOV_EXCL_STOP
}

void helicsCoreDisconnect(helics_core core, helics_error* err)
{
    auto* cr = getCore(core, err);
    if (cr == nullptr) {
        return;
    }

    try {
        cr->disconnect();
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

helics_bool helicsBrokerWaitForDisconnect(helics_broker broker, int msToWait, helics_error* err)
{
    auto* brk = getBroker(broker, err);
    if (brk == nullptr) {
        return helics_true;
    }
    bool res = brk->waitForDisconnect(std::chrono::milliseconds(msToWait));
    return res ? helics_true : helics_false;
}

helics_bool helicsCoreWaitForDisconnect(helics_core core, int msToWait, helics_error* err)
{
    auto* cr = getCore(core, err);
    if (cr == nullptr) {
        return helics_true;
    }
    bool res = cr->waitForDisconnect(std::chrono::milliseconds(msToWait));
    return res ? helics_true : helics_false;
}

void helicsBrokerDisconnect(helics_broker broker, helics_error* err)
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
        return helicsErrorHandler(err);
    }
    // LCOV_EXCL_STOP
}

void helicsFederateDestroy(helics_federate fed)
{
    helicsFederateFinalize(fed, nullptr);
    helicsFederateFree(fed);
}

void helicsBrokerDestroy(helics_broker broker)
{
    helicsBrokerDisconnect(broker, nullptr);
    helicsBrokerFree(broker);
}

void helicsCoreDestroy(helics_core core)
{
    helicsCoreDisconnect(core, nullptr);
    helicsCoreFree(core);
}

void helicsCoreFree(helics_core core)
{
    auto* coreObj = helics::getCoreObject(core, nullptr);
    if (coreObj != nullptr) {
        coreObj->valid = 0;
        getMasterHolder()->clearCore(coreObj->index);
    }
    helics::CoreFactory::cleanUpCores();
}

void helicsBrokerFree(helics_broker broker)
{
    auto* brokerObj = helics::getBrokerObject(broker, nullptr);
    if (brokerObj != nullptr) {
        brokerObj->valid = 0;
        getMasterHolder()->clearBroker(brokerObj->index);
    }
    helics::BrokerFactory::cleanUpBrokers();
}

void helicsFederateFree(helics_federate fed)
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
    auto ret = std::async(std::launch::async, []() { helics::CoreFactory::cleanUpCores(std::chrono::milliseconds(2000)); });
    helics::BrokerFactory::cleanUpBrokers(std::chrono::milliseconds(2000));
    ret.get();

    // helics::LoggerManager::closeLogger();
    // helics::cleanupHelicsLibrary();
}

void helicsAbort(int errorCode, const char* message)
{
    auto v = getMasterHolder();
    if (v) {
        v->abortAll(errorCode, message);
    }
}

static const char* invalidQueryString = "Query object is invalid";

static const int validQueryIdentifier = 0x2706'3885;

static helics::QueryObject* getQueryObj(helics_query query, helics_error* err)
{
    if ((err != nullptr) && (err->error_code != 0)) {
        return nullptr;
    }
    if (query == nullptr) {
        assignError(err, helics_error_invalid_object, invalidQueryString);
        return nullptr;
    }
    auto* queryPtr = reinterpret_cast<helics::QueryObject*>(query);
    if (queryPtr->valid != validQueryIdentifier) {
        assignError(err, helics_error_invalid_object, invalidQueryString);
        return nullptr;
    }
    return queryPtr;
}

helics_query helicsCreateQuery(const char* target, const char* query)
{
    auto* queryObj = new helics::QueryObject;
    queryObj->query = AS_STRING(query);
    queryObj->target = AS_STRING(target);
    queryObj->valid = validQueryIdentifier;
    return reinterpret_cast<void*>(queryObj);
}

constexpr auto invalidStringConst = "#invalid";

const char* helicsQueryExecute(helics_query query, helics_federate fed, helics_error* err)
{
    auto* fedObj = getFed(fed, err);
    if (fedObj == nullptr) {
        return invalidStringConst;
    }

    auto* queryObj = getQueryObj(query, err);
    if (queryObj == nullptr) {
        return invalidStringConst;
    }
    if (queryObj->target.empty()) {
        queryObj->response = fedObj->query(queryObj->query, queryObj->mode);
    } else {
        queryObj->response = fedObj->query(queryObj->target, queryObj->query, queryObj->mode);
    }

    return queryObj->response.c_str();
}

const char* helicsQueryCoreExecute(helics_query query, helics_core core, helics_error* err)
{
    auto* coreObj = getCore(core, err);
    if (coreObj == nullptr) {
        return invalidStringConst;
    }
    auto* queryObj = getQueryObj(query, err);
    if (queryObj == nullptr) {
        return invalidStringConst;
    }
    try {
        queryObj->response = coreObj->query(queryObj->target, queryObj->query, queryObj->mode);
        return queryObj->response.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    return invalidStringConst;
    // LCOV_EXCL_START
}

const char* helicsQueryBrokerExecute(helics_query query, helics_broker broker, helics_error* err)
{
    auto* brokerObj = getBroker(broker, err);
    if (brokerObj == nullptr) {
        return invalidStringConst;
    }

    auto* queryObj = getQueryObj(query, err);
    if (queryObj == nullptr) {
        return invalidStringConst;
    }
    try {
        queryObj->response = brokerObj->query(queryObj->target, queryObj->query, queryObj->mode);
        return queryObj->response.c_str();
    }
    // LCOV_EXCL_START
    catch (...) {
        helicsErrorHandler(err);
    }
    return invalidStringConst;
    // LCOV_EXCL_STOP
}

void helicsQueryExecuteAsync(helics_query query, helics_federate fed, helics_error* err)
{
    auto fedObj = getFedSharedPtr(fed, err);
    if (fedObj == nullptr) {
        return;
    }
    auto* queryObj = getQueryObj(query, err);
    if (queryObj == nullptr) {
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

const char* helicsQueryExecuteComplete(helics_query query, helics_error* err)
{
    auto* queryObj = getQueryObj(query, err);
    if (queryObj == nullptr) {
        return invalidStringConst;
    }
    if (queryObj->asyncIndexCode.isValid()) {
        queryObj->response = queryObj->activeFed->queryComplete(queryObj->asyncIndexCode);
    }
    queryObj->activeAsync = false;
    queryObj->activeFed = nullptr;
    queryObj->asyncIndexCode = helics::query_id_t();
    return queryObj->response.c_str();
}

helics_bool helicsQueryIsCompleted(helics_query query)
{
    auto* queryObj = getQueryObj(query, nullptr);
    if (queryObj == nullptr) {
        return helics_false;
    }
    if (queryObj->asyncIndexCode.isValid()) {
        auto res = queryObj->activeFed->isQueryCompleted(queryObj->asyncIndexCode);
        return (res) ? helics_true : helics_false;
    }
    return helics_false;
}

void helicsQuerySetTarget(helics_query query, const char* target, helics_error* err)
{
    auto* queryObj = getQueryObj(query, err);
    if (queryObj == nullptr) {
        return;
    }
    queryObj->target = AS_STRING(target);
}

void helicsQuerySetQueryString(helics_query query, const char* queryString, helics_error* err)
{
    auto* queryObj = getQueryObj(query, err);
    if (queryObj == nullptr) {
        return;
    }
    queryObj->query = AS_STRING(queryString);
}

void helicsQuerySetOrdering(helics_query query, int32_t mode, helics_error* err)
{
    auto* queryObj = getQueryObj(query, err);
    if (queryObj == nullptr) {
        return;
    }
    queryObj->mode = (mode == 0) ? helics_sequencing_mode_fast : helics_sequencing_mode_ordered;
}

void helicsQueryFree(helics_query query)
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
#ifdef ENABLE_ZMQ_CORE
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

helics::FedObject* MasterObjectHolder::findFed(const std::string& fedName)
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
    auto core = cores.lock();
    if ((index < static_cast<int>(core->size())) && (index >= 0)) {
        (*core)[index]->valid = 0;
        (*core)[index] = nullptr;
        if (core->size() > 10) {
            if (std::none_of(core->begin(), core->end(), [](const auto& cr) { return static_cast<bool>(cr); })) {
                core->clear();
            }
        }
    }
}

void MasterObjectHolder::clearFed(int index)
{
    auto fed = feds.lock();
    if ((index < static_cast<int>(fed->size())) && (index >= 0)) {
        (*fed)[index]->valid = 0;
        (*fed)[index] = nullptr;
        if (fed->size() > 10) {
            if (std::none_of(fed->begin(), fed->end(), [](const auto& fd) { return static_cast<bool>(fd); })) {
                fed->clear();
            }
        }
    }
}

void MasterObjectHolder::abortAll(int code, const std::string& error)
{
    {
        auto fedHandle = feds.lock();
        for (auto& fed : fedHandle) {
            if ((fed) && (fed->fedptr)) {
                fed->fedptr->globalError(code, fed->fedptr->getName() + " sending->" + error);
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
        auto coreHandle = cores.lock();
        for (auto& cr : coreHandle) {
            if ((cr) && (cr->coreptr)) {
                cr->valid = 0;
                cr->coreptr->disconnect();
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

const char* MasterObjectHolder::addErrorString(std::string newError)
{
    auto estring = errorStrings.lock();
    estring->push_back(std::move(newError));
    auto& v = estring->back();
    return v.c_str();
}

std::shared_ptr<MasterObjectHolder> getMasterHolder()
{
    static auto instance = std::make_shared<MasterObjectHolder>();
    static gmlc::concurrency::TripWireTrigger tripTriggerholder;
    return instance;
}

gmlc::concurrency::TripWireTrigger tripTrigger;

void clearAllObjects()
{
    auto v = getMasterHolder();
    if (v) {
        v->deleteAll();
    }
}
