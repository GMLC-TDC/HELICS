/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "../common/GuardedTypes.hpp"
#include "../common/JsonBuilder.hpp"
#include "ActionMessage.hpp"
#include "BrokerBase.hpp"
#include "Core.hpp"
#include "HandleManager.hpp"
#include "gmlc/concurrency/DelayedObjects.hpp"
#include "gmlc/concurrency/TriggerVariable.hpp"
#include "gmlc/containers/AirLock.hpp"
#include "gmlc/containers/DualMappedPointerVector.hpp"
#include "gmlc/containers/DualMappedVector.hpp"
#include "gmlc/containers/MappedPointerVector.hpp"
#include "gmlc/containers/SimpleQueue.hpp"
#include "helics-time.hpp"
#include "helics/helics-config.h"

#include "json/forwards.h"
#include <any>
#include <array>
#include <atomic>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace helics {
class TestHandle;
class FederateState;

class BasicHandleInfo;
class FilterCoordinator;
class FilterInfo;
class TimeoutMonitor;
enum class handle_type : char;
/** enumeration of possible operating conditions for a federate*/
enum class operation_state : std::uint8_t { operating = 0, error = 5, disconnected = 10 };

/** function to print string for the state*/
const std::string& state_string(operation_state state);

/** helper class for containing some wrapper around a federate for the core*/
class FedInfo {
  public:
    FederateState* fed = nullptr;
    operation_state state{operation_state::operating};

    constexpr FedInfo() = default;
    constexpr explicit FedInfo(FederateState* newfed) noexcept: fed(newfed) {}
    FederateState* operator->() noexcept { return fed; }
    const FederateState* operator->() const noexcept { return fed; }
    operator bool() const noexcept { return (fed != nullptr); }
};

/** base class implementing a standard interaction strategy between federates
@details the CommonCore is virtual class that manages local federates and handles most of the
interaction between federate it is meant to be instantiated for specific inter-federate
communication strategies*/
class CommonCore: public Core, public BrokerBase {
  public:
    /** default constructor*/
    CommonCore() noexcept;
    /**function mainly to match some other object constructors does the same thing as the default
     * constructor*/
    explicit CommonCore(bool arg) noexcept;
    /** construct from a core name*/
    explicit CommonCore(const std::string& coreName);
    /** virtual destructor*/
    virtual ~CommonCore() override;
    virtual void configure(const std::string& configureString) override final;
    virtual void configureFromArgs(int argc, char* argv[]) override final;
    virtual void configureFromVector(std::vector<std::string> args) override final;
    virtual bool isConfigured() const override final;
    virtual bool isOpenToNewFederates() const override final;
    virtual void globalError(LocalFederateId federateID,
                             int errorCode,
                             const std::string& errorString) override final;
    virtual void localError(LocalFederateId federateID,
                            int errorCode,
                            const std::string& errorString) override final;
    virtual void finalize(LocalFederateId federateID) override final;
    virtual void enterInitializingMode(LocalFederateId federateID) override final;
    virtual void setCoreReadyToInit() override final;
    virtual iteration_result
        enterExecutingMode(LocalFederateId federateID,
                           iteration_request iterate = NO_ITERATION) override final;
    virtual LocalFederateId registerFederate(const std::string& name,
                                             const CoreFederateInfo& info) override final;
    virtual const std::string& getFederateName(LocalFederateId federateID) const override final;
    virtual LocalFederateId getFederateId(const std::string& name) const override final;
    virtual int32_t getFederationSize() override final;
    virtual Time timeRequest(LocalFederateId federateID, Time next) override final;
    virtual iteration_time requestTimeIterative(LocalFederateId federateID,
                                                Time next,
                                                iteration_request iterate) override final;
    virtual Time getCurrentTime(LocalFederateId federateID) const override final;
    virtual uint64_t getCurrentReiteration(LocalFederateId federateID) const override final;
    virtual void
        setTimeProperty(LocalFederateId federateID, int32_t property, Time time) override final;
    virtual void setIntegerProperty(LocalFederateId federateID,
                                    int32_t property,
                                    int16_t propertyValue) override final;
    virtual Time getTimeProperty(LocalFederateId federateID, int32_t property) const override final;
    virtual int16_t getIntegerProperty(LocalFederateId federateID,
                                       int32_t property) const override final;
    virtual void setFlagOption(LocalFederateId federateID,
                               int32_t flag,
                               bool flagValue = true) override final;
    virtual bool getFlagOption(LocalFederateId federateID, int32_t flag) const override final;

