/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "../core/LocalFederateId.hpp"
#include "../core/helicsTime.hpp"
#include "../helics_enums.h"
#include "FederateInfo.hpp"
#include "helics/helics-config.h"
#include "helicsTypes.hpp"
#include "helics_cxx_export.h"

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace gmlc::libguarded {
template<class T, class M>
class shared_guarded;
}  // namespace gmlc::libguarded

/**
 * HELICS Application API
 */
namespace helics {
class Core;
class CoreApp;
class AsyncFedCallInfo;
class MessageOperator;
class ConnectorFederateManager;
class PotentialInterfacesManager;
class Filter;
class Translator;
class CloningFilter;
class Federate;
enum class FilterTypes;

/** base class for a federate in the application API
 */
class HELICS_CXX_EXPORT Federate {
  public:
    /** the allowable operation modes of the federate*/
    enum class Modes : char {
        /** when created the federate is in startup state */
        STARTUP = 0,
        /** entered after the enterInitializingMode call has returned */
        INITIALIZING = 1,
        /** entered after the enterExectuationState call has returned */
        EXECUTING = 2,
        /** the federate has finished executing normally final values may be retrieved */
        FINALIZE = 3,
        /** error state no core communication is possible but values can be retrieved */
        ERROR_STATE = 4,
        // the following states are for asynchronous operations
        /** indicator that the federate is pending entry to initialization mode */
        PENDING_INIT = 5,
        /** state pending EnterExecution State */
        PENDING_EXEC = 6,
        /** state that the federate is pending a timeRequest */
        PENDING_TIME = 7,
        /** state that the federate is pending an iterative time request */
        PENDING_ITERATIVE_TIME = 8,
        /** state that the federate is pending a finalize call */
        PENDING_FINALIZE = 9,
        /** the simulation has finished normally but everything is still connected */
        FINISHED = 10,

        /** the simulation is pending an iterative call to initializing mode */
        PENDING_ITERATIVE_INIT = 12,
    };

  protected:
    std::atomic<Modes> currentMode{Modes::STARTUP};  //!< the current state of the simulation
    char nameSegmentSeparator = '/';  //!< the separator between automatically prependend names
    /** set to false to allow some invalid configurations to be ignored instead of error */
    bool strictConfigChecking{true};
    /** set to true to force all outgoing data to json serialization*/
    bool useJsonSerialization{false};
    /** set to true for observer mode, no outgoing synchronized communications*/
    bool observerMode{false};
    /** allow to retrigger time requests from callbacks (user specified)*/
    bool retriggerTimeRequest{false};
    /*** specify that the federate will only be used on a single thread*/
    bool singleThreadFederate{false};
    /*** specify that the federate has potential interfaces*/
    bool hasPotentialInterfaces{false};

  private:
    LocalFederateId fedID;  //!< the federate ID of the object for use in the core
  protected:
    std::shared_ptr<Core> coreObject;  //!< reference to the core simulation API
    Time mCurrentTime = Time::minVal();  //!< the current simulation time
    Time mStopTime = Time::maxVal();  //!< the stopping time for the federate
    std::string configFile;  //!< any config file used
  private:
    /// pointer to a class defining the async call information
    std::unique_ptr<gmlc::libguarded::shared_guarded<AsyncFedCallInfo, std::mutex>> asyncCallInfo;
    std::unique_ptr<ConnectorFederateManager> cManager;  //!< class for managing filter operations
    /// class for managing potential interfaces
    std::unique_ptr<PotentialInterfacesManager> potManager;
    std::atomic<int> potInterfacesSequence{0};
    std::string mName;  //!< the name of the federate
    std::function<void(Time, Time, bool)> timeRequestEntryCallback;
    std::function<void(Time, bool)> timeUpdateCallback;
    std::function<void(Modes, Modes)> modeUpdateCallback;
    std::function<void(Time, bool)> timeRequestReturnCallback;
    std::function<void(bool)> initializingEntryCallback;
    std::function<void()> executingEntryCallback;
    std::function<void()> cosimulationTerminationCallback;
    std::function<void(int, std::string_view)> errorHandlerCallback;

  public:
    /**constructor taking a federate information structure
    @param fedname the name of the federate can be empty to use a name from the federateInfo
    @param fedInfo  a federate information structure
    */
    Federate(std::string_view fedname, const FederateInfo& fedInfo);
    /**constructor taking a core and a federate information structure
    @param fedname the name of the federate can be empty to use a name from the federateInfo
    @param core a shared pointer to a core object, the pointer will be copied
    @param fedInfo  a federate information structure
    */
    Federate(std::string_view fedname,
             const std::shared_ptr<Core>& core,
             const FederateInfo& fedInfo = FederateInfo{});

