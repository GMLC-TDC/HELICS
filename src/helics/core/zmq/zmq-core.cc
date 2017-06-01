/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/config.h"
#include "helics/common/base64.h"
#include "helics/common/blocking_queue.h"
#include "helics/core/core.h"
#include "helics/core/core-data.h"
#include "helics/core/helics-time.h"
#include "helics/core/zmq/zmq-core.h"
#include "helics/core/zmq/zmq-fedstate.h"
#include "helics/core/zmq/zmq-handle.h"
#include "helics/core/zmq/zmq-helper.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

#include <zmq.h>

#define USE_LOGGING 0
#if USE_LOGGING
#if HELICS_HAVE_GLOG
#include <glog/logging.h>
#define ENDL ""
#else
#define LOG(LEVEL) std::cout
#define ENDL std::endl
#endif
#else
#define LOG(LEVEL) std::ostringstream()
#define ENDL std::endl
#endif

static const std::string DEFAULT_BROKER = "tcp://localhost:5555";

static inline std::string gen_id() {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
		std::string uuid_str = boost::lexical_cast<std::string>(uuid);
#ifdef _WIN32
    std::string pid_str = boost::lexical_cast<std::string>(GetCurrentProcessId());
#else
    std::string pid_str = boost::lexical_cast<std::string>(getpid());
#endif
    return pid_str+"-"+uuid_str;
}

