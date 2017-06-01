/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#include "helics/core/test-core.h"
#include "helics-time.h"
#include "helics/config.h"
#include "helics/common/blocking_queue.h"
#include "helics/core/core.h"
#include "helics/core/core-data.h"

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

namespace helics
{
using federate_id_t= Core::federate_id_t;
using Handle= Core::Handle;
using FederateInfo= Core::FederateInfo;

enum TestHandleType {
	HANDLE_PUB,
	HANDLE_SUB,
	HANDLE_END,
	HANDLE_SRC_FILTER,
	HANDLE_DST_FILTER,
};

class TestHandle
{
  public:
    TestHandle (federate_id_t fed_id_,
                TestHandleType what_,
                const char *key_,
                const char *type_,
                const char *units_,
                bool required_ = false,
                bool is_endpoint_ = false)
        : id (0),
          fed_id (fed_id_),
          what (what_),
          key (key_),
          type (type_),
          units (units_),
          required (required_),
          is_endpoint (is_endpoint_),
          handles()
    {
    }

    ~TestHandle () {}

    Handle id;
    federate_id_t fed_id;
    TestHandleType what;
    std::string key;
    std::string type;
    std::string units;
    bool required;
    std::string data;
    bool is_endpoint;
    std::vector<Handle> handles;
    bool has_update=false;
	FilterOperator *filterOp = nullptr;
	std::string filterTarget;

    typedef std::map<std::string, TestHandle *>::iterator iter;
};

class FederateState
{
  public:
    FederateState (const char *name_, const FederateInfo &info_)
        : name (name_), info (info_), state (HELICS_CREATED)

    {
    }

    std::string name;
    const FederateInfo &info;
    unsigned int id = (unsigned int)(-1); //!< id code, default to something invalid
    helics_federate_state_type state = HELICS_NONE;
    std::map<std::string, TestHandle *> subs;
    std::map<std::string, TestHandle *> pubs;
    std::map<std::string, TestHandle *> ends;
	std::map<std::string, TestHandle *> filters;
    BlockingQueue<std::string> queue;
    bool iterative = false;
    bool converged_requested = true;
	bool processing = false;
	bool messages_pending = false;
    Time time_requested = timeZero;
    Time time_delta=1.0;
    Time time_last_processed=timeZero;
	Time time_look_ahead = timeZero;
	Time time_impact = timeZero;
	std::uint64_t max_iterations = 1;
    std::vector<Handle> events;
	std::map<Handle, std::deque<message_t *>> filter_queue;
	std::map<Handle, std::deque<message_t *>> message_queue;
	std::mutex _mutex;

