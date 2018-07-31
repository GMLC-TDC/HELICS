/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "CommonCore.hpp"
#include "../common/logger.h"
#include "../common/stringToCmdLine.h"
#include "../flag-definitions.h"
#include "ActionMessage.hpp"
#include "BasicHandleInfo.hpp"
#include "CoreFactory.hpp"
#include "CoreFederateInfo.hpp"
#include "EndpointInfo.hpp"
#include "FederateState.hpp"
#include "FilterFunctions.hpp"
#include "FilterInfo.hpp"
#include "ForwardingTimeCoordinator.hpp"
#include "PublicationInfo.hpp"
#include "SubscriptionInfo.hpp"
#include "core-exceptions.hpp"
#include "loggingHelper.hpp"
#include <boost/filesystem.hpp>
#include "queryHelpers.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <fstream>
#include <functional>

#include "../common/DelayedObjects.hpp"
#include "../common/fmt_format.h"
#include "../common/JsonProcessingFunctions.hpp"

namespace helics
{
using federate_id_t = Core::federate_id_t;
using handle_id_t = Core::handle_id_t;

CommonCore::CommonCore () noexcept {}

CommonCore::CommonCore (bool /*arg*/) noexcept {}

CommonCore::CommonCore (const std::string &core_name) : BrokerBase (core_name) {}

void CommonCore::initialize (const std::string &initializationString)
{
    if ((brokerState ==
         created))  // don't do the compare exchange here since we do that in the initialize fromArgs
    {  // and we can tolerate a spurious call
        StringToCmdLine cmdline (initializationString);
        initializeFromArgs (cmdline.getArgCount (), cmdline.getArgV ());
    }
}

void CommonCore::initializeFromArgs (int argc, const char *const *argv)
{
    broker_state_t exp = created;
    if (brokerState.compare_exchange_strong (exp, broker_state_t::initialized))
    {
        // initialize the brokerbase
        initializeFromCmdArgs (argc, argv);
    }
}

bool CommonCore::connect ()
{
    if (brokerState >= broker_state_t::initialized)
    {
        broker_state_t exp = broker_state_t::initialized;
        if (brokerState.compare_exchange_strong (exp, broker_state_t::connecting))
        {
            bool res = brokerConnect ();
            if (res)
            {
                // now register this core object as a broker

                ActionMessage m (CMD_REG_BROKER);
                m.name = getIdentifier ();
                m.info ().target = getAddress ();
                setActionFlag(m, core_flag);
                transmit (0, m);
                brokerState = broker_state_t::connected;
            }
            else
            {
                brokerState = broker_state_t::initialized;
            }
            return res;
        }
        while (brokerState == broker_state_t::connecting)
        {
            std::this_thread::yield ();
        }
    }
    return isConnected ();
}

bool CommonCore::isConnected () const
{
    auto getCurrentState = brokerState.load ();
    return ((getCurrentState == operating) || (getCurrentState == connected));
}

void CommonCore::processDisconnect (bool skipUnregister)
{
    if (brokerState > broker_state_t::initialized)
    {
        if (brokerState < broker_state_t::terminating)
        {
            brokerState = broker_state_t::terminating;
            if (global_broker_id != 0)
            {
                ActionMessage dis (CMD_DISCONNECT);
                dis.source_id = global_broker_id;
                transmit (0, dis);
            }
            else
            {
                ActionMessage dis (CMD_DISCONNECT_NAME);
                dis.payload = getIdentifier ();
                transmit (0, dis);
            }
            addActionMessage (CMD_STOP);
            return;
        }
        brokerDisconnect ();
    }
    brokerState = terminated;
    if (!skipUnregister)
    {
        unregister ();
    }
}

void CommonCore::disconnect () { processDisconnect (); }

void CommonCore::unregister ()
{
    /*We need to ensure that the destructor is not called immediately upon calling unregister
    otherwise this would be a mess and probably cause segmentation faults so we capture it in a local variable
    that will be destroyed on function exit
    */
    auto keepCoreAlive = CoreFactory::findCore (identifier);
    if (keepCoreAlive)
    {
        if (keepCoreAlive.get () == this)
        {
            keepCoreAlive = nullptr;
            CoreFactory::unregisterCore (identifier);
        }
    }

    if (!prevIdentifier.empty ())
    {
        auto keepCoreAlive2 = CoreFactory::findCore (prevIdentifier);
        if (keepCoreAlive2)
        {
            if (keepCoreAlive2.get () == this)
            {
                keepCoreAlive2 = nullptr;
                CoreFactory::unregisterCore (prevIdentifier);
            }
        }
    }
}
CommonCore::~CommonCore ()
{
    // make sure everything is synced up so just run the lock
    joinAllThreads ();
}

FederateState *CommonCore::getFederateAt (federate_id_t federateID) const
{
    auto feds = federates.lock ();
    return (*feds)[federateID];
}

FederateState *CommonCore::getFederate (const std::string &federateName) const
{
    auto feds = federates.lock ();
    return feds->find (federateName);
}

FederateState *CommonCore::getHandleFederate (handle_id_t id_)
{
    auto local_fed_id = handles.read ([id_](auto &hand) { return hand.getLocalFedID (id_); });
    if (local_fed_id != invalid_fed_id)
    {
        auto feds = federates.lock ();
        return (*feds)[local_fed_id];
    }

    return nullptr;
}

FederateState *CommonCore::getFederateCore (federate_id_t federateID)
{
    auto fed = loopFederates.find (federateID);
    return (fed != loopFederates.end ()) ? (*fed) : nullptr;
}

FederateState *CommonCore::getFederateCore (const std::string &federateName)
{
    auto fed = loopFederates.find (federateName);
    return (fed != loopFederates.end ()) ? (*fed) : nullptr;
}

FederateState *CommonCore::getHandleFederateCore (handle_id_t id_)
{
    // this list is now constant no need to lock
    auto local_fed_id = handles.read ([id_](auto &hand) { return hand.getLocalFedID (id_); });
    if (local_fed_id != invalid_fed_id)
    {
        return loopFederates[local_fed_id];
    }

    return nullptr;
}

BasicHandleInfo *CommonCore::getHandleInfo (handle_id_t id_) const
{
    return handles.read ([id_](auto &hand) { return hand.getHandleInfo (id_); });
}

BasicHandleInfo *CommonCore::getLocalEndpoint (const std::string &name)
{
    return handles.read ([&name](auto &hand) { return hand.getEndpoint (name); });
}

bool CommonCore::isLocal (Core::federate_id_t global_id) const
{
    return (loopFederates.find (global_id) != loopFederates.end ());
}

int32_t CommonCore::getRoute (Core::federate_id_t global_id) const
{
    auto fnd = routing_table.find (global_id);
    return (fnd != routing_table.end ()) ? fnd->second : 0;
}

bool CommonCore::isInitialized () const { return (brokerState >= initialized); }

bool CommonCore::isOpenToNewFederates () const { return ((brokerState != created) && (brokerState < operating)); }
void CommonCore::error (federate_id_t federateID, int errorCode)
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("federateID not valid error"));
    }
    ActionMessage m (CMD_ERROR);
    m.source_id = fed->global_id;
    m.counter = static_cast<int16_t> (errorCode);
    addActionMessage (m);
    fed->addAction (m);
    iteration_result ret = iteration_result::next_step;
    while (ret != iteration_result::error)
    {
        ret = fed->genericUnspecifiedQueueProcess ();
        if (ret == iteration_result::halted)
        {
            break;
        }
    }
}

void CommonCore::finalize (federate_id_t federateID)
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("federateID not valid finalize"));
    }
    ActionMessage bye (CMD_DISCONNECT);
    bye.source_id = fed->global_id;
    bye.dest_id = fed->global_id;
    addActionMessage (bye);
    iteration_result ret = iteration_result::next_step;
    while (ret != iteration_result::halted)
    {
        ret = fed->genericUnspecifiedQueueProcess ();
        if (ret == iteration_result::error)
        {
            break;
        }
    }
}

bool CommonCore::allInitReady () const
{
    if (delayInitCounter > 0)
    {
        return false;
    }
    // the federate count must be greater than the min size
    if (static_cast<decltype (minFederateCount)> (loopFederates.size ()) < minFederateCount)
    {
        return false;
    }
    // all federates must be requesting init
    return std::all_of (loopFederates.begin (), loopFederates.end (),
                        [](const auto &fed) { return fed->init_transmitted.load (); });
}

bool CommonCore::allDisconnected () const
{
    // all federates must have hit finished state
    auto pred = [](const auto &fed) {
        auto state = fed->getState ();
        return (HELICS_FINISHED == state) || (HELICS_ERROR == state);
    };
    return std::all_of (loopFederates.begin (), loopFederates.end (), pred);
}

void CommonCore::setCoreReadyToInit ()
{
    // use the flag mechanics that do the same thing
    setFlag (invalid_fed_id, ENABLE_INIT_ENTRY);
}

/** this function will generate an appropriate exception for the error
code listed in a Federate*/
static void generateFederateException (const FederateState *fed)
{
    auto eCode = fed->lastErrorCode ();
    switch (eCode)
    {
    case 0:
        return;
    case ERROR_CODE_INVALID_ARGUMENT:
        throw (InvalidParameter (fed->lastErrorString ()));
    case ERROR_CODE_INVALID_FUNCTION_CALL:
        throw (InvalidFunctionCall (fed->lastErrorString ()));
    case ERROR_CODE_INVALID_OBJECT:
        throw (InvalidIdentifier (fed->lastErrorString ()));
    case ERROR_CODE_INVALID_STATE_TRANSITION:
        throw (InvalidFunctionCall (fed->lastErrorString ()));
    case ERROR_CODE_CONNECTION_FAILURE:
        throw (ConnectionFailure (fed->lastErrorString ()));
    case ERROR_CODE_REGISTRATION_FAILURE:
        throw (RegistrationFailure (fed->lastErrorString ()));
    default:
        throw (HelicsException (fed->lastErrorString ()));
    }
}
void CommonCore::enterInitializingState (federate_id_t federateID)
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("federateID not valid for Entering Init"));
    }
    switch (fed->getState ())
    {
    case HELICS_CREATED:
        break;
    case HELICS_INITIALIZING:
        return;
    default:
        throw (InvalidFunctionCall ("May only enter initializing state from created state"));
    }

    bool exp = false;
    if (fed->init_requested.compare_exchange_strong (exp, true))
    {  // only enter this loop once per federate
        ActionMessage m (CMD_INIT);
        m.source_id = fed->global_id;
        addActionMessage (m);

        auto check = fed->enterInitializationState ();
        if (check != iteration_result::next_step)
        {
            fed->init_requested = false;
            if (check == iteration_result::halted)
            {
                throw (HelicsTerminated ());
            }
            generateFederateException (fed);
        }
        return;
    }
    throw (InvalidFunctionCall ("federate already has requested entry to initializing State"));
}

iteration_result CommonCore::enterExecutingState (federate_id_t federateID, iteration_request iterate)
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("federateID not valid (EnterExecutingState)"));
    }
    if (HELICS_EXECUTING == fed->getState ())
    {
        return iteration_result::next_step;
    }
    if (HELICS_INITIALIZING != fed->getState ())
    {
        throw (InvalidFunctionCall ("federate is in invalid state for calling entry to exec mode"));
    }
    // do an exec check on the fed to process previously received messages so it can't get in a deadlocked state
    ActionMessage exec (CMD_EXEC_CHECK);
    fed->addAction (exec);
    // TODO:: check for error conditions?
    return fed->enterExecutingState (iterate);
}

federate_id_t CommonCore::registerFederate (const std::string &name, const CoreFederateInfo &info)
{
    if (brokerState == created)
    {
        throw (RegistrationFailure ("Core has not been initialized yet"));
    }
    if (brokerState >= operating)
    {
        throw (RegistrationFailure ("Core has already moved to operating state"));
    }
    FederateState *fed;
    federate_id_t local_id;
    {
        auto feds = federates.lock ();
        auto id = feds->insert (name, name, info);
        if (id)
        {
            local_id = static_cast<decltype (fed->local_id)> (*id);
            fed = (*feds)[*id];
        }
        else
        {
            throw (
              RegistrationFailure ("duplicate names " + name + "detected multiple federates with the same name"));
        }
    }

    // setting up the Logger
    // auto ptr = fed.get();
    // if we are using the Logger, log all messages coming from the federates so they can control the level*/
    fed->setLogger ([this](int /*level*/, const std::string &ident, const std::string &message) {
        sendToLogger (0, -2, ident, message);
    });

    fed->local_id = local_id;
    fed->setParent (this);

    ActionMessage m (CMD_REG_FED);
    m.name = name;
    addActionMessage (m);
    // now wait for the federateQueue to get the response
    auto valid = fed->waitSetup ();
    if (valid == iteration_result::next_step)
    {
        return local_id;
    }
    throw (RegistrationFailure (fed->lastErrorString ()));
}

