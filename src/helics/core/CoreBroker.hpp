/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include <array>
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <thread>
#include <unordered_map>

#include "../common/AirLock.hpp"
#include "../common/DelayedObjects.hpp"
#include "../common/DualMappedVector.hpp"
#include "../common/simpleQueue.hpp"
#include "helics_includes/any.hpp"

#include "../common/TriggerVariable.hpp"
#include "ActionMessage.hpp"
#include "BasicHandleInfo.hpp"
#include "Broker.hpp"
#include "BrokerBase.hpp"
#include "HandleManager.hpp"
#include "JsonMapBuilder.hpp"
#include "TimeDependencies.hpp"
#include "UnknownHandleManager.hpp"

namespace helics
{
/** class defining the common information for a federate*/
class BasicFedInfo
{
  public:
    const std::string name;  //!< name of the federate
    global_federate_id_t global_id;  //!< the identification code for the federate
    route_id_t route_id;  //!< the routing information for data to be sent to the federate
    global_broker_id_t parent;  //!< the id of the parent broker/core
    bool _disconnected = false;
    explicit BasicFedInfo (const std::string &fedname) : name (fedname){};
};

/** class defining the common information about a broker federate*/
class BasicBrokerInfo
{
  public:
    const std::string name;  //!< the name of the broker

    global_broker_id_t global_id;  //!< the global identifier for the broker
    route_id_t route_id;  //!< the identifier for the route to take to the broker
    global_broker_id_t parent;  //!< the id of the parent broker/core
    bool _initRequested = false;  //!< flag indicating the broker has requesting initialization
    bool _disconnected = false;  //!< flag indicating that the broker has disconnected
    bool _hasTimeDependency = false;  //!< flag indicating that a broker has endpoints it is coordinating
    bool _core = false;  //!< if set to true the broker is a core false is a broker;
    bool _nonLocal = false;  //!< indicator that the broker has a subbroker as a parent.
    bool _route_key = false;  //!< indicator that the broker has a unique route id
    std::string routeInfo;  //!< string describing the connection information for the route
    explicit BasicBrokerInfo (const std::string &brokerName) : name (brokerName){};
};

class TimeCoordinator;
class Logger;
class TimeoutMonitor;

/** class implementing most of the functionality of a generic broker
Basically acts as a router for information,  deals with stuff internally if it can and sends higher up if it can't
or does something else if it is the root of the tree
*/
class CoreBroker : public Broker, public BrokerBase
{
  protected:
    bool _gateway = false;  //!< set to true if this broker should act as a gateway.
  private:
    std::atomic<bool> _isRoot{false};  //!< set to true if this object is a root broker
    bool isRootc = false;
    int routeCount = 1;  //!< counter for creating new routes;
    DualMappedVector<BasicFedInfo, std::string, global_federate_id_t> _federates;  //!< container for all federates
    DualMappedVector<BasicBrokerInfo, std::string, global_broker_id_t>
      _brokers;  //!< container for all the broker information
    std::string previous_local_broker_identifier;  //!< the previous identifier in case a rename is required

    HandleManager handles;  //!< structure for managing handles and search operations on handles
    UnknownHandleManager unknownHandles;  //!< structure containing unknown targeted handles
    std::vector<std::pair<std::string, global_federate_id_t>>
      delayedDependencies;  //!< set of dependencies that need to be created on init
    std::unordered_map<global_federate_id_t, federate_id_t>
      global_id_translation;  //!< map to translate global ids to local ones
    std::unordered_map<global_federate_id_t, route_id_t>
      routing_table;  //!< map for external routes  <global federate id, route id>
    std::unordered_map<std::string, route_id_t>
      knownExternalEndpoints;  //!< external map for all known external endpoints with names and route
    std::unordered_map<std::string, std::string> global_values;  //!< storage for global values
    std::mutex name_mutex_;  //!< mutex lock for name and identifier
    std::atomic<int> queryCounter{1};  // counter for active queries going to the local API
    DelayedObjects<std::string> ActiveQueries;  //!< holder for
    JsonMapBuilder fedMap;  //!< builder for the federate_map
    std::vector<ActionMessage> fedMapRequestors;  //!< list of requesters for the active federate map
    JsonMapBuilder depMap;  //!< builder for the dependency graph
    std::vector<ActionMessage> depMapRequestors;  //!< list of requesters for the dependency graph
    JsonMapBuilder dataflowMap;  //!< builder for the dependency graph
    std::vector<ActionMessage> dataflowMapRequestors;  //!< list of requesters for the dependency graph