    /** DISABLE_COPY_AND_ASSIGN */
  private:
    FederateState (const FederateState &) = delete;
    FederateState &operator= (const FederateState &) = delete;
};

#define CMD_CONNECT "HELLO"
#define CMD_INIT "INIT"
#define CMD_EXEC "EXECUTE"
#define CMD_STOP "STOP"
#define CMD_TIME "TIME"
#define CMD_TIME_IT "TIME_IT"
#define CMD_SIZE "SIZE"
#define CMD_ACK "ACK"
#define CMD_PUB "PUB"
#define CMD_BYE "BYE"

void broker (TestCore *core)
{
    //unsigned int hello_count = 0;
    unsigned int init_count = 0;
    unsigned int execute_count = 0;

    while (true)
    {
        std::string chunk;
        std::string command = core->_queue.pop ();
        std::istringstream iss (command);

        LOG (INFO) << "\"\"\"" << command << std::endl << "\"\"\"" << ENDL;

        std::getline (iss, chunk);
        if (CMD_STOP == chunk)
        {
            break;
        }
        else if (CMD_SIZE == chunk)
        {
            unsigned int size;
            std::getline (iss, chunk);
            size = std::stoul (chunk, nullptr, 10);
            std::unique_lock<std::mutex> lock (core->_mutex);
            if (0 == core->_max_federates)
            {
                core->_max_federates = size;
            }
            else
            {
                assert (size == core->_max_federates);
            }
        }
#if 0
        else if (CMD_CONNECT == chunk)
        {
            ++hello_count;
            std::unique_lock<std::mutex> lock (core->_mutex);
            assert (hello_count <= core->_max_federates);
            if (hello_count == core->_max_federates)
            {
                for (size_t i = 0; i < core->_federates.size (); ++i)
                {
                    std::ostringstream oss;
                    oss << CMD_ACK << std::endl;
                    core->_federates[i]->queue.push (oss.str ());
                }
            }
        }
#endif
        else if (CMD_INIT == chunk)
        {
            ++init_count;
            std::unique_lock<std::mutex> lock (core->_mutex);
            if (init_count == core->_max_federates)
            {
                // pub/sub checks
                LOG (INFO) << "performing pub/sub check" << ENDL;
                for (size_t i = 0; i < core->_handles.size(); ++i)
				{
                    TestHandle *pub = core->_handles[i];
                    LOG (INFO) << "handle " << i << " is " << pub->what << ENDL;
                    if (pub->what == HANDLE_PUB)
					{
                        LOG (INFO) << "  is a pub" << ENDL;
                        for (size_t j = 0; j < core->_handles.size(); ++j)
						{
                            TestHandle *sub = core->_handles[j];
                            if (sub->what == HANDLE_SUB)
							{
                                LOG (INFO) << "    " << j << " is a sub" << ENDL;
                                if (pub->key == sub->key)
								{
                                    pub->handles.push_back(sub->id);
                                    LOG (INFO) << "    " << j << " is a sub and matches" << ENDL;
                                }
                                else
								{
                                    LOG (INFO) << "    " << j << " is a sub -- no match" << ENDL;
                                }
                            }
							// pub duplicate name check (ambiguity for what pub values should come from)
							else if (sub->what == HANDLE_PUB)
							{
								if (pub->key == sub->key && i != j)
								{
									LOG (WARN) << "    " << j << " is a pub with the same name" << ENDL;
								}
							}
                        }
                        if (pub->handles.empty())
						{
                            LOG (INFO) << "nobody cares about " << pub->key << ENDL;
                        }
                    }
                }
				// check if every sub with required pub has a match
				for (size_t i = 0; i < core->_handles.size(); ++i)
				{
					TestHandle *sub = core->_handles[i];
					if (sub->what == HANDLE_SUB && sub->required)
					{
						bool hasMatch = false;
						// look through list of subs for each pub
						for (size_t j = 0; j < core->_handles.size(); ++j)
						{
							TestHandle *pub = core->_handles[j];
							if (pub->what == HANDLE_PUB)
							{
								for (auto handle : pub->handles)
								{
									if (sub->id == handle)
									{
										hasMatch = true;
									}
								}
							}
						}
						if (!hasMatch)
						{
							LOG (WARN) << "sub " << i << " has no corresponding pub" << ENDL;
						}
					}
				}
				// duplicate endpoint name check
				for (size_t i = 0; i < core->_handles.size(); ++i)
				{
					TestHandle *endp1 = core->_handles[i];
					if (endp1->what == HANDLE_END)
					{
						for (size_t j = 0; j < core->_handles.size(); ++j)
						{
							TestHandle *endp2 = core->_handles[j];
							if (endp2->what == HANDLE_END)
							{
								if (endp1->key == endp2->key && i != j)
								{
									LOG (WARN) << "handle " << i << " has the same name as endpoint " << j << ENDL;
								}
							}
						}
					}
				}

				// process registered filters
				for (size_t i = 0; i < core->_handles.size(); ++i)
				{
					TestHandle *endp = core->_handles[i];
					if (endp->what == HANDLE_END)
					{
						// add filters to the handles array of the target endpoint
						for (size_t j = 0; j < core->_handles.size(); ++j)
						{
							TestHandle *filter = core->_handles[j];
							if (filter->what == HANDLE_SRC_FILTER || filter->what == HANDLE_DST_FILTER)
							{
								if (filter->filterTarget == endp->key && i != j)
								{
									endp->handles.push_back(filter->id);
								}
							}
						}

						// filter checks
						if (endp->handles.size() > 1)
						{
							bool hasTimeSrcFilter = false;
							for (auto filter_id : endp->handles)
							{
								auto filter = core->_handles[filter_id];
								auto fed = core->_federates[filter->fed_id];

								// check for multiple non-callback source filters, and non-callback destination filters
								if (!fed->info.time_agnostic)
								{
									switch (filter->what) {
									case HANDLE_SRC_FILTER:
										if (hasTimeSrcFilter)
										{
											LOG(WARN) << "filter " << filter_id << " added to endpoint " << endp->id << " already contains non-callback source filter" << ENDL;
										}
										hasTimeSrcFilter = true;
										break;
									case HANDLE_DST_FILTER:
										LOG(WARN) << "filter " << filter_id << " added to endpoint " << endp->id << " contains non-callback destination filter" << ENDL;
										break;
									}
								}

								// check if callbacks were set for time agnostic filters
								else
								{
									if (filter->filterOp == nullptr)
									{
										LOG(WARN) << "filter " << filter_id << " is time agnostic without a callback set" << ENDL;
									}
								}
							}
						}
					}
				}
                for (size_t i = 0; i < core->_federates.size (); ++i)
                {
                    std::ostringstream oss;
                    oss << CMD_ACK << std::endl;
                    core->_federates[i]->queue.push (oss.str ());
                }
            }
        }
        else if (CMD_EXEC == chunk)
        {
            ++execute_count;
            std::lock_guard<std::mutex> lock (core->_mutex);
            if (execute_count == core->_max_federates)
            {
                core->_n_processing = core->_max_federates;
                for (size_t i = 0; i < core->_federates.size (); ++i)
                {
                    std::ostringstream oss;
                    oss << CMD_ACK << std::endl;
                    core->_federates[i]->queue.push (oss.str ());
                }
            }
        }
        else if (CMD_TIME == chunk || CMD_TIME_IT == chunk || CMD_BYE == chunk)
        {
            bool was_iter = (CMD_TIME_IT == chunk);
            bool was_bye = (CMD_BYE == chunk);
            federate_id_t id;
            Time time_requested;
            Time::baseType time_base;
            bool converged_requested = true;
            std::getline (iss, chunk);
            id = std::stoul (chunk, nullptr, 10);
            std::getline (iss, chunk);
            LOG (INFO) << "TIME WAS '" << chunk << "'" << ENDL;
            {
                std::istringstream iss_ (chunk);
                iss_ >> time_base;
            }
            time_requested.setBaseTimeCode (time_base);
            if (was_iter)
            {
                std::getline (iss, chunk);
                std::istringstream iss_ (chunk);
                iss_ >> converged_requested;
            }
            std::unique_lock<std::mutex> lock (core->_mutex);
            if (was_bye)
            {
                core->_federates[id]->state = HELICS_NONE;
                core->_federates[id]->iterative = false;
                core->_federates[id]->converged_requested = true;
                core->_federates[id]->time_requested = Time::maxVal ();
                core->_federates[id]->time_last_processed = core->_time_granted;
                core->_federates[id]->processing = false;
                core->_federates[id]->events.clear();
                --core->_n_processing;
                ++core->_n_byes;
                if (core->_n_byes == core->_max_federates)
                {
#if 0
                    for (size_t i = 0; i < core->_max_federates; ++i)
                    {
                        std::ostringstream oss;
                        oss << CMD_ACK << std::endl;
                        core->_federates[i]->queue.push (oss.str ());
                    }
#endif
                    break;
                }
            }
            else
            {
                core->_federates[id]->iterative = was_iter;
                core->_federates[id]->converged_requested = converged_requested;
                core->_federates[id]->time_requested = time_requested;
                core->_federates[id]->time_last_processed = core->_time_granted;
                core->_federates[id]->processing = false;
                core->_federates[id]->events.clear();
                --core->_n_processing;
            }

            /* if all sims are done, determine next time step */
            if (0 == core->_n_processing)
            {
                std::vector<Time> time_actionable (core->_max_federates);
                for (size_t i = 0; i < core->_max_federates; ++i)
                {
                    if (core->_federates[i]->messages_pending)
                    {
                        time_actionable[i] =
                          core->_federates[i]->time_last_processed + core->_federates[i]->time_delta;
                    }
                    else
                    {
                        time_actionable[i] = core->_federates[i]->time_requested;

						// Check for incoming messages that could affect the required next action time
						for (auto msgQ : core->_federates[i]->message_queue)
						{
							if (!msgQ.second.empty())
							{
								// adjust actionable time if there are any messages that come in earlier
								if (msgQ.second.at(0)->time < time_actionable[i])
								{
									// max[1,ceil[(msgTime - lastProcessedTime) / timeDelta]]*timeDelta + lastProcessedTime
									time_actionable[i] = fmax(1, ceil((msgQ.second.at(0)->time - core->_federates[i]->time_last_processed) / core->_federates[i]->time_delta)) * core->_federates[i]->time_delta + core->_federates[i]->time_last_processed;
								}
							}
						}
                    }
                }
                Time maybe_time_granted = *std::min_element (time_actionable.begin (), time_actionable.end ());
                LOG (INFO) << "maybe_time_granted = " << maybe_time_granted << ENDL;
                /* check for convergence of the sims that agree with the
                 * actionable time */
                bool coverged_all = true;
                for (size_t i = 0; i < core->_max_federates; ++i)
                {
                    LOG (INFO) << "[" << i << "]: ";
                    if (maybe_time_granted == time_actionable[i])
                    {
                        LOG (INFO) << "YES " << core->_federates[i]->converged_requested << ENDL;
                        coverged_all = coverged_all && core->_federates[i]->converged_requested;
                    }
                    else
                    {
                        LOG (INFO) << "NO " << ENDL;
                    }
                }
                if (coverged_all)
                {
                    LOG (INFO) << "CONVERGED" << ENDL;
                    /* process pending value updates */
                    for (size_t i = 0; i < core->_pending_values.size (); ++i)
                    {
                        Handle pending = core->_pending_values[i];
                        TestHandle *pub = core->_handles[pending];
                        assert(nullptr != pub);
                        assert(pub->what == HANDLE_PUB);
                        if (pub->has_update)
						{
                            pub->has_update = false;
                            LOG (INFO) << "PENDING " << pending << " updating " << pub->handles.size() << " subs" << ENDL;
                            for (size_t j = 0; j < pub->handles.size(); ++j)
							{
                                auto subhand = pub->handles[j];
                                TestHandle *sub = core->_handles[subhand];
                                assert(nullptr != sub);
                                assert(sub->what == HANDLE_SUB);
                                sub->data = pub->data;
                                core->_federates[sub->fed_id]->events.push_back (sub->id);
                                LOG(INFO) << "  " << subhand << ENDL;
                            }
                        }
                        else
						{
                            LOG(INFO) << "PENDING " << pending << " already handled" << ENDL;
                        }
                    }
                    core->_pending_values.clear ();
                    /* send ack to waiting sims */
                    core->_time_granted = maybe_time_granted;
                    core->_iter = 0;
                    for (size_t i = 0; i < core->_max_federates; ++i)
                    {
                        if (core->_time_granted == time_actionable[i])
                        {
                            if (core->_federates[i]->state == HELICS_EXECUTING)
                            {
                                LOG (INFO)
                                  << "granting " << core->_time_granted << " to " << core->_federates[i]->name << ENDL;
                                ++core->_n_processing;
                                core->_federates[i]->processing = true;
                                core->_federates[i]->messages_pending = false;
                                std::ostringstream oss;
                                oss << CMD_ACK << std::endl << true << std::endl;
                                core->_federates[i]->queue.push (oss.str ());
                            }
                        }
                        else
                        {
#if 0
              /* this code was causing an ambiguous call */
              /* fast forward time last processed */
              Time jump = (core->_time_granted - core->_federates[i]->time_last_processed) / core->_federates[i]->time_delta;
              core->_federates[i]->time_last_processed += core->_federates[i]->time_delta * jump;
#else
                            while (core->_federates[i]->time_last_processed + core->_federates[i]->time_delta <=
                                   core->_time_granted)
                            {
                                core->_federates[i]->time_last_processed += core->_federates[i]->time_delta;
                            }
#endif
                        }
                    }
                }
                else
                {
                    LOG (INFO) << "DID NOT CONVERGE" << ENDL;
                    ++core->_iter;
                    assert (core->_iter < core->_max_iterations);
                    for (size_t i = 0; i < core->_max_federates; ++i)
                    {
                        if (maybe_time_granted == time_actionable[i] && core->_federates[i]->iterative)
                        {
                            ++core->_n_processing;
                            core->_federates[i]->processing = true;
                            core->_federates[i]->messages_pending = false;
                            std::ostringstream oss;
                            oss << CMD_ACK << std::endl << false << std::endl;
                            core->_federates[i]->queue.push (oss.str ());
                        }
                    }
                }
            }
        }
        else if (CMD_PUB == chunk)
        {
            Handle handle;
            uint64_t len;
            std::getline (iss, chunk);
            handle = std::stoull (chunk, nullptr, 10);
            std::getline (iss, chunk);
            len = std::stoull (chunk, nullptr, 10);
            std::string data(len, '\0');
            for (uint64_t i = 0; i < len; ++i)
			{
                data[i] = iss.get();
            }
            LOG (INFO) << "GOT PUB:" << ENDL;
            LOG (INFO) << "'" << data << "'" << ENDL;
            LOG (INFO) << "END PUB:" << ENDL;
			std::lock_guard<std::mutex> lock (core->_mutex);
            TestHandle *pub = core->_handles[handle];
            assert(pub->what == HANDLE_PUB);
            pub->data = data;
            pub->has_update = true;
            core->_pending_values.push_back (handle);
            LOG (INFO) << "  will process " << pub->handles.size() << " updates" << ENDL;
            for (size_t i = 0; i < pub->handles.size(); ++i)
			{
                TestHandle *sub = core->_handles[pub->handles[i]];
                assert(nullptr != sub);
                assert(sub->what == HANDLE_SUB);
                core->_federates[sub->fed_id]->messages_pending = true;
            }
        }
    }
}


void TestCore::initialize (const char *initializationString)
{
	std::lock_guard<std::mutex> lock (_mutex);

    if (strlen (initializationString) > 0)
    {
        int maxFed = atoi (initializationString);
        _max_federates = maxFed;
    }
    else
    {
        _max_federates = 0;
    }
    assert(_max_federates > 0);

    _initialized = true;
    _federates.clear ();
    _n_processing = 0;
    _n_byes = 0;
    _time_granted = 0;
    _iter = 0;
    _max_iterations = 100;
    _thread_broker = std::thread (broker, this);
}


TestCore::~TestCore ()
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


bool TestCore::isInitialized () { return _initialized; }


void TestCore::error (federate_id_t federateID, int /*errorCode*/)
{
	std::lock_guard<std::mutex> lock(_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());

    _federates[federateID]->state = HELICS_FAILURE;
}


void TestCore::finalize (federate_id_t federateID)
{
	std::unique_lock<std::mutex> lock(_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());

    std::ostringstream oss;
    oss << CMD_BYE << std::endl << federateID << std::endl << 0UL << std::endl;

    _federates[federateID]->state = HELICS_NONE;

    lock.unlock();

    _queue.push (oss.str ());

#if 0
    std::string chunk;
    std::string command = _federates[federateID]->queue.pop ();
    std::istringstream iss (command);
    std::getline (iss, chunk);
    assert (CMD_ACK == chunk);
#endif
}


void TestCore::enterInitializingState (federate_id_t federateID)
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
    oss << CMD_INIT << std::endl;
    _queue.push (oss.str ());