const std::string &CommonCore::getFederateName (federate_id_t federateID) const
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("federateID not valid (federateName)"));
    }
    return fed->getIdentifier ();
}

static const std::string unknownString ("#unknown");

const std::string &CommonCore::getFederateNameNoThrow (federate_id_t federateID) const noexcept
{
    auto fed = getFederateAt (federateID);
    return (fed == nullptr) ? unknownString : fed->getIdentifier ();
}

federate_id_t CommonCore::getFederateId (const std::string &name)
{
    auto feds = federates.lock ();
    auto fed = feds->find (name);
    if (fed != nullptr)
    {
        return fed->local_id;
    }

    return invalid_fed_id;
}

int32_t CommonCore::getFederationSize ()
{
    if (brokerState >= operating)
    {
        return _global_federation_size;
    }
    // if we are in initialization return the local federation size
    return static_cast<int32_t> (federates.lock ()->size ());
}

Time CommonCore::timeRequest (federate_id_t federateID, Time next)
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("federateID not valid timeRequest"));
    }
    if (HELICS_EXECUTING == fed->getState ())
    {
        auto ret = fed->requestTime (next, iteration_request::no_iterations);
        if (ret.state != iteration_result::error)
        {
            return ret.grantedTime;
        }
        throw (FunctionExecutionFailure ("federate has an error"));
    }
    throw (InvalidFunctionCall ("time request may only be called in execution state"));
}

iteration_time CommonCore::requestTimeIterative (federate_id_t federateID, Time next, iteration_request iterate)
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("federateID not valid timeRequestIterative"));
    }

    if (HELICS_EXECUTING != fed->getState ())
    {
        throw (InvalidFunctionCall ("time request may only be called in execution state"));
    }

    // limit the iterations
    if (iterate == iteration_request::iterate_if_needed)
    {
        if (fed->getCurrentIteration () >= maxIterationCount)
        {
            iterate = iteration_request::no_iterations;
        }
    }

    return fed->requestTime (next, iterate);
}

Time CommonCore::getCurrentTime (federate_id_t federateID) const
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw InvalidIdentifier ("federateID not valid (getCurrentTime)");
    }
    return fed->grantedTime ();
}

uint64_t CommonCore::getCurrentReiteration (federate_id_t federateID) const
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw InvalidIdentifier ("federateID not valid (getCurrentReiteration)");
    }
    return fed->getCurrentIteration ();
}

void CommonCore::setMaximumIterations (federate_id_t federateID, int32_t iterations)
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("federateID not valid (getMaximumIterations)"));
    }
    ActionMessage cmd (CMD_FED_CONFIGURE);
    cmd.index = UPDATE_MAX_ITERATION;
    cmd.dest_id = iterations;
    fed->updateFederateInfo (cmd);
}

void CommonCore::setTimeDelta (federate_id_t federateID, Time time)
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("federateID not valid (setTimeDelta)"));
    }
    if (time < timeZero)
    {
        throw (InvalidParameter ("timeDelta must be greater than or equal to zero"));
    }
    // time delta should not be zero but we allow it here for convenience
    if (time == timeZero)
    {
        time = timeEpsilon;
    }
    ActionMessage cmd (CMD_FED_CONFIGURE);
    cmd.index = UPDATE_MINDELTA;
    cmd.actionTime = time;
    fed->updateFederateInfo (cmd);
}

void CommonCore::setOutputDelay (federate_id_t federateID, Time outputDelayTime)
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("federateID not valid (setOutputDelay)"));
    }
    if (outputDelayTime < timeZero)
    {
        throw (InvalidParameter ("outputDelay time must be >=0"));
    }
    ActionMessage cmd (CMD_FED_CONFIGURE);
    cmd.index = UPDATE_OUTPUT_DELAY;
    cmd.actionTime = outputDelayTime;
    fed->updateFederateInfo (cmd);
}

void CommonCore::setInputDelay (federate_id_t federateID, Time inputDelayTime)
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("federateID not valid (SetinputDelay)"));
    }

    if (inputDelayTime < timeZero)
    {
        throw (InvalidParameter ("impact window must be >=0"));
    }
    ActionMessage cmd (CMD_FED_CONFIGURE);
    cmd.index = UPDATE_INPUT_DELAY;
    cmd.actionTime = inputDelayTime;
    fed->updateFederateInfo (cmd);
}

void CommonCore::setPeriod (federate_id_t federateID, Time timePeriod)
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("federateID not valid (setPeriod)"));
    }
    if (timePeriod < timeZero)
    {
        throw (InvalidParameter ("period must be greater than 0"));
    }
    ActionMessage cmd (CMD_FED_CONFIGURE);
    cmd.index = UPDATE_PERIOD;
    cmd.actionTime = timePeriod;
    fed->updateFederateInfo (cmd);
}
void CommonCore::setTimeOffset (federate_id_t federateID, Time timeOffset)
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("federateID not valid (setTimeOffset)"));
    }
    ActionMessage cmd (CMD_FED_CONFIGURE);
    cmd.index = UPDATE_OFFSET;
    cmd.actionTime = timeOffset;
    fed->updateFederateInfo (cmd);
}

void CommonCore::setLoggingLevel (federate_id_t federateID, int loggingLevel)
{
    if (federateID == invalid_fed_id)
    {
        ActionMessage cmd (CMD_CORE_CONFIGURE);
        cmd.index = UPDATE_LOG_LEVEL;
        cmd.dest_id = loggingLevel;
        addActionMessage (cmd);
        // setLogLevel (loggingLevel);
        return;
    }

    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("federateID not valid (setLoggingLevel)"));
    }
    ActionMessage cmd (CMD_FED_CONFIGURE);
    cmd.index = UPDATE_LOG_LEVEL;
    cmd.dest_id = loggingLevel;
    fed->updateFederateInfo (cmd);
}

void CommonCore::setFlag (federate_id_t federateID, int flag, bool flagValue)
{
    if (federateID == invalid_fed_id)
    {
        if (flag == DELAY_INIT_ENTRY)
        {
            if (flagValue)
            {
                ++delayInitCounter;
            }
            else
            {
                ActionMessage cmd (CMD_CORE_CONFIGURE);
                cmd.index = UPDATE_FLAG;
                cmd.dest_id = ENABLE_INIT_ENTRY;
                addActionMessage (cmd);
            }
        }
        else if (flag == ENABLE_INIT_ENTRY)
        {
            ActionMessage cmd (CMD_CORE_CONFIGURE);
            cmd.index = UPDATE_FLAG;
            cmd.dest_id = ENABLE_INIT_ENTRY;
            addActionMessage (cmd);
        }
        return;
    }

    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("federateID not valid (setFlag)"));
    }
    ActionMessage cmd (CMD_FED_CONFIGURE);
    cmd.index = UPDATE_FLAG;
    cmd.dest_id = flag;
    if (flagValue)
    {
        setActionFlag (cmd, indicator_flag);
    }
    fed->updateFederateInfo (cmd);
}
// comparison auto lambda  Functions like a template
// static auto compareFunc = [](const auto &A, const auto &B) { return (A->id < B->id); };

BasicHandleInfo *CommonCore::createBasicHandle (federate_id_t global_federateId,
                                                federate_id_t local_federateId,
                                                handle_type_t HandleType,
                                                const std::string &key,
                                                const std::string &type,
                                                const std::string &units,
                                                bool required)
{
    return handles.modify ([&](auto &hand) {
        auto hndl = hand.addHandle (global_federateId, HandleType, key, type, units);
        hndl->local_fed_id = local_federateId;
        if (required)
        {
            setActionFlag (*hndl, required_flag);
        }
        return hndl;
    });
}

BasicHandleInfo *CommonCore::createBasicHandle (federate_id_t global_federateId,
                                                federate_id_t local_federateId,
                                                handle_type_t HandleType,
                                                const std::string &key,
                                                const std::string &target,
                                                const std::string &type_in,
                                                const std::string &type_out)
{
    return handles.modify ([&](auto &hand) {
        auto hndl = hand.addHandle (global_federateId, HandleType, key, target, type_in, type_out);
        hndl->local_fed_id = local_federateId;
        return hndl;
    });
}

handle_id_t CommonCore::registerSubscription (federate_id_t federateID,
                                              const std::string &key,
                                              const std::string &type,
                                              const std::string &units,
                                              handle_check_mode check_mode)
{
    auto fed = getFederateAt (federateID);

    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("federateID not valid (registerSubscription)"));
    }
    if (fed->getState () != HELICS_CREATED)
    {
        throw (InvalidFunctionCall ("subscriptions must be registered before calling enterInitializationMode"));
    }

    auto handle = createBasicHandle (fed->global_id, fed->local_id, handle_type_t::subscription, key, type, units,
                                     (check_mode == handle_check_mode::required));

    LOG_DEBUG (0, fed->getIdentifier (), fmt::format ("registering SUB {}",key));
    auto id = handle->handle;
    fed->interfaces ().createSubscription (id, key, type, units, check_mode);

    ActionMessage m (CMD_REG_SUB);
    m.source_id = fed->global_id;
    m.source_handle = id;
    m.name = key;
    m.info ().type = type;
    m.info ().units = units;
    if (check_mode == handle_check_mode::required)
    {
        setActionFlag (m, required_flag);
    }
    addActionMessage (m);

    return id;
}

handle_id_t CommonCore::getSubscription (federate_id_t federateID, const std::string &key) const
{
    auto fed = getFederateAt (federateID);
    if (fed != nullptr)
    {
        return fed->interfaces ().getSubscription (key)->id;
    }
    return invalid_handle;
}

handle_id_t CommonCore::registerPublication (federate_id_t federateID,
                                             const std::string &key,
                                             const std::string &type,
                                             const std::string &units)
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("federateID not valid (getSubscription)"));
    }
    if (fed->getState () != HELICS_CREATED)
    {
        throw (InvalidFunctionCall ("publications must be registered before calling enterInitializationMode"));
    }
    LOG_DEBUG (0, fed->getIdentifier (), fmt::format ("registering PUB {}", key));
    auto pub = handles.read ([&key](auto &hand) { return hand.getPublication (key); });
    if (pub != nullptr)  // this key is already found
    {
        throw (RegistrationFailure ("Publication key already exists"));
    }
    auto handle =
      createBasicHandle (fed->global_id, fed->local_id, handle_type_t::publication, key, type, units, false);

    auto id = handle->handle;

    fed->interfaces ().createPublication (id, key, type, units);

    ActionMessage m (CMD_REG_PUB);
    m.source_id = fed->global_id;
    m.source_handle = id;
    m.name = key;
    m.info ().type = type;
    m.info ().units = units;

    actionQueue.push (m);
    return id;
}

handle_id_t CommonCore::getPublication (federate_id_t federateID, const std::string &key) const
{
    auto pub = handles.read ([&key](auto &hand) { return hand.getPublication (key); });
    if (pub->local_fed_id != federateID)
    {
        return invalid_handle;
        }
    return pub->handle;
}

const std::string nullStr;

const std::string &CommonCore::getHandleName (handle_id_t handle) const
{
    auto handleInfo = getHandleInfo (handle);
    if (handleInfo != nullptr)
    {
        return handleInfo->key;
    }
    return nullStr;
}

const std::string &CommonCore::getUnits (handle_id_t handle) const
{
    auto handleInfo = getHandleInfo (handle);
    if (handleInfo != nullptr)
    {
        return handleInfo->units;
    }
    return nullStr;
}

const std::string &CommonCore::getType (handle_id_t handle) const
{
    auto handleInfo = getHandleInfo (handle);
    if (handleInfo != nullptr)
    {
        if (handleInfo->handle_type == handle_type_t::subscription)
        {
            auto fed = getFederateAt (handleInfo->local_fed_id);
            auto subInfo = fed->interfaces ().getSubscription (handleInfo->handle);
            if (subInfo != nullptr)
            {
                if (!subInfo->pubType.empty ())
                {
                    return subInfo->pubType;
                }
            }
        }
        return handleInfo->type;
    }
    return nullStr;
}

const std::string &CommonCore::getOutputType (handle_id_t handle) const
{
    auto handleInfo = getHandleInfo (handle);
    if (handleInfo != nullptr)
    {
        switch (handleInfo->handle_type)
        {
        case handle_type_t::publication:
        case handle_type_t::endpoint:
            return handleInfo->type;
        case handle_type_t::destination_filter:
        case handle_type_t::source_filter:
            return handleInfo->type_out;
        default:
            return nullStr;
        }
    }
    return nullStr;
}