namespace helics
{
typedef Core::federate_id_t federate_id_t;
typedef Core::Handle Handle;
typedef Core::FederateInfo FederateInfo;

void broker (ZeroMQCore *core)
{
    void *zctx;
    void *zsock;
    std::string ID;
    int rc;

    zctx = zmq_ctx_new();
    assert(zctx);
    LOG(INFO) << "created ZMQ context" << ENDL;

    zsock = zmq_socket(zctx, ZMQ_DEALER);
    assert(zsock);
    LOG(INFO) << "created ZMQ socket" << ENDL;

    ID = gen_id();

    rc = zmq_setsockopt(zsock, ZMQ_IDENTITY, ID.data(), ID.size());
    if (0 != rc) {
        LOG(ERROR) << "zmq_setsockopt ZMQ_IDENTITY " << ID << ENDL;
    }
    assert (0 == rc);
    LOG(INFO) << "socket identity is " << ID << ENDL;

    rc = zmq_connect(zsock, core->_broker_address.c_str());
    if (0 != rc) {
        LOG(ERROR) << "zmq_connect " << core->_broker_address << ENDL;
    }
    assert (0 == rc);
    LOG(INFO) << "socket connected to " << core->_broker_address << ENDL;

    while (true)
    {
        bool any_event = false;
        std::string command;

        /* check thread queue for command */
        if (core->_queue.try_pop(&command))
        {
            std::string chunk;
            any_event |= true;
            LOG (INFO) << "command queue returned an event" << ENDL;
            LOG (INFO) << "\"\"\"" << command << std::endl << "\"\"\"" << ENDL;
            std::istringstream iss (command);
            std::getline (iss, chunk);
            if (CMD_STOP == chunk) {
                break;
            }
            zmqx_send(zsock, command);
            if (CMD_BYE == chunk) {
                std::getline (iss, chunk);
                federate_id_t id;
                id = boost::lexical_cast<federate_id_t>(chunk);
                std::ostringstream oss;
                oss << CMD_ACK << std::endl;
                core->_federates[id]->queue.push(oss.str());
            }
        }

        /* check zmq queue for message */
        zmq_pollitem_t items[] = {
            { zsock, 0, ZMQ_POLLIN, 0 },
        };

        rc = zmq_poll(items, 1, 0);

        if (items[0].revents & ZMQ_POLLIN) {
            any_event |= true;
            LOG(INFO) << "zmq_poll returned an event" << ENDL;

            (void) zmqx_recv(zsock, command);
            LOG(INFO) << "command='" << command << "'" << ENDL;

            if (command == CMD_CONNECT) {
                size_t n;
                zmqx_recv(zsock, n);
                assert(n > 0);
                std::ostringstream oss;
                oss << CMD_ACK << std::endl << n << std::endl;
                core->_queue_response.push(oss.str());
            }
            else if (command == CMD_INIT) {
                unsigned int n;
                zmqx_recv(zsock, n);
                assert(n > 0);
                core->_n_feds = n;
                LOG(INFO) << n << " federates have initialized, locally " << core->_federates.size() << ENDL;
                std::ostringstream oss;
                oss << CMD_ACK << std::endl;
                for (size_t i=0; i<core->_federates.size(); ++i) {
                    LOG(INFO) << "pushing ACK to fed " << i << ENDL;
                    core->_federates[i]->queue.push(oss.str());
                }
            }
            else if (command == CMD_EXEC) {
                std::ostringstream oss;
                oss << CMD_ACK << std::endl;
                for (size_t i=0; i<core->_federates.size(); ++i) {
                    LOG(INFO) << "pushing ACK to fed " << i << ENDL;
                    core->_federates[i]->queue.push(oss.str());
                }
            }
            else if (command == CMD_TIME) {
                unsigned int id;
                Time::baseType granted;
                bool test;
                zmqx_recv(zsock, id);
                zmqx_recv(zsock, granted);
                zmqx_recv(zsock, test);
                core->_time_granted.setBaseTimeCode(granted);
                std::ostringstream oss;
                oss << CMD_ACK << std::endl << test << std::endl;
                core->_federates[id]->queue.push(oss.str());
            }
            else if (command == CMD_TIME_IT) {
                unsigned int id;
                bool test;
                zmqx_recv(zsock, id);
                zmqx_recv(zsock, test);
                std::ostringstream oss;
                oss << CMD_ACK << std::endl << test << std::endl;
                core->_federates[id]->queue.push(oss.str());
            }
            else if (command == CMD_PUB) {
                Handle handle;
                std::string data;
                zmqx_recv(zsock, handle);
                zmqx_recv(zsock, data);
                LOG (INFO) << "pub: '" << data << "'" << ENDL;
                ZeroMQHandle *sub = core->_handles[handle];
                sub->data = data;
                core->_federates[sub->fed_id]->events.push_back (sub->id);
            }
            else if (command == CMD_DIE) {
                LOG(ERROR) << "received DIE from broker" << ENDL;
                break;
            }
            else {
                LOG(ERROR) << "unrecognized command '" << command << "'" << ENDL;
            }
        }

        /*
		if (!any_event) {
            LOG(INFO) << "no events detected, busy sleeping 1s with thread yield" << ENDL;
			// "busy sleep" while suggesting that other threads run
			// for a small amount of time
            auto us = std::chrono::seconds(1);
			auto start = std::chrono::high_resolution_clock::now();
			auto end = start + us;
			do {
				std::this_thread::yield();
			} while (std::chrono::high_resolution_clock::now() < end);
		}
        */
    }

    rc = zmq_close(zsock);
    assert (0 == rc);
    LOG(INFO) << "zmq socket closed" << ENDL;

    rc = zmq_term(zctx);
    assert (0 == rc);
    LOG(INFO) << "zmq context closed" << ENDL;
}


void ZeroMQCore::initialize (const char *initializationString)
{
	std::lock_guard<std::mutex> lock (_mutex);
    const char *env;

    if ((env = getenv("HELICS_BROKER"))!=nullptr)
    {
        _broker_address = env;
    }
    else if (initializationString && strlen (initializationString) > 5)
    {
        _broker_address = initializationString;
    }
    else
    {
        _broker_address = DEFAULT_BROKER;
    }

    _initialized = true;
    _federates.clear ();
    _n_cores = 0;
    _n_feds = 0;
    _time_granted = 0;
    _iter = 0;
    _max_iterations = 100;
    _thread_broker = std::thread (broker, this);

    std::ostringstream oss;
    oss << CMD_CONNECT << std::endl;
    _queue.push (oss.str ());

    std::string chunk;
    std::string command = _queue_response.pop();
    LOG(INFO) << "_queue_response: " << command << ENDL;
    std::istringstream iss (command);
    std::getline (iss, chunk);
    assert (CMD_ACK == chunk);
    std::getline (iss, chunk);
    _n_cores = boost::lexical_cast<unsigned int>(chunk);
    LOG(INFO) << "There are " << _n_cores << " cores in this co-sim." << ENDL;
}


ZeroMQCore::~ZeroMQCore ()
{
    _queue.push (CMD_STOP);
    _thread_broker.join ();

    for (size_t i = 0; i < _federates.size (); ++i)
    {
        delete _federates[i];
    }
    for (size_t i = 0; i < _handles.size (); ++i)
    {
        delete _handles[i];
    }
}


bool ZeroMQCore::isInitialized () { return _initialized; }


void ZeroMQCore::error (federate_id_t federateID, int /*errorCode*/)
{
    std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());

