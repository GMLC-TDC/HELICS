/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/core/core-common.h"
#include "ActionMessage.h"
#include "BasicHandleInfo.h"
#include "EndpointInfo.h"
#include "FederateState.h"
#include "FilterInfo.h"
#include "PublicationInfo.h"
#include "SubscriptionInfo.h"
#include "helics-time.h"
#include "helics/common/blocking_queue.h"
#include "helics/config.h"
#include "helics/core/core-data.h"
#include "helics/core/core.h"
#include "helics/core/core-exceptions.h"
#include "FilterFunctions.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#define USE_LOGGING 1
#if USE_LOGGING
#if HELICS_HAVE_GLOG
#include <glog/logging.h>
#define ENDL ""
#else
#define LOG(LEVEL) std::cout
#define ENDL std::endl
#endif
#else
#define LOG(LEVEL) std::ostringstream ()
#define ENDL std::endl
#endif

namespace helics
{
using federate_id_t = Core::federate_id_t;
using Handle = Core::Handle;

CommonCore::CommonCore (){};

void CommonCore::initialize (const std::string &initializationString)
{
    bool exp = false;
    if (_initialized.compare_exchange_strong (exp, true))
    {
		auto loc = initializationString.find("-minfed=");
		auto l2 = initializationString.find_first_of(' ', loc);
		_min_federates = std::stoi(initializationString.substr(loc + 8, l2));
		loc = initializationString.find("-identifier=");
		l2 = initializationString.find_first_of(' ', loc);
		identifier = initializationString.substr(loc + 12, l2);
        _broker_thread = std::thread (&CommonCore::broker, this);
    }
}


CommonCore::~CommonCore ()
{
    if (_initialized)
    {
        _queue.push (CMD_STOP);
        _broker_thread.join ();
    }
}


void CommonCore::setFederationSize (unsigned int /*size*/) {}

FederateState *CommonCore::getFederate (federate_id_t federateID) const
{
    assert (isInitialized ());
    if (_operating)
    {
        // this list is now constant no need to lock
        if ((federateID < _federates.size ()) && (federateID >= 0))
        {
            return _federates[federateID].get ();
        }
        else
        {
            auto fnd = global_id_translation.find (federateID);
            if (fnd != global_id_translation.end ())
            {
                return _federates[fnd->second].get ();
            }
        }
    }
    else
    {
        // need to lock here since the list could be changing
        std::lock_guard<std::mutex> lock (_mutex);
        if ((federateID < _federates.size ()) && (federateID >= 0))
        {
            return _federates[federateID].get ();
        }
        else
        {
            auto fnd = global_id_translation.find (federateID);
            if (fnd != global_id_translation.end ())
            {
                return _federates[fnd->second].get ();
            }
        }
    }
    return nullptr;
}

FederateState *CommonCore::getHandleFederate (Handle id_)
{
    assert (isInitialized ());
    if (_operating)
    {
        // this list is now constant no need to lock
        if ((id_ < handles.size ()) && (id_ >= 0))
        {
            return _federates[handles[id_]->local_fed_id].get ();
        }
    }
    else
    {
        // need to lock here since the list could be changing
        std::lock_guard<std::mutex> lock (_mutex);
        if ((id_ < handles.size ()) && (id_ >= 0))
        {
            return _federates[handles[id_]->local_fed_id].get ();
        }
    }
    return nullptr;
}

BasicHandleInfo *CommonCore::getHandleInfo (Handle id_) const
{
    if (_operating)
    {
        // this list is now constant no need to lock
        if ((id_ < handles.size ()) && (id_ >= 0))
        {
            return handles[id_].get ();
        }
    }
    else
    {
        // need to lock here since the list could be changing
        std::lock_guard<std::mutex> lock (_mutex);
        if ((id_ < handles.size ()) && (id_ >= 0))
        {
            return handles[id_].get ();
        }
    }
    return nullptr;
}


BasicHandleInfo *CommonCore::getLocalEndpoint (const std::string &name)
{
    auto fnd = endpoints.find (name);
    if (fnd != endpoints.end ())
    {
		return getHandleInfo(fnd->second);
       
    }
	return nullptr;
}


bool CommonCore::isLocal (Core::federate_id_t global_id) const
{
    auto fnd = global_id_translation.find (global_id);
    return (fnd != global_id_translation.end ());
}

int32_t CommonCore::getRoute (Core::federate_id_t global_id) const
{
    auto fnd = routing_table.find (global_id);
    return (fnd != routing_table.end ()) ? fnd->second : 0;
}

bool CommonCore::isInitialized () const { return _initialized; }


void CommonCore::error (federate_id_t federateID, int errorCode)
{
    auto fed = getFederate (federateID);
    fed->setState (HELICS_ERROR);
    ActionMessage m (ActionMessage::action_t::cmd_error);
	m.source_id = fed->global_id;
    m.source_handle = errorCode;
    _queue.push (m);
}


void CommonCore::finalize (federate_id_t federateID)
{
    auto fed = getFederate (federateID);
    fed->setState (HELICS_FINISHED);
    ActionMessage bye (CMD_BYE);
    bye.source_id = fed->global_id;
    _queue.push (bye);
}

bool CommonCore::allInitReady () const
{
    std::lock_guard<std::mutex> lock (_mutex);
	//the federate count must be greater than the min size
	if (_federates.size() < _min_federates)
	{
		return false;
	}
	//all federates must be requesting init
    for (auto &fed : _federates)
    {
        if (fed->init_requested == false)
        {
            return false;
        }
    }
    return true;
}

void CommonCore::enterInitializingState (federate_id_t federateID)
{
    auto fed = getFederate (federateID);
    if (HELICS_INITIALIZING == fed->getState ())
    {
        return;
    }
    assert (HELICS_CREATED == fed->getState ());

    std::unique_lock<std::mutex> lock (fed->_mutex);
    fed->init_requested = true;
    lock.unlock ();
    ActionMessage m (CMD_INIT);
    m.source_id = fed->global_id;
    _queue.push (m);

    auto check = fed->processQueue ();
    if (check == false)
    {
        assert (false);
        // TODO::throw an error here
    }
}


bool CommonCore::enterExecutingState (federate_id_t federateID, bool iterationCompleted)
{
    auto fed = getFederate (federateID);
    if (HELICS_EXECUTING == fed->getState ())
    {
        return true;
    }
    assert (HELICS_INITIALIZING == fed->getState ());

    ActionMessage exec (CMD_EXEC_REQUEST);
    exec.iterationComplete = iterationCompleted;
    _queue.push (exec);

    auto ret = fed->processQueue ();

    return ret;
}


federate_id_t CommonCore::registerFederate (const std::string &name, const CoreFederateInfo &info)
{
    

    assert (isInitialized ());

    auto fed = std::make_unique<FederateState> (name, info);

	std::unique_lock<std::mutex> lock(_mutex);
    auto id = fed->local_id = static_cast<decltype (fed->local_id)> (_federates.size ());

    _federates.push_back (std::move (fed));
	lock.unlock();
    ActionMessage m (CMD_REG_FED);
    m.name = name;
	transmit(0, m);  //just directly transmit, no need to process in the queue
	auto b=fed->processQueue();
	if (b)
	{
		return id;
		
	}
	throw (registrationFailure());
}


const std::string &CommonCore::getFederateName (federate_id_t federateID) const
{
	auto fed = getFederate(federateID);
	if (fed != nullptr)
	{
		return fed->name;
	}
	throw(invalidIdentifier());
}


federate_id_t CommonCore::getFederateId (const std::string &name)
{
    std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());

