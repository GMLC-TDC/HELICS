/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "Core.hpp"
#include "helics-time.hpp"

#include "helics/helics-config.h"

#include "../common/GuardedTypes.hpp"
#include <deque>
namespace helics
{
/** data class containing the information about an endpoint*/
class EndpointInfo
{
  public:
    /** constructor from all data*/
    EndpointInfo (global_handle handle, const std::string &key_, const std::string &type_)
        : id (handle), key (key_), type (type_)
    {
    }

    const global_handle id;  //!< identifier for the handle
    const std::string key;  //!< name of the endpoint
    const std::string type;  //!< type of the endpoint
    bool hasFilter = false;  //!< indicator that the message has a filter
    bool not_interruptible = false;  //!< indicator that the message is not allowed to generate a time interrupt
  private:
    shared_guarded<std::deque<std::unique_ptr<Message>>> message_queue;  //!< storage for the messages
  public:
    /** get the next message up to the specified time*/
    std::unique_ptr<Message> getMessage (Time maxTime);
    /** get the number of messages in the queue up to the specified time*/
    int32_t queueSize (Time maxTime) const;
    /** add a message to the queue*/
    void addMessage (std::unique_ptr<Message> message);
    /** get the timestamp of the first message in the queue*/
    Time firstMessageTime () const;
    /** clear all the message queues*/
    void clearQueue ();
};
}  // namespace helics
