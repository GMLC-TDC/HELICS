/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_FEDERATE_HPP_
#define HELICS_CPP98_FEDERATE_HPP_
#pragma once

#include "Filter.hpp"
#include "config.hpp"
#include "helics/helics.h"
#include "helicsExceptions.hpp"

#include <complex>
#include <string>
#include <vector>

#if defined(HELICS_HAS_FUNCTIONAL) && HELICS_HAS_FUNCTIONAL != 0
#    include <functional>
#    include <utility>
#endif

namespace helicscpp {
/** hold federate information in the C++98 API*/
class FederateInfo {
  public:
    /** constructor*/
    FederateInfo() { fi = helicsCreateFederateInfo(); }
    /** construct from a type string
    @param coretype the type of core to use for the federate*/
    explicit FederateInfo(const std::string& coretype)
    {
        fi = helicsCreateFederateInfo();
        helicsFederateInfoSetCoreTypeFromString(fi, coretype.c_str(), hThrowOnError());
    }
    /** construct from a type
   @param coretype the type of core to use for the federate*/
    explicit FederateInfo(int coretype)
    {
        fi = helicsCreateFederateInfo();
        helicsFederateInfoSetCoreType(fi, coretype, hThrowOnError());
    }
    /** copy constructor for federate info*/
    FederateInfo(const FederateInfo& fedInfo)
    {
        fi = helicsFederateClone(fedInfo.fi, hThrowOnError());
    }
    /** copy assignment for federateInfo*/
    FederateInfo& operator=(const FederateInfo& fedInfo)
    {
        HelicsFederateInfo fi_new = helicsFederateClone(fedInfo.fi, hThrowOnError());
        helicsFederateInfoFree(fi);
        fi = fi_new;
        return *this;
    }
#ifdef HELICS_HAS_RVALUE_REFS
    /** move constructor for federateInfo*/
    FederateInfo(FederateInfo&& fedInfo) HELICS_NOTHROW
    {
        fi = fedInfo.fi;
        fedInfo.fi = HELICS_NULL_POINTER;
    }
    /** move assignment for federateInfo*/
    FederateInfo& operator=(FederateInfo&& fedInfo) HELICS_NOTHROW
    {
        helicsFederateInfoFree(fi);
        fi = fedInfo.fi;
        fedInfo.fi = HELICS_NULL_POINTER;
        return *this;
    }
#endif
    /** destructor*/
    ~FederateInfo() { helicsFederateInfoFree(fi); }
    void loadFromArgs(const std::string& argString)
    {
        helicsFederateInfoLoadFromString(fi, argString.c_str(), HELICS_NULL_POINTER);
    }
    /** set the core name to use in the federateInfo
    @param corename the core name to use*/
    void setCoreName(const std::string& corename)
    {
        helicsFederateInfoSetCoreName(fi, corename.c_str(), HELICS_NULL_POINTER);
    }
    /// Set the separator character
    void setSeparator(char sep) { helicsFederateInfoSetSeparator(fi, sep, HELICS_NULL_POINTER); }
    /** set the core init string to use in the federateInfo
    @param coreInit the core name to use*/
    void setCoreInit(const std::string& coreInit)
    {
        helicsFederateInfoSetCoreInitString(fi, coreInit.c_str(), HELICS_NULL_POINTER);
    }
    /// Set a string for the broker initialization in command line argument format
    void setBrokerInit(const std::string& brokerInit)
    {
        helicsFederateInfoSetBrokerInitString(fi, brokerInit.c_str(), HELICS_IGNORE_ERROR);
    }
    /** set the core type from a string with the core type
    @param coretype the string defining a core type
    */
    void setCoreType(const std::string& coretype)
    {
        helicsFederateInfoSetCoreTypeFromString(fi, coretype.c_str(), hThrowOnError());
    }
    /** set the core type from an integer \ref HelicsCoreTypes
    @param coretype an integer code with the federate type
    */
    void setCoreType(int coretype)
    {
        helicsFederateInfoSetCoreType(fi, coretype, HELICS_NULL_POINTER);
    }
    /** set the broker to connect with
    @param broker a string with the broker connection information or name
    */
    void setBroker(const std::string& broker)
    {
        helicsFederateInfoSetBroker(fi, broker.c_str(), HELICS_NULL_POINTER);
    }
    /** set the broker key to use
    @param brokerkey a string with the broker key information
    */
    void setBrokerKey(const std::string& brokerkey)
    {
        helicsFederateInfoSetBrokerKey(fi, brokerkey.c_str(), HELICS_NULL_POINTER);
    }
    /** set a flag
    @param flag /ref helics_federate_flags
    @param value the value of the flag usually HELICS_TRUE or HELICS_FALSE
    */
    void setFlagOption(int flag, bool value = true)
    {
        helicsFederateInfoSetFlagOption(fi,
                                        flag,
                                        value ? HELICS_TRUE : HELICS_FALSE,
                                        HELICS_NULL_POINTER);
    }
    /** set a time federate or core property
    @param timeProperty /ref helics_federate_properties an integer code with the property
    @param timeValue the value to set the property to
    */
    void setProperty(int timeProperty, HelicsTime timeValue)
    {
        helicsFederateInfoSetTimeProperty(fi, timeProperty, timeValue, HELICS_NULL_POINTER);
    }
    /** set an integral federate or core property
  @param integerProperty /ref helics_federate_properties an integer code with the property
  @param propertyValue the value to set the property to
  */
    void setProperty(int integerProperty, int propertyValue)
    {
        helicsFederateInfoSetIntegerProperty(fi,
                                             integerProperty,
                                             propertyValue,
                                             HELICS_NULL_POINTER);
    }
    void sendCommand(const std::string& target, const std::string& cmd)
    {
        helicsFederateSendCommand(fi, target.c_str(), cmd.c_str(), HELICS_IGNORE_ERROR);
    }

