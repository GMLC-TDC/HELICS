/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "../core/helics-time.hpp"
#include "helics/helics-config.h"
#include "helicsTypes.hpp"

#include "../helics_enums.h"
#include "FederateInfo.hpp"
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>

namespace libguarded
{
template <class T, class M>
class shared_guarded;
}

/**
 * HELICS Application API
 */
namespace helics
{
class Core;
class AsyncFedCallInfo;
class MessageOperator;
class FilterFederateManager;
class Filter;
class CloningFilter;

/** base class for a federate in the application API
 */
class Federate
{
  public:
    /** the allowable operation modes of the federate*/
    enum class modes : char
    {
        startup = 0,  //!< when created the federate is in startup state
        initializing = 1,  //!< entered after the enterInitializingMode call has returned
        executing = 2,  //!< entered after the enterExectuationState call has returned
        finalize = 3,  //!< the federate has finished executing normally final values may be retrieved
        error = 4,  //!< error state no core communication is possible but values can be retrieved
        // the following states are for asynchronous operations
        pending_init = 5,  //!< indicator that the federate is pending entry to initialization state
        pending_exec = 6,  //!< state pending EnterExecution State
        pending_time = 7,  //!< state that the federate is pending a timeRequest
        pending_iterative_time = 8,  //!< state that the federate is pending an iterative time request
        pending_finalize = 9  //!< state that the federate is pending a finalize call
    };

  protected:
    std::atomic<modes> currentMode{modes::startup};  //!< the current state of the simulation
    char separator_ = '/';  //!< the separator between automatically prependend names
  private:
    federate_id_t fedID;  //!< the federate ID of the object for use in the core
  protected:
    std::shared_ptr<Core> coreObject;  //!< reference to the core simulation API
    Time currentTime;  //!< the current simulation time
  private:
    std::unique_ptr<libguarded::shared_guarded<AsyncFedCallInfo, std::mutex>>
      asyncCallInfo;  //!< pointer to a class defining the async call information
    std::unique_ptr<FilterFederateManager> fManager;  //!< class for managing filter operations
    std::string name;  //!< the name of the federate
  public:
    /**constructor taking a federate information structure
    @param[in] fi  a federate information structure
    */
    Federate (const std::string &fedname, const FederateInfo &fi);
    /**constructor taking a core and a federate information structure
    @param core a shared pointer to a core object, the pointer will be copied
    @param[in] fi  a federate information structure
    */
    Federate (const std::string &fedname, const std::shared_ptr<Core> &core, const FederateInfo &fi);
    /**constructor taking a file with the required information
    @param[in] configString can be either a JSON file or a string containing JSON code or a TOML file
    */
    explicit Federate (const std::string &configString);
    /**constructor taking a file with the required information and the name of the federate
    @param[in] name the name of the federate
    @param[in] configString can be either a JSON file or a string containing JSON code or a TOML file with
    extension (.toml, .TOML)
    */
    Federate (const std::string &fedname, const std::string &configString);
    /**default constructor*/
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
    void enterInitializingMode ();

    /** enter the initialization mode after all interfaces have been defined
    @details  the call will not block
    */
    void enterInitializingModeAsync ();
    /** called after one of the async calls and will indicate true if an async operation has completed
    @details only call from the same thread as the one that called the initial async call and will return false
    if called when no aysnc operation is in flight*/
    bool isAsyncOperationCompleted () const;
    /** second part of the async process for entering initializationState call after a call to
    enterInitializingModeAsync if call any other time it will throw an InvalidFunctionCall exception*/
    void enterInitializingModeComplete ();
    /** enter the normal execution mode
    @details call will block until all federates have entered this mode
    @param iterate an optional flag indicating the desired iteration mode
    */
    iteration_result enterExecutingMode (iteration_request iterate = iteration_request::no_iterations);
    /** enter the normal execution mode
    @details call will block until all federates have entered this mode
    */
    void enterExecutingModeAsync (iteration_request iterate = iteration_request::no_iterations);
    /** complete the async call for entering Execution state
    @details call will not block but will return quickly.  The enterInitializingModeComplete must be called
    before doing other operations
    */
    iteration_result enterExecutingModeComplete ();
    /** terminate the simulation
    @details call is will block until the finalize has been acknowledged, no commands that interact with the core
    may be called after this function function */
    void finalize ();
    /** terminate the simulation in a non-blocking call
    @details finalizeComplete must be called after this call to complete the finalize procedure*/
    void finalizeAsync ();
    /** complete the asynchronous terminate pair*/
    void finalizeComplete ();