    virtual InterfaceHandle registerPublication(LocalFederateId federateID,
                                                const std::string& key,
                                                const std::string& type,
                                                const std::string& units) override final;
    virtual InterfaceHandle getPublication(LocalFederateId federateID,
                                           const std::string& key) const override final;
    virtual InterfaceHandle registerInput(LocalFederateId federateID,
                                          const std::string& key,
                                          const std::string& type,
                                          const std::string& units) override final;

    virtual InterfaceHandle getInput(LocalFederateId federateID,
                                     const std::string& key) const override final;

    virtual const std::string& getHandleName(InterfaceHandle handle) const override final;

    virtual void setHandleOption(InterfaceHandle handle,
                                 int32_t option,
                                 int32_t option_value) override final;

    virtual int32_t getHandleOption(InterfaceHandle handle, int32_t option) const override final;
    virtual void closeHandle(InterfaceHandle handle) override final;
    virtual void removeTarget(InterfaceHandle handle,
                              std::string_view targetToRemove) override final;
    virtual void addDestinationTarget(InterfaceHandle handle,
                                      std::string_view dest,
                                      handle_type hint) override final;
    virtual void addSourceTarget(InterfaceHandle handle,
                                 std::string_view name,
                                 handle_type hint) override final;
    virtual const std::string& getDestinationTargets(InterfaceHandle handle) const override final;

    virtual const std::string& getSourceTargets(InterfaceHandle handle) const override final;
    virtual const std::string& getInjectionUnits(InterfaceHandle handle) const override final;
    virtual const std::string& getExtractionUnits(InterfaceHandle handle) const override final;
    virtual const std::string& getInjectionType(InterfaceHandle handle) const override final;
    virtual const std::string& getExtractionType(InterfaceHandle handle) const override final;
    virtual void setValue(InterfaceHandle handle, const char* data, uint64_t len) override final;
    virtual const std::shared_ptr<const SmallBuffer>& getValue(InterfaceHandle handle,
                                                               uint32_t* inputIndex) override final;
    virtual const std::vector<std::shared_ptr<const SmallBuffer>>&
        getAllValues(InterfaceHandle handle) override final;
    virtual const std::vector<InterfaceHandle>&
        getValueUpdates(LocalFederateId federateID) override final;
    virtual InterfaceHandle registerEndpoint(LocalFederateId federateID,
                                             const std::string& name,
                                             const std::string& type) override final;

    virtual InterfaceHandle registerTargetedEndpoint(LocalFederateId federateID,
                                                     const std::string& name,
                                                     const std::string& type) override final;
    virtual InterfaceHandle getEndpoint(LocalFederateId federateID,
                                        const std::string& name) const override final;
    virtual InterfaceHandle registerFilter(const std::string& filterName,
                                           const std::string& type_in,
                                           const std::string& type_out) override final;
    virtual InterfaceHandle registerCloningFilter(const std::string& filterName,
                                                  const std::string& type_in,
                                                  const std::string& type_out) override final;
    virtual InterfaceHandle getFilter(const std::string& name) const override final;
    virtual void addDependency(LocalFederateId federateID,
                               const std::string& federateName) override final;
    virtual void linkEndpoints(const std::string& source, const std::string& dest) override final;
    virtual void makeConnections(const std::string& file) override final;
    virtual void dataLink(const std::string& source, const std::string& target) override final;
    virtual void addSourceFilterToEndpoint(const std::string& filter,
                                           const std::string& endpoint) override final;
    virtual void addDestinationFilterToEndpoint(const std::string& filter,
                                                const std::string& endpoint) override final;
    virtual void
        send(InterfaceHandle sourceHandle, const void* data, uint64_t length) override final;
    virtual void sendAt(InterfaceHandle sourceHandle,
                        Time time,
                        const void* data,
                        uint64_t length) override final;
    virtual void sendTo(InterfaceHandle sourceHandle,
                        std::string_view destination,
                        const void* data,
                        uint64_t length) override final;
    virtual void sendToAt(InterfaceHandle sourceHandle,
                          std::string_view destination,
                          Time time,
                          const void* data,
                          uint64_t length) override final;
    virtual void sendMessage(InterfaceHandle sourceHandle,
                             std::unique_ptr<Message> message) override final;
    virtual uint64_t receiveCount(InterfaceHandle destination) override final;
    virtual std::unique_ptr<Message> receive(InterfaceHandle destination) override final;
    virtual std::unique_ptr<Message> receiveAny(LocalFederateId federateID,
                                                InterfaceHandle& endpoint_id) override final;
    virtual uint64_t receiveCountAny(LocalFederateId federateID) override final;
    virtual void logMessage(LocalFederateId federateID,
                            int logLevel,
                            const std::string& messageToLog) override final;
    virtual void setFilterOperator(InterfaceHandle filter,
                                   std::shared_ptr<FilterOperator> callback) override final;

