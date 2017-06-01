/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_CORE_
#define _HELICS_CORE_
#pragma once

/**
 * Open Issues:

 * Initializing state and communication.   There was talk of being able to communicate during the initializing
 state. There is a potential problem since there are no synchronization points so value based communication won't
 complete.

 * Processing updates
    - Loop over all destinations
    - Query for which topics/endpoints have updates, loop over those, i.e. getValueUpdates()
    - Register callback functions, one per topic/endpoint

 * How best to handle DES simulations
    - Timestamped messages
    - What does delay mean?
    - Current time callback?
    - State changes occur during time request sync points?

 *  Object oriented interface or quasi-singleton (i.e. HLA, FMI) ?
    Should C interface for core have an explicit core object in every method.
    Core class is a singleton so that is not strictly necessary; could assume the this pointer from a global.

 * What is time?
      - Units?
      - Data type?
      - Federates with different time units?
      - What can we learn from HLA/FMI?

      SGS - I proposed a simple Time object that has units.  This
       hides the underlying data type and enables units.  If needed in
       future finer time resolution than ns could be added.  I'm
       concerned that if we say "ns" 2 years from now that won't be
       sufficient.  Looking at the history of time in NS-3 and another
       project of mine, time got progressively more complex over
       time.x I think Philip wanted just an int64 on telecon.  Is this
       simple enough to not be a huge pain?

  * Related to time, federate types
      - Time stepped
      - DES
      - Supports reiteration or not
      - Periodic?
      - Continuous
      - Others?

  * Unit translation for value exchanges
      - If core is only byte-based API, translation not possible.
      - Application API could maintain units, cast bytes retrieved from core to appropriate data type, and
 translate?

  * How does data get from one point to another?
      - Does the core own/cache the values (e.g., FNCS) or pass them on (e.g., MPI)?

*/

#include "core-data.h"
#include <utility>

/**
 * HELICS Core API
 */
namespace helics
{
/**
 * The GMLC TD&C core interface.  Abstract class that is
 * implemented for the specific communication systems (e.g. ZMQ and
 * MPI).
 *
 * Multiple federates are allowed.  Due to the collective blocking
 * nature of some calls, like nextTime(), federates need to be in
 * separate threads in order to function correctly.
 *
 * Since this class is intended to be exposed via a C interface,
 * C++ specific features are minimized to enable easier wrapping
 * (e.g. char * instead of string).
 *
 *  For Memory management all message_t and data_t pointers return from the core API should be released via 
 a call to the dereference function,  the core assumes all data given it via send or publish calls could be invalid after
 the call returns
 *
 * Implementations should be thread safe.
 *
 * Note: Methods should all be pure virtual, leaving syntactical sugar off while iterating API design.
 */




class Core
{
  public:
    Core (){};
    virtual ~Core () = default;


    /** class defining some information about the federate*/
    class FederateInfo
    {
      public:
        Time timeDelta = timeZero;  // for periodic federates this is the period or minimum time resolution of a
                               // federate
        Time lookAhead = timeZero;  //!< the lookahead value, the window of time between the time request return and the availability of values
		Time impactWindow = timeZero;  //!< the time it takes values to propagate to the Federate
		//TODO: make into a bitfield with named constants and add setFlags function
        bool observer = false;  //!< flag indicating that the federate is an observer
        bool uninteruptible =
          false;  //!< flag indicating that the federate should never return a time other than requested
		bool time_agnostic = false;  //!< flag indicating that the federate does not participate in time advancement and should be ignored in all timeRequest operations
		bool source_only = false;   //!< flag indicating that the federate does not recieve or do anything with received information.  
		bool filter_only = false; //!< flag indicating that the source filter federate is not modifying the destination of a filtered message only time or content
	};

    /**
     * FederateID uniquely identifies a federate.
     */
	using federate_id_t = unsigned int;

    /**
     * HandleID uniquely identifies a handle.
     */
    using Handle= unsigned int;

    /**
     * Simulator control.
     */

    /**
     * Initialize the core.
     *
     * Should be invoked a single time to initialize the co-simulation core.
     *
     * This is potentially a blocking call that does not complete until all federates
     * have called initialize.
     *
     * Invoked by the CoreFactory, users should call directly.
     */
    virtual void initialize (const char *initializationString) = 0;

    /**
     * Returns true if the core has been initialized.
     */
    virtual bool isInitialized () = 0;

    /**
     * Federate has encountered an unrecoverable error.
     */
    virtual void error (federate_id_t federateID, int errorCode = -1) = 0;

