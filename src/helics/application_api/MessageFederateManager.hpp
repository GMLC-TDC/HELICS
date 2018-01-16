/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _MESSAGE_FEDERATE_MANAGER_
#define _MESSAGE_FEDERATE_MANAGER_
#pragma once

#include "Message.hpp"
#include "../common/simpleQueue.hpp"
#include "../core/core.h"
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "libguarded/guarded.hpp"

namespace helics
{
class Core;
/** structure containing information about an endpoint*/
struct endpoint_info
{
    std::string name;
    std::string type;
    endpoint_id_t id = invalid_id_value;
    Core::handle_id_t handle;
    int callbackIndex = -1;
    endpoint_info (std::string n_name, std::string n_type)
        : name (std::move (n_name)), type (std::move (n_type)){};
};

/** class handling the implementation details of a value Federate
@details the functions will parallel those in message Federate and contain the actual implementation details
*/
class MessageFederateManager
{
  public:
      /** construct from a pointer to a core and a specified federate id
      */
    MessageFederateManager (std::shared_ptr<Core> coreOb, Core::federate_id_t id);
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
    void registerCallback (std::function<void(endpoint_id_t, Time)> callback);
    /** register a callback function to call when the specified endpoint receives a message
    @param[in] id  the endpoint id to register the callback for
    @param[in] callback the function to call
    */
    void registerCallback (endpoint_id_t id, std::function<void(endpoint_id_t, Time)> callback);
    /** register a callback function to call when one of the specified endpoint ids receives a message
    @param[in] ids  the set of ids to register the callback for
    @param[in] callback the function to call
    */
    void
    registerCallback (const std::vector<endpoint_id_t> &ids, std::function<void(endpoint_id_t, Time)> callback);

    /**disconnect from the coreObject*/
    void disconnect();
    /**get the number of registered endpoints*/
    int getEndpointCount() const;
  private:
    std::unordered_map<std::string, endpoint_id_t>
      endpointNames;  //!< container to translate names to endpoint id's

    std::vector<endpoint_info> local_endpoints;  //!< storage for the local endpoint information
    std::vector<std::function<void(endpoint_id_t, Time)>> callbacks;  //!< vector of callbacks

    std::map<Core::handle_id_t, endpoint_id_t> handleLookup;  //!< map to lookup endpoints from core handles
    std::map<Core::handle_id_t, std::pair<endpoint_id_t, std::string>> subHandleLookup;  //!< map for subscriptions
    Time CurrentTime;  //!< the current simulation time
    std::shared_ptr<Core> coreObject;  //!< the pointer to the actual core
    Core::federate_id_t fedID;  //!< storage for the federate ID
    mutable std::mutex endpointLock;  //!< lock for protecting the endpoint list
    std::vector<SimpleQueue<std::unique_ptr<Message>>> messageQueues;  //!< the storage for the message queues
    libguarded::guarded<std::vector<unsigned int>> messageOrder;  //!< maintaining a list of the ordered messages
    int allCallbackIndex = -1;  //!< index of the all callback function
    bool hasSubscriptions = false;  //!< indicator that the message filter subscribes to data values
  private:  // private functions
    void removeOrderedMessage (unsigned int index);
};
} //namespace helics
#endif