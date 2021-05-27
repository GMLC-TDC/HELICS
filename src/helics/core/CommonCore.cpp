/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "CommonCore.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../common/fmt_format.h"
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
#include "PublicationInfo.hpp"
#include "TimeoutMonitor.h"
#include "core-exceptions.hpp"
#include "coreTypeOperations.hpp"
#include "fileConnections.hpp"
#include "gmlc/concurrency/DelayedObjects.hpp"
#include "helicsVersion.hpp"
#include "helics_definitions.hpp"
#include "loggingHelper.hpp"
#include "queryHelpers.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <functional>
#include <limits>
#include <map>
#include <memory>
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
    if (transitionBrokerState(broker_state_t::created, broker_state_t::configuring)) {
        // initialize the brokerbase
        auto result = parseArgs(configureString);
        if (result != 0) {
            setBrokerState(broker_state_t::created);
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
    if (transitionBrokerState(broker_state_t::created, broker_state_t::configuring)) {
        // initialize the brokerbase
        auto result = parseArgs(argc, argv);
        if (result != 0) {
            setBrokerState(broker_state_t::created);
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
    if (transitionBrokerState(broker_state_t::created, broker_state_t::configuring)) {
        // initialize the brokerbase
        auto result = parseArgs(std::move(args));
        if (result != 0) {
            setBrokerState(broker_state_t::created);
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
    if (cBrokerState == broker_state_t::errored) {
        return false;
    }
    if (cBrokerState >= broker_state_t::configured) {
        if (transitionBrokerState(broker_state_t::configured, broker_state_t::connecting)) {
            timeoutMon->setTimeout(timeout.to_ms());
            bool res = brokerConnect();
            if (res) {
                // now register this core object as a broker

                ActionMessage m(CMD_REG_BROKER);
                m.source_id = global_federate_id{};
                m.name = getIdentifier();
                m.setStringData(getAddress());

                if (!brokerKey.empty()) {
                    m.setString(1, brokerKey);
                }

                setActionFlag(m, core_flag);
                if (no_ping) {
                    setActionFlag(m, slow_responding_flag);
                }
                transmit(parent_route_id, m);
                setBrokerState(broker_state_t::connected);
                disconnection.activate();
            } else {
                setBrokerState(broker_state_t::configured);
            }
            return res;
        }

        LOG_WARNING(global_id.load(), getIdentifier(), "multiple connect calls");
        while (getBrokerState() == broker_state_t::connecting) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    return isConnected();
}

bool CommonCore::isConnected() const
{
    auto currentState = getBrokerState();
    return ((currentState == broker_state_t::operating) ||
            (currentState == broker_state_t::connected));
}

const std::string& CommonCore::getAddress() const
{
    if ((getBrokerState() != broker_state_t::connected) || (address.empty())) {
        address = generateLocalAddressString();
    }
    return address;
}

void CommonCore::processDisconnect(bool skipUnregister)
{
    auto cBrokerState = getBrokerState();
    if (cBrokerState > broker_state_t::configured) {
        if (cBrokerState < broker_state_t::terminating) {
            setBrokerState(broker_state_t::terminating);
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
    setBrokerState(broker_state_t::terminated);
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

FederateState* CommonCore::getFederateAt(local_federate_id federateID) const
{
    /*
    #ifndef __apple_build_version__
        static thread_local FederateState *lastV = nullptr;
        if ((lastV == nullptr) || (lastV->local_id != federateID))
        {
            auto feds = federates.lock ();
            lastV = (*feds)[federateID];
        }
        return lastV;
    #else
    #if __clang_major__ >= 8
        static thread_local FederateState *lastV = nullptr;
        if ((lastV == nullptr) || (lastV->local_id != federateID))
        {
            auto feds = federates.lock ();
            lastV = (*feds)[federateID];
        }
        return lastV;
    #else
        auto feds = federates.lock ();
        return (*feds)[federateID];
    #endif
    #endif
    */
    auto feds = federates.lock();
    return (*feds)[federateID.baseValue()];
}

FederateState* CommonCore::getFederate(const std::string& federateName) const
{
    auto feds = federates.lock();
    return feds->find(federateName);
}

FederateState* CommonCore::getHandleFederate(interface_handle handle)
{
    auto local_fed_id = handles.read([handle](auto& hand) { return hand.getLocalFedID(handle); });
    if (local_fed_id.isValid()) {
        auto feds = federates.lock();
        return (*feds)[local_fed_id.baseValue()];
    }

    return nullptr;
}

FederateState* CommonCore::getFederateCore(global_federate_id federateID)
{
    auto fed = loopFederates.find(federateID);
    return (fed != loopFederates.end()) ? (fed->fed) : nullptr;
}

FederateState* CommonCore::getFederateCore(const std::string& federateName)
{
    auto fed = loopFederates.find(federateName);
    return (fed != loopFederates.end()) ? (fed->fed) : nullptr;
}

FederateState* CommonCore::getHandleFederateCore(interface_handle handle)
{
    auto local_fed_id = handles.read([handle](auto& hand) { return hand.getLocalFedID(handle); });
    if (local_fed_id.isValid()) {
        return loopFederates[local_fed_id.baseValue()].fed;
    }

    return nullptr;
}

const BasicHandleInfo* CommonCore::getHandleInfo(interface_handle handle) const
{
    return handles.read([handle](auto& hand) { return hand.getHandleInfo(handle.baseValue()); });
}

const BasicHandleInfo* CommonCore::getLocalEndpoint(const std::string& name) const
{
    return handles.read([&name](auto& hand) { return hand.getEndpoint(name); });
}

bool CommonCore::isLocal(global_federate_id global_fedid) const
{
    return (loopFederates.find(global_fedid) != loopFederates.end());
}

route_id CommonCore::getRoute(global_federate_id global_fedid) const
{
    auto fnd = routing_table.find(global_fedid);
    return (fnd != routing_table.end()) ? fnd->second : parent_route_id;
}

bool CommonCore::isConfigured() const
{
    return (getBrokerState() >= broker_state_t::configured);
}

bool CommonCore::isOpenToNewFederates() const
{
    auto cBrokerState = getBrokerState();
    return ((cBrokerState != broker_state_t::created) &&
            (cBrokerState < broker_state_t::operating) &&
            (maxFederateCount == std::numeric_limits<int32_t>::max() ||
             (federates.lock_shared()->size() < static_cast<size_t>(maxFederateCount))));
}

bool CommonCore::hasError() const
{
    return getBrokerState() == broker_state_t::errored;
}
void CommonCore::globalError(local_federate_id federateID,
                             int errorCode,
                             const std::string& errorString)
{
    if (federateID == local_core_id) {
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
    iteration_result ret = iteration_result::next_step;
    while (ret != iteration_result::error) {
        ret = fed->genericUnspecifiedQueueProcess();
        if (ret == iteration_result::halted) {
            break;
        }
    }
}

void CommonCore::localError(local_federate_id federateID,
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
    iteration_result ret = iteration_result::next_step;
    while (ret != iteration_result::error) {
        ret = fed->genericUnspecifiedQueueProcess();
        if (ret == iteration_result::halted) {
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
    lastErrorCode.load();
    return lastErrorString;
}

void CommonCore::finalize(local_federate_id federateID)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid finalize"));
    }
    ActionMessage bye(CMD_DISCONNECT);
    bye.source_id = fed->global_id.load();
    bye.dest_id = bye.source_id;
    addActionMessage(bye);
    fed->finalize();
}

bool CommonCore::allInitReady() const
{
    if (delayInitCounter > 0) {
        return false;
    }
    // the federate count must be greater than the min size
    if (static_cast<decltype(minFederateCount)>(loopFederates.size()) < minFederateCount) {
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

void CommonCore::setCoreReadyToInit()
{
    // use the flag mechanics that do the same thing
    setFlagOption(local_core_id, defs::flags::enable_init_entry);
}

/** this function will generate an appropriate exception for the error
code listed in a Federate*/
static void generateFederateException(const FederateState* fed)
{
    auto eCode = fed->lastErrorCode();
    switch (eCode) {
        case 0:
            return;
        case defs::errors::invalid_argument:
            throw(InvalidParameter(fed->lastErrorString()));
        case defs::errors::invalid_function_call:
            throw(InvalidFunctionCall(fed->lastErrorString()));
        case defs::errors::invalid_object:
            throw(InvalidIdentifier(fed->lastErrorString()));
        case defs::errors::invalid_state_transition:
            throw(InvalidFunctionCall(fed->lastErrorString()));
        case defs::errors::connection_failure:
            throw(ConnectionFailure(fed->lastErrorString()));
        case defs::errors::registration_failure:
            throw(RegistrationFailure(fed->lastErrorString()));
        default:
            throw(HelicsException(fed->lastErrorString()));
    }
}
void CommonCore::enterInitializingMode(local_federate_id federateID)
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
        if (check != iteration_result::next_step) {
            fed->init_requested = false;
            if (check == iteration_result::halted) {
                throw(HelicsSystemFailure());
            }
            generateFederateException(fed);
        }
        return;
    }
    throw(InvalidFunctionCall("federate already has requested entry to initializing State"));
}

iteration_result CommonCore::enterExecutingMode(local_federate_id federateID,
                                                iteration_request iterate)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (EnterExecutingState)"));
    }
    if (HELICS_EXECUTING == fed->getState()) {
        return iteration_result::next_step;
    }
    if (HELICS_INITIALIZING != fed->getState()) {
        throw(InvalidFunctionCall("federate is in invalid state for calling entry to exec mode"));
    }
    // do an exec check on the fed to process previously received messages so it can't get in a
    // deadlocked state
    ActionMessage execc(CMD_EXEC_CHECK);
    fed->addAction(execc);

    ActionMessage exec(CMD_EXEC_REQUEST);
    exec.source_id = fed->global_id.load();
    exec.dest_id = fed->global_id.load();
    setIterationFlags(exec, iterate);
    setActionFlag(exec, indicator_flag);
    addActionMessage(exec);

    // TODO(PT): check for error conditions?
    return fed->enterExecutingMode(iterate, false);
}

local_federate_id CommonCore::registerFederate(const std::string& name,
                                               const CoreFederateInfo& info)
{
    if (!waitCoreRegistration()) {
        if (getBrokerState() == broker_state_t::errored) {
            if (!lastErrorString.empty()) {
                throw(RegistrationFailure(lastErrorString));
            }
        }
        throw(RegistrationFailure(
            "core is unable to register and has timed out, federate cannot be registered"));
    }
    if (getBrokerState() >= broker_state_t::operating) {
        throw(RegistrationFailure("Core has already moved to operating state"));
    }
    FederateState* fed = nullptr;
    bool checkProperties{false};
    local_federate_id local_id;
    {
        auto feds = federates.lock();
        if (static_cast<decltype(maxFederateCount)>(feds->size()) >= maxFederateCount) {
            throw(RegistrationFailure("maximum number of federates in the core has been reached"));
        }
        auto id = feds->insert(name, name, info);
        if (id) {
            local_id = local_federate_id(static_cast<int32_t>(*id));
            fed = (*feds)[*id];
        } else {
            throw(RegistrationFailure("duplicate names " + name +
                                      " detected multiple federates with the same name"));
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
    fed->setLogger([this](int level, const std::string& ident, const std::string& message) {
        sendToLogger(parent_broker_id, log_level::fed + level, ident, message);
    });

    fed->local_id = local_id;
    fed->setParent(this);

    ActionMessage m(CMD_REG_FED);
    m.name = name;
    addActionMessage(m);
    // check some properties that should be inherited from the federate if it is the first one
    if (checkProperties) {
        // if this is the first federate then the core should inherit the logging level properties
        for (const auto& prop : info.intProps) {
            switch (prop.first) {
                case defs::properties::log_level:
                case defs::properties::file_log_level:
                case defs::properties::console_log_level:
                    setIntegerProperty(local_core_id,
                                       prop.first,
                                       static_cast<int16_t>(prop.second));
                default:
                    break;
            }
        }
    }
    // now wait for the federateQueue to get the response
    auto valid = fed->waitSetup();
    if (valid == iteration_result::next_step) {
        return local_id;
    }
    throw(RegistrationFailure(std::string("fed received Failure ") + fed->lastErrorString()));
}

const std::string& CommonCore::getFederateName(local_federate_id federateID) const
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (federateName)"));
    }
    return fed->getIdentifier();
}

static const std::string unknownString("#unknown");

const std::string& CommonCore::getFederateNameNoThrow(global_federate_id federateID) const noexcept
{
    static const std::string filterString = getIdentifier() + "_filters";

    auto* fed = getFederateAt(local_federate_id(federateID.localIndex()));
    return (fed == nullptr) ? ((federateID == filterFedID) ? filterString : unknownString) :
                              fed->getIdentifier();
}

local_federate_id CommonCore::getFederateId(const std::string& name) const
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
    if (getBrokerState() >= broker_state_t::operating) {
        return _global_federation_size;
    }
    // if we are in initialization return the local federation size
    return static_cast<int32_t>(federates.lock()->size());
}

Time CommonCore::timeRequest(local_federate_id federateID, Time next)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid timeRequest"));
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
            auto ret = fed->requestTime(next, iteration_request::no_iterations, false);
            switch (ret.state) {
                case iteration_result::error:
                    throw(FunctionExecutionFailure(fed->lastErrorString()));
                case iteration_result::halted:
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

iteration_time CommonCore::requestTimeIterative(local_federate_id federateID,
                                                Time next,
                                                iteration_request iterate)
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
            return iteration_time{Time::maxVal(), iteration_result::halted};
        case HELICS_CREATED:
        case HELICS_INITIALIZING:
            return iteration_time{timeZero, iteration_result::error};
        case HELICS_UNKNOWN:
        case HELICS_ERROR:
            return iteration_time{Time::maxVal(), iteration_result::error};
    }

    // limit the iterations
    if (iterate == iteration_request::iterate_if_needed) {
        if (fed->getCurrentIteration() >= maxIterationCount) {
            iterate = iteration_request::no_iterations;
        }
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

Time CommonCore::getCurrentTime(local_federate_id federateID) const
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw InvalidIdentifier("federateID not valid (getCurrentTime)");
    }
    return fed->grantedTime();
}

uint64_t CommonCore::getCurrentReiteration(local_federate_id federateID) const
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw InvalidIdentifier("federateID not valid (getCurrentReiteration)");
    }
    return fed->getCurrentIteration();
}

void CommonCore::setIntegerProperty(local_federate_id federateID,
                                    int32_t property,
                                    int16_t propertyValue)
{
    if (federateID == local_core_id) {
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

void CommonCore::setTimeProperty(local_federate_id federateID, int32_t property, Time time)
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

Time CommonCore::getTimeProperty(local_federate_id federateID, int32_t property) const
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (setTimeDelta)"));
    }
    return fed->getTimeProperty(property);
}

int16_t CommonCore::getIntegerProperty(local_federate_id federateID, int32_t property) const
{
    if (federateID == local_core_id) {
        if (property == helics_property_int_log_level ||
            property == helics_property_int_console_log_level) {
            return consoleLogLevel;
        }
        if (property == helics_property_int_file_log_level) {
            return fileLogLevel;
        }
        return 0;
    }
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (setTimeDelta)"));
    }
    return fed->getIntegerProperty(property);
}

void CommonCore::setFlagOption(local_federate_id federateID, int32_t flag, bool flagValue)
{
    if (flag == defs::flags::dumplog || flag == defs::flags::force_logging_flush) {
        ActionMessage cmd(CMD_BASE_CONFIGURE);
        cmd.messageID = flag;
        if (flagValue) {
            setActionFlag(cmd, indicator_flag);
        }
        addActionMessage(cmd);
    }
    if (federateID == local_core_id) {
        if (flag == defs::flags::delay_init_entry) {
            if (flagValue) {
                ++delayInitCounter;
            } else {
                ActionMessage cmd(CMD_CORE_CONFIGURE);
                cmd.messageID = defs::flags::delay_init_entry;
                addActionMessage(cmd);
            }
        } else {
            ActionMessage cmd(CMD_CORE_CONFIGURE);
            cmd.messageID = flag;
            if (flagValue) {
                setActionFlag(cmd, indicator_flag);
            }
            addActionMessage(cmd);
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

bool CommonCore::getFlagOption(local_federate_id federateID, int32_t flag) const
{
    switch (flag) {
        case defs::flags::enable_init_entry:
            return (delayInitCounter.load() == 0);
        case defs::flags::delay_init_entry:
            return (delayInitCounter.load() != 0);
        case defs::flags::dumplog:
        case defs::flags::force_logging_flush:
        case defs::flags::debugging:
            return getFlagValue(flag);
        case defs::flags::forward_compute:
        case defs::flags::single_thread_federate:
        case defs::flags::rollback:
            return false;
        default:
            break;
    }

    if (federateID == local_core_id) {
        return false;
    }
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (setTimeDelta)"));
    }
    return fed->getOptionFlag(flag);
}

const BasicHandleInfo& CommonCore::createBasicHandle(global_federate_id global_federateId,
                                                     local_federate_id local_federateId,
                                                     handle_type HandleType,
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

interface_handle CommonCore::registerInput(local_federate_id federateID,
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
                                           handle_type::input,
                                           key,
                                           type,
                                           units,
                                           fed->getInterfaceFlags());

    auto id = handle.getInterfaceHandle();
    fed->createInterface(handle_type::input, id, key, type, units);

    LOG_INTERFACES(parent_broker_id,
                   fed->getIdentifier(),
                   fmt::format("registering Input {}", key));
    ActionMessage m(CMD_REG_INPUT);
    m.source_id = fed->global_id.load();
    m.source_handle = id;
    m.flags = handle.flags;
    m.name = key;
    m.setStringData(type, units);

    actionQueue.push(std::move(m));
    return id;
}

interface_handle CommonCore::getInput(local_federate_id federateID, const std::string& key) const
{
    const auto* ci = handles.read([&key](auto& hand) { return hand.getInput(key); });
    if (ci->local_fed_id != federateID) {
        return {};
    }
    return ci->getInterfaceHandle();
}

interface_handle CommonCore::registerPublication(local_federate_id federateID,
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
                                           handle_type::publication,
                                           key,
                                           type,
                                           units,
                                           fed->getInterfaceFlags());

    auto id = handle.handle.handle;
    fed->createInterface(handle_type::publication, id, key, type, units);

    ActionMessage m(CMD_REG_PUB);
    m.source_id = fed->global_id.load();
    m.source_handle = id;
    m.name = key;
    m.flags = handle.flags;
    m.setStringData(type, units);

    actionQueue.push(std::move(m));
    return id;
}

interface_handle CommonCore::getPublication(local_federate_id federateID,
                                            const std::string& key) const
{
    const auto* pub = handles.read([&key](auto& hand) { return hand.getPublication(key); });
    if (pub->local_fed_id != federateID) {
        return {};
    }
    return pub->getInterfaceHandle();
}

const std::string emptyStr;

const std::string& CommonCore::getHandleName(interface_handle handle) const
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo != nullptr) {
        return handleInfo->key;
    }
    return emptyStr;
}

const std::string& CommonCore::getInjectionUnits(interface_handle handle) const
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo != nullptr) {
        switch (handleInfo->handleType) {
            case handle_type::input: {
                auto* fed = getFederateAt(handleInfo->local_fed_id);
                auto* inpInfo = fed->interfaces().getInput(handle);
                if (inpInfo != nullptr) {
                    return inpInfo->getInjectionUnits();
                }
                break;
            }
            case handle_type::publication:
                return handleInfo->units;
            default:
                return emptyStr;
        }
    }
    return emptyStr;
}  // namespace helics