    /** set the local identification for the core*/
    void setIdentifier(const std::string& name);
    /** get the local identifier for the core*/
    virtual const std::string& getIdentifier() const override final { return identifier; }
    virtual const std::string& getAddress() const override final;
    const std::string& getFederateNameNoThrow(GlobalFederateId federateID) const noexcept;

    /** set the core logging level*/
    virtual void setLoggingLevel(int logLevel) override;
    virtual void setLoggingCallback(
        LocalFederateId federateID,
        std::function<void(int, std::string_view, std::string_view)> logFunction) override final;

    virtual void setLogFile(const std::string& lfile) override final;

    virtual std::string query(const std::string& target, const std::string& queryStr) override;
    virtual void
        setQueryCallback(LocalFederateId federateID,
                         std::function<std::string(std::string_view)> queryFunction) override;
    virtual void setGlobal(const std::string& valueName, const std::string& value) override;
    virtual void sendCommand(const std::string& target,
                             const std::string& commandStr,
                             const std::string& source) override;
    virtual std::pair<std::string, std::string> getCommand(LocalFederateId federateID) override;

    virtual std::pair<std::string, std::string> waitCommand(LocalFederateId federateID) override;

    virtual bool connect() override final;
    virtual bool isConnected() const override final;
    virtual void disconnect() override final;
    virtual bool waitForDisconnect(
        std::chrono::milliseconds msToWait = std::chrono::milliseconds(0)) const override final;
    /** unregister the core from any process find functions*/
    void unregister();
    /**TODO(PT): figure out how to make this non-public, it needs to be called in a lambda function,
     * may need a helper class of some sort*/
    virtual void processDisconnect(bool skipUnregister = false) override final;

    virtual void setInterfaceInfo(InterfaceHandle handle, std::string info) override final;
    virtual const std::string& getInterfaceInfo(InterfaceHandle handle) const override final;

  private:
    /** implementation details of the connection process
     */
    virtual bool brokerConnect() = 0;
    /** implementation details of the disconnection process
     */
    virtual void brokerDisconnect() = 0;

  protected:
    virtual void processCommand(ActionMessage&& command) override final;

    virtual void processPriorityCommand(ActionMessage&& command) override final;

    /** transit an ActionMessage to another core or broker
    @param rid the identifier for the route information to send the message to
    @param command the actionMessage to send*/
    virtual void transmit(route_id rid, const ActionMessage& command) = 0;
    /** transit an ActionMessage to another core or broker
    @param rid the identifier for the route information to send the message to
    @param command the actionMessage to send*/
    virtual void transmit(route_id rid, ActionMessage&& command) = 0;
    /** add a route to whatever internal structure manages the routes
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
    /** get the federate Information from the federateID*/
    FederateState* getFederateAt(LocalFederateId federateID) const;
    /** get the federate Information from the federateID*/
    FederateState* getFederate(const std::string& federateName) const;
    /** get the federate Information from a handle
    @param handle a handle identifier as generated by the one of the functions*/
    FederateState* getHandleFederate(InterfaceHandle handle);
    /** get the basic handle information*/
    const BasicHandleInfo* getHandleInfo(InterfaceHandle handle) const;
    /** get a localEndpoint from the name*/
    const BasicHandleInfo* getLocalEndpoint(const std::string& name) const;
    /** get a filtering function object*/
    FilterCoordinator* getFilterCoordinator(InterfaceHandle handle);
    /** check if all federates managed by the core are ready to enter initialization state*/
    bool allInitReady() const;
    /** check if all connections are disconnected (feds and time dependencies)*/
    bool allDisconnected() const;
    /** get the minimum operating state of the connected federates*/
    operation_state minFederateState() const;

  private:
    /** get the federate Information from the federateID*/
    FederateState* getFederateCore(GlobalFederateId federateID);
    /** get the federate Information from the federateID*/
    FederateState* getFederateCore(const std::string& federateName);
    /** get the federate Information from a handle
    @param handle an identifier as generated by the one of the functions
    @return the federateState pointer object*/
    FederateState* getHandleFederateCore(InterfaceHandle handle);

