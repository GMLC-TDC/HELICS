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

#include "../common/AirLock.hpp"
#include "../common/DelayedObjects.hpp"
#include "../common/DualMappedPointerVector.hpp"
#include "../common/DualMappedVector.hpp"
#include "../common/GuardedTypes.hpp"
#include "../common/MappedPointerVector.hpp"
#include "../common/TriggerVariable.hpp"
#include "helics_includes/any.hpp"
#include "HandleManager.hpp"
#include <array>
#include <atomic>
#include <cstdint>
#include <thread>
#include <utility>

namespace helics
{
class TestHandle;
class FederateState;

class BasicHandleInfo;
class FilterCoordinator;
class Logger;
class FilterInfo;

enum class handle_type_t : char;

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
    explicit CommonCore (bool arg) noexcept;
    /** construct from a core name*/
    explicit CommonCore (const std::string &core_name);
    /** virtual destructor*/
    virtual ~CommonCore () override;
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
    virtual void enterInitializingMode (federate_id_t federateID) override final;
    virtual void setCoreReadyToInit () override final;
    virtual iteration_result
    enterExecutingMode (federate_id_t federateID, iteration_request iterate = NO_ITERATION) override final;
    virtual federate_id_t registerFederate (const std::string &name, const CoreFederateInfo &info) override final;
    virtual const std::string &getFederateName (federate_id_t federateID) const override final;
    virtual federate_id_t getFederateId (const std::string &name) const override final;
    virtual int32_t getFederationSize () override final;
    virtual Time timeRequest (federate_id_t federateID, Time next) override final;
    virtual iteration_time
    requestTimeIterative (federate_id_t federateID, Time next, iteration_request iterate) override final;
    virtual Time getCurrentTime (federate_id_t federateID) const override final;
    virtual uint64_t getCurrentReiteration (federate_id_t federateID) const override final;
    virtual void setTimeProperty (federate_id_t federateID, int32_t property, Time time) override final;
    virtual void setIntegerProperty (federate_id_t federateID, int32_t property, int16_t propertyValue) override final;
    virtual Time getTimeProperty(federate_id_t federateID, int32_t property) const override final;
    virtual int16_t getIntegerProperty(federate_id_t federateID, int32_t property) const override final;
    virtual void setFlagOption (federate_id_t federateID, int32_t flag, bool flagValue = true) override final;
    virtual bool getFlagOption(federate_id_t federateID, int32_t flag) const override final;

    virtual interface_handle registerPublication (federate_id_t federateID,
                                             const std::string &key,
                                             const std::string &type,
                                             const std::string &units) override final;
    virtual interface_handle getPublication (federate_id_t federateID, const std::string &key) const override final;
    virtual interface_handle registerInput(federate_id_t federateID,
        const std::string &key,
        const std::string &type,
        const std::string &units) override final;

    virtual interface_handle getInput(federate_id_t federateID, const std::string &key) const override final;

    virtual const std::string &getHandleName (interface_handle handle) const override final;

    virtual void setHandleOption (interface_handle handle, int32_t option, bool option_value) override final;