    /**constructor taking a CoreApp and a federate information structure
    @param fedname the name of the federate can be empty to use a name from the federateInfo
    @param core a CoreApp with the core core connect to.
    @param fedInfo  a federate information structure
    */
    Federate(std::string_view fedname, CoreApp& core, const FederateInfo& fedInfo = FederateInfo{});
    /**constructor taking a file with the required information
    @param configString can be either a JSON file or a string containing JSON code or a TOML file
    */
    explicit Federate(const std::string& configString);
    /**constructor taking a file with the required information and the name of the federate
    @param fedname the name of the federate
    @param configString can be either a JSON file or a string containing JSON code or a TOML file
    with extension (.toml, .TOML)
    */
    Federate(std::string_view fedname, const std::string& configString);
    /**default constructor*/
    Federate() noexcept;
    /** move constructor*/
    Federate(Federate&& fed) noexcept;
    /** deleted copy constructor*/
    Federate(const Federate& fed) = delete;
    /** virtual destructor function */
    virtual ~Federate();
    /** default move assignment*/
    Federate& operator=(Federate&& fed) noexcept;
    /** delete copy assignment*/
    Federate& operator=(const Federate& fed) = delete;
    /** enter the initialization mode after all interfaces have been defined
    @details  the call will block until all federates have entered initialization mode
    */
    void enterInitializingMode();

    /** enter the initialization mode after all interfaces have been defined
    @details  the call will not block but a call to \ref enterInitializingModeComplete should be
    made to complete the call sequence
    */
    void enterInitializingModeAsync();
    /** called after one of the async calls and will indicate true if an async operation has
    completed
    @details only call from the same thread as the one that called the initial async call and will
    return false if called when no async operation is in flight*/
    bool isAsyncOperationCompleted() const;
    /** second part of the async process for entering initializationState call after a call to
    enterInitializingModeAsync if call any other time it will throw an InvalidFunctionCall
    exception*/
    void enterInitializingModeComplete();

    /** iterate in the created mode.
    @details  the call will block until all federates have flagged they are ready for the next stage
    of initialization all federates requesting iterations on the created mode will be notified they
    can continue with setup.
    */
    void enterInitializingModeIterative();

    /** iterate in the created mode.
    @details  the call will not block but a call to \ref enterInitializingModeIterativeComplete
    should be made to complete the call sequence
    */
    void enterInitializingModeIterativeAsync();

    /** second part of the async process for entering initialization mode iterative call after a
    call to enterInitializingModeIterativeAsync; if called any other time it will throw an
    InvalidFunctionCall exception. The federate will be in the created state(or ERROR state) after
    this call*/
    void enterInitializingModeIterativeComplete();

    /** enter the normal execution mode
    @details call will block until all federates have entered this mode
    @param iterate an optional flag indicating the desired iteration mode
    */
    IterationResult enterExecutingMode(IterationRequest iterate = IterationRequest::NO_ITERATIONS);
    /** enter the normal execution mode
    @details call will return immediately but \ref enterExecutingModeComplete should be called to
    complete the operation
    @param iterate an optional flag indicating the desired iteration mode
    */
    void enterExecutingModeAsync(IterationRequest iterate = IterationRequest::NO_ITERATIONS);
    /** complete the async call for entering Execution state
    @details call will not block but will return quickly.  The enterInitializingModeComplete must be
    called before doing other operations
    */
    IterationResult enterExecutingModeComplete();
    /** terminate the simulation
    @details call is will block until the finalize has been acknowledged, no commands that interact
    with the core may be called after this function function */
    void finalize();
    /** terminate the simulation in a non-blocking call
    @details finalizeComplete must be called after this call to complete the finalize procedure*/
    void finalizeAsync();
    /** complete the asynchronous terminate pair*/
    void finalizeComplete();
    /** Run a processing loop for X amount of time. If the period is set to 0 it just processes
     * currently pending communications.
     */
    void processCommunication(std::chrono::milliseconds period = std::chrono::milliseconds(0));
    /** disconnect a simulation from the core (will also call finalize before disconnecting if
     * necessary)*/
    virtual void disconnect();

    /** specify the simulator had a local error with error code and message
    @param errorcode an integral code for the error
    @param message a string describing the error to display in a log
    */
    void localError(int errorcode, std::string_view message);

    /** specify the simulator had a global error with error code and message
    @details global errors propagate through the entire federation and will halt operations
    @param errorcode an integral code for the error
    @param message a string describing the error to display in the log
    */
    void globalError(int errorcode, std::string_view message);

    /** specify the simulator had a local error with error code
    @param errorcode an integral code for the error
    */
    void localError(int errorcode);

    /** specify the simulator had a global error with error code
    @param errorcode an integral code for the error
    */
    void globalError(int errorcode);

