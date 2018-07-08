/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#ifndef HELICS_CPP98_FEDERATE_HPP_
#define HELICS_CPP98_FEDERATE_HPP_
#pragma once

#include "../shared_api_library/MessageFilters.h"
#include "../shared_api_library/helics.h"
#include "Filter.hpp"
#include "config.hpp"
#include <complex>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>

// defines for setFlag values in core/flag-definitions.h
// enum for core_type:int in core/core-types.h

namespace helics
{
class FederateInfo
{
  public:
    FederateInfo () { fi = helicsFederateInfoCreate (); }

    explicit FederateInfo (const std::string &fedname)
    {
        fi = helicsFederateInfoCreate ();
        helicsFederateInfoSetFederateName (fi, fedname.c_str ());
    }

    FederateInfo (const std::string &fedname, const std::string &coretype)
    {
        fi = helicsFederateInfoCreate ();
        helicsFederateInfoSetFederateName (fi, fedname.c_str ());
        helicsFederateInfoSetCoreTypeFromString (fi, coretype.c_str ());
    }

    ~FederateInfo () { helicsFederateInfoFree (fi); }

    void setFederateName (const std::string &name) { helicsFederateInfoSetFederateName (fi, name.c_str ()); }

    void setCoreName (const std::string &corename) { helicsFederateInfoSetCoreName (fi, corename.c_str ()); }

    void setCoreInitString (const std::string &coreInit)
    {
        helicsFederateInfoSetCoreInitString (fi, coreInit.c_str ());
    }

    void setCoreTypeFromString (const std::string &coretype)
    {
        helicsFederateInfoSetCoreTypeFromString (fi, coretype.c_str ());
    }

    void setCoreType (int coretype) { helicsFederateInfoSetCoreType (fi, coretype); }

    void setFlag (int flag, int value) { helicsFederateInfoSetFlag (fi, flag, value); }

    void setOutputDelay (helics_time_t outputDelay) { helicsFederateInfoSetOutputDelay (fi, outputDelay); }

    void setTimeDelta (helics_time_t timeDelta) { helicsFederateInfoSetTimeDelta (fi, timeDelta); }

    void setInputDelay (helics_time_t inputDelay) { helicsFederateInfoSetInputDelay (fi, inputDelay); }

    void setTimeOffset (helics_time_t timeOffset) { helicsFederateInfoSetTimeOffset (fi, timeOffset); }

    void setPeriod (helics_time_t period) { helicsFederateInfoSetPeriod (fi, period); }

    void setMaxIterations (int max_iterations) { helicsFederateInfoSetMaxIterations (fi, max_iterations); }

    void setLoggingLevel (int logLevel) { helicsFederateInfoSetLoggingLevel (fi, logLevel); }

    helics_federate_info_t getInfo () { return fi; }

  private:
    helics_federate_info_t fi;
};

/** defining an exception class for state transition errors*/
class InvalidStateTransition : public std::runtime_error
{
  public:
    explicit InvalidStateTransition (const char *s) : std::runtime_error (s) {}
};

/** defining an exception class for invalid function calls*/
class InvalidFunctionCall : public std::runtime_error
{
  public:
    explicit InvalidFunctionCall (const char *s) : std::runtime_error (s) {}
};
/** defining an exception class for invalid parameter values*/
class InvalidParameterValue : public std::runtime_error
{
  public:
    explicit InvalidParameterValue (const char *s) : std::runtime_error (s) {}
};

typedef struct
{
  public:
    helics_time_t grantedTime;  //!< the time of the granted step
    helics_iteration_status status;  //!< the convergence state
    /** default constructor*/
} helics_iteration_time;

class Federate
{
  public:
    // Default constructor, not meant to be used
      Federate() : fed(NULL), exec_async_iterate(false) { std::cout << "def fed constructor" << std::endl; };

