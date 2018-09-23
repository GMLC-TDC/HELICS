/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "core-data.hpp"
#include "federate_id.hpp"
#include <functional>
#include <utility>

/**
 * HELICS Core API
 */
namespace helics
{
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

class CoreFederateInfo;

/** the class defining the core interface through an abstract class*/
class Core
{
  public:
    /** default constructor*/
    Core () = default;
    /**virtual destructor*/
    virtual ~Core () = default;

    /**
     * Simulator control.
     */

    /**
     * Initialize the core.
     *
     * Should be invoked a single time to initialize the co-simulation core.
     *
     */
    virtual void initialize (const std::string &initializationString) = 0;
    /**
     * Initialize the core from command line arguments.
     *
     * Should be invoked a single time to initialize the co-simulation core.
     *
     */
    virtual void initializeFromArgs (int argc, const char *const *argv) = 0;
    /**
     * Returns true if the core has been initialized.
     */
    virtual bool isInitialized () const = 0;
    /**
    * connect the core to a broker if needed
    @return true if the connection was successful
    */
    virtual bool connect () = 0;
    /**
     * check if the core is connected properly
     */
    virtual bool isConnected () const = 0;

    /**
     * disconnect the core from its broker
     */
    virtual void disconnect () = 0;
    /** waits in the current thread until the core is disconnected
     */
    virtual void waitForDisconnect (int msToWait = -1) const = 0;

    /** check if the core is ready to accept new federates
     */
    virtual bool isOpenToNewFederates () const = 0;
    /** get an identifier string for the core
     */
    virtual const std::string &getIdentifier () const = 0;
    /** get the connection network or connection address for the core*/
    virtual const std::string &getAddress () const = 0;
    /**
     * Federate has encountered an unrecoverable error.
     */
    virtual void error (federate_id_t federateID, int32_t errorCode = -1) = 0;

    /**
     * Federate has completed.
     *
     * Should be invoked a single time to complete the simulation.
     *
     */
    virtual void finalize (federate_id_t federateID) = 0;

    /**
     * Federates may be in four Modes.
     *    -# Startup
     *       Configuration of the federate.
     *       State begins when registerFederate() is invoked and ends when enterInitializingMode() is invoked.
     *    -# Initializing
     *       Configure of the simulation state prior to the start of time stepping.
     *       State begins when enterInitializingMode() is invoked and ends when enterExecutingMode(true) is
     *       invoked.
     *    -# Executing
     *       State begins when enterExecutingMode() is invoked and ends when finalize() is invoked.
     *    -# Finalized
     *       state after finalize is invoked.
     */

    /**
     * Change the federate state to the Initializing state.
     *
     * May only be invoked in Created state otherwise an error is thrown
     */
    virtual void enterInitializingMode (federate_id_t federateID) = 0;

    /** set the core to ready to enter init
    @details this function only needs to be called for cores that don't have any federates but may
    have filters for cores with federates it won't do anything*/
    virtual void setCoreReadyToInit () = 0;

    /**
     * Change the federate state to the Executing state.
     *
     * May only be invoked in Initializing state.
     @param[in] federateID  the identifier of the federate
     @param[in] iterationCompleted  if true no more iterations on this federate are requested
     if nonconverged the federate requests an iterative update
     @return nonconverged if the executing state has not been entered and there are updates, complete if the
     simulation is ready to move on to the executing state
     */
    virtual iteration_result
    enterExecutingMode (federate_id_t federateID, iteration_request iterate = NO_ITERATION) = 0;

    /**
     * Register a federate.
     *
     * The returned FederateId is local to invoking process,
     * FederateId's should not be used as a global identifier.
     *
     * May only be invoked in initialize state otherwise throws an error
     */
    virtual federate_id_t registerFederate (const std::string &name, const CoreFederateInfo &info) = 0;

    /**
     * Returns the federate name.
     *
     */
    virtual const std::string &getFederateName (federate_id_t federateID) const = 0;