    const char* getCommand() { return helicsFederateGetCommand(fi, HELICS_IGNORE_ERROR); }

    const char* getCommandSource()
    {
        return helicsFederateGetCommandSource(fi, HELICS_IGNORE_ERROR);
    }
    /** get the underlying HelicsFederateInfo object*/
    HelicsFederateInfo getInfo() { return fi; }

  private:
    HelicsFederateInfo fi;  //!< handle for the underlying federate_info object
};

#if defined(HELICS_HAS_FUNCTIONAL) && HELICS_HAS_FUNCTIONAL != 0
namespace details {
    /** helper function for the callback executor for queries*/
    inline void helicCppQueryCallbackExecutor(const char* query,
                                              int stringSize,
                                              HelicsQueryBuffer buffer,
                                              void* userData)
    {
        auto cback = reinterpret_cast<std::function<std::string(const std::string&)>*>(userData);
        std::string val(query, stringSize);
        std::string result = (*cback)(val);
        helicsQueryBufferFill(buffer, result.c_str(), static_cast<int>(result.size()), nullptr);
    }
}  // namespace details
#endif

/** an iteration time structure */
typedef struct HelicsIterationTime {
    HelicsTime grantedTime;  //!< the time of the granted step
    HelicsIterationResult status;  //!< the convergence state
} HelicsIterationTime;

/** Federate object managing a C++98 Federate object*/
class Federate {
  public:
    /// Default constructor
    Federate() HELICS_NOTHROW: fed(NULL), exec_async_iterate(false) {}
    /// Copy constructor
    Federate(const Federate& fedObj): exec_async_iterate(fedObj.exec_async_iterate)
    {
        fed = helicsFederateClone(fedObj.fed, hThrowOnError());
    }
    /// Copy assignment operator
    Federate& operator=(const Federate& fedObj)
    {
        exec_async_iterate = fedObj.exec_async_iterate;
        fed = helicsFederateClone(fedObj.fed, hThrowOnError());
        return *this;
    }
#ifdef HELICS_HAS_RVALUE_REFS
    /** move constructor*/
    Federate(Federate&& fedObj) HELICS_NOTHROW: exec_async_iterate(fedObj.exec_async_iterate)
    {
        fed = fedObj.fed;
        fedObj.fed = HELICS_NULL_POINTER;
    }
    /** move assignment operator*/
    Federate& operator=(Federate&& fedObj) HELICS_NOTHROW
    {
        exec_async_iterate = fedObj.exec_async_iterate;
        fed = fedObj.fed;
        fedObj.fed = HELICS_NULL_POINTER;
        return *this;
    }
#endif
    /** destructor*/
    virtual ~Federate()
    {
        if (fed != HELICS_NULL_POINTER) {
            helicsFederateFree(fed);
        }
#if defined(HELICS_HAS_FUNCTIONAL) && HELICS_HAS_FUNCTIONAL != 0
        if (callbackBuffer != nullptr) {
            auto cback =
                reinterpret_cast<std::function<std::string(const std::string&)>*>(callbackBuffer);
            delete cback;
        }
#endif
    }
    /** cast operator to get the underlying HelicsFederate object*/
    operator HelicsFederate() const { return fed; }
    /** get the underlying HelicsFederate object*/
    HelicsFederate baseObject() const { return fed; }
    /** set a flag for the federate
   @param flag an index into the flag /ref flag-definitions.h
   @param flagValue the value of the flag defaults to true
   */
    void setFlagOption(int flag, bool flagValue = true)
    {
        helicsFederateSetFlagOption(fed,
                                    flag,
                                    flagValue ? HELICS_TRUE : HELICS_FALSE,
                                    hThrowOnError());
    }
    /**  set a time property option for the federate
    @param tProperty an index of the option to set
    @param timeValue  and integer option value for an integer based property
    */
    void setProperty(int tProperty, HelicsTime timeValue)
    {
        helicsFederateSetTimeProperty(fed, tProperty, timeValue, hThrowOnError());
    }
    /**  set an integer option for the federate
    @param intProperty an index of the option to set
    @param value an integer option value for an integer based property
    */
    void setProperty(int intProperty, int value)
    {
        helicsFederateSetIntegerProperty(fed, intProperty, value, hThrowOnError());
    }

