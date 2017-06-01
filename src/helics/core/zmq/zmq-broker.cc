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
#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <boost/lexical_cast.hpp>

#include <zmq.h>

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
#define LOG(LEVEL) std::ostringstream()
#define ENDL std::endl
#endif

using namespace helics;

using Handle=Core::Handle;
using FederateInfo = Core::FederateInfo;
using federate_id_t = Core::federate_id_t;

struct CoreInfo
{
    CoreInfo(std::string id_) : id(id_) { }

    std::string id;
    std::vector<ZeroMQHandle*> handles;
    std::vector<ZeroMQFederateState*> federates;
};

static void *zctx;
static void *zsock;

static void graceful_death(const std::vector<CoreInfo> &cores, bool die=true);

static void graceful_death(const std::vector<CoreInfo> &cores, bool die)
{
    int code=EXIT_SUCCESS;

    if (die) {
        /* send term message to all cores */
        for (unsigned int i=0; i<cores.size(); ++i) {
            zmqx_sendmore(zsock, cores[i].id);
            zmqx_send(zsock, CMD_DIE);
        }
    }

    /* clean up socket */
    if (zsock) {
        auto rc = zmq_close(zsock);
        if (-1 == rc) {
            perror("zmq_close(zsock)");
            code = EXIT_FAILURE;
        }
    }

    /* clean up context */
    if (zctx) {
        auto rc = zmq_ctx_destroy(zctx);
        if (-1 == rc) {
            perror("zmq_ctx_destroy");
            code = EXIT_FAILURE;
        }
    }

    exit(code);
}