    /** disconnect a simulation from the core (will also call finalize before disconnecting if necessary)*/
    virtual void disconnect ();
    /** specify the simulator had an unrecoverable error
     */
    void error (int errorcode);
    /** specify the simulator had an error with error code and message
     */
    void error (int errorcode, const std::string &message);

    /** specify a separator to use for naming separation between the federate name and the interface name
    @example setSeparator('.') will result in future registrations of local endpoints such as fedName.endpoint
    setSeparator('/') will result in fedName/endpoint
    the default is '/'  any character can be used though many will not make that much sense.  This call is not
    thread safe and should be called before any local interfaces are created otherwise it may not be possible to
    retrieve them without using the full name.  recommended possibilities are ('.','/', ':','-','_')
     */
    void setSeparator (char separator) { separator_ = separator; }
    /** request a time advancement
    @param[in] the next requested time step
    @return the granted time step*/
    Time requestTime (Time nextInternalTimeStep);

    /** request a time advancement to the next allowed time
  @return the granted time step*/
    Time requestNextStep () { return requestTime (timeZero); }

    /** request a time advancement
    @param[in] the next requested time step
    @param[in] iterate a requested iteration mode
    @return the granted time step in a structure containing a return time and an iteration_result*/
    iteration_time requestTimeIterative (Time nextInternalTimeStep, iteration_request iterate);

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
    @return the granted time step in an iteration_time structure which contains a time and iteration result*/
    iteration_time requestTimeIterativeComplete ();

    /** set a time option for the federate
    @param option the option to set
    @param timeValue the value to be set
    */
    void setProperty (int32_t option, double timeValue);

    /** set a time option for the federate
    @param[in] option the option to set
    @param[in] timeValue the value to be set
    */
    void setProperty (int32_t option, Time timeValue);

    /** set a flag for the federate
    @param[in] flag an index into the flag /ref flag-definitions.h
    @param[in] flagValue the value of the flag defaults to true
    */
    virtual void setFlagOption (int flag, bool flagValue = true);
    /**  set an integer option for the federate
    @ details debug and trace only do anything if they were enabled in the compilation
    @param loggingLevel (-1: none, 0: error_only, 1: warnings, 2: normal, 3: debug, 4: trace)
    */
    void setProperty (int32_t option, int32_t optionValue);

    /** get the value of a time option for the federate
    @param[in] option the option to get
    */
    Time getTimeProperty (int32_t option);

    /** get the value of a flag option
    @param[in] flag an index into the flag /ref flag-definitions.h
    */
    virtual bool getFlagOption (int flag);
    /**  set an integer option for the federate
    @param option,  the option to inquire
    */
    int getIntegerProperty (int32_t option);

    /** define a logging function to use for logging message and notices from the federation and individual
    federate
    @param logFunction the callback function for doing something with a log message
    it takes 3 inputs an integer for logLevel 0-4+  0 -error, 1- warning 2-status, 3-debug 44trace
    A string indicating the source of the message and another string with the actual message
    */
    void
    setLoggingCallback (const std::function<void(int, const std::string &, const std::string &)> &logFunction);

    /** make a query of the core
    @details this call is blocking until the value is returned which make take some time depending on the size of
    the federation and the specific string being queried
    @param target  the target of the query can be "federation", "federate", "broker", "core", or a specific name of
    a federate, core, or broker
    @param queryStr a string with the query see other documentation for specific properties to query, can be
    defined by the federate
    @return a string with the value requested.  this is either going to be a vector of strings value or a JSON
    string stored in the first element of the vector.  The string "#invalid" is returned if the query was not valid
    */
    std::string query (const std::string &target, const std::string &queryStr);

    /** make a query of the core
    @details this call is blocking until the value is returned which make take some time depending on the size of
    the federation and the specific string being queried
    @param queryStr a string with the query see other documentation for specific properties to query, can be
    defined by the federate if the local federate does not recognize the query it sends it on to the federation
    @return a string with the value requested.  this is either going to be a vector of strings value or a JSON
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
   @return a string with the value requested.  the format of the string will be either a single string a string
   vector like "[string1; string2]" or JSON The string "#invalid" is returned if the query was not valid
    */
    std::string queryComplete (query_id_t queryIndex);