    _federates[federateID]->state = HELICS_FAILURE;
}


void ZeroMQCore::finalize (federate_id_t federateID)
{
    std::unique_lock<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());

    // hold on the the queue so we can release the outer lock
    BlockingQueue<std::string> &fed_queue = _federates[federateID]->queue;

    lock.unlock();

    std::ostringstream oss;
    oss << CMD_BYE << std::endl << federateID << std::endl << 0UL << std::endl;
    _queue.push (oss.str ());

    _federates[federateID]->state = HELICS_NONE;

    std::string chunk;
    LOG (INFO) << "popping message from fed_queue " << federateID << ENDL;
    std::string command = fed_queue.pop ();
    std::istringstream iss (command);
    std::getline (iss, chunk);
    assert (CMD_ACK == chunk);
}


void ZeroMQCore::enterInitializingState (federate_id_t federateID)
{
    std::unique_lock<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());
	if (HELICS_INITIALIZING == _federates[federateID]->state)
	{
		return;
	}
    assert (HELICS_CREATED == _federates[federateID]->state);

    // hold on the the queue so we can release the outer lock
    BlockingQueue<std::string> &fed_queue = _federates[federateID]->queue;

    lock.unlock();

    std::ostringstream oss;
    oss << CMD_INIT << std::endl << federateID << std::endl;
    _queue.push (oss.str ());

    std::string chunk;
    LOG (INFO) << "popping message from fed_queue " << federateID << ENDL;
    std::string command = fed_queue.pop ();
    std::istringstream iss (command);
    std::getline (iss, chunk);
    assert (CMD_ACK == chunk);

    lock.lock();

    LOG (INFO) << "done initializing " << federateID << ENDL;

    _federates[federateID]->state = HELICS_INITIALIZING;
}


bool ZeroMQCore::enterExecutingState(federate_id_t federateID, bool iterationCompleted)
{
    std::unique_lock<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());
	if (HELICS_EXECUTING == _federates[federateID]->state)
	{
		return true;
	}
    assert (HELICS_INITIALIZING == _federates[federateID]->state);

    // hold on the the queue so we can release the outer lock
    BlockingQueue<std::string> &fed_queue = _federates[federateID]->queue;

    lock.unlock();

    std::ostringstream oss;
    oss << CMD_EXEC << std::endl << federateID << std::endl;
    _queue.push (oss.str ());

    std::string chunk;
    std::string command = fed_queue.pop ();
    std::istringstream iss (command);
    std::getline (iss, chunk);
    assert (CMD_ACK == chunk);

    lock.lock();

    _federates[federateID]->state = HELICS_EXECUTING;

	return true;
}


federate_id_t ZeroMQCore::registerFederate (const char *name, const FederateInfo &info)
{
	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());

    ZeroMQFederateState *me = nullptr;

    me = new ZeroMQFederateState (name, info);
    me->id = static_cast<decltype (me->id)> (_federates.size ());
    _federates.push_back (me);

    std::ostringstream oss;
    oss << CMD_REG_FED << std::endl
        << me->id << std::endl
        << name << std::endl
        << to_string(info) << std::endl;
    _queue.push (oss.str ());

    return me->id;
}


