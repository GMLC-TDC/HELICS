/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#pragma once

#include "../common/simpleQueue.hpp"
#include "ActionMessage.hpp"
#include "BrokerBase.hpp"
#include "Core.hpp"
#include "helics-time.hpp"
#include "helics/helics-config.h"

#include "../common/DualMappedPointerVector.hpp"
#include "../common/DualMappedVector.hpp"
#include "../common/GuardedTypes.hpp"
#include "../common/MappedPointerVector.hpp"
#include "../common/AirLock.hpp"
#include "HandlePointerManager.hpp"
#include <atomic>
#include <cstdint>
#include <thread>
#include <utility>
#include <array>

namespace helics
{
class TestHandle;
class FederateState;

class BasicHandleInfo;
class FilterCoordinator;
class Logger;
class FilterInfo;

enum BasicHandleType : char;

/** base class implementing a standard interaction strategy between federates
@details the CommonCore is virtual class that manages local federates and handles most of the
interaction between federate it is meant to be instantiated for specific inter-federate communication
strategies*/
class CommonCore : public Core, public BrokerBase
{
  public:
    /** default constructor*/
    CommonCore () noexcept;
    /**function mainly to match some other object constructors does the same thing as the default constructor*/
    CommonCore (bool arg) noexcept;
    /** construct from a core name*/
    CommonCore (const std::string &core_name);
    /** virtual destructor*/
    virtual ~CommonCore ();
    virtual void initialize (const std::string &initializationString) override final;
    /** initialize the core manager with command line arguments
    @param[in] argc the number of arguments
    @param[in] argv char pointers to the arguments
    */
    virtual void initializeFromArgs (int argc, const char *const *argv) override;
    virtual bool isInitialized () const override final;
    virtual bool isOpenToNewFederates () const override final;
    virtual void error (federate_id_t federateID, int errorCode = -1) override final;
    virtual void finalize (federate_id_t federateID) override final;
    virtual void enterInitializingState (federate_id_t federateID) override final;
    virtual void setCoreReadyToInit () override final;
    virtual iteration_result
    enterExecutingState (federate_id_t federateID, iteration_request iterate = NO_ITERATION) override final;
    virtual federate_id_t registerFederate (const std::string &name, const CoreFederateInfo &info) override final;
    virtual const std::string &getFederateName (federate_id_t federateID) const override final;
    virtual federate_id_t getFederateId (const std::string &name) override final;
    virtual int32_t getFederationSize () override final;
    virtual Time timeRequest (federate_id_t federateID, Time next) override final;
    virtual iteration_time
    requestTimeIterative (federate_id_t federateID, Time next, iteration_request iterate) override final;
    virtual Time getCurrentTime (federate_id_t federateID) const override final;
    virtual uint64_t getCurrentReiteration (federate_id_t federateID) const override final;
    virtual void setMaximumIterations (federate_id_t federateID, int32_t iterations) override final;
    virtual void setTimeDelta (federate_id_t federateID, Time time) override final;
    virtual void setOutputDelay (federate_id_t federateID, Time outputDelayTime) override final;
    virtual void setPeriod (federate_id_t federateID, Time timePeriod) override final;
    virtual void setTimeOffset (federate_id_t federateID, Time timeOffset) override final;
    virtual void setInputDelay (federate_id_t federateID, Time impactTime) override final;
    virtual void setLoggingLevel (federate_id_t federateID, int loggingLevel) override final;
    virtual void setFlag (federate_id_t federateID, int flag, bool flagValue = true) override final;
    virtual handle_id_t registerSubscription (federate_id_t federateID,
                                              const std::string &key,
                                              const std::string &type,
                                              const std::string &units,
                                              handle_check_mode check_mode) override final;
    virtual handle_id_t getSubscription (federate_id_t federateID, const std::string &key) const override final;
    virtual handle_id_t registerPublication (federate_id_t federateID,
                                             const std::string &key,
                                             const std::string &type,
                                             const std::string &units) override final;
    virtual handle_id_t getPublication (federate_id_t federateID, const std::string &key) const override final;
    virtual const std::string &getHandleName (handle_id_t handle) const override final;
    virtual const std::string &getTarget (handle_id_t handle) const override final;
    virtual const std::string &getUnits (handle_id_t handle) const override final;
    virtual const std::string &getType (handle_id_t handle) const override final;
    virtual const std::string &getOutputType (handle_id_t handle) const override final;
    virtual void setValue (handle_id_t handle, const char *data, uint64_t len) override final;
    virtual std::shared_ptr<const data_block> getValue (handle_id_t handle) override final;

