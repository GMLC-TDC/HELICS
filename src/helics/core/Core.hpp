/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "LocalFederateId.hpp"
#include "core-data.hpp"

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

/** @file
 * The HELICS core interface.  Abstract class that is
 * implemented for the specific communication systems (e.g. ZMQ and
 * MPI).
 *
 * Multiple federates are allowed.  Due to the collective blocking
 * nature of some calls, like requestTime(), federates may need to be in
 * separate threads in order to function correctly.
 *
 *
 *
 * Note: Methods should all be pure virtual.
 */

/** @namespace  helics
@brief the main namespace for the helics co-simulation library
User functions will be in the helics namespace with internal functions possible in a lower level
namespace
*/
namespace helics {
class CoreFederateInfo;

/** the class defining the core interface through an abstract class*/
class Core {
  public:
    /** default constructor*/
    Core() = default;
    /**virtual destructor*/
    virtual ~Core() = default;

    /**
     * Configure the core from a configuration string
     *
     * Should be invoked a single time to initialize the co-simulation core.
     *
     */
    virtual void configure(std::string_view configureString) = 0;
    /**
     * Configure the core from command line arguments.
     *
     * Should be invoked a single time to configure the co-simulation core for operation
     *
     */
    virtual void configureFromArgs(int argc, char* argv[]) = 0;
    /**
     * Configure the core from command line arguments contained in a vector in reverse order
     *
     * Should be invoked a single time to configure the co-simulation core for operations
     *
     */
    virtual void configureFromVector(std::vector<std::string> args) = 0;
    /**
     * Returns true if the core has been configured.
     */
    virtual bool isConfigured() const = 0;

    /**
    * connect the core to a broker if needed
    @return true if the connection was successful
    */
    virtual bool connect() = 0;
    /**
     * check if the core is connected properly
     */
    virtual bool isConnected() const = 0;

    /**
     * disconnect the core from its broker
     */
    virtual void disconnect() = 0;

    /*return true if the core has an error*/
    virtual bool hasError() const = 0;
    /** waits in the current thread until the core is disconnected
    @return true if the disconnect was successful
     */
    virtual bool waitForDisconnect(
        std::chrono::milliseconds msToWait = std::chrono::milliseconds(0)) const = 0;

    /** check if the core is ready to accept new federates
     */
    virtual bool isOpenToNewFederates() const = 0;
    /** get an identifier string for the core
     */
    virtual const std::string& getIdentifier() const = 0;
    /** get the connection network or connection address for the core*/
    virtual const std::string& getAddress() const = 0;

    /**
     * Federate has encountered a global error and the federation should halt.
     * @param federateID the federate
     * @param errorCode a numerical code associated with the error
     * @param errorString a text message associated with the error
     */
    virtual void globalError(LocalFederateId federateID,
                             int32_t errorCode,
                             std::string_view errorString) = 0;

    /**
     * Federate has encountered a local error and should be disconnected.
     * @param federateID the federate
     * @param errorCode a numerical code associated with the error
     * @param errorString a text message associated with the error
     */
    virtual void
        localError(LocalFederateId federateID, int32_t errorCode, std::string_view errorString) = 0;

    /** get the last error code from a core*/
    virtual int getErrorCode() const = 0;
    /** get the last error message*/
    virtual std::string getErrorMessage() const = 0;
    /**
     * Federate has completed.
     *
     * Should be invoked a single time to complete the simulation.
     *
     */
    virtual void finalize(LocalFederateId federateID) = 0;

    /**
     * Federates may be in five Modes.
     *    -# Startup
     *       Configuration of the federate.
     *       State begins when registerFederate() is invoked and ends when enterInitializingMode()
     * is invoked.
     *    -# Initializing
     *       Configure of the simulation state prior to the start of time stepping.
     *       State begins when enterInitializingMode() is invoked and ends when
     * enterExecutingMode(true) is invoked.
     *    -# Executing
     *       State begins when enterExecutingMode() is invoked and ends when finalize() is invoked.
     *    -# Finalized
     *       state after finalize is invoked.
     *    -# Error
     *       state invoked after an error is called.
     */

    /**
     * Change the federate state to the Initializing state.
     *
     *@param federateID  the identifier of the federate
     *@param iterate  the requested iteration mode, ITERATE_IF_NEEDED will operate identically to
     *FORCE_ITERATION in this case
     *
     * May only be invoked in Created state otherwise an error is thrown
     * for callback federates this call passes full control to the core
     @return will return true if the call resulted in Initializing mode being reached
     */
    virtual bool enterInitializingMode(LocalFederateId federateID,
                                       IterationRequest iterate = NO_ITERATION) = 0;

