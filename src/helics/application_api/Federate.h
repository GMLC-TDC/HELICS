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

#include "helics/config.h"
#include "helics/core/helics-time.h"
#include "helics_includes/string_view.h"

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
/** data class defining federate properties and information
 */
class FederateInfo
{
  public:
    std::string name;  //!< federate name
    bool observer =
      false;  //!< indicator that the federate is an observer and doesn't participate in time advancement
    bool rollback = false;  //!< indicator that the federate has rollback features
    bool timeAgnostic = false;  //!< indicator that the federate doesn't use time
    bool forwardCompute = false;  //!< indicator that the federate does computation ahead of the timing call[must
                                  //! support rollback if set to true]
    bool uninterruptible =
      false;  //!< indicator that the time request cannot return something other than the requested time
    bool sourceOnly = false;  //!< indicator that the federate is a source only
    int32_t max_iterations = 10;  //!< the maximum number of iteration cycles a federate should execute
    int32_t logLevel =
      1;  //!< the logging level for the federate (-1: none, 0: error, 1:warning,2:normal,3:debug,4:trace)
    core_type coreType;  //!< the type of the core
    std::string coreName;  //!< the name of the core
    Time timeDelta = timeZero;  //!< the minimum time between granted time requests
    Time lookAhead = timeZero;  //!< the lookahead value
    Time impactWindow = timeZero;  //!< the impact window
    Time period = timeZero;  //!< the periodicity of the Federate granted time can only come on integer multipliers
                             //!< of the period
    Time offset = timeZero;  //!< the offset to the time period
    std::string coreInitString;  //!< an initialization string for the core API object

    /** default constructor*/
    FederateInfo () = default;
    /** construct from the federate name*/
    FederateInfo (std::string fedname) : name (fedname){};
    /** construct from the name and type*/
    FederateInfo (std::string fedname, core_type cType)
        : name (std::move (fedname)), coreType (cType){};
};

std::string getHelicsVersionString();

int getHelicsVersionMajor();
int getHelicsVersionMinor();
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
        pendingInit,  //!< indicator that the federate is pending entry to initialization state
        pendingExec,  //!< state pending EnterExecution State
        pendingTime,  //!< state that the federate is pending a timeRequest
        pendingIterativeTime,  //!< state that the federate is pending an iterative time request
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
    /**constructor taking a core and a federate information structure, sore information in fi is ignored
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
    bool asyncOperationCompleted () const;
    /** second part of the async process for entering initializationState call after a call to
    enterInitializationStateAsync if call any other time it will throw an invalidFunctionCall exception*/
    void enterInitializationStateFinalize ();
    /** enter the normal execution mode
    @details call will block until all federates have entered this mode
    */
    convergence_state enterExecutionState (convergence_state ProcessComplete = convergence_state::complete);
    /** enter the normal execution mode
    @details call will block until all federates have entered this mode
    */
    void enterExecutionStateAsync (convergence_state ProcessComplete = convergence_state::complete);
    /** finalize the async call for entering Execution state
    @details call will not block but will return quickly.  The enterInitializationStateFinalize must be called
    before doing other operations
    */
    convergence_state enterExecutionStateFinalize ();
    /** terminate the simulation
    @details call is normally non-blocking, but may block if called in the midst of an
    asynchronous call sequence, not core calling commands may be called */
    void finalize ();

    /** disconnect a simulation from the core */
    virtual void disconnect(); 
    /** specify the simulator had an unrecoverable error
     */
    void error (int errorcode);
    /** specify the simulator had an error with error code and message
     */
    void error (int errorcode, const std::string &message);

    /** specify a separator to use for naming separation
     */
    void setSeparator (char separator) { separator_ = separator; }
    /** request a time advancement
    @param[in] the next requested time step
    @return the granted time step*/
    Time requestTime (Time nextInternalTimeStep);

    /** request a time advancement
    @param[in] the next requested time step
    @return the granted time step*/
    iterationTime requestTimeIterative (Time nextInternalTimeStep, convergence_state iterationComplete);

    /** request a time advancement
    @param[in] the next requested time step
    */
    void requestTimeAsync (Time nextInternalTimeStep);

    /** request a time advancement
    @param[in] the next requested time step
    @return the granted time step*/
    void requestTimeIterativeAsync (Time nextInternalTimeStep, convergence_state iterationComplete);

    /** request a time advancement
    @param[in] the next requested time step
    @return the granted time step*/
    Time requestTimeFinalize ();

    /** finalize the time advancement request
    @return the granted time step*/
    iterationTime requestTimeIterativeFinalize ();

    /** set the mimimum time delta for the federate
    @param[in] tdelta the minimum time delta to return from a time request function
    */
    void setTimeDelta (Time tdelta);
    /** set the look ahead time
    @details the look ahead is the propagation time for messages/event to propagate from the Federate
    the federate
    @param[in] lookAhead the look ahead time
    */
    void setLookAhead (Time lookAhead);

    /** set the impact Window time
    @details the impact window is the time window around the time request in which other federates cannot affect
    the federate
    @param[in] lookAhead the look ahead time
    */
    void setImpactWindow (Time window);
    /** set the period and offset of the federate
    @details the federate will on grant time on N*period+offset interval
    @param[in] period the length of time between each subsequent grants
    @param[in] offset the shift of the period from 0  offset must be < period
    */
    void setPeriod (Time period, Time offset = timeZero);
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
    @return an integer used to get the results of the query in the future
    */
    int queryAsync (const std::string &target, const std::string &queryStr);

    /** make a query of the core in an async fashion
    @details this call is blocking until the value is returned which make take some time depending on the size of
    the federation and the specific string being queried
    @param queryStr a string with the query see other documentation for specific properties to query, can be
    defined by the federate
    @return an integer used to get the results of the query in the future
    */
    int queryAsync (const std::string &queryStr);

    /** get the results of an async query
    @details the call will block until the results are returned inquiry of queryCompleted() to check if the results
    have been returned or not yet

    @param queryIndex the int value returned from the queryAsync call
   @return a string with the value requested.  the format of the string will be either a single string a string vector like "[string1; string2]" or json The string "#invalid" is returned if the query was not valid
    */
    std::string queryFinalize (int queryIndex);

    /** check if an asynchronous query call has been completed
    @return true if the results are ready for @queryFinalize
    */
    bool queryCompleted (int queryIndex) const;

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
    op_states currentState () const { return state; }
    /** get the current Time
    @details the most recent granted time of the federate*/
    Time getCurrentTime () const { return currentTime; }
    /** get the federate name*/
    const std::string &getName () const { return FedInfo.name; }
    /** get a pointer to the core object used by the federate*/
    std::shared_ptr<Core> getCorePointer () { return coreObject; }
};
/** generate a FederateInfo object from a json file
 */
FederateInfo LoadFederateInfo (const std::string &jsonString);

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
} //namespace helics
#endif