    std::string chunk;
    std::string command = fed_queue.pop ();
    std::istringstream iss (command);
    std::getline (iss, chunk);
    assert (CMD_ACK == chunk);

    lock.lock();

    _federates[federateID]->state = HELICS_INITIALIZING;
}


bool TestCore::enterExecutingState(federate_id_t federateID, bool iterationCompleted)
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
    oss << CMD_EXEC << std::endl;
    _queue.push (oss.str ());

    std::string chunk;
    std::string command = fed_queue.pop ();
    std::istringstream iss (command);
    std::getline (iss, chunk);
    assert (CMD_ACK == chunk);

    lock.lock();

    _federates[federateID]->state = HELICS_EXECUTING;
    _federates[federateID]->processing = true;

	return true;
}


federate_id_t TestCore::registerFederate (const char *name, const FederateInfo &info)
{
	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());

    FederateState *me = nullptr;

    me = new FederateState (name, info);
    me->id = static_cast<decltype (me->id)> (_federates.size ());
    _federates.push_back (me);

#if 0
    std::ostringstream oss;
    oss << CMD_CONNECT << std::endl;
    _queue.push (oss.str ());

    std::string chunk;
    std::string command = me->queue.pop ();
    std::istringstream iss (command);
    std::getline (iss, chunk);
    assert (CMD_ACK == chunk);