  private:
    std::string prevIdentifier;  //!< storage for the case of requiring a renaming
    std::map<GlobalFederateId, route_id>
        routing_table;  //!< map for external routes  <global federate id, route id>
    gmlc::containers::SimpleQueue<ActionMessage>
        delayTransmitQueue;  //!< FIFO queue for transmissions to the root that need to be delayed
                             //!< for a certain time
    std::unordered_map<std::string, route_id>
        knownExternalEndpoints;  //!< external map for all known external endpoints with names and
                                 //!< route

    std::unique_ptr<TimeoutMonitor>
        timeoutMon;  //!< class to handle timeouts and disconnection notices
    /** actually transmit messages that were delayed until the core was actually registered*/
    void transmitDelayedMessages();
    /** respond to delayed message with an error*/
    void errorRespondDelayedMessages(const std::string& estring);
    /** actually transmit messages that were delayed for a particular source
    @param source the identifier for the message to transmit
    */
    void transmitDelayedMessages(GlobalFederateId source);

    /**function for doing the actual routing either to a local fed or up the broker chain*/
    void routeMessage(ActionMessage& cmd, GlobalFederateId dest);
    /** function for routing a message from based on the destination specified in the
     * ActionMessage*/
    void routeMessage(const ActionMessage& cmd);

    /**function for doing the actual routing either to a local fed or up the broker chain*/
    void routeMessage(ActionMessage&& cmd, GlobalFederateId dest);
    /** function for routing a message from based on the destination specified in the
     * ActionMessage*/
    void routeMessage(ActionMessage&& cmd);

    /** process any filter or route the message*/
    void processMessageFilter(ActionMessage& cmd);
    /** process a filter message return*/
    void processFilterReturn(ActionMessage& cmd);
    /** process a destination filter message return*/
    void processDestFilterReturn(ActionMessage& command);
    /** create a source filter */
    FilterInfo* createFilter(GlobalBrokerId dest,
                             InterfaceHandle handle,
                             const std::string& key,
                             const std::string& type_in,
                             const std::string& type_out,
                             bool cloning);

    /** check if we can remove some dependencies*/
    void checkDependencies();
    /** deal with a query response addressed to this core*/
    void processQueryResponse(const ActionMessage& m);

    /** handle command with the core itself as a destination at the core*/
    void processCommandsForCore(const ActionMessage& cmd);
    /** process configure commands for the core*/
    void processCoreConfigureCommands(ActionMessage& cmd);
    /** check if a newly registered subscription has a local publication
    if it does return true*/
    bool checkForLocalPublication(ActionMessage& cmd);
    /** get an index for an airlock function is threadsafe*/
    uint16_t getNextAirlockIndex();
    /** load the basic core info into a JSON object*/
    void loadBasicJsonInfo(
        Json::Value& base,
        const std::function<void(Json::Value& fedval, const FedInfo& fed)>& fedLoader) const;
    /** generate a mapbuilder for the federates*/
    void initializeMapBuilder(const std::string& request, std::uint16_t index, bool reset) const;
    /** generate results for core queries*/
    std::string coreQuery(const std::string& queryStr) const;

    /** generate results for some core queries that do not depend on the main processing loop
     * running*/
    std::string quickCoreQueries(const std::string& queryStr) const;

    /** generate the filteredEndpoint query results for a particular federate*/
    std::string filteredEndpointQuery(const FederateState* fed) const;
    /** process a command instruction for the core*/
    void processCommandInstruction(ActionMessage& command);

  private:
    int32_t _global_federation_size = 0;  //!< total size of the federation
    std::atomic<int16_t> delayInitCounter{0};  //!< counter for the number of times the entry to
                                               //!< initialization Mode was explicitly delayed
    shared_guarded<gmlc::containers::MappedPointerVector<FederateState, std::string>>
        federates;  //!< threadsafe local federate information list for external functions
    gmlc::containers::DualMappedVector<FedInfo, std::string, GlobalFederateId>
        loopFederates;  // federate pointers stored for the core loop
    std::atomic<int32_t> messageCounter{
        54};  //!< counter for the number of messages that have been sent, nothing
    //!< magical about 54 just a number bigger than 1 to prevent
    //!< confusion

