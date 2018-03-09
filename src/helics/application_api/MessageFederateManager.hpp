/*
Copyright © 2017-2018, Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../common/DualMappedPointerVector.hpp"
#include "../common/GuardedTypes.hpp"
#include "../common/simpleQueue.hpp"
#include "../core/Core.hpp"
#include "data_view.hpp"
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
namespace helics
{
class Core;
/** data class containing information on an endpoint for Message Federate Manager*/
class endpoint_info
{
  public:
    std::string name;
    std::string type;
    endpoint_id_t id = invalid_id_value;
    Core::handle_id_t handle = invalid_handle;
    int callbackIndex = -1;
    endpoint_info () = default;
    endpoint_info (std::string n_name, std::string n_type, endpoint_id_t n_id, Core::handle_id_t n_handle)
        : name (std::move (n_name)), type (std::move (n_type)), id (n_id), handle (n_handle){};
};

/** class handling the implementation details of a value Federate
@details the functions will parallel those in message Federate and contain the actual implementation details
*/
class MessageFederateManager
{
  public:
    /** construct from a pointer to a core and a specified federate id
     */
    MessageFederateManager (Core *coreOb, Core::federate_id_t id);
    ~MessageFederateManager ();
    /** register an endpoint
    @details call is only valid in startup mode
    @param[in] name the name of the endpoint
    @param[in] type the defined type of the interface for endpoint checking if requested
    */
    endpoint_id_t registerEndpoint (const std::string &name, const std::string &type);

    /** @brief give the core a hint for known communication paths
    Specifying a path that is not present will cause the simulation to abort with an error message
    @param[in] localEndpoint the local endpoint of a known communication pair
    @param[in] remoteEndpoint of a communication pair
    */
    void registerKnownCommunicationPath (endpoint_id_t localEndpoint, const std::string &remoteEndpoint);
    /** subscribe to valueFederate publication to be delivered as Messages to the given endpoint
    @param[in] endpoint the specified endpoint to deliver the values
    @param[in] name the name of the publication to subscribe
    @param[in] type the type of publication
    */
    void subscribe (endpoint_id_t endpoint, const std::string &name, const std::string &type);
    /** check if the federate has any outstanding messages*/
    bool hasMessage () const;
    /* check if a given endpoint has any unread messages*/
    bool hasMessage (endpoint_id_t id) const;

    /**
     * Returns the number of pending receives for the specified destination endpoint.
     */
    uint64_t receiveCount (endpoint_id_t id) const;
    /**
     * Returns the number of pending receives for the specified destination endpoint.
     */
    uint64_t receiveCount () const;
    /** receive a packet from a particular endpoint
    @param[in] endpoint the identifier for the endpoint
    @return a message object*/
    std::unique_ptr<Message> getMessage (endpoint_id_t endpoint);
    /* receive a communication message for any endpoint in the federate*/
    std::unique_ptr<Message> getMessage ();

    /**/
    void sendMessage (endpoint_id_t source, const std::string &dest, data_view message);

    void sendMessage (endpoint_id_t source, const std::string &dest, data_view message, Time sendTime);

    void sendMessage (endpoint_id_t source, std::unique_ptr<Message> message);

    /** update the time from oldTime to newTime
    @param[in] newTime the newTime of the federate
    @param[in] oldTime the oldTime of the federate
    */
    void updateTime (Time newTime, Time oldTime);
    /** transition from Startup To the Initialize State*/
    void startupToInitializeStateTransition ();
    /** transition from initialize to execution State*/
    void initializeToExecuteStateTransition ();

    /** get the name of an endpoint from its id
    @param[in] id the endpoint to query
    @return empty string if an invalid id is passed*/
    std::string getEndpointName (endpoint_id_t id) const;

    /** get the id of a registered publication from its id
    @param[in] name the publication id
    @return ivalid_publication_id if name is not recognized otherwise returns the publication_id*/
    endpoint_id_t getEndpointId (const std::string &name) const;
    /** get the type of an endpoint from its id
    @param[in] id the endpoint to query
    @return empty string if an invalid id is passed or no type was specified*/
    std::string getEndpointType (endpoint_id_t id) const;
    /** register a callback function to call when any endpoint receives a message
    @details there can only be one generic callback
    @param[in] callback the function to call
    */
    void registerCallback (const std::function<void(endpoint_id_t, Time)> &callback);
    /** register a callback function to call when the specified endpoint receives a message
    @param[in] id  the endpoint id to register the callback for
    @param[in] callback the function to call
    */
    void registerCallback (endpoint_id_t id, const std::function<void(endpoint_id_t, Time)> &callback);
    /** register a callback function to call when one of the specified endpoint ids receives a message
    @param[in] ids  the set of ids to register the callback for
    @param[in] callback the function to call
    */
    void registerCallback (const std::vector<endpoint_id_t> &ids,
                           const std::function<void(endpoint_id_t, Time)> &callback);

    /**disconnect from the coreObject*/
    void disconnect ();
    /**get the number of registered endpoints*/
    int getEndpointCount () const;

  private:
    shared_guarded<DualMappedPointerVector<endpoint_info, std::string, Core::handle_id_t>>
      local_endpoints;  //!< storage for the local endpoint information
    std::vector<std::function<void(endpoint_id_t, Time)>> callbacks;  //!< vector of callbacks

    std::map<Core::handle_id_t, std::pair<endpoint_id_t, std::string>> subHandleLookup;  //!< map for subscriptions
    Time CurrentTime;  //!< the current simulation time
    Core * coreObject;  //!< the pointer to the actual core
    std::atomic<endpoint_id_t::underlyingType> endpointCount{0};  //!< the count of actual endpoints
    const Core::federate_id_t fedID;  //!< storage for the federate ID
    mutable std::mutex endpointLock;  //!< lock for protecting the endpoint list
    std::vector<SimpleQueue<std::unique_ptr<Message>>> messageQueues;  //!< the storage for the message queues
    guarded<std::vector<unsigned int>> messageOrder;  //!< maintaining a list of the ordered messages
    int allCallbackIndex = -1;  //!< index of the all callback function
    bool hasSubscriptions = false;  //!< indicator that the message filter subscribes to data values
  private:  // private functions
    void removeOrderedMessage (unsigned int index);
};
}  // namespace helics