    /** set the core to ready to enter init
    @details this function only needs to be called for cores that don't have any federates but may
    have filters for cores with federates it won't do anything*/
    virtual void setCoreReadyToInit() = 0;

    /**
     * Change the federate state to the Executing state.
     *
     * May only be invoked in Initializing state.
     *@param federateID  the identifier of the federate
     *@param iterate  the requested iteration mode
     *if nonconverged the federate requests an iterative update
     *
     *@return an iteration_time result enumeration value indicating the current state of iterations
     *and a time with the current simulation time (usually 0) unless the federate is joining
     *dynamically
     */
    virtual iteration_time enterExecutingMode(LocalFederateId federateID,
                                              IterationRequest iterate = NO_ITERATION) = 0;

    /**
     * Register a federate.
     *
     * The returned FederateId is local to invoking process,
     * FederateId's should not be used as a global identifier.
     *
     * May only be invoked in initialize state otherwise throws an error
     */
    virtual LocalFederateId registerFederate(std::string_view name,
                                             const CoreFederateInfo& info) = 0;

    /**
     * Returns the federate name.
     *
     */
    virtual const std::string& getFederateName(LocalFederateId federateID) const = 0;

    /**
     * Returns the federate Id.
     *
     */
    virtual LocalFederateId getFederateId(std::string_view name) const = 0;

    /**
     * Returns the global number of federates that are registered only return accurately after the
     * initialization state has been entered
     */
    virtual int32_t getFederationSize() = 0;

    /**
     * Time management.
     */

    /**
     * Request a new time advancement window for non-reiterative federates.
     *
     * RequestTime() blocks until all non-reiterative federates have
     * invoked requestTime() and all reiterative federates have
     * converged (called requestTimeIterative() with localConverged
     * value of true). Return time is the minimum of all supplied
     * times.
     *
     * May only be invoked in Executing state.
     *
     * Iterative federates may not invoke this method.
     *
     * @param federateID the identification of the federate requesting the time
     @param next the next time that is requested from the federate
     */
    virtual Time timeRequest(LocalFederateId federateID, Time next) = 0;

    /**
     * Request a new time advancement window for reiterative federates.
     *
     * Reiterative federates block on requestTimeIterative() until all
     * reiterative federates have invoked requestTimeIterative(). The
     * bool returned a global AND of all localConverged values. If
     * globalConverged is false, time returned is the previous
     * granted time.  Time should not advance and another iteration
     * attempted.   Federates should recompute state based on newly
     * published values. Time is advanced only when all reiterative
     * federates have converged. If globalConverged is True,
     * grantedTime is the minimum of over all next
     * times in both reiterative and non-reiterative federates.
     *
     * If a federate determines it cannot converge it should invoke the error() method.
     *
     * Federates only participate it in reiterations for times that
     * are evenly divisible by the federates time delta.
     *
     * May only be invoked in Executing state.
     *
     * Non-reiterative federates may not invoke this method.
     *@param federateID the identifier for the federate to process
     * @param next the requested time
     * @param iterate the requested iteration mode /ref iteration_request
     * @return an /ref iteration_time object with two field grantedTime and a enumeration indicating
     the state of the iteration
     */
    virtual iteration_time
        requestTimeIterative(LocalFederateId federateID, Time next, IterationRequest iterate) = 0;

    /** blocking call that processes helics communication messages
     * this call can be used when expecting communication from other federates or when the federate
     * has nothing else to do and doesn't want to advance time
     *
     * @param fedId the ID of the federate to process communications for
     * @param msToWait the amount of time to wait before the function returns from processing
     * communications
     */
    virtual void processCommunications(LocalFederateId fedId,
                                       std::chrono::milliseconds msToWait) = 0;
    /** set a timebased property on a federate
    @param federateID the federate to set a time based property on
    @param property the property to set see /ref defs::properties
    @param timeValue the requested value of the property
    */
    virtual void setTimeProperty(LocalFederateId federateID, int32_t property, Time timeValue) = 0;
    /** get a timebased property on a federate
    @param federateID the federate to set a time based property on
    @param property the property to set see /ref defs::properties
    @return the current value of the requested property
    */
    virtual Time getTimeProperty(LocalFederateId federateID, int32_t property) const = 0;
    /** set an integer property on a federate
    @param federateID the federate to set a time based property on
    @param property the property to set see /ref defs::properties
    @param propValue the requested value of the property
    */
    virtual void
        setIntegerProperty(LocalFederateId federateID, int32_t property, int16_t propValue) = 0;
    /** get an integer property on a federate
    @param federateID the federate to set a time based property on
    @param property the property to set see /ref defs::properties
    @return the current value of the property
    */
    virtual int16_t getIntegerProperty(LocalFederateId federateID, int32_t property) const = 0;
    /** get the most recent granted Time
    @param federateID the identifier of the federate to get the time
    @return the most recent granted time or the startup time
    */
    virtual Time getCurrentTime(LocalFederateId federateID) const = 0;

