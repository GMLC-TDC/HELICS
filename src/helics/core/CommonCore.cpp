/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "CommonCore.hpp"

#include "../common/JsonGeneration.hpp"
#include "../common/JsonProcessingFunctions.hpp"
#include "../common/LogBuffer.hpp"
#include "../common/fmt_format.h"
#include "../common/logging.hpp"
#include "ActionMessage.hpp"
#include "BasicHandleInfo.hpp"
#include "CoreFactory.hpp"
#include "CoreFederateInfo.hpp"
#include "EndpointInfo.hpp"
#include "FederateState.hpp"
#include "FilterCoordinator.hpp"
#include "FilterFederate.hpp"
#include "FilterInfo.hpp"
#include "ForwardingTimeCoordinator.hpp"
#include "InputInfo.hpp"
#include "LogManager.hpp"
#include "PublicationInfo.hpp"
#include "TimeoutMonitor.h"
#include "core-exceptions.hpp"
#include "coreTypeOperations.hpp"
#include "fileConnections.hpp"
#include "gmlc/concurrency/DelayedObjects.hpp"
#include "gmlc/utilities/stringOps.h"
#include "gmlc/utilities/string_viewConversion.h"
#include "helicsVersion.hpp"
#include "helics_definitions.hpp"
#include "loggingHelper.hpp"
#include "queryHelpers.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace helics {

const std::string& state_string(operation_state state)
{
    static const std::string c1{"connected"};
    static const std::string estate{"error"};
    static const std::string dis{"disconnected"};
    switch (state) {
        case operation_state::operating:
            return c1;
        case operation_state::disconnected:
            return dis;
        case operation_state::error:
        default:
            return estate;
    }
}

// timeoutMon is a unique_ptr
CommonCore::CommonCore() noexcept: timeoutMon(new TimeoutMonitor) {}

CommonCore::CommonCore(bool /*arg*/) noexcept: timeoutMon(new TimeoutMonitor) {}

CommonCore::CommonCore(const std::string& coreName):
    BrokerBase(coreName), timeoutMon(new TimeoutMonitor)
{
}

void CommonCore::configure(const std::string& configureString)
{
    if (transitionBrokerState(BrokerState::created, BrokerState::configuring)) {
        // initialize the brokerbase
        auto result = parseArgs(configureString);
        if (result != 0) {
            setBrokerState(BrokerState::created);
            if (result < 0) {
                throw(helics::InvalidParameter("invalid arguments in configure string"));
            }
            return;
        }
        configureBase();
    }
}

void CommonCore::configureFromArgs(int argc, char* argv[])
{
    if (transitionBrokerState(BrokerState::created, BrokerState::configuring)) {
        // initialize the brokerbase
        auto result = parseArgs(argc, argv);
        if (result != 0) {
            setBrokerState(BrokerState::created);
            if (result < 0) {
                throw(helics::InvalidParameter("invalid arguments in command line"));
            }
            return;
        }
        configureBase();
    }
}

void CommonCore::configureFromVector(std::vector<std::string> args)
{
    if (transitionBrokerState(BrokerState::created, BrokerState::configuring)) {
        // initialize the brokerbase
        auto result = parseArgs(std::move(args));
        if (result != 0) {
            setBrokerState(BrokerState::created);
            if (result < 0) {
                throw(helics::InvalidParameter("invalid arguments in arguments structure"));
            }
            return;
        }
        configureBase();
    }
}

bool CommonCore::connect()
{
    auto cBrokerState = getBrokerState();
    if (cBrokerState == BrokerState::errored) {
        return false;
    }
    if (cBrokerState >= BrokerState::configured) {
        if (transitionBrokerState(BrokerState::configured, BrokerState::connecting)) {
            timeoutMon->setTimeout(timeout.to_ms());
            bool res = brokerConnect();
            if (res) {
                // now register this core object as a broker

                ActionMessage m(CMD_REG_BROKER);
                m.source_id = GlobalFederateId{};
                m.name(getIdentifier());
                m.setStringData(getAddress());

                if (!brokerKey.empty()) {
                    m.setString(1, brokerKey);
                }

                setActionFlag(m, core_flag);
                if (useJsonSerialization) {
                    setActionFlag(m, use_json_serialization_flag);
                }

                if (no_ping) {
                    setActionFlag(m, slow_responding_flag);
                }
                if (observer) {
                    setActionFlag(m, observer_flag);
                }
                transmit(parent_route_id, m);
                setBrokerState(BrokerState::connected);
                disconnection.activate();
            } else {
                setBrokerState(BrokerState::configured);
            }
            return res;
        }

        LOG_WARNING(global_id.load(), getIdentifier(), "multiple connect calls");
        while (getBrokerState() == BrokerState::connecting) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    return isConnected();
}

bool CommonCore::isConnected() const
{
    auto currentState = getBrokerState();
    return ((currentState >= BrokerState::connected) &&
            (currentState <= BrokerState::connected_error));
}

const std::string& CommonCore::getIdentifier() const
{
    return identifier;
}

const std::string& CommonCore::getAddress() const
{
    if ((getBrokerState() != BrokerState::connected) || (address.empty())) {
        address = generateLocalAddressString();
    }
    return address;
}

void CommonCore::processDisconnect(bool skipUnregister)
{
    auto cBrokerState = getBrokerState();
    if (cBrokerState > BrokerState::configured) {
        if (cBrokerState < BrokerState::terminating) {
            setBrokerState(BrokerState::terminating);
            sendDisconnect();
            if ((global_broker_id_local != parent_broker_id) &&
                (global_broker_id_local.isValid())) {
                ActionMessage dis(CMD_DISCONNECT);
                dis.source_id = global_broker_id_local;
                transmit(parent_route_id, dis);
            } else {
                ActionMessage dis(CMD_DISCONNECT_NAME);
                dis.payload = getIdentifier();
                transmit(parent_route_id, dis);
            }
            addActionMessage(CMD_STOP);
            return;
        }
        brokerDisconnect();
    }
    setBrokerState(BrokerState::terminated);
    if (!skipUnregister) {
        unregister();
    }
    disconnection.trigger();
}

void CommonCore::disconnect()
{
    ActionMessage udisconnect(CMD_USER_DISCONNECT);
    addActionMessage(udisconnect);
    int cnt{0};
    while (!waitForDisconnect(std::chrono::milliseconds(200))) {
        ++cnt;
        LOG_WARNING(global_id.load(),
                    getIdentifier(),
                    "waiting on disconnect: current state=" + brokerStateName(getBrokerState()));
        if (cnt % 4 == 0) {
            if (!isRunning()) {
                LOG_WARNING(
                    global_id.load(),
                    getIdentifier(),
                    "main loop is stopped but have not received disconnect notice, assuming disconnected");
                return;
            }
            addActionMessage(udisconnect);
        }
        if (cnt % 13 == 0) {
            std::cerr << "waiting on disconnect " << std::endl;
        }
    }
}

bool CommonCore::waitForDisconnect(std::chrono::milliseconds msToWait) const
{
    if (msToWait <= std::chrono::milliseconds(0)) {
        disconnection.wait();
        return true;
    }
    return disconnection.wait_for(msToWait);
}

void CommonCore::unregister()
{
    /*We need to ensure that the destructor is not called immediately upon calling unregister
    otherwise this would be a mess and probably cause segmentation faults so we capture it in a
    local variable that will be destroyed on function exit
    */
    auto keepCoreAlive = CoreFactory::findCore(identifier);
    if (keepCoreAlive) {
        if (keepCoreAlive.get() == this) {
            CoreFactory::unregisterCore(identifier);
        }
    }

    if (!prevIdentifier.empty()) {
        auto keepCoreAlive2 = CoreFactory::findCore(prevIdentifier);
        if (keepCoreAlive2) {
            if (keepCoreAlive2.get() == this) {
                CoreFactory::unregisterCore(prevIdentifier);
            }
        }
    }
}
CommonCore::~CommonCore()
{
    joinAllThreads();
}

FederateState* CommonCore::getFederateAt(LocalFederateId federateID) const
{
    auto feds = federates.lock();
    return (*feds)[federateID.baseValue()];
}

FederateState* CommonCore::getFederate(const std::string& federateName) const
{
    auto feds = federates.lock();
    return feds->find(federateName);
}

FederateState* CommonCore::getHandleFederate(InterfaceHandle handle)
{
    auto local_fed_id = handles.read([handle](auto& hand) { return hand.getLocalFedID(handle); });
    if (local_fed_id.isValid()) {
        auto feds = federates.lock();
        return (*feds)[local_fed_id.baseValue()];
    }

    return nullptr;
}

FederateState* CommonCore::getFederateCore(GlobalFederateId federateID)
{
    auto fed = loopFederates.find(federateID);
    return (fed != loopFederates.end()) ? (fed->fed) : nullptr;
}

FederateState* CommonCore::getFederateCore(const std::string& federateName)
{
    auto fed = loopFederates.find(federateName);
    return (fed != loopFederates.end()) ? (fed->fed) : nullptr;
}

FederateState* CommonCore::getHandleFederateCore(InterfaceHandle handle)
{
    auto local_fed_id = handles.read([handle](auto& hand) { return hand.getLocalFedID(handle); });
    if (local_fed_id.isValid()) {
        return loopFederates[local_fed_id.baseValue()].fed;
    }

    return nullptr;
}

const BasicHandleInfo* CommonCore::getHandleInfo(InterfaceHandle handle) const
{
    return handles.read([handle](auto& hand) { return hand.getHandleInfo(handle.baseValue()); });
}

const BasicHandleInfo* CommonCore::getLocalEndpoint(const std::string& name) const
{
    return handles.read([&name](auto& hand) { return hand.getEndpoint(name); });
}

bool CommonCore::isLocal(GlobalFederateId global_fedid) const
{
    return (loopFederates.find(global_fedid) != loopFederates.end());
}

route_id CommonCore::getRoute(GlobalFederateId global_fedid) const
{
    auto fnd = routing_table.find(global_fedid);
    return (fnd != routing_table.end()) ? fnd->second : parent_route_id;
}

bool CommonCore::isConfigured() const
{
    return (getBrokerState() >= BrokerState::configured);
}

bool CommonCore::isOpenToNewFederates() const
{
    auto cBrokerState = getBrokerState();
    return ((cBrokerState != BrokerState::created) && (cBrokerState < BrokerState::operating) &&
            (maxFederateCount == std::numeric_limits<int32_t>::max() ||
             (federates.lock_shared()->size() < static_cast<size_t>(maxFederateCount))));
}

bool CommonCore::hasError() const
{
    return getBrokerState() == BrokerState::errored;
}
void CommonCore::globalError(LocalFederateId federateID,
                             int errorCode,
                             const std::string& errorString)
{
    if (federateID == gLocalCoreId) {
        ActionMessage m(CMD_GLOBAL_ERROR);
        m.source_id = getGlobalId();
        m.messageID = errorCode;
        m.payload = errorString;
        addActionMessage(m);
        return;
    }
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid error"));
    }
    ActionMessage m(CMD_GLOBAL_ERROR);
    m.source_id = fed->global_id.load();
    m.messageID = errorCode;
    m.payload = errorString;
    addActionMessage(m);
    fed->addAction(m);
    MessageProcessingResult ret = MessageProcessingResult::NEXT_STEP;
    while (ret != MessageProcessingResult::ERROR_RESULT) {
        if (fed->getState() == FederateStates::HELICS_FINISHED ||
            fed->getState() == FederateStates::HELICS_ERROR) {
            return;
        }
        ret = fed->genericUnspecifiedQueueProcess(false);
        switch (ret) {
            case MessageProcessingResult::ERROR_RESULT:
            case MessageProcessingResult::HALTED:
            case MessageProcessingResult::BUSY:
                return;
            default:
                break;
        }
    }
}

void CommonCore::localError(LocalFederateId federateID,
                            int errorCode,
                            const std::string& errorString)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid error"));
    }
    ActionMessage m(CMD_LOCAL_ERROR);
    m.source_id = fed->global_id.load();
    m.messageID = errorCode;
    m.payload = errorString;
    addActionMessage(m);
    fed->addAction(m);
    MessageProcessingResult ret = MessageProcessingResult::NEXT_STEP;
    while (ret != MessageProcessingResult::ERROR_RESULT) {
        if (fed->getState() == FederateStates::HELICS_FINISHED ||
            fed->getState() == FederateStates::HELICS_ERROR) {
            return;
        }
        ret = fed->genericUnspecifiedQueueProcess(false);
        switch (ret) {
            case MessageProcessingResult::ERROR_RESULT:
            case MessageProcessingResult::HALTED:
            case MessageProcessingResult::BUSY:
                return;
            default:
                break;
        }
    }
}

int CommonCore::getErrorCode() const
{
    return lastErrorCode.load();
}

std::string CommonCore::getErrorMessage() const
{
    // used to sync threads and ensure a string is available
    (void)lastErrorCode.load();
    return lastErrorString;
}

void CommonCore::finalize(LocalFederateId federateID)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid finalize"));
    }

    auto cbrokerState = getBrokerState();
    switch (cbrokerState) {
        case BrokerState::terminated:
        case BrokerState::terminating:
        case BrokerState::terminating_error:
        case BrokerState::errored: {
            ActionMessage bye(CMD_STOP);
            bye.source_id = fed->global_id.load();
            bye.dest_id = bye.source_id;
            addActionMessage(bye);
            fed->addAction(bye);
        } break;
        default: {
            ActionMessage bye(CMD_DISCONNECT);
            bye.source_id = fed->global_id.load();
            bye.dest_id = bye.source_id;
            addActionMessage(bye);
        } break;
    }

    fed->finalize();
}

bool CommonCore::allInitReady() const
{
    if (delayInitCounter > 0) {
        return false;
    }
    // the federate count must be greater than the min size
    auto fcount = static_cast<decltype(minFederateCount)>(loopFederates.size());
    if (fcount < minFederateCount || fcount < minChildCount) {
        return false;
    }
    // all federates must be requesting init
    return std::all_of(loopFederates.begin(), loopFederates.end(), [](const auto& fed) {
        return fed->init_transmitted.load();
    });
}

bool CommonCore::allDisconnected() const
{
    // all federates must have hit finished state
    auto afed = (minFederateState() == operation_state::disconnected);
    if ((hasTimeDependency) || (hasFilters)) {
        if (afed) {
            if (!timeCoord->hasActiveTimeDependencies()) {
                return true;
            }
            if (timeCoord->dependencyCount() == 1 && timeCoord->getMinDependency() == filterFedID) {
                return !filterFed->hasActiveTimeDependencies();
            }
        }
        return false;
    }
    return (afed);
}

operation_state CommonCore::minFederateState() const
{
    operation_state op{operation_state::disconnected};
    for (const auto& fed : loopFederates) {
        if (fed.state < op) {
            op = fed.state;
        }
    }
    return op;
}

double CommonCore::getSimulationTime() const
{
    return simTime.load();
}

void CommonCore::setCoreReadyToInit()
{
    // use the flag mechanics that do the same thing
    setFlagOption(gLocalCoreId, defs::Flags::ENABLE_INIT_ENTRY);
}

/** this function will generate an appropriate exception for the error
code listed in a Federate*/
static void generateFederateException(const FederateState* fed)
{
    auto eCode = fed->lastErrorCode();
    switch (eCode) {
        case 0:
            return;
        case defs::Errors::INVALID_ARGUMENT:
            throw(InvalidParameter(fed->lastErrorString()));
        case defs::Errors::INVALID_FUNCTION_CALL:
            throw(InvalidFunctionCall(fed->lastErrorString()));
        case defs::Errors::INVALID_OBJECT:
            throw(InvalidIdentifier(fed->lastErrorString()));
        case defs::Errors::INVALID_STATE_TRANSITION:
            throw(InvalidFunctionCall(fed->lastErrorString()));
        case defs::Errors::CONNECTION_FAILURE:
            throw(ConnectionFailure(fed->lastErrorString()));
        case defs::Errors::REGISTRATION_FAILURE:
            throw(RegistrationFailure(fed->lastErrorString()));
        default:
            throw(HelicsException(fed->lastErrorString()));
    }
}
void CommonCore::enterInitializingMode(LocalFederateId federateID)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid for Entering Init"));
    }
    switch (fed->getState()) {
        case HELICS_CREATED:
            break;
        case HELICS_INITIALIZING:
            return;
        default:
            throw(InvalidFunctionCall("May only enter initializing state from created state"));
    }

    bool exp = false;
    if (fed->init_requested.compare_exchange_strong(
            exp, true)) {  // only enter this loop once per federate
        ActionMessage m(CMD_INIT);
        m.source_id = fed->global_id.load();
        addActionMessage(m);

        auto check = fed->enterInitializingMode();
        if (check != IterationResult::NEXT_STEP) {
            fed->init_requested = false;
            if (check == IterationResult::HALTED) {
                throw(HelicsSystemFailure());
            }
            generateFederateException(fed);
        }
        return;
    }
    throw(InvalidFunctionCall("federate already has requested entry to initializing State"));
}

IterationResult CommonCore::enterExecutingMode(LocalFederateId federateID, IterationRequest iterate)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (EnterExecutingState)"));
    }
    if (HELICS_EXECUTING == fed->getState()) {
        return IterationResult::NEXT_STEP;
    }
    if (HELICS_INITIALIZING != fed->getState()) {
        throw(InvalidFunctionCall("federate is in invalid state for calling entry to exec mode"));
    }

    // do an exec check on the fed to process previously received messages so it can't get in a
    // deadlocked state
    ActionMessage execc(CMD_EXEC_CHECK);
    fed->addAction(execc);

    // do a check on the core to make sure it isn't stopped
    auto cBrokerState = getBrokerState();
    switch (cBrokerState) {
        case BrokerState::terminating:
        case BrokerState::terminated:
        case BrokerState::terminating_error:
        case BrokerState::connected_error:
        case BrokerState::errored: {
            ActionMessage terminate(CMD_STOP);
            terminate.dest_id = fed->global_id;
            terminate.source_id = fed->global_id;
            fed->addAction(terminate);
        } break;
        default:
            break;
    }

    ActionMessage exec(CMD_EXEC_REQUEST);
    exec.source_id = fed->global_id.load();
    exec.dest_id = fed->global_id.load();
    setIterationFlags(exec, iterate);
    setActionFlag(exec, indicator_flag);
    addActionMessage(exec);

    return fed->enterExecutingMode(iterate, false);
}

LocalFederateId CommonCore::registerFederate(const std::string& name, const CoreFederateInfo& info)
{
    if (!waitCoreRegistration()) {
        if (getBrokerState() == BrokerState::errored) {
            if (!lastErrorString.empty()) {
                throw(RegistrationFailure(lastErrorString));
            }
        }
        throw(RegistrationFailure(
            "core is unable to register and has timed out, federate cannot be registered"));
    }
    if (getBrokerState() >= BrokerState::operating) {
        throw(RegistrationFailure("Core has already moved to operating state"));
    }
    FederateState* fed = nullptr;
    bool checkProperties{false};
    LocalFederateId local_id;
    {
        auto feds = federates.lock();
        if (static_cast<decltype(maxFederateCount)>(feds->size()) >= maxFederateCount) {
            throw(RegistrationFailure("maximum number of federates in the core has been reached"));
        }
        auto id = feds->insert(name, name, info);
        if (id) {
            local_id = LocalFederateId(static_cast<int32_t>(*id));
            fed = (*feds)[*id];
        } else {
            throw(RegistrationFailure("duplicate names " + name +
                                      "detected multiple federates with the same name"));
        }
        if (feds->size() == 1) {
            checkProperties = true;
        }
    }
    if (fed == nullptr) {
        throw(RegistrationFailure("unknown allocation error occurred"));
    }
    // setting up the Logger
    // auto ptr = fed.get();
    // if we are using the Logger, log all messages coming from the federates so they can control
    // the level*/
    fed->setLogger([this](int level, std::string_view ident, std::string_view message) {
        sendToLogger(parent_broker_id, LogLevels::FED + level, ident, message);
    });

    fed->local_id = local_id;
    fed->setParent(this);
    if (enable_profiling) {
        fed->setOptionFlag(defs::PROFILING, true);
    }
    ActionMessage m(CMD_REG_FED);
    m.name(name);
    if (observer || fed->getOptionFlag(HELICS_FLAG_OBSERVER)) {
        setActionFlag(m, observer_flag);
    }
    addActionMessage(m);
    // check some properties that should be inherited from the federate if it is the first one
    if (checkProperties) {
        // if this is the first federate then the core should inherit the logging level properties
        for (const auto& prop : info.intProps) {
            switch (prop.first) {
                case defs::Properties::LOG_LEVEL:
                case defs::Properties::FILE_LOG_LEVEL:
                case defs::Properties::CONSOLE_LOG_LEVEL:
                    setIntegerProperty(gLocalCoreId, prop.first, static_cast<int16_t>(prop.second));
                default:
                    break;
            }
        }
    }
    // now wait for the federateQueue to get the response
    auto valid = fed->waitSetup();
    if (valid == IterationResult::NEXT_STEP) {
        return local_id;
    }
    throw(RegistrationFailure(std::string("fed received Failure ") + fed->lastErrorString()));
}

