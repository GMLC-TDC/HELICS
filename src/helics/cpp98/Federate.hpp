/*
Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/
#ifndef HELICS_CPP98_FEDERATE_HPP_
#define HELICS_CPP98_FEDERATE_HPP_
#pragma once

#include "shared_api_library/helics.h"

#include <string>
#include <vector>
#include <complex>
#include <stdexcept>

// defines for setFlag values in core/flag-definitions.h
// enum for core_type:int in core/core-types.h

namespace helics
{
class FederateInfo
{
  public:
    FederateInfo ()
    {
        fi = helicsFederateInfoCreate ();
    }

    explicit FederateInfo (const std::string &fedname)
    {
        fi = helicsFederateInfoCreate ();
        helicsFederateInfoSetFederateName (fi, fedname.c_str());
    }

    FederateInfo (const std::string &fedname, const std::string &coretype)
    {
        fi = helicsFederateInfoCreate ();
        helicsFederateInfoSetFederateName (fi, fedname.c_str());
        helicsFederateInfoSetCoreTypeFromString (fi, coretype.c_str());
    }

    ~FederateInfo ()
    {
        helicsFederateInfoFree (fi);
    }

    void setFederateName (const std::string &name)
    {
        helicsFederateInfoSetFederateName (fi, name.c_str());
    }

    void setCoreName (const std::string &corename)
    {
        helicsFederateInfoSetCoreName (fi, corename.c_str());
    }

    void setCoreInitString (const std::string &coreInit)
    {
        helicsFederateInfoSetCoreInitString (fi, coreInit.c_str());
    }

    void setCoreTypeFromString (const std::string &coretype)
    {
        helicsFederateInfoSetCoreTypeFromString (fi, coretype.c_str());
    }

    void setCoreType (int coretype)
    {
        helicsFederateInfoSetCoreType (fi, coretype);
    }

    void setFlag (int flag, int value)
    {
        helicsFederateInfoSetFlag (fi, flag, value);
    }

    void setOutputDelay (helics_time_t outputDelay)
    {
        helicsFederateInfoSetOutputDelay (fi, outputDelay);
    }

    void setTimeDelta (helics_time_t timeDelta)
    {
        helicsFederateInfoSetTimeDelta (fi, timeDelta);
    }

    void setInputDelay (helics_time_t inputDelay)
    {
        helicsFederateInfoSetInputDelay (fi, inputDelay);
    }

    void setTimeOffset (helics_time_t timeOffset)
    {
        helicsFederateInfoSetTimeOffset (fi, timeOffset);
    }

    void setPeriod (helics_time_t period)
    {
        helicsFederateInfoSetPeriod (fi, period);
    }

    void setMaxIterations (int max_iterations)
    {
        helicsFederateInfoSetMaxIterations (fi, max_iterations);
    }

    void setLoggingLevel (int logLevel)
    {
        helicsFederateInfoSetLoggingLevel (fi, logLevel);
    }

    helics_federate_info_t getInfo ()
    {
        return fi;
    }
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
    helics_time_t grantedTime; //!< the time of the granted step
    helics_iteration_status status;	//!< the convergence state
                            /** default constructor*/
}helics_iteration_time;

class Federate
{
  public:
    // Default constructor, not meant to be used
    Federate ():fed(nullptr),exec_async_iterate(false) {};

    virtual ~Federate ()
    {
        helicsFederateFree (fed);
    }

    void enterInitializationState ()
    {
        if (helics_ok != helicsFederateEnterInitializationMode (fed))
        {
            throw (InvalidStateTransition ("cannot transition from current state to initialization state"));
        }
    }

    void enterInitializationStateAsync ()
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

    void enterInitializationStateComplete ()
    {
        if (helics_ok != helicsFederateEnterInitializationModeComplete (fed))
        {
            throw (InvalidFunctionCall ("cannot call finalize function without first calling async function"));
        }
    }

    helics_iteration_status enterExecutionState (helics_iteration_request iterate = no_iteration)
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

    void enterExecutionStateAsync (helics_iteration_request iterate = no_iteration)
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

    helics_iteration_status enterExecutionStateComplete ()
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

    void finalize ()
    {
        helicsFederateFinalize (fed);
    }

    helics_time_t requestTime (helics_time_t time)
    {
        helics_time_t grantedTime;
        helicsFederateRequestTime (fed, time,&grantedTime);
        return grantedTime;
    }

    helics_iteration_time requestTimeIterative (helics_time_t time, helics_iteration_request iterate)
    {
        helics_iteration_time itTime;
        helicsFederateRequestTimeIterative (fed, time, iterate,&(itTime.grantedTime),&(itTime.status));
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
        helicsFederateRequestTimeComplete (fed,&newTime);
        return newTime;
    }

    helics_iteration_time requestTimeIterativeComplete ()
    {
        helics_iteration_time itTime;
        helicsFederateRequestTimeIterativeComplete(fed, &(itTime.grantedTime), &(itTime.status));
        return itTime;
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
        helics_query q = helicsCreateQuery (target.c_str(), queryStr.c_str());
        std::string result (helicsQueryExecute(q,fed));
        helicsQueryFree(q);
        return result;
    }

  protected:
    helics_federate fed;
    bool exec_async_iterate;
};

} //namespace helics
#endif