    /**
    Set a flag in a a federate
    * @param federateID  the identifier for the federate
    * @param flag an index code for the flag to set
    * @param flagValue the value to set the flag to
    */
    virtual void setFlagOption(LocalFederateId federateID, int32_t flag, bool flagValue) = 0;

    /**
    Set a flag in a a federate
    * @param federateID  the identifier for the federate
    * @param flag an index code for the flag to set
    * @return the value of the flag
    */
    virtual bool getFlagOption(LocalFederateId federateID, int32_t flag) const = 0;
    /**
     * Value interface.
     */

    /**
     * Register a publication.
     *
     * May only be invoked in the initialize state.
     @param federateID the identifier for the federate
     @param key the tag for the publication
     @param type the type of data the publication produces
     @param units the units associated with the publication
     @return a handle to identify the publication
     */
    virtual InterfaceHandle registerPublication(LocalFederateId federateID,
                                                std::string_view key,
                                                std::string_view type,
                                                std::string_view units) = 0;

    /** get a publication Handle from its key
    @param federateID the identifier for the federate
    @param key the name of the publication
     @return a handle to identify the publication*/
    virtual InterfaceHandle getPublication(LocalFederateId federateID,
                                           std::string_view key) const = 0;

    /**
     * Register a control input for the specified federate.
     *
     * May only be invoked in the initialize state.
     * @param federateID the identifier for the federate to register an input interface on
     * @param key the name of the control input
     * @param type a string describing the type of the federate
     * @param units a string naming the units of the federate
     */
    virtual InterfaceHandle registerInput(LocalFederateId federateID,
                                          std::string_view key,
                                          std::string_view type,
                                          std::string_view units) = 0;
    /** get a subscription Handle from its key
    @param federateID the identifier for the federate
    @param key the tag of the named input
    @return a handle to identify the input*/
    virtual InterfaceHandle getInput(LocalFederateId federateID, std::string_view key) const = 0;

    /**
     * Returns the name or identifier for a specified handle
     */
    virtual const std::string& getHandleName(InterfaceHandle handle) const = 0;
    /** remove a target from a handles operation
     *@param handle the handle from the publication, input, endpoint or filter
     *@param targetToRemove the name of the target to remove
     */
    virtual void removeTarget(InterfaceHandle handle, std::string_view targetToRemove) = 0;

    /**
     * @return the unit string for the specified handle.
     */
    virtual const std::string& getExtractionUnits(InterfaceHandle handle) const = 0;
    /** get the injection units for an interface,  this is the type for data coming into an
     *interface
     *@details for publications this is the units associated with the transmitted data,  for inputs
     *this is the units of the transmitting publication
     *@param handle the interface handle to get the injection type for
     *
     *@return a const ref to  std::string
     */
    virtual const std::string& getInjectionUnits(InterfaceHandle handle) const = 0;

    /** get the injection type for an interface,  this is the type for data coming into an interface
     *@details for filters this is the input type, for publications this is type used to transmit
     *data, for endpoints this is the specified type and for inputs this is the type of the
     *transmitting publication
     *@param handle the interface handle to get the injection type for
     *@return a const ref to  std::string
     */
    virtual const std::string& getInjectionType(InterfaceHandle handle) const = 0;

    /** get the type for which data comes out of an interface,  this is the type for data coming
    into an interface
    @details for filters this is the output type, for publications this is the specified type, for
    endpoints this is the specified type and for inputs this is the specified type
    @param handle the interface handle to get the injection type for
    @return a const ref to  std::string  */
    virtual const std::string& getExtractionType(InterfaceHandle handle) const = 0;