    /** get the value of a flag option
    @param flag an index into the flag /ref flag-definitions.h
    */
    bool getFlagOption(int flag) const
    {
        return (helicsFederateGetFlagOption(fed, flag, hThrowOnError()) != HELICS_FALSE);
    }
    /** get the value of a time option for the federate
    @param tProperty the option to get
    */
    HelicsTime getTimeProperty(int tProperty) const
    {
        return helicsFederateGetTimeProperty(fed, tProperty, hThrowOnError());
    }
    /**  get an integer option for the federate
   @param intProperty  the option to inquire
   */
    int getIntegerProperty(int intProperty) const
    {
        return helicsFederateGetIntegerProperty(fed, intProperty, hThrowOnError());
    }
    /** specify a separator to use for naming separation between the federate name and the interface
    name setSeparator('.') will result in future registrations of local endpoints such as
    fedName.endpoint setSeparator('/') will result in fedName/endpoint the default is '/'  any
    character can be used though many will not make that much sense.  This call is not thread safe
    and should be called before any local interfaces are created otherwise it may not be possible to
    retrieve them without using the full name.  recommended possibilities are ('.','/', ':','-','_')
     */
    void setSeparator(char sep) { helicsFederateSetSeparator(fed, sep, HELICS_NULL_POINTER); }
    /** register a set of interfaces defined in a file
    @details call is only valid in startup mode
    @param configString  the location of the file or config String to load to generate the
    interfaces
    */
    void registerInterfaces(const std::string& configString)
    {
        helicsFederateRegisterInterfaces(fed, configString.c_str(), hThrowOnError());
    }
    /** get the current state of the federate*/
    HelicsFederateState getCurrentMode() const
    {
        return helicsFederateGetState(fed, HELICS_NULL_POINTER);
    }
    /** enter the initialization mode after all interfaces have been defined
    @details  the call will block until all federates have entered initialization mode
    */
    void enterInitializingMode() { helicsFederateEnterInitializingMode(fed, hThrowOnError()); }
    /** enter the initialization mode after all interfaces have been defined
   @details  the call will not block but a call to \ref enterInitializingModeComplete should be made
   to complete the call sequence
   */
    void enterInitializingModeAsync()
    {
        helicsFederateEnterInitializingModeAsync(fed, hThrowOnError());
    }
    /** called after one of the async calls and will indicate true if an async operation has
    completed
    @details only call from the same thread as the one that called the initial async call and will
    return false if called when no async operation is in flight*/
    bool isAsyncOperationCompleted() const
    {
        // returns int, 1 = true, 0 = false
        return helicsFederateIsAsyncOperationCompleted(fed, HELICS_NULL_POINTER) > 0;
    }
    /** second part of the async process for entering initializationState call after a call to
    enterInitializingModeAsync if call any other time it will throw an InvalidFunctionCall
    exception*/
    void enterInitializingModeComplete()
    {
        helicsFederateEnterInitializingModeComplete(fed, hThrowOnError());
    }
    /** enter the normal execution mode
    @details call will block until all federates have entered this mode
    @param iterate an optional flag indicating the desired iteration mode
    */
    HelicsIterationResult
        enterExecutingMode(HelicsIterationRequest iterate = HELICS_ITERATION_REQUEST_NO_ITERATION)
    {
        HelicsIterationResult out_iterate = HELICS_ITERATION_RESULT_NEXT_STEP;
        if (iterate == HELICS_ITERATION_REQUEST_NO_ITERATION) {
            helicsFederateEnterExecutingMode(fed, hThrowOnError());
        } else {
            out_iterate = helicsFederateEnterExecutingModeIterative(fed, iterate, hThrowOnError());
        }
        return out_iterate;
    }
    /** enter the normal execution mode
    @details call will return immediately but \ref enterExecutingModeComplete should be called to
    complete the operation
    @param iterate an optional flag indicating the desired iteration mode
    */
    void enterExecutingModeAsync(
        HelicsIterationRequest iterate = HELICS_ITERATION_REQUEST_NO_ITERATION)
    {
        if (iterate == HELICS_ITERATION_REQUEST_NO_ITERATION) {
            helicsFederateEnterExecutingModeAsync(fed, hThrowOnError());
            exec_async_iterate = false;
        } else {
            helicsFederateEnterExecutingModeIterativeAsync(fed, iterate, hThrowOnError());
            exec_async_iterate = true;
        }
    }