const std::string &CommonCore::getTarget (handle_id_t handle) const
{
    auto handleInfo = getHandleInfo (handle);
    if (handleInfo != nullptr)
    {
        switch (handleInfo->handle_type)
        {
        case handle_type_t::subscription:
        case handle_type_t::publication:
            return handleInfo->key;
        case handle_type_t::destination_filter:
        case handle_type_t::source_filter:
            return handleInfo->target;
        case handle_type_t::endpoint:
        default:
            return nullStr;
        }
    }
    return nullStr;
}
void CommonCore::setValue (handle_id_t handle, const char *data, uint64_t len)
{
    auto handleInfo = getHandleInfo (handle);
    if (handleInfo == nullptr)
    {
        throw (InvalidIdentifier ("Handle not valid (setValue)"));
    }
    if (handleInfo->handle_type != handle_type_t::publication)
    {
        throw (InvalidIdentifier ("handle does not point to a publication"));
    }

    if (!handleInfo->used)
    {
        return;  // if the value is not required do nothing
    }
    auto fed = getFederateAt (handleInfo->local_fed_id);
    if (fed->checkAndSetValue (handle, data, len))
    {
        LOG_DEBUG (0, fed->getIdentifier (),
                   fmt::format ("setting Value for {} size {}",handleInfo->key, len));
        ActionMessage mv (CMD_PUB);
        mv.source_id = handleInfo->fed_id;
        mv.source_handle = handle;
        mv.counter = static_cast<uint16_t> (fed->getCurrentIteration ());
        mv.payload = std::string (data, len);
        mv.actionTime = fed->nextAllowedSendTime ();

        actionQueue.push (std::move (mv));
    }
}

std::shared_ptr<const data_block> CommonCore::getValue (handle_id_t handle)
{
    auto handleInfo = getHandleInfo (handle);
    if (handleInfo == nullptr)
    {
        throw (InvalidIdentifier ("Handle is invalid (getValue)"));
    }
    if (handleInfo->handle_type != handle_type_t::subscription)
    {
        throw (InvalidIdentifier ("Handle does not identify a subscription"));
    }
    // todo:: this is a long chain should be refactored
    return getFederateAt (handleInfo->local_fed_id)->interfaces ().getSubscription (handle)->getData ();
}

const std::vector<handle_id_t> &CommonCore::getValueUpdates (federate_id_t federateID)
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("federateID not valid (getValueUpdates)"));
    }
    return fed->getEvents ();
}

handle_id_t
CommonCore::registerEndpoint (federate_id_t federateID, const std::string &name, const std::string &type)
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("federateID not valid (registerEndpoint)"));
    }
    if (fed->getState () != HELICS_CREATED)
    {
        throw (InvalidFunctionCall ("endpoints must be registered before calling enterInitializationMode"));
    }
    auto ept = handles.read ([&name](auto &hand) { return hand.getEndpoint (name); });
    if (ept != nullptr)
    {
        throw (RegistrationFailure ("endpoint name is already used"));
    }
    auto handle =
      createBasicHandle (fed->global_id, fed->local_id, handle_type_t::endpoint, name, type, "", false);

    auto id = handle->handle;
    fed->interfaces ().createEndpoint (id, name, type);
    fed->hasEndpoints = true;
    ActionMessage m (CMD_REG_END);
    m.source_id = fed->global_id;
    m.source_handle = id;
    m.name = name;
    m.info ().type = type;

    actionQueue.push (m);

    return id;
}

handle_id_t CommonCore::getEndpoint (federate_id_t federateID, const std::string &name) const
{
    auto ept = handles.read ([&name](auto &hand) { return hand.getEndpoint (name); });
    if (ept->local_fed_id != federateID)
    {
        return invalid_handle;
        }
    return ept->handle;
}

handle_id_t CommonCore::registerSourceFilter (const std::string &filterName,
                                              const std::string &source,
                                              const std::string &type_in,
                                              const std::string &type_out)
{
    if (brokerState == operating)
    {
        throw (InvalidFunctionCall ("Core has already entered initialization state"));
    }
    // check to make sure the name isn't already used
    if (!filterName.empty ())
    {
        auto handle = handles.read ([&filterName](auto &hand) { return hand.getFilter (filterName); });
        if (handle != nullptr)
        {
            throw (InvalidIdentifier ("there already exists a filter with this name"));
        }
    }

    auto handle =
      createBasicHandle (global_broker_id, 0, handle_type_t::source_filter, filterName, source, type_in, type_out);

    auto id = handle->handle;
    auto filtInfo = createSourceFilter (global_broker_id, id, handle->key, source, type_in, type_out, false);

    ActionMessage m (CMD_REG_SRC_FILTER);
    m.source_id = global_broker_id;
    m.source_handle = id;
    m.name = filtInfo->key;
    m.info ().target = source;
    m.info ().type = type_in;
    m.info ().type_out = type_out;
    auto ept = handles.modify ([&source](auto &hand) {
        auto epthand = hand.getEndpoint (source);
        if (epthand != nullptr)
        {
            setActionFlag (*epthand, has_source_filter_flag);
        }
        return epthand;
    });
    if (ept != nullptr)
    {
        auto endhandle = ept->handle;
        auto endid = ept->fed_id;
        setActionFlag (m, processing_complete_flag);
        // send to broker and core
        addActionMessage (m);
        // now send the same command to the endpoint
        m.dest_handle = endhandle;
        m.dest_id = endid;
        // send to
        addActionMessage (m);
    }
    else
    {
        //
        addActionMessage (m);
    }
    return id;
}

handle_id_t CommonCore::registerCloningSourceFilter (const std::string &filterName,
                                                     const std::string &source,
                                                     const std::string &type_in,
                                                     const std::string &type_out)
{
    if (brokerState == operating)
    {
        throw (InvalidFunctionCall ("Core has already entered initialization state"));
    }
    // check to make sure the name isn't already used
    if (!filterName.empty ())
    {
        auto handle = handles.read ([&filterName](auto &hand) { return hand.getFilter (filterName); });
        if (handle != nullptr)
        {
            throw (InvalidIdentifier ("there already exists a filter with this name"));
        }
    }

    auto handle =
      createBasicHandle (global_broker_id, 0, handle_type_t::source_filter, filterName, source, type_in, type_out);

    auto id = handle->handle;
    setActionFlag (*handle, clone_flag);
    auto filtInfo = createSourceFilter (global_broker_id, id, handle->key, source, type_in, type_out, true);

    ActionMessage m (CMD_REG_SRC_FILTER);
    m.source_id = global_broker_id;
    m.source_handle = id;
    m.name = filtInfo->key;
    setActionFlag (m, clone_flag);
    m.info ().target = source;
    m.info ().type = type_in;
    m.info ().type_out = type_out;

    auto ept = handles.modify ([&source](auto &hand) {
        auto epthand = hand.getEndpoint (source);
        if (epthand != nullptr)
        {
            setActionFlag (*epthand, has_source_filter_flag);
        }
        return epthand;
    });
    if (ept != nullptr)
    {
        auto endhandle = ept->handle;
        auto endid = ept->fed_id;
        // send to broker and core
        addActionMessage (m);
        // now send the same command to the endpoint
        m.dest_handle = endhandle;
        m.dest_id = endid;
        // send to
        addActionMessage (m);
    }
    else
    {
        addActionMessage (m);
    }
    return id;
}

handle_id_t CommonCore::getSourceFilter (const std::string &name) const
{
    auto filtslock = filters.lock ();
    auto filter = filtslock->find (name);
    if (filter != nullptr)
    {
        return (!filter->dest_filter) ? filter->handle : invalid_handle;
    }
    return invalid_handle;
}

handle_id_t CommonCore::registerDestinationFilter (const std::string &filterName,
                                                   const std::string &dest,
                                                   const std::string &type_in,
                                                   const std::string &type_out)
{
    if (brokerState == operating)
    {
        throw (InvalidFunctionCall ("Core has already entered initialization state"));
    }

    // check to make sure the name isn't already used
    if (!filterName.empty ())
    {
        auto handle = handles.read ([&filterName](auto &hand) { return hand.getFilter (filterName); });
        if (handle != nullptr)
        {
            throw (InvalidIdentifier ("there already exists a filter with this name"));
        }
    }

    auto handle = createBasicHandle (global_broker_id, 0, handle_type_t::destination_filter, filterName, dest,
                                     type_in, type_out);

    auto id = handle->handle;

    auto filtInfo = createDestFilter (global_broker_id, id, handle->key, dest, type_in, type_out, false);

    ActionMessage m (CMD_REG_DST_FILTER);
    m.source_id = global_broker_id;
    m.source_handle = id;
    m.name = filtInfo->key;
    m.info ().target = dest;
    m.info ().type = type_in;
    m.info ().type_out = type_out;

    auto ept = handles.modify ([&dest](auto &hand) {
        auto epthand = hand.getEndpoint (dest);
        if (epthand != nullptr)
        {
            if (checkActionFlag (*epthand, has_non_cloning_dest_filter_flag))
            {
                throw (RegistrationFailure ("endpoint " + dest + " already has a noncloning destination filter"));
            }
            setActionFlag (*epthand, has_dest_filter_flag);
            setActionFlag (*epthand, has_non_cloning_dest_filter_flag);
        }
        return epthand;
    });
    if (ept != nullptr)
    {
        auto endhandle = ept->handle;
        auto endid = ept->fed_id;
        setActionFlag (m, processing_complete_flag);
        // send to broker and core
        addActionMessage (m);
        // now send the same command to the endpoint
        m.dest_handle = endhandle;
        m.dest_id = endid;
        // send to
        addActionMessage (std::move (m));
    }
    else
    {
        addActionMessage (std::move (m));
    }
    return id;
}

handle_id_t CommonCore::registerCloningDestinationFilter (const std::string &filterName,
                                                          const std::string &dest,
                                                          const std::string &type_in,
                                                          const std::string &type_out)
{
    if (brokerState == operating)
    {
        throw (InvalidFunctionCall ("Core has already entered initialization state"));
    }

    // check to make sure the name isn't already used
    if (!filterName.empty ())
    {
        auto handle = handles.read ([&filterName](auto &hand) { return hand.getFilter (filterName); });
        if (handle != nullptr)
        {
            throw (InvalidIdentifier ("there already exists a filter with this name"));
        }
    }

    auto handle = createBasicHandle (global_broker_id, 0, handle_type_t::destination_filter, filterName, dest,
                                     type_in, type_out);

    setActionFlag (*handle, clone_flag);
    auto id = handle->handle;

    auto filtInfo = createDestFilter (global_broker_id, id, handle->key, dest, type_in, type_out, true);
    ActionMessage m (CMD_REG_DST_FILTER);
    m.source_id = global_broker_id;
    m.source_handle = id;
    m.name = filtInfo->key;
    setActionFlag (m, clone_flag);
    m.info ().target = dest;
    m.info ().type = type_in;
    m.info ().type_out = type_out;

    auto ept = handles.modify ([&dest](auto &hand) {
        auto epthand = hand.getEndpoint (dest);
        if (epthand != nullptr)
        {
            setActionFlag (*epthand, has_dest_filter_flag);
        }
        return epthand;
    });
    if (ept != nullptr)
    {
        auto endhandle = ept->handle;
        auto endid = ept->fed_id;
        setActionFlag (m, processing_complete_flag);
        // send to broker and core
        addActionMessage (m);
        // now send the same command to the endpoint
        m.dest_handle = endhandle;
        m.dest_id = endid;
        // send to
        addActionMessage (std::move (m));
    }
    else
    {
        addActionMessage (std::move (m));
    }
    return id;
}

handle_id_t CommonCore::getDestinationFilter (const std::string &name) const
{
    auto filtslock = filters.lock_shared ();
    auto filter = filtslock->find (name);
    if (filter != nullptr)
    {
        return (filter->dest_filter) ? filter->handle : invalid_handle;
    }
    return invalid_handle;
}

FilterInfo *CommonCore::createSourceFilter (federate_id_t dest,
                                            Core::handle_id_t handle,
                                            const std::string &key,
                                            const std::string &target,
                                            const std::string &type_in,
                                            const std::string &type_out,
                                            bool cloning)
{
    auto filt = std::make_unique<FilterInfo> ((dest == 0) ? global_broker_id.load () : dest, handle, key, target,
                                              type_in, type_out, false);

    auto retTarget = filt.get ();
    auto actualKey = key;
    retTarget->cloning = cloning;
    if (actualKey.empty ())
    {
        actualKey = "sFilter_";
        actualKey.append (std::to_string (handle));
    }
    auto filtlock = filters.lock ();
    if (filt->fed_id == global_broker_id)
    {
        filtlock->insert (actualKey, {filt->fed_id, filt->handle}, std::move (filt));
    }
    else
    {
        actualKey.push_back ('_');
        actualKey.append (std::to_string (filt->fed_id));
        filtlock->insert (actualKey, {filt->fed_id, filt->handle}, std::move (filt));
    }

    return retTarget;
}