    /**
     * Federate has completed.
     *
     * Should be invoked a single time to complete the simulation.
     *
     * This is potentially a blocking call that does not return until all federates
     * have called finalize.
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
     *       Configure of the simulation state prior to the start of timestepping.
     *       State begins when enterInitializingState() is invoked and ends when enterExecutingState(true) is invoked.
     *    -# Executing
     *       State begins when enterExicutingState() is invoked and ends when Finalize() is invoked.
     */

    /**
     * Change the federate state to the Initializing state.
     *
     * May only be invoked in Created state.
     */
    virtual void enterInitializingState (federate_id_t federateID) = 0;

    /**
     * Change the federate state to the Executing state.
     *
     * May only be invoked in Initializing state.
     @param[in] federateID  the identifier of the federate
     @param[in] iterationCompleted  if true no more iterations on this federate are requested
     if false the federate requests an iterative update
     @return false if the executing state has not been entered and there are updates, true if the simulation is
     ready to move on to the executing state
     */
    virtual bool enterExecutingState (federate_id_t federateID, bool iterationCompleted = true) = 0;

    /**
     * Register a federate.
     *
     * The returned FederateId is local to invoking process,
     * FederateId's should not be used as a global identifier.
     *
     * May only be invoked in initialize state.
     */
    virtual federate_id_t registerFederate (const char *name, const FederateInfo &info) = 0;

    /**
     * Returns the federate name.
     *
     * May only be invoked in Initializing and Executing states
     */
    virtual const char *getFederateName (federate_id_t federateId) = 0;

    /**
     * Returns the federate Id.
     *
     * May only be invoked in Initializing and Executing states
     */
    virtual federate_id_t getFederateId (const char *name) = 0;


    /**
     * Returns the global number of federates that are registered.
     *
     * May only be invoked in Initializing and Executing States.
     */
    virtual unsigned int getFederationSize () = 0;

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
     * \param next
     */
    virtual Time timeRequest (federate_id_t federateId, Time next) = 0;

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
     * If a federate determines it cannot converge it should invoke the die() method.
     *
     * Federates only participate it in reiterations for times that
     * are evenly divisible by the federates time delta.
     *
     * May only be invoked in Executing state.
     *
     * Non-reiterative federates may not invoke this method.
     *
     * \param next
     * \param localConverged has the local federate converged
     */
    // SGS TODO make this not a pair for C API?
    virtual std::pair<Time, bool>
    requestTimeIterative (federate_id_t federateId, Time next, bool localConverged) = 0;

    /**
     * Returns the current reiteration count for the specified federate.
     */
    virtual uint64_t getCurrentReiteration (federate_id_t federateId) = 0;

    /**
     * Set the maximum number of iterations allowed.
     *
     * The minimum value set in any federate is used.
     *
     * Default value is the maximum allowed value for uint64_t.
     *
     * May only be invoked in the initialize state.
     */
    virtual void setMaximumIterations (federate_id_t federateId, uint64_t iterations) = 0;

    /**
     * Set the minimum time resolution for the specified federate.
     *
     * The value is used to constrain when the timeRequest methods
     * return to values that are multiples of the specified delta.
     * This is useful for federates that are time-stepped and making
     * sub-time-step updates is not meaningful.
     *
     * \param time
     */
    virtual void setTimeDelta (federate_id_t federateId, Time time) = 0;

    /**
     * Set the lookahead time for the specified federate.
     *
     * The value is used to determine the interaction amongs various federates as to
     * when a specific federate can influence another
     * \param federateId  the identifier for the federate
     * \param timeLookAhead
     */
    virtual void setLookAhead (federate_id_t federateId, Time timeLookAhead) = 0;
	/**
	* Set the ImpactWindow time.
	*
	* The value is used to determine the interaction amongs various federates as to
	* when a specific federate can influence another
	* \param federateId  the identifier for the federate
	* \param timeImpact
	*/
	virtual void setImpactWindow(federate_id_t federateId, Time timeImpact) = 0;

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
     * @param[in] required  if set to true the core will error if the subscription does not have a corresponding
     * publication when converting to init mode
     */
    virtual Handle registerSubscription (federate_id_t federateId,
                                         const char *key,
                                         const char *type,
                                         const char *units,
                                         bool required) = 0;

    virtual Handle getSubscription (federate_id_t federateId, const char *key) = 0;

    /**
     * Register a publication.
     *
     * May only be invoked in the initialize state.
     */
    virtual Handle
    registerPublication (federate_id_t federateId, const char *key, const char *type, const char *units) = 0;

    virtual Handle getPublication (federate_id_t federateId, const char *key) = 0;

