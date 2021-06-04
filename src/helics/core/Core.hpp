/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "core-data.hpp"
#include "federate_id.hpp"

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
 * Implementations should be thread safe.
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
     * Simulator control.
     */
    /**
     * Configure the core.
     *
     * Should be invoked a single time to configure the co-simulation core for operation
     *
     */
    [[deprecated("please use configure instead")]] void
        initialize(const std::string& configureString)
    {
        configure(configureString);
    }
    /**Configure the core from command line arguments.
     *
     * Should be invoked a single time to initialize the co-simulation core for operation
     *
     */
    [[deprecated("please use configureFromArgs instead")]] void initializeFromArgs(int argc,
                                                                                   char* argv[])
    {
        configureFromArgs(argc, argv);
    }
    /**
     * Configure the core from a configuration string
     *
     * Should be invoked a single time to initialize the co-simulation core.
     *
     */
    virtual void configure(const std::string& configureString) = 0;
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
     * Returns true if the core has been configured.
     */
    [[deprecated("please use isConfigured instead")]] bool isInitialized() const
    {
        return isConfigured();
    }
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
    @param federateID the federate
    */
    virtual void globalError(local_federate_id federateID,
                             int32_t errorCode,
                             const std::string& errorString) = 0;

    /**
     * Federate has encountered a local error and should be disconnected.
     */
    virtual void localError(local_federate_id federateID,
                            int32_t errorCode,
                            const std::string& errorString) = 0;

    /**
     * Federate has encountered an unrecoverable error.
     */
    void error(local_federate_id federateID, int32_t errorCode = -1)
    {
        globalError(federateID, errorCode, "");
    }
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
    virtual void finalize(local_federate_id federateID) = 0;

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
     * May only be invoked in Created state otherwise an error is thrown
     */
    virtual void enterInitializingMode(local_federate_id federateID) = 0;

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
     *@return an iteration result enumeration value indicating the current state of iterations
     */
    virtual iteration_result enterExecutingMode(local_federate_id federateID,
                                                iteration_request iterate = NO_ITERATION) = 0;

    /**
     * Register a federate.
     *
     * The returned FederateId is local to invoking process,
     * FederateId's should not be used as a global identifier.
     *
     * May only be invoked in initialize state otherwise throws an error
     */
    virtual local_federate_id registerFederate(const std::string& name,
                                               const CoreFederateInfo& info) = 0;

    /**
     * Returns the federate name.
     *
     */
    virtual const std::string& getFederateName(local_federate_id federateID) const = 0;

    /**
     * Returns the federate Id.
     *
     */
    virtual local_federate_id getFederateId(const std::string& name) const = 0;

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
    virtual Time timeRequest(local_federate_id federateID, Time next) = 0;

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
    virtual iteration_time requestTimeIterative(local_federate_id federateID,
                                                Time next,
                                                iteration_request iterate) = 0;

    /**
     * Returns the current reiteration count for the specified federate.
     */
    virtual uint64_t getCurrentReiteration(local_federate_id federateID) const = 0;

    /** set a timebased property on a federate
    @param federateID the federate to set a time based property on
    @param property the property to set see /ref defs::properties
    @param timeValue the requested value of the property
    */
    virtual void
        setTimeProperty(local_federate_id federateID, int32_t property, Time timeValue) = 0;
    /** get a timebased property on a federate
    @param federateID the federate to set a time based property on
    @param property the property to set see /ref defs::properties
    @return the current value of the requested property
    */
    virtual Time getTimeProperty(local_federate_id federateID, int32_t property) const = 0;
    /** set an integer property on a federate
    @param federateID the federate to set a time based property on
    @param property the property to set see /ref defs::properties
    @param propValue the requested value of the property
    */
    virtual void
        setIntegerProperty(local_federate_id federateID, int32_t property, int16_t propValue) = 0;
    /** get an integer property on a federate
    @param federateID the federate to set a time based property on
    @param property the property to set see /ref defs::properties
    @return the current value of the property
    */
    virtual int16_t getIntegerProperty(local_federate_id federateID, int32_t property) const = 0;
    /** get the most recent granted Time
    @param federateID the identifier of the federate to get the time
    @return the most recent granted time or the startup time
    */
    virtual Time getCurrentTime(local_federate_id federateID) const = 0;

    /**
    Set a flag in a a federate
    * @param federateID  the identifier for the federate
    * @param flag an index code for the flag to set
    * @param flagValue the value to set the flag to
    */
    virtual void setFlagOption(local_federate_id federateID, int32_t flag, bool flagValue) = 0;

    /**
    Set a flag in a a federate
    * @param federateID  the identifier for the federate
    * @param flag an index code for the flag to set
    * @return the value of the flag
    */
    virtual bool getFlagOption(local_federate_id federateID, int32_t flag) const = 0;
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
    virtual interface_handle registerPublication(local_federate_id federateID,
                                                 const std::string& key,
                                                 const std::string& type,
                                                 const std::string& units) = 0;

    /** get a publication Handle from its key
    @param federateID the identifier for the federate
    @param key the name of the publication
     @return a handle to identify the publication*/
    virtual interface_handle getPublication(local_federate_id federateID,
                                            const std::string& key) const = 0;

    /**
     * Register a control input for the specified federate.
     *
     * May only be invoked in the initialize state.
     * @param federateID the identifier for the federate to register an input interface on
     * @param key the name of the control input
     * @param type a string describing the type of the federate
     * @param units a string naming the units of the federate
     */
    virtual interface_handle registerInput(local_federate_id federateID,
                                           const std::string& key,
                                           const std::string& type,
                                           const std::string& units) = 0;
    /** get a subscription Handle from its key
    @param federateID the identifier for the federate
    @param key the tag of the named input
    @return a handle to identify the input*/
    virtual interface_handle getInput(local_federate_id federateID,
                                      const std::string& key) const = 0;

    /**
     * Returns the name or identifier for a specified handle
     */
    virtual const std::string& getHandleName(interface_handle handle) const = 0;
    /** remove a target from a handles operation
     *@param handle the handle from the publication, input, endpoint or filter
     *@param targetToRemove the name of the target to remove
     */
    virtual void removeTarget(interface_handle handle, const std::string& targetToRemove) = 0;

    /**
     * @return the unit string for the specified handle.
     */
    virtual const std::string& getExtractionUnits(interface_handle handle) const = 0;
    /** get the injection units for an interface,  this is the type for data coming into an
     *interface
     *@details for publications this is the units associated with the transmitted data,  for inputs
     *this is the units of the transmitting publication
     *@param handle the interface handle to get the injection type for
     *
     *@return a const ref to  std::string
     */
    virtual const std::string& getInjectionUnits(interface_handle handle) const = 0;

    /**
     * Returns units for specified handle.
     */
    [[deprecated("please use getExtractionUnits instead")]] const std::string&
        getUnits(interface_handle handle) const
    {
        return getExtractionUnits(handle);
    }
    /** get the injection type for an interface,  this is the type for data coming into an interface
     *@details for filters this is the input type, for publications this is type used to transmit
     *data, for endpoints this is the specified type and for inputs this is the type of the
     *transmitting publication
     *@param handle the interface handle to get the injection type for
     *@return a const ref to  std::string
     */
    virtual const std::string& getInjectionType(interface_handle handle) const = 0;

    /** get the type for which data comes out of an interface,  this is the type for data coming
    into an interface
    @details for filters this is the output type, for publications this is the specified type, for
    endpoints this is the specified type and for inputs this is the specified type
    @param handle the interface handle to get the injection type for
    @return a const ref to  std::string  */
    virtual const std::string& getExtractionType(interface_handle handle) const = 0;

    /** set a handle option
     *@param handle the handle from the publication, input, endpoint or filter
     *@param option the option to set
     *@param option_value the value to set the option (mostly 0 or 1)
     */
    virtual void setHandleOption(interface_handle handle, int32_t option, int32_t option_value) = 0;

    /** get a handle option
    @param handle the handle from the publication, input, endpoint or filter
    @param option the option to set see /ref defs::options
    */
    virtual int32_t getHandleOption(interface_handle handle, int32_t option) const = 0;

    /** close a handle from further connections
    @param handle the handle from the publication, input, endpoint or filter
    */
    virtual void closeHandle(interface_handle handle) = 0;
    /**
     * Publish specified data to the specified key.
     *
     @param handle the handle from the publication, input, endpoint or filter
     @param data the raw data to send
     @param len the size of the data
     */
    virtual void setValue(interface_handle handle, const char* data, uint64_t len) = 0;

    /**
     * Return the data for the specified handle or the latest input
     * @param handle the input handle from which to get the data
     * @param[out] inputIndex return the index of input (always 1 for inputs with only a single
     * source)
     */
    virtual const std::shared_ptr<const data_block>& getValue(interface_handle handle,
                                                              uint32_t* inputIndex = nullptr) = 0;

    /**
     * Return all the available data for the specified handle or the latest input
     *
     */
    virtual const std::vector<std::shared_ptr<const data_block>>&
        getAllValues(interface_handle handle) = 0;

    /**
     * Returns vector of input handles that received an update during the last
     * time request.  The data remains valid until the next call to getValueUpdates for the given
     federateID
     *@param federateID the identification code of the federate to get which interfaces have been
     updated
     @return a reference to the location of an array of handles that have been updated
     */
    virtual const std::vector<interface_handle>& getValueUpdates(local_federate_id federateID) = 0;

    /**
     * Message interface.
     * Designed for point-to-point communication patterns.
     */

    /**
     * Register an endpoint.
     *
     * May only be invoked in the Initialization state.
     */
    virtual interface_handle registerEndpoint(local_federate_id federateID,
                                              const std::string& name,
                                              const std::string& type) = 0;

    /** get an endpoint Handle from its name
    @param federateID the identifier for the federate
    @param name the name of the endpoint
    @return a handle to identify the endpoint*/
    virtual interface_handle getEndpoint(local_federate_id federateID,
                                         const std::string& name) const = 0;

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
    virtual interface_handle registerCloningFilter(const std::string& filterName,
                                                   const std::string& type_in,
                                                   const std::string& type_out) = 0;

    /**
     * Register source filter.
     *
     * May only be invoked in the Initialization state.
     @param filterName the name of the filter (may be left blank and one will be automatically
     assigned)
     @param type_in the input type of the filter
     @param type_out the output type of the filter (may be left blank if the filter doesn't change
     type) this is important for ordering in filters with operators
     @return the handle for the new filter
     */
    virtual interface_handle registerFilter(const std::string& filterName,
                                            const std::string& type_in,
                                            const std::string& type_out) = 0;
    /**
    * add a destination target,  the handle can be for a filter or a publication
    @details a filter will create an additional processing step for messages before they get to a
    destination endpoint, for publications this will establish a linkage from the publication to the
    named input
    *
    @param handle an interface to add the target to
    @param dest the target endpoint for the filter
    */
    virtual void addDestinationTarget(interface_handle handle, const std::string& dest) = 0;

    /** add a source target,  the handle can be a subscription, input, filter or endpoint
    @details for subscriptions and inputs this establishes a link from a publication, for endpoints
    this creates a linkage to a particular publication, for filters it add a source endpoint to
    filter
    @param handle the identifier of the interface
    @param name the name of the filter or its target
    */
    virtual void addSourceTarget(interface_handle handle, const std::string& name) = 0;

    /** get a destination filter Handle from its name or target(this may not be unique so it will
    only find the first one)
    @param name the name of the filter or its target
    @return a handle to identify the filter*/
    virtual interface_handle getFilter(const std::string& name) const = 0;

    /**
    * add a time dependency between federates
    * @details this function is primarily useful for Message federates which do not otherwise
    restrict the dependencies
    * adding a dependency gives additional information to the core that the specified federate(given
    by id) will be sending Messages to the named Federate(by federateName)
    @param federateID  the identifier for the federate
    @param federateName the name of the dependent federate
    */
    virtual void addDependency(local_federate_id federateID, const std::string& federateName) = 0;
    /**
     * Register known frequently communicating source/destination end points.
     *
     * May be used for error checking for compatible types and possible optimization by
     * pre-registering the intent for these endpoints to communicate.
     */
    virtual void registerFrequentCommunicationsPair(const std::string& source,
                                                    const std::string& dest) = 0;

    /** load a file containing connection information
    @param file a JSON or TOML file containing connection information*/
    virtual void makeConnections(const std::string& file) = 0;

    /** create a data connection between a named publication and a named input
    @param source the name of the publication
    @param target the name of the input*/
    virtual void dataLink(const std::string& source, const std::string& target) = 0;
    /** create a filter connection between a named filter and a named endpoint for messages coming
    from that endpoint
    @param filter the name of the filter
    @param target the name of the source target*/
    virtual void addSourceFilterToEndpoint(const std::string& filter,
                                           const std::string& target) = 0;
    /** create a filter connection between a named filter and a named endpoint for destination
    processing
    @param filter the name of the filter
    @param target the name of the source target*/
    virtual void addDestinationFilterToEndpoint(const std::string& filter,
                                                const std::string& target) = 0;
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
    virtual void send(interface_handle sourceHandle,
                      const std::string& destination,
                      const char* data,
                      uint64_t length) = 0;

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
     @param time the time the event is scheduled for
     @param sourceHandle the source of the event
     @param destination  the target of the event
     @param data the raw data for the event
     @param length the record length of the event
     */
    virtual void sendEvent(Time time,
                           interface_handle sourceHandle,
                           const std::string& destination,
                           const char* data,
                           uint64_t length) = 0;

    /**
     * Send for filters.
     *
     * Continues sending the message to the next filter or to final destination.
     *
     */
    virtual void sendMessage(interface_handle sourceHandle, std::unique_ptr<Message> message) = 0;

    /**
     * Returns the number of pending receives for the specified destination endpoint.
     */
    virtual uint64_t receiveCount(interface_handle destination) = 0;

    /**
     * Returns the next buffered message the specified destination endpoint.
     @details this is a non-blocking call and will return a nullptr if no message are available
     */
    virtual std::unique_ptr<Message> receive(interface_handle destination) = 0;

    /**
     * Receives a message for any destination.
     @details this is a non-blocking call and will return a nullptr if no messages are available
     @param federateID the identifier for the federate
     @param[out] endpoint_id the endpoint handle related to the message gets stored here
     */
    virtual std::unique_ptr<Message> receiveAny(local_federate_id federateID,
                                                interface_handle& endpoint_id) = 0;

    /**
     * Returns number of messages for all destinations.
     */
    virtual uint64_t receiveCountAny(local_federate_id federateID) = 0;

    /** send a log message to the Core for logging
    @param federateID the federate that is sending the log message
    @param logLevel  an integer for the log level /ref helics_log_levels
    @param messageToLog the string to send to a logger
    */
    virtual void
        logMessage(local_federate_id federateID, int logLevel, const std::string& messageToLog) = 0;

    /** set the filter callback operator
    @param filter  the handle of the filter
    @param callback pointer to the operator class executing the filter
    */
    virtual void setFilterOperator(interface_handle filter,
                                   std::shared_ptr<FilterOperator> callback) = 0;

    /** define a logging function to use for logging message and notices from the federation and
    individual federate
    @param federateID  the identifier for the individual federate or 0 for the Core Logger
    @param logFunction the callback function for doing something with a log message
    it takes 3 inputs an integer for logLevel /ref helics_log_levels
    A string indicating the source of the message and another string with the actual message
    */
    virtual void setLoggingCallback(
        local_federate_id federateID,
        std::function<void(int, const std::string&, const std::string&)> logFunction) = 0;

    /** set the core logging level*/
    virtual void setLoggingLevel(int logLevel) = 0;

    /** set the core logging file*/
    virtual void setLogFile(const std::string& lfile) = 0;

    /** set a federation global value
    @details this overwrites any previous value for this name
    @param valueName the name of the global to set
    @param value the value of the global
    */
    virtual void setGlobal(const std::string& valueName, const std::string& value) = 0;
    /** make a query for information from the co-simulation
    @details the format is somewhat unspecified  target is the name of an object typically one of
    "federation",  "broker", "core", or the name of a specific object/core/broker
    target can also be "global" to query a global value stored in the broker
    @param target the specific target of the query
    @param queryStr the actual query
    @param mode the synchronization mode for the query
    @return a string containing the response to the query.  Query is a blocking call and will not
    return until the query is answered so use with caution
    */
    virtual std::string query(const std::string& target,
                              const std::string& queryStr,
                              helics_sequencing_mode mode) = 0;

    /** supply a query callback function
    @details the intention of the query callback is to allow federates to answer particular requests
    through the query interface this allows other federates to make requests or queries of other
    federates in an asynchronous fashion.
    @param federateID the identifier for the federate
    @param queryFunction  a function object that returns a string as a result of a query in the form
    of const string ref. This callback will be called when a federate received a query that cannot
    be answered that directed at a particular federate
    */
    virtual void setQueryCallback(local_federate_id federateID,
                                  std::function<std::string(const std::string&)> queryFunction) = 0;

    /**
     * setter for the interface information
     * @param handle the identifiers for the interface to set the info data on
     * @param info a string containing the info data
     */
    virtual void setInterfaceInfo(interface_handle handle, std::string info) = 0;

    /**
     * getter for the interface information
     * @param handle the identifiers for the interface to query
     * @return a string containing the Info data stored in an interface
     */
    virtual const std::string& getInterfaceInfo(interface_handle handle) const = 0;
};

}  // namespace helics