const char *ZeroMQCore::getFederateName (federate_id_t federateID)
{
	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());

    return _federates[federateID]->name.c_str();
}


federate_id_t ZeroMQCore::getFederateId (const char *name)
{
	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (nullptr != name);

    std::string name_as_string (name);

    for (size_t i = 0; i < _federates.size (); ++i)
    {
        if (name_as_string == std::string (_federates[i]->name))
        {
            return federate_id_t (i);
        }
    }

    assert (false);
    return federate_id_t (-1);
}


void ZeroMQCore::setFederationSize (unsigned int size)
{
    std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());

    std::ostringstream oss;
    oss << CMD_SIZE << std::endl << size << std::endl;
    _queue.push (oss.str ());
}


unsigned int ZeroMQCore::getFederationSize ()
{
	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());

    return _n_feds;
}


Time ZeroMQCore::timeRequest (federate_id_t federateID, Time next)
{
    std::unique_lock<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());
    assert (HELICS_EXECUTING == _federates[federateID]->state);

    /* locally invalidate events */
    _federates[federateID]->events.clear();

    // hold on the the queue so we can release the outer lock
    BlockingQueue<std::string> &fed_queue = _federates[federateID]->queue;

    lock.unlock();

    bool converged = false;
    while (!converged)
    {
        std::ostringstream oss;
        oss << CMD_TIME << std::endl << federateID << std::endl << next.getBaseTimeCode () << std::endl;
        _queue.push (oss.str ());
        /* wait for ack from broker */
        std::string chunk;
        std::string command = fed_queue.pop ();
        std::istringstream iss (command);
        std::getline (iss, chunk);
        assert (CMD_ACK == chunk);
        std::getline (iss, chunk);
        {
            std::istringstream iss_ (chunk);
            iss_ >> converged;
        }
    }
    return _time_granted;
}


std::pair<Time, bool> ZeroMQCore::requestTimeIterative (federate_id_t federateID, Time next, bool localConverged)
{
    std::unique_lock<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());
    assert (HELICS_EXECUTING == _federates[federateID]->state);

    // hold on the the queue so we can release the outer lock
    BlockingQueue<std::string> &fed_queue = _federates[federateID]->queue;

    lock.unlock();

    std::ostringstream oss;
    oss << CMD_TIME_IT << std::endl
        << federateID << std::endl
        << next.getBaseTimeCode () << std::endl
        << localConverged << std::endl;
    _queue.push (oss.str ());
    /* wait for ack from broker */
    std::string chunk;
    std::string command = fed_queue.pop ();
    std::istringstream iss (command);
    std::getline (iss, chunk);
    assert (CMD_ACK == chunk);
    std::getline (iss, chunk);
    {
        std::istringstream iss_ (chunk);
        iss_ >> localConverged;
    }
    return std::make_pair (_time_granted, localConverged);
}


uint64_t ZeroMQCore::getCurrentReiteration (federate_id_t federateID)
{
    std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    return _iter;
}


void ZeroMQCore::setMaximumIterations (federate_id_t federateID, uint64_t iterations)
{
    std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
	assert(federateID < _federates.size());

	_federates[federateID]->max_iterations = iterations;
    _max_iterations = iterations;
}


void ZeroMQCore::setTimeDelta (federate_id_t federateID, Time time)
{
    std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());

    _federates[federateID]->time_delta = time;
}


void ZeroMQCore::setLookAhead(federate_id_t federateID, Time lookAheadTime)
{
	assert(isInitialized());
	assert(federateID < _federates.size());

	_federates[federateID]->time_look_ahead = lookAheadTime;
}

void ZeroMQCore::setImpactWindow(federate_id_t federateID, Time impactTime)
{
	assert(isInitialized());
	assert(federateID < _federates.size());

	_federates[federateID]->time_impact = impactTime;
}