    /** set a handle option
     *@param handle the handle from the publication, input, endpoint or filter
     *@param option the option to set
     *@param option_value the value to set the option (mostly 0 or 1)
     */
    virtual void setHandleOption(InterfaceHandle handle, int32_t option, int32_t option_value) = 0;

    /** get a handle option
    @param handle the handle from the publication, input, endpoint or filter
    @param option the option to set see /ref defs::options
    */
    virtual int32_t getHandleOption(InterfaceHandle handle, int32_t option) const = 0;

    /** close a handle from further connections
    @param handle the handle from the publication, input, endpoint or filter
    */
    virtual void closeHandle(InterfaceHandle handle) = 0;
    /**
     * Publish specified data to the specified key.
     *
     @param handle the handle from the publication, input, endpoint or filter
     @param data the raw data to send
     @param len the size of the data
     */
    virtual void setValue(InterfaceHandle handle, const char* data, uint64_t len) = 0;

    /**
     * Return the data for the specified handle or the latest input
     * @param handle the input handle from which to get the data
     * @param[out] inputIndex return the index of input (always 1 for inputs with only a single
     * source)
     */
    virtual const std::shared_ptr<const SmallBuffer>& getValue(InterfaceHandle handle,
                                                               uint32_t* inputIndex = nullptr) = 0;

    /**
     * Return all the available data for the specified handle or the latest input
     *
     */
    virtual const std::vector<std::shared_ptr<const SmallBuffer>>&
        getAllValues(InterfaceHandle handle) = 0;

    /**
     * Returns vector of input handles that received an update during the last
     * time request.  The data remains valid until the next call to getValueUpdates for the given
     federateID
     *@param federateID the identification code of the federate to get which interfaces have been
     updated
     @return a reference to the location of an array of handles that have been updated
     */
    virtual const std::vector<InterfaceHandle>& getValueUpdates(LocalFederateId federateID) = 0;

    /**
     * Message interface.
     * Designed for point-to-point communication patterns.
     */

    /**
     * Register an endpoint.
     @param federateID the federate to associate the endpoint with
     @param name the name of the endpoint
     @param type the type of data the endpoint should accept or generate(can be left empty)
     */
    virtual InterfaceHandle registerEndpoint(LocalFederateId federateID,
                                             std::string_view name,
                                             std::string_view type) = 0;

    /**
     * Register an endpoint which can only send or receive to specific targets
     @param federateID the federate to associate the endpoint with
     @param name the name of the endpoint
     @param type the type of data the endpoint should accept or generate(can be left empty)
     */
    virtual InterfaceHandle registerTargetedEndpoint(LocalFederateId federateID,
                                                     std::string_view name,
                                                     std::string_view type) = 0;

    /** get an endpoint Handle from its name
    @param federateID the identifier for the federate
    @param name the name of the endpoint
    @return a handle to identify the endpoint*/
    virtual InterfaceHandle getEndpoint(LocalFederateId federateID,
                                        std::string_view name) const = 0;

    /**
    * Register a data sink which can only receive data from specific targets
    @param federateID the federate to associate the endpoint with
    @param name the name of the sink
    @return a handle to identify the sink
    */
    virtual InterfaceHandle registerDataSink(LocalFederateId federateID, std::string_view name) = 0;

    /** get an interface handle to a data sink
    @param federateID the identifier for the federate
    @param name the name of the data sink
    @return a handle to identify the sink*/
    virtual InterfaceHandle getDataSink(LocalFederateId federateID,
                                        std::string_view name) const = 0;

    /**
    * Register a cloning filter, a cloning filter operates on a copy of the message vs the actual
    message
    *
    @param filterName the name of the filter (may be left blank and one will be automatically
    assigned)
    @param type_in the input type of the filter
    @param type_out the output type of the filter (may be left blank if the filter doesn't change
    type)
    @return the handle for the new filter
    */
    virtual InterfaceHandle registerCloningFilter(std::string_view filterName,
                                                  std::string_view type_in,
                                                  std::string_view type_out) = 0;

    /**
     * Register filter.
     *
     * May only be invoked in the Initialization state.
     @param filterName the name of the filter (may be left blank and one will be automatically
     assigned)
     @param type_in the input type of the filter
     @param type_out the output type of the filter (may be left blank if the filter doesn't change
     type) this is important for ordering in filters with operators
     @return the handle for the new filter
     */
    virtual InterfaceHandle registerFilter(std::string_view filterName,
                                           std::string_view type_in,
                                           std::string_view type_out) = 0;