    std::string name_as_string (name);

    for (auto &fed : _federates)
    {
        if (name_as_string == fed->name)
        {
            return fed->local_id;
        }
    }

    return invalid_fed_id;
}


int32_t CommonCore::getFederationSize ()
{
	if (_initialized)
	{
		return _global_federation_size;
	}
	//if we are not initialization return the local federation size
	std::lock_guard<std::mutex> lock(_mutex);
	return static_cast<int32_t>(_federates.size());
    
}


Time CommonCore::timeRequest (federate_id_t federateID, Time next)
{
    auto fed = getFederate (federateID);
	if (HELICS_EXECUTING == fed->getState())
	{
		auto ret = fed->requestTime(next, true);
		return ret.first;
	}
	throw(invalidFunctionCall());
}


std::pair<Time, bool> CommonCore::requestTimeIterative (federate_id_t federateID, Time next, bool localConverged)
{
    auto fed = getFederate (federateID);
	if (fed == nullptr)
	{
		throw(invalidIdentifier());
	}
	if (HELICS_EXECUTING == fed->getState())
	{
		// limit the iterations
		if (localConverged == false)
		{
			if (fed->iteration >= _max_iterations)
			{
				localConverged = true;
			}
		}

		return fed->requestTime(next, localConverged);
	}
	throw(invalidFunctionCall());
}