Handle ZeroMQCore::registerSubscription (federate_id_t federateID,
                                       const char *key,
                                       const char *type,
                                       const char *units,
                                       bool required)
{
    LOG (INFO) << "registering SUB " << key << ENDL;

    std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());
    assert (_federates[federateID]->state == HELICS_CREATED);

    ZeroMQHandle *handle = new ZeroMQHandle (federateID, HANDLE_SUB, key, type, units, required);
    handle->id = static_cast<decltype (handle->id)> (_handles.size ());
    _handles.push_back (handle);
    _federates[federateID]->subs.emplace (key, handle);

    std::ostringstream oss;
    oss << CMD_REG_SUB << std::endl
      << federateID << std::endl
      << handle->id << std::endl
      << key << std::endl
      << type << std::endl
      << units << std::endl
      << required << std::endl;
    _queue.push(oss.str());

    return handle->id;
}


Handle ZeroMQCore::getSubscription (federate_id_t federateID, const char *key)
{
    std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());

    auto it = _federates[federateID]->subs.find (key);
    assert(it != _federates[federateID]->subs.end());

    return it->second->id;
}


Handle ZeroMQCore::registerPublication (federate_id_t federateID, const char *key, const char *type, const char *units)
{
    LOG (INFO) << "registering PUB " << key << ENDL;
	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());
    assert (_federates[federateID]->state == HELICS_CREATED);

    ZeroMQHandle *handle = new ZeroMQHandle (federateID, HANDLE_PUB, key, type, units);
    handle->id = static_cast<decltype (handle->id)> (_handles.size ());
    _handles.push_back (handle);
    _federates[federateID]->pubs.emplace (key, handle);

    std::ostringstream oss;
    oss << CMD_REG_PUB << std::endl
      << federateID << std::endl
      << handle->id << std::endl
      << key << std::endl
      << type << std::endl
      << units << std::endl;
    _queue.push(oss.str());

    return handle->id;
}


Handle ZeroMQCore::getPublication (federate_id_t federateID, const char *key)
{
	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());

    auto it = _federates[federateID]->pubs.find (key);
    assert(it != _federates[federateID]->pubs.end());

    return it->second->id;
}


const char *ZeroMQCore::getUnits (Handle handle)
{
	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (handle < _handles.size ());

    return _handles[handle]->units.c_str();
}


const char *ZeroMQCore::getType (Handle handle)
{
	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (handle < _handles.size ());

    return _handles[handle]->type.c_str();
}


void ZeroMQCore::setValue (Handle handle_, const char *data, uint64_t len)
{
    assert (isInitialized ());
    assert (handle_ < _handles.size ());
    assert (_handles[handle_]->what == HANDLE_PUB);

    LOG (INFO) << "setValue: '" << std::string(data,len) << "'" << ENDL;

    std::string encoded = base64_encode((unsigned char const*)data, len);

    // we write the data byte-wise in case our
    // getline delimiter is part of the data
    std::ostringstream oss;
    oss << CMD_PUB << std::endl
      << handle_ << std::endl
      << len << std::endl
      << encoded.size() << std::endl
      << encoded << std::endl;

    _queue.push (oss.str ());
}


data_t *ZeroMQCore::getValue (Handle handle_)
{
    assert (isInitialized ());
    assert (handle_ < _handles.size ());
    assert (_handles[handle_]->what == HANDLE_SUB);

    std::string &the_data = _handles[handle_]->data;

    data_t *data = new data_t;
    if (!the_data.empty()) {
        data->data = new char[the_data.size()+1];
        size_t len = the_data.copy(data->data, the_data.size(), 0);
        data->data[len] = '\0';
        data->len = len;
    }
    else {
        data->data = nullptr;
        data->len = 0;
    }
    return data;
}


void ZeroMQCore::dereference (data_t *data)
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

void ZeroMQCore::dereference(message_t *msg)
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


const Handle *ZeroMQCore::getValueUpdates (federate_id_t federateID, uint64_t *size)
{
    assert (isInitialized ());
    assert (federateID < _federates.size ());

    *size = _federates[federateID]->events.size();

    if (0 == *size) {
        return nullptr;
    }
    else {
        return &_federates[federateID]->events[0];
    }
}


