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

    FederateInfo (std::string fedname)
    {
        fi = helicsFederateInfoCreate ();
        helicsFederateInfoSetFederateName (fi, fedname.c_str());
    }

    FederateInfo (std::string fedname, std::string coretype)
    {
        fi = helicsFederateInfoCreate ();
        helicsFederateInfoSetFederateName (fi, fedname.c_str());
        helicsFederateInfoSetCoreTypeFromString (fi, coretype.c_str());
    }

    ~FederateInfo ()
    {
        helicsFederateInfoFree (fi);
    }

    void setFederateName (std::string name)
    {
        helicsFederateInfoSetFederateName (fi, name.c_str());
    }

    void setCoreName (std::string corename)
    {
        helicsFederateInfoSetCoreName (fi, corename.c_str());
    }

    void setCoreInitString (std::string coreInit)
    {
        helicsFederateInfoSetCoreInitString (fi, coreInit.c_str());
    }

    void setCoreTypeFromString (std::string coretype)
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
    InvalidStateTransition (const char *s) : std::runtime_error (s) {}
};

/** defining an exception class for invalid function calls*/
class InvalidFunctionCall : public std::runtime_error
{
  public:
    InvalidFunctionCall (const char *s) : std::runtime_error (s) {}
};
/** defining an exception class for invalid parameter values*/
class InvalidParameterValue : public std::runtime_error
{
  public:
    InvalidParameterValue (const char *s) : std::runtime_error (s) {}
};

class Federate
{
  public:
    // Default constructor, not meant to be used
    Federate () {};

    virtual ~Federate ()
    {
        helicsFreeFederate (fed);
    }

    void enterInitializationState ()
    {
        if (helics_ok != helicsEnterInitializationMode (fed))
        {
            throw (InvalidStateTransition ("cannot transition from current state to initialization state"));
        }
    }

    void enterInitializationStateAsync ()
    {
        if (helics_ok != helicsEnterInitializationModeAsync (fed))
        {
            throw (InvalidStateTransition ("cannot transition from current state to initialization state"));
        }
    }

    bool isAsyncOperationCompleted () const
    {
        // returns int, 1 = true, 0 = false
        return helicsIsAsyncOperationCompleted (fed) > 0;
    }

    void enterInitializationStateComplete ()
    {
        if (helics_ok != helicsEnterInitializationModeComplete (fed))
        {
            throw (InvalidFunctionCall ("cannot call finalize function without first calling async function"));
        }
    }

    iteration_status enterExecutionState (iteration_request iterate = no_iteration)
    {
        iteration_status out_iterate = next_step;
        if (iterate == no_iteration)
        {
            helicsEnterExecutionMode (fed);
        }
        else
        {
            helicsEnterExecutionModeIterative (fed, iterate, &out_iterate);
        }
        return out_iterate;
    }

    void enterExecutionStateAsync (iteration_request iterate = no_iteration)
    {
        if (iterate == no_iteration)
        {
            helicsEnterExecutionModeAsync (fed);
            exec_async_iterate = false;
        }
        else
        {
            helicsEnterExecutionModeIterativeAsync (fed, iterate);
            exec_async_iterate = true;
        }
    }

    iteration_status enterExecutionStateComplete ()
    {
        iteration_status out_iterate = next_step;
        if (exec_async_iterate)
        {
            helicsEnterExecutionModeIterativeComplete (fed, &out_iterate);
        }
        else
        {
            helicsEnterExecutionModeComplete (fed);
        }
        return out_iterate;
    }

    void finalize ()
    {
        helicsFederateFinalize (fed);
    }

    helics_time_t requestTime (helics_time_t time)
    {
        return helicsRequestTime (fed, time);
    }

    helics_iterative_time requestTimeIterative (helics_time_t time, iteration_request iterate)
    {
        return helicsRequestTimeIterative (fed, time, iterate);
    }

    void requestTimeAsync (helics_time_t time)
    {
        if (helics_ok != helicsRequestTimeAsync (fed, time))
        {
            throw (InvalidFunctionCall ("cannot call request time in present state"));
        }
    }

    void requestTimeIterativeAsync (helics_time_t time, iteration_request iterate)
    {
        helicsRequestTimeIterativeAsync (fed, time, iterate);
    }

    helics_time_t requestTimeComplete ()
    {
        return helicsRequestTimeComplete (fed);
    }

    helics_iterative_time requestTimeIterativeFinalize ()
    {
        return helicsRequestTimeIterativeComplete (fed);
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
        std::string result (helicsExecuteQuery(fed, q));
        helicsFreeQuery (q);
        return result;
    }

  protected:
    helics_federate fed;
    bool exec_async_iterate;
};

} //namespace helics
#endif