uint64_t CommonCore::getCurrentReiteration (federate_id_t federateID)
{
	auto fed = getFederate(federateID);
	if (fed != nullptr)
	{
		return getFederate(federateID)->iteration;
	}
	throw(invalidIdentifier());
}


void CommonCore::setMaximumIterations (federate_id_t federateID, uint64_t iterations)
{
    auto fed = getFederate (federateID);
	if (fed != nullptr)
	{
		std::lock_guard<std::mutex> lock(fed->_mutex);
		_max_iterations = iterations;
	}
	throw(invalidIdentifier());
  
}


void CommonCore::setTimeDelta (federate_id_t federateID, Time time)
{
    auto fed = getFederate (federateID);
	if (fed != nullptr)
	{
		if (time <= timeZero)
		{
			throw(invalidParameter());
		}
		std::lock_guard<std::mutex> lock(fed->_mutex);
		fed->info.timeDelta = time;
	}
	throw(invalidIdentifier());
}


void CommonCore::setLookAhead (federate_id_t federateID, Time lookAheadTime)
{
    auto fed = getFederate (federateID);
	if (fed != nullptr)
	{
		if (lookAheadTime < timeZero)
		{
			throw(invalidParameter());
		}
		std::lock_guard<std::mutex> lock(fed->_mutex);
		fed->info.lookAhead = lookAheadTime;
	}
	throw(invalidIdentifier());
}

void CommonCore::setImpactWindow (federate_id_t federateID, Time impactTime)
{
    auto fed = getFederate (federateID);
	if (fed != nullptr)
	{
		if (impactTime < timeZero)
		{
			throw(invalidParameter());
		}
		std::lock_guard<std::mutex> lock(fed->_mutex);
		fed->info.impactWindow = impactTime;
	}
	throw(invalidIdentifier());
}


Core::Handle CommonCore::getNewHandle () { return handleCounter++; }

// comparison auto lambda  Functions like a template
static auto compareFunc = [](const auto &A, const auto &B) { return (A->id < B->id); };

void CommonCore::createBasicHandle (Handle id_,
                                    federate_id_t federateId,
                                    BasicHandleType HandleType,
                                    const std::string &key,
                                    const std::string &type,
                                    const std::string &units,
                                    bool required)
{
    auto hndl = std::make_unique<BasicHandleInfo> (id_, federateId, HandleType, key, type, units, required);

    std::lock_guard<std::mutex> lock (_mutex);

    // may need to resize the handles
    if (handles.size () <= id_)
    {
        handles.resize (id_ + 5);
    }
    handles[id_] = std::move (hndl);
}

Handle CommonCore::registerSubscription (federate_id_t federateID,
                                         const std::string &key,
                                         const std::string &type,
                                         const std::string &units,
                                         bool required)
{
    LOG (INFO) << "registering SUB " << key << ENDL;

    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        return (-1);  // probably should throw an error here
    }
    assert (fed->getState () == HELICS_CREATED);

    auto id = getNewHandle ();
    fed->createSubscription (id, key, type, units, required);

    createBasicHandle (id, fed->global_id, HANDLE_SUB, key, type, units, required);
    ActionMessage m (CMD_REG_SUB);
    m.source_id = fed->global_id;
    m.source_handle = id;
    m.name = key;
    m.info().type = type;
    m.info().units = units;
    m.required = required;
    _queue.push (m);
    return id;
}


Handle CommonCore::getSubscription (federate_id_t federateID, const std::string &key)
{
	auto fed = getFederate(federateID);
	if (fed != nullptr)
	{
		return fed->getSubscription(key)->id;
	}
	return invalid_Handle;
}


Handle
CommonCore::registerPublication (federate_id_t federateID, const std::string &key, const std::string &type, const std::string &units)
{
    LOG (INFO) << "registering PUB " << key << ENDL;


    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
		throw(invalidIdentifier());
    }
    assert (fed->getState () == HELICS_CREATED);
    std::unique_lock<std::mutex> lock (_mutex);
    auto fnd = publications.find (key);
    if (fnd != publications.end ())
    {
        throw (-1);  // TODO: make a set of exceptions;
    }
    auto id = getNewHandle ();
    publications.emplace (key, id);
    lock.unlock ();
    fed->createPublication (id, key, type, units);

    createBasicHandle (id, fed->global_id, HANDLE_PUB, key, type, units, false);

    ActionMessage m (CMD_REG_PUB);
    m.source_id = fed->global_id;
    m.source_handle = id;
    m.name = key;
    m.info().type = type;
    m.info().units = units;

    _queue.push (m);
    return id;
}