Handle ZeroMQCore::registerEndpoint (federate_id_t federateID, const char *name, const char *type)
{
    LOG (INFO) << "registering END " << name << ENDL;

	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());

    ZeroMQHandle *handle = new ZeroMQHandle (federateID, HANDLE_END, name, type, "", false, true);
    handle->id = static_cast<decltype (handle->id)> (_handles.size ());
    _handles.push_back (handle);
    _federates[federateID]->ends.emplace (name, handle);

    std::ostringstream oss;
    oss << CMD_REG_END << std::endl
      << federateID << std::endl
      << handle->id << std::endl
      << name << std::endl
      << type << std::endl;
    _queue.push(oss.str());

    return handle->id;
}


Handle ZeroMQCore::registerSourceFilter (federate_id_t federateID,
                                       const char *filterName,
                                       const char *source,
                                       const char *type_in)
{
	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());

	ZeroMQHandle *handle = new ZeroMQHandle(federateID, HANDLE_SRC_FILTER, filterName, type_in, "");
	handle->id = static_cast<decltype (handle->id)> (_handles.size());
	handle->filterTarget = source;
	_handles.push_back(handle);
	_federates[federateID]->filters.emplace(filterName, handle);

    std::ostringstream oss;
    oss << CMD_REG_SRC << std::endl
      << federateID << std::endl
      << handle->id << std::endl
      << filterName << std::endl
      << source << std::endl
      << type_in << std::endl;
    _queue.push(oss.str());

	return handle->id;
}


Handle ZeroMQCore::registerDestinationFilter (federate_id_t federateID,
                                            const char *filterName,
                                            const char *dest,
                                            const char *type_in)
{
	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());

	ZeroMQHandle *handle = new ZeroMQHandle(federateID, HANDLE_END_FILTER, filterName, type_in, "");
	handle->id = static_cast<decltype (handle->id)> (_handles.size());
	handle->filterTarget = dest;
	_handles.push_back(handle);
	_federates[federateID]->filters.emplace(filterName, handle);

    std::ostringstream oss;
    oss << CMD_REG_DST << std::endl
      << federateID << std::endl
      << handle->id << std::endl
      << filterName << std::endl
      << dest << std::endl
      << type_in << std::endl;
    _queue.push(oss.str());

	return handle->id;
}


void ZeroMQCore::registerFrequentCommunicationsPair (const char *source, const char *dest)
{
	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (false);
}

void ZeroMQCore::addDependency(federate_id_t federateId, const char *federateName)
{

}

void ZeroMQCore::send (Handle sourceHandle, const char *destination, const char *data, uint64_t length)
{
    assert (isInitialized ());
	assert (sourceHandle < _handles.size ());

	message_t *msg = new message_t();

	const char *source = _handles[sourceHandle]->key.c_str();

	char *srcCopy = new char[strlen(source) + 1];
	char *dstCopy = new char[strlen(destination) + 1];
	char *dataCopy = new char[length];

	strcpy(srcCopy, source);
	strcpy(dstCopy, destination);

	for (int i = 0; i < static_cast<int>(length); i++) {
		dataCopy[i] = data[i];
	}

	msg->time = _time_granted;
	msg->data = dataCopy;
	msg->len = length;
	msg->origsrc = srcCopy;
	msg->src = srcCopy;
	msg->dst = dstCopy;

	queueMessage(msg);
}


void ZeroMQCore::sendEvent (Time time,
                          Handle sourceHandle,
                          const char *destination,
                          const char *data,
                          uint64_t length)
{
    assert (isInitialized ());
	assert (sourceHandle < _handles.size ());

	message_t *msg = new message_t();

	const char *source = _handles[sourceHandle]->key.c_str();

	char *srcCopy = new char[strlen(source) + 1];
	char *dstCopy = new char[strlen(destination) + 1];
	char *dataCopy = new char[length];

	strcpy(srcCopy, source);
	strcpy(dstCopy, destination);

	for (int i = 0; i < static_cast<int>(length); i++) {
		dataCopy[i] = data[i];
	}

	msg->time = time;
	msg->data = dataCopy;
	msg->len = length;
	msg->origsrc = srcCopy;
	msg->src = srcCopy;
	msg->dst = dstCopy;

	queueMessage(msg);
}


