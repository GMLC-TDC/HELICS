/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef _HELICS_FEDERATE_API_
#define _HELICS_FEDERATE_API_
#pragma once

#include "helics/helics-config.h"
#include "../core/helics-time.h"
#include "helics_includes/string_view.h"

#include "helicsTypes.hpp"

#include "../core/coreFederateInfo.h"
#include "../flag-definitions.h"
#include <atomic>
#include <string>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>


/**
 * HELICS Application API
 */
namespace helics
{
class Core;
class asyncFedCallInfo;
class MessageOperator;
/** data class defining federate properties and information
 */
class FederateInfo: public CoreFederateInfo
{
  public:
	std::string name;  //!< federate name
	
	bool rollback = false;  //!< indicator that the federate has rollback features
	bool forwardCompute = false;  //!< indicator that the federate does computation ahead of the timing call[must
                                  //! support rollback at least in a limited sense if set to true]
	core_type coreType;  //!< the type of the core
	std::string coreName;  //!< the name of the core
	std::string coreInitString;  //!< an initialization string for the core API object

	/** default constructor*/
	FederateInfo () = default;
	/** construct from the federate name*/
	FederateInfo (std::string fedname) : name (fedname){};
	/** construct from the name and type*/
	FederateInfo (std::string fedname, core_type cType)
		: name (std::move (fedname)), coreType (cType){};
    /** load a federateInfo object from command line arguments
    @param argc the number of arguments
    @param argv an array of char * pointers to the arguments
    */
    FederateInfo(int argc, const char * const *argv);
    /** load a federateInfo object from command line arguments
    @param argc the number of arguments
    @param argv an array of char * pointers to the arguments
    */
    void loadInfoFromArgs(int argc, const char * const *argv);
};

/** generate a FederateInfo object from a json file
*/
FederateInfo LoadFederateInfo(const std::string &jsonString);

/** get a string with the helics version info*/
std::string getHelicsVersionString();

/** get the major version number*/
int getHelicsVersionMajor();
/** get the minor version number*/
int getHelicsVersionMinor();
/** get the patch version number*/
int getHelicsVersionPatch();

class Core;

/** base class for a federate in the application API
 */
class Federate
{
  public:
	/** the allowable states of the federate*/
	enum class op_states : char
	{
		startup = 0,  //!< when created the federate is in startup state
		initialization,  //!< entered after the enterInitializationState call has returned
		execution,  //!< entered after the enterExectuationState call has returned
		finalize,  //!< the federate has finished executing normally final values may be retrieved
		error,  //!< error state no core communication is possible but values can be retrieved
		// the following states are for asynchronous operations
		pending_init,  //!< indicator that the federate is pending entry to initialization state
		pending_exec,  //!< state pending EnterExecution State
		pending_time,  //!< state that the federate is pending a timeRequest
		pending_iterative_time,  //!< state that the federate is pending an iterative time request
	};

  protected:
	std::atomic<op_states> state{op_states::startup};  //!< the current state of the simulation
	char separator_ = '/';  //!< the separator between automatically prependend names
  private:
	int32_t fedID = -2'000'000'000;

  protected:
	std::shared_ptr<Core> coreObject;  //!< reference to the core simulation API
	Time currentTime;  //!< the current simulation time
	FederateInfo FedInfo;  //!< the information structure that contains the data on the federate
  private:
	std::unique_ptr<asyncFedCallInfo> asyncCallInfo;  //!< pointer to a class defining the async call information
  public:
	/**constructor taking a federate information structure
	@param[in] fi  a federate information structure
	*/
	Federate (const FederateInfo &fi);
	/**constructor taking a core and a federate information structure
	@param[in] fi  a federate information structure
	*/
	Federate (std::shared_ptr<Core> core, const FederateInfo &fi);
	/**constructor taking a file with the required information
	@param[in] jsonString can be either a JSON file or a string containing JSON code
	*/
	Federate (const std::string &jsonString);

	Federate () noexcept;
	Federate (Federate &&fed) noexcept;
	Federate (const Federate &fed) = delete;
	/** virtual destructor function */
	virtual ~Federate ();
	/** default move assignment*/
	Federate &operator= (Federate &&fed) noexcept;
	/** delete copy assignment*/
	Federate &operator= (const Federate &fed) = delete;
	/** enter the initialization mode after all interfaces have been defined
	@details  the call will block until all federates have entered initialization mode
	*/
	void enterInitializationState ();

