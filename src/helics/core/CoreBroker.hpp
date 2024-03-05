/*
Copyright (c) 2017-2024,
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
#include "FederateIdExtra.hpp"
#include "HandleManager.hpp"
#include "TimeDependencies.hpp"
#include "UnknownHandleManager.hpp"
#include "gmlc/concurrency/DelayedObjects.hpp"
#include "gmlc/concurrency/TriggerVariable.hpp"
#include "gmlc/containers/AirLock.hpp"
#include "gmlc/containers/DualStringMappedVector.hpp"
#include "gmlc/containers/SimpleQueue.hpp"

#include <any>
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
enum class ConnectionState : std::uint8_t {
    CONNECTED = 0,
    INIT_REQUESTED = 1,
    OPERATING = 2,
    ERROR_STATE = 40,
    REQUEST_DISCONNECT = 48,
    DISCONNECTED = 50
};

/// forward declaration of QueryReuse
enum class QueryReuse : std::uint8_t;

/** class defining the common information for a federate*/
struct BasicFedInfo {
    const std::string name;  //!< name of the federate
    GlobalFederateId global_id;  //!< the identification code for the federate
    route_id route;  //!< the routing information for data to be sent to the federate
    GlobalBrokerId parent;  //!< the id of the parent broker/core
    ConnectionState state{ConnectionState::CONNECTED};
    bool nonCounting{false};  //!< indicator the federate shouldn't count toward limits or total
    bool observer{false};  //!, indicator that the federate is an observer only
    bool dynamic{false};  //!< indicator that the federate joined dynamically
    bool reentrant{false};  //!< indicator that the federate can be reentrant
    explicit BasicFedInfo(std::string_view fedname): name(fedname) {}
};

/** class defining the common information about a broker federate*/
class BasicBrokerInfo {
  public:
    const std::string name;  //!< the name of the broker

    GlobalBrokerId global_id;  //!< the global identifier for the broker
    route_id route;  //!< the identifier for the route to take to the broker
    GlobalBrokerId parent;  //!< the id of the parent broker/core
    /// specify the current status of the broker
    ConnectionState state{ConnectionState::CONNECTED};
    /** flag indicating that a broker has general endpoints it is coordinating */
    bool _hasTimeDependency{false};
    bool _core{false};  //!< if set to true the broker is a core, false is a broker
    bool _nonLocal{false};  //!< indicator that the broker has a subbroker as a parent.
    bool _route_key{false};  //!< indicator that the broker has a unique route id
    bool _sent_disconnect_ack{false};  //!< indicator that the disconnect ack has been sent
    bool _disable_ping{false};  //!< indicator that the broker doesn't respond to pings
    bool _observer{false};  //!< indicator that the broker is an observer
    bool initIterating{false};  //!< indicator that initIteration was requested
    std::string routeInfo;  //!< string describing the connection information for the route
    explicit BasicBrokerInfo(std::string_view brokerName): name(brokerName) {}
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
    bool initIterating{false};  //!< using init iterations in some cores
    int routeCount = 1;  //!< counter for creating new routes;
    /// container for all federates
    gmlc::containers::
        DualStringMappedVector<BasicFedInfo, GlobalFederateId, reference_stability::unstable>
            mFederates;
    /// container for all the broker information
    gmlc::containers::
        DualStringMappedVector<BasicBrokerInfo, GlobalBrokerId, reference_stability::unstable>
            mBrokers;
    /// the previous identifier in case a rename is required
    std::string mPreviousLocalBrokerIdentifier;

    HandleManager handles;  //!< structure for managing handles and search operations on handles
    UnknownHandleManager unknownHandles;  //!< structure containing unknown targeted handles
    /// set of dependencies that need to be created on init
    std::vector<std::pair<std::string, GlobalFederateId>> delayedDependencies;
    /// map to translate global ids to local ones
    std::unordered_map<GlobalFederateId, LocalFederateId> global_id_translation;
    /// map for external routes  <global federate id, route id>
    std::unordered_map<GlobalFederateId, route_id> routing_table;
    /// external map for all known external endpoints with names and route
    std::unordered_map<std::string, route_id> knownExternalEndpoints;
    std::unordered_map<std::string, std::string> global_values;  //!< storage for global values
    std::unordered_map<std::string, std::int64_t> renamers;  //!< storage for counting federates
    std::mutex name_mutex_;  //!< mutex lock for name and identifier
    std::atomic<int> queryCounter{1};  // counter for active queries going to the local API
    bool force_connection{false};
    gmlc::concurrency::DelayedObjects<std::string> activeQueries;  //!< holder for active queries
    /// holder for the query map builder information
    std::vector<std::tuple<fileops::JsonMapBuilder, std::vector<ActionMessage>, QueryReuse>>
        mapBuilders;
    /// timeout manager for queries
    std::deque<std::pair<int32_t, decltype(std::chrono::steady_clock::now())>> queryTimeouts;