void ZeroMQCore::sendMessage (message_t *message)
{
    assert (isInitialized ());
	assert(message != nullptr);

	message_t *msgCopy = new message_t();

	char *origSrcCopy = new char[strlen(message->origsrc) + 1];
	char *srcCopy = new char[strlen(message->src) + 1];
	char *dstCopy = new char[strlen(message->dst) + 1];
	char *dataCopy = new char[message->len];

	strcpy(origSrcCopy, message->origsrc);
	strcpy(srcCopy, message->origsrc);
	strcpy(dstCopy, message->dst);

	for (int i = 0; i < static_cast<int>(message->len); i++) {
		dataCopy[i] = message->data[i];
	}

	msgCopy->time = message->time;
	msgCopy->data = dataCopy;
	msgCopy->len = message->len;
	msgCopy->origsrc = origSrcCopy;
	msgCopy->src = srcCopy;
	msgCopy->dst = dstCopy;

	queueMessage(msgCopy);
}


void ZeroMQCore::queueMessage(message_t *message)
{
	assert(isInitialized());
	assert(message != nullptr);

	ZeroMQHandle *dest = nullptr;
	federate_id_t fed_id=0xFFFF'FFFF;

	// Find the destination endpoint
	for (auto fed : _federates) {
		if (fed->ends.count(message->dst) > 0) {
			fed_id = fed->id;
			dest = fed->ends[message->dst];
			break;
		}
	}

	//make sure it was found
	if ((dest == nullptr)||(fed_id==0xFFFF'FFFF)) {
		//THROW something here?
		return;
	}

	// Handle any source endpoint filters
	for (auto filt : _handles) {
		if (filt->what == HANDLE_SRC_FILTER) {
			if (filt->filterTarget == message->origsrc) {
				FilterOperator *op = filt->filterOp;
				(*op)(message);
			}
		}
	}

	// Get the destination endpoint message queue
	auto msgQ = &_federates[fed_id]->message_queue[dest->id];

	std::lock_guard<std::mutex> lock(_federates[fed_id]->_mutex);

	if (msgQ->empty()) {
		msgQ->push_back(message);
	}
	else {
		size_t i = 0;

		// Order the messages from earliest time to latest time, with subordering by arrival time
		while (i < msgQ->size() && msgQ->at(i)->time <= message->time) i++;
		msgQ->insert(msgQ->begin() + i, message);
	}
}


uint64_t ZeroMQCore::receiveCount (Handle destination)
{
    assert (isInitialized ());
	assert (destination < _handles.size ());

	federate_id_t src_id = _handles[destination]->fed_id;

	std::lock_guard<std::mutex> lock(_federates[src_id]->_mutex);

	return _federates[src_id]->message_queue[destination].size();
}


message_t *ZeroMQCore::receive (Handle destination)
{
    assert (isInitialized ());
	assert (destination < _handles.size ());

	federate_id_t fed_id = _handles[destination]->fed_id;

	auto end_point_msg_q = &_federates[fed_id]->message_queue[destination];

	std::lock_guard<std::mutex> lock(_federates[fed_id]->_mutex);

	if (end_point_msg_q->empty()) {
		return nullptr;
	}

	message_t *result = end_point_msg_q->front();
	end_point_msg_q->pop_front();

	// Handle any destination endpoint filters
	for (auto filt : _handles) {
		if (filt->what == HANDLE_END_FILTER && filt->filterTarget == result->dst) {
			FilterOperator *op = filt->filterOp;
			(*op)(result);
		}
	}

	return result;
}


std::pair<const Handle, message_t*> ZeroMQCore::receiveAny (federate_id_t federateID)
{
	std::lock_guard<std::mutex> lock(_federates[federateID]->_mutex);

    assert (isInitialized ());
	assert (federateID < _federates.size ());

	std::pair<const Handle, std::deque<message_t*>> *earliest_msg = nullptr;

	// Find the end point with the earliest message time
	for (auto &end_point : _federates[federateID]->message_queue) {
		if (end_point.second.size() > 0) {
			if (earliest_msg == nullptr) {
				earliest_msg = &end_point;
			}
			else {
				if (earliest_msg->second.front()->time < end_point.second.front()->time) {
					earliest_msg = &end_point;
				}
			}
		}
	}

	// Return the message found and remove from the queue
	if (earliest_msg != nullptr) {
		message_t *result = earliest_msg->second.front();
		earliest_msg->second.pop_front();

		// Handle any destination endpoint filters
		for (auto filt : _handles) {
			if (filt->what == HANDLE_END_FILTER && filt->filterTarget == result->dst) {
				FilterOperator *op = filt->filterOp;
				(*op)(result);
			}
		}

		return{ earliest_msg->first,result };
	}
	else {
		return{ 0xFFFFFFFF,nullptr };
	}
}


uint64_t ZeroMQCore::receiveCountAny (federate_id_t federateID)
{
	std::lock_guard<std::mutex> lock(_federates[federateID]->_mutex);

    assert (isInitialized ());
	assert (federateID < _federates.size ());

	uint64_t count = 0;

	for (auto const &end_point : _federates[federateID]->message_queue) {
		count += end_point.second.size();
	}

    return count;
}

void ZeroMQCore::logMessage (federate_id_t federateID, int logCode, const char *logMessage) {}


void ZeroMQCore::setFilterOperator(Handle filter, FilterOperator* callback)
{
	std::lock_guard<std::mutex> lock(_mutex);

	assert(isInitialized());

	// make sure callback is a valid pointer
	if (callback == nullptr) {
		// either error, or unset the filter operator?
		return;
	}

	_handles[filter]->filterOp = callback;
}

std::string to_string(const Core::FederateInfo &info)
{
    const char delim = '#';
    std::ostringstream oss;
    oss << info.timeDelta.getBaseTimeCode() << delim;
    oss << info.lookAhead.getBaseTimeCode() << delim;
    oss << info.impactWindow.getBaseTimeCode() << delim;
    oss << info.observer << delim;
    oss << info.uninteruptible << delim;
    oss << info.time_agnostic << delim;
    oss << info.source_only << delim;
    oss << info.filter_only << delim;
    return oss.str();
}

static Time::baseType to_time(const std::string &raw)
{
    Time::baseType time_base;
    std::istringstream iss (raw);
    iss >> time_base;
    return time_base;
}

static bool to_bool(const std::string &raw)
{
    bool ret;
    std::istringstream iss (raw);
    iss >> ret;
    return ret;
}

Core::FederateInfo to_info(const std::string &raw)
{
    const char delim = '#';
    Core::FederateInfo info;
    Time::baseType time_base;
    std::string chunk;
    std::istringstream iss (raw);

    std::getline (iss, chunk, delim);
    info.timeDelta.setBaseTimeCode (to_time(chunk));
    std::getline (iss, chunk, delim);
    info.lookAhead.setBaseTimeCode(to_time(chunk));
    std::getline (iss, chunk, delim);
    info.impactWindow.setBaseTimeCode(to_time(chunk));
    std::getline (iss, chunk, delim);
    info.observer = to_bool(chunk);
    std::getline (iss, chunk, delim);
    info.uninteruptible = to_bool(chunk);
    std::getline (iss, chunk, delim);
    info.time_agnostic = to_bool(chunk);
    std::getline (iss, chunk, delim);
    info.source_only = to_bool(chunk);
    std::getline (iss, chunk, delim);
    info.filter_only = to_bool(chunk);

    return info;
}

uint64_t ZeroMQCore::receiveFilterCount(federate_id_t federateID)
{
	return 0;
}

std::pair<const Handle, message_t*> ZeroMQCore::receiveAnyFilter(federate_id_t federateID)
{
	return{ 0xFFFFFFFF, nullptr };
}

}  // namespace helics