    /** complete the async call for entering Execution state
    @details call will not block but will return quickly.  The enterInitializingModeComplete must be
    called before doing other operations
    */
    HelicsIterationResult enterExecutingModeComplete()
    {
        HelicsIterationResult out_iterate = HELICS_ITERATION_RESULT_NEXT_STEP;
        if (exec_async_iterate) {
            out_iterate = helicsFederateEnterExecutingModeIterativeComplete(fed, hThrowOnError());
        } else {
            helicsFederateEnterExecutingModeComplete(fed, hThrowOnError());
        }
        return out_iterate;
    }
    /** terminate the simulation
   @details call is will block until the finalize has been acknowledged, no commands that interact
   with the core may be called after this function function */
    void finalize() { helicsFederateFinalize(fed, hThrowOnError()); }
    /** terminate the simulation in a non-blocking call
    @details finalizeComplete must be called after this call to complete the finalize procedure*/
    void finalizeAsync() { helicsFederateFinalizeAsync(fed, hThrowOnError()); }
    /** complete the asynchronous terminate pair*/
    void finalizeComplete() { helicsFederateFinalizeComplete(fed, hThrowOnError()); }
    /** get the current time from a federate */
    HelicsTime getCurrentTime() { return helicsFederateGetCurrentTime(fed, hThrowOnError()); }
    /** request a time advancement
   @param time the next requested time step
   @return the granted time step*/
    HelicsTime requestTime(HelicsTime time)
    {
        return helicsFederateRequestTime(fed, time, hThrowOnError());
    }
    /** request a time advancement to the next allowed time
    @return the granted time step*/
    HelicsTime requestNextStep() { return helicsFederateRequestNextStep(fed, hThrowOnError()); }