	/** enter the initialization mode after all interfaces have been defined
	@details  the call will not block
	*/
	void enterInitializationStateAsync ();
	/** called after one of the async calls and will indicate true if an async operation has completed
	@details only call from the same thread as the one that called the initial async call and will return false
	if called when no aysnc operation is in flight*/
	bool isAsyncOperationCompleted () const;
	/** second part of the async process for entering initializationState call after a call to
	enterInitializationStateAsync if call any other time it will throw an invalidFunctionCall exception*/
	void enterInitializationStateComplete ();
	/** enter the normal execution mode
	@details call will block until all federates have entered this mode
	*/
	iteration_result enterExecutionState (iteration_request iterate = iteration_request::no_iterations);
	/** enter the normal execution mode
	@details call will block until all federates have entered this mode
	*/
	void enterExecutionStateAsync (iteration_request iterate = iteration_request::no_iterations);
	/** complete the async call for entering Execution state
	@details call will not block but will return quickly.  The enterInitializationStateFinalize must be called
	before doing other operations
	*/
	iteration_result enterExecutionStateComplete ();
	/** terminate the simulation
	@details call is normally non-blocking, but may block if called in the midst of an
	asynchronous call sequence, no core calling commands may be called after completion of this function */
	void finalize ();

	/** disconnect a simulation from the core (will also call finalize before disconnecting if necessary)*/
	virtual void disconnect(); 
	/** specify the simulator had an unrecoverable error
	 */
	void error (int errorcode);
	/** specify the simulator had an error with error code and message
	 */
	void error (int errorcode, const std::string &message);

	/** specify a separator to use for naming separation between the federate name and the interface name
	@example setSeparator('.') will result in future registrations of local endpoints such as fedName.endpoint
	setSeparator('/') will result in fedName/endpoint
	the default is '/'  any character can be used though many will not make that much sense.  This call is not thread safe.
	 */
	void setSeparator (char separator) { separator_ = separator; }
	/** request a time advancement
	@param[in] the next requested time step
	@return the granted time step*/
	Time requestTime (Time nextInternalTimeStep);

	/** request a time advancement
	@param[in] the next requested time step
	@return the granted time step*/
	iterationTime requestTimeIterative (Time nextInternalTimeStep, iteration_request iterate);

	/** request a time advancement
	@param[in] the next requested time step
	*/
	void requestTimeAsync (Time nextInternalTimeStep);

	/** request a time advancement
	@param[in] the next requested time step
	@param iterate a requested iteration level (none, require, optional)
	@return the granted time step*/
	void requestTimeIterativeAsync (Time nextInternalTimeStep, iteration_request iterate);

	/** request a time advancement
	@param[in] the next requested time step
	@return the granted time step*/
	Time requestTimeComplete ();

	/** finalize the time advancement request
	@return the granted time step in an iterationTime structure which contains a time and iteration result*/
	iterationTime requestTimeIterativeComplete ();

	/** set the minimum time delta for the federate
	@param[in] tdelta the minimum time delta to return from a time request function
	*/
	void setTimeDelta (Time tdelta);
    /** set the look ahead time or output delay
	@details the look ahead is the propagation time for messages/event to propagate from the Federate
    to the outside federation
    @param[in] outputDelay the value of the time delay (must be >=0)
    @throws invalid_value when using a time <0
	*/
    void setOutputDelay (Time outputDelay);

	/** set the impact Window time
	@details the impact window is the time window around the time request in which other federates cannot affect
	the federate
    @param[in] inputDelay the look ahead time
    @throws invalid_value when using a time <0
	*/
    void setInputDelay (Time inputDelay);
	/** set the period and offset of the federate
	@details the federate will on grant time on N*period+offset interval
	@param[in] period the length of time between each subsequent grants
	@param[in] offset the shift of the period from 0  offset must be < period
	*/
	void setPeriod (Time period, Time offset = timeZero);
	/** set a flag for the federate
	@param[in] flag an index into the flag /ref flag-definitions.h
	@param[in] flagvalue the value of the flag defaults to true
	*/
	virtual void setFlag(int flag, bool flagValue = true);
	/**  set the logging level for the federate
	@ details debug and trace only do anything if they were enabled in the compilation
	@param loggingLevel (-1: none, 0: error_only, 1: warnings, 2: normal, 3: debug, 4: trace)
	*/
	void setLoggingLevel (int loggingLevel);