    /** specify a separator to use for naming separation between the federate name and the interface
    name setSeparator('.') will result in future registrations of local endpoints such as
    fedName.endpoint setSeparator('/') will result in fedName/endpoint the default is '/'  any
    character can be used though many will not make that much sense.  This call is not thread safe
    and should be called before any local interfaces are created otherwise it may not be possible to
    retrieve them without using the full name.  recommended possibilities are ('.','/', ':','-','_')
     */
    void setSeparator(char separator) { nameSegmentSeparator = separator; }
    /** request a time advancement
    @param nextInternalTimeStep the next requested time step
    @return the granted time step*/
    Time requestTime(Time nextInternalTimeStep);

    /** request a time advancement to the next allowed time
    @return the granted time step*/
    Time requestNextStep() { return requestTime(timeZero); }

    /** request a time advancement by a certain amount
    @param timeDelta the amount of time to advance
    @return the granted time step*/
    Time requestTimeAdvance(Time timeDelta) { return requestTime(mCurrentTime + timeDelta); }

    /** request a time advancement
    @param nextInternalTimeStep the next requested time step
    @param iterate a requested iteration mode
    @return the granted time step in a structure containing a return time and an IterationResult*/
    iteration_time requestTimeIterative(Time nextInternalTimeStep, IterationRequest iterate);

    /**  request a time advancement and return immediately for asynchronous function.
    @details /ref requestTimeComplete should be called to finish the operation and get the result
    @param nextInternalTimeStep the next requested time step
    */
    void requestTimeAsync(Time nextInternalTimeStep);

    /** request a time advancement with iterative call and return for asynchronous function.
    @details /ref requestTimeIterativeComplete should be called to finish the operation and get the
    result
    @param nextInternalTimeStep the next requested time step
    @param iterate a requested iteration level (none, require, optional)
    */
    void requestTimeIterativeAsync(Time nextInternalTimeStep, IterationRequest iterate);

    /** request a time advancement
    @return the granted time step*/
    Time requestTimeComplete();

    /** finalize the time advancement request
    @return the granted time step in an iteration_time structure which contains a time and iteration
    result*/
    iteration_time requestTimeIterativeComplete();

    /** set a tag (key-value pair) for a federate
    @details the tag is an arbitrary user defined string and value; the tags for a federate are
    queryable through a "tags" query or "tag/<tagname>"
    @param tag the name of the tag to set the value for
    @param value the value for the given tag*/
    void setTag(std::string_view tag, std::string_view value);
    /** get the value of a specific tag (key-value pair) for a federate
    @details the tag is an arbitrary user defined string and value; the tags for a federate are
    queryable
    @param tag the name of the tag to get the value for
    @return a const std::string& containing the value of the tag, if the tag is not defined the
    value is an empty string*/
    const std::string& getTag(std::string_view tag) const;

    /** set a time property for a federate
    @param option the option to set
    @param timeValue the value to be set
    */
    virtual void setProperty(int32_t option, double timeValue);

    /** set a time option for the federate
    @param option the option to set
    @param timeValue the value to be set
    */
    virtual void setProperty(int32_t option, Time timeValue);

    /** set a flag for the federate
    @param flag an index into the flag /ref flag-definitions.h
    @param flagValue the value of the flag defaults to true
    */
    virtual void setFlagOption(int flag, bool flagValue = true);
    /**  set an integer option for the federate
    @param option an index of the option to set
    @param optionValue an integer option value for an integer based property
    */
    virtual void setProperty(int32_t option, int32_t optionValue);

    /** get the value of a time option for the federate
    @param option the option to get
    */
    virtual Time getTimeProperty(int32_t option) const;

    /** get the value of a flag option
    @param flag an index into the flag /ref flag-definitions.h
    */
    virtual bool getFlagOption(int flag) const;
    /**  get an integer option for the federate
    @param option  the option to inquire see /ref defs
    */
    virtual int getIntegerProperty(int32_t option) const;

    /** define a logging function to use for logging message and notices from the federation and
    individual federate
    @param logFunction the callback function for doing something with a log message
    it takes 3 inputs an integer for logLevel /ref helics_log_level
    A string indicating the source of the message and another string with the actual message
    */
    void setLoggingCallback(
        const std::function<void(int, std::string_view, std::string_view)>& logFunction);

    /** register a callback function to call when the system enters initializingMode
    @details this callback will execute in the calling thread just prior to returning control to the
    caller
    @param callback the function to call; the function signature is void(bool) where the
     boolean is set to true if the iterating
    to true if the request is possibly iterating
    */
    void setInitializingEntryCallback(std::function<void(bool)> callback);

    /** register a callback function to call when the system enters executingMode
    @details this callback will execute once in the calling thread just prior to calling
    timeUpdateCallback for the first time and
    @param callback the function to call; the function signature is void(void)
    */
    void setExecutingEntryCallback(std::function<void()> callback);

