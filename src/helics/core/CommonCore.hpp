/*
Copyright (c) 2017-2024,
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
#include "FederateIdExtra.hpp"
#include "HandleManager.hpp"
#include "gmlc/concurrency/DelayedObjects.hpp"
#include "gmlc/concurrency/TriggerVariable.hpp"
#include "gmlc/containers/AirLock.hpp"
#include "gmlc/containers/DualStringMappedVector.hpp"
#include "gmlc/containers/MappedPointerVector.hpp"
#include "gmlc/containers/SimpleQueue.hpp"
#include "helics/helics-config.h"
#include "helicsTime.hpp"

#include <any>
#include <array>
#include <atomic>
#include <chrono>
#include <deque>
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
class FilterFederate;
class TranslatorFederate;
class TimeoutMonitor;
enum class InterfaceType : char;
enum class QueryReuse : std::uint8_t;
/** enumeration of possible operating conditions for a federate*/
enum class OperatingState : std::uint8_t { OPERATING = 0, ERROR_STATE = 5, DISCONNECTED = 10 };

/** function to print string for the state*/
const std::string& stateString(OperatingState state);

/** helper class for containing some wrapper around a federate for the core*/
class FedInfo {
  public:
    FederateState* fed = nullptr;
    OperatingState state{OperatingState::OPERATING};

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
    explicit CommonCore(std::string_view coreName);
    /** virtual destructor*/
    virtual ~CommonCore() override;
    virtual void configure(std::string_view configureString) override final;
    virtual void configureFromArgs(int argc, char* argv[]) override final;
    virtual void configureFromVector(std::vector<std::string> args) override final;
    virtual bool isConfigured() const override final;
    virtual bool isOpenToNewFederates() const override final;
    virtual bool hasError() const override final;
    virtual void globalError(LocalFederateId federateID,
                             int errorCode,
                             std::string_view errorString) override final;
    virtual void localError(LocalFederateId federateID,
                            int errorCode,
                            std::string_view errorString) override final;
    virtual int getErrorCode() const override final;
    virtual std::string getErrorMessage() const override final;
    virtual void finalize(LocalFederateId federateID) override final;
    virtual bool enterInitializingMode(LocalFederateId federateID,
                                       IterationRequest request) override final;
    virtual void setCoreReadyToInit() override final;
    virtual iteration_time
        enterExecutingMode(LocalFederateId federateID,
                           IterationRequest iterate = NO_ITERATION) override final;
    virtual LocalFederateId registerFederate(std::string_view name,
                                             const CoreFederateInfo& info) override final;
    virtual const std::string& getFederateName(LocalFederateId federateID) const override final;
    virtual LocalFederateId getFederateId(std::string_view name) const override final;
    virtual int32_t getFederationSize() override final;
    virtual Time timeRequest(LocalFederateId federateID, Time next) override final;
    virtual iteration_time requestTimeIterative(LocalFederateId federateID,
                                                Time next,
                                                IterationRequest iterate) override final;
    virtual void processCommunications(LocalFederateId federateID,
                                       std::chrono::milliseconds msToWait) override final;
    virtual Time getCurrentTime(LocalFederateId federateID) const override final;
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
                                                std::string_view key,
                                                std::string_view type,
                                                std::string_view units) override final;
    virtual InterfaceHandle getPublication(LocalFederateId federateID,
                                           std::string_view key) const override final;
    virtual InterfaceHandle registerInput(LocalFederateId federateID,
                                          std::string_view key,
                                          std::string_view type,
                                          std::string_view units) override final;

    virtual InterfaceHandle getInput(LocalFederateId federateID,
                                     std::string_view key) const override final;

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
                                      InterfaceType hint) override final;
    virtual void addSourceTarget(InterfaceHandle handle,
                                 std::string_view name,
                                 InterfaceType hint) override final;
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
                                             std::string_view name,
                                             std::string_view type) override final;

    virtual InterfaceHandle registerTargetedEndpoint(LocalFederateId federateID,
                                                     std::string_view name,
                                                     std::string_view type) override final;
    virtual InterfaceHandle getEndpoint(LocalFederateId federateID,
                                        std::string_view name) const override final;

    virtual InterfaceHandle registerDataSink(LocalFederateId federateID,
                                             std::string_view name) override final;

    virtual InterfaceHandle getDataSink(LocalFederateId federateID,
                                        std::string_view name) const override final;

    virtual InterfaceHandle registerFilter(std::string_view filterName,
                                           std::string_view type_in,
                                           std::string_view type_out) override final;
    virtual InterfaceHandle registerCloningFilter(std::string_view filterName,
                                                  std::string_view type_in,
                                                  std::string_view type_out) override final;
    virtual InterfaceHandle registerTranslator(std::string_view translatorName,
                                               std::string_view endpointType,
                                               std::string_view units) override final;
    virtual InterfaceHandle getFilter(std::string_view name) const override final;
    virtual InterfaceHandle getTranslator(std::string_view name) const override final;
    virtual void addDependency(LocalFederateId federateID,
                               std::string_view federateName) override final;
    virtual void linkEndpoints(std::string_view source, std::string_view dest) override final;
    virtual void addAlias(std::string_view interfaceKey, std::string_view alias) override final;
    virtual void makeConnections(const std::string& file) override final;
    virtual void dataLink(std::string_view source, std::string_view target) override final;
    virtual void addSourceFilterToEndpoint(std::string_view filter,
                                           std::string_view endpoint) override final;
    virtual void addDestinationFilterToEndpoint(std::string_view filter,
                                                std::string_view endpoint) override final;
    virtual void
        send(InterfaceHandle sourceHandle, const void* data, uint64_t length) override final;
    virtual void sendAt(InterfaceHandle sourceHandle,
                        const void* data,
                        uint64_t length,
                        Time time) override final;
    virtual void sendTo(InterfaceHandle sourceHandle,
                        const void* data,
                        uint64_t length,
                        std::string_view destination) override final;
    virtual void sendToAt(InterfaceHandle sourceHandle,
                          const void* data,
                          uint64_t length,
                          std::string_view destination,
                          Time time) override final;
    virtual void sendMessage(InterfaceHandle sourceHandle,
                             std::unique_ptr<Message> message) override final;
    virtual uint64_t receiveCount(InterfaceHandle destination) override final;
    virtual std::unique_ptr<Message> receive(InterfaceHandle destination) override final;
    virtual std::unique_ptr<Message> receiveAny(LocalFederateId federateID,
                                                InterfaceHandle& endpoint_id) override final;
    virtual uint64_t receiveCountAny(LocalFederateId federateID) override final;
    virtual void logMessage(LocalFederateId federateID,
                            int logLevel,
                            std::string_view messageToLog) override final;
    virtual void setFilterOperator(InterfaceHandle filter,
                                   std::shared_ptr<FilterOperator> callback) override final;
    virtual void
        setTranslatorOperator(InterfaceHandle translator,
                              std::shared_ptr<TranslatorOperator> callbacks) override final;
    virtual void setFederateOperator(LocalFederateId federateID,
                                     std::shared_ptr<FederateOperator> callback) override;
    /** set the local identification for the core*/
    void setIdentifier(std::string_view name);
    /** get the local identifier for the core*/
    virtual const std::string& getIdentifier() const override final;
    virtual const std::string& getAddress() const override final;
    const std::string& getFederateNameNoThrow(GlobalFederateId federateID) const noexcept;

    /** set the core logging level*/
    virtual void setLoggingLevel(int logLevel) override;
    virtual void setLoggingCallback(
        LocalFederateId federateID,
        std::function<void(int, std::string_view, std::string_view)> logFunction) override final;

    virtual void setLogFile(std::string_view lfile) override final;

    virtual std::string query(std::string_view target,
                              std::string_view queryStr,
                              HelicsSequencingModes mode) override;
    virtual void setQueryCallback(LocalFederateId federateID,
                                  std::function<std::string(std::string_view)> queryFunction,
                                  int order) override;
    virtual void setGlobal(std::string_view valueName, std::string_view value) override;
    virtual void sendCommand(std::string_view target,
                             std::string_view commandStr,
                             std::string_view source,
                             HelicsSequencingModes mode) override;
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

    /** check to make sure there are no in-flight queries that need to be resolved before
     * disconnect*/
    void checkInFlightQueriesForDisconnect();

    /** set the local information field of the interface*/
    virtual void setInterfaceInfo(InterfaceHandle handle, std::string_view info) override final;
    /** get the local information field of the interface*/
    virtual const std::string& getInterfaceInfo(InterfaceHandle handle) const override final;

    virtual void setInterfaceTag(InterfaceHandle handle,
                                 std::string_view tag,
                                 std::string_view value) override final;
    virtual const std::string& getInterfaceTag(InterfaceHandle handle,
                                               std::string_view tag) const override final;

    virtual void setFederateTag(LocalFederateId fid,
                                std::string_view tag,
                                std::string_view value) override final;
    virtual const std::string& getFederateTag(LocalFederateId fid,
                                              std::string_view tag) const override final;

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
    virtual void addRoute(route_id rid, int interfaceId, std::string_view routeInfo) = 0;
    /** remove or disconnect a route from use
    @param rid the identification of the route
    */
    virtual void removeRoute(route_id rid) = 0;
    /** get the federate Information from the federateID*/
    FederateState* getFederateAt(LocalFederateId federateID) const;
    /** get the federate Information from the federateID*/
    FederateState* getFederate(std::string_view federateName) const;
    /** get the federate Information from a handle
    @param handle a handle identifier as generated by the one of the functions*/
    FederateState* getHandleFederate(InterfaceHandle handle);
    /** get the basic handle information*/
    const BasicHandleInfo* getHandleInfo(InterfaceHandle handle) const;
    /** get a localEndpoint from the name*/
    const BasicHandleInfo* getLocalEndpoint(std::string_view name) const;

    /** check if all federates managed by the core are ready to enter initialization state*/
    bool allInitReady() const;
    /** check if all connections are disconnected (feds and time dependencies)*/
    bool allDisconnected() const;
    /** get the minimum operating state of the connected federates*/
    OperatingState minFederateState() const;

    virtual double getSimulationTime() const override;

  private:
    /** get the federate Information from the federateID*/
    FederateState* getFederateCore(GlobalFederateId federateID);
    /** get the federate Information from the federateID*/
    FederateState* getFederateCore(std::string_view federateName);
    /** get the federate Information from a handle
    @param handle an identifier as generated by the one of the functions
    @return the federateState pointer object*/
    FederateState* getHandleFederateCore(InterfaceHandle handle);

  private:
    std::atomic<double> simTime{BrokerBase::mInvalidSimulationTime};
    GlobalFederateId keyFed{};
    std::string prevIdentifier;  //!< storage for the case of requiring a renaming
    /** map for external routes  <global federate id, route id> */
    std::map<GlobalFederateId, route_id> routing_table;
    /** FIFO queue for transmissions to the root that need to be delayed for a certain time */
    gmlc::containers::SimpleQueue<ActionMessage> delayTransmitQueue;
    /** external map for all known external endpoints with names and route */
    std::unordered_map<std::string, route_id> knownExternalEndpoints;
    std::vector<std::pair<std::string, std::string>> tags;  //!< storage for user defined tags
    /** class to handle timeouts and disconnection notices */
    std::unique_ptr<TimeoutMonitor> timeoutMon;
    /** actually transmit messages that were delayed until the core was actually registered*/
    void transmitDelayedMessages();
    /** respond to delayed message with an error*/
    void errorRespondDelayedMessages(std::string_view estring);
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
    /** check that a new interface is valid and is allowed to be created*/
    FederateState*
        checkNewInterface(LocalFederateId federateID, std::string_view key, InterfaceType type);
    /** check if we can remove some dependencies*/
    void checkDependencies();
    /** deal with a query response addressed to this core*/
    void processQueryResponse(const ActionMessage& message);
    /** manage query timeouts*/
    void checkQueryTimeouts();
    /** handle command with the core itself as a destination at the core*/
    void processCommandsForCore(const ActionMessage& cmd);
    /** process configure commands for the core*/
    void processCoreConfigureCommands(ActionMessage& cmd);
    /** handle init messages*/
    void processInitRequest(ActionMessage& cmd);
    /** process and exec request command*/
    void processExecRequest(ActionMessage& cmd);
    /** process commands related to disconnect messages*/
    void processDisconnectCommand(ActionMessage& cmd);

    /** process a timing tick message */
    void processTimingTick(ActionMessage& cmd);
    /** handle the processing for a query command*/
    void processQueryCommand(ActionMessage& cmd);
    /** handle logging and error related commands*/
    void processLogAndErrorCommand(ActionMessage& cmd);
    /** handle data linking related commands*/
    void processLinkingCommand(ActionMessage& cmd);
    /** check if a newly registered subscription has a local publication
    if it does return true*/
    bool checkForLocalPublication(ActionMessage& cmd);
    /** get an index for an airlock function is threadsafe*/
    uint16_t getNextAirlockIndex();
    /** load the basic core info into a JSON object*/
    void loadBasicJsonInfo(
        nlohmann::json& base,
        const std::function<void(nlohmann::json& fedval, const FedInfo& fed)>& fedLoader) const;
    /** generate a mapbuilder for the federates
    @param request the query to build the map for
    @param index the key of the request
    @param reuse enumeration of whether a query is reusable or not
    @param force_ordering true if the request should use the force_ordering pathways
    */
    void initializeMapBuilder(std::string_view request,
                              std::uint16_t index,
                              QueryReuse reuse,
                              bool force_ordering) const;
    /** generate results for core queries*/
    std::string coreQuery(std::string_view queryStr, bool force_ordering) const;

    /** generate results for some core queries that do not depend on the main processing loop
     * running*/
    std::string quickCoreQueries(std::string_view queryStr) const;

    /** generate the filteredEndpoint query results for a particular federate*/
    std::string filteredEndpointQuery(const FederateState* fed) const;
    /** process a command instruction for the core*/
    void processCommandInstruction(ActionMessage& command);

  private:
    int32_t mGlobalFederationSize{0};  //!< total size of the federation
    /// counter for the number of times the entry to initialization Mode was explicitly delayed
    std::atomic<int16_t> delayInitCounter{0};
    bool filterTiming{false};  //!< if there are filters needing a time connection
    /** threadsafe local federate information list for external functions */
    shared_guarded<gmlc::containers::MappedPointerVector<FederateState, std::string>> federates;
    /** federate pointers stored for the core loop */
    gmlc::containers::DualStringMappedVector<FedInfo, GlobalFederateId> loopFederates;

    /** counter for the number of messages that have been sent, nothing magical about 54 just a
     * number bigger than 1 to prevent confusion */
    std::atomic<int32_t> messageCounter{54};
    ordered_guarded<HandleManager> handles;  //!< local handle information;
    /// copy of handles to use in the primary processing loop without thread protection
    HandleManager loopHandles;
    /// sets of ongoing time blocks from filtering
    std::vector<std::pair<GlobalFederateId, int32_t>> timeBlocks;
    TranslatorFederate* translatorFed{nullptr};
    std::atomic<std::thread::id> translatorThread{std::thread::id{}};
    std::atomic<GlobalFederateId> translatorFedID;

    /** delayedTimingMessages from ongoing Filter actions */
    std::map<int32_t, std::vector<ActionMessage>> delayedTimingMessages;

    /// counter for queries start at 1 so the default value isn't used
    std::atomic<int> queryCounter{1};
    /// holder for active queries
    gmlc::concurrency::DelayedObjects<std::string> activeQueries;
    /// timeout manager for queries
    std::deque<std::pair<int32_t, decltype(std::chrono::steady_clock::now())>> queryTimeouts;
    /// holder for the query map builder information
    mutable std::vector<std::tuple<fileops::JsonMapBuilder, std::vector<ActionMessage>, QueryReuse>>
        mapBuilders;

    FilterFederate* filterFed{nullptr};
    std::atomic<std::thread::id> filterThread{std::thread::id{}};
    std::atomic<GlobalFederateId> filterFedID;
    std::atomic<uint16_t> nextAirLock{0};  //!< the index of the next airlock to use
    /// airlocks for updating filter operators and other functions
    std::array<gmlc::containers::AirLock<std::any>, 4> dataAirlocks;
    gmlc::concurrency::TriggerVariable disconnection;  //!< controller for the disconnection process
    /// flag indicating that one or more federates has requested iterative initialization
    std::atomic<bool> initIterations{false};

  private:
    // generate a filter Federate
    void generateFilterFederate();
    // generate a translator Federate
    void generateTranslatorFederate();
    // generate a timing connection between the core and filter Federate
    void connectFilterTiming();
    /** check if a given federate has a timeblock*/
    bool hasTimeBlock(GlobalFederateId federateID);
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
                                             InterfaceType HandleType,
                                             std::string_view key,
                                             std::string_view type,
                                             std::string_view units,
                                             uint16_t flags = 0);

    /** check if a global id represents a local federate
    @param global_fedid the identifier for the federate
    @return true if it is a local federate*/
    bool isLocal(GlobalFederateId global_fedid) const;
    /** get a route id for a non-local federate
    @param global_fedid the identifier for the federate
    @return parent_route if unknown, otherwise returns the route_id*/
    route_id getRoute(GlobalFederateId global_fedid) const;
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
    /** manage any timeblock messages*/
    void manageTimeBlocks(const ActionMessage& command);

    /** generate a query response for a federate if possible
    @param fed a pointer to the federateState object to query
    @param queryStr  the string containing the actual query
    @return "#wait" if the lock cannot be granted immediately and no result can be obtained
    otherwise an answer to the query
    */
    std::string federateQuery(const FederateState* fed,
                              std::string_view queryStr,
                              bool force_ordering) const;

    /** send an error code and message to all the federates*/
    void sendErrorToFederates(int errorCode, std::string_view message);
    /** check for a disconnect and take actions if the object can disconnect*/
    bool checkAndProcessDisconnect();
    /** send a disconnect message to time dependencies and child federates*/
    void sendDisconnect(action_message_def::action_t disconnectType = CMD_STOP);
    /** broadcast a message to all federates*/
    void broadcastToFederates(ActionMessage& cmd);
    /** generate a counter for when to reset object*/
    int generateMapObjectCounter() const;
    friend class TimeoutMonitor;
};

}  // namespace helics