    std::vector<ActionMessage> earlyMessages;  //!< list of messages that came before connection
    gmlc::concurrency::TriggerVariable disconnection;  //!< controller for the disconnection process
    /// class to handle timeouts and disconnection notices
    std::unique_ptr<TimeoutMonitor> timeoutMon;
    std::atomic<uint16_t> nextAirLock{0};  //!< the index of the next airlock to use
    /// airlocks for updating filter operators and other functions
    std::array<gmlc::containers::AirLock<std::any>, 3> dataAirlocks;
    // variables for a time logging federate
    std::string
        mTimeMonitorFederate;  //!< name of the federate to use for logging time and time markers
    GlobalFederateId mTimeMonitorFederateId{};  //!< id of the timing federate
    /// local id for a special receiving destination
    GlobalFederateId mTimeMonitorLocalFederateId{};
    Time mTimeMonitorPeriod{timeZero};  //!< period to display the time logging
    Time mTimeMonitorLastLogTime{Time::minVal()};  //!< the time of the last timing log message
    Time mTimeMonitorCurrentTime{Time::minVal()};  //!< the last time from the timing federate
    std::atomic<double> simTime{mInvalidSimulationTime};  //!< loaded simTime for logging
    Time mNextTimeBarrier{Time::maxVal()};  //!< the last known time barrier
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
    /// FIFO queue for transmissions to the root that need to be delayed for a certain time
    gmlc::containers::SimpleQueue<ActionMessage> delayTransmitQueue;
    /* function to transmit the delayed messages*/
    void transmitDelayedMessages();
    /**function for routing a message,  it will override the destination id with the specified
     * argument
     */
    void routeMessage(ActionMessage& cmd, GlobalFederateId dest);
    void routeMessage(ActionMessage&& cmd, GlobalFederateId dest);
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
    void executeInitializationOperations(bool iterating);
    /** get an index for an airlock, function is threadsafe*/
    uint16_t getNextAirlockIndex();
    /** verify the broker key contained in a message
    @return false if the keys do not match*/
    bool verifyBrokerKey(ActionMessage& mess) const;
    /** verify the broker key contained in a string
    @return false if the keys do not match*/
    bool verifyBrokerKey(std::string_view key) const;

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

    virtual void setLoggingCallback(
        std::function<void(int, std::string_view, std::string_view)> logFunction) override final;

    virtual bool waitForDisconnect(
        std::chrono::milliseconds msToWait = std::chrono::milliseconds(0)) const override final;

    virtual void setTimeBarrier(Time barrierTime) override final;

    virtual void clearTimeBarrier() override final;

    virtual void globalError(int32_t errorCode, std::string_view errorString) override final;

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
    virtual void addRoute(route_id rid, int interfaceId, std::string_view routeInfo) = 0;
    /** remove or disconnect a route from use
    @param rid the identification of the route
    */
    virtual void removeRoute(route_id rid) = 0;

  public:
    /**default constructor
    @param setAsRootBroker  set to true to indicate this object is a root broker*/
    explicit CoreBroker(bool setAsRootBroker = false) noexcept;
    /** constructor to set the name of the broker*/
    explicit CoreBroker(std::string_view broker_name);
    /** destructor*/
    virtual ~CoreBroker();
    /** start up the broker with an initialization string containing commands and parameters*/
    virtual void configure(std::string_view configureString) override final;
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
    ConnectionState getAllConnectionState() const;

    /** set the local identification string for the broker*/
    void setIdentifier(std::string_view name);
    /** get the local identification for the broker*/
    virtual const std::string& getIdentifier() const override final { return identifier; }
    virtual const std::string& getAddress() const override final;
    virtual void setLoggingLevel(int logLevel) override final;
    virtual void setLogFile(std::string_view lfile) override final;
    virtual std::string
        query(std::string_view target,
              std::string_view queryStr,
              HelicsSequencingModes mode = HELICS_SEQUENCING_MODE_FAST) override final;
    virtual void setGlobal(std::string_view valueName, std::string_view value) override final;
    virtual void sendCommand(std::string_view target,
                             std::string_view commandStr,
                             HelicsSequencingModes mode) override final;
    virtual void makeConnections(const std::string& file) override final;
    virtual void linkEndpoints(std::string_view source, std::string_view target) override final;
    virtual void dataLink(std::string_view publication, std::string_view input) override final;

    virtual void addSourceFilterToEndpoint(std::string_view filter,
                                           std::string_view endpoint) override final;

    virtual void addDestinationFilterToEndpoint(std::string_view filter,
                                                std::string_view endpoint) override final;
    virtual void addAlias(std::string_view interfaceKey, std::string_view alias) override final;

  protected:
    virtual std::shared_ptr<helicsCLI11App> generateCLI() override;

    virtual double getSimulationTime() const override;

  private:
    int getCountableFederates() const;
    /** check if we can remove some dependencies*/
    void checkDependencies();

    void connectInterfaces(
        const BasicHandleInfo& origin,
        uint32_t originFlags,
        const BasicHandleInfo& target,
        uint32_t targetFlags,
        std::pair<action_message_def::action_t, action_message_def::action_t> actions);

    /** find any existing publishers for a subscription*/
    void findAndNotifyInputTargets(BasicHandleInfo& handleInfo, const std::string& key);
    void findAndNotifyPublicationTargets(BasicHandleInfo& handleInfo, const std::string& key);