    TriggerVariable disconnection;  //!< controller for the disconnection process
    std::unique_ptr<TimeoutMonitor> timeoutMon;  //!< class to handle timeouts and disconnection notices
    std::atomic<uint16_t> nextAirLock{0};  //!< the index of the next airlock to use
    std::array<AirLock<stx::any>, 3> dataAirlocks;  //!< airlocks for updating filter operators and other functions
  private:
    /** function that processes all the messages
    @param[in] command -- the message to process
    */
    virtual void processCommand (ActionMessage &&command) override;
    /** function to process a priority command independent of the main queue
    @detailed called from addMessage function which detects if the command is a priority command
    this mainly deals with some of the registration functions
    @param[in] command the command to process
    */
    void processPriorityCommand (ActionMessage &&command) override;

    /** process configure commands for the broker*/
    void processBrokerConfigureCommands (ActionMessage &cmd);

    SimpleQueue<ActionMessage>
      delayTransmitQueue;  //!< FIFO queue for transmissions to the root that need to be delayed for a certain time
    /* function to transmit the delayed messages*/
    void transmitDelayedMessages ();
    /**function for routing a message,  it will override the destination id with the specified argument
     */
    void routeMessage (ActionMessage &cmd, global_federate_id_t dest);
    void routeMessage (ActionMessage &&cmd, global_federate_id_t dest);
    /** function for routing a message from based on the destination specified in the ActionMessage*/
    void routeMessage (const ActionMessage &cmd);
    void routeMessage (const ActionMessage &&cmd);
    /** transmit a message to the parent or root */
    void transmitToParent (ActionMessage &&cmd);
    /**/
    route_id_t fillMessageRouteInformation (ActionMessage &mess);

    /** handle initialization operations*/
    void executeInitializationOperations ();
    /** get an index for an airlock, function is threadsafe*/
    uint16_t getNextAirlockIndex ();

  public:
    /** connect the core to its broker
    @details should be done after initialization has complete*/
    virtual bool connect () override final;
    /** disconnect the broker from any other brokers and communications
     */
    virtual void disconnect () override final;
    /** unregister the broker from the factory find methods*/
    void unregister ();
    /** disconnect the broker from any other brokers and communications
    **if the flag is set it should not do the unregister step of the disconnection, if this is set it is presumed
    the unregistration has already happened or it will be taken care of manually
    */
    virtual void processDisconnect (bool skipUnregister = false) override final;
    /** check if the broker is connected*/
    virtual bool isConnected () const override final;
    /** set the broker to be a root broker
    @details only valid before the initialization function is called*/
    virtual void setAsRoot () override final;
    /** return true if the broker is a root broker
     */
    virtual bool isRoot () const override final { return _isRoot; };

    virtual bool isOpenToNewFederates () const override;
    /** display the help for command line arguments on the broker*/
    static void displayHelp ();

    virtual void setLoggingCallback (
      const std::function<void(int, const std::string &, const std::string &)> &logFunction) override final;

    virtual bool waitForDisconnect (int msToWait = -1) const override final;

  private:
    /** implementation details of the connection process
     */
    virtual bool brokerConnect () = 0;
    /** implementation details of the disconnection process
     */
    virtual void brokerDisconnect () = 0;

  protected:
    /** this function is the one that will change for various flavors of broker communication
    @details it takes a route info- a code of where to send the data and an action message
    and proceeds to transmit it to the appropriate location
    @param[in] route -the identifier for the routing information
    @param[in] command the actionMessage to transmit
    */
    virtual void transmit (route_id_t route, const ActionMessage &command) = 0;
    /** this function is the one that will change for various flavors of broker communication
    @details it takes a route info- a code of where to send the data and an action message
    and proceeds to transmit it to the appropriate location, this variant does a move operation instead of copy
    @param[in] route -the identifier for the routing information
    @param[in] command the actionMessage to transmit
    */
    virtual void transmit (route_id_t route, ActionMessage &&command) = 0;
    /** add a route to the type specific routing information and establish the connection
    @details add a route to a table, the connection information is contained in the string with the described
    identifier
    @param[in] route_id  the identifier for the route
    @param[in] routeInfo  a string describing the connection info
    */
    virtual void addRoute (route_id_t route_id, const std::string &routeInfo) = 0;
    /** remove or disconnect a route from use
    @param route_id the identification of the route
    */
    virtual void removeRoute (route_id_t route_id) = 0;

