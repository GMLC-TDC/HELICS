/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_VALUE_FEDERATE_HPP_
#define HELICS_CPP98_VALUE_FEDERATE_HPP_
#pragma once

#include "../shared_api_library/ValueFederate.h"
#include "Federate.hpp"
#include "Input.hpp"
#include "Publication.hpp"
#include <exception>
#include <sstream>

namespace helicscpp
{
enum PubSubTypes
{
    STRING_TYPE = helics_data_type_string,
    DOUBLE_TYPE = helics_data_type_double,
    INT_TYPE = helics_data_type_int,
    COMPLEX_TYPE = helics_data_type_complex,
    VECTOR_TYPE = helics_data_type_vector,
    TIME_TYPE = helics_data_type_time,
    BOOLEAN_TYPE = helics_data_type_boolean,
    RAW_TYPE = helics_data_type_raw
};

class ValueFederate : public virtual Federate
{
  private:
    std::vector<helics_input> ipts;
    std::vector<helics_publication> pubs;

  public:
    friend class helicscpp::FederateInfo;

    explicit ValueFederate (const std::string &fedname, FederateInfo &fi)
    {
        fed = helicsCreateValueFederate (fedname.c_str (), fi.getInfo (), hThrowOnError ());
        if (fed == NULL)
        {
            throw (HelicsException (helics_error_registration_failure, "Fed==NULL"));
        }
    }

    explicit ValueFederate (const std::string &configString)
    {
        fed = helicsCreateValueFederateFromConfig (configString.c_str (), hThrowOnError ());
        if (fed == NULL)
        {
            throw (HelicsException (helics_error_registration_failure, "Fed==NULL"));
        }
    }

    ValueFederate (const ValueFederate &vfed) : Federate (vfed), ipts (vfed.ipts), pubs (vfed.pubs) {}
    ValueFederate &operator= (const ValueFederate &fedObj)
    {
        Federate::operator= (fedObj);
        ipts = fedObj.ipts;
        pubs = fedObj.pubs;
        if (fed == NULL)
        {
            throw (HelicsException (helics_error_registration_failure, "Fed==NULL move constructor"));
        }
        return *this;
    }
#ifdef HELICS_HAS_RVALUE_REFS
    ValueFederate (ValueFederate &&fedObj) HELICS_NOTHROW : Federate (),
                                                            ipts (std::move (fedObj.ipts)),
                                                            pubs (std::move (fedObj.pubs))
    {
        Federate::operator= (std::move (fedObj));
    }
    ValueFederate &operator= (ValueFederate &&fedObj) HELICS_NOTHROW
    {
        ipts = std::move (fedObj.ipts);
        pubs = std::move (fedObj.pubs);
        Federate::operator= (std::move (fedObj));
        return *this;
    }
#endif
    // Default constructor, not meant to be used
    ValueFederate () HELICS_NOTHROW {}

    /** Methods to register publications **/
    Publication
    registerTypePublication (const std::string &name, const std::string &type, const std::string &units = "")
    {
        helics_publication pub = helicsFederateRegisterTypePublication (fed, name.c_str (), type.c_str (),
                                                                        units.c_str (), hThrowOnError ());
        pubs.push_back (pub);
        return Publication (pub);
    }

    Publication registerPublication (const std::string &name, helics_data_type type, const std::string &units = "")
    {
        helics_publication pub =
          helicsFederateRegisterPublication (fed, name.c_str (), type, units.c_str (), hThrowOnError ());
        pubs.push_back (pub);
        return Publication (pub);
    }

    Publication
    registerGlobalTypePublication (const std::string &name, const std::string &type, const std::string &units = "")
    {
        helics_publication pub = helicsFederateRegisterGlobalTypePublication (fed, name.c_str (), type.c_str (),
                                                                              units.c_str (), hThrowOnError ());
        pubs.push_back (pub);
        return Publication (pub);
    }

    Publication
    registerGlobalPublication (const std::string &name, helics_data_type type, const std::string &units = "")
    {
        helics_publication pub =
          helicsFederateRegisterGlobalPublication (fed, name.c_str (), type, units.c_str (), hThrowOnError ());
        pubs.push_back (pub);
        return Publication (pub);
    }

    Publication registerPublicationIndexed (const std::string &name,
                                            int index1,
                                            helics_data_type type,
                                            const std::string &units = "")
    {
        std::string indexed_name = name + '_' + toStr (index1);
        return registerGlobalPublication (indexed_name, type, units);
    }

    Publication registerPublicationIndexed (const std::string &name,
                                            int index1,
                                            int index2,
                                            helics_data_type type,
                                            const std::string &units = std::string ())
    {
        std::string indexed_name = name + '_' + toStr (index1) + '_' + toStr (index2);
        return registerGlobalPublication (indexed_name, type, units);
    }

    void registerFromPublicationJSON (const std::string &json)
    {
        helicsFederateRegisterFromPublicationJSON (fed, json.c_str (), hThrowOnError ());
    }

    Publication getPublication (const std::string &name)
    {
        return Publication (helicsFederateGetPublication (fed, name.c_str (), hThrowOnError ()));
    }
    Publication getPublication (int index)
    {
        return Publication (helicsFederateGetPublicationByIndex (fed, index, hThrowOnError ()));
    }
    /** Methods to register subscriptions **/
    Input registerSubscription (const std::string &name, const std::string &units = std::string ())
    {
        helics_input sub =
          helicsFederateRegisterSubscription (fed, name.c_str (), units.c_str (), hThrowOnError ());
        ipts.push_back (sub);
        return Input (sub);
    }

    Input registerSubscriptionIndexed (const std::string &name, int index1, const std::string &units = "")
    {
        std::string indexed_name = name + '_' + toStr (index1);
        return registerSubscription (indexed_name, units);
    }

    Input
    registerSubscriptionIndexed (const std::string &name, int index1, int index2, const std::string &units = "")
    {
        std::string indexed_name = name + '_' + toStr (index1) + '_' + toStr (index2);
        return registerSubscription (indexed_name, units);
    }

    Input getInput (const std::string &name)
    {
        return Input (helicsFederateGetInput (fed, name.c_str (), hThrowOnError ()));
    }
    Input getSubscription (int index)
    {
        return Input (helicsFederateGetInputByIndex (fed, index, hThrowOnError ()));
    }

    int getInputCount () const { return helicsFederateGetInputCount (fed); }

    int getPublicationCount () const { return helicsFederateGetPublicationCount (fed); }
    // TODO: use c api to implement this method... callbacks too?
    /** Get a list of all subscriptions with updates since the last call **/
    std::vector<helics_input> queryUpdates () { return std::vector<helics_input> (); }

    /** clear all the update flags from all federate inputs*/
    void clearUpdates () { helicsFederateClearUpdates (fed); }
    /** publish data contained in a JSON file*/
    void publishJSON (const std::string &json)
    {
        helicsFederatePublishJSON (fed, json.c_str (), hThrowOnError ());
    }

  private:
    // Utility function for converting numbers to string
    template <typename T>
    std::string toStr (T num)
    {
        std::ostringstream ss;
        ss << num;
        return ss.str ();
    }
};
}  // namespace helicscpp
#endif