    void findAndNotifyFilterTargets(BasicHandleInfo& handleInfo, const std::string& key);
    void findAndNotifyEndpointTargets(BasicHandleInfo& handleInfo, const std::string& key);

    void findRegexMatch(const std::string& target,
                        InterfaceType type,
                        GlobalHandle handle,
                        uint16_t flags);
    /** process a disconnect message*/
    void processDisconnectCommand(ActionMessage& command);
    /** handle disconnect timing */
    void disconnectTiming(ActionMessage& command);
    /** processBrokerDisconnect  */
    void processBrokerDisconnect(ActionMessage& command, BasicBrokerInfo* brk);
    /** process an error message*/
    void processError(ActionMessage& command);
    /** disconnect a broker/core*/
    void disconnectBroker(BasicBrokerInfo& brk);
    /** mark this broker and all other that have this as a parent as disconnected*/
    void markAsDisconnected(GlobalBrokerId brkid);
    /** check to make sure there are no in-flight queries that need to be resolved*/
    void checkInFlightQueries(GlobalBrokerId brkid);
    /** run a check for a named interface*/
    void checkForNamedInterface(ActionMessage& command);
    /** remove a named target from an interface*/
    void removeNamedTarget(ActionMessage& command);
    /** handle the processing for a query command*/
    void processQueryCommand(ActionMessage& cmd);
    /** answer a query or route the message the appropriate location*/
    void processQuery(ActionMessage& message);
    /** process and init related command*/
    void processInitCommand(ActionMessage& cmd);
    /** manage query timeouts*/
    void checkQueryTimeouts();
    /** answer a query or route the message the appropriate location*/
    void processQueryResponse(const ActionMessage& message);
    /** generate an answer to a local query*/
    void processLocalQuery(const ActionMessage& message);
    /** generate an actual response string to a query*/
    std::string generateQueryAnswer(std::string_view request, bool force_ordering);
    /** run queries that are not dependent on the main loop to be running*/
    std::string quickBrokerQueries(std::string_view request) const;
    /** process a command instruction message*/
    void processCommandInstruction(ActionMessage& message);
    /** process a command instruction targeted at this broker*/
    void processLocalCommandInstruction(ActionMessage& message);
    /** generate a list of names of interfaces from a list of global_ids in a string*/
    std::string getNameList(std::string_view gidString) const;
    /** locate the route to take to a particular federate*/
    route_id getRoute(GlobalFederateId fedid) const;
    /** locate the route to take to a particular federate*/
    route_id getRoute(int32_t fedid) const { return getRoute(GlobalFederateId(fedid)); }

    const BasicBrokerInfo* getBrokerById(GlobalBrokerId brokerid) const;

    BasicBrokerInfo* getBrokerById(GlobalBrokerId brokerid);

    void addLocalInfo(BasicHandleInfo& handleInfo, const ActionMessage& message);
    void addPublication(ActionMessage& message);
    void addInput(ActionMessage& message);
    void addEndpoint(ActionMessage& message);
    void addFilter(ActionMessage& message);
    void addTranslator(ActionMessage& message);
    void addDataSink(ActionMessage& message);

    bool checkInterfaceCreation(ActionMessage& message, InterfaceType type);
    // Handle the registration of new brokers
    void brokerRegistration(ActionMessage&& command);
    /// @brief send an error response to broker registration
    /// @param command the original message that was sent
    /// @param errorCode the error code to reply with
    void sendBrokerErrorAck(ActionMessage& command, std::int32_t errorCode);
    // Helper function for linking interfaces
    void linkInterfaces(ActionMessage& command);
    // Handle the registration of new federates
    void fedRegistration(ActionMessage&& command);
    /// @brief send an error response to fed registration
    /// @param command the original message that was sent
    /// @param errorCode the error code to reply with
    void sendFedErrorAck(ActionMessage& command, std::int32_t errorCode);
    //   bool updateSourceFilterOperator (ActionMessage &m);
    /** generate a JSON string containing one of the data Maps*/
    void initializeMapBuilder(std::string_view request,
                              std::uint16_t index,
                              QueryReuse reuse,
                              bool force_ordering);

    std::string generateGlobalStatus(fileops::JsonMapBuilder& builder);
    /** send an error code to all direct cores*/
    void sendErrorToImmediateBrokers(int errorCode);
    /** send a disconnect message to time dependencies and child brokers*/
    void sendDisconnect(action_message_def::action_t disconnectType);
    /** generate a string about the federation summarizing connections*/
    std::string generateFederationSummary() const;
    /** label the broker and all children as disconnected*/
    void labelAsDisconnected(GlobalBrokerId brkid);

    /** process message for the time monitor*/
    void processTimeMonitorMessage(ActionMessage& message);
    /** load up the monitor federate*/
    void loadTimeMonitor(bool firstLoad, std::string_view newFederate);
    /** generate a time barrier request*/
    void generateTimeBarrier(ActionMessage& message);
    int generateMapObjectCounter() const;
    /** handle the renaming operation*/
    std::string generateRename(std::string_view name);
    friend class TimeoutMonitor;
};

}  // namespace helics