const std::string& CommonCore::getExtractionUnits(interface_handle handle) const
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo != nullptr) {
        switch (handleInfo->handleType) {
            case handle_type::input:
            case handle_type::publication:
                return handleInfo->units;
            default:
                break;
        }
    }
    return emptyStr;
}

const std::string& CommonCore::getInjectionType(interface_handle handle) const
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo != nullptr) {
        switch (handleInfo->handleType) {
            case handle_type::input: {
                auto* fed = getFederateAt(handleInfo->local_fed_id);
                auto* inpInfo = fed->interfaces().getInput(handle);
                if (inpInfo != nullptr) {
                    return inpInfo->getInjectionType();
                }
                break;
            }
            case handle_type::endpoint:
                return handleInfo->type;
            case handle_type::filter:
                return handleInfo->type_in;
            default:
                return emptyStr;
        }
    }
    return emptyStr;
}

const std::string& CommonCore::getExtractionType(interface_handle handle) const
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo != nullptr) {
        switch (handleInfo->handleType) {
            case handle_type::publication:
            case handle_type::input:
            case handle_type::endpoint:
                return handleInfo->type;
            case handle_type::filter:
                return handleInfo->type_out;
            default:
                return emptyStr;
        }
    }
    return emptyStr;
}

void CommonCore::setHandleOption(interface_handle handle, int32_t option, int32_t option_value)
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
    if (handleInfo->handleType != handle_type::filter) {
        auto* fed = getHandleFederate(handle);
        if (fed != nullptr) {
            fcn.dest_id = fed->global_id;
            fed->setProperties(fcn);
        }
    } else {
        // must be for filter
    }
}

