/*

Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "IpcQueueHelper.h"

#include <boost/date_time/posix_time/ptime.hpp>
#include <string>
#include <thread>

namespace boostipc = boost::interprocess;

namespace helics {
namespace ipc {
    OwnedQueue::~OwnedQueue()
    {
        if (rqueue) {
            ipc_queue::remove(connectionName.c_str());
        }
        if (queue_state) {
            ipc_state::remove(stateName.c_str());
        }
    }

    bool OwnedQueue::connect(const std::string& connection, int maxMessages, int maxSize)
    {
        // remove the old queue if are connecting again
        if (rqueue) {
            ipc_queue::remove(connectionName.c_str());
        }
        if (queue_state) {
            ipc_state::remove(stateName.c_str());
        }
        connectionNameOrig = connection;
        connectionName = stringTranslateToCppName(connection);
        stateName = connectionName + "_state";
        ipc_queue::remove(connectionName.c_str());

        ipc_state::remove(stateName.c_str());

        try {
            queue_state = std::make_unique<ipc_state>(boostipc::create_only,
                                                      stateName.c_str(),
                                                      boostipc::read_write);
        }
        catch (boost::interprocess::interprocess_exception const& ipe) {
            errorString = std::string("Unable to open local state shared memory:") + ipe.what();
            return false;
        }
        queue_state->truncate(sizeof(SharedQueueState) + 256);
        // Map the whole shared memory in this process
        boostipc::mapped_region region(*queue_state, boostipc::read_write);

        auto* sstate = new (region.get_address()) SharedQueueState;
        sstate->setState(queue_state_t::startup);

        try {
            rqueue = std::make_unique<ipc_queue>(boostipc::create_only,
                                                 connectionName.c_str(),
                                                 maxMessages,
                                                 maxSize);
        }
        catch (boost::interprocess::interprocess_exception const& ipe) {
            errorString = std::string("Unable to open local connection:") + ipe.what();
            return false;
        }
        sstate->setState(queue_state_t::connected);
        mxSize = maxSize;
        buffer.resize(maxSize);
        connected = true;
        return true;
    }

    void OwnedQueue::changeState(queue_state_t newState)
    {
        if (connected) {
            boostipc::mapped_region region(*queue_state, boostipc::read_write);

            auto* sstate = reinterpret_cast<SharedQueueState*>(region.get_address());
            sstate->setState(newState);
        }
    }

    ActionMessage OwnedQueue::getMessage()
    {
        if (!connected) {
            return (CMD_ERROR);
        }
        size_t rx_size = 0;
        unsigned int priority{0};
        while (true) {
            rqueue->receive(buffer.data(), mxSize, rx_size, priority);
            if (rx_size < 8) {
                continue;
            }
            ActionMessage cmd(buffer.data(), rx_size);
            if (!isValidCommand(cmd)) {
                std::cerr << "invalid command received ipc" << std::endl;
                continue;
            }
            return cmd;
        }
    }

    stx::optional<ActionMessage> OwnedQueue::getMessage(int timeout)
    {
        if (!connected) {
            return stx::nullopt;
        }
        size_t rx_size = 0;
        unsigned int priority{0};
        while (true) {
            if (timeout >= 0) {
                boost::posix_time::ptime abs_time =
                    boost::date_time::microsec_clock<boost::posix_time::ptime>::universal_time();
                abs_time += boost::posix_time::milliseconds(timeout);
                bool res =
                    rqueue->timed_receive(buffer.data(), mxSize, rx_size, priority, abs_time);
                if (!res) {
                    return stx::nullopt;
                }
            } else if (timeout <= 0) {
                bool res = rqueue->try_receive(buffer.data(), mxSize, rx_size, priority);
                if (!res) {
                    return stx::nullopt;
                }
            }

            if (rx_size < 8) {
                continue;
            }
            ActionMessage cmd(buffer.data(), rx_size);
            if (!isValidCommand(cmd)) {
                std::cerr << "invalid command received ipc" << std::endl;
                continue;
            }
            return cmd;
        }
    }

    bool SendToQueue::connect(const std::string& connection, bool initOnly, int retries)
    {
        connectionNameOrig = connection;
        connectionName = stringTranslateToCppName(connection);
        std::string stateName = connectionName + "_state";
        bool goodToConnect = false;
        int tries = 0;
        while (!goodToConnect) {
            try {
                auto queue_state = std::make_unique<ipc_state>(boostipc::open_only,
                                                               stateName.c_str(),
                                                               boostipc::read_write);
                boostipc::mapped_region region(*queue_state, boostipc::read_write);

                auto* sstate = reinterpret_cast<SharedQueueState*>(region.get_address());

                switch (sstate->getState()) {
                    case queue_state_t::connected:
                    case queue_state_t::startup:
                        goodToConnect = true;
                        break;
                    case queue_state_t::operating:
                        if (!initOnly) {
                            goodToConnect = true;
                        }
                        break;
                    case queue_state_t::unknown:  // probably still undergoing setup
                    default:
                        break;
                }
                if (!goodToConnect) {
                    ++tries;
                    if (tries <= retries) {
                        queue_state.reset();
                        std::this_thread::sleep_for(std::chrono::milliseconds(200));
                    } else {
                        errorString = "timed out waiting for the queue to become available";
                        return false;
                    }
                }
            }

            catch (boost::interprocess::interprocess_exception const&) {
                // this likely means the shared_memory_object doesn't exist yet
                ++tries;
                if (tries <= retries) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                } else {
                    errorString = "timed out waiting for the queue to become available";
                    return false;
                }
            }
        }

        while (!connected) {
            try {
                txqueue = std::make_unique<ipc_queue>(boostipc::open_only, connectionName.c_str());
                connected = true;
            }
            catch (boost::interprocess::interprocess_exception const& ipe) {
                // this likely means the shared file doesn't exist yet
                ++tries;
                if (tries <= retries) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(200));
                } else {
                    errorString = std::string("Unable to open connection:") + ipe.what();
                    break;
                }
            }
        }
        return connected;
    }

    void SendToQueue::sendMessage(const ActionMessage& cmd, int priority)
    {
        if (connected) {
            cmd.to_vector(buffer);

            txqueue->send(buffer.data(), buffer.size(), priority);
        }
    }
}  // namespace ipc
}  // namespace helics