    /**
     * Returns the federate Id.
     *
     */
    virtual federate_id_t getFederateId (const std::string &name) const = 0;

    /**
     * Returns the global number of federates that are registered only return accurately after the initialization
     * state has been entered
     */
    virtual int32_t getFederationSize () = 0;

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
     * @param next
     */
    virtual Time timeRequest (federate_id_t federateID, Time next) = 0;

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
     * @param localConverged has the local federate converged
     @return an iteration_time object with two field grantedTime and a enumeration indicating the state of the
     iteration
     */
    virtual iteration_time
    requestTimeIterative (federate_id_t federateID, Time next, iteration_request iterate) = 0;

    /**
     * Returns the current reiteration count for the specified federate.
     */
    virtual uint64_t getCurrentReiteration (federate_id_t federateID) const = 0;

    virtual void setTimeProperty(federate_id_t federateID, int32_t property, Time timeValue) = 0;

    virtual Time getTimeProperty(federate_id_t federateID, int32_t property) const=0;

    virtual void setIntegerProperty(federate_id_t federateID, int32_t property, int16_t propValue) = 0;

    virtual int16_t getIntegerProperty(federate_id_t federateID, int32_t property) const = 0;
    /** get the most recent granted Time
    @param federateID, the id of the federate to get the time
    @return the most recent granted time or the startup time
    */
    virtual Time getCurrentTime (federate_id_t federateID) const = 0;
    /**
     * Set the maximum number of iterations allowed.
     *
     * The minimum value set in any federate is used.
     *
     * Default value is the maximum allowed value for uint64_t.
     *
     * May only be invoked in the initialize state.
     */

  //  virtual void setMaximumIterations (federate_id_t federateID, int32_t iterations) = 0;

    /**
     * Set the minimum time resolution for the specified federate.
     *
     * The value is used to constrain when the timeRequest methods
     * return to values that are multiples of the specified delta.
     * This is useful for federates that are time-stepped and making
     * sub-time-step updates is not meaningful.
     *
     * @param time
     */
 //   virtual void setTimeProperty (TIME_DELTA_PROPERTY, federate_id_t federateID, Time time) = 0;

    /**
     * Set the outputDelay time for the specified federate.
     *
     * The value is used to determine the interaction amongst various federates as to
     * when a specific federate can influence another
     * @param federateID  the identifier for the federate
     * @param timeoutputDelay
     */
  //  virtual void setOutputDelay (federate_id_t federateID, Time timeoutputDelay) = 0;
    /**
     * Set the period for a specified federate.
     *
     * The value is used to determine the interaction amongst various federates as to
     * when a specific federate can influence another
     * @param federateID  the identifier for the federate
     * @param timeoutputDelay
     */
  //  virtual void setTimeProperty (PERIOD_PROPERTY, federate_id_t federateID, Time timePeriod) = 0;
    /**
    * Set the periodic offset for a specified federate.
    *
    * The value is used as a time shift for calculating the allowable time in a federate
    the granted time must one of N*period+offset




    * @param federateID  the identifier for the federate
    * @param timeOffset the periodic phase shift
    */
   // virtual void setTimeOffset (federate_id_t federateID, Time timeOffset) = 0;
    /**
     * Set the inputDelay time.
     *
     * The value is used to determine the interaction amongst various federates as to
     * when a specific federate can influence another
     * @param federateID  the identifier for the federate
     * @param timeImpact the length of time it take outside message to propagate into a federate
     */
   // virtual void setInputDelay (federate_id_t federateID, Time timeImpact) = 0;
    /**
    Set the logging level
    @details set the logging level for an individual federate
    set federateID to 0 for the core logging level
    * @param federateID  the identifier for the federate
    * @param loggingLevel the level of logging to enable
    <0-no logging, 0 -error only, 1- warnings, 2-normal, 3-debug, 4-trace
    */
  //  virtual void setLoggingLevel (federate_id_t federateID, int loggingLevel) = 0;

