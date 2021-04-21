/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "gmlc/containers/extra/optional.hpp"
#include "helics/core/ActionMessage.hpp"

#include <algorithm>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <cctype>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using ipc_queue = boost::interprocess::message_queue;
using ipc_state = boost::interprocess::shared_memory_object;

namespace helics {
namespace ipc {
    /** translate a string to a C++ qualified name for variable naming purposes
     */
    inline std::string stringTranslateToCppName(std::string in)
    {
        std::replace_if(
            in.begin(), in.end(), [](auto c) { return !(std::isalnum(c) || (c == '_')); }, '_');
        return in;
    }
    /** enumeration of queue states*/
    enum class queue_state_t : int {
        unknown = -1,
        startup = 0,
        connected = 1,
        operating = 2,
        closing = 3,
    };

    /** class defining a shared queue state meaning interaction with a queue the object is not the
     * owner of
     */
    class SharedQueueState {
      private:
        using ipcmutex = boost::interprocess::interprocess_mutex;
        mutable ipcmutex data_lock;
        queue_state_t state = queue_state_t::startup;

      public:
        queue_state_t getState() const
        {
            try {
                boost::interprocess::scoped_lock<ipcmutex> lock(data_lock);
                return state;
            }
            catch (const boost::interprocess::lock_exception&) {
                return queue_state_t::unknown;
            }
        }
        bool setState(queue_state_t newState)
        {
            bool success = false;
            int tries = 0;
            while (!success) {
                try {
                    boost::interprocess::scoped_lock<ipcmutex> lock(data_lock);
                    state = newState;
                    success = true;
                }
                catch (const boost::interprocess::lock_exception&) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    ++tries;
                    if (tries > 20) {
                        std::cout << "error in connecting to process lock\n";
                        state = newState;
                        return false;
                    }
                }
            }
            return success;
        }
    };

    /** class implementing a queue owned by a particular object*/
    class OwnedQueue {
      private:
        std::unique_ptr<ipc_queue> rqueue;
        std::unique_ptr<ipc_state> queue_state;
        std::string connectionNameOrig;
        std::string connectionName;
        std::string stateName;
        std::string errorString;
        std::vector<char> buffer;
        int mxSize = 0;
        bool connected = false;

      public:
        OwnedQueue() = default;
        ~OwnedQueue();
        bool connect(const std::string& connection, int maxMessages, int maxSize);

        void changeState(queue_state_t newState);

        stx::optional<ActionMessage> getMessage(int timeout);
        ActionMessage getMessage();

        const std::string& getError() const { return errorString; }
    };

    /** class implementing interactions with a queue to transmit data*/
    class SendToQueue {
      private:
        std::unique_ptr<ipc_queue> txqueue;  //!< the actual interprocess queue
        std::string connectionNameOrig;  //!< the connection name as specified
        std::string
            connectionName;  //!< translation of the connection name using only valid characters
        std::string errorString;  //!< buffer for any error code
        std::vector<char> buffer;  //!< storage for serialized data of the message
        bool connected = false;  //!< flag indicating connectivity

      public:
        SendToQueue() = default;

        bool connect(const std::string& connection, bool initOnly, int retries);

        void sendMessage(const ActionMessage& cmd, int priority);

        const std::string& getError() const { return errorString; }
    };
}  // namespace ipc
}  // namespace helics
