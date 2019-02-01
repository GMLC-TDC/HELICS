/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_FEDERATE_HPP_
#define HELICS_CPP98_FEDERATE_HPP_
#pragma once

#include "../shared_api_library/MessageFilters.h"
#include "../shared_api_library/helics.h"
#include "Filter.hpp"
#include "config.hpp"
#include "helicsExceptions.hpp"
#include <complex>

#include <string>
#include <vector>

namespace helicscpp
{
/** hold federate information in the C++98 API*/
class FederateInfo
{
  public:
    FederateInfo () { fi = helicsCreateFederateInfo (); }

    FederateInfo (const std::string &coretype)
    {
        fi = helicsCreateFederateInfo ();
        helicsFederateInfoSetCoreTypeFromString (fi, coretype.c_str (), hThrowOnError ());
    }
    /** copy constructor for federate info*/
    FederateInfo (const FederateInfo &fedInfo) { fi = helicsFederateClone (fedInfo.fi, hThrowOnError ()); }
    /** copy assignment for federateInfo*/
    FederateInfo &operator= (const FederateInfo &fedInfo)
    {
        helics_federate_info fi_new = helicsFederateClone (fedInfo.fi, hThrowOnError ());
        helicsFederateInfoFree (fi);
        fi = fi_new;
        return *this;
    }
#ifdef HELICS_HAS_RVALUE_REFS
    /** move constructor for federateInfo*/
    FederateInfo (FederateInfo &&fedInfo)
    {
        fi = fedInfo.fi;
        fedInfo.fi = NULL;
    }
    /** move assignment for federateInfo*/
    FederateInfo &operator= (FederateInfo &&fedInfo)
    {
        helicsFederateInfoFree (fi);
        fi = fedInfo.fi;
        fedInfo.fi = NULL;
        return *this;
    }
#endif

    ~FederateInfo () { helicsFederateInfoFree (fi); }

    void setCoreName (const std::string &corename) { helicsFederateInfoSetCoreName (fi, corename.c_str (), NULL); }

    void setCoreInitString (const std::string &coreInit)
    {
        helicsFederateInfoSetCoreInitString (fi, coreInit.c_str (), NULL);
    }

    void setCoreTypeFromString (const std::string &coretype)
    {
        helicsFederateInfoSetCoreTypeFromString (fi, coretype.c_str (), hThrowOnError ());
    }

    void setCoreType (int coretype) { helicsFederateInfoSetCoreType (fi, coretype, NULL); }

    void setFlagOption (int flag, int value) { helicsFederateInfoSetFlagOption (fi, flag, value, NULL); }

    void setProperty (int timeProperty, helics_time timeValue)
    {
        helicsFederateInfoSetTimeProperty (fi, timeProperty, timeValue, NULL);
    }

    void setProperty (int integerProperty, int propertyValue)
    {
        helicsFederateInfoSetIntegerProperty (fi, integerProperty, propertyValue, NULL);
    }

    helics_federate_info getInfo () { return fi; }

  private:
    helics_federate_info fi;  //!< handle for the underlying federate_info object
};

/** an iteration time structure */
typedef struct
{
  public:
    helics_time grantedTime;  //!< the time of the granted step
    helics_iteration_result status;  //!< the convergence state
} helics_iteration_time;

/** Federate object managing a C++98 Federate object*/
class Federate
{
  public:
    // Default constructor
    Federate () : fed (NULL), exec_async_iterate (false){};

