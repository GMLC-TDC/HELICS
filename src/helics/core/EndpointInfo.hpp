/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../common/GuardedTypes.hpp"
#include "basic_CoreTypes.hpp"

#include <atomic>
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
    EndpointInformation(GlobalHandle gid, std::string_view key_, std::string_view type_):
        id(gid), key(key_), type(type_)
    {
    }
};
/** data class containing the information about an endpoint*/
class EndpointInfo {
  public:
    /** constructor from all data*/
    EndpointInfo(GlobalHandle handle, std::string_view key_, std::string_view type_):
        id(handle), key(key_), type(type_)
    {
    }

    const GlobalHandle id;  //!< identifier for the handle
    const std::string key;  //!< name of the endpoint
    const std::string type;  //!< type of the endpoint
  private:
    /// storage for the messages
    shared_guarded<std::deque<std::unique_ptr<Message>>> message_queue;
    std::atomic<int32_t> mAvailableMessages{0};  //!< indicator of how many message are available

    std::vector<EndpointInformation> sourceInformation;
    std::vector<EndpointInformation> targetInformation;
    std::vector<std::pair<GlobalHandle, std::string_view>> targets;
    mutable std::string sourceTargets;
    mutable std::string destinationTargets;

  public:
    bool hasFilter{false};  //!< indicator that the message has a filter
    bool required{false};
    bool targetedEndpoint{false};  //!< indicator that the endpoint is a targeted endpoint only
    bool sourceOnly{false};  //!< the endpoint can only be a source of data and cannot receive data
    bool receiveOnly{false};  //!< the endpoint can only receive data and cannot send
    int32_t requiredConnections{0};  //!< an exact number of connections required
    /** get the next message up to the specified time*/
    std::unique_ptr<Message> getMessage(Time maxTime);
    /** get the number of messages in the queue up to the specified time*/
    int32_t availableMessages() const;
    /** get the number of messages available up to a specific time inclusive*/
    int32_t queueSize(Time maxTime) const;
    /** get the number of messages available prior to a specific time*/
    int32_t queueSizeUpTo(Time maxTime) const;
    /** add a message to the queue*/
    void addMessage(std::unique_ptr<Message> message);
    /** update current data not including data at the specified time
    @param newTime the time to move the subscription to
    @return true if the value has changed
    */
    bool updateTimeUpTo(Time newTime);
    /** update current data to all new data at newTime
    @param newTime the time to move the subscription to
    @return true if the value has changed
    */
    bool updateTimeInclusive(Time newTime);

    /** check if the endpoint has any connection information  source or target*/
    bool hasConnection() const;
    /** check if the endpoint has any source connection information*/
    bool hasSource() const;
    /** check if the endpoint has any target connection information*/
    bool hasTarget() const;
    /** update current data to get all data through the first iteration at newTime
    @param newTime the time to move the subscription to
    @return true if the value has changed
    */
    bool updateTimeNextIteration(Time newTime);
    /** get the timestamp of the first message in the queue*/
    Time firstMessageTime() const;
    /** clear all the message queues*/
    void clearQueue();
    /** add a target to send messages*/
    void addDestination(GlobalHandle dest, std::string_view destName, std::string_view destType);
    /** add an endpoint to receive information from*/
    void addSource(GlobalHandle source, std::string_view sourceName, std::string_view sourceType);
    /** remove a target from connection*/
    void removeTarget(GlobalHandle targetId);
    /** disconnect a federate */
    void disconnectFederate(GlobalFederateId fedToDisconnect);
    /** get the vector of endpoint targets*/
    const std::vector<std::pair<GlobalHandle, std::string_view>>& getTargets() const
    {
        return targets;
    }
    /** get a string with the names of the source endpoints*/
    const std::string& getSourceTargets() const;
    /** get a string with the names of the destination endpoints*/
    const std::string& getDestinationTargets() const;

    /** check the interfaces for specific issues*/
    void checkInterfacesForIssues(std::vector<std::pair<int, std::string>>& issues);

    void setProperty(int32_t option, int32_t value);
    int32_t getProperty(int32_t option) const;
};
}  // namespace helics