Handle CommonCore::getPublication (federate_id_t federateID, const std::string &key)
{
	auto fed = getFederate(federateID);
	if (fed != nullptr)
	{
		return fed->getPublication(key)->id;
	}
	return invalid_Handle;
}

const std::string nullStr;

const std::string &CommonCore::getUnits (Handle handle) const
{
    auto handleInfo = getHandleInfo (handle);
    if (handleInfo!=nullptr)
    {
        return handleInfo->units;
    }
	return nullStr;
}


const std::string &CommonCore::getType (Handle handle) const
{
    auto handleInfo = getHandleInfo (handle);
    if (handleInfo!=nullptr)
    {
        return handleInfo->units;
    }
	return nullStr;
}


void CommonCore::setValue (Handle handle_, const char *data, uint64_t len)
{
    auto handleInfo = getHandleInfo (handle_);
    assert (handleInfo->what == HANDLE_PUB);


    if (!handleInfo->flag)
    {
        return;  // if the value is not required do nothing
    }
    LOG (INFO) << "setValue: '" << std::string (data, len) << "'" << ENDL;
    ActionMessage mv (CMD_PUB);
    mv.source_id = handleInfo->fed_id;
    mv.source_handle = handle_;
    mv.payload = std::string (data, len);

    _queue.push (mv);
}


data_t *CommonCore::getValue (Handle handle_)
{
    auto handleInfo = getHandleInfo (handle_);
    assert (handleInfo->what == HANDLE_SUB);

    return getFederate (handleInfo->local_fed_id)->getSubscription (handle_)->getData ();
}


void CommonCore::dereference (data_t *data)
{
    if (data)
    {
        if (data->data)
        {
            delete[] data->data;
        }
        delete data;
    }
}

void CommonCore::dereference (message_t *msg)
{
    if (msg)
    {
        if (msg->data)
        {
            delete[] msg->data;
        }

        if (msg->dst)
        {
            delete[] msg->dst;
        }

        if (msg->origsrc == msg->src)
        {
            delete[] msg->origsrc;
        }
        else
        {
            delete[] msg->origsrc;
            delete[] msg->src;
        }

        delete msg;
    }
}


const Handle *CommonCore::getValueUpdates (federate_id_t federateID, uint64_t *size)
{
    auto fed = getFederate (federateID);
	if (fed == nullptr)
	{
		throw(invalidIdentifier());
	}
    *size = fed->events.size ();

    if (0 == *size)
    {
        return nullptr;
    }
    else
    {
        return fed->events.data ();
    }
}


Handle CommonCore::registerEndpoint (federate_id_t federateID, const std::string &name, const std::string &type)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
		throw(invalidIdentifier()); 
    }
    assert (fed->getState () == HELICS_CREATED);
    std::unique_lock<std::mutex> lock (_mutex);
    auto fnd = endpoints.find (name);
    if (fnd != endpoints.end ())
    {
        throw (-1);  // TODO: make a set of exceptions;
    }
    auto id = getNewHandle ();
    endpoints.emplace (name, id);
    lock.unlock ();

    fed->createEndpoint (id, name, type);

    createBasicHandle (id, federateID, HANDLE_END, name, type, "", false);

    ActionMessage m (CMD_REG_END);
    m.source_id = fed->global_id;
    m.source_handle = id;
    m.name = name;
    m.info().type = type;

    _queue.push (m);
    return id;
}


Handle CommonCore::registerSourceFilter (federate_id_t federateID,
                                         const std::string &filterName,
                                         const std::string &source,
                                         const std::string &type_in)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
		throw(invalidIdentifier());
    }
    assert (fed->getState () == HELICS_CREATED);

    auto id = getNewHandle ();
    fed->createFilter (id, false, filterName, source, type_in);

    createBasicHandle (id, federateID, HANDLE_FILTER, filterName, type_in, source, false);

    ActionMessage m (CMD_REG_SRC);
    m.source_id = fed->global_id;
    m.source_handle = id;
    m.name = filterName;
    m.info().target = source;
    m.info().type = type_in;

    _queue.push (m);
    return id;
}