    /** check if an asynchronous query call has been completed
    @return true if the results are ready for @queryFinalize
    */
    bool isQueryCompleted (query_id_t queryIndex) const;

	/** set a federation global value
	@details this overwrites any previous value for this name
	@param valueName the name of the global to set
	@param value the value of the global
	*/
    void setGlobal (const std::string &valueName, const std::string &value);
    /** define a filter interface
    @details a source filter will be sent any packets that come from a particular source
    if multiple filters are defined on the same source, they will be placed in some order defined by the core
    @param[in] the name of the endpoint
    @param[in] the inputType which the source filter can receive
    */
    Filter &registerGlobalFilter (const std::string &filterName,
                                  const std::string &inputType = std::string (),
                                  const std::string &outputType = std::string ());

    /** define a cloning filter interface on a source
    @details a source filter will be sent any packets that come from a particular source
    if multiple filters are defined on the same source, they will be placed in some order defined by the core
    @param filterName the name of the filter
    @param inputType the inputType which the filter can handle
    @param outputType the outputType of the filter which the filter produces
    */
    CloningFilter &registerGlobalCloningFilter (const std::string &filterName,
                                                const std::string &inputType = std::string (),
                                                const std::string &outputType = std::string ());

    /** define a filter interface
    @details a source filter will be sent any packets that come from a particular source
    if multiple filters are defined on the same source, they will be placed in some order defined by the core
    @param[in] the name of the endpoint
    @param[in] the inputType which the source filter can receive
    */
    Filter &registerFilter (const std::string &filterName,
                            const std::string &inputType = std::string (),
                            const std::string &outputType = std::string ());

    /** define a cloning filter interface on a source
    @details a source filter will be sent any packets that come from a particular source
    if multiple filters are defined on the same source, they will be placed in some order defined by the core
    @param filterName the name of the filter
    @param inputType the inputType which the filter can handle
    @param outputType the outputType of the filter which the filter produces
    */
    CloningFilter &registerCloningFilter (const std::string &filterName,
                                          const std::string &inputType = std::string (),
                                          const std::string &outputType = std::string ());

    /** define a filter interface on a source
    @details a source filter will be sent any packets that come from a particular source
    if multiple filters are defined on the same source, they will be placed in some order defined by the core
    @param[in] the name of the endpoint
    @param[in] the inputType which the source filter can receive
    */
    Filter &registerFilter () { return registerGlobalFilter (std::string (), std::string (), std::string ()); }

    /** define a cloning filter interface on a source
    @details a source filter will be sent any packets that come from a particular source
    if multiple filters are defined on the same source, they will be placed in some order defined by the core
    @param[in] the name of the endpoint
    @param[in] the inputType which the source filter can receive
    */
    CloningFilter &registerCloningFilter ()
    {
        return registerGlobalCloningFilter (std::string (), std::string (), std::string ());
    }

    /** add a source target to a filter
   @param id the identifier of the filter
   target the name of the endpoint to filter the data from
   */
    void addSourceTarget (const Filter &filt, const std::string &targetEndpoint);
    /** add a destination target to a filter
  @param id the identifier of the filter
  target the name of the endpoint to filter the data going to
  */
    void addDestinationTarget (const Filter &filt, const std::string &targetEndpoint);

    /** get the name of a filter
    @param[in] id the filter to query
    @return empty string if an invalid id is passed*/
    const std::string &getFilterName (const Filter &filt) const;

    /** get the id of a source filter from the name of the endpoint
    @param[in] filterName the name of the filter
    @return invalid_filter_id if name is not recognized otherwise returns the filter id*/
    const Filter &getFilter (const std::string &filterName) const;

    /** get the id of a source filter from the name of the endpoint
  @param[in] filterName the name of the filter
  @return invalid_filter_id if name is not recognized otherwise returns the filter id*/
    const Filter &getFilter (int index) const;

    /** get the id of a source filter from the name of the endpoint
  @param[in] filterName the name of the filter
  @return invalid_filter_id if name is not recognized otherwise returns the filter id*/
    Filter &getFilter (const std::string &filterName);