const std::string& CommonCore::getFederateName(LocalFederateId federateID) const
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (federateName)"));
    }
    return fed->getIdentifier();
}

static const std::string unknownString("#unknown");

const std::string& CommonCore::getFederateNameNoThrow(GlobalFederateId federateID) const noexcept
{
    static const std::string filterString = getIdentifier() + "_filters";

    auto* fed = getFederateAt(LocalFederateId(federateID.localIndex()));
    return (fed == nullptr) ? ((federateID == filterFedID) ? filterString : unknownString) :
                              fed->getIdentifier();
}

LocalFederateId CommonCore::getFederateId(const std::string& name) const
{
    auto feds = federates.lock();
    auto* fed = feds->find(name);
    if (fed != nullptr) {
        return fed->local_id;
    }

    return {};
}

int32_t CommonCore::getFederationSize()
{
    if (getBrokerState() >= BrokerState::operating) {
        return _global_federation_size;
    }
    // if we are in initialization return the local federation size
    return static_cast<int32_t>(federates.lock()->size());
}

Time CommonCore::timeRequest(LocalFederateId federateID, Time next)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid timeRequest"));
    }
    auto cBrokerState = getBrokerState();
    switch (cBrokerState) {
        case BrokerState::terminating:
        case BrokerState::terminated:
        case BrokerState::terminating_error:
        case BrokerState::connected_error:
        case BrokerState::errored: {
            ActionMessage terminate(CMD_STOP);
            terminate.dest_id = fed->global_id;
            terminate.source_id = fed->global_id;
            fed->addAction(terminate);
        } break;
        default:
            break;
    }
    switch (fed->getState()) {
        case HELICS_EXECUTING: {
            // generate the request through the core
            ActionMessage treq(CMD_TIME_REQUEST);
            treq.source_id = fed->global_id.load();
            treq.dest_id = fed->global_id.load();
            treq.actionTime = next;
            setActionFlag(treq, indicator_flag);
            addActionMessage(treq);
            auto ret = fed->requestTime(next, IterationRequest::NO_ITERATIONS, false);
            switch (ret.state) {
                case IterationResult::ERROR_RESULT:
                    throw(FunctionExecutionFailure(fed->lastErrorString()));
                case IterationResult::HALTED:
                    return Time::maxVal();
                default:
                    return ret.grantedTime;
            }
        }
        case HELICS_FINISHED:
            return Time::maxVal();
        default:
            throw(InvalidFunctionCall("time request should only be called in execution state"));
    }
}

iteration_time CommonCore::requestTimeIterative(LocalFederateId federateID,
                                                Time next,
                                                IterationRequest iterate)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid timeRequestIterative"));
    }

    switch (fed->getState()) {
        case HELICS_EXECUTING:
            break;
        case HELICS_FINISHED:
        case HELICS_TERMINATING:
            return iteration_time{Time::maxVal(), IterationResult::HALTED};
        case HELICS_CREATED:
        case HELICS_INITIALIZING:
            return iteration_time{timeZero, IterationResult::ERROR_RESULT};
        case HELICS_UNKNOWN:
        case HELICS_ERROR:
            return iteration_time{Time::maxVal(), IterationResult::ERROR_RESULT};
    }

    // limit the iterations
    if (iterate == IterationRequest::ITERATE_IF_NEEDED) {
        if (fed->getCurrentIteration() >= maxIterationCount) {
            iterate = IterationRequest::NO_ITERATIONS;
        }
    }

    auto cBrokerState = getBrokerState();
    switch (cBrokerState) {
        case BrokerState::terminating:
        case BrokerState::terminated:
        case BrokerState::connected_error:
        case BrokerState::terminating_error:
        case BrokerState::errored: {
            ActionMessage terminate(CMD_STOP);
            terminate.dest_id = fed->global_id;
            terminate.source_id = fed->global_id;
            fed->addAction(terminate);
        } break;
        default:
            break;
    }
    // generate the request through the core
    ActionMessage treq(CMD_TIME_REQUEST);
    treq.source_id = fed->global_id.load();
    treq.dest_id = fed->global_id.load();
    treq.actionTime = next;
    setIterationFlags(treq, iterate);
    setActionFlag(treq, indicator_flag);
    addActionMessage(treq);

    return fed->requestTime(next, iterate, false);
}

void CommonCore::processCommunications(LocalFederateId federateID,
                                       std::chrono::milliseconds msToWait)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (processCommunications)"));
    }

    switch (fed->getState()) {
        case HELICS_FINISHED:
        case HELICS_TERMINATING:
            return;
        default:
            break;
    }
    fed->processCommunications(msToWait);
}
Time CommonCore::getCurrentTime(LocalFederateId federateID) const
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw InvalidIdentifier("federateID not valid (getCurrentTime)");
    }
    return fed->grantedTime();
}

uint64_t CommonCore::getCurrentReiteration(LocalFederateId federateID) const
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw InvalidIdentifier("federateID not valid (getCurrentReiteration)");
    }
    return fed->getCurrentIteration();
}

void CommonCore::setIntegerProperty(LocalFederateId federateID,
                                    int32_t property,
                                    int16_t propertyValue)
{
    if (federateID == gLocalCoreId) {
        if (!waitCoreRegistration()) {
            throw(FunctionExecutionFailure(
                "core is unable to register and has timed out, property was not set"));
        }
        ActionMessage cmd(CMD_CORE_CONFIGURE);
        cmd.dest_id = global_id.load();
        cmd.messageID = property;
        cmd.setExtraData(propertyValue);
        addActionMessage(cmd);
        return;
    }
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (getMaximumIterations)"));
    }
    ActionMessage cmd(CMD_FED_CONFIGURE_INT);
    cmd.messageID = property;
    cmd.setExtraData(propertyValue);
    fed->setProperties(cmd);
}

void CommonCore::setTimeProperty(LocalFederateId federateID, int32_t property, Time time)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (setTimeDelta)"));
    }
    if (time < timeZero) {
        throw(InvalidParameter("time properties must be greater than or equal to zero"));
    }

    ActionMessage cmd(CMD_FED_CONFIGURE_TIME);
    cmd.messageID = property;
    cmd.actionTime = time;
    fed->setProperties(cmd);
}

Time CommonCore::getTimeProperty(LocalFederateId federateID, int32_t property) const
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (setTimeDelta)"));
    }
    return fed->getTimeProperty(property);
}

int16_t CommonCore::getIntegerProperty(LocalFederateId federateID, int32_t property) const
{
    if (federateID == gLocalCoreId) {
        if (property == HELICS_PROPERTY_INT_LOG_LEVEL ||
            property == HELICS_PROPERTY_INT_CONSOLE_LOG_LEVEL) {
            return mLogManager->getConsoleLevel();
        }
        if (property == HELICS_PROPERTY_INT_FILE_LOG_LEVEL) {
            return mLogManager->getFileLevel();
        }
        if (property == HELICS_PROPERTY_INT_LOG_BUFFER) {
            return static_cast<int16_t>(mLogManager->getLogBuffer().capacity());
        }
        return 0;
    }
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (setTimeDelta)"));
    }
    return fed->getIntegerProperty(property);
}

void CommonCore::setFlagOption(LocalFederateId federateID, int32_t flag, bool flagValue)
{
    if (flag == defs::Flags::DUMPLOG || flag == defs::Flags::FORCE_LOGGING_FLUSH) {
        ActionMessage cmd(CMD_BASE_CONFIGURE);
        cmd.messageID = flag;
        if (flagValue) {
            setActionFlag(cmd, indicator_flag);
        }
        addActionMessage(cmd);
    }
    if (federateID == gLocalCoreId) {
        switch (flag) {
            case defs::Flags::DELAY_INIT_ENTRY:
                if (flagValue) {
                    ++delayInitCounter;
                } else {
                    ActionMessage cmd(CMD_CORE_CONFIGURE);
                    cmd.messageID = defs::Flags::DELAY_INIT_ENTRY;
                    addActionMessage(cmd);
                }
                break;
            case defs::LOG_BUFFER:
                mLogManager->getLogBuffer().enable(flagValue);
                break;
            default: {
                ActionMessage cmd(CMD_CORE_CONFIGURE);
                cmd.messageID = flag;
                if (flagValue) {
                    setActionFlag(cmd, indicator_flag);
                }
                addActionMessage(cmd);
            } break;
        }
        return;
    }

    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (setFlag)"));
    }
    ActionMessage cmd(CMD_FED_CONFIGURE_FLAG);
    cmd.messageID = flag;
    if (flagValue) {
        setActionFlag(cmd, indicator_flag);
    }
    fed->setProperties(cmd);
}

bool CommonCore::getFlagOption(LocalFederateId federateID, int32_t flag) const
{
    switch (flag) {
        case defs::Flags::ENABLE_INIT_ENTRY:
            return (delayInitCounter.load() == 0);
        case defs::Flags::DELAY_INIT_ENTRY:
            return (delayInitCounter.load() != 0);
        case defs::Flags::DUMPLOG:
        case defs::Flags::FORCE_LOGGING_FLUSH:
        case defs::Flags::DEBUGGING:
            return getFlagValue(flag);
        case defs::Flags::FORWARD_COMPUTE:
        case defs::Flags::SINGLE_THREAD_FEDERATE:
        case defs::Flags::ROLLBACK:
            return false;
        default:
            break;
    }
    if (federateID == gLocalCoreId) {
        if (flag == defs::Properties::LOG_BUFFER) {
            return (mLogManager->getLogBuffer().capacity() > 0);
        }
        return false;
    }
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (setTimeDelta)"));
    }
    return fed->getOptionFlag(flag);
}

const BasicHandleInfo& CommonCore::createBasicHandle(GlobalFederateId global_federateId,
                                                     LocalFederateId local_federateId,
                                                     InterfaceType HandleType,
                                                     const std::string& key,
                                                     const std::string& type,
                                                     const std::string& units,
                                                     uint16_t flags)
{
    return handles.modify([&](auto& hand) -> const BasicHandleInfo& {
        auto& hndl = hand.addHandle(global_federateId, HandleType, key, type, units);
        hndl.local_fed_id = local_federateId;
        hndl.flags = flags;
        return hndl;
    });
}

static const std::string emptyString;

InterfaceHandle CommonCore::registerInput(LocalFederateId federateID,
                                          const std::string& key,
                                          const std::string& type,
                                          const std::string& units)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (registerNamedInput)"));
    }
    const auto* ci = handles.read([&key](auto& hand) { return hand.getInput(key); });
    if (ci != nullptr) {  // this key is already found
        throw(RegistrationFailure("named Input already exists"));
    }
    const auto& handle = createBasicHandle(fed->global_id,
                                           fed->local_id,
                                           InterfaceType::INPUT,
                                           key,
                                           type,
                                           units,
                                           fed->getInterfaceFlags());

    auto id = handle.getInterfaceHandle();
    fed->createInterface(InterfaceType::INPUT, id, key, type, units);

    LOG_INTERFACES(parent_broker_id,
                   fed->getIdentifier(),
                   fmt::format("registering Input {}", key));
    ActionMessage m(CMD_REG_INPUT);
    m.source_id = fed->global_id.load();
    m.source_handle = id;
    m.flags = handle.flags;
    m.name(key);
    m.setStringData(type, units);

    actionQueue.push(std::move(m));
    return id;
}

InterfaceHandle CommonCore::getInput(LocalFederateId federateID, const std::string& key) const
{
    const auto* ci = handles.read([&key](auto& hand) { return hand.getInput(key); });
    if (ci->local_fed_id != federateID) {
        return {};
    }
    return ci->getInterfaceHandle();
}

InterfaceHandle CommonCore::registerPublication(LocalFederateId federateID,
                                                const std::string& key,
                                                const std::string& type,
                                                const std::string& units)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (registerPublication)"));
    }
    LOG_INTERFACES(parent_broker_id, fed->getIdentifier(), fmt::format("registering PUB {}", key));
    const auto* pub = handles.read([&key](auto& hand) { return hand.getPublication(key); });
    if (pub != nullptr)  // this key is already found
    {
        throw(RegistrationFailure("Publication key already exists"));
    }
    const auto& handle = createBasicHandle(fed->global_id,
                                           fed->local_id,
                                           InterfaceType::PUBLICATION,
                                           key,
                                           type,
                                           units,
                                           fed->getInterfaceFlags());

    auto id = handle.handle.handle;
    fed->createInterface(InterfaceType::PUBLICATION, id, key, type, units);

    ActionMessage m(CMD_REG_PUB);
    m.source_id = fed->global_id.load();
    m.source_handle = id;
    m.name(key);
    m.flags = handle.flags;
    m.setStringData(type, units);

    actionQueue.push(std::move(m));
    return id;
}

InterfaceHandle CommonCore::getPublication(LocalFederateId federateID, const std::string& key) const
{
    const auto* pub = handles.read([&key](auto& hand) { return hand.getPublication(key); });
    if (pub->local_fed_id != federateID) {
        return {};
    }
    return pub->getInterfaceHandle();
}

const std::string emptyStr;

const std::string& CommonCore::getHandleName(InterfaceHandle handle) const
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo != nullptr) {
        return handleInfo->key;
    }
    return emptyStr;
}

const std::string& CommonCore::getInjectionUnits(InterfaceHandle handle) const
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo != nullptr) {
        switch (handleInfo->handleType) {
            case InterfaceType::INPUT: {
                auto* fed = getFederateAt(handleInfo->local_fed_id);
                auto* inpInfo = fed->interfaces().getInput(handle);
                if (inpInfo != nullptr) {
                    return inpInfo->getInjectionUnits();
                }
                break;
            }
            case InterfaceType::PUBLICATION:
                return handleInfo->units;
            default:
                return emptyStr;
        }
    }
    return emptyStr;
}  // namespace helics

const std::string& CommonCore::getExtractionUnits(InterfaceHandle handle) const
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo != nullptr) {
        switch (handleInfo->handleType) {
            case InterfaceType::INPUT:
            case InterfaceType::PUBLICATION:
                return handleInfo->units;
            default:
                break;
        }
    }
    return emptyStr;
}

const std::string& CommonCore::getInjectionType(InterfaceHandle handle) const
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo != nullptr) {
        switch (handleInfo->handleType) {
            case InterfaceType::INPUT: {
                auto* fed = getFederateAt(handleInfo->local_fed_id);
                auto* inpInfo = fed->interfaces().getInput(handle);
                if (inpInfo != nullptr) {
                    return inpInfo->getInjectionType();
                }
                break;
            }
            case InterfaceType::ENDPOINT:
                return handleInfo->type;
            case InterfaceType::FILTER:
                return handleInfo->type_in;
            default:
                return emptyStr;
        }
    }
    return emptyStr;
}

const std::string& CommonCore::getExtractionType(InterfaceHandle handle) const
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo != nullptr) {
        switch (handleInfo->handleType) {
            case InterfaceType::PUBLICATION:
            case InterfaceType::INPUT:
            case InterfaceType::ENDPOINT:
                return handleInfo->type;
            case InterfaceType::FILTER:
                return handleInfo->type_out;
            default:
                return emptyStr;
        }
    }
    return emptyStr;
}

void CommonCore::setHandleOption(InterfaceHandle handle, int32_t option, int32_t option_value)
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo == nullptr) {
        return;
    }
    handles.modify([handle, option, option_value](auto& hand) {
        return hand.setHandleOption(handle, option, option_value);
    });

    ActionMessage fcn(CMD_INTERFACE_CONFIGURE);
    fcn.dest_handle = handle;
    fcn.messageID = option;
    fcn.counter = static_cast<uint16_t>(handleInfo->handleType);
    fcn.setExtraDestData(option_value);
    if (option_value != 0) {
        setActionFlag(fcn, indicator_flag);
    }
    if (handleInfo->handleType != InterfaceType::FILTER) {
        auto* fed = getHandleFederate(handle);
        if (fed != nullptr) {
            fcn.dest_id = fed->global_id;
            fed->setProperties(fcn);
        }
    } else {
        // must be for filter
    }
}

int32_t CommonCore::getHandleOption(InterfaceHandle handle, int32_t option) const
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo == nullptr) {
        return 0;
    }
    switch (option) {
        case defs::Options::CONNECTION_REQUIRED:
        case defs::Options::CONNECTION_OPTIONAL:
            return handles.read(
                [handle, option](auto& hand) { return hand.getHandleOption(handle, option); });
        default:
            break;
    }
    if (handleInfo->handleType != InterfaceType::FILTER) {
        auto* fed = getFederateAt(handleInfo->local_fed_id);
        if (fed != nullptr) {
            return fed->getHandleOption(handle, static_cast<char>(handleInfo->handleType), option);
        }
    } else {
        // must be for filter
    }
    return 0;
}

void CommonCore::closeHandle(InterfaceHandle handle)
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo == nullptr) {
        throw(InvalidIdentifier("invalid handle"));
    }
    if (checkActionFlag(*handleInfo, disconnected_flag)) {
        return;
    }
    ActionMessage cmd(CMD_CLOSE_INTERFACE);
    cmd.setSource(handleInfo->handle);
    cmd.messageID = static_cast<int32_t>(handleInfo->handleType);
    addActionMessage(cmd);
    handles.modify(
        [handle](auto& hand) { setActionFlag(*hand.getHandleInfo(handle), disconnected_flag); });
}

void CommonCore::removeTarget(InterfaceHandle handle, std::string_view targetToRemove)
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo == nullptr) {
        throw(InvalidIdentifier("invalid handle"));
    }

    ActionMessage cmd;
    cmd.setSource(handleInfo->handle);
    cmd.name(targetToRemove);
    auto* fed = getFederateAt(handleInfo->local_fed_id);
    if (fed != nullptr) {
        cmd.actionTime = fed->grantedTime();
    }
    switch (handleInfo->handleType) {
        case InterfaceType::PUBLICATION:
            cmd.setAction(CMD_REMOVE_NAMED_INPUT);
            break;
        case InterfaceType::FILTER:
            cmd.setAction(CMD_REMOVE_NAMED_ENDPOINT);
            break;
        case InterfaceType::INPUT:
            cmd.setAction(CMD_REMOVE_NAMED_PUBLICATION);
            fed->addAction(cmd);
            break;
        case InterfaceType::ENDPOINT:
            cmd.setAction(CMD_REMOVE_NAMED_FILTER);
            break;
        default:
            return;
    }
    addActionMessage(std::move(cmd));
}

void CommonCore::addDestinationTarget(InterfaceHandle handle,
                                      std::string_view dest,
                                      InterfaceType hint)
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo == nullptr) {
        throw(InvalidIdentifier("invalid handle"));
    }
    ActionMessage cmd;
    cmd.setSource(handleInfo->handle);
    cmd.flags = handleInfo->flags;
    cmd.counter = static_cast<uint16_t>(handleInfo->handleType);
    setActionFlag(cmd, destination_target);
    cmd.payload = dest;
    switch (handleInfo->handleType) {
        case InterfaceType::ENDPOINT:
            if (hint == InterfaceType::FILTER) {
                cmd.setAction(CMD_ADD_NAMED_FILTER);
            } else {
                cmd.setAction(CMD_ADD_NAMED_ENDPOINT);
                cmd.counter = static_cast<uint16_t>(InterfaceType::ENDPOINT);
            }
            if (handleInfo->key.empty()) {
                cmd.setStringData(handleInfo->type, handleInfo->units);
            }
            break;
        case InterfaceType::FILTER:
            cmd.setAction(CMD_ADD_NAMED_ENDPOINT);
            if (handleInfo->key.empty()) {
                if ((!handleInfo->type_in.empty()) || (!handleInfo->type_out.empty())) {
                    cmd.setStringData(handleInfo->type_in, handleInfo->type_out);
                }
            }
            if (checkActionFlag(*handleInfo, clone_flag)) {
                setActionFlag(cmd, clone_flag);
            }
            break;
        case InterfaceType::PUBLICATION:
            cmd.setAction(CMD_ADD_NAMED_INPUT);
            if (handleInfo->key.empty()) {
                cmd.setStringData(handleInfo->type, handleInfo->units);
            }
            break;
        case InterfaceType::INPUT:
        default:
            throw(InvalidIdentifier("inputs cannot have destination targets"));
    }

    addActionMessage(std::move(cmd));
}

