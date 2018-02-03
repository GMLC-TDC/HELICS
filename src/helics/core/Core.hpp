/*

Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_CORE_
#define _HELICS_CORE_
#pragma once

#include "core-data.hpp"
#include <functional>
#include <memory>
#include <string>
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
 *  For Memory management all message_t and data_t pointers return from the core API should be released via
 a call to the dereference function,  the core assumes all data given it via send or publish calls could be invalid
 after the call returns
 *
 * Implementations should be thread safe.
 *
 * Note: Methods should all be pure virtual, leaving syntactical sugar off while iterating API design.
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
     * FederateID uniquely identifies a federate.
     */
    using federate_id_t = int32_t;

    /**
     * HandleID uniquely identifies a handle.
     */
    using handle_id_t = int32_t;

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

    /** check if the core is ready to accept new federates
     */
    virtual bool isOpenToNewFederates () const = 0;
    /** get and identifier string for the core
     */
    virtual const std::string &getIdentifier () const = 0;
    /**
     * Federate has encountered an unrecoverable error.
     */
    virtual void error (federate_id_t federateID, int errorCode = -1) = 0;

    /**
     * Federate has completed.
     *
     * Should be invoked a single time to complete the simulation.
     *
     */
    virtual void finalize (federate_id_t federateID) = 0;

    /**
     * Federates may be in three states.
     *
     * Federates are in three states.
     *    -# Startup
     *       Configuration of the federate.
     *       State begins when registerFederate() is invoked and ends when enterInitializingState() is invoked.
     *    -# Initializing
     *       Configure of the simulation state prior to the start of time stepping.
     *       State begins when enterInitializingState() is invoked and ends when enterExecutingState(true) is
     * invoked.
     *    -# Executing
     *       State begins when enterExecutingState() is invoked and ends when Finalize() is invoked.
     */

    /**
     * Change the federate state to the Initializing state.
     *
     * May only be invoked in Created state otherwise an error is thrown
     */
    virtual void enterInitializingState (federate_id_t federateID) = 0;

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
    enterExecutingState (federate_id_t federateID, helics_iteration_request iterate = NO_ITERATION) = 0;

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
    virtual federate_id_t getFederateId (const std::string &name) = 0;

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
    requestTimeIterative (federate_id_t federateID, Time next, helics_iteration_request iterate) = 0;

    /**
     * Returns the current reiteration count for the specified federate.
     */
    virtual uint64_t getCurrentReiteration (federate_id_t federateID) const = 0;

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

    virtual void setMaximumIterations (federate_id_t federateID, int32_t iterations) = 0;

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
    virtual void setTimeDelta (federate_id_t federateID, Time time) = 0;

    /**
     * Set the outputDelay time for the specified federate.
     *
     * The value is used to determine the interaction amongst various federates as to
     * when a specific federate can influence another
     * @param federateID  the identifier for the federate
     * @param timeoutputDelay
     */
    virtual void setOutputDelay (federate_id_t federateID, Time timeoutputDelay) = 0;
    /**
     * Set the period for a specified federate.
     *
     * The value is used to determine the interaction amongst various federates as to
     * when a specific federate can influence another
     * @param federateID  the identifier for the federate
     * @param timeoutputDelay
     */
    virtual void setPeriod (federate_id_t federateID, Time timePeriod) = 0;
    /**
    * Set the periodic offset for a specified federate.
    *
    * The value is used as a time shift for calculating the allowable time in a federate
    the granted time must one of N*period+offset
    



    * @param federateID  the identifier for the federate
    * @param timeOffset the periodic phase shift
    */
    virtual void setTimeOffset (federate_id_t federateID, Time timeOffset) = 0;
    /**
     * Set the inputDelay time.
     *
     * The value is used to determine the interaction amongst various federates as to
     * when a specific federate can influence another
     * @param federateID  the identifier for the federate
     * @param timeImpact the length of time it take outside message to propagate into a federate
     */
    virtual void setInputDelay (federate_id_t federateID, Time timeImpact) = 0;
    /**
    Set the logging level
    @details set the logging level for an individual federate
    set federateID to 0 for the core logging level
    * @param federateID  the identifier for the federate
    * @param loggingLevel the level of logging to enable
    <0-no logging, 0 -error only, 1- warnings, 2-normal, 3-debug, 4-trace
    */
    virtual void setLoggingLevel (federate_id_t federateID, int loggingLevel) = 0;

    /**
    Set a flag in a a federate
    * @param federateID  the identifier for the federate
    * @param flag an index code for the flag to set
    @param flagValue the value to set the flag to
    */
    virtual void setFlag (federate_id_t federateID, int flag, bool flagValue = true) = 0;
    /**
     * Value interface.
     */

    /**
     * Register a subscription for the specified federate.
     *
     * May only be invoked in the initialize state.
     * @param[in] federateID
     * @param[in] key the name of the subscription
     * @param[in] type a string describing the type of the federate
     * @param[in] units a string naming the units of the federate
     * @param[in] check_mode  if set to required the core will error if the subscription does not have a
     * corresponding publication when converting to init mode
     */
    virtual handle_id_t registerSubscription (federate_id_t federateID,
                                              const std::string &key,
                                              const std::string &(type),
                                              const std::string &units,
                                              handle_check_mode check_mode) = 0;
    /** get a subscription Handle from its key
    @param federateID the identifier for the federate
    @key the tag of the subscription
    @return a handle to identify the subscription*/
    virtual handle_id_t getSubscription (federate_id_t federateID, const std::string &key) const = 0;

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
    virtual handle_id_t registerPublication (federate_id_t federateID,
                                             const std::string &key,
                                             const std::string &type,
                                             const std::string &units) = 0;

    /** get a publication Handle from its key
    @param federateID the identifier for the federate
    @key the name of the publication
     @return a handle to identify the publication*/
    virtual handle_id_t getPublication (federate_id_t federateID, const std::string &key) const = 0;

    /**
     * Returns the name or identifier for a specified handle
     */
    virtual const std::string &getHandleName (handle_id_t handle) const = 0;

    /**
    * Returns the target of a specified handle
    @details for publications and subscriptions this is the key
    for filters this is the target and for endpoints this will return an empty string
    */
    virtual const std::string &getTarget (handle_id_t handle) const = 0;
    /**
     * Returns units for specified handle.
     */
    virtual const std::string &getUnits (handle_id_t handle) const = 0;

    /**
     * Returns type for specified handle.
     @details for endpoints, publications, and filters, this is the input type
     for subscriptions this is the type of the publication(if available)
     @param handle the handle from the publication, subscription, endpoint or filter
     */
    virtual const std::string &getType (handle_id_t handle) const = 0;

    /**
    * Returns output type for specified handle.
    @details for filters this is the outputType, for Subscriptions this is the expected type
    for endpoints and publications this is the same as getType();
    @param handle the handle from the publication, subscription, endpoint or filter
    */
    virtual const std::string &getOutputType (handle_id_t handle) const = 0;

    /**
     * Publish specified data to the specified key.
     *
     * @param handle a handle to a publication to use for the value
     @param[in] data the raw data to send
     @param len the size of the data
     */
    virtual void setValue (handle_id_t handle, const char *data, uint64_t len) = 0;

    /**
     * Return the data for the specified handle.
     *
     */
    virtual std::shared_ptr<const data_block> getValue (handle_id_t handle) = 0;

    /**
     * Returns vector of subscription handles that received an update during the last
     * time request.  The data remains valid until the next call to getValueUpdates for the given federateID
     *@param federateID the identification code of the federate to query
     @return a reference to the location of an array of handles that have been updated
     */
    virtual const std::vector<handle_id_t> &getValueUpdates (federate_id_t federateID) = 0;

    /**
     * Message interface.
     * Designed for point-to-point communication patterns.
     */

    /**
     * Register an endpoint.
     *
     * May only be invoked in the Initialization state.
     */
    virtual handle_id_t
    registerEndpoint (federate_id_t federateID, const std::string &name, const std::string &type) = 0;

    /** get a endpoint Handle from its name or target(this may not be unique so it will only find the first one)
    @param federateID the identifier for the federate
    @param name the name of the endpoint
    @return a handle to identify the endpoint*/
    virtual handle_id_t getEndpoint (federate_id_t federateID, const std::string &name) const = 0;

    /**
     * Register source filter.
     *
     * May only be invoked in the Initialization state.
     @param filterName the name of the filter (may be left blank and one will be automatically assigned)
     @param source the target endpoint for the filter
     @param type_in the input type of the filter
     @param type_out the output type of the filter (may be left blank if the filter doesn't change type)
     this is important for ordering in filters with operators
     @return the handle for the new filter
     */
    virtual handle_id_t registerSourceFilter (const std::string &filterName,
                                              const std::string &source,
                                              const std::string &type_in,
                                              const std::string &type_out) = 0;
    /**
    * Register destination filter.
    @details a destination filter will create an additional processing step of messages before they get to a
    destination endpoint
    *
    * May only be invoked in the Initialization state.
    @param filterName the name of the filter (may be left blank)
    @param dest the target endpoint for the filter
    @param type_in the input type of the filter (may be left blank,  this is for error checking and will produce a
    warning if it doesn't match with the input type of the target endpoint
    @return the handle for the new filter
    */
    virtual handle_id_t registerDestinationFilter (const std::string &filterName,
                                                   const std::string &dest,
                                                   const std::string &type_in,
                                                   const std::string &type_out) = 0;

    /** get a source filter Handle from its name or target(this may not be unique so it will only find the first
    one)
    @param name the name of the filter or its target
    @return a handle to identify the filter*/
    virtual handle_id_t getSourceFilter (const std::string &name) const = 0;

    /** get a destination filter Handle from its name or target(this may not be unique so it will only find the
    first one)
    @param name the name of the filter or its target
    @return a handle to identify the filter*/
    virtual handle_id_t getDestinationFilter (const std::string &name) const = 0;

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
    send (handle_id_t sourceHandle, const std::string &destination, const char *data, uint64_t length) = 0;

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
                            handle_id_t sourceHandle,
                            const std::string &destination,
                            const char *data,
                            uint64_t length) = 0;

    /**
     * Send for filters.
     *
     * Continues sending the message to the next filter or to final destination.
     *
     */
    virtual void sendMessage (handle_id_t sourceHandle, std::unique_ptr<Message> message) = 0;

    /**
     * Returns the number of pending receives for the specified destination endpoint or filter.
     */
    virtual uint64_t receiveCount (handle_id_t destination) = 0;

    /**
     * Returns the next buffered message the specified destination endpoint or filter.
     @details this is a non-blocking call and will return a nullptr if no message are available
     */
    virtual std::unique_ptr<Message> receive (handle_id_t destination) = 0;

    /**
     * Receives a message for any destination.
     @details this is a non-blocking call and will return a nullptr if no messages are available
     @param federateID the identifier for the federate
     @param[out] endpoint_id the endpoint handle related to the message gets stored here
     */
    virtual std::unique_ptr<Message> receiveAny (federate_id_t federateID, handle_id_t &enpoint_id) = 0;

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
    virtual void setFilterOperator (handle_id_t filter, std::shared_ptr<FilterOperator> callback) = 0;

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

// set at a large negative number but not the largest negative number
constexpr Core::federate_id_t invalid_fed_id = -2'000'000'000;
constexpr Core::handle_id_t invalid_handle = -2'000'000'000;
constexpr Core::handle_id_t direct_send_handle =
  -1'745'234;  //!< this special handle can be used to directly send a message in a core

}  // namespace helics

#endif /* _HELICS_CORE_ */
