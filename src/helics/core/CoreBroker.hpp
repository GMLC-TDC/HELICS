/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../common/JsonBuilder.hpp"
#include "ActionMessage.hpp"
#include "BasicHandleInfo.hpp"
#include "Broker.hpp"
#include "BrokerBase.hpp"
#include "HandleManager.hpp"
#include "TimeDependencies.hpp"
#include "UnknownHandleManager.hpp"
#include "federate_id_extra.hpp"
#include "gmlc/concurrency/DelayedObjects.hpp"
#include "gmlc/concurrency/TriggerVariable.hpp"
#include "gmlc/containers/AirLock.hpp"
#include "gmlc/containers/DualMappedVector.hpp"
#include "gmlc/containers/SimpleQueue.hpp"
#include "helics/external/any.hpp"

#include <array>
#include <atomic>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace helics {

/** enumeration of possible states of a remote federate or broker*/
enum class connection_state : std::uint8_t {
    connected = 0,
    init_requested = 1,
    operating = 2,
    error = 40,
    request_disconnect = 48,
    disconnected = 50
};

/** class defining the common information for a federate*/
class BasicFedInfo {
  public:
    const std::string name;  //!< name of the federate
    global_federate_id global_id;  //!< the identification code for the federate
    route_id route;  //!< the routing information for data to be sent to the federate
    global_broker_id parent;  //!< the id of the parent broker/core
    connection_state state{connection_state::connected};
    bool nonCounting{false};  // indicator the federate shouldn't count toward limits or total
    explicit BasicFedInfo(const std::string& fedname): name(fedname) {}
};

/** class defining the common information about a broker federate*/
class BasicBrokerInfo {
  public:
    const std::string name;  //!< the name of the broker

    global_broker_id global_id;  //!< the global identifier for the broker
    route_id route;  //!< the identifier for the route to take to the broker
    global_broker_id parent;  //!< the id of the parent broker/core

    connection_state state{
        connection_state::connected};  //!< specify the current status of the broker

    bool _hasTimeDependency{
        false};  //!< flag indicating that a broker has general endpoints it is coordinating
    bool _core{false};  //!< if set to true the broker is a core false is a broker;
    bool _nonLocal{false};  //!< indicator that the broker has a subbroker as a parent.
    bool _route_key{false};  //!< indicator that the broker has a unique route id
    bool _sent_disconnect_ack{false};  //!< indicator that the disconnect ack has been sent
    bool _disable_ping{false};  //!< indicator that the broker doesn't respond to pings
    // 1 byte gap
    std::string routeInfo;  //!< string describing the connection information for the route
    explicit BasicBrokerInfo(const std::string& brokerName): name(brokerName) {}
};

class TimeCoordinator;
class Logger;
class TimeoutMonitor;

/** class implementing most of the functionality of a generic broker
Basically acts as a router for information,  deals with stuff internally if it can and sends higher
up if it can't or does something else if it is the root of the tree
*/
class CoreBroker: public Broker, public BrokerBase {
  protected:
    bool _gateway = false;  //!< set to true if this broker should act as a gateway.
  private:
    std::atomic<bool> _isRoot{false};  //!< set to true if this object is a root broker
    bool isRootc{false};
    bool connectionEstablished{false};  //!< the setup has been received by the core loop thread
    int routeCount = 1;  //!< counter for creating new routes;
    gmlc::containers::DualMappedVector<BasicFedInfo, std::string, global_federate_id>
        _federates;  //!< container for all federates
    gmlc::containers::DualMappedVector<BasicBrokerInfo, std::string, global_broker_id>
        _brokers;  //!< container for all the broker information
    std::string
        previous_local_broker_identifier;  //!< the previous identifier in case a rename is required