#endif

    return me->id;
}


const char *TestCore::getFederateName (federate_id_t federateID)
{
	std::lock_guard<std::mutex> lock (_federates[federateID]->_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());

    return _federates[federateID]->name.c_str();
}


federate_id_t TestCore::getFederateId (const char *name)
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


void TestCore::setFederationSize (unsigned int size)
{
    std::unique_lock<std::mutex> lock (_mutex);

    assert (isInitialized ());

    lock.unlock();

    std::ostringstream oss;
    oss << CMD_SIZE << std::endl << size << std::endl;
    _queue.push (oss.str ());
}


unsigned int TestCore::getFederationSize ()
{
	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());

    return _max_federates;
}


Time TestCore::timeRequest (federate_id_t federateID, Time next)
{
    std::unique_lock<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());
    assert (HELICS_EXECUTING == _federates[federateID]->state);

    // hold on the the queue so we can release the outer lock
    BlockingQueue<std::string> &fed_queue = _federates[federateID]->queue;

    lock.unlock();

    //if (1 == getFederationSize ())
    //{
    //    _time_granted = next;
    //    return next;
    //}
    //else
   // {
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
   // }
}


std::pair<Time, bool> TestCore::requestTimeIterative (federate_id_t federateID, Time next, bool localConverged)
{
    std::unique_lock<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());
    assert (HELICS_EXECUTING == _federates[federateID]->state);

    // hold on the the queue so we can release the outer lock
    BlockingQueue<std::string> &fed_queue = _federates[federateID]->queue;

    lock.unlock();

    /*if (1 == getFederationSize ())
    {
        if (localConverged)
        {
            _time_granted = next;
            _iter = 0;
            return std::make_pair (next, true);
        }
        else
        {
            ++_iter;
            assert (_iter < _max_iterations);
            return std::make_pair (_time_granted, false);
        }
    }
    else
    { */
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
    //}
}