FilterInfo *CommonCore::createDestFilter (federate_id_t dest,
                                          Core::handle_id_t handle,
                                          const std::string &key,
                                          const std::string &target,
                                          const std::string &type_in,
                                          const std::string &type_out,
                                            bool cloning)
{
    auto filt = std::make_unique<FilterInfo> ((dest == 0) ? global_broker_id.load () : dest, handle, key, target,
                                              type_in, type_out, true);
    auto retTarget = filt.get ();
    auto actualKey = key;
    retTarget->cloning = cloning;
    if (actualKey.empty ())
    {
        actualKey = "dFilter_";
        actualKey.append (std::to_string (handle));
    }
    auto filtlock = filters.lock ();
    if (filt->fed_id == global_broker_id)
    {
        filtlock->insert (actualKey, {filt->fed_id, filt->handle}, std::move (filt));
    }
    else
    {
        actualKey.push_back ('_');
        actualKey.append (std::to_string (filt->fed_id));
        filtlock->insert (actualKey, {filt->fed_id, filt->handle}, std::move (filt));
    }
    return retTarget;
}

void CommonCore::registerFrequentCommunicationsPair (const std::string & /*source*/, const std::string & /*dest*/)
{
    // std::lock_guard<std::mutex> lock (_mutex);
}

void CommonCore::addDependency (federate_id_t federateID, const std::string &federateName)
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("federateID not valid (registerEndpoint)"));
    }
    ActionMessage search (CMD_SEARCH_DEPENDENCY);
    search.source_id = fed->global_id;
    search.name = federateName;
    addActionMessage (std::move (search));
}

void CommonCore::send (handle_id_t sourceHandle, const std::string &destination, const char *data, uint64_t length)
{
    auto hndl = getHandleInfo (sourceHandle);
    if (hndl == nullptr)
    {
        throw (InvalidIdentifier ("handle is not valid"));
    }

    if (hndl->handle_type != handle_type_t::endpoint)
    {
        throw (InvalidIdentifier ("handle does not point to an endpoint"));
    }
    auto fed = getFederateAt (hndl->local_fed_id);
    ActionMessage m (CMD_SEND_MESSAGE);

    m.info ().orig_source = hndl->key;
    m.info ().source = hndl->key;
    m.info ().messageID = ++messageCounter;
    m.source_handle = sourceHandle;
    m.source_id = hndl->fed_id;

    m.payload = std::string (data, length);
    m.info ().target = destination;
    m.actionTime = fed->nextAllowedSendTime ();
    addActionMessage (std::move (m));
}

void CommonCore::sendEvent (Time time,
                            handle_id_t sourceHandle,
                            const std::string &destination,
                            const char *data,
                            uint64_t length)
{
    auto hndl = getHandleInfo (sourceHandle);
    if (hndl == nullptr)
    {
        throw (InvalidIdentifier ("handle is not valid"));
    }
    if (hndl->handle_type != handle_type_t::endpoint)
    {
        throw (InvalidIdentifier ("handle does not point to an endpoint"));
    }
    ActionMessage m (CMD_SEND_MESSAGE);
    m.source_handle = sourceHandle;
    m.source_id = hndl->fed_id;
    auto minTime = getFederateAt (hndl->local_fed_id)->nextAllowedSendTime ();
    m.actionTime = std::max (time, minTime);
    m.payload = std::string (data, length);
    m.info ().orig_source = hndl->key;
    m.info ().source = hndl->key;
    m.info ().target = destination;
    m.info ().messageID = ++messageCounter;
    addActionMessage (std::move (m));
}

void CommonCore::sendMessage (handle_id_t sourceHandle, std::unique_ptr<Message> message)
{
    if (sourceHandle == direct_send_handle)
    {
        ActionMessage m (std::move (message));
        m.source_id = global_broker_id;
        m.source_handle = sourceHandle;
        addActionMessage (std::move (m));
        return;
    }
    auto hndl = getHandleInfo (sourceHandle);
    if (hndl == nullptr)
    {
        throw (InvalidIdentifier ("handle is not valid"));
    }
    if (hndl->handle_type != handle_type_t::endpoint)
    {
        throw (InvalidIdentifier ("handle does not point to an endpoint"));
    }
    ActionMessage m (std::move (message));

    m.info ().source = hndl->key;
    m.source_id = hndl->fed_id;
    m.source_handle = sourceHandle;
    if (m.info ().messageID == 0)
    {
        m.info ().messageID = ++messageCounter;
    }
    auto minTime = getFederateAt (hndl->local_fed_id)->nextAllowedSendTime ();
    if (m.actionTime < minTime)
    {
        m.actionTime = minTime;
    }
    addActionMessage (std::move (m));
}

void CommonCore::deliverMessage (ActionMessage &message)
{
    switch (message.action ())
    {
    case CMD_SEND_MESSAGE:
    {
        // Find the destination endpoint
        auto localP = getLocalEndpoint (message.info ().target);
        if (localP == nullptr)
        {
            auto kfnd = knownExternalEndpoints.find (message.info ().target);
            if (kfnd != knownExternalEndpoints.end ())
            {  // destination is known
                auto route = getRoute (kfnd->second);
                transmit (route, message);
            }
            else
            {
                transmit (0, message);
            }
            return;
        }
        // now we deal with local processing
        if (checkActionFlag (*localP, has_dest_filter_flag))
        {
            auto ffunc = getFilterCoordinator (localP->handle);
            if (ffunc->destFilter != nullptr)
            {
                if (ffunc->destFilter->fed_id != global_broker_id)
                {  // now we have deal with non-local processing destination filter
                    // first block the federate time advancement until the return is received
                    ActionMessage tblock (CMD_TIME_BLOCK, global_broker_id, localP->fed_id);
                    auto mid = ++messageCounter;
                    tblock.index = mid;
                    auto fed = getFederateCore (localP->fed_id);
                    fed->addAction (tblock);
                    // now send a message to get filtered
                    message.setAction (CMD_SEND_FOR_DEST_FILTER_AND_RETURN);
                    message.info ().messageID = mid;
                    message.source_id = localP->fed_id;
                    message.source_handle = localP->handle;
                    message.dest_id = ffunc->destFilter->fed_id;
                    message.dest_handle = ffunc->destFilter->handle;
                    ongoingDestFilterProcesses[localP->fed_id].emplace (mid);
                    routeMessage (std::move (message));
                    return;
                }
                    // the filter is part of this core
                    auto tempMessage = createMessageFromCommand (std::move (message));
                    if (ffunc->destFilter->filterOp)
                    {
                        auto nmessage = ffunc->destFilter->filterOp->process (std::move (tempMessage));
                        message.moveInfo (std::move (nmessage));
                    }
                    else
                    {
                        message.moveInfo (std::move (tempMessage));
                    }
            }
            // now go to the cloning filters
            for (auto &clFilter : ffunc->cloningDestFilters)
            {
                if (clFilter->fed_id == global_broker_id)
                {
                    auto FiltI = (filters.lock ())->find (fed_handle_pair (global_broker_id, clFilter->handle));
                    if (FiltI != nullptr)
                    {
                        if (FiltI->filterOp != nullptr)
                        {
                            if (FiltI->cloning)
                            {
                                FiltI->filterOp->process (createMessageFromCommand (message));
                            }
                        }
                    }
                }
                else
                {
                    ActionMessage clone (message);
                    clone.setAction (CMD_SEND_FOR_FILTER);
                    clone.dest_id = clFilter->fed_id;
                    clone.dest_handle = clFilter->handle;
                    routeMessage (clone);
                }
            }
        }
        message.dest_id = localP->fed_id;
        message.dest_handle = localP->handle;

        timeCoord->processTimeMessage (message);

        auto fed = getFederateCore (localP->fed_id);
        fed->addAction (std::move (message));
    }
    break;
    case CMD_SEND_FOR_FILTER:
    case CMD_SEND_FOR_FILTER_AND_RETURN:
    case CMD_SEND_FOR_DEST_FILTER_AND_RETURN:
    case CMD_FILTER_RESULT:
    case CMD_DEST_FILTER_RESULT:
    case CMD_NULL_MESSAGE:
    case CMD_NULL_DEST_MESSAGE:
    default:
    {
        auto route = getRoute (message.dest_id);
        transmit (route, message);
    }
    break;
    }
}

uint64_t CommonCore::receiveCount (handle_id_t destination)
{
    auto fed = getHandleFederate (destination);
    if (fed == nullptr)
    {
        return 0;
    }
    return fed->getQueueSize (destination);
}

std::unique_ptr<Message> CommonCore::receive (handle_id_t destination)
{
    auto fed = getHandleFederate (destination);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("invalid handle"));
    }
    if (fed->getState () != HELICS_EXECUTING)
    {
        return nullptr;
    }

    return fed->receive (destination);
}

std::unique_ptr<Message> CommonCore::receiveAny (federate_id_t federateID, handle_id_t &endpoint_id)
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("FederateID is not valid (receiveAny)"));
    }
    if (fed->getState () != HELICS_EXECUTING)
    {
        endpoint_id = invalid_handle;
        return nullptr;
    }
    return fed->receiveAny (endpoint_id);
}

uint64_t CommonCore::receiveCountAny (federate_id_t federateID)
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("FederateID is not valid (receiveCountAny)"));
    }
    if (fed->getState () != HELICS_EXECUTING)
    {
        return 0;
    }

    return fed->getQueueSize ();
}

void CommonCore::logMessage (federate_id_t federateID, int logLevel, const std::string &messageToLog)
{
    if (federateID == 0)
    {
        sendToLogger (0, logLevel, getIdentifier (), messageToLog);
        return;
    }
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("FederateID is not valid (logMessage)"));
    }
    ActionMessage m (CMD_LOG);

    m.source_id = fed->global_id;
    m.index = logLevel;
    m.payload = messageToLog;
    actionQueue.push (m);
    sendToLogger (federateID, logLevel, fed->getIdentifier (), messageToLog);
}

bool CommonCore::sendToLogger (federate_id_t federateID,
                               int logLevel,
                               const std::string &name,
                               const std::string &message) const
{
    if (!BrokerBase::sendToLogger (federateID, logLevel, name, message))
    {
        auto fed = getFederateAt (federateID);
        if (fed == nullptr)
        {
            return false;
        }
        fed->logMessage (logLevel, name, message);
    }
    return true;
}

void CommonCore::setLoggingCallback (
  federate_id_t federateID,
  std::function<void(int, const std::string &, const std::string &)> logFunction)
{
    if (federateID == invalid_fed_id)
    {
        ActionMessage loggerUpdate (CMD_CORE_CONFIGURE);
        loggerUpdate.index = UPDATE_LOGGING_CALLBACK;
		loggerUpdate.source_handle = global_broker_id;
        if (logFunction)
        {
            auto ii = getNextAirlockIndex();
            dataAirlocks[ii].load(std::move(logFunction));
            loggerUpdate.counter = ii;
    }
    else
    {
            setActionFlag(loggerUpdate, empty_flag);
        }

        actionQueue.push(loggerUpdate);
    }
    else
    {
        auto fed = getFederateAt (federateID);
        if (fed == nullptr)
        {
            throw (InvalidIdentifier ("FederateID is not valid (setLoggingCallback)"));
        }
        fed->setLogger (std::move (logFunction));
    }
}

uint16_t CommonCore::getNextAirlockIndex ()
{
    uint16_t index = nextAirLock++;
    if (index > 3)
    {  // this is an atomic operation if the nextAirLock was not adjusted this could result in an out of bounds
       // exception if this check were not done
        index %= 3;
    }
    if (index == 3)
    {
        decltype (index) exp = 4;

        while (exp > 3)
        {  // doing a lock free modulus we need to make sure the nextAirLock<4
            if (nextAirLock.compare_exchange_weak (exp, exp % 4))
            {
                break;
            }
        }
    }
    return index;
}