    /** register a callback function to call when a timeRequest function is called
    @details this callback is executed prior to any blocking operation on any valid timeRequest
    method it will execute in the calling thread
    @param callback the function to call; the function signature is void(Time, Time, bool) where the
    first Time value is the current time and the second is the requested time.  The boolean is set
    to true if the request is possibly iterating
    */
    void setTimeRequestEntryCallback(std::function<void(Time, Time, bool)> callback);

    /** register a callback function to call when the time gets updated and before other value
    callbacks are executed
    @details this callback is executed before other callbacks updating values, no values will have
    been updated when this callback is executed it is intended purely for updating time before value
    callbacks are executed.
    @param callback the function to call; the function signature is void(Time, bool) where the Time
    value is the new time and bool is true if this is an iteration
    */
    void setTimeUpdateCallback(std::function<void(Time, bool)> callback);

    /** register a callback function to call when the federate mode is changed from one state to
    another
    @details this callback is executed before other callbacks updating values and times, no values
    will have been updated when this callback is executed. It will execute before timeUpdateCallback
    in situations where both would be called.
    @param callback the function to call; the function signature is void(Modes, Modes) the first
    argument is the new Mode and the second is the old Mode
    */
    void setModeUpdateCallback(std::function<void(Modes, Modes)> callback);

    /** register a callback function to call when a timeRequest function returns
  @details this callback is executed after all other callbacks and is the last thing executed before
  returning
  @param callback the function to call; the function signature is void(Time, bool) where the
  Time value is the new time and the boolean is set to
  true if the request is an iteration
  */
    void setTimeRequestReturnCallback(std::function<void(Time, bool)> callback);

    /** register a callback function to call when the cosimulation is completed for this federate
    @details this callback will execute once when time has reached max value or when finalize is
    called and
    @param callback the function to call; the function signature is void(void)
    */
    void setCosimulationTerminatedCallback(std::function<void()> callback);

    /** register a callback function that executes when an error is generated
   @details if set this function will execute instead of throwing an exception in some cases( NOTE:
   this is currently only used for callback federates)
    @param errorHandlerCallback the function to call; the function signature is void(int errorCode,
   std::string_view errorString)
    */
    void setErrorHandlerCallback(std::function<void(int, std::string_view)> errorHandlerCallback);

    /** make a query of the core
    @details this call is blocking until the value is returned which make take some time depending
    on the size of the federation and the specific string being queried
    @param target  the target of the query can be "federation", "federate", "broker", "core", or a
    specific name of a federate, core, or broker
    @param queryStr a string with the query see other documentation for specific properties to
    query, can be defined by the federate
    @param mode fast (asynchronous; default) means the query goes on priority channels, ordered
      (synchronous) is slower but has more ordering guarantees
    @return a string with the value requested.  this is either going to be a vector of strings value
    or a JSON string stored in the first element of the vector.  The string "#invalid" is returned
    if the query was not valid
    */
    std::string query(std::string_view target,
                      std::string_view queryStr,
                      HelicsSequencingModes mode = HELICS_SEQUENCING_MODE_FAST);

    /** make a query of the core
    @details this call is blocking until the value is returned which make take some time depending
    on the size of the federation and the specific string being queried
    @param queryStr a string with the query see other documentation for specific properties to
    query, can be defined by the federate if the local federate does not recognize the query it
    sends it on to the federation
    @param mode fast (asynchronous; default) means the query goes on priority channels, ordered
    (synchronous) is slower but has more ordering guarantees
    @return a string with the value requested.  this is either going to be a vector of strings value
    or a JSON string stored in the first element of the vector.  The string "#invalid" is returned
    if the query was not valid
    */
    std::string query(std::string_view queryStr,
                      HelicsSequencingModes mode = HELICS_SEQUENCING_MODE_FAST);

    /** make a query of the core in an async fashion
    @details this call is blocking until the value is returned which make take some time depending
    on the size of the federation and the specific string being queried
    @param target  the target of the query can be "federation", "federate", "broker", "core", or a
    specific name of a federate, core, or broker
    @param queryStr a string with the query see other documentation for specific properties to
    query, can be defined by the federate
    @param mode fast (asynchronous; default) means the query goes on priority channels,
    ordered(synchronous) is slower but has more ordering guarantees
    @return a QueryId to use for returning the result
    */
    QueryId queryAsync(std::string_view target,
                       std::string_view queryStr,
                       HelicsSequencingModes mode = HELICS_SEQUENCING_MODE_FAST);

    /** make a query of the core in an async fashion
    @details this call is blocking until the value is returned which make take some time depending
    on the size of the federation and the specific string being queried
    @param queryStr a string with the query see other documentation for specific properties to
    query, can be defined by the federate
    @param mode fast (asynchronous; default) means the query goes on priority channels,
    ordered(synchronous) is slower but has more ordering guarantees
    @return a QueryId used to get the results of the query in the future
    */
    QueryId queryAsync(std::string_view queryStr,
                       HelicsSequencingModes mode = HELICS_SEQUENCING_MODE_FAST);