uint64_t TestCore::getCurrentReiteration (federate_id_t federateID)
{
    std::unique_lock<std::mutex> lock (_mutex);

    assert (isInitialized ());
    return _iter;
}


void TestCore::setMaximumIterations (federate_id_t federateID, uint64_t iterations)
{
    std::lock_guard<std::mutex> lock(_mutex);

    assert (isInitialized ());
	assert(federateID < _federates.size());

	_federates[federateID]->max_iterations = iterations;
    _max_iterations = iterations;
}


void TestCore::setTimeDelta (federate_id_t federateID, Time time)
{
	std::lock_guard<std::mutex> lock(_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());

    _federates[federateID]->time_delta = time;
}


void TestCore::setLookAhead(federate_id_t federateID, Time lookAheadTime)
{
	std::lock_guard<std::mutex> lock(_mutex);
	assert(isInitialized());
	assert(federateID < _federates.size());

	_federates[federateID]->time_look_ahead = lookAheadTime;
}

void TestCore::setImpactWindow(federate_id_t federateID, Time impactTime)
{
	std::lock_guard<std::mutex> lock(_mutex);
	assert(isInitialized());
	assert(federateID < _federates.size());

	_federates[federateID]->time_impact = impactTime;
}

Handle TestCore::registerSubscription (federate_id_t federateID,
                                       const char *key,
                                       const char *type,
                                       const char *units,
                                       bool required)
{
    LOG (INFO) << "registering SUB " << key << ENDL;

    std::unique_lock<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());
    assert (_federates[federateID]->state == HELICS_CREATED);

    TestHandle *handle = new TestHandle (federateID, HANDLE_SUB, key, type, units, required);
    handle->id = static_cast<decltype (handle->id)> (_handles.size ());
    _handles.push_back (handle);
    _federates[federateID]->subs.emplace (key, handle);
    return handle->id;
}