    /**
   * Register translator.
   *
   @param translatorName the name of the translator (may be left blank and one will be automatically
   assigned)
   @param units the specified units for the value side of the translator
   @param endpointType a user specified name of the type data on the endpoint
   @return the handle for the new translator
   */
    virtual InterfaceHandle registerTranslator(std::string_view translatorName,
                                               std::string_view endpointType,
                                               std::string_view units) = 0;

    /**
    * adds a destination for interface data, the handle can be a publication, endpoint, filter,
    or translators
    @details a filter will create an additional processing step for messages before they get to a
    destination endpoint, for publications this will establish a linkage from the publication to the
    named input
    *
    @param handle an interface to add the target to
    @param dest the target endpoint for the filter
    @param hint the interface type for the destination target
    */
    virtual void addDestinationTarget(InterfaceHandle handle,
                                      std::string_view dest,
                                      InterfaceType hint = InterfaceType::UNKNOWN) = 0;

    /** adds a source of data to an interface, the handle can be an input, filter, translator, or
    endpoint
    @details for subscriptions and inputs this establishes a link from a publication, for endpoints
    this creates a linkage to a particular publication, for filters it add a source endpoint to
    filter
    @param handle the identifier of the interface
    @param name the name of the filter or its target
    @param hint the interface type for the source target
    */
    virtual void addSourceTarget(InterfaceHandle handle,
                                 std::string_view name,
                                 InterfaceType hint = InterfaceType::UNKNOWN) = 0;

    /**
    * get the destinations for an interface
    @param handle an interface get the destination targets for
    */
    virtual const std::string& getDestinationTargets(InterfaceHandle handle) const = 0;

    /** get the sources of data for an interface
    @param handle the identifier of the interface
    */
    virtual const std::string& getSourceTargets(InterfaceHandle handle) const = 0;

    /** get a filter Handle from its name or target(this may not be unique so it will
    only find the first one)
    @param name the name of the filter or its target
    @return a handle to identify the filter*/
    virtual InterfaceHandle getFilter(std::string_view name) const = 0;

    /** get a translator handle from its name or target (this may not be unique so it will
    only find the first one)
    @param name the name of the translator or its target
    @return a handle to identify the translator*/
    virtual InterfaceHandle getTranslator(std::string_view name) const = 0;

    /**
    * add a time dependency between federates
    * @details this function is primarily useful for Message federates which do not otherwise
    restrict the dependencies
    * adding a dependency gives additional information to the core that the specified federate(given
    by id) will be sending Messages to the named Federate(by federateName)
    @param federateID  the identifier for the federate
    @param federateName the name of the dependent federate
    */
    virtual void addDependency(LocalFederateId federateID, std::string_view federateName) = 0;

    /**
     * Register communicating source/destination endpoint targets.
     * @param source the endpoint that is sending data
     * @param dest the endpoint receiving the data
     *
     */
    virtual void linkEndpoints(std::string_view source, std::string_view dest) = 0;

    /** add an interface alias
    This allows an interface to be referred to by multiple keys
    @param interfaceKey the name of the interface to generate an alias for
    @param alias the additional identification string
    */
    virtual void addAlias(std::string_view interfaceKey, std::string_view alias) = 0;

    /** load a file containing connection information
    @param file a JSON or TOML file containing connection information*/
    virtual void makeConnections(const std::string& file) = 0;

    /** create a data connection between a named publication and a named input
    @param source the name of the publication
    @param target the name of the input*/
    virtual void dataLink(std::string_view source, std::string_view target) = 0;
    /** create a filter connection between a named filter and a named endpoint for messages coming
    from that endpoint
    @param filter the name of the filter
    @param target the name of the source target*/
    virtual void addSourceFilterToEndpoint(std::string_view filter, std::string_view target) = 0;
    /** create a filter connection between a named filter and a named endpoint for destination
    processing
    @param filter the name of the filter
    @param target the name of the source target*/
    virtual void addDestinationFilterToEndpoint(std::string_view filter,
                                                std::string_view target) = 0;