void CommonCore::addSourceTarget(InterfaceHandle handle,
                                 std::string_view targetName,
                                 InterfaceType hint)
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo == nullptr) {
        throw(InvalidIdentifier("invalid handle"));
    }
    ActionMessage cmd;
    cmd.setSource(handleInfo->handle);
    cmd.flags = handleInfo->flags;
    cmd.payload = targetName;
    switch (handleInfo->handleType) {
        case InterfaceType::ENDPOINT:
            if (hint == InterfaceType::FILTER) {
                cmd.setAction(CMD_ADD_NAMED_FILTER);
            } else if (hint == InterfaceType::PUBLICATION) {
                cmd.setAction(CMD_ADD_NAMED_PUBLICATION);
            } else {
                cmd.setAction(CMD_ADD_NAMED_ENDPOINT);
                cmd.counter = static_cast<uint16_t>(InterfaceType::ENDPOINT);
            }
            break;
        case InterfaceType::FILTER:
            cmd.setAction(CMD_ADD_NAMED_ENDPOINT);
            if (handleInfo->key.empty()) {
                if ((!handleInfo->type_in.empty()) || (!handleInfo->type_out.empty())) {
                    cmd.setStringData(handleInfo->type_in, handleInfo->type_out);
                }
            }
            if (checkActionFlag(*handleInfo, clone_flag)) {
                setActionFlag(cmd, clone_flag);
            }
            break;
        case InterfaceType::INPUT:
            cmd.setAction(CMD_ADD_NAMED_PUBLICATION);
            break;
        case InterfaceType::PUBLICATION:
        default:
            throw(InvalidIdentifier("publications cannot have source targets"));
    }
    addActionMessage(std::move(cmd));
}

const std::string& CommonCore::getDestinationTargets(InterfaceHandle handle) const
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo != nullptr) {
        switch (handleInfo->handleType) {
            case InterfaceType::INPUT: {
                auto* fed = getFederateAt(handleInfo->local_fed_id);
                auto* inpInfo = fed->interfaces().getInput(handle);
                if (inpInfo != nullptr) {
                    return inpInfo->getTargets();
                }
                break;
            }
            case InterfaceType::PUBLICATION:
                return emptyStr;
            case InterfaceType::ENDPOINT: {
                auto* fed = getFederateAt(handleInfo->local_fed_id);
                auto* eptInfo = fed->interfaces().getEndpoint(handle);
                if (eptInfo != nullptr) {
                    return eptInfo->getDestinationTargets();
                }
                break;
            }
            case InterfaceType::FILTER:
            default:
                return emptyStr;
        }
    }
    return emptyStr;
}

const std::string& CommonCore::getSourceTargets(InterfaceHandle handle) const
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo != nullptr) {
        switch (handleInfo->handleType) {
            case InterfaceType::INPUT: {
                auto* fed = getFederateAt(handleInfo->local_fed_id);
                auto* inpInfo = fed->interfaces().getInput(handle);
                if (inpInfo != nullptr) {
                    return inpInfo->getTargets();
                }
                break;
            }
            case InterfaceType::PUBLICATION:
                return emptyStr;
            case InterfaceType::ENDPOINT: {
                auto* fed = getFederateAt(handleInfo->local_fed_id);
                auto* eptInfo = fed->interfaces().getEndpoint(handle);
                if (eptInfo != nullptr) {
                    return eptInfo->getSourceTargets();
                }
                break;
            }
            case InterfaceType::FILTER: {
                break;
            }
            default:
                return emptyStr;
        }
    }
    return emptyStr;
}

void CommonCore::setValue(InterfaceHandle handle, const char* data, uint64_t len)
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo == nullptr) {
        throw(InvalidIdentifier("Handle not valid (setValue)"));
    }
    if (handleInfo->handleType != InterfaceType::PUBLICATION) {
        throw(InvalidIdentifier("handle does not point to a publication"));
    }
    if (checkActionFlag(*handleInfo, disconnected_flag)) {
        return;
    }
    if (!handleInfo->used) {
        return;  // if the value is not required do nothing
    }
    auto* fed = getFederateAt(handleInfo->local_fed_id);
    if (fed->checkAndSetValue(handle, data, len)) {
        if (fed->loggingLevel() >= HELICS_LOG_LEVEL_DATA) {
            fed->logMessage(HELICS_LOG_LEVEL_DATA,
                            fed->getIdentifier(),
                            fmt::format("setting value for {} size {}", handleInfo->key, len));
        }
        auto subs = fed->getSubscribers(handle);
        if (subs.empty()) {
            return;
        }
        if (subs.size() == 1) {
            ActionMessage mv(CMD_PUB);
            mv.source_id = handleInfo->getFederateId();
            mv.source_handle = handle;
            mv.setDestination(subs[0]);
            mv.counter = static_cast<uint16_t>(fed->getCurrentIteration());
            mv.payload.assign(data, len);
            mv.actionTime = fed->nextAllowedSendTime();
            actionQueue.push(std::move(mv));
            return;
        }
        ActionMessage package(CMD_MULTI_MESSAGE);
        package.source_id = handleInfo->getFederateId();
        package.source_handle = handle;

        ActionMessage mv(CMD_PUB);
        mv.source_id = handleInfo->getFederateId();
        mv.source_handle = handle;
        mv.counter = static_cast<uint16_t>(fed->getCurrentIteration());
        mv.payload.assign(data, len);
        mv.actionTime = fed->nextAllowedSendTime();

        for (auto& target : subs) {
            mv.setDestination(target);
            auto res = appendMessage(package, mv);
            if (res < 0)  // deal with max package size if there are a lot of subscribers
            {
                actionQueue.push(std::move(package));
                package = ActionMessage(CMD_MULTI_MESSAGE);
                package.source_id = handleInfo->getFederateId();
                package.source_handle = handle;
                appendMessage(package, mv);
            }
        }
        actionQueue.push(std::move(package));
    }
}

const std::shared_ptr<const SmallBuffer>& CommonCore::getValue(InterfaceHandle handle,
                                                               uint32_t* inputIndex)
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo == nullptr) {
        throw(InvalidIdentifier("Handle is invalid (getValue)"));
    }

    if (handleInfo->handleType != InterfaceType::INPUT) {
        throw(InvalidIdentifier("Handle does not identify an input"));
    }
    auto& fed = *getFederateAt(handleInfo->local_fed_id);
    std::lock_guard<FederateState> lk(fed);
    return fed.getValue(handle, inputIndex);
}

const std::vector<std::shared_ptr<const SmallBuffer>>&
    CommonCore::getAllValues(InterfaceHandle handle)
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo == nullptr) {
        throw(InvalidIdentifier("Handle is invalid (getValue)"));
    }

    if (handleInfo->handleType != InterfaceType::INPUT) {
        throw(InvalidIdentifier("Handle does not identify an input"));
    }
    auto& fed = *getFederateAt(handleInfo->local_fed_id);
    std::lock_guard<FederateState> lk(fed);
    return fed.getAllValues(handle);
}

const std::vector<InterfaceHandle>& CommonCore::getValueUpdates(LocalFederateId federateID)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (getValueUpdates)"));
    }
    return fed->getEvents();
}

InterfaceHandle CommonCore::registerEndpoint(LocalFederateId federateID,
                                             const std::string& name,
                                             const std::string& type)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (registerEndpoint)"));
    }
    const auto* ept = handles.read([&name](auto& hand) { return hand.getEndpoint(name); });
    if (ept != nullptr) {
        throw(RegistrationFailure("endpoint name is already used"));
    }
    const auto& handle = createBasicHandle(fed->global_id,
                                           fed->local_id,
                                           InterfaceType::ENDPOINT,
                                           name,
                                           type,
                                           std::string{},
                                           fed->getInterfaceFlags());

    auto id = handle.getInterfaceHandle();
    fed->createInterface(InterfaceType::ENDPOINT, id, name, type, emptyStr);
    ActionMessage m(CMD_REG_ENDPOINT);
    m.source_id = fed->global_id.load();
    m.source_handle = id;
    m.name(name);
    m.setStringData(type);
    m.flags = handle.flags;
    actionQueue.push(std::move(m));

    return id;
}

InterfaceHandle CommonCore::registerTargetedEndpoint(LocalFederateId federateID,
                                                     const std::string& name,
                                                     const std::string& type)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (registerEndpoint)"));
    }
    const auto* ept = handles.read([&name](auto& hand) { return hand.getEndpoint(name); });
    if (ept != nullptr) {
        throw(RegistrationFailure("endpoint name is already used"));
    }
    auto flags = fed->getInterfaceFlags();
    flags |= (1U << targeted_flag);
    const auto& handle = createBasicHandle(
        fed->global_id, fed->local_id, InterfaceType::ENDPOINT, name, type, std::string{}, flags);

    auto id = handle.getInterfaceHandle();
    fed->createInterface(InterfaceType::ENDPOINT, id, name, type, emptyStr);
    ActionMessage m(CMD_REG_ENDPOINT);
    m.source_id = fed->global_id.load();
    m.source_handle = id;
    m.name(name);
    m.setStringData(type);
    m.flags = handle.flags;
    actionQueue.push(std::move(m));

    return id;
}

InterfaceHandle CommonCore::getEndpoint(LocalFederateId federateID, const std::string& name) const
{
    const auto* ept = handles.read([&name](auto& hand) { return hand.getEndpoint(name); });
    if (ept->local_fed_id != federateID) {
        return {};
    }
    return ept->handle.handle;
}

InterfaceHandle CommonCore::registerFilter(const std::string& filterName,
                                           const std::string& type_in,
                                           const std::string& type_out)
{
    // check to make sure the name isn't already used
    if (!filterName.empty()) {
        if (handles.read([&filterName](auto& hand) {
                auto* res = hand.getFilter(filterName);
                return (res != nullptr);
            })) {
            throw(RegistrationFailure("there already exists a filter with this name"));
        }
    }
    if (!waitCoreRegistration()) {
        if (getBrokerState() >= BrokerState::connected_error) {
            throw(RegistrationFailure(
                "core is terminated or in error state no further registration possible"));
        }
        throw(RegistrationFailure("registration timeout exceeded"));
    }
    auto fid = filterFedID.load();

    const auto& handle = createBasicHandle(
        fid, LocalFederateId(), InterfaceType::FILTER, filterName, type_in, type_out);
    auto id = handle.getInterfaceHandle();

    ActionMessage m(CMD_REG_FILTER);
    m.source_id = fid;
    m.source_handle = id;
    m.name(handle.key);
    if ((!type_in.empty()) || (!type_out.empty())) {
        m.setStringData(type_in, type_out);
    }
    actionQueue.push(std::move(m));
    return id;
}

InterfaceHandle CommonCore::registerCloningFilter(const std::string& filterName,
                                                  const std::string& type_in,
                                                  const std::string& type_out)
{
    // check to make sure the name isn't already used
    if (!filterName.empty()) {
        if (handles.read([&filterName](auto& hand) {
                auto* res = hand.getFilter(filterName);
                return (res != nullptr);
            })) {
            throw(RegistrationFailure("there already exists a filter with this name"));
        }
    }
    if (!waitCoreRegistration()) {
        if (getBrokerState() >= BrokerState::terminating) {
            throw(RegistrationFailure("core is terminated no further registration possible"));
        }
        throw(RegistrationFailure("registration timeout exceeded"));
    }
    auto fid = filterFedID.load();

    const auto& handle = createBasicHandle(fid,
                                           LocalFederateId(),
                                           InterfaceType::FILTER,
                                           filterName,
                                           type_in,
                                           type_out,
                                           make_flags(clone_flag));

    auto id = handle.getInterfaceHandle();

    ActionMessage m(CMD_REG_FILTER);
    m.source_id = fid;
    m.source_handle = id;
    m.name(handle.key);
    setActionFlag(m, clone_flag);
    if ((!type_in.empty()) || (!type_out.empty())) {
        m.setStringData(type_in, type_out);
    }
    actionQueue.push(std::move(m));
    return id;
}

InterfaceHandle CommonCore::getFilter(const std::string& name) const
{
    const auto* filt = handles.read([&name](auto& hand) { return hand.getFilter(name); });
    if ((filt != nullptr) && (filt->handleType == InterfaceType::FILTER)) {
        return filt->getInterfaceHandle();
    }
    return {};
}

void CommonCore::makeConnections(const std::string& file)
{
    if (fileops::hasTomlExtension(file)) {
        fileops::makeConnectionsToml(this, file);
    } else {
        fileops::makeConnectionsJson(this, file);
    }
}

void CommonCore::linkEndpoints(const std::string& source, const std::string& dest)
{
    ActionMessage M(CMD_ENDPOINT_LINK);
    M.name(source);
    M.setStringData(dest);
    addActionMessage(std::move(M));
}

void CommonCore::dataLink(const std::string& source, const std::string& target)
{
    ActionMessage M(CMD_DATA_LINK);
    M.name(source);
    M.setStringData(target);
    addActionMessage(std::move(M));
}

void CommonCore::addSourceFilterToEndpoint(const std::string& filter, const std::string& endpoint)
{
    ActionMessage M(CMD_FILTER_LINK);
    M.name(filter);
    M.setStringData(endpoint);
    addActionMessage(std::move(M));
}

void CommonCore::addDestinationFilterToEndpoint(const std::string& filter,
                                                const std::string& endpoint)
{
    ActionMessage M(CMD_FILTER_LINK);
    M.name(filter);
    M.setStringData(endpoint);
    setActionFlag(M, destination_target);
    addActionMessage(std::move(M));
}

void CommonCore::addDependency(LocalFederateId federateID, const std::string& federateName)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (registerDependency)"));
    }
    ActionMessage search(CMD_SEARCH_DEPENDENCY);
    search.source_id = fed->global_id.load();
    search.name(federateName);
    addActionMessage(std::move(search));
}

void CommonCore::sendTo(InterfaceHandle sourceHandle,
                        const void* data,
                        uint64_t length,
                        std::string_view destination)
{
    if (destination.empty()) {
        // sendTo should be the equivalent of send if there is an empty destination
        send(sourceHandle, data, length);
        return;
    }
    const auto* hndl = getHandleInfo(sourceHandle);
    if (hndl == nullptr) {
        throw(InvalidIdentifier("handle is not valid"));
    }

    if (hndl->handleType != InterfaceType::ENDPOINT) {
        throw(InvalidIdentifier("handle does not point to an endpoint"));
    }
    auto* fed = getFederateAt(hndl->local_fed_id);
    if (checkActionFlag(*hndl, targeted_flag)) {
        auto targets = fed->getMessageDestinations(sourceHandle);
        auto res = std::find_if(targets.begin(), targets.end(), [destination](const auto& val) {
            return (val.second == destination);
        });
        if (res == targets.end()) {
            throw(InvalidParameter("targeted endpoint destination not in target list"));
        }
    }
    ActionMessage m(CMD_SEND_MESSAGE);

    m.messageID = ++messageCounter;
    m.source_handle = sourceHandle;
    m.source_id = hndl->getFederateId();
    m.flags = hndl->flags;
    m.payload.assign(data, length);
    m.setStringData(destination, hndl->key, hndl->key);
    m.actionTime = fed->nextAllowedSendTime();
    addActionMessage(std::move(m));
}

void CommonCore::sendToAt(InterfaceHandle sourceHandle,
                          const void* data,
                          uint64_t length,
                          std::string_view destination,
                          Time sendTime)
{
    if (destination.empty()) {
        // sendToAt should be the equivalent of sendAt if there is an empty destination
        sendAt(sourceHandle, data, length, sendTime);
        return;
    }
    const auto* hndl = getHandleInfo(sourceHandle);
    if (hndl == nullptr) {
        throw(InvalidIdentifier("handle is not valid"));
    }

    if (hndl->handleType != InterfaceType::ENDPOINT) {
        throw(InvalidIdentifier("handle does not point to an endpoint"));
    }
    auto* fed = getFederateAt(hndl->local_fed_id);
    if (checkActionFlag(*hndl, targeted_flag)) {
        auto targets = fed->getMessageDestinations(sourceHandle);
        auto res = std::find_if(targets.begin(), targets.end(), [destination](const auto& val) {
            return (val.second == destination);
        });
        if (res == targets.end()) {
            throw(InvalidParameter("targeted endpoint destination not in target list"));
        }
    }

    ActionMessage m(CMD_SEND_MESSAGE);

    m.messageID = ++messageCounter;
    m.source_handle = sourceHandle;
    m.source_id = hndl->getFederateId();
    auto minTime = fed->nextAllowedSendTime();
    m.actionTime = std::max(sendTime, minTime);
    m.flags = hndl->flags;

    m.payload.assign(data, length);
    m.setStringData(destination, hndl->key, hndl->key);

    addActionMessage(std::move(m));
}

void CommonCore::generateMessages(
    ActionMessage& message,
    const std::vector<std::pair<GlobalHandle, std::string_view>>& targets)
{
    setActionFlag(message, filter_processing_required_flag);
    if (targets.size() == 1) {
        message.setDestination(targets.front().first);
        message.setString(0, targets.front().second);
        actionQueue.push(std::move(message));
        return;
    }
    /** now generate a multimessage*/
    ActionMessage package(CMD_MULTI_MESSAGE);
    package.source_id = message.source_id;
    package.source_handle = message.source_handle;

    for (const auto& target : targets) {
        message.setDestination(target.first);
        message.setString(0, target.second);
        auto res = appendMessage(package, message);
        if (res < 0)  // deal with max package size if there are a lot of subscribers
        {
            actionQueue.push(std::move(package));
            package = ActionMessage(CMD_MULTI_MESSAGE);
            package.source_id = message.source_id;
            package.source_handle = message.source_handle;
            appendMessage(package, message);
        }
    }
    actionQueue.push(std::move(package));
}

void CommonCore::send(InterfaceHandle sourceHandle, const void* data, uint64_t length)
{
    const auto* hndl = getHandleInfo(sourceHandle);
    if (hndl == nullptr) {
        throw(InvalidIdentifier("handle is not valid"));
    }
    if (hndl->handleType != InterfaceType::ENDPOINT) {
        throw(InvalidIdentifier("handle does not point to an endpoint"));
    }
    auto* fed = getFederateAt(hndl->local_fed_id);
    auto targets = fed->getMessageDestinations(sourceHandle);
    if (targets.empty()) {
        return;
    }

    ActionMessage m(CMD_SEND_MESSAGE);
    m.source_handle = sourceHandle;
    m.source_id = hndl->getFederateId();
    m.actionTime = fed->nextAllowedSendTime();
    m.payload.assign(data, length);
    m.messageID = ++messageCounter;
    m.setStringData("", hndl->key, hndl->key);
    generateMessages(m, targets);
}

void CommonCore::sendAt(InterfaceHandle sourceHandle, const void* data, uint64_t length, Time time)
{
    const auto* hndl = getHandleInfo(sourceHandle);
    if (hndl == nullptr) {
        throw(InvalidIdentifier("handle is not valid"));
    }
    if (hndl->handleType != InterfaceType::ENDPOINT) {
        throw(InvalidIdentifier("handle does not point to an endpoint"));
    }
    auto* fed = getFederateAt(hndl->local_fed_id);
    auto targets = fed->getMessageDestinations(sourceHandle);
    if (targets.empty()) {
        return;
    }

    ActionMessage m(CMD_SEND_MESSAGE);
    m.source_handle = sourceHandle;
    m.source_id = hndl->getFederateId();
    auto minTime = fed->nextAllowedSendTime();
    m.actionTime = std::max(time, minTime);
    m.payload.assign(data, length);
    m.messageID = ++messageCounter;
    m.setStringData("", hndl->key, hndl->key);
    generateMessages(m, targets);
}