int32_t CommonCore::getHandleOption(interface_handle handle, int32_t option) const
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo == nullptr) {
        return 0;
    }
    switch (option) {
        case defs::options::connection_required:
        case defs::options::connection_optional:
            return handles.read(
                [handle, option](auto& hand) { return hand.getHandleOption(handle, option); });
        default:
            break;
    }
    if (handleInfo->handleType != handle_type::filter) {
        auto* fed = getFederateAt(handleInfo->local_fed_id);
        if (fed != nullptr) {
            return fed->getHandleOption(handle, static_cast<char>(handleInfo->handleType), option);
        }
    } else {
        // must be for filter
    }
    return 0;
}

void CommonCore::closeHandle(interface_handle handle)
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

void CommonCore::removeTarget(interface_handle handle, const std::string& targetToRemove)
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo == nullptr) {
        throw(InvalidIdentifier("invalid handle"));
    }

    ActionMessage cmd;
    cmd.setSource(handleInfo->handle);
    cmd.name = targetToRemove;
    auto* fed = getFederateAt(handleInfo->local_fed_id);
    if (fed != nullptr) {
        cmd.actionTime = fed->grantedTime();
    }
    switch (handleInfo->handleType) {
        case handle_type::publication:
            cmd.setAction(CMD_REMOVE_NAMED_INPUT);
            break;
        case handle_type::filter:
            cmd.setAction(CMD_REMOVE_NAMED_ENDPOINT);
            break;
        case handle_type::input:
            cmd.setAction(CMD_REMOVE_NAMED_PUBLICATION);
            fed->addAction(cmd);
            break;
        case handle_type::endpoint:
            cmd.setAction(CMD_REMOVE_NAMED_FILTER);
            break;
        default:
            return;
    }
    addActionMessage(std::move(cmd));
}

void CommonCore::addDestinationTarget(interface_handle handle, const std::string& dest)
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo == nullptr) {
        throw(InvalidIdentifier("invalid handle"));
    }
    ActionMessage cmd;
    cmd.setSource(handleInfo->handle);
    cmd.flags = handleInfo->flags;
    setActionFlag(cmd, destination_target);
    cmd.payload = dest;
    switch (handleInfo->handleType) {
        case handle_type::endpoint:
            cmd.setAction(CMD_ADD_NAMED_FILTER);
            break;
        case handle_type::filter:
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
        case handle_type::publication:
            cmd.setAction(CMD_ADD_NAMED_INPUT);
            if (handleInfo->key.empty()) {
                cmd.setStringData(handleInfo->type, handleInfo->units);
            }
            break;
        case handle_type::input:
        default:
            throw(InvalidIdentifier("inputs cannot have destination targets"));
    }

    addActionMessage(std::move(cmd));
}

void CommonCore::addSourceTarget(interface_handle handle, const std::string& targetName)
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
        case handle_type::endpoint:
            cmd.setAction(CMD_ADD_NAMED_FILTER);
            break;
        case handle_type::filter:
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
        case handle_type::input:
            cmd.setAction(CMD_ADD_NAMED_PUBLICATION);
            break;
        case handle_type::publication:
        default:
            throw(InvalidIdentifier("publications cannot have source targets"));
    }
    addActionMessage(std::move(cmd));
}