	/** make a query of the core
	@details this call is blocking until the value is returned which make take some time depending on the size of
	the federation and the specific string being queried
	@param target  the target of the query can be "federation", "federate", "broker", "core", or a specific name of
	a federate, core, or broker
	@param queryStr a string with the query see other documentation for specific properties to query, can be
	defined by the federate
	@return a string with the value requested.  this is either going to be a vector of strings value or a json
	string stored in the first element of the vector.  The string "#invalid" is returned if the query was not valid
	*/
	std::string query (const std::string &target, const std::string &queryStr);

	/** make a query of the core
	@details this call is blocking until the value is returned which make take some time depending on the size of
	the federation and the specific string being queried
	@param queryStr a string with the query see other documentation for specific properties to query, can be
	defined by the federate if the local federate does not recognize the query it sends it on to the federation
	@return a string with the value requested.  this is either going to be a vector of strings value or a json
	string stored in the first element of the vector.  The string "#invalid" is returned if the query was not valid
	*/
	std::string query (const std::string &queryStr);

	/** make a query of the core in an async fashion
	@details this call is blocking until the value is returned which make take some time depending on the size of
	the federation and the specific string being queried
	@param target  the target of the query can be "federation", "federate", "broker", "core", or a specific name of
	a federate, core, or broker
	@param queryStr a string with the query see other documentation for specific properties to query, can be
	defined by the federate
    @return a query_id_t to use for returning the result
	*/
    query_id_t queryAsync (const std::string &target, const std::string &queryStr);

	/** make a query of the core in an async fashion
	@details this call is blocking until the value is returned which make take some time depending on the size of
	the federation and the specific string being queried
	@param queryStr a string with the query see other documentation for specific properties to query, can be
	defined by the federate
    @return a query_id_t used to get the results of the query in the future
	*/
    query_id_t queryAsync (const std::string &queryStr);

	/** get the results of an async query
	@details the call will block until the results are returned inquiry of queryCompleted() to check if the results
	have been returned or not yet

	@param queryIndex the int value returned from the queryAsync call
   @return a string with the value requested.  the format of the string will be either a single string a string vector like "[string1; string2]" or json The string "#invalid" is returned if the query was not valid
	*/
    std::string queryComplete (query_id_t queryIndex);

	/** check if an asynchronous query call has been completed
	@return true if the results are ready for @queryFinalize
	*/
    bool isQueryCompleted (query_id_t queryIndex) const;

	/** define a filter interface on a source
	@details a source filter will be sent any packets that come from a particular source
	if multiple filters are defined on the same source, they will be placed in some order defined by the core
	@param[in] the name of the endpoint
	@param[in] the inputType which the source filter can receive
	*/
	filter_id_t registerSourceFilter(const std::string &filterName,
		const std::string &sourceEndpoint,
		const std::string &inputType = "",
		const std::string &outputType = "");
	/** define a filter interface for a destination
	@details a destination filter will be sent any packets that are going to a particular destination
	multiple filters are not allowed to specify the same destination
	@param[in] the name of the destination endpoint
	@param[in] the inputType which the destination filter can receive
	*/
	filter_id_t registerDestinationFilter(const std::string &filterName,
		const std::string &destEndpoint,
		const std::string &inputType = "",
		const std::string &outputType = "");
	/** define a filter interface on a source
	@details a source filter will be sent any packets that come from a particular source
	if multiple filters are defined on the same source, they will be placed in some order defined by the core
	@param[in] the name of the endpoint
	@param[in] the inputType which the source filter can receive
	*/
	filter_id_t registerSourceFilter(const std::string &sourceEndpoint)
	{
		return registerSourceFilter("", sourceEndpoint, "", "");
	}
	/** define a filter interface for a destination
	@details a destination filter will be sent any packets that are going to a particular destination
	multiple filters are not allowed to specify the same destination
	@param[in] the name of the destination endpoint
	@param[in] the inputType which the destination filter can receive
	*/
	filter_id_t registerDestinationFilter(const std::string &destEndpoint)
	{
		return registerDestinationFilter("", destEndpoint, "", "");
	}
	/** get the name of a filter
	@param[in] id the filter to query
	@return empty string if an invalid id is passed*/
	std::string getFilterName(filter_id_t id) const;