void CommonCore::setFilterOperator (handle_id_t filter, std::shared_ptr<FilterOperator> callback)
{
    static std::shared_ptr<FilterOperator> nullFilt = std::make_shared<NullFilterOperator> ();
    auto hndl = getHandleInfo (filter);
    if (hndl == nullptr)
    {
        throw (InvalidIdentifier ("filter is not a valid handle"));
    }
    if ((hndl->handle_type != handle_type_t::destination_filter) &&
        (hndl->handle_type != handle_type_t::source_filter))
    {
        throw (InvalidIdentifier ("filter identifier does not point a filter"));
    }
    ActionMessage filtOpUpdate (CMD_CORE_CONFIGURE);
    filtOpUpdate.index = UPDATE_FILTER_OPERATOR;
    if (!callback)
    {
        callback = nullFilt;
    }
    auto ii = getNextAirlockIndex ();
    dataAirlocks[ii].load (std::move (callback));
    filtOpUpdate.counter = ii;
    filtOpUpdate.source_handle = filter;
    actionQueue.push (filtOpUpdate);
}

FilterCoordinator *CommonCore::getFilterCoordinator (handle_id_t id_)
{
    auto fnd = filterCoord.find (id_);
    if (fnd == filterCoord.end ())
    {
        if (brokerState < operating)
        {
            // just make a dummy filterFunction so we have something to return
            auto ff = std::make_unique<FilterCoordinator> ();
            auto ffp = ff.get ();
            filterCoord.emplace (id_, std::move (ff));
            return ffp;
        }
        return nullptr;
    }
    return fnd->second.get ();
}

void CommonCore::setIdentifier (const std::string &name)
{
    if (brokerState == created)
    {
        identifier = name;
    }
    else
    {
        throw (InvalidFunctionCall ("setIdentifier can only be called before the core is initialized"));
    }
}

void CommonCore::setQueryCallback (federate_id_t federateID,
                                   std::function<std::string (const std::string &)> queryFunction)
{
    auto fed = getFederateAt (federateID);
    if (fed == nullptr)
    {
        throw (InvalidIdentifier ("FederateID is invalid (setQueryCallback)"));
    }
    fed->setQueryCallback(std::move(queryFunction));
}

std::string CommonCore::federateQuery (const FederateState *fed, const std::string &queryStr) const
{
    if (fed == nullptr)
    {
        if ((queryStr == "exists") || (queryStr == "exist"))
        {
            return "false";
        }
        return "#invalid";
    }
    if ((queryStr == "exists") || (queryStr == "exist"))
    {
        return "true";
    }
    if (queryStr == "isinit")
    {
        return (fed->init_transmitted.load ()) ? "true" : "false";
    }
    if (queryStr == "state")
    {
        return std::to_string (static_cast<int> (fed->getState ()));
    }

    return fed->processQuery (queryStr);
}

std::string  CommonCore::coreQuery(const std::string &queryStr) const
{
    if (queryStr == "federates")
    {
        return generateStringVector(loopFederates, [](auto &fed) {return fed->getIdentifier(); });
    }
    if (queryStr == "publications")
    {
        return generateStringVector_if(handles.lock_shared(), [](auto &handle) {return handle->key; }, [](const auto &handle) {return (handle->handle_type == handle_type_t::publication); });
    }
    if (queryStr == "endpoints")
    {
        return generateStringVector_if(handles.lock_shared(), [](auto &handle) {return handle->key; }, [](const auto &handle) {return (handle->handle_type == handle_type_t::endpoint); });
    }
    if (queryStr == "dependson")
    {
        return generateStringVector(timeCoord->getDependencies(), [](auto &dep) {return std::to_string(dep); });
    }
    if (queryStr == "dependents")
    {
        return generateStringVector(timeCoord->getDependents(), [](auto &dep) {return std::to_string(dep); });
    }
    if (queryStr == "isinit")
    {
        return (allInitReady()) ? "true" : "false";
    }
    if (queryStr == "name")
    {
        return getIdentifier();
    }
    if (queryStr == "address")
    {
        return getAddress();
    }
    if (queryStr == "dependencies")
    {
        Json_helics::Value base;
        base["name"] = getIdentifier ();
        base["id"] = static_cast<int> (global_broker_id);
        base["parent"] = static_cast<int> (higher_broker_id);
        base["dependents"] = Json_helics::arrayValue;
        for (auto &dep : timeCoord->getDependents ())
        {
            base["dependents"].append (dep);
        }
        base["dependencies"] = Json_helics::arrayValue;
        for (auto &dep : timeCoord->getDependencies ())
        {
            base["dependencies"].append (dep);
        }
        return generateJsonString (base);
    }
    if (queryStr == "federate_map")
    {
        Json_helics::Value block;
        block["name"] = getIdentifier();
        block["id"] = static_cast<int> (global_broker_id);
        block["parent"] = static_cast<int> (higher_broker_id);
        block["federates"] = Json_helics::arrayValue;
        for (auto fed : loopFederates)
        {
            Json_helics::Value fedBlock;
            fedBlock["name"] = fed->getIdentifier();
            fedBlock["id"] = fed->global_id.load();
            fedBlock["parent"] = static_cast<int> (global_broker_id);
            block["federates"].append(fedBlock);
        }
        return generateJsonString(block);
    }
    if (queryStr == "dependency_graph")
    {
        Json_helics::Value block;
        block["name"] = getIdentifier();
        block["id"] = static_cast<int> (global_broker_id);
        block["parent"] = static_cast<int> (higher_broker_id);
        block["federates"] = Json_helics::arrayValue;
        block["dependents"] = Json_helics::arrayValue;
        for (auto &dep : timeCoord->getDependents())
        {
            block["dependents"].append(dep);
        }
        block["dependencies"] = Json_helics::arrayValue;
        for (auto &dep : timeCoord->getDependencies())
        {
            block["dependencies"].append(dep);
        }
        for (auto fed : loopFederates)
        {
            Json_helics::Value fedBlock;
            fedBlock["name"] = fed->getIdentifier();
            fedBlock["id"] = fed->global_id.load();
            fedBlock["parent"] = static_cast<int> (global_broker_id);
            fedBlock["dependencies"] = Json_helics::arrayValue;
            for (auto &dep : fed->getDependencies())
            {
                fedBlock["dependencies"].append(dep);
            }
            fedBlock["dependents"] = Json_helics::arrayValue;
            for (auto &dep : fed->getDependents())
            {
                fedBlock["dependents"].append(dep);
            }
            block["federates"].append(fedBlock);
        }
        return generateJsonString(block);
    }
    return "#invalid";
}

std::string CommonCore::query (const std::string &target, const std::string &queryStr)
{
    if ((target == "core") || (target == getIdentifier()))
    {
		if (queryStr == "name")
		{
            return getIdentifier ();
		}
		if (queryStr == "address")
		{
            return getAddress ();
		}
        ActionMessage querycmd(CMD_BROKER_QUERY);
        querycmd.source_id = global_broker_id;
        querycmd.dest_id = global_broker_id;
        auto index = ++queryCounter;
        querycmd.index = index;
        querycmd.payload = queryStr;
        auto fut = ActiveQueries.getFuture(index);
        addActionMessage(std::move(querycmd));
        auto ret = fut.get();
        ActiveQueries.finishedWithValue(index);
        return ret;
    }
    else if ((target == "parent") || (target == "broker"))
    {
        ActionMessage querycmd(CMD_BROKER_QUERY);
        querycmd.source_id = global_broker_id;
        querycmd.dest_id = higher_broker_id;
        querycmd.index = ++queryCounter;
        querycmd.payload = queryStr;
        auto fut = ActiveQueries.getFuture(querycmd.index);
        addActionMessage(querycmd);
        auto ret = fut.get();
        ActiveQueries.finishedWithValue(querycmd.index);
        return ret;
    }
    else if ((target == "root") || (target == "rootbroker"))
    {
        ActionMessage querycmd(CMD_BROKER_QUERY);
        querycmd.source_id = global_broker_id;
        querycmd.dest_id = 0;
        auto index = ++queryCounter;
        querycmd.index = index;
        querycmd.payload = queryStr;
        auto fut = ActiveQueries.getFuture(querycmd.index);
        if (global_broker_id == invalid_fed_id)
        {
            delayTransmitQueue.push(std::move(querycmd));
        }
        else
        {
            transmit(0, querycmd);
        }
        auto ret = fut.get();
        ActiveQueries.finishedWithValue(index);
        return ret;
    }
    else
    {
        auto fed = (target != "federate") ? getFederate (target) : getFederateAt (0);
        if (fed != nullptr)
        {
            return federateQuery (fed, queryStr);
        }
        ActionMessage querycmd (CMD_QUERY);
        querycmd.source_id = global_broker_id;
        auto index = ++queryCounter;
        querycmd.index = index;
        querycmd.payload = queryStr;
        querycmd.info ().target = target;
        auto fut = ActiveQueries.getFuture (querycmd.index);
        if (global_broker_id == invalid_fed_id)
        {
            delayTransmitQueue.push(std::move(querycmd));
        }
        else
        {
            transmit(0, querycmd);
        }
        
        auto ret = fut.get ();
        ActiveQueries.finishedWithValue (index);
        return ret;
    }
    return "#invalid";
}

void CommonCore::processPriorityCommand (ActionMessage &&command)
{
    // deal with a few types of message immediately
    LOG_TRACE (
      global_broker_id, getIdentifier (),
      fmt::format ("|| priority_cmd:{} from {}", prettyPrintString (command) , command.source_id));
    switch (command.action ())
    {
    case CMD_REG_FED:
    {
        // this one in the core needs to be the threadsafe version
        auto fed = getFederate (command.name);
        loopFederates.insert (command.name, nullptr, fed);
    }
        if (global_broker_id != 0)
        {
            // forward on to Broker
            command.source_id = global_broker_id;
            transmit (0, command);
        }
        else
        {
            // this will get processed when this core is assigned a global id
            delayTransmitQueue.push (std::move (command));
        }
        break;
    case CMD_REG_BROKER:
        // These really shouldn't happen here probably means something went wrong in setup but we can handle it
        // forward the connection request to the higher level
        LOG_WARNING (0, identifier, "Core received reg broker message, likely improper federation setup\n");
        transmit (0, command);
        break;
    case CMD_BROKER_ACK:
        if (command.payload == identifier)
        {
            if (checkActionFlag (command, error_flag))
            {
                LOG_ERROR (0, identifier, "broker responded with error\n");
                // TODO:generate error messages in response to all the delayed messages
                break;
            }
            global_broker_id = command.dest_id;
            timeCoord->source_id = global_broker_id;
            higher_broker_id = command.source_id;
            transmitDelayedMessages ();
        }
        break;
    case CMD_FED_ACK:
    {
        auto fed = getFederateCore (command.name);
        if (fed != nullptr)
        {
            if (checkActionFlag (command, error_flag))
            {
                LOG_ERROR (0, identifier,
                           fmt::format ("broker responded with error for registration of {}::{}\n", command.name,
                             commandErrorString (command.index)));
            }
            else
            {
                fed->global_id = command.dest_id;
                loopFederates.addSearchTerm (command.dest_id, command.name);
            }

            // push the command to the local queue
            fed->addAction (std::move(command));
        }
    }
    break;
    case CMD_REG_ROUTE:
        // TODO:: double check this
        addRoute (command.dest_handle, command.payload);
        break;
    case CMD_PRIORITY_DISCONNECT:
        if (allDisconnected ())
        {
            brokerState = broker_state_t::terminating;
            ActionMessage dis (CMD_DISCONNECT);
            dis.source_id = global_broker_id;
            transmit (0, dis);
            addActionMessage (CMD_STOP);
        }
        break;
    case CMD_BROKER_QUERY:
        if (command.dest_id == global_broker_id)
        {
            std::string repStr = coreQuery(command.payload);
            if (command.source_id == global_broker_id)
            {
                ActiveQueries.setDelayedValue(command.index, std::move(repStr));
            }
            else
            {
                ActionMessage queryResp(CMD_QUERY_REPLY);
                queryResp.dest_id = command.source_id;
                queryResp.source_id = global_broker_id;
                queryResp.index = command.index;
                queryResp.payload = std::move(repStr);
                queryResp.counter = command.counter;
                transmit(getRoute(queryResp.dest_id), queryResp);
            }
        }
        else
        {
            routeMessage(std::move(command));
        }
        break;
    case CMD_QUERY:
    {
        std::string repStr;
        ActionMessage queryResp (CMD_QUERY_REPLY);
        queryResp.dest_id = command.source_id;
        queryResp.source_id = command.dest_id;
        queryResp.index = command.index;
        queryResp.counter = command.counter;
        if (command.info ().target == getIdentifier ())
        {
            queryResp.source_id = global_broker_id;
            repStr = query (command.info ().target, command.payload);
        }
        else
        {
            auto fedptr = getFederateCore(command.info().target);
            repStr = federateQuery(fedptr, command.payload);
        }

        queryResp.payload = std::move(repStr);
        transmit (getRoute (queryResp.dest_id), queryResp);
    }
    break;
    case CMD_QUERY_REPLY:
        if (command.dest_id == global_broker_id)
        {
            ActiveQueries.setDelayedValue (command.index, command.payload);
        }
        break;
    case CMD_PRIORITY_ACK:
    case CMD_ROUTE_ACK:
        break;
    default:
    {
        if (!isPriorityCommand (command))
        {
            processCommand (std::move (command));
        }
    }

        // case CMD_DISCONNECT_ACK:
        //	break;
    }
}