void CommonCore::sendMessage(InterfaceHandle sourceHandle, std::unique_ptr<Message> message)
{
    if (sourceHandle == gDirectSendHandle) {
        if (!waitCoreRegistration()) {
            throw(FunctionExecutionFailure(
                "core is unable to register and has timed out, message was not sent"));
        }
        ActionMessage m(std::move(message));
        m.source_id = global_id.load();
        m.source_handle = sourceHandle;
        addActionMessage(std::move(m));
        return;
    }
    const auto* hndl = getHandleInfo(sourceHandle);
    if (hndl == nullptr) {
        throw(InvalidIdentifier("handle is not valid"));
    }
    if (hndl->handleType != InterfaceType::ENDPOINT) {
        throw(InvalidIdentifier("handle does not point to an endpoint"));
    }
    ActionMessage m(std::move(message));

    m.setString(sourceStringLoc, hndl->key);
    m.source_id = hndl->getFederateId();
    m.source_handle = sourceHandle;
    if (m.messageID == 0) {
        m.messageID = ++messageCounter;
    }
    auto* fed = getFederateAt(hndl->local_fed_id);
    auto minTime = fed->nextAllowedSendTime();
    if (m.actionTime < minTime) {
        m.actionTime = minTime;
    }

    if (fed->loggingLevel() >= HELICS_LOG_LEVEL_DATA) {
        fed->logMessage(HELICS_LOG_LEVEL_DATA,
                        "",
                        fmt::format("receive_message {}", prettyPrintString(m)));
    }
    if (m.getString(targetStringLoc).empty()) {
        if (checkActionFlag(*hndl, targeted_flag)) {
            auto targets = fed->getMessageDestinations(sourceHandle);
            if (targets.empty()) {
                return;
            }
            generateMessages(m, targets);
        } else {
            throw(InvalidParameter("no destination specified in message"));
        }
    } else {
        if (checkActionFlag(*hndl, targeted_flag)) {
            auto targets = fed->getMessageDestinations(sourceHandle);
            auto res = std::find_if(targets.begin(),
                                    targets.end(),
                                    [destination = m.getString(targetStringLoc)](const auto& val) {
                                        return (val.second == destination);
                                    });
            if (res == targets.end()) {
                throw(InvalidParameter("targeted endpoint destination not in target list"));
            }
        }
        addActionMessage(std::move(m));
    }
}

void CommonCore::deliverMessage(ActionMessage& message)
{
    switch (message.action()) {
        case CMD_SEND_MESSAGE: {
            // Find the destination endpoint
            auto* localP = (message.dest_id == parent_broker_id) ?
                loopHandles.getEndpoint(message.getString(targetStringLoc)) :
                loopHandles.findHandle(message.getDest());
            if (localP == nullptr) {
                auto kfnd = knownExternalEndpoints.find(message.getString(targetStringLoc));
                if (kfnd != knownExternalEndpoints.end()) {  // destination is known
                    transmit(kfnd->second, message);
                } else {
                    transmit(parent_route_id, message);
                }
                return;
            }
            // now we deal with local processing
            if (checkActionFlag(*localP, has_dest_filter_flag)) {
                if (!filterFed->destinationProcessMessage(message, localP)) {
                    return;
                }
            }
            if (message.dest_id == parent_broker_id) {
                message.dest_id = localP->getFederateId();
                message.dest_handle = localP->getInterfaceHandle();
            }

            auto* fed = getFederateCore(localP->getFederateId());
            if (fed != nullptr) {
                fed->addAction(std::move(message));
            }
        } break;
        case CMD_SEND_FOR_FILTER:
        case CMD_SEND_FOR_FILTER_AND_RETURN:
        case CMD_SEND_FOR_DEST_FILTER_AND_RETURN:
        case CMD_FILTER_RESULT:
        case CMD_DEST_FILTER_RESULT:
        case CMD_NULL_MESSAGE:
        case CMD_NULL_DEST_MESSAGE:
        default: {
            transmit(getRoute(message.dest_id), message);
        } break;
    }
}

uint64_t CommonCore::receiveCount(InterfaceHandle destination)
{
    auto* fed = getHandleFederate(destination);
    if (fed == nullptr) {
        return 0;
    }
    return fed->getQueueSize(destination);
}

std::unique_ptr<Message> CommonCore::receive(InterfaceHandle destination)
{
    auto* fed = getHandleFederate(destination);
    if (fed == nullptr) {
        throw(InvalidIdentifier("invalid handle"));
    }
    if (fed->getState() != HELICS_EXECUTING) {
        return nullptr;
    }

    return fed->receive(destination);
}

std::unique_ptr<Message> CommonCore::receiveAny(LocalFederateId federateID,
                                                InterfaceHandle& endpoint_id)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("FederateID is not valid (receiveAny)"));
    }
    if (fed->getState() == HELICS_CREATED) {
        endpoint_id = InterfaceHandle();
        return nullptr;
    }
    return fed->receiveAny(endpoint_id);
}

uint64_t CommonCore::receiveCountAny(LocalFederateId federateID)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("FederateID is not valid (receiveCountAny)"));
    }
    if (fed->getState() == HELICS_CREATED) {
        return 0;
    }

    return fed->getQueueSize();
}

void CommonCore::logMessage(LocalFederateId federateID,
                            int logLevel,
                            const std::string& messageToLog)
{
    GlobalFederateId gid;
    if (federateID == gLocalCoreId) {
        gid = global_id.load();
    } else {
        auto* fed = getFederateAt(federateID);
        if (fed == nullptr) {
            throw(InvalidIdentifier("FederateID is not valid (logMessage)"));
        }
        gid = fed->global_id;
    }
    ActionMessage m(CMD_LOG);
    m.source_id = gid;
    m.dest_id = gid;
    m.messageID = logLevel;
    m.payload = messageToLog;
    actionQueue.push(m);
}

void CommonCore::setLoggingLevel(int logLevel)
{
    ActionMessage cmd(CMD_CORE_CONFIGURE);
    cmd.dest_id = global_id.load();
    cmd.messageID = defs::Properties::LOG_LEVEL;
    cmd.setExtraData(logLevel);
    addActionMessage(cmd);
}

void CommonCore::setLogFile(const std::string& lfile)
{
    ActionMessage cmd(CMD_CORE_CONFIGURE);
    cmd.dest_id = global_id.load();
    cmd.messageID = UPDATE_LOGGING_FILE;
    cmd.payload = lfile;
    addActionMessage(cmd);
}

std::pair<std::string, std::string> CommonCore::getCommand(LocalFederateId federateID)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("FederateID is not valid (setLoggingCallback)"));
    }
    return fed->getCommand();
}

std::pair<std::string, std::string> CommonCore::waitCommand(LocalFederateId federateID)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("FederateID is not valid (setLoggingCallback)"));
    }
    return fed->waitCommand();
}

void CommonCore::setLoggingCallback(
    LocalFederateId federateID,
    std::function<void(int, std::string_view, std::string_view)> logFunction)
{
    if (federateID == gLocalCoreId) {
        ActionMessage loggerUpdate(CMD_CORE_CONFIGURE);
        loggerUpdate.messageID = UPDATE_LOGGING_CALLBACK;
        loggerUpdate.source_id = global_id.load();
        loggerUpdate.dest_id = global_id.load();
        if (logFunction) {
            auto ii = getNextAirlockIndex();
            dataAirlocks[ii].load(std::move(logFunction));
            loggerUpdate.counter = ii;
        } else {
            setActionFlag(loggerUpdate, empty_flag);
        }

        actionQueue.push(loggerUpdate);
    } else {
        auto* fed = getFederateAt(federateID);
        if (fed == nullptr) {
            throw(InvalidIdentifier("FederateID is not valid (setLoggingCallback)"));
        }
        fed->setLogger(std::move(logFunction));
    }
}

uint16_t CommonCore::getNextAirlockIndex()
{
    uint16_t index = nextAirLock++;
    if (index > 3) {  // the increment is an atomic operation if the nextAirLock was not adjusted
                      // this could result in an out of
        // bounds exception if this check were not done
        index %= 4;
    }
    if (index == 3) {
        decltype(index) exp = 4;

        while (exp > 3) {  // doing a lock free modulus we need to make sure the nextAirLock<4
            if (nextAirLock.compare_exchange_weak(exp, exp % 4)) {
                break;
            }
        }
    }
    return index;
}

void CommonCore::setFilterOperator(InterfaceHandle filter, std::shared_ptr<FilterOperator> callback)
{
    static std::shared_ptr<FilterOperator> nullFilt = std::make_shared<NullFilterOperator>();
    const auto* hndl = getHandleInfo(filter);
    if (hndl == nullptr) {
        throw(InvalidIdentifier("filter is not a valid handle"));
    }
    if ((hndl->handleType != InterfaceType::FILTER)) {
        throw(InvalidIdentifier("filter identifier does not point a filter"));
    }
    ActionMessage filtOpUpdate(CMD_CORE_CONFIGURE);
    filtOpUpdate.messageID = UPDATE_FILTER_OPERATOR;
    if (!callback) {
        callback = nullFilt;
    }
    auto ii = getNextAirlockIndex();
    dataAirlocks[ii].load(std::move(callback));
    filtOpUpdate.counter = ii;
    filtOpUpdate.source_id = hndl->getFederateId();
    filtOpUpdate.source_handle = filter;
    actionQueue.push(filtOpUpdate);
}

void CommonCore::setIdentifier(const std::string& name)
{
    if (getBrokerState() == BrokerState::created) {
        identifier = name;
    } else {
        throw(
            InvalidFunctionCall("setIdentifier can only be called before the core is initialized"));
    }
}

static const std::map<std::string, std::pair<std::uint16_t, bool>> mapIndex{
    {"global_time", {CURRENT_TIME_MAP, true}},
    {"global_status", {GLOBAL_STATUS, false}},
    {"dependency_graph", {DEPENDENCY_GRAPH, false}},
    {"data_flow_graph", {DATA_FLOW_GRAPH, false}},
    {"global_state", {GLOBAL_STATE, true}},
    {"global_time_debugging", {GLOBAL_TIME_DEBUGGING, true}},
    {"global_flush", {GLOBAL_FLUSH, true}},
};

void CommonCore::setQueryCallback(LocalFederateId federateID,
                                  std::function<std::string(std::string_view)> queryFunction)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("FederateID is invalid (setQueryCallback)"));
    }
    fed->setQueryCallback(std::move(queryFunction));
}

std::string CommonCore::filteredEndpointQuery(const FederateState* fed) const
{
    Json::Value base;
    if (fed != nullptr) {
        base["name"] = fed->getIdentifier();
        base["id"] = fed->global_id.load().baseValue();
        if (filterFed != nullptr) {
            filterFed->addFilteredEndpoint(base, fed->global_id);
        }
    } else {
        base["name"] = getIdentifier();
        base["id"] = global_broker_id_local.baseValue();
        base["endpoints"] = Json::arrayValue;
    }
    return fileops::generateJsonString(base);
}

std::string CommonCore::federateQuery(const FederateState* fed,
                                      const std::string& queryStr,
                                      bool force_ordering) const
{
    if (fed == nullptr) {
        if (queryStr == "exists") {
            return "false";
        }
        return generateJsonErrorResponse(JsonErrorCodes::NOT_FOUND, "Federate not found");
    }
    if (queryStr == "exists") {
        return "true";
    }
    if (queryStr == "version") {
        return std::string{"\""} + versionString + '"';
    }
    if (queryStr == "isinit") {
        return (fed->init_transmitted.load()) ? "true" : "false";
    }
    if (queryStr == "state") {
        if (!force_ordering) {
            return fmt::format("\"{}\"", fedStateString(fed->getState()));
        }
    }
    if (queryStr == "filtered_endpoints") {
        if (!force_ordering) {
            return filteredEndpointQuery(fed);
        }
    }
    auto resultString = generateInterfaceQueryResults(queryStr,
                                                      loopHandles,
                                                      fed->global_id,
                                                      [](Json::Value& /*unused*/) {});
    if (!resultString.empty()) {
        return resultString;
    }
    if (queryStr == "interfaces") {
        auto jv = generateInterfaceConfig(loopHandles, fed->global_id);
        jv["name"] = fed->getIdentifier();
        return fileops::generateJsonString(jv);
    }
    if ((queryStr == "queries") || (queryStr == "available_queries")) {
        return std::string(
                   R"(["exists","isinit","global_state","version","state","queries","interfaces","filtered_endpoints",)") +
            fed->processQuery(queryStr) + "]";
    }
    return fed->processQuery(queryStr, force_ordering);
}

static const std::set<std::string> querySet{"isinit",
                                            "isconnected",
                                            "exists",
                                            "name",
                                            "identifier",
                                            "address",
                                            "queries",
                                            "address",
                                            "federates",
                                            "inputs",
                                            "input_details",
                                            "endpoints",
                                            "endpoint_details",
                                            "filtered_endpoints",
                                            "publications",
                                            "publication_details",
                                            "filters",
                                            "filter_details",
                                            "interface_details",
                                            "tags",
                                            "version",
                                            "version_all",
                                            "federate_map",
                                            "dependency_graph",
                                            "data_flow_graph",
                                            "dependencies",
                                            "dependson",
                                            "logs",
                                            "dependents",
                                            "current_time",
                                            "global_time",
                                            "global_state",
                                            "global_flush",
                                            "current_state",
                                            "logs"};

std::string CommonCore::quickCoreQueries(const std::string& queryStr) const
{
    if ((queryStr == "queries") || (queryStr == "available_queries")) {
        return generateStringVector(querySet, [](const std::string& data) { return data; });
    }
    if (queryStr == "isconnected") {
        return (isConnected()) ? "true" : "false";
    }
    if (queryStr == "name" || queryStr == "identifier") {
        return std::string{"\""} + getIdentifier() + '"';
    }
    if (queryStr == "exists") {
        return "true";
    }
    if (queryStr == "version") {
        return std::string{"\""} + versionString + '"';
    }
    return std::string{};
}

void CommonCore::loadBasicJsonInfo(
    Json::Value& base,
    const std::function<void(Json::Value& fedval, const FedInfo& fed)>& fedLoader) const
{
    base["name"] = getIdentifier();
    base["id"] = global_broker_id_local.baseValue();
    base["parent"] = higher_broker_id.baseValue();
    if (fedLoader) {
        base["federates"] = Json::arrayValue;
        for (const auto& fed : loopFederates) {
            Json::Value fedval;
            fedval["id"] = fed.fed->global_id.load().baseValue();
            fedval["name"] = fed.fed->getIdentifier();
            fedval["parent"] = global_broker_id_local.baseValue();
            fedLoader(fedval, fed);
            base["federates"].append(std::move(fedval));
        }
    }
}

void CommonCore::initializeMapBuilder(const std::string& request,
                                      std::uint16_t index,
                                      bool reset,
                                      bool force_ordering) const
{
    if (!isValidIndex(index, mapBuilders)) {
        mapBuilders.resize(static_cast<size_t>(index) + 1);
    }
    std::get<2>(mapBuilders[index]) = reset;
    auto& builder = std::get<0>(mapBuilders[index]);
    builder.reset();
    Json::Value& base = builder.getJValue();
    base["name"] = getIdentifier();
    base["id"] = global_broker_id_local.baseValue();
    base["parent"] = higher_broker_id.baseValue();
    ActionMessage queryReq(force_ordering ? CMD_QUERY_ORDERED : CMD_QUERY);
    if (index == GLOBAL_FLUSH) {
        queryReq.setAction(CMD_QUERY_ORDERED);
    }
    queryReq.payload = request;
    queryReq.source_id = global_broker_id_local;
    queryReq.counter = index;  // indicating which processing to use
    if (loopFederates.size() > 0 || filterFed != nullptr) {
        base["federates"] = Json::arrayValue;
        for (const auto& fed : loopFederates) {
            int brkindex =
                builder.generatePlaceHolder("federates", fed->global_id.load().baseValue());
            std::string ret = federateQuery(fed.fed, request, force_ordering);
            if (ret == "#wait") {
                if (fed->getState() <= FederateStates::HELICS_EXECUTING) {
                    queryReq.messageID = brkindex;
                    queryReq.dest_id = fed.fed->global_id;
                    fed.fed->addAction(queryReq);
                } else {
                    // the federate has terminated or errored so is waiting, global_state doesn't
                    // block
                    builder.addComponent(federateQuery(fed.fed, "global_state", force_ordering),
                                         brkindex);
                }
            } else {
                builder.addComponent(ret, brkindex);
            }
        }
        if (filterFed != nullptr) {
            int brkindex = builder.generatePlaceHolder("federates", filterFedID.load().baseValue());
            std::string ret = filterFed->query(request);
            builder.addComponent(ret, brkindex);
        }
    }

    switch (index) {
        case CURRENT_TIME_MAP:
        case GLOBAL_STATUS:
            if (hasTimeDependency) {
                base["next_time"] = static_cast<double>(timeCoord->getNextTime());
            }
            break;
        case DEPENDENCY_GRAPH: {
            if (hasTimeDependency) {
                base["dependents"] = Json::arrayValue;
                for (const auto& dep : timeCoord->getDependents()) {
                    base["dependents"].append(dep.baseValue());
                }
                base["dependencies"] = Json::arrayValue;
                for (const auto& dep : timeCoord->getDependencies()) {
                    base["dependencies"].append(dep.baseValue());
                }
            }
        } break;
        case GLOBAL_STATE:
            base["state"] = brokerStateName(getBrokerState());
            break;
        case GLOBAL_TIME_DEBUGGING:
            base["state"] = brokerStateName(getBrokerState());
            if (timeCoord && !timeCoord->empty()) {
                base["time"] = Json::Value();
                timeCoord->generateDebuggingTimeInfo(base["time"]);
            }
            break;
        default:
            break;
    }
}

void CommonCore::processCommandInstruction(ActionMessage& command)
{
    auto [processed, res] = processBaseCommands(command);
    if (processed) {
        return;
    }
    auto warnString = fmt::format("Unrecognized command instruction \"{}\"", res[0]);
    LOG_WARNING(global_broker_id_local, getIdentifier(), warnString);
    if (command.source_id != global_broker_id_local) {
        ActionMessage warn(CMD_WARNING, global_broker_id_local, command.source_id);
        warn.payload = std::move(warnString);
        warn.messageID = HELICS_LOG_LEVEL_WARNING;
        warn.setString(0, getIdentifier());
        routeMessage(warn);
    }
}