    /** get the results of an async query
    @details the call will block until the results are returned inquiry of queryCompleted() to check
    if the results have been returned or not yet

    @param queryIndex the int value returned from the queryAsync call
    @return a string with the value requested.  the format of the string will be either a single
    string a string vector like "[string1; string2]" or JSON The string "#invalid" is returned if
    the query was not valid
    */
    std::string queryComplete(QueryId queryIndex);

    /** check if an asynchronous query call has been completed
    @return true if the results are ready for /ref queryFinalize
    */
    bool isQueryCompleted(QueryId queryIndex) const;

    /** supply a query callback function
    @details the intention of the query callback is to allow federates to answer particular requests
    through the query interface this allows other federates to make requests or queries of other
    federates in an asynchronous fashion.
    @param queryFunction  a function object that returns a string as a result of a query in the form
    of const string ref. This callback will be called when a federate received a query that cannot
    be answered internally that is directed at that particular federate
    */
    void setQueryCallback(const std::function<std::string(std::string_view)>& queryFunction);

    /** set a federation global value
    @details this overwrites any previous value for this name
    @param valueName the name of the global to set
    @param value the value of the global
    */
    void setGlobal(std::string_view valueName, std::string_view value);
    /** add a global alias for an interface
    @param interfaceName the given name of the interface
    @param alias the new name by which the interface can be referenced
    */
    void addAlias(std::string_view interfaceName, std::string_view alias);
    /** send a command to another core or federate
  @param target  the target of the command can be "federation", "federate", "broker", "core", or a
  specific name of a federate, core, or broker
  @param commandStr a string with the command instructions, see other documentation for specific
  properties to command, can be defined by a federate
  @param mode fast (asynchronous; default) means the command goes on priority channels,
    ordered(synchronous) is slower but has more ordering guarantees
  */
    void sendCommand(
        std::string_view target,
        std::string_view commandStr,
        HelicsSequencingModes mode = HelicsSequencingModes::HELICS_SEQUENCING_MODE_FAST);

    /** get a command for the Federate
 @return a pair of strings <command,source> with the command instructions for the federate; the
 command string will be empty if no command is given
 */
    std::pair<std::string, std::string> getCommand();

    /** get a command for the Federate, if there is none the call will block until a command is
received
@return a pair of strings <command,source> with the command instructions for the federate
*/
    std::pair<std::string, std::string> waitCommand();

    /** add a dependency for this federate
    @details adds an additional internal time dependency for the federate
    @param fedName the name of the federate to add a dependency on
    */
    void addDependency(std::string_view fedName);

    /** define a named global filter interface
    @param filterName the name of the globally visible filter
    @param inputType the inputType which the filter can handle
    @param outputType the outputType of the filter which the filter produces
    */
    Filter& registerGlobalFilter(std::string_view filterName,
                                 std::string_view inputType = std::string_view{},
                                 std::string_view outputType = std::string_view{});

    /** define a cloning filter interface on a source
    @details a cloning filter will modify copy of messages coming from or going to target endpoints
    @param filterName the name of the filter
    @param inputType the inputType which the filter can handle
    @param outputType the outputType of the filter which the filter produces
    */
    CloningFilter& registerGlobalCloningFilter(std::string_view filterName,
                                               std::string_view inputType = std::string_view{},
                                               std::string_view outputType = std::string_view{});

    /** define a filter interface
    @details a filter will modify messages coming from or going to target endpoints
    @param filterName the name of the filter
    @param inputType the inputType which the filter can handle
    @param outputType the outputType of the filter which the filter produces
    */
    Filter& registerFilter(std::string_view filterName,
                           std::string_view inputType = std::string_view{},
                           std::string_view outputType = std::string_view{});

    /** define a cloning filter interface on a source
    @details a source filter will be sent any packets that come from a particular source
    if multiple filters are defined on the same source, they will be placed in some order defined by
    the core
    @param filterName the name of the filter
    @param inputType the inputType which the filter can handle
    @param outputType the outputType of the filter which the filter produces
    */
    CloningFilter& registerCloningFilter(std::string_view filterName,
                                         std::string_view inputType = std::string_view{},
                                         std::string_view outputType = std::string_view{});

    /** define a nameless filter interface
     */
    Filter& registerFilter()
    {
        return registerGlobalFilter(std::string(), std::string_view{}, std::string_view{});
    }

    /** define a named global translator interface
    @param translatorType a code defining a known type of translator (0 for custom)
    @param translatorName the name of the globally visible translator
    @param endpointType the type associated with the translator endpoint
    @param units the units associated with the translator value interfaces
    */
    Translator& registerGlobalTranslator(std::int32_t translatorType,
                                         std::string_view translatorName,
                                         std::string_view endpointType = std::string_view{},
                                         std::string_view units = std::string_view{});