    /**
     * Returns units for specified handle.
     */
    virtual const char *getUnits (Handle handle) = 0;

    /**
     * Returns type for specified handle.
     */
    virtual const char *getType (Handle handle) = 0;

    /**
     * Publish specified data to the specified key.
     *
     * Data ownership is retained by the federate.
     */
    virtual void setValue (Handle handle, const char *data, uint64_t len) = 0;

    /**
     * Return data for the specified handle.
     *
     * Returned pointer is valid until dereference() is invoked.
     */
    virtual data_t *getValue (Handle handle) = 0;

    /**
     * Completed referencing the data.
     *
     * It is invalid to access data after this call.  Core is free to delete the data.
     */
    virtual void dereference (data_t *data) = 0;

    /**
     * Completed referencing the data.
     *
     * It is invalid to access data after this call.  Core is free to delete the data.
     */
    virtual void dereference (message_t *msg) = 0;

    /**
     * Returns array of subscription handles that received an update during the last
     * time request.
     *
     * /param size set to the size of the array.
     */
    virtual const Handle *getValueUpdates (federate_id_t federateId, uint64_t *size) = 0;

    /**
     * Message interface.
     * Designed for point-to-point communication patterns.
     */

    /**
     * Register an endpoint.
     *
     * May only be invoked in the Initialization state.
     */
    virtual Handle registerEndpoint (federate_id_t federateId, const char *name, const char *type) = 0;

    /**
     * Register source filter.
     *
     * May only be invoked in the Initialization state.
     */
    virtual Handle registerSourceFilter (federate_id_t federateId,
                                         const char *filterName,
                                         const char *source,
                                         const char *type_in) = 0;
	/**
	* Register destination filter.
	*
	* May only be invoked in the Initialization state.
	*/
    virtual Handle registerDestinationFilter (federate_id_t federateId,
                                              const char *filterName,
                                              const char *dest,
                                              const char *type_in) = 0;
	/**
	* add a time dependency between federates
	* @details this function is primarily useful for Message federates which do not otherwise restrict the dependencies
	* adding a dependency gives additional information to the core that the specifed federate(given by id) will be sending Messages to
	the named Federate(by federateName)
	@param[in] federateId  the identifier for the federate
	@param[in] federateName the name of the dependent federate
	*/
	virtual void addDependency(federate_id_t federateId, const char *federateName)=0;
    /**
     * Register known frequently communicating source/destination end points.
     *
     * May be used for error checking for compatible types and possible optimization by
     * pre-registering the intent for these endpoints to communicate.
     */
    virtual void registerFrequentCommunicationsPair (const char *source, const char *dest) = 0;

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
    virtual void send (Handle sourceHandle, const char *destination, const char *data, uint64_t length) = 0;

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
     */
    virtual void
    sendEvent (Time time, Handle sourceHandle, const char *destination, const char *data, uint64_t length) = 0;

    /**
     * Send for filters.
     *
     * Continues sending the message to the next filter or to final destination.
     *
     */
    virtual void sendMessage (message_t *message) = 0;

    /**
     * Returns the number of pending receives for the specified destination endpoint.
     */
    virtual uint64_t receiveCount (Handle destination) = 0;

    /**
     * Returns the next buffered message the specified destination endpoint.
     */
    virtual message_t *receive (Handle destination) = 0;

    /**
     * Receives a message for any destination.
     */
    virtual std::pair<const Handle, message_t*> receiveAny (federate_id_t federateId) = 0;

    /**
     * Returns number of messages for all destinations.
     */
    virtual uint64_t receiveCountAny (federate_id_t federateId) = 0;

    /** send a log message to the Core for logging
    @param[in] federateId the federate that is sending the log message
    @param[in] logCode  an integer based logging code
    @param[in] logMessage the message to log
    */
    virtual void logMessage (federate_id_t federateId, int logCode, const char *logMessage) = 0;

	/** set the filter callback *  setting a filter callback implies that the filter has no time or order dependency
	and the filter is an independent function
	@details the lifetime of the FilterOperator is managed by the user and should remain alive during the entire run of the simulation
	@param[in] filter  the handle of the filter
	@param[in] callback the function to operate on the message
	*/
	virtual void setFilterOperator(Handle filter, FilterOperator* callback) = 0;

	/**
	* Returns number of messages for all filters.
	*/
	virtual uint64_t receiveFilterCount(federate_id_t federateID) = 0;

	/**
	* Receives a message for any filter.
	*/
	virtual std::pair<const Handle, message_t*> receiveAnyFilter(federate_id_t federateID) = 0;
};

}  // namespace helics

#endif /* _HELICS_CORE_ */