int main(int argc, char **argv)
{
    const int ONE = 1;
    int rc;
    unsigned int max_cores = 0;
    std::vector<CoreInfo> cores;
    std::map<std::string,size_t> core_id_to_index;
    std::set<std::string> federate_names;
    std::vector<ZeroMQHandle*> handles;
    std::vector<std::pair<unsigned int,unsigned int> > handles_map;
    std::vector<ZeroMQFederateState*> federates;
    std::map<std::pair<size_t,size_t>, size_t> remote_fed_to_global_fed_map;
    std::vector<std::pair<unsigned int,unsigned int> > federates_map;
    std::vector<Core::Handle> pending_values;
    unsigned int n_registered = 0;
    unsigned int n_initialized = 0;
    unsigned int n_exec = 0;
    unsigned int n_processing = 0;
    unsigned int n_byes = 0;
    Time time_granted = 0;
    uint64_t iter = 0;
    uint64_t max_iterations = 100;

    /* max cores given on command line? */
    if (argc == 2) {
        max_cores = boost::lexical_cast<size_t>(argv[1]);
        LOG(INFO) << "max cores given on cmd line as " << max_cores << ENDL;
    }
    else {
        LOG(ERROR) << "max cores not given" << ENDL;
        return EXIT_FAILURE;
    }

    zctx = zmq_ctx_new();
    assert (zctx);
    LOG(INFO) << "created ZMQ context" << ENDL;

    zsock = zmq_socket(zctx, ZMQ_ROUTER);
    assert(zsock);
    LOG(INFO) << "created ZMQ socket" << ENDL;

    /* broker should not silently drop packets that can't be sent out */
    rc = zmq_setsockopt(zsock, ZMQ_ROUTER_MANDATORY, &ONE, sizeof(int));
    assert(-1 != rc);
    LOG(INFO) << "set mandatory on ZMQ socket" << ENDL;

    /* bind broker to physical address */
    rc = zmq_bind(zsock, "tcp://*:5555");
    assert(-1 != rc);
    LOG(INFO) << "socket bound to port 5555" << ENDL;

    while (true) {
        std::string identity; // simulation identifier
        std::string command; // control field

        zmq_pollitem_t items[] = {
            { zsock, 0, ZMQ_POLLIN, 0 },
        };

        LOG(INFO) << "polling" << ENDL;
        rc = zmq_poll(items, 1, -1);
        LOG(INFO) << "zmq_poll returned" << ENDL;

        if (items[0].revents & ZMQ_POLLIN) {
            (void) zmqx_recv(zsock, identity);
            (void) zmqx_recv(zsock, command);
            LOG(INFO) << "identity='" << identity << "' command='" << command << "'" << ENDL;
			std::string chunk;
			std::istringstream iss (command);
            std::getline (iss, chunk);

            if (CMD_CONNECT == chunk) {
                core_id_to_index[identity] = cores.size();
                cores.push_back(CoreInfo(identity));

                /* don't send an ack back to all connected cores until
                 * we get a connect from all */
                if (cores.size() == max_cores) {
                    for (unsigned int i=0; i<cores.size(); ++i) {
                        zmqx_sendmore(zsock, cores[i].id);
                        zmqx_sendmore(zsock, CMD_CONNECT);
                        zmqx_send(zsock, cores.size());
                    }
                }
            }
            else if (CMD_DIE == chunk) {
                assert(false);
            }
            else if (CMD_REG_FED == chunk) {
                FederateInfo info;
                size_t index = core_id_to_index.at(identity);
                size_t remote_id;
                size_t id;
                std::string name;

                std::getline (iss, chunk);
                remote_id = boost::lexical_cast<size_t>(chunk);
                /* name must be globally unique */
                std::getline (iss, name);
                std::pair<std::set<std::string>::iterator,bool> ins =
                    federate_names.insert(name);
                if (ins.second) {
                    LOG(INFO) << "registering federate '" << name << "'" << ENDL;
                }
                else {
                    LOG(ERROR) << "federate '" << name << "' was already registered" << ENDL;
                    graceful_death(cores);
                }

                /* next chunk is a stringified FederateInfo */
                std::getline (iss, chunk);
                info = to_info(chunk);

                /* DEBUGGING ONLY */
                LOG(INFO) << "FederateInfo" << std::endl << to_string(info) << ENDL;
                
                /* create the new ZeroMQFederateState with given name and info */
                ZeroMQFederateState *fed = nullptr;
                fed = new ZeroMQFederateState(name.c_str(), info);
                id = cores[index].federates.size();
                assert(id == remote_id);
                fed->id = static_cast<decltype(fed->id)>(federates.size());
                cores[index].federates.push_back(fed);
                federates.push_back(fed);
                remote_fed_to_global_fed_map[std::make_pair(index,id)] = fed->id;
                federates_map.push_back (std::make_pair(index,id));
                ++n_registered;
            }
            else if (CMD_REG_PUB == chunk) {
                size_t index = core_id_to_index.at(identity);
                unsigned int remote_fed_id;
                unsigned int id;
                std::string key;
                std::string type;
                std::string units;

                std::getline (iss, chunk);
                remote_fed_id = boost::lexical_cast<unsigned int>(chunk);
                std::getline (iss, chunk);
                id = boost::lexical_cast<unsigned int>(chunk);
                std::getline (iss, key);
                std::getline (iss, type);
                std::getline (iss, units);

                size_t global_fed_id = remote_fed_to_global_fed_map[
                    std::make_pair(index,remote_fed_id)];
                ZeroMQHandle *handle = new ZeroMQHandle (global_fed_id, HANDLE_PUB, key.c_str(), type.c_str(), units.c_str());
                handle->id = static_cast<decltype (handle->id)> (handles.size());
                handles.push_back (handle);
                handles_map.push_back (std::make_pair(index,id));
                cores[index].handles.push_back (handle);
                cores[index].federates[remote_fed_id]->pubs.emplace (key, handle);
            }
            else if (CMD_REG_SUB == chunk) {
                size_t index = core_id_to_index.at(identity);
                unsigned int remote_fed_id;
                unsigned int id;
                std::string key;
                std::string type;
                std::string units;
                bool required;

                std::getline (iss, chunk);
                remote_fed_id = boost::lexical_cast<unsigned int>(chunk);
                std::getline (iss, chunk);
                id = boost::lexical_cast<unsigned int>(chunk);
                std::getline (iss, key);
                std::getline (iss, type);
                std::getline (iss, units);
                std::getline (iss, chunk);
                required = boost::lexical_cast<bool>(chunk);

                size_t global_fed_id = remote_fed_to_global_fed_map[
                    std::make_pair(index,remote_fed_id)];
                ZeroMQHandle *handle = new ZeroMQHandle (global_fed_id, HANDLE_SUB, key.c_str(), type.c_str(), units.c_str(), required);
                handle->id = static_cast<decltype (handle->id)> (handles.size());
                handles.push_back (handle);
                handles_map.push_back (std::make_pair(index,id));
                cores[index].handles.push_back (handle);
                cores[index].federates[remote_fed_id]->subs.emplace (key, handle);
            }
            else if (CMD_REG_END == chunk) {
                size_t index = core_id_to_index.at(identity);
                unsigned int remote_fed_id;
                unsigned int id;
                std::string name;
                std::string type;

                std::getline (iss, chunk);
                remote_fed_id = boost::lexical_cast<unsigned int>(chunk);
                std::getline (iss, chunk);
                id = boost::lexical_cast<unsigned int>(chunk);
                std::getline (iss, name);
                std::getline (iss, type);

                size_t global_fed_id = remote_fed_to_global_fed_map[
                    std::make_pair(index,remote_fed_id)];
                ZeroMQHandle *handle = new ZeroMQHandle (global_fed_id, HANDLE_END, name.c_str(), type.c_str(), "", false, true);
                handle->id = static_cast<decltype (handle->id)> (handles.size());
                handles.push_back (handle);
                handles_map.push_back (std::make_pair(index,id));
                cores[index].handles.push_back (handle);
                cores[index].federates[remote_fed_id]->ends.emplace (name, handle);
            }
            else if (CMD_REG_SRC == chunk) {
                size_t index = core_id_to_index.at(identity);
                unsigned int remote_fed_id;
                unsigned int id;
                std::string name;
                std::string source;
                std::string type;

                std::getline (iss, chunk);
                remote_fed_id = boost::lexical_cast<unsigned int>(chunk);
                std::getline (iss, chunk);
                id = boost::lexical_cast<unsigned int>(chunk);
                std::getline (iss, name);
                std::getline (iss, source);
                std::getline (iss, type);

                size_t global_fed_id = remote_fed_to_global_fed_map[
                    std::make_pair(index,remote_fed_id)];
                ZeroMQHandle *handle = new ZeroMQHandle (global_fed_id, HANDLE_SRC_FILTER, name.c_str(), type.c_str(), "");
                handle->id = static_cast<decltype (handle->id)> (handles.size());
                handle->filterTarget = source;
                handles.push_back (handle);
                handles_map.push_back (std::make_pair(index,id));
                cores[index].handles.push_back (handle);
                cores[index].federates[remote_fed_id]->filters.emplace (name, handle);
            }
            else if (CMD_REG_DST == chunk) {
                size_t index = core_id_to_index.at(identity);
                unsigned int remote_fed_id;
                unsigned int id;
                std::string name;
                std::string dest;
                std::string type;

                std::getline (iss, chunk);
                remote_fed_id = boost::lexical_cast<unsigned int>(chunk);
                std::getline (iss, chunk);
                id = boost::lexical_cast<unsigned int>(chunk);
                std::getline (iss, name);
                std::getline (iss, dest);
                std::getline (iss, type);

                size_t global_fed_id = remote_fed_to_global_fed_map[
                    std::make_pair(index,remote_fed_id)];
                ZeroMQHandle *handle = new ZeroMQHandle (global_fed_id, HANDLE_END_FILTER, name.c_str(), type.c_str(), "");
                handle->id = static_cast<decltype (handle->id)> (handles.size());
                handle->filterTarget = dest;
                handles.push_back (handle);
                handles_map.push_back (std::make_pair(index,id));
                cores[index].handles.push_back (handle);
                cores[index].federates[remote_fed_id]->filters.emplace (name, handle);
            }
            else if (CMD_INIT == chunk) {
                size_t index = core_id_to_index.at(identity);
                federate_id_t id_remote;
                std::getline (iss, chunk);
                id_remote = boost::lexical_cast<federate_id_t>(chunk);
                cores[index].federates[id_remote]->state = HELICS_INITIALIZING;
                ++n_initialized;
                bool all_cores_had_a_chance_to_register = true;
                for (size_t i=0; i<cores.size(); ++i) {
                    all_cores_had_a_chance_to_register &= cores[i].federates.size() > 0UL;
                }
                if (all_cores_had_a_chance_to_register && n_registered == n_initialized)
                {
                    // pub/sub checks
                    LOG (INFO) << "performing pub/sub check" << ENDL;
                    for (size_t i = 0; i < handles.size(); ++i) {
                        ZeroMQHandle *pub = handles[i];
                        LOG (INFO) << "handle " << i << " is " << pub->what << ENDL;
                        if (pub->what == HANDLE_PUB) {
                            LOG (INFO) << "  is a pub" << ENDL;
                            for (size_t j = 0; j < handles.size(); ++j) {
                                ZeroMQHandle *sub = handles[j];
                                if (sub->what == HANDLE_SUB) {
                                    LOG (INFO) << "    " << j << " is a sub" << ENDL;
                                    if (pub->key == sub->key) {
                                        pub->handles.push_back(sub->id);
                                        LOG (INFO) << "    " << j << " is a sub and matches" << ENDL;
                                    }
                                    else {
                                        LOG (INFO) << "    " << j << " is a sub -- no match" << ENDL;
                                    }
                                }
                                // pub duplicate name check (ambiguity for what pub values should come from)
                                else if (sub->what == HANDLE_PUB) {
                                    if (pub->key == sub->key && i != j) {
                                        LOG (WARN) << "    " << j << " is a pub with the same name" << ENDL;
                                    }
                                }
                            }
                            if (pub->handles.empty()) {
                                LOG (INFO) << "nobody cares about " << pub->key << ENDL;
                            }
                        }
                    }
                    // check if every sub with required pub has a match
                    for (size_t i = 0; i < handles.size(); ++i) {
                        ZeroMQHandle *sub = handles[i];
                        if (sub->what == HANDLE_SUB && sub->required) {
                            bool hasMatch = false;
                            // look through list of subs for each pub
                            for (size_t j = 0; j < handles.size(); ++j) {
                                ZeroMQHandle *pub = handles[j];
                                if (pub->what == HANDLE_PUB) {
                                    for (auto handle : pub->handles) {
                                        if (sub->id == handle) {
                                            hasMatch = true;
                                        }
                                    }
                                }
                            }
                            if (!hasMatch) {
                                LOG (WARN) << "sub " << i << " has no corresponding pub" << ENDL;
                            }
                        }
                    }
                    // duplicate endpoint name check
                    for (size_t i = 0; i < handles.size(); ++i) {
                        ZeroMQHandle *endp1 = handles[i];
                        if (endp1->what == HANDLE_END) {
                            for (size_t j = 0; j < handles.size(); ++j) {
                                ZeroMQHandle *endp2 = handles[j];
                                if (endp2->what == HANDLE_END) {
                                    if (endp1->key == endp2->key && i != j) {
                                        LOG (WARN) << "handle " << i << " has the same name as endpoint " << j << ENDL;
                                    }
                                }
                            }
                        }
                    }
                    for (unsigned int i=0; i<cores.size(); ++i) {
                        zmqx_sendmore(zsock, cores[i].id);
                        zmqx_sendmore(zsock, CMD_INIT);
                        zmqx_send(zsock, n_initialized);
                    }
                }
            }
            else if (CMD_EXEC == chunk) {
                size_t index = core_id_to_index.at(identity);
                federate_id_t id_remote;
                std::getline (iss, chunk);
                id_remote = boost::lexical_cast<federate_id_t>(chunk);
                cores[index].federates[id_remote]->state = HELICS_EXECUTING;
                ++n_exec;
                if (n_registered == n_exec)
                {
                    n_processing = n_registered;
                    for (unsigned int i=0; i<cores.size(); ++i) {
                        zmqx_sendmore(zsock, cores[i].id);
                        zmqx_send(zsock, CMD_EXEC);
                    }
                }
            }
            else if (CMD_STOP == chunk) {
                assert(false);
            }
            else if (CMD_TIME == chunk || CMD_TIME_IT == chunk || CMD_BYE == chunk)
            {
                size_t index = core_id_to_index.at(identity);
                bool was_iter = (CMD_TIME_IT == chunk);
                bool was_bye = (CMD_BYE == chunk);
                federate_id_t id_remote;
                federate_id_t id;
                Time time_requested;
                Time::baseType time_base;
                bool converged_requested = true;
                std::getline (iss, chunk);
                id_remote = boost::lexical_cast<federate_id_t>(chunk);
                std::getline (iss, chunk);
                time_base = boost::lexical_cast<Time::baseType>(chunk);
                time_requested.setBaseTimeCode (time_base);
                if (was_iter)
                {
                    std::getline (iss, chunk);
                    converged_requested = boost::lexical_cast<bool>(chunk);
                }
                id = cores[index].federates[id_remote]->id;
                if (was_bye)
                {
                    federates[id]->state = HELICS_NONE;
                    federates[id]->iterative = false;
                    federates[id]->converged_requested = true;
                    federates[id]->time_requested = Time::maxVal ();
                    federates[id]->time_last_processed = time_granted;
                    federates[id]->processing = false;
                    federates[id]->events.clear();
                    --n_processing;
                    ++n_byes;
                    LOG(INFO) << "WAS BYE " << n_processing << " " << n_byes << " " << n_registered << ENDL;
                    if (n_byes == n_registered)
                    {
#if 0
                        for (size_t i = 0; i < n_registered; ++i)
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
                    LOG(INFO) << "updating federate " << id << " time values" << ENDL;
                    federates[id]->iterative = was_iter;
                    federates[id]->converged_requested = converged_requested;
                    federates[id]->time_requested = time_requested;
                    federates[id]->time_last_processed = time_granted;
                    federates[id]->processing = false;
                    federates[id]->events.clear();
                    --n_processing;
                }

                LOG(INFO) << n_processing << " federates are processing" << ENDL;
                /* if all sims are done, determine next time step */
                if (0 == n_processing)
                {
                    std::vector<Time> time_actionable (n_registered);
                    for (size_t i = 0; i < n_registered; ++i)
                    {
                        if (federates[i]->messages_pending)
                        {
                            time_actionable[i] =
                                federates[i]->time_last_processed + federates[i]->time_delta;
                        }
                        else
                        {
                            time_actionable[i] = federates[i]->time_requested;
                        }
                    }
                    Time maybe_time_granted = *std::min_element (time_actionable.begin (), time_actionable.end ());
                    LOG (INFO) << "maybe_time_granted = " << maybe_time_granted << ENDL;
                    /* check for convergence of the sims that agree with the
                     * actionable time */
                    bool coverged_all = true;
                    for (size_t i = 0; i < n_registered; ++i)
                    {
                        LOG (INFO) << "[" << i << "]: ";
                        if (maybe_time_granted == time_actionable[i])
                        {
                            LOG (INFO) << "YES " << federates[i]->converged_requested << ENDL;
                            coverged_all = coverged_all && federates[i]->converged_requested;
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
                        for (size_t i = 0; i < pending_values.size (); ++i)
                        {
                            Handle pending = pending_values[i];
                            ZeroMQHandle *pub = handles[pending];
                            assert(nullptr != pub);
                            assert(pub->what == HANDLE_PUB);
                            if (pub->has_update) {
                                pub->has_update = false;
                                LOG (INFO) << "PENDING " << pending << " updating " << pub->handles.size() << " subs" << ENDL;
                                for (size_t j = 0; j < pub->handles.size(); ++j) {
                                    auto subhand = pub->handles[j];
                                    ZeroMQHandle *sub = handles[subhand];
                                    assert(nullptr != sub);
                                    assert(sub->what == HANDLE_SUB);
                                    sub->data = pub->data;
                                    auto p = handles_map[sub->id];
                                    LOG (INFO) << "sending PUB " << pub->key << " to fed id " << p.first << " handle id " << p.second << ENDL;
                                    zmqx_sendmore(zsock, cores[p.first].id);
                                    zmqx_sendmore(zsock, CMD_PUB);
                                    zmqx_sendmore(zsock, p.second);
                                    zmqx_send(zsock, pub->data);
                                    federates[sub->fed_id]->events.push_back (sub->id);
                                    LOG(INFO) << "  " << subhand << ENDL;
                                }
                            }
                            else {
                                LOG(INFO) << "PENDING " << pending << " already handled" << ENDL;
                            }
                        }
                        pending_values.clear ();
                        /* send ack to waiting sims */
                        time_granted = maybe_time_granted;
                        iter = 0;
                        for (size_t i = 0; i < n_registered; ++i)
                        {
                            if (time_granted == time_actionable[i])
                            {
                                if (federates[i]->state == HELICS_EXECUTING)
                                {
                                    LOG (INFO)
                                        << "granting " << time_granted << " to " << federates[i]->name << ENDL;
                                    ++n_processing;
                                    federates[i]->processing = true;
                                    federates[i]->messages_pending = false;
                                    auto p = federates_map[i];
                                    zmqx_sendmore(zsock, cores[p.first].id);
                                    zmqx_sendmore(zsock, CMD_TIME);
                                    zmqx_sendmore(zsock, p.second);
                                    zmqx_sendmore(zsock, time_granted.getBaseTimeCode());
                                    zmqx_send(zsock, true);
                                }
                            }
                            else
                            {
#if 0
                                /* this code was causing an ambiguous call */
                                /* fast forward time last processed */
                                Time jump = (time_granted - federates[i]->time_last_processed) / federates[i]->time_delta;
                                federates[i]->time_last_processed += federates[i]->time_delta * jump;
#else
                                while (federates[i]->time_last_processed + federates[i]->time_delta <=
                                        time_granted)
                                {
                                    federates[i]->time_last_processed += federates[i]->time_delta;
                                }
#endif
                            }
                        }
                    }
                    else
                    {
                        LOG (INFO) << "DID NOT CONVERGE" << ENDL;
                        ++iter;
                        assert (iter < max_iterations);
                        for (size_t i = 0; i < n_registered; ++i)
                        {
                            if (maybe_time_granted == time_actionable[i] && federates[i]->iterative)
                            {
                                ++n_processing;
                                federates[i]->processing = true;
                                federates[i]->messages_pending = false;
                                auto p = federates_map[i];
                                zmqx_sendmore(zsock, cores[p.first].id);
                                zmqx_sendmore(zsock, CMD_TIME_IT);
                                zmqx_sendmore(zsock, p.second);
                                zmqx_send(zsock, false);
                            }
                        }
                    }
                }
            }
            else if (CMD_PUB == chunk)
            {
                size_t index = core_id_to_index.at(identity);
                Handle handle;
                Handle remote_handle;
                uint64_t len;
                uint64_t len_encoded;
                std::getline (iss, chunk);
                remote_handle = boost::lexical_cast<Handle>(chunk);
                std::getline (iss, chunk);
                len = boost::lexical_cast<uint64_t>(chunk);
                std::getline (iss, chunk);
                len_encoded = boost::lexical_cast<uint64_t>(chunk);
                std::getline (iss, chunk);
                std::string data = base64_decode(chunk);
                LOG (INFO) << "GOT PUB:" << ENDL;
                LOG (INFO) << len_encoded << " " << chunk.size() << ENDL;
                LOG (INFO) << "'" << chunk << "'" << ENDL;
                LOG (INFO) << len << " " << data.size() << ENDL;
                LOG (INFO) << "'" << data << "'" << ENDL;
                LOG (INFO) << "END PUB:" << ENDL;
                LOG (INFO) << "remote handle " << remote_handle << " handles size " << cores[index].handles.size();
                assert(remote_handle < cores[index].handles.size());
                handle = cores[index].handles[remote_handle]->id;
                LOG (INFO) << "local handle " << handle << " handles size " << handles.size() << ENDL;
                assert(handle < handles.size());
                ZeroMQHandle *pub = handles[handle];
                LOG (INFO) << "ZeroMQHandle " << pub->id << " " << pub->fed_id << " " << pub->what << " " << pub->key << ENDL;
                assert(pub->what == HANDLE_PUB);
                pub->data = data;
                pub->has_update = true;
                pending_values.push_back (handle);
                LOG (INFO) << "  will process " << pub->handles.size() << " updates" << ENDL;
                for (size_t i = 0; i < pub->handles.size(); ++i) {
                    ZeroMQHandle *sub = handles[pub->handles[i]];
                    assert(nullptr != sub);
                    assert(sub->what == HANDLE_SUB);
                    federates[sub->fed_id]->messages_pending = true;
                }
            }
        }
    }
    
    graceful_death(cores, false);

    return 0;
}