    /** get the id of a source filter from the name of the endpoint
  @param[in] filterName the name of the filter
  @return invalid_filter_id if name is not recognized otherwise returns the filter id*/
    Filter &getFilter (int index);

    /** @brief register a operator for the specified filter
    @details
    The FilterOperator gets called when there is a message to filter, There is no order or state to this
    messages can come in any order.
    @param[in] filter the identifier for the filter to trigger
    @param[in] op a shared_ptr to a message operator
    */
    void setFilterOperator (const Filter &filt, std::shared_ptr<FilterOperator> op);

    /** get the number of filters registered through this federate*/
    int getFilterCount () const;

  protected:
    /** function to deal with any operations that need to occur on a time update*/
    virtual void updateTime (Time newTime, Time oldTime);
    /** function to deal with any operations that need to occur on the transition from startup to initialize*/
    virtual void startupToInitializeStateTransition ();
    /** function to deal with any operations that need to occur on the transition from startup to initialize*/
    virtual void initializeToExecuteStateTransition ();
    /** function to generate results for a local Query
    @details should return an empty string if the query is not recognized*/
    virtual std::string localQuery (const std::string &queryStr) const;

  public:
    /** register a set of interfaces defined in a file
    @details call is only valid in startup mode
    @param[in] jsonString  the location of the file or config String to load to generate the interfaces
    */
    virtual void registerInterfaces (const std::string &configString);
    /** register filter interfaces defined in  file or string
    @details call is only valid in startup mode
    @param[in] configString  the location of the file or config String to load to generate the interfaces
    */
    void registerFilterInterfaces (const std::string &configString);
    /** disconnect an interface from its targets and remove it from consideration
     */
    void closeInterface (interface_handle handle);
    /** get the underlying federateID for the core*/
    auto getID () const noexcept { return fedID; }
    /** get the current state of the federate*/
    modes getCurrentMode () const { return currentMode.load (); }
    /** get the current Time
    @details the most recent granted time of the federate*/
    Time getCurrentTime () const { return currentTime; }
    /** get the federate name*/
    const std::string &getName () const { return name; }
    /** get a pointer to the core object used by the federate*/
    std::shared_ptr<Core> getCorePointer () { return coreObject; }
    // interface for filter objects
    /** get a count of the number of filter objects stored in the federate*/
    int filterCount () const;
    /** set the information field for an interface
    @param handle the interface handle for any interface,  the interface handle can be created from
    any interface object automatically
    @param info the information to store
    */
    void setInfo (interface_handle handle, const std::string &info);
    /** get the data currently stored for a particular interface handle
    @param handle the handle to get the information for
    @return a string with the data for the information*/
    const std::string &getInfo (interface_handle handle);

    /** set an interface option */
    void setInterfaceOption (interface_handle handle, int32_t option, bool option_value = true);
    /** get the current value for an interface option*/
    bool getInterfaceOption (interface_handle handle, int32_t option);

    /** get the injection type for an interface,  this is the type for data coming into an interface
    @details for filters this is the input type, for publications this is type used to transmit data, for endpoints this is the specified type and for inputs this is the type of the transmitting publication
    @param handle the interface handle to get the injection type for
    @return a const ref to  std::string  */
    const std::string &getInjectionType(interface_handle handle) const;

    /** get the type for which data comes out of an interface,  this is the type for data coming into an interface
    @details for filters this is the output type, for publications this is the specified type, for endpoints this is the specified type and for inputs this is the specified type
    @param handle the interface handle to get the injection type for
    @return a const ref to  std::string  */
    const std::string &getExtractionType(interface_handle handle) const;
    

  private:
    /** register filter interfaces defined in  file or string
  @details call is only valid in startup mode
  @param[in] configString  the location of the file or config String to load to generate the interfaces
  */
    void registerFilterInterfacesJson (const std::string &jsonString);
    /** register filter interfaces defined in  file or string
    @details call is only valid in startup mode
    @param[in] configString  the location of the file or config String to load to generate the interfaces
    */
    void registerFilterInterfacesToml (const std::string &tomlString);
};

/** function to do some housekeeping work
@details this runs some cleanup routines and tries to close out any residual thread that haven't been shutdown
yet*/
void cleanupHelicsLibrary ();
}  // namespace helics