    Federate (const Federate &fedObj) : exec_async_iterate (fedObj.exec_async_iterate)
    {
        fed = helicsFederateClone (fedObj.fed, hThrowOnError ());
    }
    Federate &operator= (const Federate &fedObj)
    {
        exec_async_iterate = fedObj.exec_async_iterate;
        fed = helicsFederateClone (fedObj.fed, hThrowOnError ());
        return *this;
    }
#ifdef HELICS_HAS_RVALUE_REFS
    Federate (Federate &&fedObj) : exec_async_iterate (fedObj.exec_async_iterate)
    {
        fed = fedObj.fed;
        fedObj.fed = NULL;
    }
    Federate &operator= (Federate &&fedObj)
    {
        exec_async_iterate = fedObj.exec_async_iterate;
        fed = fedObj.fed;
        fedObj.fed = NULL;
        return *this;
    }
#endif
    virtual ~Federate ()
    {
        if (fed != NULL)
        {
            helicsFederateFree (fed);
        }
    }

    operator helics_federate () const { return fed; }

    helics_federate baseObject () const { return fed; }

    void setFlag (int flag, bool value)
    {
        helicsFederateSetFlagOption (fed, flag, value ? helics_true : helics_false, hThrowOnError ());
    }

    void setProperty (int tProperty, helics_time timeValue)
    {
        helicsFederateSetTimeProperty (fed, tProperty, timeValue, hThrowOnError ());
    }

    void setProperty (int intProperty, int value)
    {
        helicsFederateSetIntegerProperty (fed, intProperty, value, hThrowOnError ());
    }

    bool getFlag (int flag) const
    {
        return (helicsFederateGetFlagOption (fed, flag, hThrowOnError ()) != helics_false);
    }

    helics_time getTimeProperty (int tProperty) const
    {
        return helicsFederateGetTimeProperty (fed, tProperty, hThrowOnError ());
    }

    void registerInterfaces (const std::string &configFile)
    {
        helicsFederateRegisterInterfaces (fed, configFile.c_str (), hThrowOnError ());
    }
    int getIntegerProperty (int intProperty) const
    {
        return helicsFederateGetIntegerProperty (fed, intProperty, hThrowOnError ());
    }

    helics_federate_state getState () const { return helicsFederateGetState (fed, NULL); }

    void enterInitializingMode () { helicsFederateEnterInitializingMode (fed, hThrowOnError ()); }

    void enterInitializingModeAsync () { helicsFederateEnterInitializingModeAsync (fed, hThrowOnError ()); }

    bool isAsyncOperationCompleted () const
    {
        // returns int, 1 = true, 0 = false
        return helicsFederateIsAsyncOperationCompleted (fed, NULL) > 0;
    }

    void enterInitializingModeComplete () { helicsFederateEnterInitializingModeComplete (fed, hThrowOnError ()); }

    helics_iteration_result
    enterExecutingMode (helics_iteration_request iterate = helics_iteration_request_no_iteration)
    {
        helics_iteration_result out_iterate = helics_iteration_result_next_step;
        if (iterate == helics_iteration_request_no_iteration)
        {
            helicsFederateEnterExecutingMode (fed, hThrowOnError ());
        }
        else
        {
            out_iterate = helicsFederateEnterExecutingModeIterative (fed, iterate, hThrowOnError ());
        }
        return out_iterate;
    }

    void enterExecutingModeAsync (helics_iteration_request iterate = helics_iteration_request_no_iteration)
    {
        if (iterate == helics_iteration_request_no_iteration)
        {
            helicsFederateEnterExecutingModeAsync (fed, hThrowOnError ());
            exec_async_iterate = false;
        }
        else
        {
            helicsFederateEnterExecutingModeIterativeAsync (fed, iterate, hThrowOnError ());
            exec_async_iterate = true;
        }
    }

    helics_iteration_result enterExecutingModeComplete ()
    {
        helics_iteration_result out_iterate = helics_iteration_result_next_step;
        if (exec_async_iterate)
        {
            out_iterate = helicsFederateEnterExecutingModeIterativeComplete (fed, hThrowOnError ());
        }
        else
        {
            helicsFederateEnterExecutingModeComplete (fed, hThrowOnError ());
        }
        return out_iterate;
    }

    void finalize () { helicsFederateFinalize (fed, hThrowOnError ()); }