void CommonCore::transmitDelayedMessages ()
{
    auto msg = delayTransmitQueue.pop ();
    while (msg)
    {
        if (msg->source_id == 0)
        {
            msg->source_id = global_broker_id;
        }
        routeMessage (*msg);
        msg = delayTransmitQueue.pop ();
    }
}

void CommonCore::sendErrorToFederates (int error_code)
{
    ActionMessage errorCom (CMD_ERROR);
    errorCom.counter = static_cast<int16_t> (error_code);
    for (auto &fed : loopFederates)
    {
        routeMessage (errorCom, fed->global_id);
    }
}

void CommonCore::transmitDelayedMessages (federate_id_t source)
{
    std::vector<ActionMessage> buffer;
    auto msg = delayTransmitQueue.pop ();
    while (msg)
    {
        if (msg->source_id == source)
        {
            routeMessage (*msg);
        }
        else
        {
            buffer.push_back (std::move (*msg));
        }
        msg = delayTransmitQueue.pop ();
    }

    if (!buffer.empty ())
    {
        for (auto &am : buffer)
        {
            delayTransmitQueue.push (std::move (am));
        }
    }

    if (!delayedTimingMessages[source].empty ())
    {
        for (auto &delayedMsg : delayedTimingMessages[source])
        {
            routeMessage (delayedMsg);
        }
        delayedTimingMessages[source].clear ();
    }
}

void CommonCore::processCommand (ActionMessage &&command)
{
    LOG_TRACE (global_broker_id, getIdentifier (),
               fmt::format ("|| cmd:{} from {}", prettyPrintString (command) ,command.source_id));
    switch (command.action ())
    {
    case CMD_IGNORE:
        break;
    case CMD_TICK:
        if (waitingForServerPingReply)
        {
            // try to reset the connection to the broker
            // brokerReconnect()
            LOG_ERROR (global_broker_id, getIdentifier (), "lost connection with server");
            sendErrorToFederates (-5);
            disconnect ();
            brokerState = broker_state_t::errored;
            addActionMessage (CMD_STOP);
        }
        else
        {
            // if (allFedWaiting())
            //{
            ActionMessage png (CMD_PING);
            png.source_id = global_broker_id;
            png.dest_id = higher_broker_id;
            transmit (0, png);
            waitingForServerPingReply = true;
            //}
        }
        break;
    case CMD_PING:
        if (command.dest_id == global_broker_id)
        {
            ActionMessage pngrep (CMD_PING_REPLY);
            pngrep.dest_id = command.source_id;
            pngrep.source_id = global_broker_id;
            routeMessage (pngrep);
        }
        break;
    case CMD_PING_REPLY:
        if (command.dest_id == global_broker_id)
        {
            waitingForServerPingReply = false;
        }
        break;
    case CMD_STOP:
        if (isConnected ())
        {
            if (!allDisconnected ())
            {  // only send a disconnect message if we haven't done so already
                ActionMessage m (CMD_DISCONNECT);
                m.source_id = global_broker_id;
                transmit (0, m);
            }
        }
        break;

    case CMD_EXEC_GRANT:
    case CMD_EXEC_REQUEST:
        if (isLocal (command.source_id))
        {
            if (!ongoingFilterProcesses[command.source_id].empty ())
            {
                delayedTimingMessages[command.source_id].push_back (command);
                break;
            }
        }
        if (command.dest_id == global_broker_id)
        {
            timeCoord->processTimeMessage (command);
            if (!enteredExecutionMode)
            {
                auto res = timeCoord->checkExecEntry ();
                if (res == message_processing_result::next_step)
                {
                    enteredExecutionMode = true;
                }
            }
        }
        else if (command.source_id == global_broker_id)
        {
            for (auto dep : timeCoord->getDependents ())
            {
                routeMessage (command, dep);
            }
        }
        else
        {
            routeMessage (command);
        }
        break;
    case CMD_TIME_REQUEST:
    case CMD_TIME_GRANT:
        if (isLocal (command.source_id))
        {
            if (!ongoingFilterProcesses[command.source_id].empty ())
            {
                delayedTimingMessages[command.source_id].push_back (command);
                break;
            }
        }
        if (command.source_id == global_broker_id)
        {
            for (auto dep : timeCoord->getDependents ())
            {
                routeMessage (command, dep);
            }
        }
        else
        {
            routeMessage (command);
        }
        break;
    case CMD_DISCONNECT:
        if (command.dest_id == 0)
        {
            if (allDisconnected ())
            {
                brokerState = broker_state_t::terminated;
                ActionMessage dis (CMD_DISCONNECT);
                dis.source_id = global_broker_id;
                transmit (0, dis);
                addActionMessage (CMD_STOP);
            }
        }
        else
        {
            routeMessage (command);
        }

        break;
    case CMD_SEARCH_DEPENDENCY:
    {
        auto fed = getFederateCore (command.name);
        if (fed != nullptr)
        {
            if (fed->global_id != invalid_fed_id)
            {
                ActionMessage dep (CMD_ADD_DEPENDENCY, fed->global_id, command.source_id);
                routeMessage (dep);
                dep = ActionMessage (CMD_ADD_DEPENDENT, command.source_id, fed->global_id);
                routeMessage (dep);
                break;
            }
        }
        // it is not found send to broker
        transmit (0, command);
    }
    break;
    case CMD_ADD_DEPENDENCY:
    case CMD_REMOVE_DEPENDENCY:
    case CMD_ADD_DEPENDENT:
    case CMD_REMOVE_DEPENDENT:
    case CMD_ADD_INTERDEPENDENCY:
    case CMD_REMOVE_INTERDEPENDENCY:
        routeMessage (command);
        break;
    case CMD_SEND_FOR_FILTER:
    case CMD_SEND_FOR_FILTER_AND_RETURN:
    case CMD_SEND_FOR_DEST_FILTER_AND_RETURN:
        processMessageFilter (command);
        break;
    case CMD_NULL_MESSAGE:
    case CMD_FILTER_RESULT:
        processFilterReturn (command);
        break;
    case CMD_DEST_FILTER_RESULT:
    case CMD_NULL_DEST_MESSAGE:
        processDestFilterReturn (command);
        break;
    case CMD_PUB:
        // route the message to all the subscribers
        if (command.dest_id == 0)
        {
            auto fed = getFederateCore (command.source_id);
            if (fed != nullptr)
            {
                auto pubInfo = fed->interfaces ().getPublication (command.source_handle);
                if (pubInfo != nullptr)
                {
                    for (auto &subscriber : pubInfo->subscribers)
                    {
                        command.dest_id = subscriber.first;
                        command.dest_handle = subscriber.second;
                        routeMessage (command);
                    }
                }
            }
        }
        else
        {
            routeMessage (command);
        }
        break;

    case CMD_LOG:
        if (command.dest_id == global_broker_id)
        {
            sendToLogger (0, command.index, getFederateNameNoThrow (command.source_id), command.payload);
        }
        else
        {
            routeMessage (command);
        }
        break;
    case CMD_ERROR:
        if (command.dest_id == global_broker_id)
        {
            sendToLogger (0, 0, getFederateNameNoThrow (command.source_id), command.payload);
        }
        else
        {
            routeMessage (command);
        }
        break;
    case CMD_REG_SUB:
        // for these registration filters any processing is already done in the
        // registration functions so this is just a router
        if (command.dest_id != 0)
        {
            auto fed = getFederateCore (command.dest_id);
            if (fed != nullptr)
            {
                fed->addAction (command);
                auto pubhandle = getHandleInfo (command.dest_handle);
                if (pubhandle != nullptr)
                {
                    pubhandle->used = true;
                }
            }
        }
        else
        {
            if (checkForLocalPublication (command))
            {
                setActionFlag (command, processing_complete_flag);
            }
            transmit (0, command);
        }

        break;
    case CMD_REG_END:
        if (command.dest_id == global_broker_id)
        {  // in this branch the message came from somewhere else and is targeted at a filter
            auto filtI = filters.lock_shared ()->find (fed_handle_pair (global_broker_id, command.dest_handle));
            if (filtI != nullptr)
            {
                filtI->target = {command.source_id, command.source_handle};
                timeCoord->addDependency (command.source_id);
            }
            auto filthandle = getHandleInfo (command.dest_handle);
            if (filthandle != nullptr)
            {
                if ((filthandle->handle_type == handle_type_t::destination_filter) ||
                    (filthandle->handle_type == handle_type_t::source_filter))
                {
                    filthandle->used = true;
                }
            }
        }
        else
        {
            transmit (0, command);

            bool added = timeCoord->addDependency (command.source_id);
            if (added)
            {
                auto fed = getFederateCore (command.source_id);
                ActionMessage add (CMD_ADD_INTERDEPENDENCY, global_broker_id, command.source_id);

                fed->addAction (add);
                timeCoord->addDependent (fed->global_id);
            }

            if (!hasTimeDependency)
            {
                if (timeCoord->addDependency (higher_broker_id))
                {
                    hasTimeDependency = true;
                    ActionMessage add (CMD_ADD_INTERDEPENDENCY, global_broker_id, higher_broker_id);
                    transmit (higher_broker_id, add);

                    timeCoord->addDependent (higher_broker_id);
                }
            }
        }

        break;
    case CMD_REG_PUB:
        // for these registration filters any processing is already done in the
        // registration functions so this is just a router
        routeMessage (command);
        break;
    case CMD_REG_DST_FILTER:
    case CMD_REG_SRC_FILTER:
        // for these registration filters any processing is already done in the
        // registration functions so this is just a router and add the time dependency
        if (command.dest_id == 0)
        {
            if (!hasFilters)
            {
                hasFilters = true;
                if (timeCoord->addDependent (higher_broker_id))
                {
                    ActionMessage add (CMD_ADD_DEPENDENCY, global_broker_id, higher_broker_id);
                    transmit (higher_broker_id, add);
                }
            }
        }
        routeMessage (command);
        if (command.dest_id != 0)
        {
            processFilterInfo (command);
        }
        break;

    case CMD_NOTIFY_SUB:
    {
        // just forward these to the appropriate federate
        auto fed = getFederateCore (command.dest_id);
        if (fed != nullptr)
        {
            fed->addAction (command);
            auto pubhandle = getHandleInfo (command.dest_handle);
            if (pubhandle != nullptr)
            {
                pubhandle->used = true;
            }
        }
    }
    break;
    case CMD_NOTIFY_END:
    {
        if (command.dest_id == global_broker_id)
        {
            helics::FilterInfo *filtI = nullptr;
            {  // scope for the lock_guard
                filtI = filters.lock_shared ()->find (fed_handle_pair (global_broker_id, command.dest_handle));
                if (filtI != nullptr)
                {
                    filtI->target = {command.source_id, command.source_handle};
                    timeCoord->addDependency (command.source_id);
                }
            }

            auto filthandle = getHandleInfo (command.dest_handle);
            if (filthandle != nullptr)
            {
                if ((filthandle->handle_type == handle_type_t::destination_filter) ||
                    (filthandle->handle_type == handle_type_t::source_filter))
                {
                    filthandle->used = true;
                }
            }
        }
    }
    break;
    case CMD_NOTIFY_PUB:
        routeMessage (command);
        break;
    case CMD_NOTIFY_SRC_FILTER:
    {
        auto endhandle = getHandleInfo (command.dest_handle);
        if (endhandle != nullptr)
        {
            setActionFlag (*endhandle, has_source_filter_flag);
        }

        processFilterInfo (command);
        if (command.source_id != global_broker_id)
        {
            auto fed = getFederateCore (command.dest_id);
            if (fed != nullptr)
            {
                command.setAction (CMD_ADD_DEPENDENT);
                fed->addAction (command);
            }
        }
    }
    break;
    case CMD_NOTIFY_DST_FILTER:
    {
        processFilterInfo (command);
        if (command.source_id != global_broker_id)
        {
            auto fed = getFederateCore (command.dest_id);
            if (fed != nullptr)
            {
                command.setAction (CMD_ADD_DEPENDENT);
                fed->addAction (command);
            }
        }
    }
    break;
    case CMD_CORE_CONFIGURE:
        processCoreConfigureCommands (command);
        break;
    case CMD_INIT:
    {
        auto fed = getFederateCore (command.source_id);
        if (fed != nullptr)
        {
            fed->init_transmitted = true;
            if (allInitReady ())
            {
                broker_state_t exp = connected;
                if (brokerState.compare_exchange_strong (exp, broker_state_t::initializing))
                {  // make sure we only do this once
                    checkDependencies ();
                    command.source_id = global_broker_id;
                    transmit (0, command);
                }
            }
        }
    }
    break;
    case CMD_INIT_GRANT:
    {
        broker_state_t exp = initializing;
        if (brokerState.compare_exchange_strong (exp, broker_state_t::operating))
        {  // forward the grant to all federates
            organizeFilterOperations ();
            for (auto &fed : loopFederates)
            {
                fed->addAction (command);
            }
            timeCoord->enteringExecMode ();
            auto res = timeCoord->checkExecEntry ();
            if (res == message_processing_result::next_step)
            {
                enteredExecutionMode = true;
            }
        }
    }
    break;

    case CMD_SEND_MESSAGE:
        if ((command.dest_id == 0) && (isLocal (command.source_id)))
        {
            deliverMessage (processMessage (command));
        }
        else
        {
            deliverMessage (command);
        }

        break;
    default:
        if (isPriorityCommand (command))
        {  // this is a backup if somehow one of these message got here
            processPriorityCommand (std::move (command));
        }
        break;
    }
}