std::string CommonCore::coreQuery(const std::string& queryStr, bool force_ordering) const
{
    auto addHeader = [this](Json::Value& base) { loadBasicJsonInfo(base, nullptr); };

    auto res = quickCoreQueries(queryStr);
    if (!res.empty()) {
        return res;
    }
    if (queryStr == "federates") {
        return generateStringVector(loopFederates,
                                    [](const auto& fed) { return fed->getIdentifier(); });
    }

    if (queryStr == "tags") {
        Json::Value tagBlock = Json::objectValue;
        for (const auto& tg : tags) {
            tagBlock[tg.first] = tg.second;
        }
        return fileops::generateJsonString(tagBlock);
    }
    if (queryStr.compare(0, 4, "tag/") == 0) {
        std::string_view tag = queryStr;
        tag.remove_prefix(4);
        for (const auto& tg : tags) {
            if (tag == tg.first) {
                return Json::valueToQuotedString(tg.second.c_str());
            }
        }
        return "\"\"";
    }
    auto interfaceQueryResult =
        generateInterfaceQueryResults(queryStr, loopHandles, GlobalFederateId{}, addHeader);
    if (!interfaceQueryResult.empty()) {
        return interfaceQueryResult;
    }

    if (queryStr == "dependson") {
        return generateStringVector(timeCoord->getDependencies(),
                                    [](auto& dep) { return std::to_string(dep.baseValue()); });
    }
    if (queryStr == "dependents") {
        return generateStringVector(timeCoord->getDependents(),
                                    [](auto& dep) { return std::to_string(dep.baseValue()); });
    }

    if (queryStr == "isinit") {
        return (allInitReady()) ? "true" : "false";
    }
    if (queryStr == "logs") {
        Json::Value base;
        loadBasicJsonInfo(base, nullptr);
        bufferToJson(mLogManager->getLogBuffer(), base);
        return fileops::generateJsonString(base);
    }
    if (queryStr == "address") {
        return Json::valueToQuotedString(getAddress().c_str());
    }
    if (queryStr == "counter") {
        return fmt::format("{}", generateMapObjectCounter());
    }
    if (queryStr == "filtered_endpoints") {
        return filteredEndpointQuery(nullptr);
    }
    if (queryStr == "current_time") {
        if (!hasTimeDependency) {
            return "{}";
        }
        return timeCoord->printTimeStatus();
    }
    if (queryStr == "version_all") {
        Json::Value base;
        loadBasicJsonInfo(base, [](Json::Value& /*val*/, const FedInfo& /*fed*/) {});
        base["version"] = versionString;
        return fileops::generateJsonString(base);
    }
    if (queryStr == "current_state") {
        Json::Value base;
        loadBasicJsonInfo(base, [](Json::Value& val, const FedInfo& fed) {
            val["state"] = state_string(fed.state);
        });
        base["state"] = brokerStateName(getBrokerState());

        return fileops::generateJsonString(base);
    }
    if (queryStr == "interfaces") {
        Json::Value base;
        loadBasicJsonInfo(base, [this](Json::Value& val, const FedInfo& fed) {
            generateInterfaceConfig(val, loopHandles, fed->global_id);
        });
        return fileops::generateJsonString(base);
    }
    auto mi = mapIndex.find(queryStr);
    if (mi != mapIndex.end()) {
        auto index = mi->second.first;
        if (isValidIndex(index, mapBuilders) && !mi->second.second) {
            auto& builder = std::get<0>(mapBuilders[index]);
            if (builder.isCompleted()) {
                auto center = generateMapObjectCounter();
                if (center == builder.getCounterCode()) {
                    return builder.generate();
                }
                builder.reset();
            }
            if (builder.isActive()) {
                return "#wait";
            }
        }

        initializeMapBuilder(queryStr, index, mi->second.second, force_ordering);
        if (std::get<0>(mapBuilders[index]).isCompleted()) {
            if (!mi->second.second) {
                auto center = generateMapObjectCounter();
                std::get<0>(mapBuilders[index]).setCounterCode(center);
            }
            return std::get<0>(mapBuilders[index]).generate();
        }
        return "#wait";
    }
    if (queryStr == "dependencies") {
        Json::Value base;
        loadBasicJsonInfo(base, nullptr);
        base["dependents"] = Json::arrayValue;
        for (const auto& dep : timeCoord->getDependents()) {
            base["dependents"].append(dep.baseValue());
        }
        base["dependencies"] = Json::arrayValue;
        for (const auto& dep : timeCoord->getDependencies()) {
            base["dependencies"].append(dep.baseValue());
        }
        return fileops::generateJsonString(base);
    }
    if (queryStr == "federate_map") {
        Json::Value base;
        loadBasicJsonInfo(base, [](Json::Value& val, const FedInfo& fed) {
            if (fed.fed->try_lock()) {
                addFederateTags(val, fed.fed);
                fed.fed->unlock();
            } else {
                addFederateTags(val, fed.fed);
            }
        });
        // add core tags if needed
        if (!tags.empty()) {
            Json::Value tagBlock = Json::objectValue;
            for (const auto& tg : tags) {
                tagBlock[tg.first] = tg.second;
            }
            base["tags"] = tagBlock;
        }
        return fileops::generateJsonString(base);
    }
    // check tag value for a matching string
    for (const auto& tg : tags) {
        if (tg.first == queryStr) {
            return Json::valueToQuotedString(tg.second.c_str());
        }
    }
    // if nothing found generate an error response
    return generateJsonErrorResponse(JsonErrorCodes::BAD_REQUEST, "unrecognized core query");
}

std::string CommonCore::query(const std::string& target,
                              const std::string& queryStr,
                              HelicsSequencingModes mode)
{
    if (getBrokerState() >= BrokerState::terminating) {
        if (target == "core" || target == getIdentifier() || target.empty()) {
            auto res = quickCoreQueries(queryStr);
            if (!res.empty()) {
                return res;
            }
            if (queryStr == "logs") {
                Json::Value base;
                loadBasicJsonInfo(base, nullptr);
                bufferToJson(mLogManager->getLogBuffer(), base);
                return fileops::generateJsonString(base);
            }
        }
        return generateJsonErrorResponse(JsonErrorCodes::DISCONNECTED, "Core has terminated");
    }
    ActionMessage querycmd(mode == HELICS_SEQUENCING_MODE_FAST ? CMD_QUERY : CMD_QUERY_ORDERED);
    querycmd.source_id = gDirectCoreId;
    querycmd.dest_id = parent_broker_id;
    querycmd.payload = queryStr;
    auto index = ++queryCounter;
    querycmd.messageID = index;
    querycmd.setStringData(target);

    if (target == "core" || target == getIdentifier() || target.empty()) {
        auto res = quickCoreQueries(queryStr);
        if (!res.empty()) {
            return res;
        }
        if (queryStr == "address") {
            res = generateJsonQuotedString(getAddress());
            return res;
        }
        querycmd.setAction(mode == HELICS_SEQUENCING_MODE_FAST ? CMD_BROKER_QUERY :
                                                                 CMD_BROKER_QUERY_ORDERED);
        querycmd.dest_id = gDirectCoreId;
    }
    if (querycmd.dest_id != gDirectCoreId) {
        // default into a federate query
        auto* fed =
            (target != "federate") ? getFederate(target) : getFederateAt(LocalFederateId(0));
        if (fed != nullptr) {
            querycmd.dest_id = fed->global_id;
            if (mode != HELICS_SEQUENCING_MODE_ORDERED) {
                std::string ret = federateQuery(fed, queryStr, false);
                if (ret != "#wait") {
                    return ret;
                }

                auto queryResult = activeQueries.getFuture(querycmd.messageID);
                fed->addAction(std::move(querycmd));
                std::future_status status = std::future_status::timeout;
                while (status == std::future_status::timeout) {
                    status = queryResult.wait_for(std::chrono::milliseconds(50));
                    switch (status) {
                        case std::future_status::ready:
                        case std::future_status::deferred: {
                            auto qres = queryResult.get();
                            activeQueries.finishedWithValue(index);
                            return qres;
                        }
                        case std::future_status::timeout: {  // federate query may need to wait or
                                                             // can get the result now
                            ret = federateQuery(fed,
                                                queryStr,
                                                mode == HELICS_SEQUENCING_MODE_ORDERED);
                            if (ret != "#wait") {
                                activeQueries.finishedWithValue(index);
                                return ret;
                            }
                        } break;
                        default:
                            status = std::future_status::ready;  // LCOV_EXCL_LINE
                    }
                }
                return generateJsonErrorResponse(JsonErrorCodes::INTERNAL_ERROR,
                                                 "Unexpected Error #13");  // LCOV_EXCL_LINE
            }
        }
    }

    auto queryResult = activeQueries.getFuture(querycmd.messageID);
    addActionMessage(std::move(querycmd));
    auto ret = queryResult.get();
    activeQueries.finishedWithValue(index);
    return ret;
}

void CommonCore::setGlobal(const std::string& valueName, const std::string& value)
{
    ActionMessage querycmd(CMD_SET_GLOBAL);
    querycmd.dest_id = gRootBrokerID;
    querycmd.source_id = gDirectCoreId;
    querycmd.payload = valueName;
    querycmd.setStringData(value);
    addActionMessage(std::move(querycmd));
}

void CommonCore::sendCommand(const std::string& target,
                             const std::string& commandStr,
                             const std::string& source,
                             HelicsSequencingModes mode)
{
    if (commandStr == "flush") {
        query(target, "global_flush", HelicsSequencingModes::HELICS_SEQUENCING_MODE_ORDERED);
        return;
    }
    ActionMessage cmdcmd(mode == HelicsSequencingModes::HELICS_SEQUENCING_MODE_ORDERED ?
                             CMD_SEND_COMMAND_ORDERED :
                             CMD_SEND_COMMAND);
    cmdcmd.dest_id = parent_broker_id;
    cmdcmd.payload = commandStr;
    cmdcmd.setString(targetStringLoc, target);
    if (!source.empty()) {
        cmdcmd.setString(sourceStringLoc, source);
        const auto* fed = getFederate(source);
        if (fed != nullptr) {
            cmdcmd.source_id = fed->global_id;
        }
    } else {
        cmdcmd.setString(sourceStringLoc, getIdentifier());
        cmdcmd.source_id = getGlobalId();
    }
    addActionMessage(std::move(cmdcmd));
}

void CommonCore::processPriorityCommand(ActionMessage&& command)
{
    // deal with a few types of message immediately
    LOG_TRACE(global_broker_id_local,
              getIdentifier(),
              fmt::format("|| priority_cmd:{} from {}",
                          prettyPrintString(command),
                          command.source_id.baseValue()));
    switch (command.action()) {
        case CMD_PING_PRIORITY:
            if (command.dest_id == global_broker_id_local) {
                ActionMessage pngrep(CMD_PING_REPLY);
                pngrep.dest_id = command.source_id;
                pngrep.source_id = global_broker_id_local;
                routeMessage(pngrep);
            }
            break;
        case CMD_REG_FED:
            // this one in the core needs to be the thread-safe version of getFederate
            loopFederates.insert(std::string(command.name()),
                                 no_search,
                                 getFederate(std::string(command.name())));
            if (global_broker_id_local != parent_broker_id) {
                // forward on to Broker
                command.source_id = global_broker_id_local;
                transmit(parent_route_id, std::move(command));
            } else {
                // this will get processed when this core is assigned a global id
                delayTransmitQueue.push(std::move(command));
            }
            break;
        case CMD_BROKER_LOCATION: {
            command.setAction(CMD_PROTOCOL);
            command.messageID = NEW_BROKER_INFORMATION;
            transmit(control_route, std::move(command));
            ActionMessage resend(CMD_RESEND);
            resend.messageID = static_cast<int32_t>(CMD_REG_BROKER);
            addActionMessage(resend);
        } break;
        case CMD_REG_BROKER:
            // These really shouldn't happen here probably means something went wrong in setup but
            // we can handle it forward the connection request to the higher level
            if (command.name() == identifier) {
                LOG_ERROR(
                    global_broker_id_local,
                    identifier,
                    "received locally sent registration message, broker loop, please set the broker address to "
                    "a valid broker");
            } else {
                LOG_WARNING(parent_broker_id,
                            identifier,
                            "Core received reg broker message, likely improper federation setup\n");
                transmit(parent_route_id, command);
            }
            break;
        case CMD_BROKER_ACK:
            if (command.name() == identifier) {
                if (checkActionFlag(command, error_flag)) {
                    auto estring =
                        std::string("broker responded with error: ") + errorMessageString(command);
                    setErrorState(command.messageID, estring);
                    errorRespondDelayedMessages(estring);
                    LOG_ERROR(parent_broker_id, identifier, estring);
                    break;
                }
                global_id = GlobalBrokerId(command.dest_id);
                global_broker_id_local = GlobalBrokerId(command.dest_id);
                filterFedID = getSpecialFederateId(global_broker_id_local, 0);
                timeCoord->source_id = global_broker_id_local;
                higher_broker_id = GlobalBrokerId(command.source_id);
                transmitDelayedMessages();
                timeoutMon->setParentId(higher_broker_id);
                if (checkActionFlag(command, slow_responding_flag)) {
                    timeoutMon->disableParentPing();
                }
                timeoutMon->reset();
                if (delayInitCounter < 0 && minFederateCount == 0 && minChildCount == 0) {
                    if (allInitReady()) {
                        if (transitionBrokerState(BrokerState::connected,
                                                  BrokerState::initializing)) {
                            // make sure we only do this once
                            ActionMessage init(CMD_INIT);
                            checkDependencies();
                            init.source_id = global_broker_id_local;
                            init.dest_id = parent_broker_id;
                            transmit(parent_route_id, init);
                        }
                    }
                }
            }
            break;
        case CMD_FED_ACK: {
            auto* fed = getFederateCore(std::string(command.name()));
            if (fed != nullptr) {
                if (checkActionFlag(command, error_flag)) {
                    LOG_ERROR(
                        parent_broker_id,
                        identifier,
                        fmt::format("broker responded with error for registration of {}::{}\n",
                                    command.name(),
                                    commandErrorString(command.messageID)));
                } else {
                    fed->global_id = command.dest_id;
                    loopFederates.addSearchTerm(command.dest_id, std::string(command.name()));
                    if (!keyFed.isValid()) {
                        keyFed = fed->global_id;
                    }
                }

                // push the command to the local queue
                fed->addAction(std::move(command));
            }
        } break;
        case CMD_REG_ROUTE:
            // TODO(PT): double check this
            addRoute(route_id(command.getExtraData()), 0, std::string(command.payload.to_string()));
            break;
        case CMD_PRIORITY_DISCONNECT:
            checkAndProcessDisconnect();
            break;
        case CMD_SEND_COMMAND:
            if (command.dest_id == global_broker_id_local) {
                processCommandInstruction(command);
                break;
            }
            if (command.dest_id == parent_broker_id) {
                const auto& target = command.getString(targetStringLoc);
                if (target == "core" || target == getIdentifier()) {
                    processCommandInstruction(command);
                    break;
                }
                auto* fed = getFederateCore(target);
                if (fed != nullptr) {
                    fed->sendCommand(command);
                    break;
                }
            }
            if (isLocal(command.dest_id)) {
                auto* fed = getFederateCore(command.dest_id);
                if (fed != nullptr) {
                    fed->sendCommand(command);
                    break;
                }
            }
            routeMessage(std::move(command));
            break;
        case CMD_BROKER_QUERY:
        case CMD_QUERY:
        case CMD_QUERY_REPLY:
            processQueryCommand(command);
            break;
        case CMD_PRIORITY_ACK:
        case CMD_ROUTE_ACK:
            break;
        case CMD_SET_GLOBAL:
            if (global_broker_id_local != parent_broker_id) {
                // forward on to Broker
                command.source_id = global_broker_id_local;
                transmit(parent_route_id, std::move(command));
            } else {
                // this will get processed when this core is assigned a global id
                delayTransmitQueue.push(std::move(command));
            }
            break;
        default: {
            if (!isPriorityCommand(command)) {
                processCommand(std::move(command));
            }
        }
    }
}

void CommonCore::transmitDelayedMessages()
{
    auto msg = delayTransmitQueue.pop();
    while (msg) {
        if (msg->source_id == parent_broker_id || msg->source_id == gDirectCoreId) {
            msg->source_id = global_broker_id_local;
        }
        routeMessage(*msg);
        msg = delayTransmitQueue.pop();
    }
}

void CommonCore::errorRespondDelayedMessages(const std::string& estring)
{
    auto msg = delayTransmitQueue.pop();
    while (msg) {
        if ((*msg).action() == CMD_QUERY ||
            (*msg).action() == CMD_BROKER_QUERY) {  // deal with in flight queries that will block
                                                    // unless a response is given
            activeQueries.setDelayedValue((*msg).messageID, std::string("#error:") + estring);
        }
        // else other message which might get into here shouldn't need any action, just drop them
        msg = delayTransmitQueue.pop();
    }
}

void CommonCore::sendErrorToFederates(int errorCode, std::string_view message)
{
    ActionMessage errorCom(CMD_LOCAL_ERROR);
    errorCom.source_id = global_broker_id_local;
    errorCom.messageID = errorCode;
    errorCom.payload = message;
    loopFederates.apply([&errorCom](auto& fed) {
        if ((fed) && (fed.state == operation_state::operating)) {
            fed->addAction(errorCom);
        }
    });
}

void CommonCore::broadcastToFederates(ActionMessage& cmd)
{
    loopFederates.apply([&cmd](auto& fed) {
        if ((fed) && (fed.state == operation_state::operating)) {
            cmd.dest_id = fed->global_id;
            fed->addAction(cmd);
        }
    });
}

void CommonCore::transmitDelayedMessages(GlobalFederateId source)
{
    std::vector<ActionMessage> buffer;
    auto msg = delayTransmitQueue.pop();
    while (msg) {
        if (msg->source_id == source) {
            routeMessage(*msg);
        } else {  // these messages were delayed for a different purpose and will be dealt with in a
                  // different way
            buffer.push_back(std::move(*msg));
        }
        msg = delayTransmitQueue.pop();
    }

    if (!buffer.empty()) {
        for (auto& am : buffer) {
            delayTransmitQueue.push(std::move(am));
        }
    }

    if (!delayedTimingMessages[source.baseValue()].empty()) {
        for (auto& delayedMsg : delayedTimingMessages[source.baseValue()]) {
            routeMessage(delayedMsg);
        }
        delayedTimingMessages[source.baseValue()].clear();
    }
}