void CommonCore::setValue(interface_handle handle, const char* data, uint64_t len)
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo == nullptr) {
        throw(InvalidIdentifier("Handle not valid (setValue)"));
    }
    if (handleInfo->handleType != handle_type::publication) {
        throw(InvalidIdentifier("handle does not point to a publication or control output"));
    }
    if (checkActionFlag(*handleInfo, disconnected_flag)) {
        return;
    }
    if (!handleInfo->used) {
        return;  // if the value is not required do nothing
    }
    auto* fed = getFederateAt(handleInfo->local_fed_id);
    if (fed->checkAndSetValue(handle, data, len)) {
        if (fed->loggingLevel() >= helics_log_level_data) {
            fed->logMessage(helics_log_level_data,
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
            mv.payload = std::string(data, len);
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
        mv.payload = std::string(data, len);
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

const std::shared_ptr<const data_block>& CommonCore::getValue(interface_handle handle,
                                                              uint32_t* inputIndex)
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo == nullptr) {
        throw(InvalidIdentifier("Handle is invalid (getValue)"));
    }

    if (handleInfo->handleType != handle_type::input) {
        throw(InvalidIdentifier("Handle does not identify an input"));
    }
    auto& fed = *getFederateAt(handleInfo->local_fed_id);
    std::lock_guard<FederateState> lk(fed);
    return fed.getValue(handle, inputIndex);
}

const std::vector<std::shared_ptr<const data_block>>&
    CommonCore::getAllValues(interface_handle handle)
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo == nullptr) {
        throw(InvalidIdentifier("Handle is invalid (getValue)"));
    }

    if (handleInfo->handleType != handle_type::input) {
        throw(InvalidIdentifier("Handle does not identify an input"));
    }
    auto& fed = *getFederateAt(handleInfo->local_fed_id);
    std::lock_guard<FederateState> lk(fed);
    return fed.getAllValues(handle);
}

const std::vector<interface_handle>& CommonCore::getValueUpdates(local_federate_id federateID)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (getValueUpdates)"));
    }
    return fed->getEvents();
}

interface_handle CommonCore::registerEndpoint(local_federate_id federateID,
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
                                           handle_type::endpoint,
                                           name,
                                           type,
                                           std::string{},
                                           fed->getInterfaceFlags());

    auto id = handle.getInterfaceHandle();
    fed->createInterface(handle_type::endpoint, id, name, type, emptyStr);
    ActionMessage m(CMD_REG_ENDPOINT);
    m.source_id = fed->global_id.load();
    m.source_handle = id;
    m.name = name;
    m.setStringData(type);
    m.flags = handle.flags;
    actionQueue.push(std::move(m));

    return id;
}

interface_handle CommonCore::getEndpoint(local_federate_id federateID,
                                         const std::string& name) const
{
    const auto* ept = handles.read([&name](auto& hand) { return hand.getEndpoint(name); });
    if (ept->local_fed_id != federateID) {
        return {};
    }
    return ept->handle.handle;
}

interface_handle CommonCore::registerFilter(const std::string& filterName,
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
        if (getBrokerState() >= broker_state_t::terminating) {
            throw(RegistrationFailure("core is terminated no further registration possible"));
        }
        throw(RegistrationFailure("registration timeout exceeded"));
    }
    auto fid = filterFedID.load();

    auto handle = createBasicHandle(
        fid, local_federate_id(), handle_type::filter, filterName, type_in, type_out);
    auto id = handle.getInterfaceHandle();

    ActionMessage m(CMD_REG_FILTER);
    m.source_id = fid;
    m.source_handle = id;
    m.name = handle.key;
    if ((!type_in.empty()) || (!type_out.empty())) {
        m.setStringData(type_in, type_out);
    }
    actionQueue.push(std::move(m));
    return id;
}

interface_handle CommonCore::registerCloningFilter(const std::string& filterName,
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
        if (getBrokerState() >= broker_state_t::terminating) {
            throw(RegistrationFailure("core is terminated no further registration possible"));
        }
        throw(RegistrationFailure("registration timeout exceeded"));
    }
    auto fid = filterFedID.load();

    const auto& handle = createBasicHandle(fid,
                                           local_federate_id(),
                                           handle_type::filter,
                                           filterName,
                                           type_in,
                                           type_out,
                                           make_flags(clone_flag));

    auto id = handle.getInterfaceHandle();

    ActionMessage m(CMD_REG_FILTER);
    m.source_id = fid;
    m.source_handle = id;
    m.name = handle.key;
    setActionFlag(m, clone_flag);
    if ((!type_in.empty()) || (!type_out.empty())) {
        m.setStringData(type_in, type_out);
    }
    actionQueue.push(std::move(m));
    return id;
}

interface_handle CommonCore::getFilter(const std::string& name) const
{
    const auto* filt = handles.read([&name](auto& hand) { return hand.getFilter(name); });
    if ((filt != nullptr) && (filt->handleType == handle_type::filter)) {
        return filt->getInterfaceHandle();
    }
    return {};
}

void CommonCore::registerFrequentCommunicationsPair(const std::string& /*source*/,
                                                    const std::string& /*dest*/)
{
    // std::lock_guard<std::mutex> lock (_mutex);
}

void CommonCore::makeConnections(const std::string& file)
{
    if (hasTomlExtension(file)) {
        makeConnectionsToml(this, file);
    } else {
        makeConnectionsJson(this, file);
    }
}

void CommonCore::dataLink(const std::string& source, const std::string& target)
{
    ActionMessage M(CMD_DATA_LINK);
    M.name = source;
    M.setStringData(target);
    addActionMessage(std::move(M));
}

void CommonCore::addSourceFilterToEndpoint(const std::string& filter, const std::string& endpoint)
{
    ActionMessage M(CMD_FILTER_LINK);
    M.name = filter;
    M.setStringData(endpoint);
    addActionMessage(std::move(M));
}

void CommonCore::addDestinationFilterToEndpoint(const std::string& filter,
                                                const std::string& endpoint)
{
    ActionMessage M(CMD_FILTER_LINK);
    M.name = filter;
    M.setStringData(endpoint);
    setActionFlag(M, destination_target);
    addActionMessage(std::move(M));
}

void CommonCore::addDependency(local_federate_id federateID, const std::string& federateName)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (registerDependency)"));
    }
    ActionMessage search(CMD_SEARCH_DEPENDENCY);
    search.source_id = fed->global_id.load();
    search.name = federateName;
    addActionMessage(std::move(search));
}

void CommonCore::send(interface_handle sourceHandle,
                      const std::string& destination,
                      const char* data,
                      uint64_t length)
{
    const auto* hndl = getHandleInfo(sourceHandle);
    if (hndl == nullptr) {
        throw(InvalidIdentifier("handle is not valid"));
    }

    if (hndl->handleType != handle_type::endpoint) {
        throw(InvalidIdentifier("handle does not point to an endpoint"));
    }
    auto* fed = getFederateAt(hndl->local_fed_id);
    ActionMessage m(CMD_SEND_MESSAGE);

    m.messageID = ++messageCounter;
    m.source_handle = sourceHandle;
    m.source_id = hndl->getFederateId();
    m.flags = hndl->flags;
    m.payload = std::string(data, length);
    m.setStringData(destination, hndl->key, hndl->key);
    m.actionTime = fed->nextAllowedSendTime();
    addActionMessage(std::move(m));
}

void CommonCore::sendEvent(Time time,
                           interface_handle sourceHandle,
                           const std::string& destination,
                           const char* data,
                           uint64_t length)
{
    const auto* hndl = getHandleInfo(sourceHandle);
    if (hndl == nullptr) {
        throw(InvalidIdentifier("handle is not valid"));
    }
    if (hndl->handleType != handle_type::endpoint) {
        throw(InvalidIdentifier("handle does not point to an endpoint"));
    }
    ActionMessage m(CMD_SEND_MESSAGE);
    m.source_handle = sourceHandle;
    m.source_id = hndl->getFederateId();
    auto minTime = getFederateAt(hndl->local_fed_id)->nextAllowedSendTime();
    m.actionTime = std::max(time, minTime);
    m.flags = hndl->flags;
    m.payload = std::string(data, length);
    m.setStringData(destination, hndl->key, hndl->key);
    m.messageID = ++messageCounter;
    addActionMessage(std::move(m));
}