Handle TestCore::getSubscription (federate_id_t federateID, const char *key)
{
    std::unique_lock<std::mutex> lock (_federates[federateID]->_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());

    auto it = _federates[federateID]->subs.find (key);
    assert(it != _federates[federateID]->subs.end());

    return it->second->id;
}


Handle TestCore::registerPublication (federate_id_t federateID, const char *key, const char *type, const char *units)
{
    LOG (INFO) << "registering PUB " << key << ENDL;
	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());
    assert (_federates[federateID]->state == HELICS_CREATED);

    TestHandle *handle = new TestHandle (federateID, HANDLE_PUB, key, type, units);
    handle->id = static_cast<decltype (handle->id)> (_handles.size ());
    _handles.push_back (handle);
    _federates[federateID]->pubs.emplace (key, handle);
    return handle->id;
}


Handle TestCore::getPublication (federate_id_t federateID, const char *key)
{
	std::lock_guard<std::mutex> lock (_federates[federateID]->_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());

    auto it = _federates[federateID]->pubs.find (key);
    assert(it != _federates[federateID]->pubs.end());

    return it->second->id;
}


const char *TestCore::getUnits (Handle handle)
{
	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (handle < _handles.size ());

    return _handles[handle]->units.c_str();
}


const char *TestCore::getType (Handle handle)
{
	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (handle < _handles.size ());

    return _handles[handle]->type.c_str();
}


void TestCore::setValue (Handle handle_, const char *data, uint64_t len)
{
    assert (isInitialized ());
    assert (handle_ < _handles.size ());
    assert (_handles[handle_]->what == HANDLE_PUB);

    LOG (INFO) << "setValue: '" << std::string(data,len) << "'" << ENDL;

    // we write the data byte-wise in case our
    // getline delimiter is part of the data
    std::ostringstream oss;
    oss << CMD_PUB << std::endl
      << handle_ << std::endl
      << len << std::endl;
    for (uint64_t i = 0; i < len; ++i) {
      oss << data[i];
    }

    _queue.push (oss.str ());
}


data_t *TestCore::getValue (Handle handle_)
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


void TestCore::dereference (data_t *data)
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

void TestCore::dereference(message_t *msg)
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


const Handle *TestCore::getValueUpdates (federate_id_t federateID, uint64_t *size)
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


Handle TestCore::registerEndpoint (federate_id_t federateID, const char *name, const char *type)
{
	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());

    TestHandle *handle = new TestHandle (federateID, HANDLE_END, name, type, "", false, true);
    handle->id = static_cast<decltype (handle->id)> (_handles.size ());
    _handles.push_back (handle);
    _federates[federateID]->ends.emplace (name, handle);
    return handle->id;
}


Handle TestCore::registerSourceFilter (federate_id_t federateID,
                                       const char *filterName,
                                       const char *source,
                                       const char *type_in)
{
	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());

	TestHandle *handle = new TestHandle(federateID, HANDLE_SRC_FILTER, filterName, type_in, "");
	handle->id = static_cast<decltype (handle->id)> (_handles.size());
	handle->filterTarget = source;
	_handles.push_back(handle);
	_federates[federateID]->filters.emplace(filterName, handle);
	return handle->id;
}


