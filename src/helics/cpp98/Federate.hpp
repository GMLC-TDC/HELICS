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
#include "helicsExceptions.hpp"
#include "Filter.hpp"
#include "config.hpp"
#include <complex>

#include <string>
#include <vector>

// defines for setFlag values in core/flag-definitions.h
// enum for core_type:int in core/core-types.h

namespace helicscpp
{
class FederateInfo
{
  public:
    FederateInfo () { fi = helicsCreateFederateInfo (); }

    FederateInfo ( const std::string &coretype)
    {
        fi = helicsCreateFederateInfo ();
        helicsFederateInfoSetCoreTypeFromString (fi, coretype.c_str (), hThrowOnError ());
    }

    ~FederateInfo () { helicsFederateInfoFree (fi); }

    void setCoreName (const std::string &corename) { helicsFederateInfoSetCoreName (fi, corename.c_str (),NULL); }

    void setCoreInitString (const std::string &coreInit)
    {
        helicsFederateInfoSetCoreInitString (fi, coreInit.c_str (),NULL);
    }

    void setCoreTypeFromString (const std::string &coretype)
    {
        helicsFederateInfoSetCoreTypeFromString (fi, coretype.c_str (), hThrowOnError ());
    }

    void setCoreType (int coretype) { helicsFederateInfoSetCoreType (fi, coretype,NULL); }

    void setFlagOption (int flag, int value) { helicsFederateInfoSetFlagOption (fi, flag, value,NULL); }

    void setTimeProperty (int timeProperty, helics_time_t timeValue) { helicsFederateInfoSetTimeProperty (fi,timeProperty, timeValue,NULL); }

    
    void setIntegerProperty (int integerProperty, int intValue) { helicsFederateInfoSetIntegerProperty (fi,integerProperty, intValue,NULL); }

    helics_federate_info_t getInfo () { return fi; }

  private:
    helics_federate_info_t fi;
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
      Federate() : fed(NULL), exec_async_iterate(false) {};

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
    virtual ~Federate() {
        if (fed != NULL)
        {
            helicsFederateFree(fed);
        }
    }

    operator helics_federate () const { return fed; }

    helics_federate baseObject () const { return fed; }

    void setFlag (int flag, bool value) { helicsFederateSetFlagOption (fed, flag, value?helics_true:helics_false, hThrowOnError ()); }

    void setTimeProperty (int tProperty, helics_time_t timeValue)
    {
        helicsFederateSetTimeProperty (fed, tProperty, timeValue, hThrowOnError ());
    }


    void setIntegerProperty (int intProperty, int value)
    {
        helicsFederateSetIntegerProperty (fed, intProperty, value, hThrowOnError ());
    }

	 bool getFlag (int flag) const { return (helicsFederateGetFlagOption (fed, flag,hThrowOnError())!=helics_false); }

   helics_time_t getTimeProperty (int tProperty) const
    {
        return helicsFederateGetTimeProperty (fed, tProperty,hThrowOnError ());
    }

    int getIntegerProperty (int intProperty) const
    {
        return helicsFederateGetIntegerProperty (fed, intProperty, hThrowOnError ());
    }

    federate_state getState() const
    {
        return helicsFederateGetState(fed, NULL);
    }

    void enterInitializingMode ()
    { helicsFederateEnterInitializingMode (fed, hThrowOnError ());
    }

    void enterInitializingModeAsync ()
    { helicsFederateEnterInitializingModeAsync (fed, hThrowOnError ());
    }

    bool isAsyncOperationCompleted () const
    {
        // returns int, 1 = true, 0 = false
        return helicsFederateIsAsyncOperationCompleted (fed,NULL) > 0;
    }

    void enterInitializingModeComplete ()
    {
        helicsFederateEnterInitializingModeComplete (fed, hThrowOnError ());
    }

    helics_iteration_status enterExecutingMode (helics_iteration_request iterate = no_iteration)
    {
        helics_iteration_status out_iterate = next_step;
        if (iterate == no_iteration)
        {
            helicsFederateEnterExecutingMode (fed, hThrowOnError ());
        }
        else
        {
            out_iterate = helicsFederateEnterExecutingModeIterative (fed, iterate, hThrowOnError ());
        }
        return out_iterate;
    }

    void enterExecutingModeAsync (helics_iteration_request iterate = no_iteration)
    {
        if (iterate == no_iteration)
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

    helics_iteration_status enterExecutingModeComplete ()
    {
        helics_iteration_status out_iterate = next_step;
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

    helics_time_t requestTime (helics_time_t time)
    {
        return helicsFederateRequestTime (fed, time, hThrowOnError ());
    }

    helics_iteration_time requestTimeIterative (helics_time_t time, helics_iteration_request iterate)
    {
        helics_iteration_time itTime;
        itTime.grantedTime =
          helicsFederateRequestTimeIterative (fed, time, iterate, &(itTime.status), hThrowOnError ());
        return itTime;
    }

    void requestTimeAsync (helics_time_t time)
    { helicsFederateRequestTimeAsync (fed, time, hThrowOnError ());
    }

    void requestTimeIterativeAsync (helics_time_t time, helics_iteration_request iterate)
    {
        helicsFederateRequestTimeIterativeAsync (fed, time, iterate, hThrowOnError ());
    }

    helics_time_t requestTimeComplete ()
    { return helicsFederateRequestTimeComplete (fed, hThrowOnError ());
    }

    helics_iteration_time requestTimeIterativeComplete ()
    {
        helics_iteration_time itTime;
        itTime.grantedTime = helicsFederateRequestTimeIterativeComplete (fed, &(itTime.status), hThrowOnError ());
        return itTime;
    }

    const char *getName () const
    {
        return helicsFederateGetName (fed);
    }
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

    Filter registerFilter (helics_filter_type_t type,
                                 const std::string &name = std::string ())
    {
        return Filter (helicsFederateRegisterFilter (fed, type, name.c_str (), hThrowOnError ()));
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
        return CloningFilter (helicsFederateRegisterCloningFilter (fed, deliveryEndpoint.c_str (),
                              hThrowOnError ()));
    }

    Filter registerGlobalFilter(helics_filter_type_t type,
        const std::string &name = std::string())
    {
        return Filter(helicsFederateRegisterGlobalFilter(fed, type, name.c_str(), hThrowOnError()));
    }


    /** create a cloning Filter on the specified federate
    @details cloning filters copy a message and send it to multiple locations source and destination can be added
    through other functions
    @param fed the fed to register through
    @param deliveryEndpoint the specified endpoint to deliver the message
    @return a helics_filter object
    */
    CloningFilter registerGlobalCloningFilter(const std::string &deliveryEndpoint)
    {
        return CloningFilter(helicsFederateRegisterGlobalCloningFilter(fed, deliveryEndpoint.c_str(),
            hThrowOnError()));
    }

  protected:
    helics_federate fed;
    bool exec_async_iterate;
};

}  // namespace helicscpp
#endif