    /** define a translator interface
    @details a translator acts as a bridge between value and message interfaces
    @param translatorType a code defining a known type of translator (0 for custom)
    @param translatorName the name of the  translator; can be an empty string
    @param endpointType the type associated with the translator endpoint
    @param units the units associated with the translator value interfaces
    */
    Translator& registerTranslator(std::int32_t translatorType,
                                   std::string_view translatorName,
                                   std::string_view endpointType = std::string_view{},
                                   std::string_view units = std::string_view{});

    /** define a named global translator interface
    @param translatorName the name of the globally visible translator
    @param endpointType the type associated with the translator endpoint
    @param units the units associated with the translator value interfaces
    */
    Translator& registerGlobalTranslator(std::string_view translatorName,
                                         std::string_view endpointType = std::string_view{},
                                         std::string_view units = std::string_view{})
    {
        return registerGlobalTranslator(0, translatorName, endpointType, units);
    }

    /** define a translator interface
    @details a translator acts as a bridge between value and message interfaces
    @param translatorName the name of the translator
    @param endpointType the type associated with the translator endpoint
    @param units the units associated with the translator value interfaces
    */
    Translator& registerTranslator(std::string_view translatorName,
                                   std::string_view endpointType = std::string_view{},
                                   std::string_view units = std::string_view{})
    {
        return registerTranslator(0, translatorName, endpointType, units);
    }

    /** define a nameless translator interface
     */
    Translator& registerTranslator() { return registerGlobalTranslator(""); }
    /** define a nameless cloning filter interface on a source
     */
    CloningFilter& registerCloningFilter()
    {
        return registerGlobalCloningFilter(std::string(), std::string(), std::string());
    }

    /** get a filter from its name
    @param filterName the name of the filter
    @return a reference to a filter object which could be invalid if filterName is not valid*/
    const Filter& getFilter(std::string_view filterName) const;

    /** get a filter from its index
    @param index the index of a filter
    @return a reference to a filter object which could be invalid if filterName is not valid*/
    const Filter& getFilter(int index) const;

    /** get a filter from its name
      @param filterName the name of the filter
      @return a reference to a filter object which could be invalid if filterName is not valid*/
    Filter& getFilter(std::string_view filterName);

    /** get a filter from its index
    @param index the index of a filter
    @return a reference to a filter object which could be invalid if filterName is not valid*/
    Filter& getFilter(int index);

    /** @brief register a operator for the specified filter
    @details
    The FilterOperator gets called when there is a message to filter, There is no order or state to
    this messages can come in any order.
    The function is deprecated in favor of the corresponding call on a filter object
    equivalent to filt.setOperator(filtOp)
    @param filt the filter object to set the operation on
    @param filtOp a shared_ptr to a \ref FilterOperator
    */
    [[deprecated]] static void setFilterOperator(Filter& filt,
                                                 std::shared_ptr<FilterOperator> filtOp);

    /** get the number of filters registered through this federate*/
    int getFilterCount() const;

    // translator retrieval
    /** get a translator from its name
    @param translatorName the name of the translator
    @return a reference to a translator object which will be invalid if translatorName is not
    valid*/
    const Translator& getTranslator(std::string_view translatorName) const;

    /** get a translator from its index
    @param index the index of a translator
    @return a reference to a translator object which will be invalid if index is not
    valid*/
    const Translator& getTranslator(int index) const;

    /** get a translator from its name
      @param translatorName the name of the translator
      @return a reference to a translator object which will be invalid if translatorName is not
      valid*/
    Translator& getTranslator(std::string_view translatorName);

    /** get a translator from its index
    @param index the index of a translator
    @return a reference to a translator object which will be invalid if index is not
    valid*/
    Translator& getTranslator(int index);

    /** @brief register an operator for the specified translator
    @details
    The TranslatorOperator gets called when there is a message or value to translate, there is no
    order or state to this as messages can come in any order.
    The function is deprecated in favor of the corresponding call on a translator object
    equivalent to trans.setOperator(transop)
    @param trans the translator object to set the operation on
    @param transOp a shared_ptr to a \ref TranslatorOperator
    */
    [[deprecated]] static void setTranslatorOperator(Translator& trans,
                                                     std::shared_ptr<TranslatorOperator> transOp);

    /** get the number of translators registered through this federate*/
    int getTranslatorCount() const;
    /** get the primary config file actually used by the federate for setup*/
    const std::string& getConfigFile() const { return configFile; }

  protected:
    /** function to run required operations for entering initializingMode*/
    void enteringInitializingMode(IterationResult iterating);