void CommonCore::processCommand(ActionMessage&& command)
{
    LOG_TRACE(global_broker_id_local,
              getIdentifier(),
              fmt::format("|| cmd:{} from {}",
                          prettyPrintString(command),
                          command.source_id.baseValue()));
    switch (command.action()) {
        case CMD_IGNORE:
            break;
        case CMD_TICK:
            if (isReasonForTick(command.messageID, TickForwardingReasons::PING_RESPONSE) ||
                isReasonForTick(command.messageID, TickForwardingReasons::NO_COMMS)) {
                if (getBrokerState() == BrokerState::operating) {
                    timeoutMon->tick(this);
                    LOG_SUMMARY(global_broker_id_local, getIdentifier(), " core tick");
                }
            }
            if (isReasonForTick(command.messageID, TickForwardingReasons::QUERY_TIMEOUT)) {
                checkQueryTimeouts();
            }

            break;
        case CMD_PING:
        case CMD_BROKER_PING:  // broker ping for core is the same as core
            if (command.dest_id == global_broker_id_local) {
                ActionMessage pngrep(CMD_PING_REPLY);
                pngrep.dest_id = command.source_id;
                pngrep.source_id = global_broker_id_local;
                routeMessage(pngrep);
            }
            break;
        case CMD_PING_REPLY:
            if (command.dest_id == global_broker_id_local) {
                timeoutMon->pingReply(command);
            }
            break;
        case CMD_RESEND:
            LOG_WARNING_SIMPLE("got resend");
            if (command.messageID == static_cast<int32_t>(CMD_REG_BROKER)) {
                if ((global_id.load() == parent_broker_id) || (!(global_id.load().isValid()))) {
                    LOG_WARNING_SIMPLE("resending broker reg");
                    ActionMessage m(CMD_REG_BROKER);
                    m.source_id = GlobalFederateId{};
                    m.name(getIdentifier());
                    m.setStringData(getAddress());
                    setActionFlag(m, core_flag);
                    m.counter = 1;
                    transmit(parent_route_id, m);
                }
            }
            break;
        case CMD_TIME_BARRIER:
        case CMD_TIME_BARRIER_CLEAR:
            broadcastToFederates(command);
            break;
        case CMD_CHECK_CONNECTIONS: {
            auto res = checkAndProcessDisconnect();
            auto pred = [](const auto& fed) {
                auto state = fed->getState();
                return (HELICS_FINISHED == state) || (HELICS_ERROR == state);
            };
            auto afed = std::all_of(loopFederates.begin(), loopFederates.end(), pred);
            LOG_WARNING(global_broker_id_local,
                        getIdentifier(),
                        fmt::format("CHECK CONNECTIONS {}, federates={}, fed_disconnected={}",
                                    res,
                                    loopFederates.size(),
                                    afed));
        }

        break;
        case CMD_USER_DISCONNECT:
        case CMD_GLOBAL_DISCONNECT:
        case CMD_BROADCAST_DISCONNECT:
        case CMD_STOP:
        case CMD_DISCONNECT:
        case CMD_DISCONNECT_FED:
        case CMD_DISCONNECT_CHECK:
        case CMD_DISCONNECT_CORE_ACK:
        case CMD_TIMEOUT_DISCONNECT:
            processDisconnectCommand(command);
            break;
        case CMD_EXEC_GRANT:
            if (command.source_id == keyFed) {
                simTime.store(0.0);
            }
            [[fallthrough]];
        case CMD_EXEC_REQUEST:
            if (isLocal(GlobalBrokerId(command.source_id))) {
                if (hasTimeBlock(command.source_id)) {
                    delayedTimingMessages[command.source_id.baseValue()].push_back(command);
                    break;
                }
            }
            if (command.dest_id == global_broker_id_local) {
                timeCoord->processTimeMessage(command);
                if (!enteredExecutionMode) {
                    auto res = timeCoord->checkExecEntry();
                    if (res == MessageProcessingResult::NEXT_STEP) {
                        enteredExecutionMode = true;
                        LOG_TIMING(global_broker_id_local, getIdentifier(), "entering Exec Mode");
                    } else {
                        timeCoord->updateTimeFactors();
                    }
                }
            } else if (!command.dest_id.isValid() && command.source_id == global_broker_id_local) {
                for (auto& dep : timeCoord->getDependents()) {
                    routeMessage(command, dep);
                }
            } else {
                routeMessage(command);
            }
            break;
        case CMD_TIME_GRANT:
            if (command.source_id == keyFed) {
                simTime.store(static_cast<double>(command.actionTime));
            }
            [[fallthrough]];
        case CMD_TIME_REQUEST:
            if (isLocal(command.source_id)) {
                if (hasTimeBlock(command.source_id)) {
                    delayedTimingMessages[command.source_id.baseValue()].push_back(command);
                    break;
                }
            }
            routeMessage(command);
            break;
        case CMD_GRANT_TIMEOUT_CHECK:
            routeMessage(command);
            break;
        case CMD_TIME_BLOCK:
        case CMD_TIME_UNBLOCK:
            manageTimeBlocks(command);
            break;
        case CMD_BROKER_QUERY_ORDERED:
        case CMD_QUERY_ORDERED:
        case CMD_QUERY_REPLY_ORDERED:
            processQueryCommand(command);
            break;
        case CMD_SEND_COMMAND_ORDERED:
            if (command.dest_id == global_broker_id_local) {
                processCommandInstruction(command);
                break;
            }
            if (command.dest_id == parent_broker_id) {
                const auto& target = command.getString(targetStringLoc);
                if (target == "core" || target == getIdentifier()) {
                    processCommandInstruction(command);
                    break;
                }
                auto* fed = getFederateCore(target);
                if (fed != nullptr) {
                    fed->sendCommand(command);
                    break;
                }
            }
            if (isLocal(command.dest_id)) {
                auto* fed = getFederateCore(command.dest_id);
                if (fed != nullptr) {
                    fed->sendCommand(command);
                    break;
                }
            }
            routeMessage(std::move(command));
            break;
        case CMD_SEARCH_DEPENDENCY: {
            auto* fed = getFederateCore(std::string(command.name()));
            if (fed != nullptr) {
                if (fed->global_id.load().isValid()) {
                    ActionMessage dep(CMD_ADD_DEPENDENCY, fed->global_id.load(), command.source_id);
                    routeMessage(dep);
                    dep =
                        ActionMessage(CMD_ADD_DEPENDENT, command.source_id, fed->global_id.load());
                    routeMessage(dep);
                    break;
                }
            }
            // it is not found send to broker
            transmit(parent_route_id, command);
        } break;
        case CMD_ADD_DEPENDENCY:
        case CMD_REMOVE_DEPENDENCY:
        case CMD_ADD_DEPENDENT:
        case CMD_REMOVE_DEPENDENT:
        case CMD_ADD_INTERDEPENDENCY:
        case CMD_REMOVE_INTERDEPENDENCY:
            routeMessage(command);
            break;
        case CMD_SEND_FOR_FILTER:
        case CMD_SEND_FOR_FILTER_AND_RETURN:
        case CMD_SEND_FOR_DEST_FILTER_AND_RETURN:
            if (command.dest_id == filterFedID.load()) {
                filterFed->processMessageFilter(command);
            }
            break;
        case CMD_NULL_MESSAGE:
        case CMD_FILTER_RESULT:
            // if (command.dest_id == filterFedID.load()) {
            filterFed->processFilterReturn(command);
            //  }
            break;
        case CMD_DEST_FILTER_RESULT:
        case CMD_NULL_DEST_MESSAGE:
            //  if (command.dest_id == filterFedID) {
            filterFed->processDestFilterReturn(command);
            //  }
            break;
        case CMD_PUB:
            routeMessage(command);
            break;
        case CMD_LOG:
        case CMD_REMOTE_LOG:
            if (command.dest_id == global_broker_id_local) {
                sendToLogger(parent_broker_id,
                             command.messageID,
                             command.getString(0),
                             command.payload.to_string(),
                             command.action() == CMD_REMOTE_LOG);
            } else {
                routeMessage(command);
            }
            break;
        case CMD_WARNING:
            if (command.dest_id == global_broker_id_local) {
                sendToLogger(command.source_id,
                             LogLevels::WARNING,
                             command.getString(0),
                             command.payload.to_string());
            } else {
                routeMessage(command);
            }
            break;
        case CMD_ERROR:
        case CMD_LOCAL_ERROR:
            if (command.dest_id == global_broker_id_local) {
                if (command.source_id == higher_broker_id ||
                    command.source_id == parent_broker_id || command.source_id == gRootBrokerID) {
                    sendErrorToFederates(command.messageID, command.payload.to_string());
                    setErrorState(command.messageID, command.payload.to_string());

                } else {
                    sendToLogger(parent_broker_id,
                                 LogLevels::ERROR_LEVEL,
                                 getFederateNameNoThrow(command.source_id),
                                 command.payload.to_string());
                    auto fed = loopFederates.find(command.source_id);
                    if (fed != loopFederates.end()) {
                        fed->state = operation_state::error;
                    } else if (command.source_id == filterFedID) {
                        filterFed->handleMessage(command);
                        // filterFed->
                    }

                    if (hasTimeDependency) {
                        timeCoord->processTimeMessage(command);
                    }
                }
                if (terminate_on_error) {
                    if (getBrokerState() != BrokerState::errored &&
                        getBrokerState() != BrokerState::connected_error) {
                        sendErrorToFederates(command.messageID, command.payload.to_string());
                        setBrokerState(BrokerState::errored);
                    }
                    command.setAction(CMD_GLOBAL_ERROR);
                    command.source_id = global_broker_id_local;
                    command.dest_id = gRootBrokerID;
                    transmit(parent_route_id, std::move(command));
                }
            } else {
                if (command.dest_id == parent_broker_id) {
                    if (terminate_on_error) {
                        if (getBrokerState() != BrokerState::errored) {
                            sendErrorToFederates(command.messageID, command.payload.to_string());
                            setBrokerState(BrokerState::errored);
                        }
                        command.setAction(CMD_GLOBAL_ERROR);
                        command.source_id = global_broker_id_local;
                        command.dest_id = gRootBrokerID;
                        transmit(parent_route_id, std::move(command));
                        break;
                    }
                    if (command.source_id.isValid()) {
                        auto fed = loopFederates.find(command.source_id);
                        if (fed != loopFederates.end()) {
                            fed->state = operation_state::error;
                        }
                    }
                }
                routeMessage(command);
            }
            break;
        case CMD_GLOBAL_ERROR:

            if (getBrokerState() == BrokerState::connecting) {
                processDisconnect();
            }
            setErrorState(command.messageID, command.payload.to_string());
            if (isConnected()) {
                sendErrorToFederates(command.messageID, command.payload.to_string());
                if (!(command.source_id == higher_broker_id ||
                      command.source_id == gRootBrokerID)) {
                    transmit(parent_route_id, std::move(command));
                }
            }
            break;
        case CMD_DATA_LINK: {
            auto* pub = loopHandles.getPublication(command.name());
            if (pub != nullptr) {
                command.name(command.getString(targetStringLoc));
                command.setAction(CMD_ADD_NAMED_INPUT);
                command.setSource(pub->handle);
                command.clearStringData();
                checkForNamedInterface(command);
            } else {
                auto* input = loopHandles.getInput(command.getString(targetStringLoc));
                if (input == nullptr) {
                    routeMessage(command);
                } else {
                    command.setAction(CMD_ADD_NAMED_PUBLICATION);
                    command.setSource(input->handle);
                    command.clearStringData();
                    checkForNamedInterface(command);
                }
            }
        } break;
        case CMD_ENDPOINT_LINK: {
            auto* ept = loopHandles.getEndpoint(command.name());
            if (ept != nullptr) {
                command.name(command.getString(targetStringLoc));
                command.setAction(CMD_ADD_NAMED_ENDPOINT);
                command.counter = static_cast<uint16_t>(InterfaceType::ENDPOINT);
                command.setSource(ept->handle);
                setActionFlag(command, destination_target);
                command.clearStringData();
                checkForNamedInterface(command);
            } else {
                auto* target = loopHandles.getEndpoint(command.getString(targetStringLoc));
                if (target == nullptr) {
                    routeMessage(command);
                } else {
                    command.setAction(CMD_ADD_NAMED_ENDPOINT);
                    command.setSource(target->handle);
                    command.counter = static_cast<uint16_t>(InterfaceType::ENDPOINT);
                    command.clearStringData();
                    checkForNamedInterface(command);
                }
            }
        } break;
        case CMD_FILTER_LINK: {
            auto* filt = loopHandles.getFilter(command.name());
            if (filt != nullptr) {
                command.name(command.getString(targetStringLoc));
                command.setAction(CMD_ADD_NAMED_ENDPOINT);
                command.setSource(filt->handle);
                if (checkActionFlag(*filt, clone_flag)) {
                    setActionFlag(command, clone_flag);
                }
                checkForNamedInterface(command);
            } else {
                auto* ept = loopHandles.getEndpoint(command.getString(targetStringLoc));
                if (ept == nullptr) {
                    routeMessage(command);
                } else {
                    command.setAction(CMD_ADD_NAMED_FILTER);
                    command.setSource(ept->handle);
                    checkForNamedInterface(command);
                }
            }
        } break;
        case CMD_REG_INPUT:
        case CMD_REG_ENDPOINT:
        case CMD_REG_PUB:
        case CMD_REG_FILTER:
            registerInterface(command);
            break;
        case CMD_ADD_NAMED_ENDPOINT:
        case CMD_ADD_NAMED_PUBLICATION:
        case CMD_ADD_NAMED_INPUT:
        case CMD_ADD_NAMED_FILTER:
            checkForNamedInterface(command);
            break;
        case CMD_ADD_ENDPOINT:
        case CMD_ADD_FILTER:
        case CMD_ADD_SUBSCRIBER:
        case CMD_ADD_PUBLISHER:
            addTargetToInterface(command);
            break;
        case CMD_REMOVE_NAMED_ENDPOINT:
        case CMD_REMOVE_NAMED_PUBLICATION:
        case CMD_REMOVE_NAMED_INPUT:
        case CMD_REMOVE_NAMED_FILTER:
            removeNamedTarget(command);
            break;
        case CMD_REMOVE_PUBLICATION:
        case CMD_REMOVE_SUBSCRIBER:
        case CMD_REMOVE_FILTER:
        case CMD_REMOVE_ENDPOINT:
            removeTargetFromInterface(command);
            break;
        case CMD_CLOSE_INTERFACE:
            disconnectInterface(command);
            break;
        case CMD_CORE_TAG:
            if (command.source_id == global_broker_id_local &&
                command.dest_id == global_broker_id_local) {
                auto tag = command.getString(0);
                for (auto& tg : tags) {
                    if (tg.first == tag) {
                        tg.second = command.getString(1);
                        break;
                    }
                }
                tags.emplace_back(tag, command.getString(1));
            }
            break;
        case CMD_CORE_CONFIGURE:
            processCoreConfigureCommands(command);
            break;
        case CMD_INTERFACE_TAG: {
            auto* handle = loopHandles.findHandle(command.getSource());
            if (handle != nullptr) {
                handle->setTag(command.getString(0), command.getString(1));
            }
            break;
        }
        case CMD_INIT: {
            auto* fed = getFederateCore(command.source_id);
            if (fed == nullptr) {
                break;
            }

            fed->init_transmitted = true;

            if (allInitReady()) {
                if (transitionBrokerState(BrokerState::connected,
                                          BrokerState::initializing)) {  // make sure we only
                                                                         // do this once
                    checkDependencies();
                    command.source_id = global_broker_id_local;
                    transmit(parent_route_id, command);
                } else if (checkActionFlag(command, observer)) {
                    command.source_id = global_broker_id_local;
                    transmit(parent_route_id, command);
                }
            }

        } break;
        case CMD_INIT_GRANT:
            if (transitionBrokerState(
                    BrokerState::initializing,
                    BrokerState::operating)) {  // forward the grant to all federates
                if (filterFed != nullptr) {
                    filterFed->organizeFilterOperations();
                }

                loopFederates.apply([&command](auto& fed) { fed->addAction(command); });
                if (filterFed != nullptr && filterTiming) {
                    filterFed->handleMessage(command);
                }
                timeCoord->enteringExecMode();
                auto res = timeCoord->checkExecEntry();
                if (res == MessageProcessingResult::NEXT_STEP) {
                    enteredExecutionMode = true;
                }
                if (!timeCoord->hasActiveTimeDependencies()) {
                    timeCoord->disconnect();
                }
            } else if (checkActionFlag(command, observer_flag)) {
                routeMessage(command);
            }
            break;

        case CMD_SEND_MESSAGE:
            if (checkActionFlag(command, filter_processing_required_flag) ||
                ((command.dest_id == parent_broker_id) && (isLocal(command.source_id)))) {
                deliverMessage(processMessage(command));
            } else {
                deliverMessage(command);
            }

            break;
        case CMD_PROFILER_DATA:
            if (enable_profiling) {
                saveProfilingData(command.payload.to_string());
            } else {
                routeMessage(std::move(command), parent_broker_id);
            }
            break;
        case CMD_SET_PROFILER_FLAG:
            routeMessage(command);
            break;
        case CMD_REQUEST_CURRENT_TIME:
            if (isLocal(command.dest_id)) {
                auto* fed = getFederateCore(command.dest_id);
                if (fed != nullptr) {
                    if ((fed->getState() != FederateStates::HELICS_FINISHED) &&
                        (fed->getState() != FederateStates::HELICS_ERROR)) {
                        fed->forceProcessMessage(command);
                    } else {
                        auto rep = fed->processPostTerminationAction(command);
                        if (rep) {
                            routeMessage(*rep);
                        }
                    }
                }
            } else {
                routeMessage(command);
            }
            break;
        default:
            if (isPriorityCommand(command)) {
                // this is a backup if somehow one of these message got here
                processPriorityCommand(std::move(command));
            } else if (isLocal(command.dest_id)) {
                routeMessage(command);
            }
            break;
    }
}

void CommonCore::registerInterface(ActionMessage& command)
{
    if (command.dest_id == parent_broker_id) {
        auto handle = command.source_handle;
        auto& lH = loopHandles;
        handles.read([handle, &lH](auto& hand) {
            auto ifc = hand.getHandleInfo(handle.baseValue());
            if (ifc != nullptr) {
                lH.addHandleAtIndex(*ifc, handle.baseValue());
            }
        });

        switch (command.action()) {
            case CMD_REG_INPUT:
            case CMD_REG_PUB:
                break;
            case CMD_REG_ENDPOINT:
                if (timeCoord->addDependency(command.source_id)) {
                    auto* fed = getFederateCore(command.source_id);
                    if (fed != nullptr) {
                        ActionMessage add(CMD_ADD_INTERDEPENDENCY,
                                          global_broker_id_local,
                                          command.source_id);

                        setActionFlag(add, parent_flag);
                        fed->addAction(add);
                        timeCoord->addDependent(fed->global_id);
                        timeCoord->setAsChild(fed->global_id);
                    }
                }

                if (!hasTimeDependency) {
                    if (timeCoord->addDependency(higher_broker_id)) {
                        hasTimeDependency = true;
                        ActionMessage add(CMD_ADD_INTERDEPENDENCY,
                                          global_broker_id_local,
                                          higher_broker_id);
                        setActionFlag(add, child_flag);

                        transmit(getRoute(higher_broker_id), add);

                        timeCoord->addDependent(higher_broker_id);
                        timeCoord->setAsParent(higher_broker_id);
                    }
                }
                break;
            case CMD_REG_FILTER:

                if (filterFed == nullptr) {
                    generateFilterFederate();
                }
                filterFed->createFilter(filterFedID.load(),
                                        command.source_handle,
                                        std::string(command.name()),
                                        command.getString(typeStringLoc),
                                        command.getString(typeOutStringLoc),
                                        checkActionFlag(command, clone_flag));
                connectFilterTiming();
                break;
            default:
                return;
        }
        if (!command.payload.empty()) {
            transmit(parent_route_id, std::move(command));
        }
    } else {
        routeMessage(std::move(command));
    }
}

void CommonCore::generateFilterFederate()
{
    auto fid = filterFedID.load();

    filterFed = new FilterFederate(fid, getIdentifier() + "_filters", global_broker_id_local, this);
    filterThread.store(std::this_thread::get_id());
    filterFedID.store(fid);

    filterFed->setCallbacks([this](const ActionMessage& m) { addActionMessage(m); },
                            [this](ActionMessage&& m) { addActionMessage(std::move(m)); },
                            [this](const ActionMessage& m) { routeMessage(m); },
                            [this](ActionMessage&& m) { routeMessage(std::move(m)); });
    hasFilters = true;

    filterFed->setHandleManager(&loopHandles);
    filterFed->setLogger([this](int level, const std::string& name, const std::string& message) {
        sendToLogger(global_broker_id_local, level, name, message);
    });
    filterFed->setAirLockFunction([this](int index) { return std::ref(dataAirlocks[index]); });
    filterFed->setDeliver([this](ActionMessage& m) { deliverMessage(m); });
    ActionMessage newFed(CMD_REG_FED);
    setActionFlag(newFed, child_flag);
    setActionFlag(newFed, non_counting_flag);
    newFed.dest_id = parent_broker_id;
    newFed.source_id = global_broker_id_local;
    newFed.setExtraData(fid.baseValue());
    newFed.name(getIdentifier() + "_filters");
    transmit(getRoute(higher_broker_id), newFed);
}

void CommonCore::connectFilterTiming()
{
    if (filterTiming) {
        return;
    }
    filterTiming = true;
    auto fid = filterFedID.load();
    if (timeCoord->addDependent(higher_broker_id)) {
        ActionMessage add(CMD_ADD_INTERDEPENDENCY, global_broker_id_local, higher_broker_id);
        setActionFlag(add, child_flag);
        transmit(getRoute(higher_broker_id), add);
        timeCoord->addDependency(higher_broker_id);
        timeCoord->setAsParent(higher_broker_id);
    }
    // now add the filterFederate as a timeDependency
    timeCoord->addDependency(fid);
    timeCoord->setAsChild(fid);
    ActionMessage ad(CMD_ADD_DEPENDENT);
    setActionFlag(ad, parent_flag);
    ad.dest_id = fid;
    ad.source_id = global_broker_id_local;
    filterFed->handleMessage(ad);
    // TODO(PT) this should be conditional as it probably isn't needed in all cases
    ad.setAction(CMD_ADD_DEPENDENCY);
    timeCoord->addDependent(fid);
    filterFed->handleMessage(ad);
    //
    filterTiming = true;
}

void CommonCore::setAsUsed(BasicHandleInfo* hand)
{
    assert(hand != nullptr);
    if (hand->used) {
        return;
    }
    hand->used = true;
    handles.modify([&](auto& handle) { handle.getHandleInfo(hand->handle.handle)->used = true; });
}
void CommonCore::checkForNamedInterface(ActionMessage& command)
{
    switch (command.action()) {
        case CMD_ADD_NAMED_PUBLICATION: {
            auto* pub = loopHandles.getPublication(command.name());
            if (pub != nullptr) {
                if (checkActionFlag(*pub, disconnected_flag)) {
                    // TODO(PT): this might generate an error if the required flag was set
                    return;
                }
                command.setAction(CMD_ADD_SUBSCRIBER);
                command.setDestination(pub->handle);
                auto name{std::move(command.payload)};
                command.payload.clear();

                addTargetToInterface(command);
                command.setAction(CMD_ADD_PUBLISHER);
                command.payload = std::move(name);
                command.swapSourceDest();
                command.setStringData(pub->type, pub->units);
                addTargetToInterface(command);
            } else {
                routeMessage(std::move(command));
            }
        } break;
        case CMD_ADD_NAMED_INPUT: {
            const std::string inputName(command.name());  // need to copy the name
            auto* inp = loopHandles.getInput(inputName);
            if (inp != nullptr) {
                if (checkActionFlag(*inp, disconnected_flag)) {
                    // TODO(PT): this might generate an error if the required flag was set
                    return;
                }
                command.setAction(CMD_ADD_PUBLISHER);
                command.setDestination(inp->handle);
                command.payload.clear();
                if (command.getStringData().empty()) {
                    auto* pub = loopHandles.findHandle(command.getSource());
                    if (pub != nullptr) {
                        command.setStringData(pub->type, pub->units);
                    }
                }
                addTargetToInterface(command);
                command.setAction(CMD_ADD_SUBSCRIBER);
                command.swapSourceDest();
                command.clearStringData();
                command.name(inputName);
                addTargetToInterface(command);
            } else {
                routeMessage(std::move(command));
            }
        } break;
        case CMD_ADD_NAMED_FILTER: {
            auto* filt = loopHandles.getFilter(command.name());
            if (filt != nullptr) {
                if (checkActionFlag(*filt, disconnected_flag)) {
                    // TODO(PT): this might generate an error if the required flag was set
                    return;
                }
                command.setAction(CMD_ADD_ENDPOINT);
                command.setDestination(filt->handle);
                command.payload.clear();
                addTargetToInterface(command);
                command.setAction(CMD_ADD_FILTER);
                command.swapSourceDest();
                if (checkActionFlag(*filt, clone_flag)) {
                    setActionFlag(command, clone_flag);
                }
                addTargetToInterface(command);
            } else {
                routeMessage(std::move(command));
            }
        } break;
        case CMD_ADD_NAMED_ENDPOINT: {
            auto* ept = loopHandles.getEndpoint(command.name());
            if (ept != nullptr) {
                if (checkActionFlag(*ept, disconnected_flag)) {
                    // TODO(PT): this might generate an error if the required flag was set
                    return;
                }

                if (command.counter == static_cast<uint16_t>(InterfaceType::ENDPOINT)) {
                    command.setAction(CMD_ADD_ENDPOINT);
                    toggleActionFlag(command, destination_target);
                } else {
                    command.setAction(CMD_ADD_FILTER);
                }
                command.setDestination(ept->handle);
                addTargetToInterface(command);

                // to the originating interface
                command.setAction(CMD_ADD_ENDPOINT);
                if (command.counter == static_cast<uint16_t>(InterfaceType::ENDPOINT)) {
                    toggleActionFlag(command, destination_target);
                }

                command.swapSourceDest();
                command.setSource(ept->handle);
                command.name(ept->key);
                command.setString(typeStringLoc, ept->type);
                addTargetToInterface(command);

            } else {
                routeMessage(std::move(command));
            }
        } break;
        default:
            break;
    }
}

