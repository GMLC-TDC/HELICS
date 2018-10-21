/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../common/DualMappedVector.hpp"
#include "../common/GuardedTypes.hpp"
#include "../common/simpleQueue.hpp"
#include "../core/Core.hpp"
#include "Endpoints.hpp"
#include "data_view.hpp"
#include <cstdint>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
namespace helics
{
class Core;

/** class handling the implementation details of a value Federate
@details the functions will parallel those in message Federate and contain the actual implementation details
*/
class MessageFederateManager
{
  public:
    /** construct from a pointer to a core and a specified federate id
     */
    MessageFederateManager (Core *coreOb, federate_id_t id);
    ~MessageFederateManager ();
    /** register an endpoint
    @details call is only valid in startup mode
    @param[in] name the name of the endpoint
    @param[in] type the defined type of the interface for endpoint checking if requested
    */
    Endpoint &registerEndpoint (const std::string &name, const std::string &type);

    /** @brief give the core a hint for known communication paths
    Specifying a path that is not present will cause the simulation to abort with an error message
    @param[in] localEndpoint the local endpoint of a known communication pair
    @param[in] remoteEndpoint of a communication pair
    */
    void registerKnownCommunicationPath (Endpoint &localEndpoint, const std::string &remoteEndpoint);
    /** subscribe to valueFederate publication to be delivered as Messages to the given endpoint
    @param[in] endpoint the specified endpoint to deliver the values
    @param[in] name the name of the publication to subscribe
    @param[in] type the type of publication
    */
    void subscribe (Endpoint &ept, const std::string &pubName);
    /** check if the federate has any outstanding messages*/
    bool hasMessage () const;
    /* check if a given endpoint has any unread messages*/
    bool hasMessage (Endpoint &ept) const;

    /**
     * Returns the number of pending receives for the specified destination endpoint.
     */
    uint64_t pendingMessages (const Endpoint &ept) const;
    /**
     * Returns the number of pending receives for the specified destination endpoint.
     */
    uint64_t pendingMessages () const;
    /** receive a packet from a particular endpoint
    @param[in] endpoint the identifier for the endpoint
    @return a message object*/
    std::unique_ptr<Message> getMessage (Endpoint &ept);
    /* receive a communication message for any endpoint in the federate*/
    std::unique_ptr<Message> getMessage ();

    /**/
    void sendMessage (Endpoint &source, const std::string &dest, data_view message);

    void sendMessage (Endpoint &source, const std::string &dest, data_view message, Time sendTime);

    void sendMessage (Endpoint &source, std::unique_ptr<Message> message);

    /** update the time from oldTime to newTime
    @param[in] newTime the newTime of the federate
    @param[in] oldTime the oldTime of the federate
    */
    void updateTime (Time newTime, Time oldTime);
    /** transition from Startup To the Initialize State*/
    void startupToInitializeStateTransition ();
    /** transition from initialize to execution State*/
    void initializeToExecuteStateTransition ();
    /** generate results for a local query */
    std::string localQuery (const std::string &queryStr) const;
    /** get the name of an endpoint from its id
    @param[in] id the endpoint to query
    @return empty string if an invalid id is passed*/
    const std::string &getEndpointName (const Endpoint &ept) const;

    /** get the id of a registered publication from its id
    @param[in] name the publication id
    @return ivalid_publication_id if name is not recognized otherwise returns the publication_id*/
    endpoint_id_t getEndpointId (const std::string &name) const;
    /** get the type of an endpoint from its id
    @param[in] id the endpoint to query
    @return empty string if an invalid id is passed or no type was specified*/
    const std::string &getEndpointType (const Endpoint &ept) const;
    /** register a callback function to call when any endpoint receives a message
    @details there can only be one generic callback
    @param[in] callback the function to call
    */
    void registerCallback (const std::function<void(Endpoint &, Time)> &callback);
    /** register a callback function to call when the specified endpoint receives a message
    @param[in] id  the endpoint id to register the callback for
    @param[in] callback the function to call
    */
    void registerCallback (Endpoint &ept, const std::function<void(Endpoint &, Time)> &callback);

    /**disconnect from the coreObject*/
    void disconnect ();
    /**get the number of registered endpoints*/
    int getEndpointCount () const;

    /** set an endpoint option */
    void setEndpointOption (Endpoint &ept, int32_t option, bool option_value);
    /** add a named filter to an endpoint for all message coming from the endpoint*/
    void addSourceFilter (Endpoint &ept, const std::string &filterName);
    /** add a named filter to an endpoint for all message going to the endpoint*/
    void addDestinationFilter (Endpoint &ept, const std::string &filterName);

  private:
    shared_guarded<DualMappedVector<Endpoint, std::string, interface_handle>>
      local_endpoints;  //!< storage for the local endpoint information
    std::vector<std::function<void(Endpoint &, Time)>> callbacks;  //!< vector of callbacks
    Time CurrentTime;  //!< the current simulation time
    Core *coreObject;  //!< the pointer to the actual core
    std::atomic<int> endpointCount{0};  //!< the count of actual endpoints
    const federate_id_t fedID;  //!< storage for the federate ID
    mutable std::mutex endpointLock;  //!< lock for protecting the endpoint list
    std::vector<SimpleQueue<std::unique_ptr<Message>>> messageQueues;  //!< the storage for the message queues
    guarded<std::vector<unsigned int>> messageOrder;  //!< maintaining a list of the ordered messages
    int allCallbackIndex = -1;  //!< index of the all callback function
  private:  // private functions
    void removeOrderedMessage (unsigned int index);
};
}  // namespace helics