    virtual bool getHandleOption (interface_handle handle, int32_t option) const override final;
    virtual void removeTarget (interface_handle handle, const std::string &targetToRemove) override final;
    virtual void addDestinationTarget (interface_handle handle, const std::string &dest) override final;
    virtual void addSourceTarget (interface_handle handle, const std::string &name) override final;
    virtual const std::string &getUnits (interface_handle handle) const override final;
    virtual const std::string &getType (interface_handle handle) const override final;
    virtual const std::string &getOutputType (interface_handle handle) const override final;
    virtual void setValue (interface_handle handle, const char *data, uint64_t len) override final;
    virtual std::shared_ptr<const data_block> getValue (interface_handle handle) override final;
    virtual std::vector<std::shared_ptr<const data_block>> getAllValues (interface_handle handle) override final;
    virtual const std::vector<interface_handle> &getValueUpdates (federate_id_t federateID) override final;
    virtual interface_handle
    registerEndpoint (federate_id_t federateID, const std::string &name, const std::string &type) override final;
    virtual interface_handle getEndpoint (federate_id_t federateID, const std::string &name) const override final;
    virtual interface_handle registerFilter (const std::string &filterName,
                                              const std::string &type_in,
                                              const std::string &type_out) override final;
    virtual interface_handle registerCloningFilter (const std::string &filterName,
                                                     const std::string &type_in,
                                                     const std::string &type_out) override final;
    virtual interface_handle getFilter (const std::string &name) const override final;
    virtual void addDependency (federate_id_t federateID, const std::string &federateName) override final;
    virtual void
    registerFrequentCommunicationsPair (const std::string &source, const std::string &dest) override final;
    virtual void dataLink (const std::string &source, const std::string &target) override final;
    virtual void addSourceFilterToEndpoint (const std::string &filter, const std::string &endpoint) override final;
    virtual void addDestinationFilterToEndpoint (const std::string &filter, const std::string &endpoint) override final;
    virtual void send (interface_handle sourceHandle,
                       const std::string &destination,
                       const char *data,
                       uint64_t length) override final;
    virtual void sendEvent (Time time,
                            interface_handle sourceHandle,
                            const std::string &destination,
                            const char *data,
                            uint64_t length) override final;
    virtual void sendMessage (interface_handle sourceHandle, std::unique_ptr<Message> message) override final;
    virtual uint64_t receiveCount (interface_handle destination) override final;
    virtual std::unique_ptr<Message> receive (interface_handle destination) override final;
    virtual std::unique_ptr<Message>
    receiveAny (federate_id_t federateID, interface_handle &endpoint_id) override final;
    virtual uint64_t receiveCountAny (federate_id_t federateID) override final;
    virtual void
    logMessage (federate_id_t federateID, int logLevel, const std::string &messageToLog) override final;
    virtual void setFilterOperator (interface_handle filter, std::shared_ptr<FilterOperator> callback) override final;

    /** set the local identification for the core*/
    void setIdentifier (const std::string &name);
    /** get the local identifier for the core*/
    virtual const std::string &getIdentifier () const override final { return identifier; }
    virtual const std::string &getAddress () const override final;
    const std::string &getFederateNameNoThrow (global_federate_id_t federateID) const noexcept;

	/** set the core logging level*/
    virtual void setLoggingLevel (int logLevel) override;
    virtual void setLoggingCallback (
      federate_id_t federateID,
      std::function<void(int, const std::string &, const std::string &)> logFunction) override final;

    virtual std::string query (const std::string &target, const std::string &queryStr) override;
    virtual void setQueryCallback (federate_id_t federateID,
                                   std::function<std::string (const std::string &)> queryFunction) override;

    virtual bool connect () override final;
    virtual bool isConnected () const override final;
    virtual void disconnect () override final;
    virtual void waitForDisconnect (int msToWait = -1) const override final;
    /** unregister the core from any process find functions*/
    void unregister ();
    /** TODO figure out how to make this non-public, it needs to be called in a lambda function, may need a helper
     * class of some sort*/
    virtual void processDisconnect (bool skipUnregister = false) override final;