void CommonCore::removeNamedTarget(ActionMessage& command)
{
    switch (command.action()) {
        case CMD_REMOVE_NAMED_PUBLICATION: {
            auto* pub = loopHandles.getPublication(command.name());
            if (pub != nullptr) {
                command.setAction(CMD_REMOVE_SUBSCRIBER);
                command.setDestination(pub->handle);
                command.payload.clear();
                removeTargetFromInterface(command);
                command.setAction(CMD_REMOVE_PUBLICATION);
                command.swapSourceDest();
                removeTargetFromInterface(command);
            } else {
                routeMessage(std::move(command));
            }
        } break;
        case CMD_REMOVE_NAMED_INPUT: {
            auto* inp = loopHandles.getInput(command.name());
            if (inp != nullptr) {
                command.setAction(CMD_REMOVE_PUBLICATION);
                command.setDestination(inp->handle);
                command.payload.clear();
                removeTargetFromInterface(command);
                command.setAction(CMD_REMOVE_SUBSCRIBER);
                command.swapSourceDest();
                removeTargetFromInterface(command);
            } else {
                routeMessage(std::move(command));
            }
        } break;
        case CMD_REMOVE_NAMED_FILTER: {
            auto* filt = loopHandles.getFilter(command.name());
            if (filt != nullptr) {
                command.setAction(CMD_REMOVE_ENDPOINT);
                command.setDestination(filt->handle);
                command.payload.clear();
                removeTargetFromInterface(command);
                command.setAction(CMD_REMOVE_FILTER);
                command.swapSourceDest();
                removeTargetFromInterface(command);
            } else {
                routeMessage(std::move(command));
            }
        } break;
        case CMD_REMOVE_NAMED_ENDPOINT: {
            auto* pub = loopHandles.getEndpoint(command.name());
            if (pub != nullptr) {
                command.setAction(CMD_REMOVE_FILTER);
                command.setDestination(pub->handle);
                command.payload.clear();
                removeTargetFromInterface(command);
                command.setAction(CMD_REMOVE_ENDPOINT);
                command.swapSourceDest();
                removeTargetFromInterface(command);
            } else {
                routeMessage(std::move(command));
            }
        } break;
        default:
            break;
    }
}

void CommonCore::disconnectInterface(ActionMessage& command)
{
    auto* handleInfo = loopHandles.getHandleInfo(command.source_handle);
    if (handleInfo == nullptr) {
        return;
    }
    if (checkActionFlag(*handleInfo, disconnected_flag)) {
        return;
    }
    setActionFlag(*handleInfo, disconnected_flag);
    if (handleInfo->getFederateId() == filterFedID.load()) {
        if (filterFed != nullptr) {
            filterFed->handleMessage(command);
        }
    } else {
        if (handleInfo->handleType != InterfaceType::FILTER) {
            auto* fed = getFederateCore(command.source_id);
            if (fed != nullptr) {
                fed->addAction(command);
            }
        }
    }

    if (!checkActionFlag(*handleInfo, nameless_interface_flag)) {
        transmit(parent_route_id, command);
    }
}

void CommonCore::addTargetToInterface(ActionMessage& command)
{
    if (command.action() == CMD_ADD_FILTER) {
        if (filterFed == nullptr) {
            generateFilterFederate();
        }
        filterFed->processFilterInfo(command);
        if (command.source_id != global_broker_id_local) {
            if (!checkActionFlag(command, error_flag)) {
                auto* fed = getFederateCore(command.dest_id);
                if (fed != nullptr) {
                    command.setAction(CMD_ADD_DEPENDENT);
                    fed->addAction(command);
                }
            }
        }
    } else if (command.dest_id == filterFedID) {
        // just forward these to the appropriate federate
        filterFed->handleMessage(command);

    } else {
        auto* fed = getFederateCore(command.dest_id);
        if (fed != nullptr) {
            if (!checkActionFlag(command, error_flag)) {
                fed->addAction(command);
            }
            auto* handle = loopHandles.getHandleInfo(command.dest_handle.baseValue());
            if (handle != nullptr) {
                setAsUsed(handle);
            }
        }
    }
}

void CommonCore::removeTargetFromInterface(ActionMessage& command)
{
    if (command.dest_id == filterFedID) {
        filterFed->handleMessage(command);
    } else {  // just forward these to the appropriate federate
        if (command.action() == CMD_REMOVE_FILTER) {
            command.dest_id = filterFedID;
            removeTargetFromInterface(command);
        } else {
            auto* fed = getFederateCore(command.dest_id);
            if (fed != nullptr) {
                fed->addAction(command);
            }
        }
    }
}

void CommonCore::checkQueryTimeouts()
{
    if (!queryTimeouts.empty()) {
        auto ctime = std::chrono::steady_clock::now();
        for (auto& qt : queryTimeouts) {
            if (activeQueries.isRecognized(qt.first) && !activeQueries.isCompleted(qt.first)) {
                if (Time(ctime - qt.second) > queryTimeout) {
                    activeQueries.setDelayedValue(
                        qt.first,
                        generateJsonErrorResponse(JsonErrorCodes::GATEWAY_TIMEOUT,
                                                  "query timeout"));
                    qt.first = 0;
                }
            }
        }
        while (!queryTimeouts.empty() && queryTimeouts.front().first == 0) {
            queryTimeouts.pop_front();
        }
        if (queryTimeouts.empty()) {
            setTickForwarding(TickForwardingReasons::QUERY_TIMEOUT, false);
        }
    }
}

void CommonCore::processQueryResponse(const ActionMessage& m)
{
    if (m.counter == GENERAL_QUERY) {
        activeQueries.setDelayedValue(m.messageID, std::string(m.payload.to_string()));
        return;
    }
    if (isValidIndex(m.counter, mapBuilders)) {
        auto& builder = std::get<0>(mapBuilders[m.counter]);
        auto& requestors = std::get<1>(mapBuilders[m.counter]);
        if (builder.addComponent(std::string(m.payload.to_string()), m.messageID)) {
            auto str = builder.generate();
            if (m.counter == GLOBAL_FLUSH) {
                str = "{\"status\":true}";
            }
            for (int ii = 0; ii < static_cast<int>(requestors.size()) - 1; ++ii) {
                if (requestors[ii].dest_id == global_broker_id_local) {
                    activeQueries.setDelayedValue(requestors[ii].messageID, str);
                } else {
                    requestors[ii].payload = str;
                    routeMessage(std::move(requestors[ii]));
                }
            }
            if (requestors.back().dest_id == global_broker_id_local ||
                requestors.back().dest_id == gDirectCoreId) {
                // TODO(PT) make setDelayedValue have move set function
                activeQueries.setDelayedValue(requestors.back().messageID, str);
            } else {
                requestors.back().payload = std::move(str);
                routeMessage(std::move(requestors.back()));
            }

            requestors.clear();
            if (std::get<2>(mapBuilders[m.counter])) {
                builder.reset();
            } else {
                builder.setCounterCode(generateMapObjectCounter());
            }
        }
    }
}

void CommonCore::checkDependencies()
{
    bool isobs = false;
    bool issource = false;
    auto checkdep = [this, &isobs, &issource](auto& fed) {
        if (fed->endpointCount() > 0) {
            if (fed->getOptionFlag(defs::Flags::OBSERVER)) {
                timeCoord->removeDependency(fed->global_id);
                ActionMessage rmdep(CMD_REMOVE_DEPENDENT);

                rmdep.source_id = global_broker_id_local;
                rmdep.dest_id = fed->global_id.load();
                fed->addAction(rmdep);
                isobs = true;
            } else if (fed->getOptionFlag(defs::Flags::SOURCE_ONLY)) {
                timeCoord->removeDependent(fed->global_id);
                ActionMessage rmdep(CMD_REMOVE_DEPENDENCY);

                rmdep.source_id = global_broker_id_local;
                rmdep.dest_id = fed->global_id.load();
                fed->addAction(rmdep);
                issource = true;
            }
        }
    };
    loopFederates.apply(checkdep);

    // if there is more than 2 dependents or dependencies (higher broker + 2 or more federates)
    // then we need to be a timeCoordinator
    if (timeCoord->getDependents().size() > 2) {
        return;
    }
    if (timeCoord->getDependencies().size() > 2) {
        return;
    }
    GlobalFederateId fedid;
    GlobalBrokerId brkid;
    int localcnt = 0;
    for (const auto& dep : timeCoord->getDependents()) {
        if (isLocal(dep)) {
            ++localcnt;
            fedid = dep;
        } else {
            brkid = static_cast<GlobalBrokerId>(dep);
        }
    }
    if (localcnt > 1) {
        return;
    }
    if ((localcnt == 0) && (!brkid.isValid())) {
        hasTimeDependency = false;
        return;
    }
    // check to make sure the dependencies match
    for (auto& dep : timeCoord->getDependencies()) {
        if (!((dep == fedid) || (dep == brkid))) {
            return;
        }
    }
    // remove the core from the time dependency chain since it is just adding to the
    // communication noise in this case
    timeCoord->removeDependency(brkid);
    timeCoord->removeDependency(fedid);
    timeCoord->removeDependent(brkid);
    timeCoord->removeDependent(fedid);
    hasTimeDependency = false;
    ActionMessage rmdep(CMD_REMOVE_INTERDEPENDENCY);

    rmdep.source_id = global_broker_id_local;
    routeMessage(rmdep, brkid);
    routeMessage(rmdep, fedid);
    if (isobs) {
        ActionMessage adddep(CMD_ADD_DEPENDENT);
        adddep.source_id = fedid;
        setActionFlag(adddep, child_flag);
        routeMessage(adddep, brkid);
        adddep.setAction(CMD_ADD_DEPENDENCY);
        adddep.source_id = brkid;
        clearActionFlag(adddep, child_flag);
        setActionFlag(adddep, parent_flag);
        routeMessage(adddep, fedid);
    } else if (issource) {
        ActionMessage adddep(CMD_ADD_DEPENDENCY);
        adddep.source_id = fedid;
        setActionFlag(adddep, child_flag);
        routeMessage(adddep, brkid);

        adddep.setAction(CMD_ADD_DEPENDENT);
        clearActionFlag(adddep, child_flag);
        setActionFlag(adddep, parent_flag);
        adddep.source_id = brkid;
        routeMessage(adddep, fedid);
    } else {
        ActionMessage adddep(CMD_ADD_INTERDEPENDENCY);
        adddep.source_id = fedid;
        setActionFlag(adddep, child_flag);
        routeMessage(adddep, brkid);
        // make sure the fed depends on itself in case the broker removes itself later
        routeMessage(adddep, fedid);
        adddep.source_id = brkid;
        clearActionFlag(adddep, child_flag);
        setActionFlag(adddep, parent_flag);
        routeMessage(adddep, fedid);
    }
}

void CommonCore::processDisconnectCommand(ActionMessage& cmd)
{
    switch (cmd.action()) {
        case CMD_USER_DISCONNECT:
        case CMD_GLOBAL_DISCONNECT:
            if (isConnected()) {
                if (getBrokerState() <
                    BrokerState::terminating) {  // only send a disconnect message
                                                 // if we haven't done so already
                    setBrokerState(BrokerState::terminating);
                    sendDisconnect();
                    ActionMessage m(CMD_DISCONNECT);
                    m.source_id = global_broker_id_local;
                    transmit(parent_route_id, m);
                }
            } else if (getBrokerState() ==
                       BrokerState::errored) {  // we are disconnecting in an error state
                sendDisconnect();
                ActionMessage m(CMD_DISCONNECT);
                m.source_id = global_broker_id_local;
                transmit(parent_route_id, m);
            }
            addActionMessage(CMD_STOP);
            // we can't just fall through since this may have generated other messages that need to
            // be forwarded or processed
            break;
        case CMD_BROADCAST_DISCONNECT:
            timeCoord->processTimeMessage(cmd);
            loopFederates.apply([&cmd](auto& fed) { fed->addAction(cmd); });
            checkAndProcessDisconnect();
            break;
        case CMD_TIMEOUT_DISCONNECT:
            if (isConnected()) {
                if (cmd.source_id != global_broker_id_local) {
                    LOG_ERROR(global_broker_id_local,
                              getIdentifier(),
                              "received timeout disconnect");
                } else {
                    LOG_ERROR(global_broker_id_local, getIdentifier(), "timeout disconnect");
                }
                if (timeCoord && !timeCoord->empty()) {
                    Json::Value base;
                    base["id"] = global_broker_id_local.baseValue();
                    base["state"] = brokerStateName(getBrokerState());
                    base["time"] = Json::Value();
                    base["name"] = getIdentifier();
                    timeCoord->generateDebuggingTimeInfo(base["time"]);
                    base["federates"] = Json::arrayValue;
                    for (const auto& fed : loopFederates) {
                        std::string ret = federateQuery(fed.fed, "global_time_debugging", false);
                        if (ret == "#wait") {
                            if (fed->getState() <= FederateStates::HELICS_EXECUTING) {
                                cmd.dest_id = fed->global_id.load();
                                cmd.source_id = global_broker_id_local;
                                fed.fed->addAction(cmd);
                            }
                        } else {
                            auto element = fileops::loadJsonStr(ret);
                            base["federates"].append(element);
                        }
                    }
                    if (filterFed != nullptr) {
                        auto str = filterFed->query("global_time_debugging");
                        auto element = fileops::loadJsonStr(str);
                        base["federates"].append(element);
                    }
                    auto debugString = fileops::generateJsonString(base);
                    debugString.insert(0, "TIME DEBUGGING::");
                    LOG_WARNING(global_broker_id_local, identifier, debugString);
                }

                if (getBrokerState() < BrokerState::terminating) {
                    // only send a disconnect message
                    // if we haven't done so already
                    setBrokerState(BrokerState::terminating);
                    sendDisconnect();
                    ActionMessage m(CMD_DISCONNECT);
                    m.source_id = global_broker_id_local;
                    transmit(parent_route_id, m);
                    for (const auto& fed : loopFederates) {
                        m.dest_id = fed->global_id;
                        fed.fed->addAction(m);
                    }
                }
            } else if (getBrokerState() == BrokerState::errored) {
                // we are disconnecting in an error state
                sendDisconnect();
                ActionMessage m(CMD_DISCONNECT);
                m.source_id = global_broker_id_local;
                transmit(parent_route_id, m);
            }
            addActionMessage(CMD_STOP);
            break;
        case CMD_STOP:

            if (isConnected()) {
                if (getBrokerState() <
                    BrokerState::terminating) {  // only send a disconnect message
                                                 // if we haven't done so already
                    setBrokerState(BrokerState::terminating);
                    sendDisconnect();
                    ActionMessage m(CMD_DISCONNECT);
                    m.source_id = global_broker_id_local;
                    transmit(parent_route_id, m);
                }
            }
            if (filterThread.load() == std::this_thread::get_id()) {
                if (filterFed != nullptr) {
                    delete filterFed;
                    filterFed = nullptr;
                    filterThread.store(std::thread::id{});
                }
            }
            activeQueries.fulfillAllPromises("#disconnected");
            break;
        case CMD_DISCONNECT:
        case CMD_DISCONNECT_FED:
            if (cmd.dest_id == parent_broker_id) {
                if (getBrokerState() < BrokerState::terminating) {
                    auto fed = loopFederates.find(cmd.source_id);
                    if (fed == loopFederates.end()) {
                        return;
                    }
                    fed->state = operation_state::disconnected;
                    auto cstate = getBrokerState();
                    if ((!checkAndProcessDisconnect()) || (cstate < BrokerState::operating)) {
                        cmd.setAction(CMD_DISCONNECT_FED);
                        transmit(parent_route_id, cmd);
                        if (minFederateState() != operation_state::disconnected ||
                            filterFed != nullptr) {
                            cmd.setAction(CMD_DISCONNECT_FED_ACK);
                            cmd.dest_id = cmd.source_id;
                            cmd.source_id = parent_broker_id;
                            routeMessage(cmd);
                        }
                    }
                }
            } else {
                routeMessage(cmd);
            }

            break;
        case CMD_DISCONNECT_CHECK:
            checkAndProcessDisconnect();
            break;
        case CMD_DISCONNECT_CORE_ACK:
            if ((cmd.dest_id == global_broker_id_local) && (cmd.source_id == higher_broker_id)) {
                ActionMessage bye(CMD_DISCONNECT_FED_ACK);
                bye.source_id = parent_broker_id;
                for (auto fed : loopFederates) {
                    if (fed->getState() != FederateStates::HELICS_FINISHED) {
                        bye.dest_id = fed->global_id.load();
                        fed->addAction(bye);
                    }
                }
                addActionMessage(CMD_STOP);
            }
            break;
        default:
            break;
    }
}

void CommonCore::processCoreConfigureCommands(ActionMessage& cmd)
{
    switch (cmd.messageID) {
        case defs::Flags::ENABLE_INIT_ENTRY:
            --delayInitCounter;
            if (delayInitCounter <= 0) {
                if (allInitReady()) {
                    if (transitionBrokerState(
                            BrokerState::connected,
                            BrokerState::initializing)) {  // make sure we only do this once
                        checkDependencies();
                        cmd.setAction(CMD_INIT);
                        cmd.source_id = global_broker_id_local;
                        cmd.dest_id = parent_broker_id;
                        transmit(parent_route_id, cmd);
                    }
                }
            }
            break;
        case defs::Properties::LOG_LEVEL:
            setLogLevel(cmd.getExtraData());
            break;
        case defs::Properties::FILE_LOG_LEVEL:
            setLogLevels(mLogManager->getConsoleLevel(), cmd.getExtraData());
            break;
        case defs::Properties::CONSOLE_LOG_LEVEL:
            setLogLevels(cmd.getExtraData(), mLogManager->getFileLevel());
            break;
        case defs::Properties::LOG_BUFFER: {
            auto size = cmd.getExtraData();
            mLogManager->getLogBuffer().resize((size <= 0) ? 0UL : static_cast<std::size_t>(size));
        }

        break;
        case defs::Flags::TERMINATE_ON_ERROR:
            terminate_on_error = checkActionFlag(cmd, indicator_flag);
            break;
        case defs::Flags::SLOW_RESPONDING:
            no_ping = checkActionFlag(cmd, indicator_flag);
            break;
        case defs::Flags::DEBUGGING:
            debugging = no_ping = checkActionFlag(cmd, indicator_flag);
            break;
        case UPDATE_LOGGING_CALLBACK:
            if (checkActionFlag(cmd, empty_flag)) {
                setLoggerFunction(nullptr);
            } else {
                auto op = dataAirlocks[cmd.counter].try_unload();
                if (op) {
                    try {
                        auto M = std::any_cast<
                            std::function<void(int, std::string_view, std::string_view)>>(
                            std::move(*op));
                        M(0, identifier, "logging callback activated");
                        setLoggerFunction(std::move(M));
                    }
                    catch (const std::bad_any_cast&) {
                        // This shouldn't really happen unless someone is being malicious so just
                        // ignore it for now.
                    }
                }
            }
            break;
        case UPDATE_LOGGING_FILE:
            setLoggingFile(cmd.payload.to_string());
            break;
        case UPDATE_FILTER_OPERATOR:
            if (filterFed != nullptr) {
                filterFed->handleMessage(cmd);
            }
            break;
        default:
            LOG_WARNING(global_broker_id_local,
                        identifier,
                        "unrecognized configure option passed to core ");
            break;
    }
}