    /** request a time advancement to the next allowed time
    @param timeDelta the amount of time requested to advance
    @return the granted time step*/
    HelicsTime requestTimeAdvance(HelicsTime timeDelta)
    {
        return helicsFederateRequestTimeAdvance(fed, timeDelta, hThrowOnError());
    }
    /** request a time advancement
   @param time the next requested time step
   @param iterate a requested iteration mode
   @return the granted time step in a structure containing a return time and an IterationResult*/
    HelicsIterationTime requestTimeIterative(HelicsTime time, HelicsIterationRequest iterate)
    {
        HelicsIterationTime itTime;
        itTime.grantedTime = helicsFederateRequestTimeIterative(
            fed, time, iterate, &(itTime.status), hThrowOnError());
        return itTime;
    }
    /**  request a time advancement and return immediately for asynchronous function.
    @details /ref requestTimeComplete should be called to finish the operation and get the result
    @param time the next requested time step
    */
    void requestTimeAsync(HelicsTime time)
    {
        helicsFederateRequestTimeAsync(fed, time, hThrowOnError());
    }

    /** request a time advancement with iterative call and return for asynchronous function.
  @details /ref requestTimeIterativeComplete should be called to finish the operation and get the
  result
  @param time the next requested time step
  @param iterate a requested iteration level (none, require, optional)
  */
    void requestTimeIterativeAsync(HelicsTime time, HelicsIterationRequest iterate)
    {
        helicsFederateRequestTimeIterativeAsync(fed, time, iterate, hThrowOnError());
    }

    /** request a time advancement
   @return the granted time step*/
    HelicsTime requestTimeComplete()
    {
        return helicsFederateRequestTimeComplete(fed, hThrowOnError());
    }

    /** finalize the time advancement request
  @return the granted time step in an iteration_time structure which contains a time and iteration
  result*/
    HelicsIterationTime requestTimeIterativeComplete()
    {
        HelicsIterationTime itTime;
        itTime.grantedTime =
            helicsFederateRequestTimeIterativeComplete(fed, &(itTime.status), hThrowOnError());
        return itTime;
    }
    /** get the federate name*/
    const char* getName() const { return helicsFederateGetName(fed); }

    /** make a query of the federate
    @details this call is blocking until the value is returned which make take some time depending
    on the size of the federation and the specific string being queried
    @param target  the target of the query can be "federation", "federate", "broker", "core", or a
    specific name of a federate, core, or broker
    @param queryStr a string with the query see other documentation for specific properties to
    query, can be defined by the federate
    @param mode the ordering mode to use for the query (fast for priority channels, ordered for
    normal channels ordered with all other messages)
    @return a string with the value requested.  this is either going to be a vector of strings value
    or a JSON string stored in the first element of the vector.  The string "#invalid" is returned
    if the query was not valid
    */
    std::string query(const std::string& target,
                      const std::string& queryStr,
                      HelicsSequencingModes mode = HELICS_SEQUENCING_MODE_FAST) const
    {
        // returns HelicsQuery
        HelicsQuery q = helicsCreateQuery(target.c_str(), queryStr.c_str());
        if (mode != HELICS_SEQUENCING_MODE_FAST) {
            helicsQuerySetOrdering(q, mode, HELICS_IGNORE_ERROR);
        }
        std::string result(helicsQueryExecute(q, fed, hThrowOnError()));
        helicsQueryFree(q);
        return result;
    }

