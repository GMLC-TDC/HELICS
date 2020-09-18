/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../common/GuardedTypes.hpp"
#include "basic_core_types.hpp"

#include <deque>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace helics {

struct EndpointInformation {
    GlobalHandle id;
    std::string key;
    std::string type;
    EndpointInformation() = default;
    EndpointInformation(GlobalHandle gid, const std::string& key_, const std::string& type_):
        id(gid), key(key_), type(type_)
    {
    }
};
/** data class containing the information about an endpoint*/
class EndpointInfo {
  public:
    /** constructor from all data*/
    EndpointInfo(GlobalHandle handle, const std::string& key_, const std::string& type_):
        id(handle), key(key_), type(type_)
    {
    }

    const GlobalHandle id;  //!< identifier for the handle
    const std::string key;  //!< name of the endpoint
    const std::string type;  //!< type of the endpoint
  private:
    shared_guarded<std::deque<std::unique_ptr<Message>>>
        message_queue;  //!< storage for the messages
    std::vector<EndpointInformation> sourceInformation;
    std::vector<EndpointInformation> targetInformation;
    std::vector<std::pair<GlobalHandle, std::string_view>> targets;
    mutable std::string sourceTargets;
    mutable std::string destinationTargets;

  public:
    bool hasFilter{false};  //!< indicator that the message has a filter
    bool targettedEndpoint{false};  //!< indicator that the endpoint is a targeted endpoint only
    /** get the next message up to the specified time*/
    std::unique_ptr<Message> getMessage(Time maxTime);
    /** get the number of messages in the queue up to the specified time*/
    int32_t queueSize(Time maxTime) const;
    /** add a message to the queue*/
    void addMessage(std::unique_ptr<Message> message);
    /** get the timestamp of the first message in the queue*/
    Time firstMessageTime() const;
    /** clear all the message queues*/
    void clearQueue();
    /** add a target target*/
    void addDestinationTarget(GlobalHandle dest,
                              const std::string& destName,
                              const std::string& destType);
    /** add a source to an endpoint*/
    void addSourceTarget(GlobalHandle dest,
                         const std::string& sourceName,
                         const std::string& sourceType);
    /** remove a target from connection*/
    void removeTarget(GlobalHandle targetId);
    /** get the vector of endpoint targets*/
    const std::vector<std::pair<GlobalHandle, std::string_view>>& getTargets() const
    {
        return targets;
    }
    /** get a string with the names of the source endpoints*/
    const std::string& getSourceTargets() const;
    /** get a string with the names of the destination endpoints*/
    const std::string& getDestinationTargets() const;
};
}  // namespace helics