void CommonCore::processFilterInfo (ActionMessage &command)
{
    auto filterInfo = getFilterCoordinator (command.dest_handle);
    if (filterInfo == nullptr)
    {
        return;
    }
    switch (command.action ())
    {
    case CMD_REG_DST_FILTER:
    case CMD_NOTIFY_DST_FILTER:
    {
        bool FilterAlreadyPresent = false;
        if (filterInfo->destFilter != nullptr)
        {
            if ((filterInfo->destFilter->fed_id == command.source_id) &&
                (filterInfo->destFilter->handle == command.source_handle))
            {
                FilterAlreadyPresent = true;
                break;
            }
        }
        if (!FilterAlreadyPresent)
        {
            for (auto &filt : filterInfo->cloningDestFilters)
            {
                if ((filt->fed_id == command.source_id) && (filt->handle == command.source_handle))
                {
                    FilterAlreadyPresent = true;
                    break;
                }
            }
        }

        if (!FilterAlreadyPresent)
        {
            auto endhandle = getHandleInfo (command.dest_handle);
            if (endhandle != nullptr)
            {
                setActionFlag (*endhandle, has_dest_filter_flag);
                if ((!checkActionFlag (command, clone_flag)) && (filterInfo->hasDestFilters))
                {
                    // duplicate non cloning destination filters are not allowed
                    ActionMessage err (CMD_ERROR);
                    err.dest_id = command.source_id;
                    err.source_id = command.dest_id;
                    err.source_handle = command.dest_handle;
                    err.counter = ERROR_CODE_REGISTRATION_FAILURE;
                    err.payload = "Endpoint " + endhandle->key + " already has a destination filter";
                    routeMessage (std::move (err));
                    return;
                }
            }
            auto filter =
              filters.lock_shared ()->find (fed_handle_pair (command.source_id, command.source_handle));
            if (filter == nullptr)
            {
                filter = createDestFilter (command.source_id, command.source_handle, command.payload,
                                           command.info ().target, command.info ().type, command.info ().type_out,
                                           checkActionFlag (command, clone_flag));
            }

            filterInfo->hasDestFilters = true;
            if (checkActionFlag (command, clone_flag))
            {
                filterInfo->cloningDestFilters.push_back (filter);
            }
            else
            {
                filterInfo->destFilter = filter;
            }
        }

        break;
    }
    case CMD_REG_SRC_FILTER:
    case CMD_NOTIFY_SRC_FILTER:
    {
        bool FilterAlreadyPresent = false;
        for (auto &filt : filterInfo->allSourceFilters)
        {
            if ((filt->fed_id == command.source_id) && (filt->handle == command.source_handle))
            {
                FilterAlreadyPresent = true;
                break;
            }
        }
        if (!FilterAlreadyPresent)
        {
            auto newFilter =
              filters.lock_shared ()->find (fed_handle_pair (command.source_id, command.source_handle));
            if (newFilter == nullptr)
            {
                newFilter = createSourceFilter (command.source_id, command.source_handle, command.name,
                                                command.info ().target, command.info ().type,
                                                command.info ().type_out, checkActionFlag (command, clone_flag));
                }
            filterInfo->allSourceFilters.push_back (newFilter);
            filterInfo->hasSourceFilters = true;
        }
    }
    break;
    default:
        // all other commands do not impact filters
        break;
    }
}

void CommonCore::checkDependencies ()
{
    bool isobs = false;
    bool issource = false;
    for (auto &fed : loopFederates)
    {
        if (fed->hasEndpoints)
        {
            auto fedInfo = fed->getInfo ();
            if (fedInfo.observer)
            {
                timeCoord->removeDependency (fed->global_id);
                ActionMessage rmdep (CMD_REMOVE_DEPENDENT);

                rmdep.source_id = global_broker_id;
                rmdep.dest_id = fed->global_id;
                fed->addAction (rmdep);
                isobs = true;
            }
            else if (fed->getInfo ().source_only)
            {
                timeCoord->removeDependent (fed->global_id);
                ActionMessage rmdep (CMD_REMOVE_DEPENDENCY);

                rmdep.source_id = global_broker_id;
                rmdep.dest_id = fed->global_id;
                fed->addAction (rmdep);
                issource = true;
            }
        }
    }

    // if we have filters we need to be a timeCoordinator
    if (hasFilters)
    {
        return;
    }
    // if there is more than 2 dependents(higher broker + 2 or more federates then we need to be a timeCoordinator
    if (timeCoord->getDependents ().size () > 2)
    {
        return;
    }
    if (timeCoord->getDependencies ().size () > 2)
    {
        return;
    }
    auto fedid = invalid_fed_id;
    auto brkid = invalid_fed_id;
    int localcnt = 0;
    for (auto &dep : timeCoord->getDependents ())
    {
        if (isLocal (dep))
        {
            ++localcnt;
            fedid = dep;
        }
        else
        {
            brkid = dep;
        }
    }
    if (localcnt > 1)
    {
        return;
    }
    // check to make sure the dependencies match
    for (auto &dep : timeCoord->getDependencies ())
    {
        if (isLocal (dep))
        {
            if (dep != fedid)
            {
                return;
            }
        }
        else
        {
            if (brkid != dep)
            {
                return;
            }
        }
    }
    // remove the core from the time dependency chain since it is just adding to the communication noise in this
    // case
    timeCoord->removeDependency (brkid);
    timeCoord->removeDependency (fedid);
    timeCoord->removeDependent (brkid);
    timeCoord->removeDependent (fedid);

    ActionMessage rmdep (CMD_REMOVE_INTERDEPENDENCY);

    rmdep.source_id = global_broker_id;
    routeMessage (rmdep, brkid);
    routeMessage (rmdep, fedid);
    if (isobs)
    {
        ActionMessage adddep (CMD_ADD_DEPENDENT);
        adddep.source_id = fedid;
        routeMessage (adddep, brkid);
        adddep.setAction (CMD_ADD_DEPENDENCY);
        adddep.source_id = brkid;
        routeMessage (adddep, fedid);
    }
    else if (issource)
    {
        ActionMessage adddep (CMD_ADD_DEPENDENCY);
        adddep.source_id = fedid;
        routeMessage (adddep, brkid);
        adddep.setAction (CMD_ADD_DEPENDENT);
        adddep.source_id = brkid;
        routeMessage (adddep, fedid);
    }
    else
    {
        ActionMessage adddep (CMD_ADD_INTERDEPENDENCY);
        adddep.source_id = fedid;
        routeMessage (adddep, brkid);
        routeMessage (adddep,
                      fedid);  // make sure the fed depends on itself in case the broker removes itself later
        adddep.source_id = brkid;
        routeMessage (adddep, fedid);
    }
}

void CommonCore::organizeFilterOperations ()
{
    for (auto &fc : filterCoord)
    {
        auto *fi = fc.second.get ();
        auto *handle = getHandleInfo (fc.first);
        if (handle == nullptr)
        {
            continue;
        }
        std::string endpointType = handle->type;

        if (!fi->allSourceFilters.empty ())
        {
            fi->sourceFilters.clear ();
            fi->sourceFilters.reserve (fi->allSourceFilters.size ());
            // Now we have to do some intelligent ordering with types
            std::vector<bool> used (fi->allSourceFilters.size (), false);
            bool someUnused = true;
            bool usedMore = true;
            bool firstPass = true;
            std::string currentType = endpointType;
            while (someUnused && usedMore)
            {
                someUnused = false;
                usedMore = false;
                for (size_t ii = 0; ii < fi->allSourceFilters.size (); ++ii)
                {
                    if (used[ii])
                    {
                        continue;
                    }
                    if (firstPass)
                    {
                        if (fi->allSourceFilters[ii]->cloning)
                        {
                            fi->sourceFilters.push_back (fi->allSourceFilters[ii]);
                            used[ii] = true;
                            usedMore = true;
                        }
                        else
                        {
                            someUnused = true;
                        }
                    }
                    else
                    {
                        // TODO:: this will need some work to finish sorting out but should work for initial tests
                        if (matchingTypes (fi->allSourceFilters[ii]->inputType, currentType))
                        {
                            used[ii] = true;
                            usedMore = true;
                            fi->sourceFilters.push_back (fi->allSourceFilters[ii]);
                            currentType = fi->allSourceFilters[ii]->outputType;
                        }
                        else
                        {
                            someUnused = true;
                        }
                    }
                }
                if (firstPass)
                {
                    firstPass = false;
                    usedMore = true;
                }
            }
            for (size_t ii = 0; ii < fi->allSourceFilters.size (); ++ii)
            {
                if (used[ii])
                {
                    continue;
                }
                LOG_WARNING (global_broker_id, fi->allSourceFilters[ii]->key,
                             "unable to match types on some filters");
            }
        }
    }
}
void CommonCore::processCoreConfigureCommands (ActionMessage &cmd)
{
    switch (cmd.index)
    {
    case UPDATE_FLAG:
        if (cmd.dest_id == ENABLE_INIT_ENTRY)
        {
            if (delayInitCounter <= 1)
            {
                delayInitCounter = 0;
                if (allInitReady ())
                {
                    broker_state_t exp = connected;
                    if (brokerState.compare_exchange_strong (exp, broker_state_t::initializing))
                    {  // make sure we only do this once
                        checkDependencies ();
                        cmd.source_id = global_broker_id;
                        transmit (0, cmd);
                    }
                }
            }
            else
            {
                --delayInitCounter;
            }
        }
        break;
    case UPDATE_LOG_LEVEL:
        setLogLevel (cmd.dest_id);
        break;
    case UPDATE_LOGGING_CALLBACK:
        if (checkActionFlag(cmd, empty_flag))
        {
            setLoggerFunction(nullptr);
        }
        else
        {
            auto op = dataAirlocks[cmd.counter].try_unload();
            if (op)
            {
                auto M = stx::any_cast<std::function<void(int,const std::string &, const std::string &)>> (std::move(*op));
                setLoggerFunction(std::move(M));
            }
        }
        break;
    case UPDATE_FILTER_OPERATOR:
    {
        auto filtHandle = filters.lock ();
        auto FiltI = filtHandle->find (fed_handle_pair{global_broker_id.load (), cmd.source_handle});
        int ii = cmd.counter;
        auto op = dataAirlocks[ii].try_unload ();
        if (op)
        {
            auto M = stx::any_cast<std::shared_ptr<FilterOperator>> (std::move (*op));
            FiltI->filterOp = std::move (M);
        }
    }
    break;
    }
}

void CommonCore::processCommandsForCore (const ActionMessage &cmd)
{
    if (isTimingCommand (cmd))
    {
        if (!enteredExecutionMode)
        {
            timeCoord->processTimeMessage (cmd);
            auto res = timeCoord->checkExecEntry ();
            if (res == message_processing_result::next_step)
            {
                enteredExecutionMode = true;
            }
        }
        else
        {
            if (timeCoord->processTimeMessage (cmd))
            {
                timeCoord->updateTimeFactors ();
            }
        }
        if (cmd.action () == CMD_DISCONNECT)
        {
            if (allDisconnected ())
            {
                brokerState = broker_state_t::terminated;
                ActionMessage dis (CMD_DISCONNECT);
                dis.source_id = global_broker_id;
                transmit (0, dis);
                addActionMessage (CMD_STOP);
            }
        }
    }
    else if (isDependencyCommand (cmd))
    {
        timeCoord->processDependencyUpdateMessage (cmd);
    }
    else
    {
        LOG_WARNING (global_broker_id, "core", "dropping message:" + prettyPrintString (cmd));
    }
}