    void finalizeAsync () { helicsFederateFinalizeAsync (fed, hThrowOnError ()); }

    void finalizeComplete () { helicsFederateFinalizeComplete (fed, hThrowOnError ()); }

    helics_time requestTime (helics_time time) { return helicsFederateRequestTime (fed, time, hThrowOnError ()); }

    helics_time requestNextStep () { return helicsFederateRequestNextStep (fed, hThrowOnError ()); }

    helics_iteration_time requestTimeIterative (helics_time time, helics_iteration_request iterate)
    {
        helics_iteration_time itTime;
        itTime.grantedTime =
          helicsFederateRequestTimeIterative (fed, time, iterate, &(itTime.status), hThrowOnError ());
        return itTime;
    }

    void requestTimeAsync (helics_time time) { helicsFederateRequestTimeAsync (fed, time, hThrowOnError ()); }

    void requestTimeIterativeAsync (helics_time time, helics_iteration_request iterate)
    {
        helicsFederateRequestTimeIterativeAsync (fed, time, iterate, hThrowOnError ());
    }

    helics_time requestTimeComplete () { return helicsFederateRequestTimeComplete (fed, hThrowOnError ()); }

    helics_iteration_time requestTimeIterativeComplete ()
    {
        helics_iteration_time itTime;
        itTime.grantedTime = helicsFederateRequestTimeIterativeComplete (fed, &(itTime.status), hThrowOnError ());
        return itTime;
    }

    const char *getName () const { return helicsFederateGetName (fed); }
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
    std::string query (const std::string &target, const std::string &queryStr) const
    {
        // returns helics_query
        helics_query q = helicsCreateQuery (target.c_str (), queryStr.c_str ());
        std::string result (helicsQueryExecute (q, fed, hThrowOnError ()));
        helicsQueryFree (q);
        return result;
    }

    Filter registerFilter (helics_filter_type type, const std::string &name = std::string ())
    {
        return Filter (helicsFederateRegisterFilter (fed, type, name.c_str (), hThrowOnError ()));
    }

    /** create a cloning Filter on the specified federate
    @details cloning filters copy a message and send it to multiple locations source and destination can be added
    through other functions
    @param deliveryEndpoint the specified endpoint to deliver the message
    @return a helics_filter object
    */
    CloningFilter registerCloningFilter (const std::string &deliveryEndpoint)
    {
        return CloningFilter (
          helicsFederateRegisterCloningFilter (fed, deliveryEndpoint.c_str (), hThrowOnError ()));
    }

    Filter registerGlobalFilter (helics_filter_type type, const std::string &name = std::string ())
    {
        return Filter (helicsFederateRegisterGlobalFilter (fed, type, name.c_str (), hThrowOnError ()));
    }

    /** create a cloning Filter on the specified federate
    @details cloning filters copy a message and send it to multiple locations source and destination can be added
    through other functions
    @param deliveryEndpoint the specified endpoint to deliver the message
    @return a CloningFilter object
    */
    CloningFilter registerGlobalCloningFilter (const std::string &deliveryEndpoint)
    {
        return CloningFilter (
          helicsFederateRegisterGlobalCloningFilter (fed, deliveryEndpoint.c_str (), hThrowOnError ()));
    }

    Filter getFilter (const std::string &name)
    {
        return Filter (helicsFederateGetFilter (fed, name.c_str (), hThrowOnError ()));
    }
    Filter getSubscription (int index)
    {
        return Filter (helicsFederateGetFilterByIndex (fed, index, hThrowOnError ()));
    }

    /** set a global federation value*/
    void setGlobal (const std::string &valueName, const std::string &value)
    {
        helicsFederateSetGlobal (fed, valueName.c_str (), value.c_str (), hThrowOnError ());
    }

  protected:
    helics_federate fed;
    bool exec_async_iterate;
};

}  // namespace helicscpp
#endif