  public:
    /**default constructor
    @param setAsRootBroker  set to true to indicate this object is a root broker*/
    explicit CoreBroker (bool setAsRootBroker = false) noexcept;
    /** constructor to set the name of the broker*/
    explicit CoreBroker (const std::string &broker_name);
    /** destructor*/
    virtual ~CoreBroker ();
    /** start up the broker with an initialization string containing commands and parameters*/
    virtual void initialize (const std::string &initializationString) override final;
    /** initialize from command line arguments
     */
    virtual void initializeFromArgs (int argc, const char *const *argv) override;

    /** check if all the local federates are ready to be initialized
    @return true if everyone is ready, false otherwise
    */
    bool allInitReady () const;
    bool allDisconnected () const;
    /** set the local identification string for the broker*/
    void setIdentifier (const std::string &name);
    /** get the local identification for the broker*/
    virtual const std::string &getIdentifier () const override final { return identifier; }
    virtual const std::string &getAddress () const override final;
    virtual void setLoggingLevel (int logLevel) override final;
    virtual std::string query (const std::string &target, const std::string &queryStr) override final;
    virtual void setGlobal (const std::string &valueName, const std::string &value) override final;
    virtual void makeConnections (const std::string &file) override final;
    virtual void dataLink (const std::string &publication, const std::string &input) override final;

    virtual void addSourceFilterToEndpoint (const std::string &filter, const std::string &endpoint) override final;

    virtual void
    addDestinationFilterToEndpoint (const std::string &filter, const std::string &endpoint) override final;

  private:
    /** check if we can remove some dependencies*/
    void checkDependencies ();
    /** find any existing publishers for a subscription*/
    void FindandNotifyInputTargets (BasicHandleInfo &handleInfo);
    void FindandNotifyPublicationTargets (BasicHandleInfo &handleInfo);

    void FindandNotifyFilterTargets (BasicHandleInfo &handleInfo);
    void FindandNotifyEndpointTargets (BasicHandleInfo &handleInfo);

    void checkForNamedInterface (ActionMessage &command);
    /** answer a query or route the message the appropriate location*/
    void processQuery (const ActionMessage &m);
    /** answer a query or route the message the appropriate location*/
    void processQueryResponse (const ActionMessage &m);
    /** generate an answer to a local query*/
    void processLocalQuery (const ActionMessage &m);
    /** generate an actual response string to a query*/
    std::string generateQueryAnswer (const std::string &query);
    /** locate the route to take to a particular federate*/
    route_id_t getRoute (global_federate_id_t fedid) const;
    /** locate the route to take to a particular federate*/
    route_id_t getRoute (int32_t fedid) const { return getRoute (global_federate_id_t (fedid)); }

    const BasicBrokerInfo *getBrokerById (global_broker_id_t brokerid) const;

    BasicBrokerInfo *getBrokerById (global_broker_id_t brokerid);

    void addLocalInfo (BasicHandleInfo &handleInfo, const ActionMessage &m);
    void addPublication (ActionMessage &m);
    void addInput (ActionMessage &m);
    void addEndpoint (ActionMessage &m);
    void addFilter (ActionMessage &m);
    //   bool updateSourceFilterOperator (ActionMessage &m);
    /** generate a JSON string containing the federate/broker/Core Map*/
    void initializeFederateMap ();
    /** generate a JSON string containing the dependency information for all federation object*/
    void initializeDependencyGraph ();
    /** generate a json string containing the data flow information for all federation object*/
    void initializeDataFlowGraph ();

    /** send an error code to all direct cores*/
    void sendErrorToImmediateBrokers (int error_code);
    /** send a disconnect message to time dependencies and child brokers*/
    void sendDisconnect ();
    /** generate a string about the federation summarizing connections*/
    std::string generateFederationSummary () const;
    /** label the broker and all children as disconnected*/
    void labelAsDisconnected (global_broker_id_t broker);

    friend class TimeoutMonitor;
};

}  // namespace helics