    virtual const std::vector<handle_id_t> &getValueUpdates (federate_id_t federateID) override final;
    virtual handle_id_t
    registerEndpoint (federate_id_t federateID, const std::string &name, const std::string &type) override final;
    virtual handle_id_t getEndpoint (federate_id_t federateID, const std::string &name) const override final;
    virtual handle_id_t registerSourceFilter (const std::string &filterName,
                                              const std::string &source,
                                              const std::string &type_in,
                                              const std::string &type_out) override final;
    virtual handle_id_t registerDestinationFilter (const std::string &filterName,
                                                   const std::string &dest,
                                                   const std::string &type_in,
                                                   const std::string &type_out) override final;
    virtual handle_id_t registerCloningSourceFilter (const std::string &filterName,
                                                     const std::string &source,
                                                     const std::string &type_in,
                                                     const std::string &type_out) override final;
    virtual handle_id_t registerCloningDestinationFilter (const std::string &filterName,
                                                          const std::string &dest,
                                                          const std::string &type_in,
                                                          const std::string &type_out) override final;
    virtual handle_id_t getSourceFilter (const std::string &name) const override final;
    virtual handle_id_t getDestinationFilter (const std::string &name) const override final;
    virtual void addDependency (federate_id_t federateID, const std::string &federateName) override final;
    virtual void
    registerFrequentCommunicationsPair (const std::string &source, const std::string &dest) override final;
    virtual void send (handle_id_t sourceHandle,
                       const std::string &destination,
                       const char *data,
                       uint64_t length) override final;
    virtual void sendEvent (Time time,
                            handle_id_t sourceHandle,
                            const std::string &destination,
                            const char *data,
                            uint64_t length) override final;
    virtual void sendMessage (handle_id_t sourceHandle, std::unique_ptr<Message> message) override final;
    virtual uint64_t receiveCount (handle_id_t destination) override final;
    virtual std::unique_ptr<Message> receive (handle_id_t destination) override final;
    virtual std::unique_ptr<Message>
    receiveAny (federate_id_t federateID, handle_id_t &endpoint_id) override final;
    virtual uint64_t receiveCountAny (federate_id_t federateID) override final;
    virtual void
    logMessage (federate_id_t federateID, int logLevel, const std::string &messageToLog) override final;
    virtual void setFilterOperator (handle_id_t filter, std::shared_ptr<FilterOperator> callback) override final;

    /** set the local identification for the core*/
    void setIdentifier (const std::string &name);
    /** get the local identifier for the core*/
    const std::string &getIdentifier () const override final { return identifier; }
    const std::string &getFederateNameNoThrow (federate_id_t federateID) const noexcept;
    virtual void setLoggingCallback (
      federate_id_t federateID,
      std::function<void(int, const std::string &, const std::string &)> logFunction) override final;

    virtual std::string query (const std::string &target, const std::string &queryStr) override;
    virtual void setQueryCallback (federate_id_t federateID,
                                   std::function<std::string (const std::string &)> queryFunction) override;
    /** get a string representing the connection info to send data to this object*/
    virtual std::string getAddress () const = 0;

    virtual bool connect () override final;
    virtual bool isConnected () const override final;
    virtual void disconnect () override final;
    /** unregister the core from any process find functions*/
    void unregister ();
    virtual void processDisconnect (bool skipUnregister = false) override final;

  private:
    /** implementation details of the connection process
     */
    virtual bool brokerConnect () = 0;
    /** implementation details of the disconnection process
     */
    virtual void brokerDisconnect () = 0;