    /**
    Set a flag in a a federate
    * @param federateID  the identifier for the federate
    * @param flag an index code for the flag to set
    @param flagValue the value to set the flag to
    */
    virtual void setFlagOption (federate_id_t federateID, int32_t flag, bool flagValue) = 0;

    /**
    Set a flag in a a federate
    * @param federateID  the identifier for the federate
    * @param flag an index code for the flag to set
    @param flagValue the value to set the flag to
    */
    virtual bool getFlagOption(federate_id_t federateID, int32_t flag) const = 0;
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
    virtual interface_handle registerPublication (federate_id_t federateID,
                                             const std::string &key,
                                             const std::string &type,
                                             const std::string &units) = 0;

    /** get a publication Handle from its key
    @param federateID the identifier for the federate
    @key the name of the publication
     @return a handle to identify the publication*/
    virtual interface_handle getPublication (federate_id_t federateID, const std::string &key) const = 0;

    /**
    * Register a control input for the specified federate.
    *
    * May only be invoked in the initialize state.
    * @param[in] federateID
    * @param[in] key the name of the control input
    * @param[in] type a string describing the type of the federate
    * @param[in] units a string naming the units of the federate
    */
    virtual interface_handle registerInput(federate_id_t federateID,
        const std::string &key,
        const std::string &type,
        const std::string &units) = 0;
    /** get a subscription Handle from its key
    @param federateID the identifier for the federate
    @key the tag of the named input
    @return a handle to identify the input*/
    virtual interface_handle getInput(federate_id_t federateID, const std::string &key) const = 0;

    /**
     * Returns the name or identifier for a specified handle
     */
    virtual const std::string &getHandleName (interface_handle handle) const = 0;
    /** remove a target from a handles operation
	@param handle the handle to remove the target on
	@param targetToRemove the name of the target to remove*/
    virtual void removeTarget(interface_handle handle, const std::string &targetToRemove) = 0;

    /**
     * Returns units for specified handle.
     */
    virtual const std::string &getUnits (interface_handle handle) const = 0;

    /**
     * Returns type for specified handle.
     @details for endpoints, publications, and filters, this is the input type
     for subscriptions and control inputs this is the type of the publication or control input(if available)
     @param handle the handle from the publication, subscription,control_input/output, endpoint or filter
     */
    virtual const std::string &getType (interface_handle handle) const = 0;

    /**
    * Returns output type for specified handle.
    @details for filters this is the outputType, for Subscriptions and control inputs this is the expected type
    for endpoints and publications and control Inputs this is the same as getType();
    @param handle the handle from the interface
    */
    virtual const std::string &getOutputType (interface_handle handle) const = 0;

    /** set a handle option 
    @param handle the handle to set the option for
    @param option the option to set
    @param value the value to set the option (mostly 0 or 1)
    */
    virtual void setHandleOption(interface_handle handle, int32_t option, bool option_value) = 0;

    /** get a handle option
    @param handle the handle to set the option for
    @param option the option to set
    @param value the value to set the option (mostly 0 or 1)
    */
    virtual bool getHandleOption(interface_handle handle, int32_t option) const = 0;

    /**
     * Publish specified data to the specified key.
     *
     * @param handle a handle to a publication or control output to use for the value
     @param[in] data the raw data to send
     @param len the size of the data
     */
    virtual void setValue (interface_handle handle, const char *data, uint64_t len) = 0;

    /**
     * Return the data for the specified handle or the latest input
     *
     */
    virtual std::shared_ptr<const data_block> getValue (interface_handle handle) = 0;
    /**
    * Return all the data for the specified handle or the latest input
    *
    */
    virtual std::vector<std::shared_ptr<const data_block>> getAllValues(interface_handle handle) = 0;