void CommonCore::processQueryCommand(ActionMessage& cmd)
{
    bool force_ordered{false};
    switch (cmd.action()) {
        case CMD_BROKER_QUERY_ORDERED:
            force_ordered = true;
            [[fallthrough]];
        case CMD_BROKER_QUERY:

            if (cmd.dest_id == global_broker_id_local || cmd.dest_id == gDirectCoreId) {
                std::string repStr = coreQuery(std::string(cmd.payload.to_string()), force_ordered);
                if (repStr != "#wait") {
                    if (cmd.source_id == gDirectCoreId) {
                        // TODO(PT) make setDelayedValue have a move method
                        activeQueries.setDelayedValue(cmd.messageID, repStr);
                    } else {
                        ActionMessage queryResp(force_ordered ? CMD_QUERY_REPLY_ORDERED :
                                                                CMD_QUERY_REPLY);
                        queryResp.dest_id = cmd.source_id;
                        queryResp.source_id = global_broker_id_local;
                        queryResp.messageID = cmd.messageID;
                        queryResp.payload = std::move(repStr);
                        queryResp.counter = cmd.counter;
                        transmit(getRoute(queryResp.dest_id), queryResp);
                    }
                } else {
                    if (cmd.source_id == gDirectCoreId) {
                        if (queryTimeouts.empty()) {
                            setTickForwarding(TickForwardingReasons::QUERY_TIMEOUT, true);
                        }
                        queryTimeouts.emplace_back(cmd.messageID, std::chrono::steady_clock::now());
                    }
                    ActionMessage queryResp(force_ordered ? CMD_QUERY_REPLY_ORDERED :
                                                            CMD_QUERY_REPLY);
                    queryResp.dest_id = cmd.source_id;
                    queryResp.source_id = global_broker_id_local;
                    queryResp.messageID = cmd.messageID;
                    queryResp.counter = cmd.counter;
                    std::get<1>(
                        mapBuilders[mapIndex.at(std::string(cmd.payload.to_string())).first])
                        .push_back(queryResp);
                }

            } else {
                routeMessage(std::move(cmd));
            }
            break;
        case CMD_QUERY_ORDERED:
            force_ordered = true;
            // FALLTHROUGH
        case CMD_QUERY:
            if (cmd.dest_id == parent_broker_id) {
                if (cmd.source_id == gDirectCoreId) {
                    if (queryTimeouts.empty()) {
                        setTickForwarding(TickForwardingReasons::QUERY_TIMEOUT, true);
                    }
                    queryTimeouts.emplace_back(cmd.messageID, std::chrono::steady_clock::now());
                }
                const auto& target = cmd.getString(targetStringLoc);
                if (target == "root" || target == "federation") {
                    cmd.setAction(force_ordered ? CMD_BROKER_QUERY_ORDERED : CMD_BROKER_QUERY);
                    cmd.dest_id = gRootBrokerID;
                    cmd.clearStringData();
                } else if (target == "parent" || target == "broker") {
                    cmd.setAction(force_ordered ? CMD_BROKER_QUERY_ORDERED : CMD_BROKER_QUERY);
                    cmd.dest_id = higher_broker_id;
                    cmd.clearStringData();
                }
                if (global_broker_id_local != parent_broker_id) {
                    // forward on to Broker
                    cmd.source_id = global_broker_id_local;
                    transmit(parent_route_id, std::move(cmd));
                } else {
                    // this will get processed when this core is assigned a global id
                    cmd.source_id = gDirectCoreId;
                    delayTransmitQueue.push(std::move(cmd));
                }
            } else {
                std::string repStr;
                ActionMessage queryResp(force_ordered ? CMD_QUERY_REPLY_ORDERED : CMD_QUERY_REPLY);
                queryResp.dest_id = cmd.source_id;
                queryResp.source_id = cmd.dest_id;
                queryResp.messageID = cmd.messageID;
                queryResp.counter = cmd.counter;
                const std::string& target = cmd.getString(targetStringLoc);
                if (target == getIdentifier()) {
                    queryResp.source_id = global_broker_id_local;
                    repStr = coreQuery(std::string(cmd.payload.to_string()), force_ordered);
                } else {
                    auto* fedptr = getFederateCore(target);
                    repStr =
                        federateQuery(fedptr, std::string(cmd.payload.to_string()), force_ordered);
                    if (repStr == "#wait") {
                        if (fedptr != nullptr) {
                            cmd.dest_id = fedptr->global_id;
                            fedptr->addAction(std::move(cmd));
                            break;
                        }
                        repStr = "#error";
                    }
                }

                queryResp.payload = std::move(repStr);
                if (queryResp.dest_id == gDirectCoreId) {
                    processQueryResponse(queryResp);
                } else {
                    transmit(getRoute(queryResp.dest_id), queryResp);
                }
            }
            break;
        case CMD_QUERY_REPLY:
        case CMD_QUERY_REPLY_ORDERED:
            if (cmd.dest_id == global_broker_id_local || cmd.dest_id == gDirectCoreId) {
                processQueryResponse(cmd);
            } else {
                transmit(getRoute(cmd.dest_id), cmd);
            }
            break;
        default:
            break;
    }
}

void CommonCore::processCommandsForCore(const ActionMessage& cmd)
{
    if (isTimingCommand(cmd)) {
        if (!enteredExecutionMode) {
            timeCoord->processTimeMessage(cmd);
            auto res = timeCoord->checkExecEntry();
            if (res == MessageProcessingResult::NEXT_STEP) {
                enteredExecutionMode = true;
            }
        } else {
            if (timeCoord->processTimeMessage(cmd)) {
                timeCoord->updateTimeFactors();
            }
        }
        if (isDisconnectCommand(cmd)) {
            if ((cmd.action() == CMD_DISCONNECT) && (cmd.source_id == higher_broker_id)) {
                setBrokerState(BrokerState::terminating);
                if (hasTimeDependency || hasFilters) {
                    timeCoord->disconnect();
                }
                ActionMessage bye(CMD_DISCONNECT_FED_ACK);
                bye.source_id = parent_broker_id;
                loopFederates.apply([&bye](auto& fed) {
                    auto state = fed->getState();
                    if ((HELICS_FINISHED == state) || (HELICS_ERROR == state)) {
                        return;
                    }
                    bye.dest_id = fed->global_id.load();
                    fed->addAction(bye);
                });

                addActionMessage(CMD_STOP);
            } else {
                checkAndProcessDisconnect();
            }
        }
    } else if (isDependencyCommand(cmd)) {
        timeCoord->processDependencyUpdateMessage(cmd);
    } else if (cmd.action() == CMD_TIME_BLOCK || cmd.action() == CMD_TIME_UNBLOCK) {
        manageTimeBlocks(cmd);
    } else if (cmd.action() == CMD_GRANT_TIMEOUT_CHECK) {
        auto v = timeCoord->grantTimeoutCheck(cmd);
        if (!v.isNull()) {
            auto debugString = fileops::generateJsonString(v);
            debugString.insert(0, "TIME DEBUGGING::");
            LOG_WARNING(global_broker_id_local, getIdentifier(), debugString);
        }
    } else {
        LOG_WARNING(global_broker_id_local,
                    getIdentifier(),
                    "dropping message:" + prettyPrintString(cmd));
    }
}

bool CommonCore::hasTimeBlock(GlobalFederateId federateID)
{
    for (auto& tb : timeBlocks) {
        if (federateID == tb.first) {
            return (tb.second != 0);
        }
    }
    return false;
}

bool CommonCore::waitCoreRegistration()
{
    int sleepcnt = 0;
    auto brkid = global_id.load();
    while ((brkid == parent_broker_id) || (!brkid.isValid())) {
        if (sleepcnt > 6) {
            LOG_WARNING(parent_broker_id,
                        identifier,
                        fmt::format("broker state={}, broker id={}, sleepcnt={}",
                                    static_cast<int>(getBrokerState()),
                                    brkid.baseValue(),
                                    sleepcnt));
        }
        if (getBrokerState() <= BrokerState::configured) {
            connect();
        }
        if (getBrokerState() >= BrokerState::terminating) {
            return false;
        }
        if (sleepcnt == 4) {
            LOG_WARNING(parent_broker_id,
                        identifier,
                        "now waiting for the core to finish registration before proceeding");
        }
        if (sleepcnt == 20) {
            LOG_WARNING(parent_broker_id, identifier, "resending reg message");
            ActionMessage M(CMD_RESEND);
            M.messageID = static_cast<int32_t>(CMD_REG_BROKER);
            addActionMessage(M);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        brkid = global_id.load();
        ++sleepcnt;
        if (Time(static_cast<int64_t>(sleepcnt) * 100, time_units::ms) > timeout) {
            return false;
        }
    }
    return true;
}

void CommonCore::manageTimeBlocks(const ActionMessage& command)
{
    if (command.action() == CMD_TIME_BLOCK) {
        bool found{false};
        for (auto& tb : timeBlocks) {
            if (command.source_id == tb.first) {
                ++tb.second;
                found = true;
            }
        }
        if (!found) {
            timeBlocks.emplace_back(command.source_id, 1);
        }
    } else if (command.action() == CMD_TIME_UNBLOCK) {
        for (auto& tb : timeBlocks) {
            if (command.source_id == tb.first) {
                --tb.second;
                if (tb.second <= 0) {
                    tb.second = 0;
                    transmitDelayedMessages(command.source_id);
                }
            }
        }
    }
}

bool CommonCore::checkAndProcessDisconnect()
{
    if ((getBrokerState() >= BrokerState::terminating) &&
        (getBrokerState() <= BrokerState::terminated)) {
        return true;
    }
    if (allDisconnected()) {
        checkInFlightQueriesForDisconnect();
        setBrokerState(BrokerState::terminating);
        timeCoord->disconnect();
        if (enable_profiling) {
            writeProfilingData();
        }

        ActionMessage dis(CMD_DISCONNECT);
        dis.source_id = global_broker_id_local;
        transmit(parent_route_id, dis);
        return true;
    }
    if (hasFilters) {
        if (!filterFed->hasActiveTimeDependencies()) {
            ActionMessage dis(CMD_DISCONNECT);
            dis.source_id = global_broker_id_local;
            transmit(parent_route_id, dis);
            dis.source_id = filterFedID;
            filterFed->handleMessage(dis);
            return true;
        }
    }
    return false;
}

int CommonCore::generateMapObjectCounter() const
{
    int result = static_cast<int>(getBrokerState());
    for (const auto& fed : loopFederates) {
        result += static_cast<int>(fed.state);
    }
    result += static_cast<int>(loopHandles.size());
    return result;
}

void CommonCore::checkInFlightQueriesForDisconnect()
{
    for (auto& mb : mapBuilders) {
        auto& builder = std::get<0>(mb);
        auto& requestors = std::get<1>(mb);
        if (builder.isCompleted()) {
            return;
        }
        if (builder.clearComponents()) {
            auto str = builder.generate();
            for (int ii = 0; ii < static_cast<int>(requestors.size()) - 1; ++ii) {
                if (requestors[ii].dest_id == global_broker_id_local) {
                    activeQueries.setDelayedValue(requestors[ii].messageID, str);
                } else {
                    requestors[ii].payload = str;
                    routeMessage(std::move(requestors[ii]));
                }
            }
            if (requestors.back().dest_id == global_broker_id_local) {
                // TODO(PT) add rvalue reference method
                activeQueries.setDelayedValue(requestors.back().messageID, str);
            } else {
                requestors.back().payload = std::move(str);
                routeMessage(std::move(requestors.back()));
            }

            requestors.clear();
            if (std::get<2>(mb)) {
                builder.reset();
            }
        }
    }
}

void CommonCore::sendDisconnect()
{
    LOG_CONNECTIONS(global_broker_id_local, identifier, "sending disconnect");
    checkInFlightQueriesForDisconnect();
    ActionMessage bye(CMD_STOP);
    bye.source_id = global_broker_id_local;
    for (auto fed : loopFederates) {
        if (fed->getState() != FederateStates::HELICS_FINISHED) {
            fed->addAction(bye);
        }
        if (hasTimeDependency) {
            timeCoord->removeDependency(fed->global_id);
            timeCoord->removeDependent(fed->global_id);
        }
    }
    if (hasTimeDependency) {
        timeCoord->disconnect();
    }
    if (filterFed != nullptr) {
        filterFed->handleMessage(bye);
    }
}

bool CommonCore::checkForLocalPublication(ActionMessage& cmd)
{
    auto* pub = loopHandles.getPublication(cmd.name());
    if (pub != nullptr) {
        // now send the same command to the publication
        cmd.dest_handle = pub->getInterfaceHandle();
        cmd.dest_id = pub->getFederateId();
        setAsUsed(pub);
        // send to
        routeMessage(cmd);
        // now send the notification to the subscription in the federateState
        ActionMessage notice(CMD_ADD_PUBLISHER);
        notice.dest_id = cmd.source_id;
        notice.dest_handle = cmd.source_handle;
        notice.source_id = pub->getFederateId();
        notice.source_handle = pub->getInterfaceHandle();
        notice.setStringData(pub->type, pub->units);
        routeMessage(notice);
        return true;
    }
    return false;
}

void CommonCore::routeMessage(ActionMessage& cmd, GlobalFederateId dest)
{
    if (!dest.isValid()) {
        return;
    }
    cmd.dest_id = dest;
    if ((dest == parent_broker_id) || (dest == higher_broker_id)) {
        transmit(parent_route_id, cmd);
    } else if (dest == global_broker_id_local) {
        processCommandsForCore(cmd);
    } else if (dest == filterFedID) {
        filterFed->handleMessage(cmd);
    } else if (isLocal(dest)) {
        auto* fed = getFederateCore(dest);
        if (fed != nullptr) {
            if (fed->getState() != FederateStates::HELICS_FINISHED) {
                fed->addAction(cmd);
            } else {
                auto rep = fed->processPostTerminationAction(cmd);
                if (rep) {
                    routeMessage(*rep);
                }
            }
        }
    } else {
        auto route = getRoute(dest);
        transmit(route, cmd);
    }
}

void CommonCore::routeMessage(const ActionMessage& cmd)
{
    if ((cmd.dest_id == parent_broker_id) || (cmd.dest_id == higher_broker_id)) {
        transmit(parent_route_id, cmd);
    } else if (cmd.dest_id == global_broker_id_local) {
        processCommandsForCore(cmd);
    } else if (cmd.dest_id == filterFedID) {
        auto ncmd{cmd};
        filterFed->handleMessage(ncmd);
    } else if (isLocal(cmd.dest_id)) {
        auto* fed = getFederateCore(cmd.dest_id);
        if (fed != nullptr) {
            if ((fed->getState() != FederateStates::HELICS_FINISHED) &&
                (fed->getState() != FederateStates::HELICS_ERROR)) {
                fed->addAction(cmd);
            } else {
                auto rep = fed->processPostTerminationAction(cmd);
                if (rep) {
                    routeMessage(*rep);
                }
            }
        }
    } else {
        auto route = getRoute(cmd.dest_id);
        transmit(route, cmd);
    }
}

void CommonCore::routeMessage(ActionMessage&& cmd, GlobalFederateId dest)
{
    if (!dest.isValid()) {
        return;
    }
    cmd.dest_id = dest;
    if ((dest == parent_broker_id) || (dest == higher_broker_id)) {
        transmit(parent_route_id, cmd);
    } else if (cmd.dest_id == global_broker_id_local) {
        processCommandsForCore(cmd);
    } else if (cmd.dest_id == filterFedID) {
        filterFed->handleMessage(cmd);
    } else if (isLocal(dest)) {
        auto* fed = getFederateCore(dest);
        if (fed != nullptr) {
            if (fed->getState() != FederateStates::HELICS_FINISHED) {
                fed->addAction(std::move(cmd));
            } else {
                auto rep = fed->processPostTerminationAction(cmd);
                if (rep) {
                    routeMessage(*rep);
                }
            }
        }
    } else {
        auto route = getRoute(dest);
        transmit(route, cmd);
    }
}

void CommonCore::routeMessage(ActionMessage&& cmd)
{
    GlobalFederateId dest = cmd.dest_id;
    if ((dest == parent_broker_id) || (dest == higher_broker_id)) {
        transmit(parent_route_id, cmd);
    } else if (dest == global_broker_id_local) {
        processCommandsForCore(cmd);
    } else if (dest == filterFedID) {
        filterFed->handleMessage(cmd);
    } else if (isLocal(dest)) {
        auto* fed = getFederateCore(dest);
        if (fed != nullptr) {
            if (fed->getState() != FederateStates::HELICS_FINISHED) {
                fed->addAction(std::move(cmd));
            } else {
                auto rep = fed->processPostTerminationAction(cmd);
                if (rep) {
                    routeMessage(*rep);
                }
            }
        }
    } else {
        auto route = getRoute(dest);
        transmit(route, cmd);
    }
}  // namespace helics

// Checks for filter operations
ActionMessage& CommonCore::processMessage(ActionMessage& m)
{
    auto* handle = loopHandles.getEndpoint(m.source_handle);
    if (handle == nullptr) {
        return m;
    }
    clearActionFlag(m, filter_processing_required_flag);
    if (checkActionFlag(*handle, has_source_filter_flag)) {
        if (filterFed != nullptr) {
            return filterFed->processMessage(m, handle);
        }
    }

    return m;
}

const std::string& CommonCore::getInterfaceInfo(InterfaceHandle handle) const
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo != nullptr) {
        return handleInfo->getTag("local_info_");
    }
    return emptyStr;
}

void CommonCore::setInterfaceInfo(helics::InterfaceHandle handle, std::string info)
{
    handles.modify(
        [&](auto& hdls) { hdls.getHandleInfo(handle.baseValue())->setTag("local_info_", info); });
}

const std::string& CommonCore::getInterfaceTag(InterfaceHandle handle, const std::string& tag) const
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo != nullptr) {
        return handleInfo->getTag(tag);
    }
    return emptyStr;
}

void CommonCore::setInterfaceTag(helics::InterfaceHandle handle,
                                 const std::string& tag,
                                 const std::string& value)
{
    static const std::string trueString{"true"};
    if (tag.empty()) {
        throw InvalidParameter("tag cannot be an empty string for setInterfaceTag");
    }
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo == nullptr) {
        throw InvalidIdentifier("the handle specifier for setInterfaceTag is not valid");
        return;
    }

    const std::string& valueStr{value.empty() ? trueString : value};

    handles.modify(
        [&](auto& hdls) { hdls.getHandleInfo(handle.baseValue())->setTag(tag, valueStr); });

    if (handleInfo != nullptr) {
        ActionMessage tagcmd(CMD_INTERFACE_TAG);
        tagcmd.setSource(handleInfo->handle);
        tagcmd.setDestination(handleInfo->handle);
        tagcmd.setStringData(tag, value);
        addActionMessage(std::move(tagcmd));
    }
}

const std::string& CommonCore::getFederateTag(LocalFederateId fid, const std::string& tag) const
{
    auto* fed = getFederateAt(fid);
    if (fid == gLocalCoreId) {
        static thread_local std::string val;
        val = const_cast<CommonCore*>(this)->query(
            "core",
            std::string("tag/") + tag,
            HelicsSequencingModes::HELICS_SEQUENCING_MODE_ORDERED);
        val = gmlc::utilities::stringOps::removeQuotes(val);
        return val;
    }
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (registerPublication)"));
    }

    return fed->getTag(tag);
}

void CommonCore::setFederateTag(LocalFederateId fid,
                                const std::string& tag,
                                const std::string& value)
{
    static const std::string trueString{"true"};
    if (tag.empty()) {
        throw InvalidParameter("tag cannot be an empty string for setFederateTag");
    }

    if (fid == gLocalCoreId) {
        ActionMessage tagcmd(CMD_CORE_TAG);
        tagcmd.source_id = getGlobalId();
        tagcmd.dest_id = tagcmd.source_id;
        tagcmd.setStringData(tag, value);
        addActionMessage(std::move(tagcmd));
        return;
    }

    auto* fed = getFederateAt(fid);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (setFlag)"));
    }
    fed->setTag(tag, value);
}

}  // namespace helics