  protected:
    virtual void processCommand (ActionMessage &&cmd) override final;

    virtual void processPriorityCommand (ActionMessage &&command) override final;

    /** transit an ActionMessage to another core or broker
    @param route_id the identifier for the route information to send the message to
    @param[in] cmd the actionMessage to send*/
    virtual void transmit (int route_id, const ActionMessage &cmd) = 0;
    /** add a route to whatever internal structure manages the routes
    @param route_id the identification of the route
    @param routeInfo a string containing the information necessary to connect*/
    virtual void addRoute (int route_id, const std::string &routeInfo) = 0;
    /** get the federate Information from the federateID*/
    FederateState *getFederateAt (federate_id_t federateID) const;
    /** get the federate Information from the federateID*/
    FederateState *getFederate (const std::string &federateName) const;
    /** get the federate Information from a handle
    @param id a handle identifier as generated by the one of the functions*/
    FederateState *getHandleFederate (handle_id_t id_);
    /** get the basic handle information*/
    BasicHandleInfo *getHandleInfo (handle_id_t id_) const;
    /** get a localEndpoint from the name*/
    BasicHandleInfo *getLocalEndpoint (const std::string &name);
    /** get a filtering function object*/
    FilterCoordinator *getFilterCoordinator (handle_id_t id_);
    /** check if all federates managed by the core are ready to enter initialization state*/
    bool allInitReady () const;
    /** check if all federates have said good-bye*/
    bool allDisconnected () const;

    virtual bool sendToLogger (federate_id_t federateID,
                               int logLevel,
                               const std::string &name,
                               const std::string &message) const override;

  private:
    /** get the federate Information from the federateID*/
    FederateState *getFederateCore (federate_id_t federateID);
    /** get the federate Information from the federateID*/
    FederateState *getFederateCore (const std::string &federateName);
    /** get the federate Information from a handle
    @param id a handle identifier as generated by the one of the functions*/
    FederateState *getHandleFederateCore (handle_id_t id_);

  private:
    std::string prevIdentifier;  //!< storage for the case of requiring a renaming
    std::map<Core::federate_id_t, int32_t>
      routing_table;  //!< map for external routes  <global federate id, route id>
    SimpleQueue<ActionMessage>
      delayTransmitQueue;  //!< FIFO queue for transmissions to the root that need to be delays for a certain time
    std::unordered_map<std::string, int32_t>
      knownExternalEndpoints;  //!< external map for all known external endpoints with names and route

    /** actually transmit messages that were delayed until the core was actually registered*/
    void transmitDelayedMessages ();

    /** actually transmit messages that were delayed for a particular source
    @param[in] source the identifier for the message to transmit
    */
    void transmitDelayedMessages (Core::federate_id_t source);

    /**function for doing the actual routing either to a local fed or up the broker chain*/
    void routeMessage (ActionMessage &cmd, federate_id_t dest);
    /** function for routing a message from based on the destination specified in the ActionMessage*/
    void routeMessage (const ActionMessage &command);

    /**function for doing the actual routing either to a local fed or up the broker chain*/
    void routeMessage (ActionMessage &&cmd, federate_id_t dest);
    /** function for routing a message from based on the destination specified in the ActionMessage*/
    void routeMessage (ActionMessage &&command);

    /** process any filter or route the message*/
    void processMessageFilter (ActionMessage &command);
    /** process a filter message return*/
    void processFilterReturn (ActionMessage &command);
    /** process a destination filter message return*/
    void processDestFilterReturn (ActionMessage &command);
    /** create a source filter */
    FilterInfo *createSourceFilter (federate_id_t dest,
                                    Core::handle_id_t handle,
                                    const std::string &key,
                                    const std::string &target,
                                    const std::string &type_in,
                                    const std::string &type_out);

    /** create a destination filter */
    FilterInfo *createDestFilter (federate_id_t dest,
                                  Core::handle_id_t handle,
                                  const std::string &key,
                                  const std::string &target,
                                  const std::string &type_in,
                                  const std::string &type_out);

    /** check if we can remove some dependencies*/
    void checkDependencies ();