    Federate (const Federate &fedObj) : exec_async_iterate (fedObj.exec_async_iterate)
    {
        std::cout << "fed copy constructor" << std::endl;
        fed = helicsFederateClone (fedObj.fed);
        std::cout << "fed cloned " <<fed<< std::endl;
    }
    Federate &operator= (const Federate &fedObj)
    {
        std::cout << "fed assign" << std::endl;
        exec_async_iterate = fedObj.exec_async_iterate;
        fed = helicsFederateClone (fedObj.fed);
        std::cout << "fed assigned" <<fed<< std::endl;
        return *this;
    }
#ifdef HELICS_HAS_RVALUE_REFS
    Federate (Federate &&fedObj) : exec_async_iterate (fedObj.exec_async_iterate)
    {
        std::cout << "fed move constructor" << std::endl;
        fed = fedObj.fed;
        fedObj.fed = NULL;
        std::cout << "fed moved" << fed << std::endl;
    }
    Federate &operator= (Federate &&fedObj)
    {
        std::cout << "fed move assign" << std::endl;
        exec_async_iterate = fedObj.exec_async_iterate;
        fed = fedObj.fed;
        fedObj.fed = NULL;
        std::cout << "fed move assigned" << fed << std::endl;
        return *this;
    }
#endif
    virtual ~Federate() {
        if (fed != NULL)
        {
            std::cout << "fed destructor" <<fed<< std::endl;
            helicsFederateFree(fed);
            std::cout << "fed freed" << fed << std::endl;
        }
    }

    operator helics_federate () const { return fed; }

    helics_federate baseObject () const { return fed; }
    void setFlag (int flag, int value) { helicsFederateSetFlag (fed, flag, value); }

    void setOutputDelay (helics_time_t outputDelay) { helicsFederateSetOutputDelay (fed, outputDelay); }

    void setTimeDelta (helics_time_t timeDelta) { helicsFederateSetTimeDelta (fed, timeDelta); }

    void setInputDelay (helics_time_t inputDelay) { helicsFederateSetInputDelay (fed, inputDelay); }

    void setPeriod (helics_time_t period, helics_time_t offset) { helicsFederateSetPeriod (fed, period, offset); }

    void setMaxIterations (int max_iterations) { helicsFederateSetMaxIterations (fed, max_iterations); }

    void setLoggingLevel (int logLevel) { helicsFederateInfoSetLoggingLevel (fed, logLevel); }

    federate_state getState() const
    {
        federate_state fedState;
        helicsFederateGetState(fed, &fedState);
        return fedState;
    }

    void enterInitializationMode ()
    {
        if (helics_ok != helicsFederateEnterInitializationMode (fed))
        {
            throw (InvalidStateTransition ("cannot transition from current state to initialization state"));
        }
    }

    void enterInitializationModeAsync ()
    {
        if (helics_ok != helicsFederateEnterInitializationModeAsync (fed))
        {
            throw (InvalidStateTransition ("cannot transition from current state to initialization state"));
        }
    }

    bool isAsyncOperationCompleted () const
    {
        // returns int, 1 = true, 0 = false
        return helicsFederateIsAsyncOperationCompleted (fed) > 0;
    }

    void enterInitializationModeComplete ()
    {
        if (helics_ok != helicsFederateEnterInitializationModeComplete (fed))
        {
            throw (InvalidFunctionCall ("cannot call finalize function without first calling async function"));
        }
    }

    helics_iteration_status enterExecutionMode (helics_iteration_request iterate = no_iteration)
    {
        helics_iteration_status out_iterate = next_step;
        if (iterate == no_iteration)
        {
            helicsFederateEnterExecutionMode (fed);
        }
        else
        {
            helicsFederateEnterExecutionModeIterative (fed, iterate, &out_iterate);
        }
        return out_iterate;
    }