    /**
     * Returns vector of input handles that received an update during the last
     * time request.  The data remains valid until the next call to getValueUpdates for the given federateID
     *@param federateID the identification code of the federate to get which interfaces have been updated
     @return a reference to the location of an array of handles that have been updated
     */
    virtual const std::vector<interface_handle> &getValueUpdates (federate_id_t federateID) = 0;

    /**
     * Message interface.
     * Designed for point-to-point communication patterns.
     */

    /**
     * Register an endpoint.
     *
     * May only be invoked in the Initialization state.
     */
    virtual interface_handle
    registerEndpoint (federate_id_t federateID, const std::string &name, const std::string &type) = 0;

    /** get an endpoint Handle from its name
    @param federateID the identifier for the federate
    @param name the name of the endpoint
    @return a handle to identify the endpoint*/
    virtual interface_handle getEndpoint (federate_id_t federateID, const std::string &name) const = 0;

    /**
    * Register a cloning filter, a cloning filter operates on a copy of the message vs the actual message
    *
    @param filterName the name of the filter (may be left blank and one will be automatically assigned)
    @param type_in the input type of the filter
    @param type_out the output type of the filter (may be left blank if the filter doesn't change type)
    @return the handle for the new filter
    */
    virtual interface_handle registerCloningFilter(const std::string &filterName,
        const std::string &type_in,
        const std::string &type_out) = 0;

    /**
     * Register source filter.
     *
     * May only be invoked in the Initialization state.
     @param filterName the name of the filter (may be left blank and one will be automatically assigned)
     @param type_in the input type of the filter
     @param type_out the output type of the filter (may be left blank if the filter doesn't change type)
     this is important for ordering in filters with operators
     @return the handle for the new filter
     */ 
    virtual interface_handle registerFilter (const std::string &filterName,
                                              const std::string &type_in,
                                              const std::string &type_out) = 0;
    /**
    * add a destination target,  the handle can be for a filter or a publication
    @details a filter will create an additional processing step for messages before they get to a
    destination endpoint, for publications this will establish a linkage from the publication to the named input
    *
    * May only be invoked in the Initialization state.
    @param filterName the name of the filter (may be left blank)
    @param dest the target endpoint for the filter
    @param type_in the input type of the filter (may be left blank,  this is for error checking and will produce a
    warning if it doesn't match with the input type of the target endpoint
    @return the handle for the new filter
    */
    virtual void addDestinationTarget (interface_handle handle, const std::string &dest) = 0;

    /** add a source target,  the handle can be a subscription, input, filter or endpoint
    @details for subscriptions and inputs this establishes a link from a publication, for endpoints this creates a linkage to a particular publication,
    for filters it add a source endpoint to filter
    @param handle the identifier of the interface 
    @param name the name of the filter or its target
    @return a handle to identify the filter*/
    virtual void addSourceTarget (interface_handle handle, const std::string &name) = 0;

    /** get a destination filter Handle from its name or target(this may not be unique so it will only find the
    first one)
    @param name the name of the filter or its target
    @return a handle to identify the filter*/
    virtual interface_handle getFilter (const std::string &name) const = 0;

    /**
    * add a time dependency between federates
    * @details this function is primarily useful for Message federates which do not otherwise restrict the
    dependencies
    * adding a dependency gives additional information to the core that the specified federate(given by id) will be
    sending Messages to the named Federate(by federateName)
    @param[in] federateID  the identifier for the federate
    @param[in] federateName the name of the dependent federate
    */
    virtual void addDependency (federate_id_t federateID, const std::string &federateName) = 0;
    /**
     * Register known frequently communicating source/destination end points.
     *
     * May be used for error checking for compatible types and possible optimization by
     * pre-registering the intent for these endpoints to communicate.
     */
    virtual void registerFrequentCommunicationsPair (const std::string &source, const std::string &dest) = 0;