    /**
     * Send data from a source to its targets
     *
     * Time is implicitly defined as the end of the current time
     * advancement window (value returned by last call to nextTime().
     *
     * This send version was designed to enable communication of
     * data between federates with the possible introduction of
     * source and destination filters to represent properties of a
     * communication network.  This enables simulations to be run with/without
     * a communications model present.
     */
    virtual void send(InterfaceHandle sourceHandle, const void* data, uint64_t length) = 0;
    /**
     * Send data from source to destination.
     *
     * Time is implicitly defined as the end of the current time
     * advancement window (value returned by last call to nextTime().
     *
     * This send version was designed to enable communication of
     * data between federates with the possible introduction of
     * source and destination filters to represent properties of a
     * communication network.  This enables simulations to be run with/without
     * a communications model present.
     */
    virtual void sendTo(InterfaceHandle sourceHandle,
                        const void* data,
                        uint64_t length,
                        std::string_view destination) = 0;

    /**
     * Send data from source to destination with explicit expected delivery time.
     *
     * Time supplied is the time that will be reported in the message in the receiving
     * federate.
     *
     * This send version was designed to enable communication of
     * events between discrete event federates.  For this use case
     * the receiving federate can deserialize the data and schedule
     * an event for the specified time.
     @param sourceHandle the source of the event
     @param data the raw data for the event
     @param length the record length of the event
     @param time the time the event is scheduled for
     */
    virtual void
        sendAt(InterfaceHandle sourceHandle, const void* data, uint64_t length, Time time) = 0;

    /**
   * Send data from source to destination with explicit expected delivery time.
   *
   * Time supplied is the time that will be reported in the message in the receiving
   * federate.
   *
   * This send version was designed to enable communication of
   * events between discrete event federates.  For this use case
   * the receiving federate can deserialize the data and schedule
   * an event for the specified time.
   @param sourceHandle the source of the event
   @param data the raw data for the event
   @param length the record length of the event
   @param destination  the target of the event
   @param time the time the event is scheduled for
   */
    virtual void sendToAt(InterfaceHandle sourceHandle,
                          const void* data,
                          uint64_t length,
                          std::string_view destination,
                          Time time) = 0;

    /**
     * Send for filters.
     *
     * Continues sending the message to the next filter or to final destination.
     *
     */
    virtual void sendMessage(InterfaceHandle sourceHandle, std::unique_ptr<Message> message) = 0;

    /**
     * Returns the number of pending receives for the specified destination endpoint.
     */
    virtual uint64_t receiveCount(InterfaceHandle destination) = 0;

    /**
     * Returns the next buffered message the specified destination endpoint.
     @details this is a non-blocking call and will return a nullptr if no message are available
     */
    virtual std::unique_ptr<Message> receive(InterfaceHandle destination) = 0;

    /**
     * Receives a message for any destination.
     @details this is a non-blocking call and will return a nullptr if no messages are available
     @param federateID the identifier for the federate
     @param[out] endpoint_id the endpoint handle related to the message gets stored here
     */
    virtual std::unique_ptr<Message> receiveAny(LocalFederateId federateID,
                                                InterfaceHandle& endpoint_id) = 0;

    /**
     * Returns number of messages for all destinations.
     */
    virtual uint64_t receiveCountAny(LocalFederateId federateID) = 0;

    /** send a log message to the Core for logging
    @param federateID the federate that is sending the log message
    @param logLevel  an integer for the log level /ref helics_log_levels
    @param messageToLog the string to send to a logger
    */
    virtual void
        logMessage(LocalFederateId federateID, int logLevel, std::string_view messageToLog) = 0;

    /** set the filter callback operator
    @param filter  the handle of the filter
    @param callback pointer to the operator class executing the filter
    */
    virtual void setFilterOperator(InterfaceHandle filter,
                                   std::shared_ptr<FilterOperator> callback) = 0;

    /** set the translator callback operators
    @param translator  the handle of the translator
    @param callback pointer to the operator class executing the translator
    */
    virtual void setTranslatorOperator(InterfaceHandle translator,
                                       std::shared_ptr<TranslatorOperator> callback) = 0;

    /** set the callback Federate operators
    @param fed  the federate to set the callback for
    @param callback pointer to the operator class executing the federate
    */
    virtual void setFederateOperator(LocalFederateId fed,
                                     std::shared_ptr<FederateOperator> callback) = 0;

    /** define a logging function to use for logging message and notices from the federation and
    individual federate
    @param federateID  the identifier for the individual federate or 0 for the Core Logger
    @param logFunction the callback function for doing something with a log message
    it takes 3 inputs an integer for logLevel /ref helics_log_levels
    A string indicating the source of the message and another string with the actual message
    */
    virtual void setLoggingCallback(
        LocalFederateId federateID,
        std::function<void(int, std::string_view, std::string_view)> logFunction) = 0;