    ordered_guarded<HandleManager> handles;  //!< local handle information;
    HandleManager loopHandles;  //!< copy of handles to use in the primary processing loop without
                                //!< thread protection
    std::map<int32_t, std::set<int32_t>>
        ongoingFilterProcesses;  //!< sets of ongoing filtered messages
    std::map<int32_t, std::set<int32_t>>
        ongoingDestFilterProcesses;  //!< sets of ongoing destination filter processing

    std::map<int32_t, std::vector<ActionMessage>>
        delayedTimingMessages;  //!< delayedTimingMessages from ongoing Filter actions
    std::atomic<int> queryCounter{
        1};  //!< counter for queries start at 1 so the default value isn't used
    gmlc::concurrency::DelayedObjects<std::string>
        activeQueries;  //!< holder for active queries
                        /// holder for the query map builder information
    mutable std::vector<std::tuple<JsonMapBuilder, std::vector<ActionMessage>, bool>> mapBuilders;
    std::map<InterfaceHandle, std::unique_ptr<FilterCoordinator>>
        filterCoord;  //!< map of all local filters
    // The InterfaceHandle used is here is usually referencing an endpoint
    gmlc::containers::DualMappedPointerVector<FilterInfo,
                                              std::string,
                                              GlobalHandle>
        filters;  //!< storage for all the filters

    std::atomic<uint16_t> nextAirLock{0};  //!< the index of the next airlock to use
    std::array<gmlc::containers::AirLock<std::any>, 4>
        dataAirlocks;  //!< airlocks for updating filter operators and other functions
    gmlc::concurrency::TriggerVariable disconnection;  //!< controller for the disconnection process
  private:
    /** wait for the core to be registered with the broker*/
    bool waitCoreRegistration();
    /** generate the messages to a set of destinations*/
    void generateMessages(ActionMessage& message,
                          const std::vector<std::pair<GlobalHandle, std::string_view>>& targets);
    /** deliver a message to the appropriate location*/
    void deliverMessage(ActionMessage& message);
    /** function to deal with a source filters*/
    ActionMessage& processMessage(ActionMessage& message);
    /** add a new handle to the generic structure
    and return a reference to the basicHandle
    */
    const BasicHandleInfo& createBasicHandle(GlobalFederateId global_federateId,
                                             LocalFederateId local_federateId,
                                             handle_type HandleType,
                                             const std::string& key,
                                             const std::string& type,
                                             const std::string& units,
                                             uint16_t flags = 0);

    /** check if a global id represents a local federate
    @param global_fedid the identifier for the federate
    @return true if it is a local federate*/
    bool isLocal(GlobalFederateId global_fedid) const;
    /** get a route id for a non-local federate
    @param global_fedid the identifier for the federate
    @return parent_route if unknown, otherwise returns the route_id*/
    route_id getRoute(GlobalFederateId global_fedid) const;
    /** process a message for potential additions to the filter ordering
    @param command the message to process
    */
    void processFilterInfo(ActionMessage& command);
    /** function to check for a named interface*/
    void checkForNamedInterface(ActionMessage& command);
    /** function to remove a named target*/
    void removeNamedTarget(ActionMessage& command);
    /** indicate that a handle interface is used and if the used status has changed make sure it is
    indicated in all the needed places*/
    void setAsUsed(BasicHandleInfo* hand);
    /** function to consolidate the registration of interfaces in the core*/
    void registerInterface(ActionMessage& command);
    /** function to handle adding a target to an interface*/
    void addTargetToInterface(ActionMessage& command);
    /** function to deal with removing a target from an interface*/
    void removeTargetFromInterface(ActionMessage& command);
    /** function disconnect a single interface*/
    void disconnectInterface(ActionMessage& command);
    /** organize filters
    @details organize the filter and report and potential warnings and errors
    */
    void organizeFilterOperations();

    /** generate a query response for a federate if possible
    @param fed a pointer to the federateState object to query
    @param queryStr  the string containing the actual query
    @return "#wait" if the lock cannot be granted immediately and no result can be obtained
    otherwise an answer to the query
    */
    std::string federateQuery(const FederateState* fed, const std::string& queryStr) const;

    /** send an error code and message to all the federates*/
    void sendErrorToFederates(int error_code, std::string_view message);
    /** check for a disconnect and take actions if the object can disconnect*/
    bool checkAndProcessDisconnect();
    /** send a disconnect message to time dependencies and child federates*/
    void sendDisconnect();
    /** broadcast a message to all federates*/
    void broadcastToFederates(ActionMessage& cmd);

    friend class TimeoutMonitor;
};

}  // namespace helics