    /** create a data connection between a named publication and a named input
    @param source the name of the publication 
    @param target the name of the input*/
    virtual void dataConnect(const std::string &source, const std::string &target) = 0;
    /** create a filter connection between a named publication and a named input
    @param source the name of the filter
    @param target the name of the source target*/
    virtual void filterAddSourceTarget(const std::string &filter, const std::string &target) = 0;
    /** create a filter connection between a named publication and a named input
    @param source the name of the filter
    @param target the name of the source target*/
    virtual void filterAddDestinationTarget(const std::string &filter, const std::string &target) = 0;
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
    virtual void
    send (interface_handle sourceHandle, const std::string &destination, const char *data, uint64_t length) = 0;

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
    virtual void sendEvent (Time time,
                            interface_handle sourceHandle,
                            const std::string &destination,
                            const char *data,
                            uint64_t length) = 0;

    /**
     * Send for filters.
     *
     * Continues sending the message to the next filter or to final destination.
     *
     */
    virtual void sendMessage (interface_handle sourceHandle, std::unique_ptr<Message> message) = 0;

    /**
     * Returns the number of pending receives for the specified destination endpoint.
     */
    virtual uint64_t receiveCount (interface_handle destination) = 0;

    /**
     * Returns the next buffered message the specified destination endpoint.
     @details this is a non-blocking call and will return a nullptr if no message are available
     */
    virtual std::unique_ptr<Message> receive (interface_handle destination) = 0;

    /**
     * Receives a message for any destination.
     @details this is a non-blocking call and will return a nullptr if no messages are available
     @param federateID the identifier for the federate
     @param[out] endpoint_id the endpoint handle related to the message gets stored here
     */
    virtual std::unique_ptr<Message> receiveAny (federate_id_t federateID, interface_handle &enpoint_id) = 0;

    /**
     * Returns number of messages for all destinations.
     */
    virtual uint64_t receiveCountAny (federate_id_t federateID) = 0;

    /** send a log message to the Core for logging
    @param[in] federateID the federate that is sending the log message
    @param[in] logLevel  an integer for the log level (0- error, 1- warning, 2-status, 3-debug)
    @param[in] messageToLog
    */
    virtual void logMessage (federate_id_t federateID, int logLevel, const std::string &messageToLog) = 0;

    /** set the filter callback operator
    @param[in] filter  the handle of the filter
    @param[in] operator pointer to the operator class executing the filter
    */
    virtual void setFilterOperator (interface_handle filter, std::shared_ptr<FilterOperator> callback) = 0;

    /** define a logging function to use for logging message and notices from the federation and individual
    federate
    @param federateID  the identifier for the individual federate or 0 for the Core Logger
    @param logFunction the callback function for doing something with a log message
    it takes 3 inputs an integer for logLevel 0-4+  0 -error, 1- warning 2-status, 3-debug 44trace
    A string indicating the source of the message and another string with the actual message
    */
    virtual void
    setLoggingCallback (federate_id_t federateID,
                        std::function<void(int, const std::string &, const std::string &)> logFunction) = 0;

	/** set the core logging level*/
    virtual void setLoggingLevel (int logLevel) = 0;

    /** make a query for information from the co-simulation
    @details the format is somewhat unspecified  target is the name of an object typically one of
    "federation",  "broker", "core", or the name of a specific object
    query is a broken
    @param target the specific target of the query
    @param queryStr the actual query
    @return a string containing the response to the query.  Query is a blocking call and will not return until the
    query is answered so use with caution
    */
    virtual std::string query (const std::string &target, const std::string &queryStr) = 0;
    /** supply a query callback function
    @details the intention of the query callback is to allow federates to answer particular requests through the
    query interface this allows other federates to make requests or queries of other federates in an asynchronous
    fashion.
    @param federateID the identifier for the federate
    @param queryFunction  a function object that returns a string as a result of a query in the form of const
    string ref. This callback will be called when a federate received a query that cannot be answered that directed
    at a particular federate
    */
    virtual void setQueryCallback (federate_id_t federateID,
                                   std::function<std::string (const std::string &)> queryFunction) = 0;
};

}  // namespace helics