bool CommonCore::checkForLocalPublication (ActionMessage &cmd)
{
    auto handlelock = handles.lock_shared ();
    auto pub = handlelock->getPublication (cmd.payload);
    handlelock.unlock ();
    if (pub != nullptr)
    {
        // now send the same command to the publication
        cmd.dest_handle = pub->handle;
        cmd.dest_id = pub->fed_id;
        pub->used = true;
        // send to
        routeMessage (cmd);
        // now send the notification to the subscription
        ActionMessage notice (CMD_NOTIFY_PUB);
        notice.dest_id = cmd.source_id;
        notice.dest_handle = cmd.source_handle;
        notice.source_id = pub->fed_id;
        notice.source_handle = pub->handle;
        notice.payload = pub->type;
        routeMessage (notice);
        return true;
    }
    return false;
}

void CommonCore::routeMessage (ActionMessage &cmd, federate_id_t dest)
{
    cmd.dest_id = dest;
    if ((dest == 0) || (dest == higher_broker_id))
    {
        transmit (0, cmd);
    }
    else if (dest == global_broker_id)
    {
        processCommandsForCore (cmd);
    }
    else if (isLocal (dest))
    {
        auto fed = getFederateCore (dest);
        if (fed != nullptr)
        {
            fed->addAction (cmd);
        }
    }
    else
    {
        auto route = getRoute (dest);
        transmit (route, cmd);
    }
}

void CommonCore::routeMessage (const ActionMessage &cmd)
{
    if ((cmd.dest_id == 0) || (cmd.dest_id == higher_broker_id))
    {
        transmit (0, cmd);
    }
    else if (cmd.dest_id == global_broker_id)
    {
        processCommandsForCore (cmd);
    }
    else if (isLocal (cmd.dest_id))
    {
        auto fed = getFederateCore (cmd.dest_id);
        if (fed != nullptr)
        {
            fed->addAction (cmd);
        }
    }
    else
    {
        auto route = getRoute (cmd.dest_id);
        transmit (route, cmd);
    }
}

void CommonCore::routeMessage (ActionMessage &&cmd, federate_id_t dest)
{
    cmd.dest_id = dest;
    if ((dest == 0) || (dest == higher_broker_id))
    {
        transmit (0, cmd);
    }
    else if (cmd.dest_id == global_broker_id)
    {
        processCommandsForCore (cmd);
    }
    else if (isLocal (dest))
    {
        auto fed = getFederateCore (dest);
        if (fed != nullptr)
        {
            fed->addAction (std::move (cmd));
        }
    }
    else
    {
        auto route = getRoute (dest);
        transmit (route, cmd);
    }
}

void CommonCore::routeMessage (ActionMessage &&cmd)
{
    if ((cmd.dest_id == 0) || (cmd.dest_id == higher_broker_id))
    {
        transmit (0, cmd);
    }
    else if (cmd.dest_id == global_broker_id)
    {
        processCommandsForCore (cmd);
    }
    else if (isLocal (cmd.dest_id))
    {
        auto fed = getFederateCore (cmd.dest_id);
        if (fed != nullptr)
        {
            fed->addAction (std::move (cmd));
        }
    }
    else
    {
        auto route = getRoute (cmd.dest_id);
        transmit (route, cmd);
    }
}

// Checks for filter operations
ActionMessage &CommonCore::processMessage (ActionMessage &m)
{
    auto handle = getHandleInfo (m.source_handle);
    if (handle == nullptr)
    {
        return m;
    }
    if (checkActionFlag (*handle, has_source_filter_flag))
    {
        auto filtFunc = getFilterCoordinator (handle->handle);
        if (filtFunc->hasSourceFilters)
        {
            //   for (int ii = 0; ii < static_cast<int> (filtFunc->sourceFilters.size ()); ++ii)
            size_t ii = 0;
            for (auto &filt : filtFunc->sourceFilters)
            {
                if (filt->fed_id == global_broker_id)
                {
                    if (filt->cloning)
                    {
                        filt->filterOp->process (createMessageFromCommand (m));
                    }
                    else
                    {
                        // deal with local source filters
                        auto tempMessage = createMessageFromCommand (std::move (m));
                        tempMessage = filt->filterOp->process (std::move (tempMessage));
                        if (tempMessage)
                        {
                            m = ActionMessage (std::move (tempMessage));
                        }
                        else
                        {
                            // the filter dropped the message;
                            m = CMD_IGNORE;
                            return m;
                        }
                    }
                }
                else if (filt->cloning)
                {
                    ActionMessage cloneMessage (m);
                    cloneMessage.setAction (CMD_SEND_FOR_FILTER);
                    setActionFlag (m, clone_flag);
                    cloneMessage.dest_id = filt->fed_id;
                    cloneMessage.dest_handle = filt->handle;
                    routeMessage (cloneMessage);
                }
                else
                {
                    m.dest_id = filt->fed_id;
                    m.dest_handle = filt->handle;
                    m.counter = static_cast<uint16_t> (ii);
                    if (ii < filtFunc->sourceFilters.size () - 1)
                    {
                        m.setAction (CMD_SEND_FOR_FILTER_AND_RETURN);
                        ongoingFilterProcesses[handle->fed_id].insert (m.info ().messageID);
                    }
                    else
                    {
                        m.setAction (CMD_SEND_FOR_FILTER);
                    }
                    return m;
                }
                ++ii;
            }
        }
    }

    return m;
}

void CommonCore::processDestFilterReturn (ActionMessage &command)
{
    auto handle = getHandleInfo (command.dest_handle);
    if (handle == nullptr)
    {
        return;
    }
    auto messID = (command.action () == CMD_DEST_FILTER_RESULT) ? command.info ().messageID : command.source_handle;
    if (ongoingDestFilterProcesses[handle->fed_id].find (messID) !=
        ongoingDestFilterProcesses[handle->fed_id].end ())
    {
        ongoingDestFilterProcesses[handle->fed_id].erase (messID);
        if (command.action () == CMD_NULL_DEST_MESSAGE)
        {
            ActionMessage removeTimeBlock (CMD_TIME_UNBLOCK, global_broker_id, command.dest_id);
            removeTimeBlock.index = messID;
            routeMessage (removeTimeBlock);
            return;
        }
        auto filtFunc = getFilterCoordinator (handle->handle);

        // now go to the cloning filters
        for (auto &clFilter : filtFunc->cloningDestFilters)
        {
            if (clFilter->fed_id == global_broker_id)
            {
                auto FiltI = filters.lock_shared ()->find (fed_handle_pair (global_broker_id, clFilter->handle));
                if (FiltI != nullptr)
                {
                    if (FiltI->filterOp != nullptr)
                    {
                        if (FiltI->cloning)
                        {
                            FiltI->filterOp->process (createMessageFromCommand (command));
                        }
                    }
                }
            }
            else
            {
                ActionMessage clone (command);
                clone.setAction (CMD_SEND_FOR_FILTER);
                clone.dest_id = clFilter->fed_id;
                clone.dest_handle = clFilter->handle;
                routeMessage (clone);
            }
        }

        timeCoord->processTimeMessage (command);
        command.setAction (CMD_SEND_MESSAGE);
        routeMessage (std::move (command));
        // now unblock the time
        ActionMessage removeTimeBlock (CMD_TIME_UNBLOCK, global_broker_id, handle->fed_id);
        removeTimeBlock.index = messID;
        routeMessage (removeTimeBlock);
    }
}

void CommonCore::processFilterReturn (ActionMessage &cmd)
{
    auto handle = getHandleInfo (cmd.dest_handle);
    if (handle == nullptr)
    {
        return;
    }

    auto messID = (cmd.action () == CMD_FILTER_RESULT) ? cmd.info ().messageID : cmd.index;

    if (ongoingFilterProcesses[handle->fed_id].find (messID) != ongoingFilterProcesses[handle->fed_id].end ())
    {
        if (cmd.action () == CMD_NULL_MESSAGE)
        {
            ongoingFilterProcesses[handle->fed_id].erase (messID);
            if (ongoingFilterProcesses[handle->fed_id].empty ())
            {
                transmitDelayedMessages (handle->fed_id);
            }
        }
        auto filtFunc = getFilterCoordinator (handle->handle);
        if (filtFunc->hasSourceFilters)
        {
            for (decltype (cmd.counter) ii = cmd.counter + 1; ii < filtFunc->sourceFilters.size (); ++ii)
            {  // cloning filters come first so we don't need to check for them in this code branch
                auto filt = filtFunc->sourceFilters[ii];
                if (filt->fed_id == global_broker_id)
                {
                    // deal with local source filters
                    auto tempMessage = createMessageFromCommand (std::move (cmd));
                    tempMessage = filt->filterOp->process (std::move (tempMessage));
                    if (tempMessage)
                    {
                        cmd = ActionMessage (std::move (tempMessage));
                    }
                    else
                    {
                        ongoingFilterProcesses[handle->fed_id].erase (messID);
                        if (ongoingFilterProcesses[handle->fed_id].empty ())
                        {
                            transmitDelayedMessages (handle->fed_id);
                        }
                        return;
                    }
                }
                else
                {
                    cmd.dest_id = filt->fed_id;
                    cmd.dest_handle = filt->handle;
                    cmd.counter = static_cast<uint16_t> (ii);
                    if (ii < filtFunc->sourceFilters.size () - 1)
                    {
                        cmd.setAction (CMD_SEND_FOR_FILTER_AND_RETURN);
                    }
                    else
                    {
                        cmd.setAction (CMD_SEND_FOR_FILTER);
                        ongoingFilterProcesses[handle->fed_id].erase (messID);
                    }
                    routeMessage (cmd);
                    if (ongoingFilterProcesses[handle->fed_id].empty ())
                    {
                        transmitDelayedMessages (handle->fed_id);
                    }
                    return;
                }
            }
        }
        ongoingFilterProcesses[handle->fed_id].erase (messID);
        deliverMessage (cmd);
        if (ongoingFilterProcesses[handle->fed_id].empty ())
        {
            transmitDelayedMessages (handle->fed_id);
        }
    }
}

void CommonCore::processMessageFilter (ActionMessage &cmd)
{
    if (cmd.dest_id == 0)
    {
        transmit (0, cmd);
    }
    else if (cmd.dest_id == global_broker_id)
    {
        // deal with local source filters

        auto FiltI = filters.lock_shared ()->find (fed_handle_pair (global_broker_id, cmd.dest_handle));
        if (FiltI != nullptr)
        {
            if (FiltI->filterOp != nullptr)
            {
                if (FiltI->cloning)
                {
                    FiltI->filterOp->process (createMessageFromCommand (std::move (cmd)));
                }
                else
                {
                    bool destFilter = (cmd.action () == CMD_SEND_FOR_DEST_FILTER_AND_RETURN);
                    bool returnToSender = ((cmd.action () == CMD_SEND_FOR_FILTER_AND_RETURN) || destFilter);
                    auto source = cmd.source_id;
                    auto source_handle = cmd.source_handle;
                    auto mid = cmd.info ().messageID;
                    auto tempMessage = createMessageFromCommand (std::move (cmd));
                    tempMessage = FiltI->filterOp->process (std::move (tempMessage));
                    if (tempMessage)
                    {
                        cmd = ActionMessage (std::move (tempMessage));
                    }
                    else
                    {
                        cmd = CMD_IGNORE;
                    }

                    if (!returnToSender)
                    {
                        if (cmd.action () == CMD_IGNORE)
                        {
                            return;
                        }
                        cmd.source_id = source;
                        cmd.source_handle = source_handle;
                        cmd.dest_id = 0;
                        cmd.dest_handle = 0;
                        deliverMessage (cmd);
                    }
                    else
                    {
                        cmd.dest_id = source;
                        cmd.dest_handle = source_handle;
                        
                        if (cmd.action () == CMD_IGNORE)
                        {
                            cmd.setAction (destFilter ? CMD_NULL_DEST_MESSAGE : CMD_NULL_MESSAGE);
                            cmd.source_handle = mid;
                            deliverMessage (cmd);
                            return;
                        }
                        cmd.setAction (destFilter ? CMD_DEST_FILTER_RESULT : CMD_FILTER_RESULT);

                        cmd.source_handle = FiltI->handle;
                        cmd.source_id = global_broker_id;
                        deliverMessage (cmd);
                    }
                }
            }
        }
        else
        {
            assert (false);
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
    }
    else
    {
        auto route = getRoute (cmd.dest_id);
        transmit (route, cmd);
    }
}

}  // namespace helics