    HandleManager handles;  //!< structure for managing handles and search operations on handles
    UnknownHandleManager unknownHandles;  //!< structure containing unknown targeted handles
    std::vector<std::pair<std::string, global_federate_id>>
        delayedDependencies;  //!< set of dependencies that need to be created on init
    std::unordered_map<global_federate_id, local_federate_id>
        global_id_translation;  //!< map to translate global ids to local ones
    std::unordered_map<global_federate_id, route_id>
        routing_table;  //!< map for external routes  <global federate id, route id>
    std::unordered_map<std::string, route_id>
        knownExternalEndpoints;  //!< external map for all known external endpoints with names and
                                 //!< route
    std::unordered_map<std::string, std::string> global_values;  //!< storage for global values
    std::mutex name_mutex_;  //!< mutex lock for name and identifier
    std::atomic<int> queryCounter{1};  // counter for active queries going to the local API
    gmlc::concurrency::DelayedObjects<std::string> activeQueries;  //!< holder for active queries
    /// holder for the query map builder information
    std::vector<std::tuple<JsonMapBuilder, std::vector<ActionMessage>, bool>> mapBuilders;
    /// timeout manager for queries
    std::deque<std::pair<int32_t, decltype(std::chrono::steady_clock::now())>> queryTimeouts;

    std::vector<ActionMessage> earlyMessages;  //!< list of messages that came before connection
    gmlc::concurrency::TriggerVariable disconnection;  //!< controller for the disconnection process
    std::unique_ptr<TimeoutMonitor>
        timeoutMon;  //!< class to handle timeouts and disconnection notices
    std::atomic<uint16_t> nextAirLock{0};  //!< the index of the next airlock to use
    std::array<gmlc::containers::AirLock<stx::any>, 3>
        dataAirlocks;  //!< airlocks for updating filter operators and other functions
  private:
    /** function that processes all the messages
    @param command -- the message to process
    */
    virtual void processCommand(ActionMessage&& command) override;
    /** function to process a priority command independent of the main queue
    @details called from addMessage function which detects if the command is a priority command
    this mainly deals with some of the registration functions
    @param command the command to process
    */
    void processPriorityCommand(ActionMessage&& command) override;

    /** process configure commands for the broker*/
    void processBrokerConfigureCommands(ActionMessage& cmd);

    gmlc::containers::SimpleQueue<ActionMessage>
        delayTransmitQueue;  //!< FIFO queue for transmissions to the root that need to be delayed
                             //!< for a certain time
    /* function to transmit the delayed messages*/
    void transmitDelayedMessages();
    /**function for routing a message,  it will override the destination id with the specified
     * argument
     */
    void routeMessage(ActionMessage& cmd, global_federate_id dest);
    void routeMessage(ActionMessage&& cmd, global_federate_id dest);
    /** function for routing a message from based on the destination specified in the
     * ActionMessage*/
    void routeMessage(const ActionMessage& cmd);
    void routeMessage(ActionMessage&& cmd);
    /** transmit a message to the parent or root */
    void transmitToParent(ActionMessage&& cmd);
    /** propagate an error message or escalate it depending on settings*/
    void propagateError(ActionMessage&& cmd);
    /** broadcast a message to all immediate brokers*/
    void broadcast(ActionMessage& cmd);
    /**/
    route_id fillMessageRouteInformation(ActionMessage& mess);

    /** handle initialization operations*/
    void executeInitializationOperations();
    /** get an index for an airlock, function is threadsafe*/
    uint16_t getNextAirlockIndex();
    /** verify the broker key contained in a message
    @return false if the keys do not match*/
    bool verifyBrokerKey(ActionMessage& mess) const;
    /** verify the broker key contained in a string
    @return false if the keys do not match*/
    bool verifyBrokerKey(const std::string& key) const;