    /** handle command with the core itself as a destination at the core*/
    void processCommandsForCore (const ActionMessage &cmd);
    /** process configure commands for the core*/
    void processCoreConfigureCommands(ActionMessage &cmd);
    /** check if a newly registered subscription has a local publication
    if it does return true*/
    bool checkForLocalPublication (ActionMessage &cmd);

  private:
    int32_t _global_federation_size = 0;  //!< total size of the federation
    std::atomic<int16_t> delayInitCounter{
      0};  //!< counter for the number of times the entry to initialization Mode was explicitly delayed
    shared_guarded<MappedPointerVector<FederateState, std::string>>
      federates;  //!< threadsafe local federate information list for external functions
    DualMappedVector<FederateState *, std::string, federate_id_t>
      loopFederates;  // federate pointers stored for the core loop
    std::atomic<int32_t> messageCounter{54};  //!< counter for the number of messages that have been sent, nothing
                                              //!< magical about 54 just a number bigger than 1 to prevent
                                              //!< confustion

    HandlePointerManager handles;  //!< local handle information;

    std::map<int32_t, std::set<int32_t>> ongoingFilterProcesses;  //!< sets of ongoing filtered messages
    std::map<int32_t, std::set<int32_t>>
      ongoingDestFilterProcesses;  //!< sets of ongoing destination filter processing

    std::map<int32_t, std::vector<ActionMessage>>
      delayedTimingMessages;  //!< delayedTimingMessages from ongoing Filter actions
    std::atomic<int> queryCounter{0};
    std::map<handle_id_t, std::unique_ptr<FilterCoordinator>> filterCoord;  //!< map of all local filters
    using fed_handle_pair = std::pair<federate_id_t, handle_id_t>;
    DualMappedPointerVector<FilterInfo, std::string,
                            fed_handle_pair> filters;  //!< storage for all the filters
    mutable std::mutex
      _handlemutex;  //!< mutex protecting the publications, subscription, endpoint and filter structures
    std::atomic<uint16_t> lastUsedAirlock{ 0 };
    std::array<AirLock<std::shared_ptr<FilterOperator>>, 4> filterOpAirlocks;  //!< airlocks for updating the filter operators

  protected:
    /** deliver a message to the appropriate location*/
    void deliverMessage (ActionMessage &message);
    /** function to deal with a source filters*/
    ActionMessage &processMessage (ActionMessage &m);
    /** add a new handle to the generic structure
    and return a pointer to it, the pointer is non-owning
    */
    BasicHandleInfo *createBasicHandle (federate_id_t global_federateId,
                                        federate_id_t local_federateId,
                                        BasicHandleType HandleType,
                                        const std::string &key,
                                        const std::string &type,
                                        const std::string &units,
                                        bool required);
    /** add a new handle to the generic structure
    and return a pointer to it the pointer is non owning
    variation targeted at filters
    */
    BasicHandleInfo *createBasicHandle (federate_id_t global_federateId,
                                        federate_id_t local_federateId,
                                        BasicHandleType HandleType,
                                        const std::string &key,
                                        const std::string &target,
                                        const std::string &type_in,
                                        const std::string &type_out);

    /** check if a global id represents a local federate
    @param[in] global_id the federate global id
    @return true if it is a local federate*/
    bool isLocal (Core::federate_id_t global_id) const;
    /** get a route id for a non-local federate
    @param[in] global_id the federate global id
    @return 0 if unknown, otherwise returns the route_id*/
    int32_t getRoute (Core::federate_id_t global_id) const;

    /** process a message for potential additions to the filter ordering
    @param command the message to process
    */
    void processFilterInfo (ActionMessage &command);

    /** organize filters
    @detsils organize the filter and report and potential warnings and errors
    */
    void organizeFilterOperations ();

    /** generate a query response of a federate if possible
    @param federateID the identifier for the federate to query
    @param queryStr  the string containing the actual query
    */
    std::string federateQuery (Core::federate_id_t federateID, const std::string &queryStr) const;

    /** send an error code to all the federates*/
    void sendErrorToFederates (int error_code);
};

}  // namespace helics