    virtual void setInterfaceInfo(interface_handle handle, std::string info) override final;
    virtual const std::string &getInterfaceInfo(interface_handle handle) const override final;

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
    virtual void transmit (route_id_t route_id, const ActionMessage &cmd) = 0;
    /** transit an ActionMessage to another core or broker
    @param route_id the identifier for the route information to send the message to
    @param[in] cmd the actionMessage to send*/
    virtual void transmit (route_id_t route_id, ActionMessage &&cmd) = 0;
    /** add a route to whatever internal structure manages the routes
    @param route_id the identification of the route
    @param routeInfo a string containing the information necessary to connect*/
    virtual void addRoute (route_id_t route_id, const std::string &routeInfo) = 0;
    /** get the federate Information from the federateID*/
    FederateState *getFederateAt (federate_id_t federateID) const;
    /** get the federate Information from the federateID*/
    FederateState *getFederate (const std::string &federateName) const;
    /** get the federate Information from a handle
    @param id a handle identifier as generated by the one of the functions*/
    FederateState *getHandleFederate (interface_handle id_);
    /** get the basic handle information*/
    const BasicHandleInfo *getHandleInfo (interface_handle id_) const;
    /** get a localEndpoint from the name*/
    const BasicHandleInfo *getLocalEndpoint (const std::string &name) const;
    /** get a filtering function object*/
    FilterCoordinator *getFilterCoordinator (interface_handle id_);
    /** check if all federates managed by the core are ready to enter initialization state*/
    bool allInitReady () const;
    /** check if all federates have said good-bye*/
    bool allDisconnected () const;

    virtual bool sendToLogger (global_federate_id_t federateID,
                               int logLevel,
                               const std::string &name,
                               const std::string &message) const override;

  private:
    /** get the federate Information from the federateID*/
    FederateState *getFederateCore (global_federate_id_t federateID);
    /** get the federate Information from the federateID*/
    FederateState *getFederateCore (const std::string &federateName);
    /** get the federate Information from a handle
    @param id a handle identifier as generated by the one of the functions*/
    FederateState *getHandleFederateCore (interface_handle id_);

  private:
    std::string prevIdentifier;  //!< storage for the case of requiring a renaming
    std::map<global_federate_id_t, route_id_t>
      routing_table;  //!< map for external routes  <global federate id, route id>
    SimpleQueue<ActionMessage>
      delayTransmitQueue;  //!< FIFO queue for transmissions to the root that need to be delays for a certain time
    std::unordered_map<std::string, route_id_t>
      knownExternalEndpoints;  //!< external map for all known external endpoints with names and route

    /** actually transmit messages that were delayed until the core was actually registered*/
    void transmitDelayedMessages ();

    /** actually transmit messages that were delayed for a particular source
    @param[in] source the identifier for the message to transmit
    */
    void transmitDelayedMessages (global_federate_id_t source);

    /**function for doing the actual routing either to a local fed or up the broker chain*/
    void routeMessage (ActionMessage &cmd, global_federate_id_t dest);
    /** function for routing a message from based on the destination specified in the ActionMessage*/
    void routeMessage (const ActionMessage &command);

    /**function for doing the actual routing either to a local fed or up the broker chain*/
    void routeMessage (ActionMessage &&cmd, global_federate_id_t dest);
    /** function for routing a message from based on the destination specified in the ActionMessage*/
    void routeMessage (ActionMessage &&command);

    /** process any filter or route the message*/
    void processMessageFilter (ActionMessage &command);
    /** process a filter message return*/
    void processFilterReturn (ActionMessage &command);
    /** process a destination filter message return*/
    void processDestFilterReturn (ActionMessage &command);
    /** create a source filter */
    FilterInfo *createFilter (global_broker_id_t dest,
                                    interface_handle handle,
                                    const std::string &key,
                                    const std::string &type_in,
                                    const std::string &type_out,
                                    bool cloning);


    /** check if we can remove some dependencies*/
    void checkDependencies ();

    /** handle command with the core itself as a destination at the core*/
    void processCommandsForCore (const ActionMessage &cmd);
    /** process configure commands for the core*/
    void processCoreConfigureCommands(ActionMessage &cmd);
    /** check if a newly registered subscription has a local publication
    if it does return true*/
    bool checkForLocalPublication (ActionMessage &cmd);
    /** get an index for an airlock function is threadsafe*/
    uint16_t getNextAirlockIndex();
    /** generate results for core queries*/
    std::string coreQuery(const std::string &queryStr) const;