    /** function to run required operations for entering executing Mode*/
    void enteringExecutingMode(iteration_time res);
    /** function tor run required operations when finalizing*/
    void finalizeOperations();
    void preTimeRequestOperations(Time nextStep, bool iterating);
    void postTimeRequestOperations(Time newTime, bool iterating);
    /** function to deal with any operations that need to occur on a time update*/
    virtual void updateTime(Time newTime, Time oldTime);
    /** function to deal with any operations that need to occur on the transition from startup to
     * initialize*/
    virtual void startupToInitializeStateTransition();
    /** function to deal with any operations that need to occur on the transition from startup to
     * initialize*/
    virtual void initializeToExecuteStateTransition(iteration_time iterate);
    /** function to handle any disconnect operations*/
    virtual void disconnectTransition();
    /** function to generate results for a local Query
    @details should return an empty string if the query is not recognized*/
    virtual std::string localQuery(std::string_view queryStr) const;
    /** generate a string with the local variant of the name with the specified suffix
    @param[in] addition the suffix to append to the current object name*/
    std::string localNameGenerator(std::string_view addition) const;
    /** process an error */
    void handleError(int errorCode, std::string_view errorString, bool noThrow);
    void setAsyncCheck(std::function<bool()> asyncCheck);

  public:
    /** register a set of interfaces defined in a file
    @details call is only valid in startup mode
    @param configString  the location of the file or config String to load to generate the
    interfaces
    */
    virtual void registerInterfaces(const std::string& configString);
    /** register filter/translator interfaces defined in a file or string
    @details call is only valid in startup mode
    @param configString  the location of the file or config String to load to generate the
    interfaces
    */
    void registerConnectorInterfaces(const std::string& configString);
    /** register filter/translator interfaces defined in a file or string
    @details call is only valid in startup mode will be deprecated
    @param configString  the location of the file or config String to load to generate the
    interfaces
    */
    void registerFilterInterfaces(const std::string& configString)
    {
        registerConnectorInterfaces(configString);
    }
    /** get the underlying federateID for the core*/
    auto getID() const noexcept { return fedID; }
    /** get the current state of the federate*/
    Modes getCurrentMode() const noexcept { return currentMode.load(); }
    /** get the current Time
    @details the most recent granted time of the federate*/
    Time getCurrentTime() const noexcept { return mCurrentTime; }
    /** get the federate name*/
    const std::string& getName() const { return mName; }
    /** get a shared pointer to the core object used by the federate*/
    const std::shared_ptr<Core>& getCorePointer() { return coreObject; }

    /** log a message to the federate Logger
   @param level the logging level of the message
   @param message the message to log
   */
    void logMessage(int level, std::string_view message) const;

    /** log an error message to the federate Logger
    @param message the message to log
    */
    void logErrorMessage(std::string_view message) const
    {
        logMessage(HELICS_LOG_LEVEL_ERROR, message);
    }
    /** log a warning message to the federate Logger
    @param message the message to log
    */
    void logWarningMessage(std::string_view message) const
    {
        logMessage(HELICS_LOG_LEVEL_WARNING, message);
    }
    /** log an info message to the federate Logger
    @param message the message to log
    */
    void logInfoMessage(std::string_view message) const
    {
        logMessage(HELICS_LOG_LEVEL_SUMMARY, message);
    }
    /** log a debug message to the federate Logger
    @param message the message to log
    */
    void logDebugMessage(std::string_view message) const
    {
        logMessage(HELICS_LOG_LEVEL_DEBUG, message);
    }
    /** call to complete async operation with no output*/
    void completeOperation();

  private:
    void getCore(const FederateInfo& fedInfo);
    /** function to get the core into a valid state*/
    void verifyCore();
    /** function to register the federate with the core*/
    void registerFederate(const FederateInfo& fedInfo);