    void enterExecutionModeAsync (helics_iteration_request iterate = no_iteration)
    {
        if (iterate == no_iteration)
        {
            helicsFederateEnterExecutionModeAsync (fed);
            exec_async_iterate = false;
        }
        else
        {
            helicsFederateEnterExecutionModeIterativeAsync (fed, iterate);
            exec_async_iterate = true;
        }
    }

    helics_iteration_status enterExecutionModeComplete ()
    {
        helics_iteration_status out_iterate = next_step;
        if (exec_async_iterate)
        {
            helicsFederateEnterExecutionModeIterativeComplete (fed, &out_iterate);
        }
        else
        {
            helicsFederateEnterExecutionModeComplete (fed);
        }
        return out_iterate;
    }

    void finalize () { helicsFederateFinalize (fed); }

    helics_time_t requestTime (helics_time_t time)
    {
        helics_time_t grantedTime;
        helicsFederateRequestTime (fed, time, &grantedTime);
        return grantedTime;
    }

    helics_iteration_time requestTimeIterative (helics_time_t time, helics_iteration_request iterate)
    {
        helics_iteration_time itTime;
        helicsFederateRequestTimeIterative (fed, time, iterate, &(itTime.grantedTime), &(itTime.status));
        return itTime;
    }

    void requestTimeAsync (helics_time_t time)
    {
        if (helics_ok != helicsFederateRequestTimeAsync (fed, time))
        {
            throw (InvalidFunctionCall ("cannot call request time in present state"));
        }
    }

    void requestTimeIterativeAsync (helics_time_t time, helics_iteration_request iterate)
    {
        helicsFederateRequestTimeIterativeAsync (fed, time, iterate);
    }

    helics_time_t requestTimeComplete ()
    {
        helics_time_t newTime;
        helicsFederateRequestTimeComplete (fed, &newTime);
        return newTime;
    }

    helics_iteration_time requestTimeIterativeComplete ()
    {
        helics_iteration_time itTime;
        helicsFederateRequestTimeIterativeComplete (fed, &(itTime.grantedTime), &(itTime.status));
        return itTime;
    }

    std::string getName () const
    {
        char str[255];
        helicsFederateGetName (fed, &str[0], sizeof (str));
        std::string result (str);
        return result;
    }
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
    std::string query (const std::string &target, const std::string &queryStr)
    {
        // returns helics_query
        helics_query q = helicsCreateQuery (target.c_str (), queryStr.c_str ());
        std::string result (helicsQueryExecute (q, fed));
        helicsQueryFree (q);
        return result;
    }

    Filter registerSourceFilter (helics_filter_type_t type,
                                 const std::string &target,
                                 const std::string &name = std::string ())
    {
        return Filter (helicsFederateRegisterSourceFilter (fed, type, target.c_str (), name.c_str ()));
    }

    /** create a destination Filter on the specified federate
    @details filters can be created through a federate or a core , linking through a federate allows
    a few extra features of name matching to function on the federate interface but otherwise equivalent behavior
    @param fed the fed to register through
    @param type the type of filter to create
    @param target the name of endpoint to target
    @param name the name of the filter (can be NULL)
    @return a helics_filter object
    */
    Filter registerDestinationFilter (helics_filter_type_t type,
                                      const std::string &target,
                                      const std::string &name = std::string ())
    {
        return Filter (helicsFederateRegisterDestinationFilter (fed, type, target.c_str (), name.c_str ()));
    }

    /** create a cloning Filter on the specified federate
    @details cloning filters copy a message and send it to multiple locations source and destination can be added
    through other functions
    @param fed the fed to register through
    @param deliveryEndpoint the specified endpoint to deliver the message
    @return a helics_filter object
    */
    CloningFilter registerCloningFilter (const std::string &deliveryEndpoint)
    {
        return CloningFilter (helicsFederateRegisterCloningFilter (fed, deliveryEndpoint.c_str ()));
    }

  protected:
    helics_federate fed;
    bool exec_async_iterate;
};

}  // namespace helics
#endif
