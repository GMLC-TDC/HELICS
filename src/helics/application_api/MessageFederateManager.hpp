/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../common/GuardedTypes.hpp"
#include "../core/Core.hpp"
#include "../core/federate_id_extra.hpp"
#include "Endpoints.hpp"
#include "data_view.hpp"
#include "gmlc/containers/DualMappedVector.hpp"
#include "gmlc/containers/SimpleQueue.hpp"

#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
namespace helics {
class Core;
class MessageFederate;
/** class handling the implementation details of a value Federate
@details the functions will parallel those in message Federate and contain the actual implementation
details
*/
class MessageFederateManager {
  public:
    /** construct from a pointer to a core and a specified federate id
     */
    MessageFederateManager(Core* coreOb, MessageFederate* mFed, local_federate_id id);
    ~MessageFederateManager();
    /** register an endpoint
    @details call is only valid in startup mode
    @param name the name of the endpoint
    @param type the defined type of the interface for endpoint checking if requested
    */
    Endpoint& registerEndpoint(const std::string& name, const std::string& type);

    /** @brief give the core a hint for known communication paths
    Specifying a path that is not present will cause the simulation to abort with an error message
    @param localEndpoint the local endpoint of a known communication pair
    @param remoteEndpoint of a communication pair
    */
    void registerKnownCommunicationPath(const Endpoint& localEndpoint,
                                        const std::string& remoteEndpoint);
    /** subscribe to valueFederate publication to be delivered as Messages to the given endpoint
    @param ept the specified endpoint to deliver the values
    @param pubName the name of the publication to subscribe
    */
    void subscribe(const Endpoint& ept, const std::string& pubName);
    /** check if the federate has any outstanding messages*/
    bool hasMessage() const;
    /* check if a given endpoint has any unread messages*/
    static bool hasMessage(const Endpoint& ept);

    /**
     * Returns the number of pending receives for the specified destination endpoint.
     */
    static uint64_t pendingMessages(const Endpoint& ept);
    /**
     * Returns the number of pending receives for the specified destination endpoint.
     */
    uint64_t pendingMessages() const;
    /** receive a packet from a particular endpoint
    @param ept the identifier for the endpoint
    @return a message object*/
    static std::unique_ptr<Message> getMessage(const Endpoint& ept);
    /* receive a communication message for any endpoint in the federate*/
    std::unique_ptr<Message> getMessage();

    /**/
    void sendMessage(const Endpoint& source, const std::string& dest, const data_view& message);
    /**/
    void sendMessage(const Endpoint& source,
                     const std::string& dest,
                     const data_view& message,
                     Time sendTime);
    /**/
    void sendMessage(const Endpoint& source, std::unique_ptr<Message> message);

    /** update the time from oldTime to newTime
    @param newTime the newTime of the federate
    @param oldTime the oldTime of the federate
    */
    void updateTime(Time newTime, Time oldTime);
    /** transition from Startup To the Initialize State*/
    void startupToInitializeStateTransition();
    /** transition from initialize to execution State*/
    void initializeToExecuteStateTransition(iteration_result result);
    /** generate results for a local query */
    std::string localQuery(const std::string& queryStr) const;

    /** get the id of a registered publication from its id
    @param name the publication id
    @return ivalid_publication_id if name is not recognized otherwise returns the publication_id*/
    Endpoint& getEndpoint(const std::string& name);
    const Endpoint& getEndpoint(const std::string& name) const;

    Endpoint& getEndpoint(int index);
    const Endpoint& getEndpoint(int index) const;

    /** register a callback function to call when any endpoint receives a message
    @details there can only be one generic callback
    @param callback the function to call
    */
    void setEndpointNotificationCallback(const std::function<void(Endpoint&, Time)>& callback);
    /** register a callback function to call when the specified endpoint receives a message
    @param ept  the endpoint id to register the callback for
    @param callback the function to call
    */
    static void
        setEndpointNotificationCallback(const Endpoint& ept,
                                        const std::function<void(Endpoint&, Time)>& callback);

    /**disconnect from the coreObject*/
    void disconnect();
    /**get the number of registered endpoints*/
    int getEndpointCount() const;

    /** add a named filter to an endpoint for all message coming from the endpoint*/
    void addSourceFilter(const Endpoint& ept, const std::string& filterName);
    /** add a named filter to an endpoint for all message going to the endpoint*/
    void addDestinationFilter(const Endpoint& ept, const std::string& filterName);

  private:
    class EndpointData {
      public:
        gmlc::containers::SimpleQueue<std::unique_ptr<Message>> messages;
        atomic_guarded<std::function<void(Endpoint&, Time)>> callback;
    };
    shared_guarded<
        gmlc::containers::
            DualMappedVector<Endpoint, std::string, interface_handle, reference_stability::stable>>
        local_endpoints;  //!< storage for the local endpoint information
    atomic_guarded<std::function<void(Endpoint&, Time)>> allCallback;
    Time CurrentTime = Time::minVal();  //!< the current simulation time
    Core* coreObject;  //!< the pointer to the actual core
    MessageFederate* mFed;  //!< pointer back to the message Federate
    const local_federate_id fedID;  //!< storage for the federate ID
    shared_guarded<std::vector<std::unique_ptr<EndpointData>>>
        eptData;  //!< the storage for the message queues and other unique Endpoint information
    guarded<std::vector<unsigned int>>
        messageOrder;  //!< maintaining a list of the ordered messages
  private:  // private functions
    void removeOrderedMessage(unsigned int index);
};
}  // namespace helics