Handle TestCore::registerDestinationFilter (federate_id_t federateID,
                                            const char *filterName,
                                            const char *dest,
                                            const char *type_in)
{
	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (federateID < _federates.size ());

	TestHandle *handle = new TestHandle(federateID, HANDLE_DST_FILTER, filterName, type_in, "");
	handle->id = static_cast<decltype (handle->id)> (_handles.size());
	handle->filterTarget = dest;
	_handles.push_back(handle);
	_federates[federateID]->filters.emplace(filterName, handle);
	return handle->id;
}


void TestCore::registerFrequentCommunicationsPair (const char *source, const char *dest)
{
	std::lock_guard<std::mutex> lock (_mutex);

    assert (isInitialized ());
    assert (false);
}

void TestCore::addDependency(federate_id_t federateId, const char *federateName)
{

}

void TestCore::send (Handle sourceHandle, const char *destination, const char *data, uint64_t length)
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

	for (int i = 0; i < static_cast<int>(length); i++)
	{
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


void TestCore::sendEvent (Time time,
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

	for (int i = 0; i < static_cast<int>(length); i++)
	{
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


void TestCore::sendMessage (message_t *message)
{
    assert (isInitialized ());
	assert(message != nullptr);

	message_t *msgCopy = new message_t();

	char *origSrcCopy = new char[strlen(message->origsrc) + 1];
	char *srcCopy = new char[strlen(message->src) + 1];
	char *dstCopy = new char[strlen(message->dst) + 1];
	char *dataCopy = new char[message->len];

	strcpy(origSrcCopy, message->origsrc);
	strcpy(srcCopy, message->src);
	strcpy(dstCopy, message->dst);

	for (int i = 0; i < static_cast<int>(message->len); i++)
	{
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


void TestCore::queueMessage(message_t *message)
{
	assert(isInitialized());
	assert(message != nullptr);

	TestHandle *src = nullptr;
	TestHandle *dest = nullptr;
	federate_id_t fed_id=0xFFFF'FFFF;

	// Find the source and destination endpoint
	for (auto fed : _federates)
	{
		if ((src == nullptr) && (fed->ends.count(message->origsrc) > 0))
		{
			src = fed->ends[message->src];
		}
		if ((dest == nullptr) && (fed->ends.count(message->dst) > 0))
		{
			fed_id = fed->id;
			dest = fed->ends[message->dst];
		}
		if ((src != nullptr) && (dest != nullptr))
		{
			break;
		}
	}

	//make sure it was found
	if ((dest == nullptr)||(fed_id==0xFFFF'FFFF))
	{
		//THROW something here?  
		return;
	}

	// Handle source endpoint filters if the source is the original (and not a filter)
	if (message->origsrc == message->src)
	{
		for (auto filter_id : src->handles)
		{
			auto filter = _handles[filter_id];
			if (filter->what == HANDLE_SRC_FILTER)
			{
				auto fed = _federates[filter->fed_id];
				if (fed->info.time_agnostic && filter->filterOp != nullptr)
				{
					FilterOperator *op = filter->filterOp;
					(*op)(message);
				}
			}
		}
		// Pass along a message to a non-callback source filter, if there is one
		for (auto filter_id : src->handles)
		{
			auto filter = _handles[filter_id];
			if (filter->what == HANDLE_SRC_FILTER)
			{
				auto fed = _federates[filter->fed_id];
				if (!fed->info.time_agnostic)
				{
					auto filterQ = &fed->filter_queue[filter_id];
					if (filterQ->empty())
					{
						filterQ->push_back(message);
					}
					else
					{
						size_t i = 0;
						while (i < filterQ->size() && (filterQ->at(i)->time <= message->time)) i++;
						filterQ->insert(filterQ->begin() + i, message);
					}
					return;
				}
			}
		}
	}

	// Handle destination endpoint filters (time-agnostic only)
	for (auto filter_id : dest->handles)
	{
		auto filter = _handles[filter_id];
		if (filter->what == HANDLE_DST_FILTER)
		{
			auto fed = _federates[filter->fed_id];
			if (fed->info.time_agnostic && filter->filterOp != nullptr)
			{
				FilterOperator *op = filter->filterOp;
				(*op)(message);
			}
		}
	}

	// Get the destination endpoint message queue
	auto msgQ = &_federates[fed_id]->message_queue[dest->id];

	std::unique_lock<std::mutex> lock(_federates[fed_id]->_mutex);

	if (msgQ->empty())
	{
		msgQ->push_back(message);
	}
	else
	{
		size_t i = 0;

		// Order the messages from earliest time to latest time, with subordering by arrival time
		while (i < msgQ->size() && msgQ->at(i)->time <= message->time) i++;
		msgQ->insert(msgQ->begin() + i, message);
	}
}

uint64_t TestCore::receiveCount (Handle destination)
{
    assert (isInitialized ());
	assert (destination < _handles.size ());

	federate_id_t src_id = _handles[destination]->fed_id;

	std::unique_lock<std::mutex> lock(_federates[src_id]->_mutex);

	uint64_t count = 0;
	for (auto msg : _federates[src_id]->message_queue[destination])
	{
		// Count all messages up until current time
		if (msg->time > _time_granted)
		{
			break;
		}
		count++;
	}

	return count;
}

message_t *TestCore::receive (Handle destination)
{
    assert (isInitialized ());
	assert (destination < _handles.size ());

	federate_id_t fed_id = _handles[destination]->fed_id;

	auto end_point_msg_q = &_federates[fed_id]->message_queue[destination];

	std::unique_lock<std::mutex> lock(_federates[fed_id]->_mutex);

	if (end_point_msg_q->empty()) {
		return nullptr;
	}

	message_t *result = end_point_msg_q->front();

	// Return nothing if the message is scheduled for the future
	if (result->time > _time_granted)
	{
		return nullptr;
	}

	end_point_msg_q->pop_front();

	return result;
}


std::pair<const Handle, message_t*> TestCore::receiveAny (federate_id_t federateID)
{
	std::unique_lock<std::mutex> lock(_federates[federateID]->_mutex);

    assert (isInitialized ());
	assert (federateID < _federates.size ());

	std::pair<const Handle, std::deque<message_t*>> *earliest_msg = nullptr;

	// Find the end point with the earliest message time
	for (auto &end_point : _federates[federateID]->message_queue)
	{
		if (end_point.second.size() > 0)
		{
			if (end_point.second.at(0)->time <= _time_granted)
			{
				if (earliest_msg == nullptr)
				{
					earliest_msg = &end_point;
				}
				else
				{
					if (earliest_msg->second.front()->time < end_point.second.front()->time)
					{
						earliest_msg = &end_point;
					}
				}
			}
		}
	}

	// Return the message found and remove from the queue
	if (earliest_msg != nullptr)
	{
		message_t *result = earliest_msg->second.front();
		earliest_msg->second.pop_front();

		return{ earliest_msg->first,result };
	}
	else
	{
		return{ 0xFFFFFFFF,nullptr };
	}
}


uint64_t TestCore::receiveCountAny (federate_id_t federateID)
{
	std::unique_lock<std::mutex> lock(_federates[federateID]->_mutex);

    assert (isInitialized ());
	assert (federateID < _federates.size ());

	uint64_t count = 0;

	for (auto const &end_point : _federates[federateID]->message_queue)
	{
		for (auto msg : end_point.second)
		{
			if (msg->time > _time_granted)
			{
				break;
			}
			count++;
		}
	}

    return count;
}

void TestCore::logMessage (federate_id_t federateID, int logCode, const char *logMessage) {}


void TestCore::setFilterOperator(Handle filter, FilterOperator* callback)
{
	std::unique_lock<std::mutex> lock(_mutex);

	assert(isInitialized());

	// make sure callback is a valid pointer
	if (callback == nullptr)
	{
		// either error, or unset the filter operator?
		return;
	}

	_handles[filter]->filterOp = callback;
}

uint64_t TestCore::receiveFilterCount(federate_id_t federateID)
{
	std::unique_lock<std::mutex> lock(_federates[federateID]->_mutex);

	assert(isInitialized());
	assert(federateID < _federates.size());

	uint64_t count = 0;

	for (auto const &end_point : _federates[federateID]->filter_queue)
	{
		for (auto msg : end_point.second)
		{
			if (msg->time > _time_granted)
			{
				break;
			}
			count++;
		}
	}

	return count;
}

std::pair<const Handle, message_t*> TestCore::receiveAnyFilter(federate_id_t federateID)
{
	std::unique_lock<std::mutex> lock(_federates[federateID]->_mutex);

	assert(isInitialized());
	assert(federateID < _federates.size());

	std::pair<const Handle, std::deque<message_t*>> *earliest_msg = nullptr;

	// Find the end point with the earliest message time
	for (auto &end_point : _federates[federateID]->filter_queue)
	{
		if (end_point.second.size() > 0)
		{
			if (end_point.second.at(0)->time <= _time_granted)
			{
				if (earliest_msg == nullptr)
				{
					earliest_msg = &end_point;
				}
				else
				{
					if (earliest_msg->second.front()->time < end_point.second.front()->time)
					{
						earliest_msg = &end_point;
					}
				}
			}
		}
	}

	// Return the message found and remove from the queue
	if (earliest_msg != nullptr)
	{
		message_t *result = earliest_msg->second.front();
		earliest_msg->second.pop_front();

		return{ earliest_msg->first, result };
	}
	else
	{
		return{ 0xFFFFFFFF, nullptr };
	}
}

}  // namespace helics