Handle CommonCore::registerDestinationFilter (federate_id_t federateID,
                                              const std::string &filterName,
                                              const std::string &dest,
                                              const std::string &type_in)
{
    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
		throw(invalidIdentifier());
    }
    assert (fed->getState () == HELICS_CREATED);

    auto id = getNewHandle ();
    fed->createFilter (id, true, filterName, dest, type_in);

    createBasicHandle (id, federateID, HANDLE_FILTER, filterName, type_in, dest, true);

    ActionMessage m (CMD_REG_DST);
    m.source_id = fed->global_id;
    m.source_handle = id;
    m.name = filterName;
    m.info().target = dest;
    m.info().type = type_in;

    _queue.push (m);
    return id;
}


void CommonCore::registerFrequentCommunicationsPair (const std::string &source, const std::string &dest)
{
    std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (false);
}

void CommonCore::addDependency (federate_id_t federateId, const std::string &federateName) {}

void CommonCore::send (Handle sourceHandle, const std::string &destination, const char *data, uint64_t length)
{
    auto hndl = getHandleInfo (sourceHandle);
    if (hndl == nullptr)
    {
		throw(invalidIdentifier());
    }

    auto fed = getFederate (hndl->local_fed_id);
    ActionMessage m (CMD_SEND_MESSAGE);

    m.info ().orig_source = hndl->key;
    m.info ().source = hndl->key;
    m.source_handle = sourceHandle;
    m.source_id = hndl->fed_id;

    m.payload = std::string (data, length);
    m.info ().target = destination;
    m.actionTime = fed->time_granted;

    queueMessage (processMessage (hndl, m));

}


void CommonCore::sendEvent (Time time,
                            Handle sourceHandle,
                            const std::string &destination,
                            const char *data,
                            uint64_t length)
{
    auto hndl = getHandleInfo (sourceHandle);
    if (hndl == nullptr)
    {
		throw (invalidIdentifier());
    }

    ActionMessage m (CMD_SEND_MESSAGE);

    m.info ().orig_source = hndl->key;
    m.info ().source = hndl->key;
    m.source_handle = sourceHandle;
    m.source_id = hndl->fed_id;

    m.payload = std::string (data, length);
    m.info ().target = destination;
    m.actionTime = time;

    queueMessage (processMessage (hndl, m));
}


void CommonCore::sendMessage (Handle sourceHandle, message_t *message)
{
    assert (isInitialized ());
    assert (message != nullptr);

    auto hndl = getHandleInfo (sourceHandle);

    ActionMessage m (CMD_SEND_MESSAGE);

    m.info ().orig_source = message->origsrc;

    if (hndl == nullptr)
    {
        m.info ().source = message->src;
    }
    else
    {
        m.info ().source = hndl->key;
        m.source_handle = hndl->id;
        m.source_id = hndl->fed_id;
    }
    m.payload = std::string (message->data, message->len);
    m.info ().target = message->dst;
    m.actionTime = message->time;
    m.source_handle = sourceHandle;

    queueMessage (processMessage (hndl, m));
}

ActionMessage &CommonCore::processMessage (BasicHandleInfo *hndl, ActionMessage &m)
{
    if (hndl == nullptr)
    {
        return m;
    }
    auto filtFunc = getFilterFunctions (hndl->id);
    if (filtFunc->hasSourceOperators)
    {
        for (auto &so : filtFunc->sourceOperators)
        {
            auto FiltI = getFederate (so.first)->getFilter (so.second);
            assert (FiltI->filterOp != nullptr);
            auto tempMessage = createTempMessage (m);
            auto nmessage = FiltI->filterOp->process (&tempMessage);
            if (nmessage.data != tempMessage.data)
            {
                m.payload = std::string (nmessage.data, nmessage.len);
                delete[] nmessage.data;
            }
            m.actionTime = nmessage.time;
        }
    }
    if (filtFunc->hasSourceFilter)
    {
		m.setAction(CMD_SEND_FOR_FILTER);
        m.dest_handle = filtFunc->finalSourceFilter.second;
        m.dest_id = filtFunc->finalSourceFilter.first;
    }
    return m;
}

