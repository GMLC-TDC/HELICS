/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_IPC_QUEUE_HELPER_
#define _HELICS_IPC_QUEUE_HELPER_
#pragma once

#include "../ActionMessage.h"
#include <algorithm>
#include <cctype>
#include <helics_includes/optional.h>
#include <iostream>
#include <memory>
#include <thread>
#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

using ipc_queue = boost::interprocess::message_queue;
using ipc_state = boost::interprocess::shared_memory_object;

namespace helics
{
/** translate a string to a C++ qualified name for variable naming purposes
*/
inline std::string stringTranslateToCppName (std::string in)
{
    std::replace_if (in.begin (), in.end (), [](auto c) { return !(std::isalnum (c) || (c == '_')); }, '_');
    return in;
}
/** enumeration of queue states*/
enum class queue_state_t : int
{
    unknown = -1,
    startup = 0, 
    connected = 1,
    operating = 2,
    closing = 3,
};

/** class defining a shared queue state meaning interaction with a queue the object is not the owner of
*/
class shared_queue_state
{
  private:
      using ipcmutex = boost::interprocess::interprocess_mutex;
    mutable ipcmutex data_lock;
    queue_state_t state = queue_state_t::startup;

  public:
    queue_state_t getState () const
    {
        try
        {
            boost::interprocess::scoped_lock<ipcmutex> lock (data_lock);
            return state;
        }
        catch (const boost::interprocess::lock_exception &)
        {
            return queue_state_t::unknown;
        }
    }
    bool setState (queue_state_t newState)
    {
        bool success = false;
        int tries = 0;
        while (!success)
        {
            try
            {
                boost::interprocess::scoped_lock<ipcmutex> lock (data_lock);
                state = newState;
                success = true;
                
            }
            catch (const boost::interprocess::lock_exception &)
            {
                std::this_thread::sleep_for (std::chrono::milliseconds (100));
                ++tries;
                if (tries > 20)
                {
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
    class ownedQueue
    {
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
        ownedQueue () = default;
        ~ownedQueue ();
        bool connect (const std::string &connection, int maxMessages, int maxSize);

        void changeState (queue_state_t newState);

        stx::optional<ActionMessage> getMessage (int timeout);
        ActionMessage getMessage ();

        const std::string &getError () const { return errorString; }
    };

    /** class implementing interactions with a queue to transmit data*/
    class sendToQueue
    {
      private:
        std::unique_ptr<ipc_queue> txqueue; //!< the actual interprocess queue
        std::string connectionNameOrig; //!< the connection name as specified   
        std::string connectionName; //!< translation of the connection name using only valid characters
        std::string errorString; //!< buffer for any error code
        std::vector<char> buffer; //!< storage for serialized data of the message
        bool connected = false; //!< flag indicating connectivity

      public:
        sendToQueue () = default;

        bool connect (const std::string &connection, bool initOnly, int retries);

        void sendMessage (const ActionMessage &cmd, int priority);

        const std::string &getError () const { return errorString; }
    };

}  // namespace helics

#endif /* _HELICS_IPC_QUEUE_HELPER_ */