    /** make a query of the federate
    @details this call is blocking until the value is returned which may take some time depending
    on the size of the federation and the specific string being queried, query without a target
    assumes the target is the federate

    @param queryStr a string with the query, see other documentation for specific properties to
    query, can be defined by the federate
    @param mode the ordering mode to use for the query (fast for priority channels, ordered for
    normal channels ordered with all other messages)
    @return a string with the value requested.  this is either going to be a vector of strings value
    or a JSON string stored in the first element of the vector.  The string "#invalid" is returned
    if the query was not valid
    */
    std::string query(const std::string& queryStr,
                      HelicsSequencingModes mode = HELICS_SEQUENCING_MODE_FAST) const
    {
        // returns HelicsQuery
        HelicsQuery q = helicsCreateQuery(HELICS_NULL_POINTER, queryStr.c_str());
        if (mode != HELICS_SEQUENCING_MODE_FAST) {
            helicsQuerySetOrdering(q, mode, HELICS_IGNORE_ERROR);
        }
        std::string result(helicsQueryExecute(q, fed, hThrowOnError()));
        helicsQueryFree(q);
        return result;
    }

    void setQueryCallback(
        void (*queryAnswer)(const char* query, int querySize, HelicsQueryBuffer, void* userdata),
        void* userdata)

    {
        helicsFederateSetQueryCallback(fed, queryAnswer, userdata, hThrowOnError());
    }

#if defined(HELICS_HAS_FUNCTIONAL) && HELICS_HAS_FUNCTIONAL != 0
    void setQueryCallback(std::function<std::string(const std::string&)> callback)

    {
        callbackBuffer = new std::function<std::string(const std::string&)>(std::move(callback));
        helicsFederateSetQueryCallback(fed,
                                       details::helicCppQueryCallbackExecutor,
                                       callbackBuffer,
                                       hThrowOnError());
    }

#endif
    /** define a filter interface
    @details a filter will modify messages coming from or going to target endpoints
    @param type the type of the filter to register
    @param filterName the name of the filter
    */
    Filter registerFilter(HelicsFilterTypes type, const std::string& filterName = std::string())
    {
        return Filter(helicsFederateRegisterFilter(fed, type, filterName.c_str(), hThrowOnError()));
    }

    /** create a cloning Filter on the specified federate
    @details cloning filters copy a message and send it to multiple locations source and destination
    can be added through other functions
    @param deliveryEndpoint the specified endpoint to deliver the message
    @return a HelicsFilter object
    */
    CloningFilter registerCloningFilter(const std::string& deliveryEndpoint)
    {
        return CloningFilter(
            helicsFederateRegisterCloningFilter(fed, deliveryEndpoint.c_str(), hThrowOnError()));
    }
    /** define a filter interface
  @details a filter will modify messages coming from or going to target endpoints
  @param type the type of the filter to register
  @param filterName the name of the filter
  */
    Filter registerGlobalFilter(HelicsFilterTypes type,
                                const std::string& filterName = std::string())
    {
        return Filter(
            helicsFederateRegisterGlobalFilter(fed, type, filterName.c_str(), hThrowOnError()));
    }

    /** create a cloning Filter on the specified federate
    @details cloning filters copy a message and send it to multiple locations source and destination
    can be added through other functions
    @param deliveryEndpoint the specified endpoint to deliver the message
    @return a CloningFilter object
    */
    CloningFilter registerGlobalCloningFilter(const std::string& deliveryEndpoint)
    {
        return CloningFilter(helicsFederateRegisterGlobalCloningFilter(fed,
                                                                       deliveryEndpoint.c_str(),
                                                                       hThrowOnError()));
    }
    /** get the id of a source filter from the name of the endpoint
    @param filterName the name of the filter
    @return a reference to a filter object which could be invalid if filterName is not valid*/
    Filter getFilter(const std::string& filterName)
    {
        return Filter(helicsFederateGetFilter(fed, filterName.c_str(), hThrowOnError()));
    }
    /** get a filter from its index
    @param index the index of a filter
    @return a reference to a filter object which could be invalid if filterName is not valid*/
    Filter getFilter(int index)
    {
        return Filter(helicsFederateGetFilterByIndex(fed, index, hThrowOnError()));
    }