  private:
    int32_t _global_federation_size = 0;  //!< total size of the federation
    std::atomic<int16_t> delayInitCounter{
      0};  //!< counter for the number of times the entry to initialization Mode was explicitly delayed
    shared_guarded<MappedPointerVector<FederateState, std::string>>
      federates;  //!< threadsafe local federate information list for external functions
    DualMappedVector<FederateState *, std::string, global_federate_id_t>
      loopFederates;  // federate pointers stored for the core loop
    std::atomic<int32_t> messageCounter{54};  //!< counter for the number of messages that have been sent, nothing
                                              //!< magical about 54 just a number bigger than 1 to prevent
                                              //!< confusion

    ordered_guarded<HandleManager> handles;  //!< local handle information;
    HandleManager loopHandles; //!< copy of handles to use in the primary processing loop without thread protection
    std::map<int32_t, std::set<int32_t>> ongoingFilterProcesses;  //!< sets of ongoing filtered messages
    std::map<int32_t, std::set<int32_t>>
      ongoingDestFilterProcesses;  //!< sets of ongoing destination filter processing

    std::map<int32_t, std::vector<ActionMessage>>
      delayedTimingMessages;  //!< delayedTimingMessages from ongoing Filter actions
    std::atomic<int> queryCounter{1}; //counter for queries start at 1 so the default value isn't used
    DelayedObjects<std::string> ActiveQueries; //holder for active queries

    std::map<interface_handle, std::unique_ptr<FilterCoordinator>> filterCoord;  //!< map of all local filters


    DualMappedPointerVector<FilterInfo, std::string,
        global_handle> filters;  //!< storage for all the filters

    std::atomic<uint16_t> nextAirLock{ 0 }; //!< the index of the next airlock to use
    std::array<AirLock<stx::any>, 4> dataAirlocks;  //!< airlocks for updating filter operators and other functions
    TriggerVariable disconnection;  //!< controller for the disconnection process
  private:
    /** deliver a message to the appropriate location*/
    void deliverMessage (ActionMessage &message);
    /** function to deal with a source filters*/
    ActionMessage &processMessage (ActionMessage &m);
    /** add a new handle to the generic structure
    and return a reference to the basicHandle
    */
   const BasicHandleInfo &createBasicHandle (global_federate_id_t global_federateId,
                                        federate_id_t local_federateId,
                                        handle_type_t HandleType,
                                        const std::string &key,
                                        const std::string &type,
                                        const std::string &units,
                                        uint16_t flags=0);

    /** check if a global id represents a local federate
    @param[in] global_id the federate global id
    @return true if it is a local federate*/
    bool isLocal (global_federate_id_t global_id) const;
    /** get a route id for a non-local federate
    @param[in] global_id the federate global id
    @return parent_route if unknown, otherwise returns the route_id*/
    route_id_t getRoute (global_federate_id_t global_id) const;
    /** process a message for potential additions to the filter ordering
    @param command the message to process
    */
    void processFilterInfo (ActionMessage &command);
    /** function to check for a named interface*/
    void checkForNamedInterface (ActionMessage &command);
	/** indicate that a handle interface is used and if the used status has changed make sure it is indicated
	in all the needed places*/
	void setAsUsed(BasicHandleInfo *hand);
	/** function to consolidate the registration of interfaces in the core*/
	void registerInterface(ActionMessage &command);
	/** function to handle adding a target to an interface*/
	void addTargetToInterface(ActionMessage &command);
    /** organize filters
    @detsils organize the filter and report and potential warnings and errors
    */
    void organizeFilterOperations ();

    /** generate a query response of a federate if possible
    @param federateID the identifier for the federate to query
    @param queryStr  the string containing the actual query
    */
    std::string federateQuery (const FederateState *fed, const std::string &queryStr) const;

    /** send an error code to all the federates*/
    void sendErrorToFederates (int error_code);
    /** check for a disconnect and take actions if the object can disconnect*/
    bool checkAndProcessDisconnect ();
    /** send a disconnect message to time dependencies and child federates*/
    void sendDisconnect ();
};

}  // namespace helics