	/** get the name of the endpoint that a filter is associated with
	@param[in] id the filter to query
	@return empty string if an invalid id is passed*/
	std::string getFilterEndpoint(filter_id_t id) const;

	/** get the input type of a filter from its id
	@param[in] id the endpoint to query
	@return empty string if an invalid id is passed*/
	std::string getFilterInputType(filter_id_t id) const;

	/** get the output type of a filter from its id
	@param[in] id the endpoint to query
	@return empty string if an invalid id is passed*/
	std::string getFilterOutputType(filter_id_t id) const;
	/** get the id of a source filter from the name of the endpoint
	@param[in] filterName the name of the filter
	@return invalid_filter_id if name is not recognized otherwise returns the filter id*/
	filter_id_t getFilterId(const std::string &filterName) const;
	/** get the id of a source filter from the name of the filter
	@param[in] filterName the publication id
	@return invalid_filter_id if name is not recognized otherwise returns the filter id*/
	filter_id_t getSourceFilterId(const std::string &filterName) const;

	/** get the id of a destination filter from the name of the endpoint
	@param[in] filterName the publication id
	@return invalid_filter_id if name is not recognized otherwise returns the filter id*/
	filter_id_t getDestFilterId(const std::string &filterName) const;

	/** @brief register a operator for the specified filter
	@details for time_agnostic federates only,  all other settings would trigger an error
	The MessageOperator gets called when there is a message to filter, There is no order or state to this
	messages can come in any order.
	@param[in] filter the identifier for the filter to trigger
	@param[in] op A shared_ptr to a message operator
	*/
	void setFilterOperator(filter_id_t filter, std::shared_ptr<FilterOperator> op);
	/** @brief register a operator for the specified filters
	@details for time_agnostic federates only,  all other settings would trigger an error
	The MessageOperator gets called when there is a message to filter, There is no order or state to this
	message can come in any order.
	@param[in] filters the identifier for the filter to trigger
	@param[in] op A shared_ptr to a message operator
	*/
	void setFilterOperator(const std::vector<filter_id_t> &filters, std::shared_ptr<FilterOperator> op);
  protected:
	/** function to deal with any operations that need to occur on a time update*/
	virtual void updateTime (Time newTime, Time oldTime);
	/** function to deal with any operations that need to occur on the transition from startup to initialize*/
	virtual void StartupToInitializeStateTransition ();
	/** function to deal with any operations that need to occur on the transition from startup to initialize*/
	virtual void InitializeToExecuteStateTransition ();

  public:
	/** register a set of interfaces defined in a file
	@details call is only valid in startup mode
	@param[in] jsonString  the location of the file or json String to load to generate the interfaces
	*/
	virtual void registerInterfaces (const std::string &jsonString);
	/** get the underlying federateID for the core*/
	unsigned int getID () const noexcept { return fedID; }
	/** get the current state of the federate*/
	op_states getCurrentState () const { return state; }
	/** get the current Time
	@details the most recent granted time of the federate*/
	Time getCurrentTime () const { return currentTime; }
	/** get the federate name*/
	const std::string &getName () const { return FedInfo.name; }
	/** get a pointer to the core object used by the federate*/
	std::shared_ptr<Core> getCorePointer () { return coreObject; }
};

/** defining an exception class for state transition errors*/
class InvalidStateTransition : public std::runtime_error
{
  public:
	InvalidStateTransition (const char *s) noexcept : std::runtime_error (s) {}
};

/** defining an exception class for invalid function calls*/
class InvalidFunctionCall : public std::runtime_error
{
  public:
	InvalidFunctionCall (const char *s) noexcept : std::runtime_error (s) {}
};
/** defining an exception class for invalid parameter values*/
class InvalidParameterValue : public std::runtime_error
{
  public:
	InvalidParameterValue (const char *s) noexcept : std::runtime_error (s) {}
};

/** function to do some housekeeping work
@details this runs some cleanup routines and tries to close out any residual thread that haven't been shutdown
yet*/
void cleanupHelicsLibrary();
} //namespace helics
#endif