void CommonCore::queueMessage (ActionMessage &message)
{
    assert (isInitialized ());
    if (message.action() == CMD_SEND_MESSAGE)
    {
        // Find the destination endpoint
        auto localP = getLocalEndpoint (message.info ().target);
        if (localP == nullptr)
        {  // must be a remote endpoint push it to the main queue to deal with
            _queue.push (message);
            return;
        }
        if (localP->destFilter)  // the endpoint has a destination filter
        {
            auto ffunc = getFilterFunctions (localP->id);
            assert (ffunc->hasDestOperator);
            auto FiltI = getFederate (ffunc->destOperator.first)->getFilter (ffunc->destOperator.second);
            assert (FiltI->filterOp != nullptr);

            auto tempMessage = createTempMessage (message);
            auto nmessage = FiltI->filterOp->process (&tempMessage);
            if (nmessage.data != tempMessage.data)
            {
                message.payload = std::string (nmessage.data, nmessage.len);
                delete[] nmessage.data;
            }
            message.actionTime = nmessage.time;
        }
        auto fed = getFederate (localP->local_fed_id);
        fed->queue.push (message);
    }
}


uint64_t CommonCore::receiveCount (Handle destination)
{
    auto fed = getHandleFederate (destination);
    assert (fed->getState () == HELICS_EXECUTING);

    return fed->getQueueSize (destination);
}


message_t *CommonCore::receive (Handle destination)
{
    auto fed = getHandleFederate (destination);
	if (fed == nullptr)
	{
		throw (invalidIdentifier());
	}
	if (fed->getState() != HELICS_EXECUTING)
	{
		return nullptr;
	}

    return fed->receive (destination);
}


std::pair<const Handle, message_t *> CommonCore::receiveAny (federate_id_t federateID)
{
    auto fed = getFederate (federateID);
	if (fed == nullptr)
	{
		throw (invalidIdentifier());
	}
	if (fed->getState() != HELICS_EXECUTING)
	{
		return{ invalid_Handle,nullptr };
	}
    return fed->receive ();
}


uint64_t CommonCore::receiveCountAny (federate_id_t federateID)
{
    auto fed = getFederate (federateID);
	if (fed == nullptr)
	{
		throw (invalidIdentifier());
	}
	if (fed->getState() != HELICS_EXECUTING)
	{
		return 0;
	}

    return fed->getQueueSize ();
}

void CommonCore::logMessage (federate_id_t federateID, int logCode, const std::string &logMessage)
{
    ActionMessage m (ActionMessage::action_t::cmd_log);

    auto fed = getFederate (federateID);
    if (fed == nullptr)
    {
        throw (invalidIdentifier());
    }
    m.source_id = fed->global_id;
    m.payload = logMessage;
    _queue.push (m);
}


void CommonCore::setFilterOperator (Handle filter, FilterOperator *callback)
{
    auto hndl = getHandleInfo (filter);
    assert (HANDLE_FILTER == hndl->what);

    auto FiltI = getFederate (hndl->fed_id)->getFilter (filter);

    FiltI->filterOp = callback;
}

FilterFunctions *CommonCore::getFilterFunctions (Handle id_)
{
    auto fnd = filters.find (id_);
    if (fnd == filters.end ())
    {
        auto ff = std::make_unique<FilterFunctions> ();
        auto ffp = ff.get ();
        filters.emplace (id_, std::move (ff));
        return ffp;
    }
    else
    {
        return fnd->second.get ();
    }
}



uint64_t CommonCore::receiveFilterCount(federate_id_t federateID)
{
	auto fed = getFederate(federateID);
	if (fed == nullptr)
	{
		throw(invalidIdentifier());
	}
	if (fed->getState() != HELICS_EXECUTING)
	{
		return 0;
	}

	return fed->getQueueSize();
}

std::pair<const Handle, message_t*> CommonCore::receiveAnyFilter(federate_id_t federateID)
{
	auto fed = getFederate(federateID);
	if (fed == nullptr)
	{
		throw(invalidIdentifier());
	}
	if (fed->getState() != HELICS_EXECUTING)
	{
		return{ invalid_Handle,nullptr };
	}
	return fed->receive();
}


void CommonCore::setIdentifier(const std::string &name)
{
	if (!_initialized)
	{
		std::lock_guard<std::mutex> lock(_mutex);
		identifier = name;
	}
}