    /** function to deal with any operations that occur on a mode switch*/
    void updateFederateMode(Modes newMode);
    /** run the actions necessary for starting up with potential interfaces*/
    void potentialInterfacesStartupSequence();
    /** function to deal with any operations that need to occur on a time update*/
    void updateSimulationTime(Time newTime, Time oldTime, bool iterating);
    /** register connector(filters,translators) interfaces defined in  file or string
  @details call is only valid in startup mode
  @param jsonString  the location of the file or config String to load to generate the interfaces
  */
    void registerConnectorInterfacesJson(const std::string& jsonString);
    /** register connector(filters,translators) interfaces defined in  file or string
    @details call is only valid in startup mode
    @param tomlString  the location of the file or config String to load to generate the interfaces
    */
    void registerConnectorInterfacesToml(const std::string& tomlString);
    /** check if a filter type and operation is valid
     */
    void registerConnectorInterfacesJsonDetail(const fileops::JsonBuffer& json);
    bool
        checkValidFilterType(bool useTypes, FilterTypes opType, const std::string& operation) const;
};

/** base class for the interface objects*/
class HELICS_CXX_EXPORT Interface {
  protected:
    Core* mCore{nullptr};  //!< pointer to the core object
    InterfaceHandle handle{};  //!< the id as generated by the Federate
    std::string mName;  //!< the name or key of the interface
  public:
    Interface() = default;
    Interface(Federate* federate, InterfaceHandle hid, std::string_view actName);
    Interface(Core* core, InterfaceHandle hid, std::string_view actName):
        mCore(core), handle(hid), mName(actName)
    {
    }
    virtual ~Interface() = default;
    /** get the underlying handle that can be used to make direct calls to the Core API
     */
    InterfaceHandle getHandle() const { return handle; }
    /** implicit conversion operator for extracting the handle*/
    operator InterfaceHandle() const { return handle; }
    /** check if the Publication links to a valid operation*/
    bool isValid() const { return handle.isValid(); }
    bool operator<(const Interface& inp) const { return (handle < inp.handle); }
    bool operator>(const Interface& inp) const { return (handle > inp.handle); }
    bool operator==(const Interface& inp) const { return (handle == inp.handle); }
    bool operator!=(const Interface& inp) const { return (handle != inp.handle); }
    /** get the Name/Key for the input
    @details the name is the local name if given, key is the full key name*/
    const std::string& getLocalName() const { return mName; }
    /** get the Name/Key for the input
    @details the name is the local name if given, key is the full key name*/
    const std::string& getName() const;
    /** get an associated target*/
    [[deprecated]] const std::string& getTarget() const;
    /** add a source of information to an interface*/
    void addSourceTarget(std::string_view newTarget, InterfaceType hint = InterfaceType::UNKNOWN);
    /** add destination for data sent via the interface*/
    void addDestinationTarget(std::string_view newTarget,
                              InterfaceType hint = InterfaceType::UNKNOWN);
    /** remove a named interface from the target lists*/
    void removeTarget(std::string_view targetToRemove);
    /** add an alternate global name for an interface*/
    void addAlias(std::string_view alias);
    /** get the interface information field of the input*/
    const std::string& getInfo() const;
    /** set the interface information field of the input*/
    void setInfo(std::string_view info);
    /** set a tag (key-value pair) for an interface
    @details the tag is an arbitrary user defined string and value; the tags for an interface are
    queryable
    @param tag the name of the tag to set the value for
    @param value the value for the given tag*/
    void setTag(std::string_view tag, std::string_view value);
    /** get the value of a specific tag (key-value pair) for an interface
    @details the tag is an arbitrary user defined string and value; the tags for an interface are
    queryable
    @param tag the name of the tag to get the value for
    @return a const std::string& containing the value of the tag, if the tag is not defined the
    value is an empty string*/
    const std::string& getTag(std::string_view tag) const;
    /** set a handle flag for the input*/
    virtual void setOption(int32_t option, int32_t value = 1);

    /** get the current value of a flag for the handle*/
    virtual int32_t getOption(int32_t option) const;

    /** get the injection type for an interface,  this is the type for data coming into an interface
    @details for filters this is the input type, for publications this is the type used to transmit
    data, for endpoints this is the specified type and for inputs this is the type of the
    transmitting publication
    @return a const ref to  std::string  */
    const std::string& getInjectionType() const;

    /** get the extraction type for an interface,  this is the type for data coming out of an
    interface
    @details for filters this is the output type, for publications this is the specified type, for
    endpoints this is the specified type and for inputs this is the specified type
    @return a const ref to  std::string  */
    const std::string& getExtractionType() const;

    /** get the injection units for an interface,  this is the units associated with data coming
  into an interface
  @details for inputs this is the input type, for publications this is the units used to transmit
  data, and for inputs this is the units of the transmitting publication
  @return a const ref to  std::string  */
    const std::string& getInjectionUnits() const;

    /** get the extraction units for an interface,  this is the units associated with data coming
    out of an interface
    @details for publications this is the specified units, for inputs this is the specified type
    @return a const ref to  std::string  */
    const std::string& getExtractionUnits() const;
    /** get the display name for an input
    @details the name is the given local name or if empty the name of the target*/
    virtual const std::string& getDisplayName() const = 0;
    /** get the source targets for an interface, either the sources for endpoints or inputs, or the
     * source endpoints for a filter*/
    const std::string& getSourceTargets() const;
    /** get the destination targets for an interface, either the destinations of data for endpoints
     * or publications, or the destination endpoints for a filter*/
    const std::string& getDestinationTargets() const;
    /** get the number of source targets*/
    std::size_t getSourceTargetCount() const;
    /** get the number of destination targets*/
    std::size_t getDestinationTargetCount() const;
    /** close the interface*/
    void close();
    /** disconnect the object from the core*/
    void disconnectFromCore();
};

/** function to do some housekeeping work
@details this runs some cleanup routines and tries to close out any residual thread that haven't
been shutdown yet*/
HELICS_CXX_EXPORT void cleanupHelicsLibrary();
}  // namespace helics