  public:
    /** connect the core to its broker
    @details should be done after initialization has complete*/
    virtual bool connect() override final;
    /** disconnect the broker from any other brokers and communications
     */
    virtual void disconnect() override final;
    /** unregister the broker from the factory find methods*/
    void unregister();
    /** disconnect the broker from any other brokers and communications
    **if the flag is set it should not do the unregister step of the disconnection, if this is set
    it is presumed the unregistration has already happened or it will be taken care of manually
    */
    virtual void processDisconnect(bool skipUnregister = false) override final;
    /** check if the broker is connected*/
    virtual bool isConnected() const override final;
    /** set the broker to be a root broker
    @details only valid before the initialization function is called*/
    virtual void setAsRoot() override final;
    /** return true if the broker is a root broker
     */
    virtual bool isRoot() const override final { return _isRoot; };

    virtual bool isOpenToNewFederates() const override;

    virtual void
        setLoggingCallback(const std::function<void(int, const std::string&, const std::string&)>&
                               logFunction) override final;

    virtual bool waitForDisconnect(
        std::chrono::milliseconds msToWait = std::chrono::milliseconds(0)) const override final;

    virtual void setTimeBarrier(Time barrierTime) override final;

    virtual void clearTimeBarrier() override final;

    virtual void globalError(int32_t errorCode, const std::string& errorString) override final;

  private:
    /** implementation details of the connection process
     */
    virtual bool brokerConnect() = 0;
    /** implementation details of the disconnection process
     */
    virtual void brokerDisconnect() = 0;

  protected:
    /** this function is the one that will change for various flavors of broker communication
    @details it takes a route info- a code of where to send the data and an action message
    and proceeds to transmit it to the appropriate location
    @param route -the identifier for the routing information
    @param command the actionMessage to transmit
    */
    virtual void transmit(route_id route, const ActionMessage& command) = 0;
    /** this function is the one that will change for various flavors of broker communication
    @details it takes a route info- a code of where to send the data and an action message
    and proceeds to transmit it to the appropriate location, this variant does a move operation
    instead of copy
    @param route -the identifier for the routing information
    @param command the actionMessage to transmit
    */
    virtual void transmit(route_id route, ActionMessage&& command) = 0;
    /** add a route to the type specific routing information and establish the connection
    @details add a route to a table, the connection information is contained in the string with the
    described identifier
    @param rid the identification of the route
    @param interfaceId an interface id code that can be used to identify the interface route should
    be added to, in most cases this should be zero since there is only one interface
    @param routeInfo a string containing the information necessary to connect
    */
    virtual void addRoute(route_id rid, int interfaceId, const std::string& routeInfo) = 0;
    /** remove or disconnect a route from use
    @param rid the identification of the route
    */
    virtual void removeRoute(route_id rid) = 0;

  public:
    /**default constructor
    @param setAsRootBroker  set to true to indicate this object is a root broker*/
    explicit CoreBroker(bool setAsRootBroker = false) noexcept;
    /** constructor to set the name of the broker*/
    explicit CoreBroker(const std::string& broker_name);
    /** destructor*/
    virtual ~CoreBroker();
    /** start up the broker with an initialization string containing commands and parameters*/
    virtual void configure(const std::string& configureString) override final;
    /** initialize from command line arguments
     */
    virtual void configureFromArgs(int argc, char* argv[]) override final;
    /** initialize from command line arguments in a vector*/
    virtual void configureFromVector(std::vector<std::string> args) override final;

    /** check if all the local federates are ready to be initialized
    @return true if everyone is ready, false otherwise
    */
    bool allInitReady() const;
    /** get a value for the summary connection status of all the connected systems*/
    connection_state getAllConnectionState() const;

    /** set the local identification string for the broker*/
    void setIdentifier(const std::string& name);
    /** get the local identification for the broker*/
    virtual const std::string& getIdentifier() const override final { return identifier; }
    virtual const std::string& getAddress() const override final;
    virtual void setLoggingLevel(int logLevel) override final;
    virtual void setLogFile(const std::string& lfile) override final;
    virtual std::string
        query(const std::string& target,
              const std::string& queryStr,
              helics_sequencing_mode mode = helics_sequencing_mode_fast) override final;
    virtual void setGlobal(const std::string& valueName, const std::string& value) override final;
    virtual void makeConnections(const std::string& file) override final;
    virtual void dataLink(const std::string& publication, const std::string& input) override final;