void CommonCore::broker ()
{
    while (true)
    {
        auto command = _queue.pop ();
        // LOG (INFO) << "\"\"\"" << command << std::endl << "\"\"\"" << ENDL;

        switch (command.action())
        {
        case CMD_IGNORE:
            break;
        case CMD_STOP:
			processCommand(command);
            return;
        default:
			processCommand(command);
        }
    }
}


void CommonCore::processCommand (ActionMessage &command)
{
    // LOG (INFO) << "\"\"\"" << command << std::endl << "\"\"\"" << ENDL
    switch (command.action())
    {
    case CMD_IGNORE:
	default:
        break;
    case CMD_CONNECT:
        // forward the connection request to the higher level
        transmit (0, command);
        break;
   
    case CMD_REG_ROUTE:
		addRoute(command.dest_handle, command.payload);
        break;
    case CMD_STOP:
    {
        ActionMessage m (CMD_DISCONNECT);
        m.source_id = global_broker_id;
        transmit (0, m);
    }
        return;  //the exit point of the simulation
    case CMD_TIME_REQUEST:
    case CMD_TIME_GRANT:
	case CMD_EXEC_GRANT:
	case CMD_EXEC_REQUEST:
    {
        // route the message to all dependent feds
        auto &dep = getFederate (command.source_id)->dependents;
        for (auto &fed_id : dep)
        {
            if (isLocal (fed_id))
            {
                auto fed = getFederate (fed_id);
                fed->queue.push (command);
            }
            else
            {
                auto route = getRoute (fed_id);
                command.dest_id = fed_id;
                transmit (route, command);
            }
        }
    }
    break;
    case CMD_SEND_FOR_FILTER:
        if (isLocal (command.dest_id))
        {
            auto fed = getFederate (command.dest_id);
            fed->queue.push (command);
        }
        else
        {  // send it onward
            transmit (getRoute (command.dest_id), command);
        }
        break;
    case CMD_PUB:
    {
        // route the message to all the subscribers
        auto pubInfo = getFederate (command.source_id)->getPublication (command.source_handle);
        for (auto &subscriber : pubInfo->subscribers)
        {
            command.dest_id = subscriber.first;
            command.dest_handle = subscriber.second;

            if (isLocal (subscriber.first))
            {
                auto fed = getFederate (subscriber.first);
                fed->queue.push (command);
            }
            else
            {
                transmit (getRoute (subscriber.first), command);
            }
        }
    }
    break;
    case CMD_BYE:
        break;
    case CMD_LOG:
    case CMD_ERROR:
        transmit (0, command);
        break;
    case CMD_REG_PUB:
        transmit (0, command);
        break;
    case CMD_REG_SUB:
        transmit (0, command);
        break;
    case CMD_REG_END:
        transmit (0, command);
        break;
    case CMD_REG_DST:
        transmit (0, command);
        break;
    case CMD_REG_SRC:
        transmit (0, command);
        break;
    case CMD_REG_FED:
        transmit (0, command);
        break;
    case CMD_BROKER_ACK:
        if (command.payload == identifier)
        {
            global_broker_id = command.dest_id;
        }
        break;
    case CMD_FED_ACK:
    {
        auto id = getFederateId (command.payload.c_str ());
        if (id != invalid_fed_id)
        {
            auto fed = getFederate (id);
            fed->queue.push (command);
            // now add the new global id to the translation table
            std::lock_guard<std::mutex> lock (_mutex);
            global_id_translation.emplace (fed->local_id, command.dest_id);
        }
    }
    break;
    case CMD_INIT:
        if (allInitReady ())
        {
            command.source_id = global_broker_id;
            transmit (0, command);
        }
        break;
    case CMD_INIT_GRANT:
        // forward the grant to all federates
        for (auto &fed : _federates)
        {
            fed->queue.push (command);
        }
        break;

    case CMD_SEND_MESSAGE:
    {
        auto fnd = endpoints.find (command.info ().target);
        if (fnd != endpoints.end ())
        {  // destination is local
            auto fed = getHandleFederate (fnd->second);
            fed->queue.push (command);
            command.dest_id = fed->global_id;
            command.dest_handle = fnd->second;
        }
        else
        {
            auto kfnd = knownExternalEndpoints.find (command.info ().target);
            if (kfnd != knownExternalEndpoints.end ())
            {  // destination is known
                auto route = getRoute (kfnd->second);
                transmit (route, command);
            }
            else
            {
                transmit (0, command);
            }
        }
    }
    break;
    }
}



}  // namespace helics