    /** set a federation global value
    @details this overwrites any previous value for this name
    @param valueName the name of the global to set
    @param value the value of the global
    */
    void setGlobal(const std::string& valueName, const std::string& value)
    {
        helicsFederateSetGlobal(fed, valueName.c_str(), value.c_str(), hThrowOnError());
    }

    /** set a tag (key-value pair) for a federate
    @details the tag is an arbitrary user defined string and value; the tags for a federate are
    queryable through a "tags" query or "tag/<tagname>"
    @param tag the name of the tag to set the value for
    @param value the value for the given tag*/
    void setTag(const std::string& tag, const std::string& value)
    {
        helicsFederateSetTag(fed, tag.c_str(), value.c_str(), hThrowOnError());
    }
    /** get the value of a specific tag (key-value pair) for a federate
    @details the tag is an arbitrary user defined string and value; the tags for a federate are
    queryable
    @param tag the name of the tag to get the value for
    @return a const char * containing the text value of the tag, if the tag is not defined the
    value is a length 0 string*/
    const char* getTag(const std::string& tag) const
    {
        return helicsFederateGetTag(fed, tag.c_str(), hThrowOnError());
    }

    /** add a dependency for this federate
     @details adds an additional internal time dependency for the federate
     @param fedName the name of the federate to add a dependency on
     */
    void addDependency(const std::string& fedName)
    {
        helicsFederateAddDependency(fed, fedName.c_str(), hThrowOnError());
    }

    /** generate a local federate error
    @param errorCode an error code to give to the error
    @param errorString a string message associated with the error
    */
    void localError(int errorCode, const std::string& errorString)
    {
        helicsFederateLocalError(fed, errorCode, errorString.c_str(), hThrowOnError());
    }

    /** generate a global error to terminate the federation
    @param errorCode an error code to give to the error
    @param errorString a string message associated with the error
    */
    void globalError(int errorCode, const std::string& errorString)
    {
        helicsFederateGlobalError(fed, errorCode, errorString.c_str(), hThrowOnError());
    }

    /** log an error message*/
    void logErrorMessage(const std::string& message)
    {
        helicsFederateLogErrorMessage(fed, message.c_str(), hThrowOnError());
    }
    /** log a warning message*/
    void logWarningMessage(const std::string& message)
    {
        helicsFederateLogWarningMessage(fed, message.c_str(), hThrowOnError());
    }
    /** log an info message*/
    void logInfoMessage(const std::string& message)
    {
        helicsFederateLogInfoMessage(fed, message.c_str(), hThrowOnError());
    }
    /** log a debug message*/
    void logDebugMessage(const std::string& message)
    {
        helicsFederateLogDebugMessage(fed, message.c_str(), hThrowOnError());
    }
    /** log a message with a specified log level*/
    void logMessage(int level, const std::string& message)
    {
        helicsFederateLogLevelMessage(fed, level, message.c_str(), hThrowOnError());
    }
    /** get a Core Object*/
    HelicsCore getCore() { return helicsFederateGetCore(fed, hThrowOnError()); }
    /** get the C object for use in the C library*/
    HelicsFederate getObject() const { return fed; }

  protected:
    HelicsFederate fed;  //!< underlying HelicsFederate object
    bool exec_async_iterate;  //!< indicator that the federate is in an async operation
#if defined(HELICS_HAS_FUNCTIONAL) && HELICS_HAS_FUNCTIONAL != 0
  private:
    void* callbackBuffer{nullptr};  //!< buffer to contain pointer to a callback
#endif
};

}  // namespace helicscpp

#endif