    virtual void addSourceFilterToEndpoint(const std::string& filter,
                                           const std::string& endpoint) override final;

    virtual void addDestinationFilterToEndpoint(const std::string& filter,
                                                const std::string& endpoint) override final;

  protected:
    virtual std::shared_ptr<helicsCLI11App> generateCLI() override;

  private:
    int getCountableFederates() const;
    /** check if we can remove some dependencies*/
    void checkDependencies();
    /** find any existing publishers for a subscription*/
    void FindandNotifyInputTargets(BasicHandleInfo& handleInfo);
    void FindandNotifyPublicationTargets(BasicHandleInfo& handleInfo);

    void FindandNotifyFilterTargets(BasicHandleInfo& handleInfo);
    void FindandNotifyEndpointTargets(BasicHandleInfo& handleInfo);
    /** process a disconnect message*/
    void processDisconnect(ActionMessage& command);
    /** process an error message*/
    void processError(ActionMessage& command);
    /** disconnect a broker/core*/
    void disconnectBroker(BasicBrokerInfo& brk);
    /** mark this broker and all other that have this as a parent as disconnected*/
    void markAsDisconnected(global_broker_id brkid);
    /** check to make sure there are no inflight queries that need to be resolved*/
    void checkInFlightQueries(global_broker_id brkid);
    /** run a check for a named interface*/
    void checkForNamedInterface(ActionMessage& command);
    /** remove a named target from an interface*/
    void removeNamedTarget(ActionMessage& command);
    /** handle the processing for a query command*/
    void processQueryCommand(ActionMessage& cmd);
    /** answer a query or route the message the appropriate location*/
    void processQuery(ActionMessage& m);

    /** manage query timeouts*/
    void checkQueryTimeouts();
    /** answer a query or route the message the appropriate location*/
    void processQueryResponse(const ActionMessage& m);
    /** generate an answer to a local query*/
    void processLocalQuery(const ActionMessage& m);
    /** generate an actual response string to a query*/
    std::string generateQueryAnswer(const std::string& request, bool force_ordering);
    /** generate a list of names of interfaces from a list of global_ids in a string*/
    std::string getNameList(std::string gidString) const;
    /** locate the route to take to a particular federate*/
    route_id getRoute(global_federate_id fedid) const;
    /** locate the route to take to a particular federate*/
    route_id getRoute(int32_t fedid) const { return getRoute(global_federate_id(fedid)); }

    const BasicBrokerInfo* getBrokerById(global_broker_id brokerid) const;

    BasicBrokerInfo* getBrokerById(global_broker_id brokerid);

    void addLocalInfo(BasicHandleInfo& handleInfo, const ActionMessage& m);
    void addPublication(ActionMessage& m);
    void addInput(ActionMessage& m);
    void addEndpoint(ActionMessage& m);
    void addFilter(ActionMessage& m);

    //   bool updateSourceFilterOperator (ActionMessage &m);
    /** generate a JSON string containing one of the data Maps*/
    void initializeMapBuilder(const std::string& request,
                              std::uint16_t index,
                              bool reset,
                              bool force_ordering);

    std::string generateGlobalStatus(JsonMapBuilder& builder);

    /** send an error code to all direct cores*/
    void sendErrorToImmediateBrokers(int errorCode);
    /** send a disconnect message to time dependencies and child brokers*/
    void sendDisconnect();
    /** generate a string about the federation summarizing connections*/
    std::string generateFederationSummary() const;
    /** label the broker and all children as disconnected*/
    void labelAsDisconnected(global_broker_id brkid);

    /** generate a time barrier request*/
    void generateTimeBarrier(ActionMessage& m);
    int generateMapObjectCounter() const;
    friend class TimeoutMonitor;
};

}  // namespace helics
