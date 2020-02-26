/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "CommonCore.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "../common/fmt_format.h"
#include "../common/logger.h"
#include "ActionMessage.hpp"
#include "BasicHandleInfo.hpp"
#include "CoreFactory.hpp"
#include "CoreFederateInfo.hpp"
#include "EndpointInfo.hpp"
#include "FederateState.hpp"
#include "FilterCoordinator.hpp"
#include "FilterInfo.hpp"
#include "ForwardingTimeCoordinator.hpp"
#include "NamedInputInfo.hpp"
#include "PublicationInfo.hpp"
#include "TimeoutMonitor.h"
#include "core-exceptions.hpp"
#include "coreTypeOperations.hpp"
#include "fileConnections.hpp"
#include "gmlc/concurrency/DelayedObjects.hpp"
#include "helics_definitions.hpp"
#include "loggingHelper.hpp"
#include "queryHelpers.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <functional>

namespace helics {
// timeoutMon is a unique_ptr
CommonCore::CommonCore() noexcept: timeoutMon(new TimeoutMonitor) {}

CommonCore::CommonCore(bool /*arg*/) noexcept: timeoutMon(new TimeoutMonitor) {}

CommonCore::CommonCore(const std::string& core_name):
    BrokerBase(core_name), timeoutMon(new TimeoutMonitor)
{
}

void CommonCore::configure(const std::string& configureString)
{
    broker_state_t exp = broker_state_t::created;
    if (brokerState.compare_exchange_strong(exp, broker_state_t::configuring)) {
        // initialize the brokerbase
        auto result = parseArgs(configureString);
        if (result != 0) {
            brokerState = broker_state_t::created;
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
    broker_state_t exp = broker_state_t::created;
    if (brokerState.compare_exchange_strong(exp, broker_state_t::configuring)) {
        // initialize the brokerbase
        auto result = parseArgs(argc, argv);
        if (result != 0) {
            brokerState = broker_state_t::created;
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
    broker_state_t exp = broker_state_t::created;
    if (brokerState.compare_exchange_strong(exp, broker_state_t::configuring)) {
        // initialize the brokerbase
        auto result = parseArgs(std::move(args));
        if (result != 0) {
            brokerState = broker_state_t::created;
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
    if (brokerState >= broker_state_t::configured) {
        broker_state_t exp = broker_state_t::configured;
        if (brokerState.compare_exchange_strong(exp, broker_state_t::connecting)) {
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
                brokerState = broker_state_t::connected;
                disconnection.activate();
            } else {
                brokerState = broker_state_t::configured;
            }
            return res;
        }

        LOG_WARNING(global_id.load(), getIdentifier(), "multiple connect calls");
        while (brokerState == broker_state_t::connecting) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    return isConnected();
}

bool CommonCore::isConnected() const
{
    auto currentState = brokerState.load(std::memory_order_acquire);
    return (
        (currentState == broker_state_t::operating) || (currentState == broker_state_t::connected));
}

const std::string& CommonCore::getAddress() const
{
    if ((brokerState != broker_state_t::connected) || (address.empty())) {
        address = generateLocalAddressString();
    }
    return address;
}

void CommonCore::processDisconnect(bool skipUnregister)
{
    if (brokerState > broker_state_t::configured) {
        if (brokerState < broker_state_t::terminating) {
            brokerState = broker_state_t::terminating;
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
    brokerState = broker_state_t::terminated;
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
        LOG_WARNING(
            global_id.load(),
            getIdentifier(),
            "waiting on disconnect: current state=" + brokerStateName(brokerState.load()));
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
    otherwise this would be a mess and probably cause segmentation faults so we capture it in a local variable
    that will be destroyed on function exit
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
    return (brokerState >= broker_state_t::configured);
}

bool CommonCore::isOpenToNewFederates() const
{
    return ((brokerState != broker_state_t::created) && (brokerState < broker_state_t::operating));
}
void CommonCore::globalError(local_federate_id federateID, int error_code, const std::string &error_string)
{
    auto fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid error"));
    }
    ActionMessage m(CMD_ERROR);
    m.source_id = fed->global_id.load();
    m.messageID = error_code;
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

void CommonCore::localError(local_federate_id federateID, int error_code, const std::string &error_string)
{
    auto fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid error"));
    }
    ActionMessage m(CMD_ERROR);
    m.source_id = fed->global_id.load();
    m.messageID = error_code;
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

void CommonCore::finalize(local_federate_id federateID)
{
    auto fed = getFederateAt(federateID);
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
    auto afed = allFedDisconnected();
    if ((hasTimeDependency) || (hasFilters)) {
        return (afed) && (!timeCoord->hasActiveTimeDependencies());
    }
    return (afed);
}

bool CommonCore::allFedDisconnected() const
{
    // all federates must have hit finished state
    auto pred = [](const auto& fed) { return fed.disconnected; };
    return std::all_of(loopFederates.begin(), loopFederates.end(), pred);
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
    auto fed = getFederateAt(federateID);
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
            exp, true)) { // only enter this loop once per federate
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

iteration_result
    CommonCore::enterExecutingMode(local_federate_id federateID, iteration_request iterate)
{
    auto fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (EnterExecutingState)"));
    }
    if (HELICS_EXECUTING == fed->getState()) {
        return iteration_result::next_step;
    }
    if (HELICS_INITIALIZING != fed->getState()) {
        throw(InvalidFunctionCall("federate is in invalid state for calling entry to exec mode"));
    }
    // do an exec check on the fed to process previously received messages so it can't get in a deadlocked state
    ActionMessage exec(CMD_EXEC_CHECK);
    fed->addAction(exec);
    // TODO:: check for error conditions?
    return fed->enterExecutingMode(iterate);
}

local_federate_id
    CommonCore::registerFederate(const std::string& name, const CoreFederateInfo& info)
{
    if (!waitCoreRegistration()) {
        if (brokerState == broker_state_t::errored) {
            if (!lastErrorString.empty()) {
                throw(RegistrationFailure(lastErrorString));
            }
        }
        throw(RegistrationFailure(
            "core is unable to register and has timed out, federate cannot be registered"));
    }
    if (brokerState >= broker_state_t::operating) {
        throw(RegistrationFailure("Core has already moved to operating state"));
    }
    FederateState* fed = nullptr;
    local_federate_id local_id;
    {
        auto feds = federates.lock();
        auto id = feds->insert(name, name, info);
        if (id) {
            local_id = local_federate_id(static_cast<int32_t>(*id));
            fed = (*feds)[*id];
        } else {
            throw(RegistrationFailure(
                "duplicate names " + name + "detected multiple federates with the same name"));
        }
    }
    if (fed == nullptr) {
        throw(RegistrationFailure("unknown allocation error occurred"));
    }
    // setting up the Logger
    // auto ptr = fed.get();
    // if we are using the Logger, log all messages coming from the federates so they can control the level*/
    fed->setLogger([this](int /*level*/, const std::string& ident, const std::string& message) {
        sendToLogger(parent_broker_id, log_level::error - 2, ident, message);
    });

    fed->local_id = local_id;
    fed->setParent(this);

    ActionMessage m(CMD_REG_FED);
    m.name = name;
    addActionMessage(m);
    // now wait for the federateQueue to get the response
    auto valid = fed->waitSetup();
    if (valid == iteration_result::next_step) {
        return local_id;
    }
    throw(RegistrationFailure(std::string("fed received Failure ") + fed->lastErrorString()));
}

const std::string& CommonCore::getFederateName(local_federate_id federateID) const
{
    auto fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (federateName)"));
    }
    return fed->getIdentifier();
}

static const std::string unknownString("#unknown");

const std::string& CommonCore::getFederateNameNoThrow(global_federate_id federateID) const noexcept
{
    auto fed = getFederateAt(local_federate_id(federateID.localIndex()));
    return (fed == nullptr) ? unknownString : fed->getIdentifier();
}

local_federate_id CommonCore::getFederateId(const std::string& name) const
{
    auto feds = federates.lock();
    auto fed = feds->find(name);
    if (fed != nullptr) {
        return fed->local_id;
    }

    return local_federate_id();
}

int32_t CommonCore::getFederationSize()
{
    if (brokerState >= broker_state_t::operating) {
        return _global_federation_size;
    }
    // if we are in initialization return the local federation size
    return static_cast<int32_t>(federates.lock()->size());
}

Time CommonCore::timeRequest(local_federate_id federateID, Time next)
{
    auto fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid timeRequest"));
    }
    switch (fed->getState()) {
        case HELICS_EXECUTING: {
            auto ret = fed->requestTime(next, iteration_request::no_iterations);
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

iteration_time CommonCore::requestTimeIterative(
    local_federate_id federateID,
    Time next,
    iteration_request iterate)
{
    auto fed = getFederateAt(federateID);
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

    return fed->requestTime(next, iterate);
}

Time CommonCore::getCurrentTime(local_federate_id federateID) const
{
    auto fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw InvalidIdentifier("federateID not valid (getCurrentTime)");
    }
    return fed->grantedTime();
}

uint64_t CommonCore::getCurrentReiteration(local_federate_id federateID) const
{
    auto fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw InvalidIdentifier("federateID not valid (getCurrentReiteration)");
    }
    return fed->getCurrentIteration();
}

void CommonCore::setIntegerProperty(
    local_federate_id federateID,
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
    auto fed = getFederateAt(federateID);
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
    auto fed = getFederateAt(federateID);
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
    auto fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (setTimeDelta)"));
    }
    return fed->getTimeProperty(property);
}

int16_t CommonCore::getIntegerProperty(local_federate_id federateID, int32_t property) const
{
    if (federateID == local_core_id) {
        //TODO: PT add some code to actually get the properties from the core if appropriate
        return 0;
    }
    auto fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (setTimeDelta)"));
    }
    return fed->getIntegerProperty(property);
}

void CommonCore::setFlagOption(local_federate_id federateID, int32_t flag, bool flagValue)
{
    if (federateID == local_core_id) {
        if (flag == defs::flags::delay_init_entry) {
            if (flagValue) {
                ++delayInitCounter;
            } else {
                ActionMessage cmd(CMD_CORE_CONFIGURE);
                cmd.messageID = defs::flags::delay_init_entry;
                if (flagValue) {
                    setActionFlag(cmd, indicator_flag);
                }
                addActionMessage(cmd);
            }
        } else if (flag == defs::flags::enable_init_entry) {
            ActionMessage cmd(CMD_CORE_CONFIGURE);
            cmd.messageID = defs::flags::enable_init_entry;
            if (flagValue) {
                setActionFlag(cmd, indicator_flag);
            }
            addActionMessage(cmd);
        } else if (flag == defs::flags::slow_responding) {
            ActionMessage cmd(CMD_CORE_CONFIGURE);
            cmd.messageID = defs::flags::slow_responding;
            if (flagValue) {
                setActionFlag(cmd, indicator_flag);
            }
            addActionMessage(cmd);
        }
        return;
    }

    auto fed = getFederateAt(federateID);
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
    if (federateID == local_core_id) {
        return false;
    }
    auto fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (setTimeDelta)"));
    }
    return fed->getOptionFlag(flag);
}

const BasicHandleInfo& CommonCore::createBasicHandle(
    global_federate_id global_federateId,
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

interface_handle CommonCore::registerInput(
    local_federate_id federateID,
    const std::string& key,
    const std::string& type,
    const std::string& units)
{
    auto fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (registerNamedInput)"));
    }
    auto ci = handles.read([&key](auto& hand) { return hand.getInput(key); });
    if (ci != nullptr) // this key is already found
    {
        throw(RegistrationFailure("named Input already exists"));
    }
    auto& handle = createBasicHandle(
        fed->global_id,
        fed->local_id,
        handle_type::input,
        key,
        type,
        units,
        fed->getInterfaceFlags());

    auto id = handle.getInterfaceHandle();
    fed->createInterface(handle_type::input, id, key, type, units);

    LOG_INTERFACES(
        parent_broker_id, fed->getIdentifier(), fmt::format("registering Input {}", key));
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
    auto ci = handles.read([&key](auto& hand) { return hand.getInput(key); });
    if (ci->local_fed_id != federateID) {
        return {};
    }
    return ci->getInterfaceHandle();
}

interface_handle CommonCore::registerPublication(
    local_federate_id federateID,
    const std::string& key,
    const std::string& type,
    const std::string& units)
{
    auto fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (registerPublication)"));
    }
    LOG_INTERFACES(parent_broker_id, fed->getIdentifier(), fmt::format("registering PUB {}", key));
    auto pub = handles.read([&key](auto& hand) { return hand.getPublication(key); });
    if (pub != nullptr) // this key is already found
    {
        throw(RegistrationFailure("Publication key already exists"));
    }
    auto& handle = createBasicHandle(
        fed->global_id,
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

interface_handle
    CommonCore::getPublication(local_federate_id federateID, const std::string& key) const
{
    auto pub = handles.read([&key](auto& hand) { return hand.getPublication(key); });
    if (pub->local_fed_id != federateID) {
        return interface_handle();
    }
    return pub->getInterfaceHandle();
}

const std::string emptyStr;

const std::string& CommonCore::getHandleName(interface_handle handle) const
{
    auto handleInfo = getHandleInfo(handle);
    if (handleInfo != nullptr) {
        return handleInfo->key;
    }
    return emptyStr;
}

const std::string& CommonCore::getInjectionUnits(interface_handle handle) const
{
    auto handleInfo = getHandleInfo(handle);
    if (handleInfo != nullptr) {
        switch (handleInfo->handleType) {
            case handle_type::input: {
                auto fed = getFederateAt(handleInfo->local_fed_id);
                auto inpInfo = fed->interfaces().getInput(handle);
                if (inpInfo != nullptr) {
                    if (!inpInfo->inputUnits.empty()) {
                        return inpInfo->inputUnits;
                    }
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
}

const std::string& CommonCore::getExtractionUnits(interface_handle handle) const
{
    auto handleInfo = getHandleInfo(handle);
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
    auto handleInfo = getHandleInfo(handle);
    if (handleInfo != nullptr) {
        switch (handleInfo->handleType) {
            case handle_type::input: {
                auto fed = getFederateAt(handleInfo->local_fed_id);
                auto inpInfo = fed->interfaces().getInput(handle);
                if (inpInfo != nullptr) {
                    if (!inpInfo->inputType.empty()) {
                        return inpInfo->inputType;
                    }
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
    auto handleInfo = getHandleInfo(handle);
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

void CommonCore::setHandleOption(interface_handle handle, int32_t option, bool option_value)
{
    auto handleInfo = getHandleInfo(handle);
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

    if (option_value) {
        setActionFlag(fcn, indicator_flag);
    }
    if (handleInfo->handleType != handle_type::filter) {
        auto fed = getHandleFederate(handle);
        if (fed != nullptr) {
            fcn.dest_id = fed->global_id;
            fed->setProperties(fcn);
        }
    } else {
        // must be for filter
    }
}

bool CommonCore::getHandleOption(interface_handle handle, int32_t option) const
{
    auto handleInfo = getHandleInfo(handle);
    if (handleInfo == nullptr) {
        return false;
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
        auto fed = getFederateAt(handleInfo->local_fed_id);
        if (fed != nullptr) {
            return fed->getHandleOption(handle, static_cast<char>(handleInfo->handleType), option);
        }
    } else {
        // must be for filter
    }
    return false;
}

void CommonCore::closeHandle(interface_handle handle)
{
    auto handleInfo = getHandleInfo(handle);
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
    auto handleInfo = getHandleInfo(handle);
    if (handleInfo == nullptr) {
        throw(InvalidIdentifier("invalid handle"));
    }

    ActionMessage cmd;
    cmd.setSource(handleInfo->handle);
    cmd.name = targetToRemove;
    auto fed = getFederateAt(handleInfo->local_fed_id);
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
    auto handleInfo = getHandleInfo(handle);
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
    auto handleInfo = getHandleInfo(handle);
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
    auto handleInfo = getHandleInfo(handle);
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
        return; // if the value is not required do nothing
    }
    auto fed = getFederateAt(handleInfo->local_fed_id);
    if (fed->checkAndSetValue(handle, data, len)) {
        LOG_DATA_MESSAGES(
            parent_broker_id,
            fed->getIdentifier(),
            fmt::format("setting Value for {} size {}", handleInfo->key, len));

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
        } else {
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
                if (res < 0) // deal with max package size if there are a lot of subscribers
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
}

std::shared_ptr<const data_block> CommonCore::getValue(interface_handle handle)
{
    auto handleInfo = getHandleInfo(handle);
    if (handleInfo == nullptr) {
        throw(InvalidIdentifier("Handle is invalid (getValue)"));
    }

    if (handleInfo->handleType != handle_type::input) {
        throw(InvalidIdentifier("Handle does not identify an input"));
    }
    // TODO::PT this is a long chain should be refactored
    return getFederateAt(handleInfo->local_fed_id)->interfaces().getInput(handle)->getData();
}

std::vector<std::shared_ptr<const data_block>> CommonCore::getAllValues(interface_handle handle)
{
    auto handleInfo = getHandleInfo(handle);
    if (handleInfo == nullptr) {
        throw(InvalidIdentifier("Handle is invalid (getValue)"));
    }

    if (handleInfo->handleType != handle_type::input) {
        throw(InvalidIdentifier("Handle does not identify an input"));
    }
    // todo:: this is a long chain should be refactored
    return getFederateAt(handleInfo->local_fed_id)->interfaces().getInput(handle)->getAllData();
}

const std::vector<interface_handle>& CommonCore::getValueUpdates(local_federate_id federateID)
{
    auto fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (getValueUpdates)"));
    }
    return fed->getEvents();
}

interface_handle CommonCore::registerEndpoint(
    local_federate_id federateID,
    const std::string& name,
    const std::string& type)
{
    auto fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (registerEndpoint)"));
    }
    auto ept = handles.read([&name](auto& hand) { return hand.getEndpoint(name); });
    if (ept != nullptr) {
        throw(RegistrationFailure("endpoint name is already used"));
    }
    auto& handle = createBasicHandle(
        fed->global_id,
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

interface_handle
    CommonCore::getEndpoint(local_federate_id federateID, const std::string& name) const
{
    auto ept = handles.read([&name](auto& hand) { return hand.getEndpoint(name); });
    if (ept->local_fed_id != federateID) {
        return interface_handle();
    }
    return ept->handle.handle;
}

interface_handle CommonCore::registerFilter(
    const std::string& filterName,
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
        if (brokerState.load() >= broker_state_t::terminating) {
            throw(RegistrationFailure("core is terminated no further registration possible"));
        }
        throw(RegistrationFailure("registration timeout exceeded"));
    }
    auto brkid = global_id.load();

    auto handle = createBasicHandle(
        brkid, local_federate_id(), handle_type::filter, filterName, type_in, type_out);
    auto id = handle.getInterfaceHandle();

    ActionMessage m(CMD_REG_FILTER);
    m.source_id = brkid;
    m.source_handle = id;
    m.name = handle.key;
    if ((!type_in.empty()) || (!type_out.empty())) {
        m.setStringData(type_in, type_out);
    }
    actionQueue.push(std::move(m));
    return id;
}

interface_handle CommonCore::registerCloningFilter(
    const std::string& filterName,
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
        if (brokerState.load() >= broker_state_t::terminating) {
            throw(RegistrationFailure("core is terminated no further registration possible"));
        }
        throw(RegistrationFailure("registration timeout exceeded"));
    }
    auto brkid = global_id.load();

    auto& handle = createBasicHandle(
        brkid,
        local_federate_id(),
        handle_type::filter,
        filterName,
        type_in,
        type_out,
        make_flags(clone_flag));

    auto id = handle.getInterfaceHandle();

    ActionMessage m(CMD_REG_FILTER);
    m.source_id = brkid;
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
    auto filt = handles.read([&name](auto& hand) { return hand.getFilter(name); });
    if ((filt != nullptr) && (filt->handleType == handle_type::filter)) {
        return filt->getInterfaceHandle();
    }
    return interface_handle{};
}

FilterInfo* CommonCore::createFilter(
    global_broker_id dest,
    interface_handle handle,
    const std::string& key,
    const std::string& type_in,
    const std::string& type_out,
    bool cloning)
{
    auto filt = std::make_unique<FilterInfo>(
        (dest == parent_broker_id) ? global_id.load() : dest,
        handle,
        key,
        type_in,
        type_out,
        false);

    auto retTarget = filt.get();
    auto actualKey = key;
    retTarget->cloning = cloning;
    if (actualKey.empty()) {
        actualKey = "sFilter_";
        actualKey.append(std::to_string(handle.baseValue()));
    }
    if (filt->core_id == global_id.load()) {
        filters.insert(actualKey, global_handle(dest, filt->handle), std::move(filt));
    } else {
        actualKey.push_back('_');
        actualKey.append(std::to_string(filt->core_id.baseValue()));
        filters.insert(actualKey, {filt->core_id, filt->handle}, std::move(filt));
    }

    return retTarget;
}

void CommonCore::registerFrequentCommunicationsPair(
    const std::string& /*source*/,
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

void CommonCore::addDestinationFilterToEndpoint(
    const std::string& filter,
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
    auto fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("federateID not valid (registerDependency)"));
    }
    ActionMessage search(CMD_SEARCH_DEPENDENCY);
    search.source_id = fed->global_id.load();
    search.name = federateName;
    addActionMessage(std::move(search));
}

void CommonCore::send(
    interface_handle sourceHandle,
    const std::string& destination,
    const char* data,
    uint64_t length)
{
    auto hndl = getHandleInfo(sourceHandle);
    if (hndl == nullptr) {
        throw(InvalidIdentifier("handle is not valid"));
    }

    if (hndl->handleType != handle_type::endpoint) {
        throw(InvalidIdentifier("handle does not point to an endpoint"));
    }
    auto fed = getFederateAt(hndl->local_fed_id);
    ActionMessage m(CMD_SEND_MESSAGE);

    m.messageID = ++messageCounter;
    m.source_handle = sourceHandle;
    m.source_id = hndl->getFederateId();

    m.payload = std::string(data, length);
    m.setStringData(destination, hndl->key, hndl->key);
    m.actionTime = fed->nextAllowedSendTime();
    addActionMessage(std::move(m));
}

void CommonCore::sendEvent(
    Time time,
    interface_handle sourceHandle,
    const std::string& destination,
    const char* data,
    uint64_t length)
{
    auto hndl = getHandleInfo(sourceHandle);
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
    auto hndl = getHandleInfo(sourceHandle);
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
    auto minTime = getFederateAt(hndl->local_fed_id)->nextAllowedSendTime();
    if (m.actionTime < minTime) {
        m.actionTime = minTime;
    }
    addActionMessage(std::move(m));
}

void CommonCore::deliverMessage(ActionMessage& message)
{
    switch (message.action()) {
        case CMD_SEND_MESSAGE: {
            // Find the destination endpoint
            auto localP = (message.dest_id == parent_broker_id) ?
                loopHandles.getEndpoint(message.getString(targetStringLoc)) :
                loopHandles.findHandle(message.getDest());
            if (localP == nullptr) {
                auto kfnd = knownExternalEndpoints.find(message.getString(targetStringLoc));
                if (kfnd != knownExternalEndpoints.end()) { // destination is known
                    transmit(kfnd->second, message);
                } else {
                    transmit(parent_route_id, message);
                }
                return;
            }
            // now we deal with local processing
            if (checkActionFlag(*localP, has_dest_filter_flag)) {
                auto ffunc = getFilterCoordinator(localP->getInterfaceHandle());
                if (ffunc != nullptr) {
                    if (ffunc->destFilter != nullptr) {
                        if (!checkActionFlag(*(ffunc->destFilter), disconnected_flag)) {
                            if (ffunc->destFilter->core_id !=
                                global_broker_id_local) { // now we have deal with non-local processing destination filter
                                // first block the federate time advancement until the return is received
                                auto fed_id = localP->getFederateId();
                                ActionMessage tblock(
                                    CMD_TIME_BLOCK, global_broker_id_local, fed_id);
                                auto mid = ++messageCounter;
                                tblock.messageID = mid;
                                auto fed = getFederateCore(fed_id);
                                fed->addAction(tblock);
                                // now send a message to get filtered
                                message.setAction(CMD_SEND_FOR_DEST_FILTER_AND_RETURN);
                                message.messageID = mid;
                                message.source_id = fed_id;
                                message.source_handle = localP->getInterfaceHandle();
                                message.dest_id = ffunc->destFilter->core_id;
                                message.dest_handle = ffunc->destFilter->handle;
                                ongoingDestFilterProcesses[fed_id.baseValue()].emplace(mid);
                                routeMessage(std::move(message));
                                return;
                            }
                            // the filter is part of this core
                            auto tempMessage = createMessageFromCommand(std::move(message));
                            if (ffunc->destFilter->filterOp) {
                                auto nmessage =
                                    ffunc->destFilter->filterOp->process(std::move(tempMessage));
                                message.moveInfo(std::move(nmessage));
                            } else {
                                message.moveInfo(std::move(tempMessage));
                            }
                        }
                    }
                    // now go to the cloning filters
                    for (auto clFilter : ffunc->cloningDestFilters) {
                        if (checkActionFlag(*clFilter, disconnected_flag)) {
                            continue;
                        }
                        if (clFilter->core_id == global_broker_id_local) {
                            auto FiltI = filters.find(
                                global_handle(global_broker_id_local, clFilter->handle));
                            if (FiltI != nullptr) {
                                if (FiltI->filterOp != nullptr) {
                                    // this is a cloning filter so it generates a bunch(?) of new messages
                                    auto new_messages = FiltI->filterOp->processVector(
                                        createMessageFromCommand(message));
                                    for (auto& msg : new_messages) {
                                        if (msg) {
                                            if (msg->dest ==
                                                localP
                                                    ->key) //in case the clone filter send to itself.
                                            {
                                                ActionMessage cmd(std::move(msg));
                                                cmd.dest_id = localP->handle.fed_id;
                                                cmd.dest_handle = localP->handle.handle;
                                                routeMessage(std::move(cmd));
                                            } else {
                                                ActionMessage cmd(std::move(msg));
                                                deliverMessage(cmd);
                                            }
                                        }
                                    }
                                }
                            }
                        } else {
                            ActionMessage clone(message);
                            clone.setAction(CMD_SEND_FOR_FILTER);
                            clone.dest_id = clFilter->core_id;
                            clone.dest_handle = clFilter->handle;
                            routeMessage(clone);
                        }
                    }
                }
            }
            if (message.dest_id == parent_broker_id) {
                message.dest_id = localP->getFederateId();
                message.dest_handle = localP->getInterfaceHandle();
            }

            timeCoord->processTimeMessage(message);

            auto fed = getFederateCore(localP->getFederateId());
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
    auto fed = getHandleFederate(destination);
    if (fed == nullptr) {
        return 0;
    }
    return fed->getQueueSize(destination);
}

std::unique_ptr<Message> CommonCore::receive(interface_handle destination)
{
    auto fed = getHandleFederate(destination);
    if (fed == nullptr) {
        throw(InvalidIdentifier("invalid handle"));
    }
    if (fed->getState() != HELICS_EXECUTING) {
        return nullptr;
    }

    return fed->receive(destination);
}

std::unique_ptr<Message>
    CommonCore::receiveAny(local_federate_id federateID, interface_handle& endpoint_id)
{
    auto fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("FederateID is not valid (receiveAny)"));
    }
    if (fed->getState() != HELICS_EXECUTING) {
        endpoint_id = interface_handle();
        return nullptr;
    }
    return fed->receiveAny(endpoint_id);
}

uint64_t CommonCore::receiveCountAny(local_federate_id federateID)
{
    auto fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("FederateID is not valid (receiveCountAny)"));
    }
    if (fed->getState() != HELICS_EXECUTING) {
        return 0;
    }

    return fed->getQueueSize();
}

void CommonCore::logMessage(
    local_federate_id federateID,
    int logLevel,
    const std::string& messageToLog)
{
    global_federate_id gid;
    if (federateID == local_core_id) {
        gid = global_id.load();
    } else {
        auto fed = getFederateAt(federateID);
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
        auto fed = getFederateAt(federateID);
        if (fed == nullptr) {
            throw(InvalidIdentifier("FederateID is not valid (setLoggingCallback)"));
        }
        fed->setLogger(std::move(logFunction));
    }
}

uint16_t CommonCore::getNextAirlockIndex()
{
    uint16_t index = nextAirLock++;
    if (index >
        3) { // the increment is an atomic operation if the nextAirLock was not adjusted this could result in an out of
        // bounds exception if this check were not done
        index %= 4;
    }
    if (index == 3) {
        decltype(index) exp = 4;

        while (exp > 3) { // doing a lock free modulus we need to make sure the nextAirLock<4
            if (nextAirLock.compare_exchange_weak(exp, exp % 4)) {
                break;
            }
        }
    }
    return index;
}

void CommonCore::setFilterOperator(
    interface_handle filter,
    std::shared_ptr<FilterOperator> callback)
{
    static std::shared_ptr<FilterOperator> nullFilt = std::make_shared<NullFilterOperator>();
    auto hndl = getHandleInfo(filter);
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
    filtOpUpdate.source_handle = filter;
    actionQueue.push(filtOpUpdate);
}

FilterCoordinator* CommonCore::getFilterCoordinator(interface_handle handle)
{
    auto fnd = filterCoord.find(handle);
    if (fnd == filterCoord.end()) {
        if (brokerState < broker_state_t::operating) {
            // just make a dummy filterFunction so we have something to return
            auto ff = std::make_unique<FilterCoordinator>();
            auto ffp = ff.get();
            filterCoord.emplace(handle, std::move(ff));
            return ffp;
        }
        return nullptr;
    }
    return fnd->second.get();
}

void CommonCore::setIdentifier(const std::string& name)
{
    if (brokerState == broker_state_t::created) {
        identifier = name;
    } else {
        throw(
            InvalidFunctionCall("setIdentifier can only be called before the core is initialized"));
    }
}

void CommonCore::setQueryCallback(
    local_federate_id federateID,
    std::function<std::string(const std::string&)> queryFunction)
{
    auto fed = getFederateAt(federateID);
    if (fed == nullptr) {
        throw(InvalidIdentifier("FederateID is invalid (setQueryCallback)"));
    }
    fed->setQueryCallback(std::move(queryFunction));
}

std::string CommonCore::filteredEndpointQuery(const FederateState* fed) const
{
    Json::Value base;
    base["name"] = (fed != nullptr) ? fed->getIdentifier() : getIdentifier();
    base["id"] =
        (fed != nullptr) ? fed->global_id.load().baseValue() : global_broker_id_local.baseValue();
    base["endpoints"] = Json::arrayValue;
    for (auto& filt : filterCoord) {
        auto fc = filt.second.get();
        auto ep = loopHandles.getEndpoint(filt.first);
        if (fed != nullptr && ep->getFederateId() != fed->global_id) {
            continue;
        }
        Json::Value eptBlock;

        eptBlock["name"] = ep->key;
        eptBlock["id"] = ep->handle.handle.baseValue();
        if (fc->hasSourceFilters) {
            std::string srcFilters = "[";
            for (auto& fcc : fc->sourceFilters) {
                if (!fcc->key.empty()) {
                    srcFilters.append(fcc->key);
                } else {
                    srcFilters += std::to_string(fcc->core_id.baseValue()) + ':' +
                        std::to_string(fcc->handle.baseValue());
                }
                if (fcc->cloning) {
                    srcFilters.append("(cloning)");
                }
                srcFilters.push_back(',');
            }
            if (srcFilters.back() == ',') {
                srcFilters.pop_back();
            }
            srcFilters.push_back(']');
            eptBlock["srcFilters"] = srcFilters;
        }
        if (fc->hasDestFilters) {
            if (fc->destFilter != nullptr) {
                if (!fc->destFilter->key.empty()) {
                    eptBlock["destFilter"] = fc->destFilter->key;
                } else {
                    eptBlock["destFilter"] = std::to_string(fc->destFilter->core_id.baseValue()) +
                        ':' + std::to_string(fc->destFilter->handle.baseValue());
                }
            }
            if (!fc->cloningDestFilters.empty()) {
                std::string dcloningFilter = "[";
                for (auto& fcc : fc->cloningDestFilters) {
                    if (!fcc->key.empty()) {
                        dcloningFilter.append(fcc->key);
                    } else {
                        dcloningFilter += std::to_string(fcc->core_id.baseValue()) + ':' +
                            std::to_string(fcc->handle.baseValue());
                    }
                    dcloningFilter.push_back(',');
                }
                if (dcloningFilter.back() == ',') {
                    dcloningFilter.pop_back();
                }
                dcloningFilter.push_back(']');
                eptBlock["cloningdestFilter"] = dcloningFilter;
            }
        }
        base["endpoints"].append(eptBlock);
    }
    return generateJsonString(base);
}

std::string CommonCore::federateQuery(const FederateState* fed, const std::string& queryStr) const
{
    if (fed == nullptr) {
        if ((queryStr == "exists") || (queryStr == "exist")) {
            return "false";
        }
        return "#invalid";
    }
    if ((queryStr == "exists") || (queryStr == "exist")) {
        return "true";
    }
    if (queryStr == "isinit") {
        return (fed->init_transmitted.load()) ? "true" : "false";
    }
    if (queryStr == "state") {
        return std::to_string(static_cast<int>(fed->getState()));
    }
    if (queryStr == "filtered_endpoints") {
        return filteredEndpointQuery(fed);
    }
    if ((queryStr == "queries") || (queryStr == "available_queries")) {
        return std::string("[exists;isinit;state;queries;filtered_endpoints;") +
            fed->processQuery(queryStr) + "]";
    }
    return fed->processQuery(queryStr);
}

std::string CommonCore::quickCoreQueries(const std::string& queryStr) const
{
    if ((queryStr == "queries") || (queryStr == "available_queries")) {
        return "[isinit;isconnected;name;address;queries;address;federates;inputs;endpoints;filtered_endpoints;"
               "publications;filters;federate_map;dependency_graph;dependencies;dependson;dependents]";
    }
    if (queryStr == "isconnected") {
        return (isConnected()) ? "true" : "false";
    }
    if (queryStr == "name") {
        return getIdentifier();
    }
    return std::string{};
}
std::string CommonCore::coreQuery(const std::string& queryStr) const
{
    auto res = quickCoreQueries(queryStr);
    if (!res.empty()) {
        return res;
    }
    if (queryStr == "federates") {
        return generateStringVector(loopFederates, [](const auto& fed) {
            return fed->getIdentifier();
        });
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
            filters,
            [](const auto& filt) { return filt->key; },
            [this](const auto& filt) {
                return ((filt->core_id == global_broker_id_local) && !filt->key.empty());
            });
    }

    if (queryStr == "endpoints") {
        return generateStringVector_if(
            loopHandles,
            [](const auto& handle) { return handle.key; },
            [](const auto& handle) { return (handle.handleType == handle_type::endpoint); });
    }
    if (queryStr == "dependson") {
        return generateStringVector(timeCoord->getDependencies(), [](auto& dep) {
            return std::to_string(dep.baseValue());
        });
    }
    if (queryStr == "dependents") {
        return generateStringVector(timeCoord->getDependents(), [](auto& dep) {
            return std::to_string(dep.baseValue());
        });
    }

    if (queryStr == "isinit") {
        return (allInitReady()) ? "true" : "false";
    }

    if (queryStr == "address") {
        return getAddress();
    }
    if (queryStr == "filtered_endpoints") {
        return filteredEndpointQuery(nullptr);
    }
    if (queryStr == "dependencies") {
        Json::Value base;
        base["name"] = getIdentifier();
        base["id"] = global_broker_id_local.baseValue();
        base["parent"] = higher_broker_id.baseValue();
        base["dependents"] = Json::arrayValue;
        for (auto& dep : timeCoord->getDependents()) {
            base["dependents"].append(dep.baseValue());
        }
        base["dependencies"] = Json::arrayValue;
        for (auto& dep : timeCoord->getDependencies()) {
            base["dependencies"].append(dep.baseValue());
        }
        return generateJsonString(base);
    }
    if (queryStr == "federate_map") {
        Json::Value block;
        block["name"] = getIdentifier();
        block["id"] = global_broker_id_local.baseValue();
        block["parent"] = higher_broker_id.baseValue();
        block["federates"] = Json::arrayValue;
        for (auto fed : loopFederates) {
            Json::Value fedBlock;
            fedBlock["name"] = fed->getIdentifier();
            fedBlock["id"] = fed->global_id.load().baseValue();
            fedBlock["parent"] = global_broker_id_local.baseValue();
            block["federates"].append(fedBlock);
        }
        return generateJsonString(block);
    }
    if (queryStr == "dependency_graph") {
        Json::Value block;
        block["name"] = getIdentifier();
        block["id"] = global_broker_id_local.baseValue();
        block["parent"] = higher_broker_id.baseValue();
        block["federates"] = Json::arrayValue;
        block["dependents"] = Json::arrayValue;
        for (auto& dep : timeCoord->getDependents()) {
            block["dependents"].append(dep.baseValue());
        }
        block["dependencies"] = Json::arrayValue;
        for (auto& dep : timeCoord->getDependencies()) {
            block["dependencies"].append(dep.baseValue());
        }
        for (auto fed : loopFederates) {
            Json::Value fedBlock;
            fedBlock["name"] = fed->getIdentifier();
            fedBlock["id"] = fed->global_id.load().baseValue();
            fedBlock["parent"] = global_broker_id_local.baseValue();
            fedBlock["dependencies"] = Json::arrayValue;
            for (auto& dep : fed->getDependencies()) {
                fedBlock["dependencies"].append(dep.baseValue());
            }
            fedBlock["dependents"] = Json::arrayValue;
            for (auto& dep : fed->getDependents()) {
                fedBlock["dependents"].append(dep.baseValue());
            }
            block["federates"].append(fedBlock);
        }
        return generateJsonString(block);
    }
    return "#invalid";
}

std::string CommonCore::query(const std::string& target, const std::string& queryStr)
{
    if (brokerState.load() >= broker_state_t::terminating) {
        if ((target == "core") || (target == getIdentifier())) {
            auto res = quickCoreQueries(queryStr);
            if (!res.empty()) {
                return res;
            }
        }
        return "#disconnected";
    }
    if ((target == "core") || (target == getIdentifier())) {
        auto res = quickCoreQueries(queryStr);
        if (!res.empty()) {
            return res;
        }
        if (queryStr == "address") {
            return getAddress();
        }
        ActionMessage querycmd(CMD_BROKER_QUERY);
        querycmd.source_id = global_id.load();
        querycmd.dest_id = global_id.load();
        auto index = ++queryCounter;
        querycmd.messageID = index;
        querycmd.payload = queryStr;
        auto queryResult = ActiveQueries.getFuture(index);
        addActionMessage(std::move(querycmd));
        auto ret = queryResult.get();
        ActiveQueries.finishedWithValue(index);
        return ret;
    }
    if ((target == "parent") || (target == "broker")) {
        ActionMessage querycmd(CMD_BROKER_QUERY);
        querycmd.source_id = global_id.load();
        querycmd.dest_id = higher_broker_id;
        querycmd.messageID = ++queryCounter;
        querycmd.payload = queryStr;
        auto queryResult = ActiveQueries.getFuture(querycmd.messageID);
        addActionMessage(querycmd);
        auto ret = queryResult.get();
        ActiveQueries.finishedWithValue(querycmd.messageID);
        return ret;
    }
    if (target == "root" || target == "rootbroker") {
        ActionMessage querycmd(CMD_BROKER_QUERY);
        querycmd.source_id = global_id.load();
        auto index = ++queryCounter;
        querycmd.messageID = index;
        querycmd.payload = queryStr;
        auto queryResult = ActiveQueries.getFuture(querycmd.messageID);
        if (global_id.load() == parent_broker_id) {
            delayTransmitQueue.push(std::move(querycmd));
        } else {
            transmit(parent_route_id, querycmd);
        }
        auto ret = queryResult.get();
        ActiveQueries.finishedWithValue(index);
        return ret;
    }
    // default into a federate query
    auto fed = (target != "federate") ? getFederate(target) : getFederateAt(local_federate_id(0));
    if (fed != nullptr) {
        std::string ret = federateQuery(fed, queryStr);
        if (ret != "#wait") {
            return ret;
        }
        ActionMessage fedquerycmd(CMD_QUERY);
        fedquerycmd.source_id = global_id.load();
        fedquerycmd.dest_id = fed->global_id;
        auto index = ++queryCounter;
        fedquerycmd.messageID = index;
        fedquerycmd.payload = queryStr;
        fedquerycmd.setStringData(target);
        auto queryResult = ActiveQueries.getFuture(fedquerycmd.messageID);
        fed->addAction(std::move(fedquerycmd));
        std::future_status status = std::future_status::timeout;
        while (status == std::future_status::timeout) {
            status = queryResult.wait_for(std::chrono::milliseconds(50));
            if (status == std::future_status::ready) {
                auto qres = queryResult.get();
                ActiveQueries.finishedWithValue(index);
                return qres;
            }
            ret = federateQuery(fed, queryStr);
            if (ret != "#wait") {
                ActiveQueries.finishedWithValue(index);
                return ret;
            }
        }
    }

    ActionMessage querycmd(CMD_QUERY);
    querycmd.source_id = global_id.load();
    auto index = ++queryCounter;
    querycmd.messageID = index;
    querycmd.payload = queryStr;
    querycmd.setStringData(target);
    auto queryResult = ActiveQueries.getFuture(querycmd.messageID);
    if (global_id.load() == parent_broker_id) {
        delayTransmitQueue.push(std::move(querycmd));
    } else {
        transmit(parent_route_id, querycmd);
    }

    auto ret = queryResult.get();
    ActiveQueries.finishedWithValue(index);
    return ret;
}

void CommonCore::setGlobal(const std::string& valueName, const std::string& value)
{
    ActionMessage querycmd(CMD_SET_GLOBAL);
    querycmd.source_id = global_id.load();
    querycmd.payload = valueName;
    querycmd.setStringData(value);
    if (global_id.load() == parent_broker_id) {
        delayTransmitQueue.push(std::move(querycmd));
    } else {
        transmit(parent_route_id, querycmd);
    }
}

void CommonCore::processPriorityCommand(ActionMessage&& command)
{
    // deal with a few types of message immediately
    LOG_TRACE(
        global_broker_id_local,
        getIdentifier(),
        fmt::format(
            "|| priority_cmd:{} from {}",
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
        case CMD_REG_FED: {
            // this one in the core needs to be the thread-safe version
            auto fed = getFederate(command.name);
            loopFederates.insert(command.name, no_search, fed);
        }
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
            // These really shouldn't happen here probably means something went wrong in setup but we can handle it
            // forward the connection request to the higher level
            if (command.name == identifier) {
                LOG_ERROR(
                    global_broker_id_local,
                    identifier,
                    "received locally sent registration message, broker loop, please set the broker address to "
                    "a valid broker");
            } else {
                LOG_WARNING(
                    parent_broker_id,
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
                timeCoord->source_id = global_broker_id_local;
                higher_broker_id = global_broker_id(command.source_id);
                transmitDelayedMessages();
                timeoutMon->setParentId(higher_broker_id);
                if (checkActionFlag(command, slow_responding_flag)) {
                    timeoutMon->disableParentPing();
                }
                timeoutMon->reset();
            }
            break;
        case CMD_FED_ACK: {
            auto fed = getFederateCore(command.name);
            if (fed != nullptr) {
                if (checkActionFlag(command, error_flag)) {
                    LOG_ERROR(
                        parent_broker_id,
                        identifier,
                        fmt::format(
                            "broker responded with error for registration of {}::{}\n",
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
            // TODO:: double check this
            addRoute(route_id(command.getExtraData()), command.payload);
            break;
        case CMD_PRIORITY_DISCONNECT:
            checkAndProcessDisconnect();
            break;
        case CMD_BROKER_QUERY:
            if (command.dest_id == global_broker_id_local) {
                std::string repStr = coreQuery(command.payload);
                if (command.source_id == global_broker_id_local) {
                    ActiveQueries.setDelayedValue(command.messageID, std::move(repStr));
                } else {
                    ActionMessage queryResp(CMD_QUERY_REPLY);
                    queryResp.dest_id = command.source_id;
                    queryResp.source_id = global_broker_id_local;
                    queryResp.messageID = command.messageID;
                    queryResp.payload = std::move(repStr);
                    queryResp.counter = command.counter;
                    transmit(getRoute(queryResp.dest_id), queryResp);
                }
            } else {
                routeMessage(std::move(command));
            }
            break;
        case CMD_QUERY: {
            std::string repStr;
            ActionMessage queryResp(CMD_QUERY_REPLY);
            queryResp.dest_id = command.source_id;
            queryResp.source_id = command.dest_id;
            queryResp.messageID = command.messageID;
            queryResp.counter = command.counter;
            const std::string& target = command.getString(targetStringLoc);
            if (target == getIdentifier()) {
                queryResp.source_id = global_broker_id_local;
                repStr = query(target, command.payload);
            } else {
                auto fedptr = getFederateCore(target);
                repStr = federateQuery(fedptr, command.payload);
                if (repStr == "#wait") {
                    if (fedptr != nullptr) {
                        command.dest_id = fedptr->global_id;
                        fedptr->addAction(std::move(command));
                        break;
                    } else {
                        repStr = "#error";
                    }
                }
            }

            queryResp.payload = std::move(repStr);
            transmit(getRoute(queryResp.dest_id), queryResp);
        } break;
        case CMD_QUERY_REPLY:
            if (command.dest_id == global_broker_id_local) {
                ActiveQueries.setDelayedValue(command.messageID, command.payload);
            }
            break;
        case CMD_PRIORITY_ACK:
        case CMD_ROUTE_ACK:
            break;
        default: {
            if (!isPriorityCommand(command)) {
                processCommand(std::move(command));
            }
        }

            // case CMD_DISCONNECT_ACK:
            //	break;
    }
}

void CommonCore::transmitDelayedMessages()
{
    auto msg = delayTransmitQueue.pop();
    while (msg) {
        if (msg->source_id == parent_broker_id) {
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
            (*msg).action() ==
                CMD_BROKER_QUERY) { // deal with in flight queries that will block unless a response is given
            ActiveQueries.setDelayedValue((*msg).messageID, std::string("#error:") + estring);
        }
        // else other message which might get into here shouldn't need any action, just drop them
        msg = delayTransmitQueue.pop();
    }
}

void CommonCore::sendErrorToFederates(int error_code)
{
    ActionMessage errorCom(CMD_ERROR);
    errorCom.messageID = error_code;
    loopFederates.apply([&errorCom](auto& fed) {
        if ((fed) && (!fed.disconnected)) {
            fed->addAction(errorCom);
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
        } else { // these messages were delayed for a different purpose and will be dealt with in a different way
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
    LOG_TRACE(
        global_broker_id_local,
        getIdentifier(),
        fmt::format(
            "|| cmd:{} from {}", prettyPrintString(command), command.source_id.baseValue()));
    switch (command.action()) {
        case CMD_IGNORE:
            break;
        case CMD_TICK:
            if (brokerState == broker_state_t::operating) {
                timeoutMon->tick(this);
                LOG_SUMMARY(global_broker_id_local, getIdentifier(), " core tick");
            }
            break;
        case CMD_PING:
        case CMD_BROKER_PING: // broker ping for core is the same as core
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
        case CMD_CHECK_CONNECTIONS: {
            auto res = checkAndProcessDisconnect();
            auto pred = [](const auto& fed) {
                auto state = fed->getState();
                return (HELICS_FINISHED == state) || (HELICS_ERROR == state);
            };
            auto afed = std::all_of(loopFederates.begin(), loopFederates.end(), pred);
            LOG_WARNING(
                global_broker_id_local,
                getIdentifier(),
                fmt::format(
                    "CHECK CONNECTIONS {}, federates={}, fed_disconnected={}",
                    res,
                    loopFederates.size(),
                    afed));
        }

        break;
        case CMD_USER_DISCONNECT:
            if (isConnected()) {
                if (brokerState <
                    broker_state_t::
                        terminating) { // only send a disconnect message if we haven't done so already
                    brokerState = broker_state_t::terminating;
                    sendDisconnect();
                    ActionMessage m(CMD_DISCONNECT);
                    m.source_id = global_broker_id_local;
                    transmit(parent_route_id, m);
                }
            }
            addActionMessage(CMD_STOP);
            // we can't just fall through since this may have generated other messages that need to be forwarded or
            // processed
            break;
        case CMD_BROADCAST_DISCONNECT: {
            timeCoord->processTimeMessage(command);
            loopFederates.apply([&command](auto& fed) { fed->addAction(command); });
            checkAndProcessDisconnect();
        } break;
        case CMD_STOP:

            if (isConnected()) {
                if (brokerState <
                    broker_state_t::
                        terminating) { // only send a disconnect message if we haven't done so already
                    brokerState = broker_state_t::terminating;
                    sendDisconnect();
                    ActionMessage m(CMD_DISCONNECT);
                    m.source_id = global_broker_id_local;
                    transmit(parent_route_id, m);
                }
            }
            ActiveQueries.fulfillAllPromises("#disconnected");
            break;

        case CMD_EXEC_GRANT:
        case CMD_EXEC_REQUEST:
            if (isLocal(global_broker_id(command.source_id))) {
                if (!ongoingFilterProcesses[command.source_id.baseValue()].empty()) {
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
        case CMD_TIME_REQUEST:
        case CMD_TIME_GRANT:
            if (isLocal(command.source_id)) {
                if (!ongoingFilterProcesses[command.source_id.baseValue()].empty()) {
                    delayedTimingMessages[command.source_id.baseValue()].push_back(command);
                    break;
                }
            }
            if (command.source_id == global_broker_id_local) {
                for (auto dep : timeCoord->getDependents()) {
                    routeMessage(command, dep);
                }
            } else {
                routeMessage(command);
            }
            break;
        case CMD_DISCONNECT:
        case CMD_DISCONNECT_FED:
            if (command.dest_id == parent_broker_id) {
                if (brokerState < broker_state_t::terminating) {
                    auto fed = loopFederates.find(command.source_id);
                    if (fed == loopFederates.end()) {
                        return;
                    }
                    fed->disconnected = true;
                    auto cstate = brokerState.load();
                    if ((!checkAndProcessDisconnect()) || (cstate < broker_state_t::operating)) {
                        command.setAction(CMD_DISCONNECT_FED);
                        transmit(parent_route_id, command);
                        if (!allFedDisconnected()) {
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
            auto fed = getFederateCore(command.name);
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
            processMessageFilter(command);
            break;
        case CMD_NULL_MESSAGE:
        case CMD_FILTER_RESULT:
            processFilterReturn(command);
            break;
        case CMD_DEST_FILTER_RESULT:
        case CMD_NULL_DEST_MESSAGE:
            processDestFilterReturn(command);
            break;
        case CMD_PUB:
            routeMessage(command);
            break;
        case CMD_LOG:
            if (command.dest_id == global_broker_id_local) {
                sendToLogger(
                    parent_broker_id,
                    command.messageID,
                    getFederateNameNoThrow(command.source_id),
                    command.payload);
            } else {
                routeMessage(command);
            }
            break;
        case CMD_WARNING:
            if (command.dest_id == global_broker_id_local) {
                sendToLogger(
                    command.source_id,
                    log_level::warning,
                    getFederateNameNoThrow(command.source_id),
                    command.payload);
            } else {
                routeMessage(command);
            }
            break;
        case CMD_ERROR:
            if (command.dest_id == global_broker_id_local) {
                if (command.source_id == higher_broker_id ||
                    command.source_id == parent_broker_id) {
                    sendErrorToFederates(command.messageID);
                    brokerState = broker_state_t::errored;
                } else {
                    sendToLogger(
                        parent_broker_id,
                        log_level::error,
                        getFederateNameNoThrow(command.source_id),
                        command.payload);
                }
            } else {
                routeMessage(command);
            }
            break;
        case CMD_DATA_LINK: {
            auto pub = loopHandles.getPublication(command.name);
            if (pub != nullptr) {
                command.name = command.getString(targetStringLoc);
                command.setAction(CMD_ADD_NAMED_INPUT);
                command.setSource(pub->handle);
                command.clearStringData();
                checkForNamedInterface(command);
            } else {
                auto input = loopHandles.getInput(command.getString(targetStringLoc));
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
            auto filt = loopHandles.getFilter(command.name);
            if (filt != nullptr) {
                command.name = command.getString(targetStringLoc);
                command.setAction(CMD_ADD_NAMED_ENDPOINT);
                command.setSource(filt->handle);
                if (checkActionFlag(*filt, clone_flag)) {
                    setActionFlag(command, clone_flag);
                }
                checkForNamedInterface(command);
            } else {
                auto ept = loopHandles.getEndpoint(command.getString(targetStringLoc));
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
            auto fed = getFederateCore(command.source_id);
            if (fed != nullptr) {
                fed->init_transmitted = true;
                if (allInitReady()) {
                    broker_state_t exp = broker_state_t::connected;
                    if (brokerState.compare_exchange_strong(
                            exp, broker_state_t::initializing)) { // make sure we only do this once
                        checkDependencies();
                        command.source_id = global_broker_id_local;
                        transmit(parent_route_id, command);
                    }
                }
            }
        } break;
        case CMD_INIT_GRANT: {
            broker_state_t exp = broker_state_t::initializing;
            if (brokerState.compare_exchange_strong(
                    exp, broker_state_t::operating)) { // forward the grant to all federates
                organizeFilterOperations();
                loopFederates.apply([&command](auto& fed) { fed->addAction(command); });
                timeCoord->enteringExecMode();
                auto res = timeCoord->checkExecEntry();
                if (res == message_processing_result::next_step) {
                    enteredExecutionMode = true;
                }
                if (!timeCoord->hasActiveTimeDependencies()) {
                    timeCoord->disconnect();
                }
            }
        } break;

        case CMD_SEND_MESSAGE:
            if ((command.dest_id == parent_broker_id) && (isLocal(command.source_id))) {
                deliverMessage(processMessage(command));
            } else {
                deliverMessage(command);
            }

            break;
        default:
            if (isPriorityCommand(
                    command)) { // this is a backup if somehow one of these message got here
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
                    auto fed = getFederateCore(command.source_id);
                    if (fed != nullptr) {
                        ActionMessage add(
                            CMD_ADD_INTERDEPENDENCY, global_broker_id_local, command.source_id);

                        fed->addAction(add);
                        timeCoord->addDependent(fed->global_id);
                    }
                }

                if (!hasTimeDependency) {
                    if (timeCoord->addDependency(higher_broker_id)) {
                        hasTimeDependency = true;
                        ActionMessage add(
                            CMD_ADD_INTERDEPENDENCY, global_broker_id_local, higher_broker_id);
                        transmit(getRoute(higher_broker_id), add);

                        timeCoord->addDependent(higher_broker_id);
                    }
                }
                break;
            case CMD_REG_FILTER:

                createFilter(
                    global_broker_id_local,
                    command.source_handle,
                    command.name,
                    command.getString(typeStringLoc),
                    command.getString(typeOutStringLoc),
                    checkActionFlag(command, clone_flag));
                if (!hasFilters) {
                    hasFilters = true;
                    if (timeCoord->addDependent(higher_broker_id)) {
                        ActionMessage add(
                            CMD_ADD_INTERDEPENDENCY, global_broker_id_local, higher_broker_id);
                        transmit(getRoute(higher_broker_id), add);
                        timeCoord->addDependency(higher_broker_id);
                    }
                }
                break;
            default:
                return;
        }
        if (!command.name.empty()) {
            transmit(parent_route_id, std::move(command));
        }
    } else if (command.dest_id == global_broker_id_local) {
        if (command.action() == CMD_REG_ENDPOINT) {
            auto filtI = filters.find(global_handle(global_broker_id_local, command.dest_handle));
            if (filtI != nullptr) {
                filtI->sourceTargets.emplace_back(command.source_id, command.source_handle);
                timeCoord->addDependency(command.source_id);
            }
            auto filthandle = loopHandles.getFilter(command.dest_handle);
            if (filthandle != nullptr) {
                filthandle->used = true;
            }
        } else if (command.action() == CMD_REG_FILTER) {
            processFilterInfo(command);
        }
    } else {
        routeMessage(std::move(command));
    }
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
            auto pub = loopHandles.getPublication(command.name);
            if (pub != nullptr) {
                if (checkActionFlag(*pub, disconnected_flag)) {
                    // TODO:: this might generate an error if the required flag was set
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
            auto inputName = command.name;
            auto inp = loopHandles.getInput(inputName);
            if (inp != nullptr) {
                if (checkActionFlag(*inp, disconnected_flag)) {
                    // TODO:: this might generate an error if the required flag was set
                    return;
                }
                command.setAction(CMD_ADD_PUBLISHER);
                command.setDestination(inp->handle);
                command.name.clear();
                if (command.getStringData().empty()) {
                    auto pub = loopHandles.findHandle(command.getSource());
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
            auto filt = loopHandles.getFilter(command.name);
            if (filt != nullptr) {
                if (checkActionFlag(*filt, disconnected_flag)) {
                    // TODO:: this might generate an error if the required flag was set
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
            auto ept = loopHandles.getEndpoint(command.name);
            if (ept != nullptr) {
                if (checkActionFlag(*ept, disconnected_flag)) {
                    // TODO:: this might generate an error if the required flag was set
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
            auto pub = loopHandles.getPublication(command.name);
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
            auto inp = loopHandles.getInput(command.name);
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
            auto filt = loopHandles.getFilter(command.name);
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
            auto pub = loopHandles.getEndpoint(command.name);
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
    if (handleInfo->getFederateId() == global_broker_id_local) {
        // DO something with filters
        auto* filt = filters.find(command.getSource());
        if (filt != nullptr) {
            ActionMessage rem(CMD_REMOVE_FILTER);
            rem.setSource(handleInfo->handle);
            for (auto& target : filt->sourceTargets) {
                rem.setDestination(target);
                routeMessage(rem);
            }
            for (auto& target : filt->destTargets) {
                if (std::find(filt->sourceTargets.begin(), filt->sourceTargets.end(), target) !=
                    filt->sourceTargets.end()) {
                    rem.setDestination(target);
                    routeMessage(rem);
                }
            }
            filt->sourceTargets.clear();
            filt->destTargets.clear();
            setActionFlag(*filt, disconnected_flag);
        }
    } else {
        if (handleInfo->handleType != handle_type::filter) {
            auto fed = getFederateCore(command.source_id);
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
        processFilterInfo(command);
        if (command.source_id != global_broker_id_local) {
            if (!checkActionFlag(command, error_flag)) {
                auto fed = getFederateCore(command.dest_id);
                if (fed != nullptr) {
                    command.setAction(CMD_ADD_DEPENDENT);
                    fed->addAction(command);
                }
            }
        }
    }
    // just forward these to the appropriate federate
    else if (command.dest_id == global_broker_id_local) {
        if (command.action() == CMD_ADD_ENDPOINT) {
            auto filtI = filters.find(global_handle(global_broker_id_local, command.dest_handle));
            if (filtI != nullptr) {
                if (checkActionFlag(command, destination_target)) {
                    filtI->destTargets.emplace_back(command.getSource());
                } else {
                    filtI->sourceTargets.emplace_back(command.getSource());
                }
                if (!checkActionFlag(command, error_flag)) {
                    timeCoord->addDependency(command.source_id);
                }
            }

            auto filthandle = loopHandles.getFilter(command.dest_handle);
            if (filthandle != nullptr) {
                filthandle->used = true;
            }
        }
    } else {
        auto fed = getFederateCore(command.dest_id);
        if (fed != nullptr) {
            if (!checkActionFlag(command, error_flag)) {
                fed->addAction(command);
            }
            auto handle = loopHandles.getHandleInfo(command.dest_handle.baseValue());
            if (handle != nullptr) {
                setAsUsed(handle);
            }
        }
    }
}

void CommonCore::removeTargetFromInterface(ActionMessage& command)
{
    if (command.action() == CMD_REMOVE_FILTER) {
        auto* filterC = getFilterCoordinator(command.dest_handle);
        if (filterC == nullptr) {
            return;
        }
        filterC->closeFilter(command.getSource());
    } else if (command.dest_id == global_broker_id_local) {
        if (command.action() == CMD_REMOVE_ENDPOINT) {
            auto filtI = filters.find(command.getDest());
            if (filtI != nullptr) {
                filtI->removeTarget(command.getSource());
            }
        }
    } else { // just forward these to the appropriate federate
        auto fed = getFederateCore(command.dest_id);
        if (fed != nullptr) {
            fed->addAction(command);
        }
    }
}

void CommonCore::processFilterInfo(ActionMessage& command)
{
    auto filterC = getFilterCoordinator(command.dest_handle);
    if (filterC == nullptr) {
        return;
    }
    bool FilterAlreadyPresent = false;
    if (checkActionFlag(command, destination_target)) {
        if (checkActionFlag(command, clone_flag)) {
            for (auto& filt : filterC->cloningDestFilters) {
                if ((filt->core_id == command.source_id) &&
                    (filt->handle == command.source_handle)) {
                    FilterAlreadyPresent = true;
                    break;
                }
            }
        } else { // there can only be one non-cloning destination filter
            if (filterC->destFilter != nullptr) {
                if ((filterC->destFilter->core_id == command.source_id) &&
                    (filterC->destFilter->handle == command.source_handle)) {
                    FilterAlreadyPresent = true;
                }
            }
        }

        if (!FilterAlreadyPresent) {
            auto endhandle = loopHandles.getEndpoint(command.dest_handle);
            if (endhandle != nullptr) {
                setActionFlag(*endhandle, has_dest_filter_flag);
                if ((!checkActionFlag(command, clone_flag)) && (filterC->hasDestFilters)) {
                    // duplicate non cloning destination filters are not allowed
                    ActionMessage err(CMD_ERROR);
                    err.dest_id = command.source_id;
                    err.setSource(command.getDest());
                    err.messageID = defs::errors::registration_failure;
                    err.payload =
                        "Endpoint " + endhandle->key + " already has a destination filter";
                    routeMessage(std::move(err));
                    return;
                }
            }
            auto newFilter = filters.find(command.getSource());
            if (newFilter == nullptr) {
                newFilter = createFilter(
                    global_broker_id(command.source_id),
                    command.source_handle,
                    command.name,
                    command.getString(typeStringLoc),
                    command.getString(typeOutStringLoc),
                    checkActionFlag(command, clone_flag));
            }

            filterC->hasDestFilters = true;
            if (checkActionFlag(command, clone_flag)) {
                filterC->cloningDestFilters.push_back(newFilter);
            } else {
                if (endhandle != nullptr) {
                    setActionFlag(*endhandle, has_non_cloning_dest_filter_flag);
                }
                filterC->destFilter = newFilter;
            }
        }
    } else {
        for (auto& filt : filterC->allSourceFilters) {
            if ((filt->core_id == command.source_id) && (filt->handle == command.source_handle)) {
                FilterAlreadyPresent = true;
                break;
            }
        }
        if (!FilterAlreadyPresent) {
            auto newFilter = filters.find(command.getSource());
            if (newFilter == nullptr) {
                newFilter = createFilter(
                    global_broker_id(command.source_id),
                    command.source_handle,
                    command.name,
                    command.getString(typeStringLoc),
                    command.getString(typeOutStringLoc),
                    checkActionFlag(command, clone_flag));
            }
            filterC->allSourceFilters.push_back(newFilter);
            filterC->hasSourceFilters = true;
            auto endhandle = loopHandles.getEndpoint(command.dest_handle);
            if (endhandle != nullptr) {
                setActionFlag(*endhandle, has_source_filter_flag);
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

    // if the core has filters we need to be a timeCoordinator
    if (hasFilters) {
        return;
    }
    // if there is more than 2 dependents or dependencies (higher broker + 2 or more federates) then we need to be
    // a timeCoordinator
    if (timeCoord->getDependents().size() > 2) {
        return;
    }
    if (timeCoord->getDependencies().size() > 2) {
        return;
    }
    global_federate_id fedid;
    global_broker_id brkid;
    int localcnt = 0;
    for (auto& dep : timeCoord->getDependents()) {
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
    // remove the core from the time dependency chain since it is just adding to the communication noise in this
    // case
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
        routeMessage(adddep, brkid);
        adddep.setAction(CMD_ADD_DEPENDENCY);
        adddep.source_id = brkid;
        routeMessage(adddep, fedid);
    } else if (issource) {
        ActionMessage adddep(CMD_ADD_DEPENDENCY);
        adddep.source_id = fedid;
        routeMessage(adddep, brkid);
        adddep.setAction(CMD_ADD_DEPENDENT);
        adddep.source_id = brkid;
        routeMessage(adddep, fedid);
    } else {
        ActionMessage adddep(CMD_ADD_INTERDEPENDENCY);
        adddep.source_id = fedid;
        routeMessage(adddep, brkid);
        routeMessage(
            adddep,
            fedid); // make sure the fed depends on itself in case the broker removes itself later
        adddep.source_id = brkid;
        routeMessage(adddep, fedid);
    }
}

void CommonCore::organizeFilterOperations()
{
    for (auto& fc : filterCoord) {
        auto* fi = fc.second.get();
        auto* handle = getHandleInfo(fc.first);
        if (handle == nullptr) {
            continue;
        }
        std::string endpointType = handle->type;

        if (!fi->allSourceFilters.empty()) {
            fi->sourceFilters.clear();
            fi->sourceFilters.reserve(fi->allSourceFilters.size());
            // Now we have to do some intelligent ordering with types
            std::vector<bool> used(fi->allSourceFilters.size(), false);
            bool someUnused = true;
            bool usedMore = true;
            bool firstPass = true;
            std::string currentType = endpointType;
            while (someUnused && usedMore) {
                someUnused = false;
                usedMore = false;
                for (size_t ii = 0; ii < fi->allSourceFilters.size(); ++ii) {
                    if (used[ii]) {
                        continue;
                    }
                    if (firstPass) {
                        if (fi->allSourceFilters[ii]->cloning) {
                            fi->sourceFilters.push_back(fi->allSourceFilters[ii]);
                            used[ii] = true;
                            usedMore = true;
                        } else {
                            someUnused = true;
                        }
                    } else {
                        // TODO:: this will need some work to finish sorting out but should work for initial tests
                        if (core::matchingTypes(fi->allSourceFilters[ii]->inputType, currentType)) {
                            used[ii] = true;
                            usedMore = true;
                            fi->sourceFilters.push_back(fi->allSourceFilters[ii]);
                            currentType = fi->allSourceFilters[ii]->outputType;
                        } else {
                            someUnused = true;
                        }
                    }
                }
                if (firstPass) {
                    firstPass = false;
                    usedMore = true;
                }
            }
            for (size_t ii = 0; ii < fi->allSourceFilters.size(); ++ii) {
                if (used[ii]) {
                    continue;
                }
                LOG_WARNING(
                    global_broker_id_local,
                    fi->allSourceFilters[ii]->key,
                    "unable to match types on some filters");
            }
        }
    }
}
void CommonCore::processCoreConfigureCommands(ActionMessage& cmd)
{
    switch (cmd.messageID) {
        case defs::flags::enable_init_entry:
            if (delayInitCounter <= 1) {
                delayInitCounter = 0;
                if (allInitReady()) {
                    broker_state_t exp = broker_state_t::connected;
                    if (brokerState.compare_exchange_strong(
                            exp, broker_state_t::initializing)) { // make sure we only do this once
                        checkDependencies();
                        cmd.setAction(CMD_INIT);
                        cmd.source_id = global_broker_id_local;
                        cmd.dest_id = parent_broker_id;
                        transmit(parent_route_id, cmd);
                    }
                }
            } else {
                --delayInitCounter;
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
        case defs::flags::slow_responding:
            no_ping = checkActionFlag(cmd, indicator_flag);
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
        case UPDATE_FILTER_OPERATOR: {
            auto FiltI = filters.find(global_handle(global_broker_id_local, cmd.source_handle));
            int ii = cmd.counter;
            auto op = dataAirlocks[ii].try_unload();
            if (op) {
                auto M = stx::any_cast<std::shared_ptr<FilterOperator>>(std::move(*op));
                FiltI->filterOp = std::move(M);
            }
        } break;
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
                brokerState = broker_state_t::terminating;
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
    } else {
        LOG_WARNING(global_broker_id_local, "core", "dropping message:" + prettyPrintString(cmd));
    }
}

bool CommonCore::waitCoreRegistration()
{
    int sleepcnt = 0;
    auto brkid = global_id.load();
    while ((brkid == parent_broker_id) || (!brkid.isValid())) {
        if (sleepcnt > 6) {
            LOG_WARNING(
                parent_broker_id,
                identifier,
                fmt::format(
                    "broker state={}, broker id={}, sleepcnt={}",
                    static_cast<int>(brokerState.load()),
                    brkid.baseValue(),
                    sleepcnt));
        }
        if (brokerState.load() <= broker_state_t::configured) {
            connect();
        }
        if (brokerState.load() >= broker_state_t::terminating) {
            return false;
        }
        if (sleepcnt == 4) {
            LOG_WARNING(
                parent_broker_id,
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

bool CommonCore::checkAndProcessDisconnect()
{
    if ((brokerState == broker_state_t::terminating) ||
        (brokerState == broker_state_t::terminated)) {
        return true;
    }
    if (allDisconnected()) {
        brokerState = broker_state_t::terminating;
        timeCoord->disconnect();
        ActionMessage dis(CMD_DISCONNECT);
        dis.source_id = global_broker_id_local;
        transmit(parent_route_id, dis);
        return true;
    }
    return false;
}

void CommonCore::sendDisconnect()
{
    LOG_CONNECTIONS(global_broker_id_local, "core", "sending disconnect");
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
}
bool CommonCore::checkForLocalPublication(ActionMessage& cmd)
{
    auto pub = loopHandles.getPublication(cmd.name);
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
    } else if (isLocal(dest)) {
        auto fed = getFederateCore(dest);
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
    } else if (isLocal(cmd.dest_id)) {
        auto fed = getFederateCore(cmd.dest_id);
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
    } else if (isLocal(dest)) {
        auto fed = getFederateCore(dest);
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
    } else if (isLocal(dest)) {
        auto fed = getFederateCore(dest);
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

// Checks for filter operations
ActionMessage& CommonCore::processMessage(ActionMessage& m)
{
    auto handle = loopHandles.getEndpoint(m.source_handle);
    if (handle == nullptr) {
        return m;
    }
    if (checkActionFlag(*handle, has_source_filter_flag)) {
        auto filtFunc = getFilterCoordinator(handle->getInterfaceHandle());
        if (filtFunc->hasSourceFilters) {
            //   for (int ii = 0; ii < static_cast<int> (filtFunc->sourceFilters.size ()); ++ii)
            size_t ii = 0;
            for (auto& filt : filtFunc->sourceFilters) {
                if (checkActionFlag(*filt, disconnected_flag)) {
                    continue;
                }
                if (filt->core_id == global_broker_id_local) {
                    if (filt->cloning) {
                        // cloning filter returns a vector
                        auto new_messages =
                            filt->filterOp->processVector(createMessageFromCommand(m));
                        for (auto& msg : new_messages) {
                            if (msg) {
                                ActionMessage cmd(std::move(msg));
                                deliverMessage(cmd);
                            }
                        }
                    } else {
                        // deal with local source filters
                        auto tempMessage = createMessageFromCommand(std::move(m));
                        tempMessage = filt->filterOp->process(std::move(tempMessage));
                        if (tempMessage) {
                            m = ActionMessage(std::move(tempMessage));
                        } else {
                            // the filter dropped the message;
                            m = CMD_IGNORE;
                            return m;
                        }
                    }
                } else if (filt->cloning) {
                    ActionMessage cloneMessage(m);
                    cloneMessage.setAction(CMD_SEND_FOR_FILTER);
                    setActionFlag(cloneMessage, clone_flag);
                    cloneMessage.dest_id = filt->core_id;
                    cloneMessage.dest_handle = filt->handle;
                    routeMessage(cloneMessage);
                } else {
                    m.dest_id = filt->core_id;
                    m.dest_handle = filt->handle;
                    m.counter = static_cast<uint16_t>(ii);
                    if (ii < filtFunc->sourceFilters.size() - 1) {
                        m.setAction(CMD_SEND_FOR_FILTER_AND_RETURN);
                        ongoingFilterProcesses[handle->getFederateId().baseValue()].insert(
                            m.messageID);
                    } else {
                        m.setAction(CMD_SEND_FOR_FILTER);
                    }
                    return m;
                }
                ++ii;
            }
        }
    }

    return m;
}

void CommonCore::processDestFilterReturn(ActionMessage& command)
{
    auto handle = loopHandles.getEndpoint(command.dest_handle);
    if (handle == nullptr) {
        return;
    }
    auto messID = command.messageID;
    auto& ongoingDestProcess = ongoingDestFilterProcesses[handle->getFederateId().baseValue()];
    if (ongoingDestProcess.find(messID) != ongoingDestProcess.end()) {
        ongoingDestProcess.erase(messID);
        if (command.action() == CMD_NULL_DEST_MESSAGE) {
            ActionMessage removeTimeBlock(
                CMD_TIME_UNBLOCK, global_broker_id_local, command.dest_id);
            removeTimeBlock.messageID = messID;
            routeMessage(removeTimeBlock);
            return;
        }
        auto filtFunc = getFilterCoordinator(handle->getInterfaceHandle());

        // now go to the cloning filters
        for (auto clFilter : filtFunc->cloningDestFilters) {
            if (checkActionFlag(*clFilter, disconnected_flag)) {
                continue;
            }
            if (clFilter->core_id == global_broker_id_local) {
                auto FiltI = filters.find(global_handle(global_broker_id_local, clFilter->handle));
                if (FiltI != nullptr) {
                    if (FiltI->filterOp != nullptr) {
                        if (FiltI->cloning) {
                            (void)(FiltI->filterOp->process(createMessageFromCommand(command)));
                        }
                    }
                }
            } else {
                ActionMessage clone(command);
                clone.setAction(CMD_SEND_FOR_FILTER);
                clone.dest_id = clFilter->core_id;
                clone.dest_handle = clFilter->handle;
                routeMessage(clone);
            }
        }

        timeCoord->processTimeMessage(command);
        command.setAction(CMD_SEND_MESSAGE);
        routeMessage(std::move(command));
        // now unblock the time
        ActionMessage removeTimeBlock(
            CMD_TIME_UNBLOCK, global_broker_id_local, handle->getFederateId());
        removeTimeBlock.messageID = messID;
        routeMessage(removeTimeBlock);
    }
}

void CommonCore::processFilterReturn(ActionMessage& cmd)
{
    auto handle = loopHandles.getEndpoint(cmd.dest_handle);
    if (handle == nullptr) {
        return;
    }

    auto messID = cmd.messageID;
    auto fid = handle->getFederateId();
    auto fid_index = fid.baseValue();
    if (ongoingFilterProcesses[fid_index].find(messID) != ongoingFilterProcesses[fid_index].end()) {
        if (cmd.action() == CMD_NULL_MESSAGE) {
            ongoingFilterProcesses[fid_index].erase(messID);
            if (ongoingFilterProcesses[fid_index].empty()) {
                transmitDelayedMessages(fid);
            }
        }
        auto filtFunc = getFilterCoordinator(handle->getInterfaceHandle());
        if (filtFunc->hasSourceFilters) {
            for (
                decltype(cmd.counter) ii = cmd.counter + 1; ii < filtFunc->sourceFilters.size();
                ++ii) { // cloning filters come first so we don't need to check for them in this code branch
                auto filt = filtFunc->sourceFilters[ii];
                if (checkActionFlag(*filt, disconnected_flag)) {
                    continue;
                }
                if (filt->core_id == global_broker_id_local) {
                    // deal with local source filters
                    auto tempMessage = createMessageFromCommand(std::move(cmd));
                    tempMessage = filt->filterOp->process(std::move(tempMessage));
                    if (tempMessage) {
                        cmd = ActionMessage(std::move(tempMessage));
                    } else {
                        ongoingFilterProcesses[fid_index].erase(messID);
                        if (ongoingFilterProcesses[fid_index].empty()) {
                            transmitDelayedMessages(fid);
                        }
                        return;
                    }
                } else {
                    cmd.dest_id = filt->core_id;
                    cmd.dest_handle = filt->handle;
                    cmd.counter = static_cast<uint16_t>(ii);
                    if (ii < filtFunc->sourceFilters.size() - 1) {
                        cmd.setAction(CMD_SEND_FOR_FILTER_AND_RETURN);
                    } else {
                        cmd.setAction(CMD_SEND_FOR_FILTER);
                        ongoingFilterProcesses[fid_index].erase(messID);
                    }
                    routeMessage(cmd);
                    if (ongoingFilterProcesses[fid_index].empty()) {
                        transmitDelayedMessages(fid);
                    }
                    return;
                }
            }
        }
        ongoingFilterProcesses[fid_index].erase(messID);
        deliverMessage(cmd);
        if (ongoingFilterProcesses[fid_index].empty()) {
            transmitDelayedMessages(fid);
        }
    }
}

void CommonCore::processMessageFilter(ActionMessage& cmd)
{
    if (cmd.dest_id == parent_broker_id) {
        transmit(parent_route_id, cmd);
    } else if (cmd.dest_id == global_broker_id_local) {
        // deal with local source filters

        auto FiltI = filters.find(cmd.getDest());
        if (FiltI != nullptr) {
            if ((!checkActionFlag(*FiltI, disconnected_flag)) && (FiltI->filterOp)) {
                if (FiltI->cloning) {
                    auto new_messages =
                        FiltI->filterOp->processVector(createMessageFromCommand(std::move(cmd)));
                    for (auto& msg : new_messages) {
                        if (msg) {
                            cmd = ActionMessage(std::move(msg));
                            deliverMessage(cmd);
                        }
                    }
                } else {
                    bool destFilter = (cmd.action() == CMD_SEND_FOR_DEST_FILTER_AND_RETURN);
                    bool returnToSender =
                        ((cmd.action() == CMD_SEND_FOR_FILTER_AND_RETURN) || destFilter);
                    auto source = cmd.getSource();
                    auto mid = cmd.messageID;
                    auto tempMessage = createMessageFromCommand(std::move(cmd));
                    tempMessage = FiltI->filterOp->process(std::move(tempMessage));
                    if (tempMessage) {
                        cmd = ActionMessage(std::move(tempMessage));
                    } else {
                        cmd = CMD_IGNORE;
                    }

                    if (!returnToSender) {
                        if (cmd.action() == CMD_IGNORE) {
                            return;
                        }
                        cmd.setSource(source);
                        cmd.dest_id = parent_broker_id;
                        cmd.dest_handle = interface_handle();
                        deliverMessage(cmd);
                    } else {
                        cmd.setDestination(source);
                        if (cmd.action() == CMD_IGNORE) {
                            cmd.setAction(destFilter ? CMD_NULL_DEST_MESSAGE : CMD_NULL_MESSAGE);
                            cmd.messageID = mid;
                            deliverMessage(cmd);
                            return;
                        }
                        cmd.setAction(destFilter ? CMD_DEST_FILTER_RESULT : CMD_FILTER_RESULT);

                        cmd.source_handle = FiltI->handle;
                        cmd.source_id = global_broker_id_local;
                        deliverMessage(cmd);
                    }
                }
            } else // the filter didn't have a function or was deactivated but still was requested to process
            {
                bool destFilter = (cmd.action() == CMD_SEND_FOR_DEST_FILTER_AND_RETURN);
                bool returnToSender =
                    ((cmd.action() == CMD_SEND_FOR_FILTER_AND_RETURN) || destFilter);
                auto source = cmd.getSource();
                if (!returnToSender) {
                    cmd.setAction(CMD_SEND_MESSAGE);
                    cmd.dest_id = parent_broker_id;
                    cmd.dest_handle = interface_handle();
                    deliverMessage(cmd);
                } else {
                    cmd.setDestination(source);
                    cmd.setAction(destFilter ? CMD_DEST_FILTER_RESULT : CMD_FILTER_RESULT);

                    cmd.source_handle = FiltI->handle;
                    cmd.source_id = global_broker_id_local;
                    deliverMessage(cmd);
                }
            }
        } else {
            assert(false);
            // this is an odd condition (not sure what to do yet)
            /*	m.dest_id = filtFunc->sourceOperators[ii].fed_id;
                m.dest_handle = filtFunc->sourceOperators[ii].handle;
                if ((ii < static_cast<int> (filtFunc->sourceOperators.size() - 1)) ||
                    (filtFunc->finalSourceFilter.fed_id != invalid_fed_id))
                {
                    m.setAction(CMD_SEND_FOR_FILTER_OPERATION);
                }
                else
                {
                    m.setAction(CMD_SEND_FOR_FILTER);
                }
                return m;
                */
        }
    } else {
        auto route = getRoute(cmd.dest_id);
        transmit(route, cmd);
    }
}

const std::string& CommonCore::getInterfaceInfo(interface_handle handle) const
{
    auto handleInfo = getHandleInfo(handle);
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
} // namespace helics