    /** set the core logging level*/
    virtual void setLoggingLevel(int logLevel) = 0;

    /** set the core logging file*/
    virtual void setLogFile(std::string_view lfile) = 0;

    /** set a federation global value
    @details this overwrites any previous value for this name
    @param valueName the name of the global to set
    @param value the value of the global
    */
    virtual void setGlobal(std::string_view valueName, std::string_view value) = 0;

    /** send a command to a specific target
  @details the format is somewhat unspecified; target is the name of an object, typically one of
  "federation",  "broker", "core", or the name of a specific object/core/broker
  @param target the specific target of the command
  @param commandStr the actual command
  @param source the designated source of the command, for return values or indication
  @param mode the sequencing mode for the command, fast or ordered
  */
    virtual void sendCommand(std::string_view target,
                             std::string_view commandStr,
                             std::string_view source,
                             HelicsSequencingModes mode) = 0;

    /** get a command for a specific federate
     */
    virtual std::pair<std::string, std::string> getCommand(LocalFederateId federateID) = 0;

    /** get a command for a specific federate. block until a command is received
     */
    virtual std::pair<std::string, std::string> waitCommand(LocalFederateId federateID) = 0;

    /** make a query for information from the co-simulation
    @details the format is somewhat unspecified  target is the name of an object typically one of
    "federation",  "broker", "core", or the name of a specific object/core/broker
    target can also be "global_value" to query a global value stored in the broker, or "global" to
    get a json structure with the name and value
    @param target the specific target of the query
    @param queryStr the actual query
    @param mode the sequencing mode for the query, fast or ordered
    @return a string containing the response to the query.  Query is a blocking call and will not
    return until the query is answered so use with caution
    */
    virtual std::string
        query(std::string_view target, std::string_view queryStr, HelicsSequencingModes mode) = 0;

    /** supply a query callback function
    @details the intention of the query callback is to allow federates to answer particular requests
    through the query interface this allows other federates to make requests or queries of other
    federates in an asynchronous fashion.
    @param federateID the identifier for the federate
    @param queryFunction  a function object that returns a string as a result of a query in the form
    of const string ref. This callback will be called when a federate received a query that cannot
    be answered that directed at a particular federate
    @param order indicator of the execution order slot for query callbacks; the value is bound
    [1,10] inclusive and values given outside this range are clamped to the boundary values. The
    callback is overwritten if multiple callbacks at the same index are given.
    */
    virtual void setQueryCallback(LocalFederateId federateID,
                                  std::function<std::string(std::string_view)> queryFunction,
                                  int order) = 0;

    /**
     * setter for the interface information
     * @param handle the identifiers for the interface to set the info data on
     * @param info a string containing the info data
     */
    virtual void setInterfaceInfo(InterfaceHandle handle, std::string_view info) = 0;

    /**
     * getter for the interface information
     * @param handle the identifiers for the interface to query
     * @return a string containing the Info data stored in an interface
     */
    virtual const std::string& getInterfaceInfo(InterfaceHandle handle) const = 0;
    /**
     * setter for interface tags which are key-value pairs
     * @param handle the identifier for the interface to set the tag data on
     * @param tag a string containing the name of the tag
     * @param value a string containing the value for the tag
     */
    virtual void
        setInterfaceTag(InterfaceHandle handle, std::string_view tag, std::string_view value) = 0;
    /**
     * getter for the interface tags
     * @param handle the identifier for the interface to set the info data on
     * @param tag the name of the tag to retrieve
     */
    virtual const std::string& getInterfaceTag(InterfaceHandle handle,
                                               std::string_view tag) const = 0;

    /**
     * setter for federate tags which are key-value pairs
     * @param fid the identifier for the federate to set the tag data on
     * @param tag a string containing the name of the tag
     * @param value a string containing the value for the tag
     */
    virtual void
        setFederateTag(LocalFederateId fid, std::string_view tag, std::string_view value) = 0;
    /**
     * getter for the federate tags
     * @param fid the identifier for the federate to get the tag data for
     * @param tag the name of the tag to retrieve
     * @return a reference to a const std::string with the tag value.
     */
    virtual const std::string& getFederateTag(LocalFederateId fid, std::string_view tag) const = 0;
};

}  // namespace helics
