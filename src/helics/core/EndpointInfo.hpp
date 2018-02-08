/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_ENDPOINTINFO_
#define _HELICS_ENDPOINTINFO_

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
    EndpointInfo (Core::handle_id_t id_,
                  Core::federate_id_t fed_id_,
                  const std::string &key_,
                  const std::string &type_)
        : id (id_), fed_id (fed_id_), key (key_), type (type_)
    {
    }

    const Core::handle_id_t id;  //!< identifier for the endpoint
    const Core::federate_id_t fed_id;  //!< local federate id
    const std::string key;  //!< name of the endpoint
    const std::string type;  //!< type of the endpoint
  private:
      shared_guarded<std::deque<std::unique_ptr<Message>>> message_queue;  //!< storage for the messages
  public:
    bool hasFilter = false;  //!< indicator that the message has a filter
    /** get the next message up to the specified time*/
    std::unique_ptr<Message> getMessage (Time maxTime);
    /** get the number of messages in the queue up to the specified time*/
    int32_t queueSize (Time maxTime) const;
    /** add a message to the queue*/
    void addMessage (std::unique_ptr<Message> message);
    /** get the timestamp of the first message in the queue*/
    Time firstMessageTime () const;
};
}  // namespace helics

#endif