void CommonCore::sendMessage(interface_handle sourceHandle, std::unique_ptr<Message> message)
{
    if (sourceHandle == direct_send_handle) {
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
    if (hndl->handleType != handle_type::endpoint) {
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

    if (fed->loggingLevel() >= helics_log_level_data) {
        fed->logMessage(helics_log_level_data,
                        "",
                        fmt::format("receive_message {}", prettyPrintString(m)));
    }
    addActionMessage(std::move(m));
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

uint64_t CommonCore::receiveCount(interface_handle destination)
{
    auto* fed = getHandleFederate(destination);
    if (fed == nullptr) {
        return 0;
    }
    return fed->getQueueSize(destination);
}

std::unique_ptr<Message> CommonCore::receive(interface_handle destination)
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

std::unique_ptr<Message> CommonCore::receiveAny(local_federate_id federateID,
                                                interface_handle& endpoint_id)
{
    auto* fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("FederateID is not valid (receiveAny)"));
    }
    if (fed->getState() == HELICS_CREATED) {
        endpoint_id = interface_handle();
        return nullptr;
    }
    return fed->receiveAny(endpoint_id);
}

uint64_t CommonCore::receiveCountAny(local_federate_id federateID)
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

void CommonCore::logMessage(local_federate_id federateID,
                            int logLevel,
                            const std::string& messageToLog)
{
    global_federate_id gid;
    if (federateID == local_core_id) {
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
    cmd.messageID = defs::properties::log_level;
    cmd.setExtraData(logLevel);
    addActionMessage(cmd);
}

void CommonCore::setLogFile(const std::string& lfile)
{
    setLoggingFile(lfile);
}

void CommonCore::setLoggingCallback(
    local_federate_id federateID,
    std::function<void(int, const std::string&, const std::string&)> logFunction)
{
    if (federateID == local_core_id) {
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

void CommonCore::setFilterOperator(interface_handle filter,
                                   std::shared_ptr<FilterOperator> callback)
{
    static std::shared_ptr<FilterOperator> nullFilt = std::make_shared<NullFilterOperator>();
    const auto* hndl = getHandleInfo(filter);
    if (hndl == nullptr) {
        throw(InvalidIdentifier("filter is not a valid handle"));
    }
    if ((hndl->handleType != handle_type::filter)) {
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
    if (getBrokerState() == broker_state_t::created) {
        identifier = name;
    } else {
        throw(
            InvalidFunctionCall("setIdentifier can only be called before the core is initialized"));
    }
}

// enumeration of subqueries that cascade and need multiple levels of processing
enum subqueries : std::uint16_t {
    general_query = 0,
    current_time_map = 2,
    dependency_graph = 3,
    data_flow_graph = 4,
    global_state = 6,
    global_time_debugging = 7,
    global_flush = 8,
    global_status = 9
};

static const std::map<std::string, std::pair<std::uint16_t, bool>> mapIndex{
    {"global_time", {current_time_map, true}},
    {"global_status", {global_status, true}},
    {"dependency_graph", {dependency_graph, false}},
    {"data_flow_graph", {data_flow_graph, false}},
    {"global_state", {global_state, true}},
    {"global_time_debugging", {global_time_debugging, true}},
    {"global_flush", {global_flush, true}},
};

void CommonCore::setQueryCallback(local_federate_id federateID,
                                  std::function<std::string(const std::string&)> queryFunction)
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
    return generateJsonString(base);
}

std::string CommonCore::federateQuery(const FederateState* fed,
                                      const std::string& queryStr,
                                      bool force_ordering) const
{
    if (fed == nullptr) {
        if (queryStr == "exists") {
            return "false";
        }
        return "#invalid";
    }
    if (queryStr == "exists") {
        return "true";
    }
    if (queryStr == "version") {
        return versionString;
    }
    if (queryStr == "isinit") {
        return (fed->init_transmitted.load()) ? "true" : "false";
    }
    if (queryStr == "state") {
        if (!force_ordering) {
            return fedStateString(fed->getState());
        }
    }
    if (queryStr == "filtered_endpoints") {
        if (!force_ordering) {
            return filteredEndpointQuery(fed);
        }
    }
    if ((queryStr == "queries") || (queryStr == "available_queries")) {
        return std::string("[exists;isinit;state;version;queries;filtered_endpoints;") +
            fed->processQuery(queryStr) + "]";
    }
    return fed->processQuery(queryStr, force_ordering);
}

std::string CommonCore::quickCoreQueries(const std::string& queryStr) const
{
    if ((queryStr == "queries") || (queryStr == "available_queries")) {
        return "[isinit;isconnected;exists;name;identifier;address;queries;address;federates;inputs;endpoints;filtered_endpoints;"
               "publications;filters;version;version_all;counter;federate_map;dependency_graph;data_flow_graph;dependencies;dependson;dependents;current_time;global_time;global_state;current_state;global_flush]";
    }
    if (queryStr == "isconnected") {
        return (isConnected()) ? "true" : "false";
    }
    if (queryStr == "name" || queryStr == "identifier") {
        return getIdentifier();
    }
    if (queryStr == "exists") {
        return "true";
    }
    if (queryStr == "version") {
        return versionString;
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
        mapBuilders.resize(index + 1);
    }
    std::get<2>(mapBuilders[index]) = reset;
    auto& builder = std::get<0>(mapBuilders[index]);
    builder.reset();
    Json::Value& base = builder.getJValue();
    base["name"] = getIdentifier();
    base["id"] = global_broker_id_local.baseValue();
    base["parent"] = higher_broker_id.baseValue();
    ActionMessage queryReq(force_ordering ? CMD_QUERY_ORDERED : CMD_QUERY);
    if (index == global_flush) {
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
                if (fed->getState() <= federate_state::HELICS_EXECUTING) {
                    queryReq.messageID = brkindex;
                    queryReq.dest_id = fed.fed->global_id;
                    fed.fed->addAction(queryReq);
                } else {
                    builder.addComponent("", brkindex);
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
        case current_time_map:
        case global_status:
            if (hasTimeDependency) {
                base["next_time"] = static_cast<double>(timeCoord->getNextTime());
            }
            break;
        case dependency_graph: {
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
        case global_state:
            base["state"] = brokerStateName(getBrokerState());
            break;
        case global_time_debugging:
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

std::string CommonCore::coreQuery(const std::string& queryStr, bool force_ordering) const
{
    auto res = quickCoreQueries(queryStr);
    if (!res.empty()) {
        return res;
    }
    if (queryStr == "federates") {
        return generateStringVector(loopFederates,
                                    [](const auto& fed) { return fed->getIdentifier(); });
    }
    if (queryStr == "publications") {
        return generateStringVector_if(
            loopHandles,
            [](const auto& handle) { return handle.key; },
            [](const auto& handle) { return (handle.handleType == handle_type::publication); });
    }
    if (queryStr == "inputs") {
        return generateStringVector_if(
            loopHandles,
            [](const auto& handle) { return handle.key; },
            [](const auto& handle) {
                return ((handle.handleType == handle_type::input) && !handle.key.empty());
            });
    }
    if (queryStr == "filters") {
        return generateStringVector_if(
            loopHandles,
            [](const auto& handle) { return handle.key; },
            [](const auto& handle) { return (handle.handleType == handle_type::filter); });
    }

    if (queryStr == "endpoints") {
        return generateStringVector_if(
            loopHandles,
            [](const auto& handle) { return handle.key; },
            [](const auto& handle) { return (handle.handleType == handle_type::endpoint); });
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

    if (queryStr == "address") {
        return getAddress();
    }
    if (queryStr == "counter") {
        return fmt::format("{}", generateMapObjectCounter());
    }
    if (queryStr == "filtered_endpoints" || queryStr == "endpoint_filters") {
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
        return generateJsonString(base);
    }
    if (queryStr == "current_state") {
        Json::Value base;
        loadBasicJsonInfo(base, [](Json::Value& val, const FedInfo& fed) {
            val["state"] = state_string(fed.state);
        });
        base["state"] = brokerStateName(getBrokerState());

        return generateJsonString(base);
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
    if (queryStr == "global_time") {
        Json::Value base;
        loadBasicJsonInfo(base, [](Json::Value& val, const FedInfo& fed) {
            val["granted_time"] = static_cast<double>(fed->grantedTime());
            val["send_time"] = static_cast<double>(fed->nextAllowedSendTime());
        });

        return generateJsonString(base);
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
        return generateJsonString(base);
    }
    if (queryStr == "federate_map") {
        Json::Value base;
        loadBasicJsonInfo(base, [](Json::Value& /*val*/, const FedInfo& /*fed*/) {});
        return generateJsonString(base);
    }
    return "#invalid";
}

std::string CommonCore::query(const std::string& target,
                              const std::string& queryStr,
                              helics_sequencing_mode mode)
{
    if (getBrokerState() >= broker_state_t::terminating) {
        if (target == "core" || target == getIdentifier() || target.empty()) {
            auto res = quickCoreQueries(queryStr);
            if (!res.empty()) {
                return res;
            }
        }
        return "#disconnected";
    }
    ActionMessage querycmd(mode == helics_sequencing_mode_fast ? CMD_QUERY : CMD_QUERY_ORDERED);
    querycmd.source_id = direct_core_id;
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
            return getAddress();
        }
        querycmd.setAction(mode == helics_sequencing_mode_fast ? CMD_BROKER_QUERY :
                                                                 CMD_BROKER_QUERY_ORDERED);
        querycmd.dest_id = direct_core_id;
    }
    if (querycmd.dest_id != direct_core_id) {
        // default into a federate query
        auto* fed =
            (target != "federate") ? getFederate(target) : getFederateAt(local_federate_id(0));
        if (fed != nullptr) {
            querycmd.dest_id = fed->global_id;
            if (mode != helics_sequencing_mode_ordered) {
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
                                                mode == helics_sequencing_mode_ordered);
                            if (ret != "#wait") {
                                activeQueries.finishedWithValue(index);
                                return ret;
                            }
                        } break;
                        default:
                            status = std::future_status::ready;  // LCOV_EXCL_LINE
                    }
                }
                return "#error";  // LCOV_EXCL_LINE
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
    querycmd.dest_id = root_broker_id;
    querycmd.source_id = direct_core_id;
    querycmd.payload = valueName;
    querycmd.setStringData(value);
    addActionMessage(std::move(querycmd));
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
            loopFederates.insert(command.name, no_search, getFederate(command.name));
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
            if (command.name == identifier) {
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
            if (command.payload == identifier) {
                if (checkActionFlag(command, error_flag)) {
                    auto estring =
                        std::string("broker responded with error: ") + errorMessageString(command);
                    setErrorState(command.messageID, estring);
                    errorRespondDelayedMessages(estring);
                    LOG_ERROR(parent_broker_id, identifier, estring);
                    break;
                }
                global_id = global_broker_id(command.dest_id);
                global_broker_id_local = global_broker_id(command.dest_id);
                filterFedID = global_federate_id(
                    global_broker_id_shift -
                    2 * (global_broker_id_local.baseValue() - global_broker_id_shift + 1));
                timeCoord->source_id = global_broker_id_local;
                higher_broker_id = global_broker_id(command.source_id);
                transmitDelayedMessages();
                timeoutMon->setParentId(higher_broker_id);
                if (checkActionFlag(command, slow_responding_flag)) {
                    timeoutMon->disableParentPing();
                }
                timeoutMon->reset();
                if (delayInitCounter < 0 && minFederateCount == 0) {
                    if (allInitReady()) {
                        if (transitionBrokerState(broker_state_t::connected,
                                                  broker_state_t::initializing)) {
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
            auto* fed = getFederateCore(command.name);
            if (fed != nullptr) {
                if (checkActionFlag(command, error_flag)) {
                    LOG_ERROR(
                        parent_broker_id,
                        identifier,
                        fmt::format("broker responded with error for registration of {}::{}\n",
                                    command.name,
                                    commandErrorString(command.messageID)));
                } else {
                    fed->global_id = command.dest_id;
                    loopFederates.addSearchTerm(command.dest_id, command.name);
                }

                // push the command to the local queue
                fed->addAction(std::move(command));
            }
        } break;
        case CMD_REG_ROUTE:
            // TODO(PT): double check this
            addRoute(route_id(command.getExtraData()), 0, command.payload);
            break;
        case CMD_PRIORITY_DISCONNECT:
            checkAndProcessDisconnect();
            checkAndProcessDisconnect();
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
        if (msg->source_id == parent_broker_id || msg->source_id == direct_core_id) {
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

void CommonCore::sendErrorToFederates(int errorCode, const std::string& message)
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

void CommonCore::transmitDelayedMessages(global_federate_id source)
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
            if (isReasonForTick(command.messageID, TickForwardingReasons::ping_response) ||
                isReasonForTick(command.messageID, TickForwardingReasons::no_comms)) {
                if (getBrokerState() == broker_state_t::operating) {
                    timeoutMon->tick(this);
                    LOG_SUMMARY(global_broker_id_local, getIdentifier(), " core tick");
                }
            }
            if (isReasonForTick(command.messageID, TickForwardingReasons::query_timeout)) {
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
                    m.source_id = global_federate_id{};
                    m.name = getIdentifier();
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
            if (isConnected()) {
                if (getBrokerState() <
                    broker_state_t::terminating) {  // only send a disconnect message
                                                    // if we haven't done so already
                    setBrokerState(broker_state_t::terminating);
                    sendDisconnect();
                    ActionMessage m(CMD_DISCONNECT);
                    m.source_id = global_broker_id_local;
                    transmit(parent_route_id, m);
                }
            } else if (getBrokerState() ==
                       broker_state_t::errored) {  // we are disconnecting in an error state
                sendDisconnect();
                ActionMessage m(CMD_DISCONNECT);
                m.source_id = global_broker_id_local;
                transmit(parent_route_id, m);
            }
            addActionMessage(CMD_STOP);
            // we can't just fall through since this may have generated other messages that need to
            // be forwarded or processed
            break;
        case CMD_BROADCAST_DISCONNECT: {
            timeCoord->processTimeMessage(command);
            loopFederates.apply([&command](auto& fed) { fed->addAction(command); });
            checkAndProcessDisconnect();
        } break;
        case CMD_STOP:

            if (isConnected()) {
                if (getBrokerState() <
                    broker_state_t::terminating) {  // only send a disconnect message
                                                    // if we haven't done so already
                    setBrokerState(broker_state_t::terminating);
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

        case CMD_EXEC_GRANT:
        case CMD_EXEC_REQUEST:
            if (isLocal(global_broker_id(command.source_id))) {
                if (hasTimeBlock(command.source_id)) {
                    delayedTimingMessages[command.source_id.baseValue()].push_back(command);
                    break;
                }
            }
            if (command.dest_id == global_broker_id_local) {
                timeCoord->processTimeMessage(command);
                if (!enteredExecutionMode) {
                    auto res = timeCoord->checkExecEntry();
                    if (res == message_processing_result::next_step) {
                        enteredExecutionMode = true;
                    }
                }
            } else if (command.source_id == global_broker_id_local) {
                for (auto dep : timeCoord->getDependents()) {
                    routeMessage(command, dep);
                }
            } else {
                routeMessage(command);
            }
            break;
        case CMD_TIME_GRANT:
        case CMD_TIME_REQUEST:
            if (isLocal(command.source_id)) {
                if (hasTimeBlock(command.source_id)) {
                    delayedTimingMessages[command.source_id.baseValue()].push_back(command);
                    break;
                }
            }
            routeMessage(command);
            break;
        case CMD_DISCONNECT:
        case CMD_DISCONNECT_FED:
            if (command.dest_id == parent_broker_id) {
                if (getBrokerState() < broker_state_t::terminating) {
                    auto fed = loopFederates.find(command.source_id);
                    if (fed == loopFederates.end()) {
                        return;
                    }
                    fed->state = operation_state::disconnected;
                    auto cstate = getBrokerState();
                    if ((!checkAndProcessDisconnect()) || (cstate < broker_state_t::operating)) {
                        command.setAction(CMD_DISCONNECT_FED);
                        transmit(parent_route_id, command);
                        if (minFederateState() != operation_state::disconnected ||
                            filterFed != nullptr) {
                            command.setAction(CMD_DISCONNECT_FED_ACK);
                            command.dest_id = command.source_id;
                            command.source_id = parent_broker_id;
                            routeMessage(command);
                        }
                    }
                }
            } else {
                routeMessage(command);
            }

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
        case CMD_DISCONNECT_CHECK:
            checkAndProcessDisconnect();
            break;
        case CMD_DISCONNECT_CORE_ACK:
            if ((command.dest_id == global_broker_id_local) &&
                (command.source_id == higher_broker_id)) {
                ActionMessage bye(CMD_DISCONNECT_FED_ACK);
                bye.source_id = parent_broker_id;
                for (auto fed : loopFederates) {
                    if (fed->getState() != federate_state::HELICS_FINISHED) {
                        bye.dest_id = fed->global_id.load();
                        fed->addAction(bye);
                    }
                }
                addActionMessage(CMD_STOP);
            }
            break;
        case CMD_SEARCH_DEPENDENCY: {
            auto* fed = getFederateCore(command.name);
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
            if (command.dest_id == global_broker_id_local) {
                sendToLogger(parent_broker_id,
                             command.messageID,
                             getFederateNameNoThrow(command.source_id),
                             command.payload);
            } else {
                routeMessage(command);
            }
            break;
        case CMD_WARNING:
            if (command.dest_id == global_broker_id_local) {
                sendToLogger(command.source_id,
                             log_level::warning,
                             getFederateNameNoThrow(command.source_id),
                             command.payload);
            } else {
                routeMessage(command);
            }
            break;
        case CMD_ERROR:
        case CMD_LOCAL_ERROR:
            if (command.dest_id == global_broker_id_local) {
                if (command.source_id == higher_broker_id ||
                    command.source_id == parent_broker_id || command.source_id == root_broker_id) {
                    sendErrorToFederates(command.messageID, command.payload);
                    setErrorState(command.messageID, command.payload);

                } else {
                    sendToLogger(parent_broker_id,
                                 log_level::error,
                                 getFederateNameNoThrow(command.source_id),
                                 command.payload);
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
                    if (getBrokerState() != broker_state_t::errored) {
                        sendErrorToFederates(command.messageID, command.payload);
                        setBrokerState(broker_state_t::errored);
                    }
                    command.setAction(CMD_GLOBAL_ERROR);
                    command.source_id = global_broker_id_local;
                    command.dest_id = root_broker_id;
                    transmit(parent_route_id, std::move(command));
                }
            } else {
                if (command.dest_id == parent_broker_id) {
                    if (terminate_on_error) {
                        if (getBrokerState() != broker_state_t::errored) {
                            sendErrorToFederates(command.messageID, command.payload);
                            setBrokerState(broker_state_t::errored);
                        }
                        command.setAction(CMD_GLOBAL_ERROR);
                        command.source_id = global_broker_id_local;
                        command.dest_id = root_broker_id;
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
            setErrorState(command.messageID, command.payload);
            sendErrorToFederates(command.messageID, command.payload);
            if (!(command.source_id == higher_broker_id || command.source_id == root_broker_id)) {
                transmit(parent_route_id, std::move(command));
            }
            break;
        case CMD_DATA_LINK: {
            auto* pub = loopHandles.getPublication(command.name);
            if (pub != nullptr) {
                command.name = command.getString(targetStringLoc);
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
        case CMD_FILTER_LINK: {
            auto* filt = loopHandles.getFilter(command.name);
            if (filt != nullptr) {
                command.name = command.getString(targetStringLoc);
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
        case CMD_CORE_CONFIGURE:
            processCoreConfigureCommands(command);
            break;
        case CMD_INIT: {
            auto* fed = getFederateCore(command.source_id);
            if (fed != nullptr) {
                fed->init_transmitted = true;
                if (allInitReady()) {
                    if (transitionBrokerState(broker_state_t::connected,
                                              broker_state_t::initializing)) {  // make sure we only
                                                                                // do this once
                        checkDependencies();
                        command.source_id = global_broker_id_local;
                        transmit(parent_route_id, command);
                    }
                }
            }
        } break;
        case CMD_INIT_GRANT:
            if (transitionBrokerState(
                    broker_state_t::initializing,
                    broker_state_t::operating)) {  // forward the grant to all federates
                if (filterFed != nullptr) {
                    filterFed->organizeFilterOperations();
                }

                loopFederates.apply([&command](auto& fed) { fed->addAction(command); });
                if (filterFed != nullptr && filterTiming) {
                    filterFed->handleMessage(command);
                }
                timeCoord->enteringExecMode();
                auto res = timeCoord->checkExecEntry();
                if (res == message_processing_result::next_step) {
                    enteredExecutionMode = true;
                }
                if (!timeCoord->hasActiveTimeDependencies()) {
                    timeCoord->disconnect();
                }
            }
            break;

        case CMD_SEND_MESSAGE:
            if ((command.dest_id == parent_broker_id) && (isLocal(command.source_id))) {
                deliverMessage(processMessage(command));
            } else {
                deliverMessage(command);
            }

            break;

        default:
            if (isPriorityCommand(command)) {  // this is a backup if somehow one of these
                                               // message got here
                processPriorityCommand(std::move(command));
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
                                        command.name,
                                        command.getString(typeStringLoc),
                                        command.getString(typeOutStringLoc),
                                        checkActionFlag(command, clone_flag));
                connectFilterTiming();
                break;
            default:
                return;
        }
        if (!command.name.empty()) {
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
    newFed.name = getIdentifier() + "_filters";
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
            auto* pub = loopHandles.getPublication(command.name);
            if (pub != nullptr) {
                if (checkActionFlag(*pub, disconnected_flag)) {
                    // TODO(PT): this might generate an error if the required flag was set
                    return;
                }
                command.setAction(CMD_ADD_SUBSCRIBER);
                command.setDestination(pub->handle);
                auto name = std::move(command.name);
                command.name.clear();

                addTargetToInterface(command);
                command.setAction(CMD_ADD_PUBLISHER);
                command.name = std::move(name);
                command.swapSourceDest();
                command.setStringData(pub->type, pub->units);
                addTargetToInterface(command);
            } else {
                routeMessage(std::move(command));
            }
        } break;
        case CMD_ADD_NAMED_INPUT: {
            const auto inputName = command.name;  // need to copy the name
            auto* inp = loopHandles.getInput(inputName);
            if (inp != nullptr) {
                if (checkActionFlag(*inp, disconnected_flag)) {
                    // TODO(PT): this might generate an error if the required flag was set
                    return;
                }
                command.setAction(CMD_ADD_PUBLISHER);
                command.setDestination(inp->handle);
                command.name.clear();
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
                command.name = inputName;
                addTargetToInterface(command);
            } else {
                routeMessage(std::move(command));
            }
        } break;
        case CMD_ADD_NAMED_FILTER: {
            auto* filt = loopHandles.getFilter(command.name);
            if (filt != nullptr) {
                if (checkActionFlag(*filt, disconnected_flag)) {
                    // TODO(PT): this might generate an error if the required flag was set
                    return;
                }
                command.setAction(CMD_ADD_ENDPOINT);
                command.setDestination(filt->handle);
                command.name.clear();
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
            auto* ept = loopHandles.getEndpoint(command.name);
            if (ept != nullptr) {
                if (checkActionFlag(*ept, disconnected_flag)) {
                    // TODO(PT): this might generate an error if the required flag was set
                    return;
                }
                command.setAction(CMD_ADD_FILTER);
                command.setDestination(ept->handle);
                command.name.clear();
                addTargetToInterface(command);
                command.setAction(CMD_ADD_ENDPOINT);
                command.swapSourceDest();
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
            auto* pub = loopHandles.getPublication(command.name);
            if (pub != nullptr) {
                command.setAction(CMD_REMOVE_SUBSCRIBER);
                command.setDestination(pub->handle);
                command.name.clear();
                removeTargetFromInterface(command);
                command.setAction(CMD_REMOVE_PUBLICATION);
                command.swapSourceDest();
                removeTargetFromInterface(command);
            } else {
                routeMessage(std::move(command));
            }
        } break;
        case CMD_REMOVE_NAMED_INPUT: {
            auto* inp = loopHandles.getInput(command.name);
            if (inp != nullptr) {
                command.setAction(CMD_REMOVE_PUBLICATION);
                command.setDestination(inp->handle);
                command.name.clear();
                removeTargetFromInterface(command);
                command.setAction(CMD_REMOVE_SUBSCRIBER);
                command.swapSourceDest();
                removeTargetFromInterface(command);
            } else {
                routeMessage(std::move(command));
            }
        } break;
        case CMD_REMOVE_NAMED_FILTER: {
            auto* filt = loopHandles.getFilter(command.name);
            if (filt != nullptr) {
                command.setAction(CMD_REMOVE_ENDPOINT);
                command.setDestination(filt->handle);
                command.name.clear();
                removeTargetFromInterface(command);
                command.setAction(CMD_REMOVE_FILTER);
                command.swapSourceDest();
                removeTargetFromInterface(command);
            } else {
                routeMessage(std::move(command));
            }
        } break;
        case CMD_REMOVE_NAMED_ENDPOINT: {
            auto* pub = loopHandles.getEndpoint(command.name);
            if (pub != nullptr) {
                command.setAction(CMD_REMOVE_FILTER);
                command.setDestination(pub->handle);
                command.name.clear();
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
        if (handleInfo->handleType != handle_type::filter) {
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
                    activeQueries.setDelayedValue(qt.first, "#timeout");
                    qt.first = 0;
                }
            }
        }
        while (!queryTimeouts.empty() && queryTimeouts.front().first == 0) {
            queryTimeouts.pop_front();
        }
        if (queryTimeouts.empty()) {
            setTickForwarding(TickForwardingReasons::query_timeout, false);
        }
    }
}

void CommonCore::processQueryResponse(const ActionMessage& m)
{
    if (m.counter == general_query) {
        activeQueries.setDelayedValue(m.messageID, m.payload);
        return;
    }
    if (isValidIndex(m.counter, mapBuilders)) {
        auto& builder = std::get<0>(mapBuilders[m.counter]);
        auto& requestors = std::get<1>(mapBuilders[m.counter]);
        if (builder.addComponent(m.payload, m.messageID)) {
            auto str = builder.generate();
            if (m.counter == global_flush) {
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
                requestors.back().dest_id == direct_core_id) {
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
            if (fed->getOptionFlag(defs::flags::observer)) {
                timeCoord->removeDependency(fed->global_id);
                ActionMessage rmdep(CMD_REMOVE_DEPENDENT);

                rmdep.source_id = global_broker_id_local;
                rmdep.dest_id = fed->global_id.load();
                fed->addAction(rmdep);
                isobs = true;
            } else if (fed->getOptionFlag(defs::flags::source_only)) {
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
    global_federate_id fedid;
    global_broker_id brkid;
    int localcnt = 0;
    for (const auto& dep : timeCoord->getDependents()) {
        if (isLocal(dep)) {
            ++localcnt;
            fedid = dep;
        } else {
            brkid = static_cast<global_broker_id>(dep);
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

void CommonCore::processCoreConfigureCommands(ActionMessage& cmd)
{
    switch (cmd.messageID) {
        case defs::flags::enable_init_entry:
            --delayInitCounter;
            if (delayInitCounter <= 0) {
                if (allInitReady()) {
                    if (transitionBrokerState(
                            broker_state_t::connected,
                            broker_state_t::initializing)) {  // make sure we only do this once
                        checkDependencies();
                        cmd.setAction(CMD_INIT);
                        cmd.source_id = global_broker_id_local;
                        cmd.dest_id = parent_broker_id;
                        transmit(parent_route_id, cmd);
                    }
                }
            }
            break;
        case defs::properties::log_level:
            setLogLevel(cmd.getExtraData());
            break;
        case defs::properties::file_log_level:
            setLogLevels(consoleLogLevel, cmd.getExtraData());
            break;
        case defs::properties::console_log_level:
            setLogLevels(cmd.getExtraData(), fileLogLevel);
            break;
        case defs::flags::terminate_on_error:
            terminate_on_error = checkActionFlag(cmd, indicator_flag);
            break;
        case defs::flags::slow_responding:
            no_ping = checkActionFlag(cmd, indicator_flag);
            break;
        case defs::flags::debugging:
            debugging = no_ping = checkActionFlag(cmd, indicator_flag);
            break;
        case UPDATE_LOGGING_CALLBACK:
            if (checkActionFlag(cmd, empty_flag)) {
                setLoggerFunction(nullptr);
            } else {
                auto op = dataAirlocks[cmd.counter].try_unload();
                if (op) {
                    auto M = stx::any_cast<
                        std::function<void(int, const std::string&, const std::string&)>>(
                        std::move(*op));
                    M(0, identifier, "logging callback activated");
                    setLoggerFunction(std::move(M));
                }
            }
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
            // FALLTHROUGH
        case CMD_BROKER_QUERY:

            if (cmd.dest_id == global_broker_id_local || cmd.dest_id == direct_core_id) {
                std::string repStr = coreQuery(cmd.payload, force_ordered);
                if (repStr != "#wait") {
                    if (cmd.source_id == direct_core_id) {
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
                    if (cmd.source_id == direct_core_id) {
                        if (queryTimeouts.empty()) {
                            setTickForwarding(TickForwardingReasons::query_timeout, true);
                        }
                        queryTimeouts.emplace_back(cmd.messageID, std::chrono::steady_clock::now());
                    }
                    ActionMessage queryResp(force_ordered ? CMD_QUERY_REPLY_ORDERED :
                                                            CMD_QUERY_REPLY);
                    queryResp.dest_id = cmd.source_id;
                    queryResp.source_id = global_broker_id_local;
                    queryResp.messageID = cmd.messageID;
                    queryResp.counter = cmd.counter;
                    std::get<1>(mapBuilders[mapIndex.at(cmd.payload).first]).push_back(queryResp);
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
                if (cmd.source_id == direct_core_id) {
                    if (queryTimeouts.empty()) {
                        setTickForwarding(TickForwardingReasons::query_timeout, true);
                    }
                    queryTimeouts.emplace_back(cmd.messageID, std::chrono::steady_clock::now());
                }
                const auto& target = cmd.getString(targetStringLoc);
                if (target == "root" || target == "federation") {
                    cmd.setAction(force_ordered ? CMD_BROKER_QUERY_ORDERED : CMD_BROKER_QUERY);
                    cmd.dest_id = root_broker_id;
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
                    cmd.source_id = direct_core_id;
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
                    repStr = coreQuery(cmd.payload, force_ordered);
                } else {
                    auto* fedptr = getFederateCore(target);
                    repStr = federateQuery(fedptr, cmd.payload, force_ordered);
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
                if (queryResp.dest_id == direct_core_id) {
                    processQueryResponse(queryResp);
                } else {
                    transmit(getRoute(queryResp.dest_id), queryResp);
                }
            }
            break;
        case CMD_QUERY_REPLY:
        case CMD_QUERY_REPLY_ORDERED:
            if (cmd.dest_id == global_broker_id_local || cmd.dest_id == direct_core_id) {
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
            if (res == message_processing_result::next_step) {
                enteredExecutionMode = true;
            }
        } else {
            if (timeCoord->processTimeMessage(cmd)) {
                timeCoord->updateTimeFactors();
            }
        }
        if (isDisconnectCommand(cmd)) {
            if ((cmd.action() == CMD_DISCONNECT) && (cmd.source_id == higher_broker_id)) {
                setBrokerState(broker_state_t::terminating);
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
    } else {
        LOG_WARNING(global_broker_id_local, "core", "dropping message:" + prettyPrintString(cmd));
    }
}

bool CommonCore::hasTimeBlock(global_federate_id fedID)
{
    for (auto& tb : timeBlocks) {
        if (fedID == tb.first) {
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
        if (getBrokerState() <= broker_state_t::configured) {
            connect();
        }
        if (getBrokerState() >= broker_state_t::terminating) {
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
    if ((getBrokerState() == broker_state_t::terminating) ||
        (getBrokerState() == broker_state_t::terminated)) {
        return true;
    }
    if (allDisconnected()) {
        checkInFlightQueriesForDisconnect();
        setBrokerState(broker_state_t::terminating);
        timeCoord->disconnect();
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
    LOG_CONNECTIONS(global_broker_id_local, "core", "sending disconnect");
    checkInFlightQueriesForDisconnect();
    ActionMessage bye(CMD_STOP);
    bye.source_id = global_broker_id_local;
    for (auto fed : loopFederates) {
        if (fed->getState() != federate_state::HELICS_FINISHED) {
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
    auto* pub = loopHandles.getPublication(cmd.name);
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

void CommonCore::routeMessage(ActionMessage& cmd, global_federate_id dest)
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
            if (fed->getState() != federate_state::HELICS_FINISHED) {
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
        auto ncmd = cmd;
        filterFed->handleMessage(ncmd);
    } else if (isLocal(cmd.dest_id)) {
        auto* fed = getFederateCore(cmd.dest_id);
        if (fed != nullptr) {
            if ((fed->getState() != federate_state::HELICS_FINISHED) &&
                (fed->getState() != federate_state::HELICS_ERROR)) {
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

void CommonCore::routeMessage(ActionMessage&& cmd, global_federate_id dest)
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
            if (fed->getState() != federate_state::HELICS_FINISHED) {
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
    global_federate_id dest = cmd.dest_id;
    if ((dest == parent_broker_id) || (dest == higher_broker_id)) {
        transmit(parent_route_id, cmd);
    } else if (dest == global_broker_id_local) {
        processCommandsForCore(cmd);
    } else if (dest == filterFedID) {
        filterFed->handleMessage(cmd);
    } else if (isLocal(dest)) {
        auto* fed = getFederateCore(dest);
        if (fed != nullptr) {
            if (fed->getState() != federate_state::HELICS_FINISHED) {
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
    if (checkActionFlag(*handle, has_source_filter_flag)) {
        if (filterFed != nullptr) {
            return filterFed->processMessage(m, handle);
        }
    }

    return m;
}

const std::string& CommonCore::getInterfaceInfo(interface_handle handle) const
{
    const auto* handleInfo = getHandleInfo(handle);
    if (handleInfo != nullptr) {
        return handleInfo->interface_info;
    }
    return emptyStr;
}

void CommonCore::setInterfaceInfo(helics::interface_handle handle, std::string info)
{
    handles.modify(
        [&](auto& hdls) { hdls.getHandleInfo(handle.baseValue())->interface_info = info; });
}
}  // namespace helics
