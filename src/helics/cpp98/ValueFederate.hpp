/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#ifndef HELICS_CPP98_VALUE_FEDERATE_HPP_
#define HELICS_CPP98_VALUE_FEDERATE_HPP_
#pragma once

#include "Federate.hpp"
#include "../shared_api_library/ValueFederate.h"
#include "Publication.hpp"
#include "Input.hpp"
#include <sstream>
#include <exception>

namespace helicscpp
{
enum PubSubTypes
{
    STRING_TYPE = HELICS_DATA_TYPE_STRING,
    DOUBLE_TYPE = HELICS_DATA_TYPE_DOUBLE,
    INT_TYPE = HELICS_DATA_TYPE_INT,
    COMPLEX_TYPE = HELICS_DATA_TYPE_COMPLEX,
    VECTOR_TYPE = HELICS_DATA_TYPE_VECTOR,
    RAW_TYPE = HELICS_DATA_TYPE_RAW
};

class ValueFederate : public virtual Federate
{
private:
    std::vector<helics_input> ipts;
    std::vector<helics_publication> pubs;
  public:
    friend class helicscpp::FederateInfo;

    explicit ValueFederate (const std::string &fedname,FederateInfo &fi)
    {
        fed = helicsCreateValueFederate (fedname.c_str(),fi.getInfo(),hThrowOnError());
        if (fed == NULL)
        {
            throw(std::runtime_error("fed==nullptr constructor"));
        }
    }

    explicit ValueFederate (const std::string &configString)
    {
        fed = helicsCreateValueFederateFromConfig(configString.c_str(),hThrowOnError());
        if (fed == NULL)
        {
            throw(std::runtime_error("fed==nullptr create from configuration file"));
        }
    }

    ValueFederate(const ValueFederate &vfed) :Federate(vfed),ipts(vfed.ipts),pubs(vfed.pubs)
    {
    }
    ValueFederate &operator=(const ValueFederate &fedObj)
    {
        Federate::operator=(fedObj);
        ipts = fedObj.ipts;
        pubs = fedObj.pubs;
        if (fed == NULL)
        {
            throw(std::runtime_error("fed==nullptr assignment"));
        }
        return *this;
    }
#ifdef HELICS_HAS_RVALUE_REFS
    ValueFederate(ValueFederate &&fedObj) :Federate(),ipts(std::move(fedObj.ipts)),pubs(std::move(fedObj.pubs))
    {
        Federate::operator=(std::move(fedObj));
        if (fed == NULL)
        {
            throw(std::runtime_error("fed==nullptr move constructor"));
        }
    }
    ValueFederate &operator=(ValueFederate &&fedObj)
    {
        ipts = std::move(fedObj.ipts);
        pubs = std::move(fedObj.pubs);
        Federate::operator=(std::move(fedObj));
        if (fed == NULL)
        {
            throw(std::runtime_error("fed==nullptr move assignment"));
        }
        return *this;
    }
#endif
    // Default constructor, not meant to be used
    ValueFederate () { }

    /** Methods to register publications **/
    Publication
    registerTypePublication (const std::string &name, const std::string &type, const std::string &units = "")
    {
        helics_publication pub = helicsFederateRegisterTypePublication (fed, name.c_str(), type.c_str(), units.c_str(),hThrowOnError());
        pubs.push_back(pub);
        return Publication(pub);
    }

    Publication
    registerPublication (const std::string &name, int type, const std::string &units = "")
    {
        helics_publication pub = helicsFederateRegisterPublication (fed, name.c_str(), type, units.c_str(),hThrowOnError());
        pubs.push_back(pub);
        return Publication(pub);
    }

    Publication
    registerGlobalTypePublication (const std::string &name, const std::string &type, const std::string &units = "")
    {
        helics_publication pub = helicsFederateRegisterGlobalTypePublication (fed, name.c_str(), type.c_str(), units.c_str(),hThrowOnError());
        pubs.push_back(pub);
        return Publication(pub);
    }

    Publication
    registerGlobalPublication (const std::string &name, int type, const std::string &units = "")
    {
        helics_publication pub =
          helicsFederateRegisterGlobalPublication (fed, name.c_str (), type, units.c_str (), hThrowOnError ());
        pubs.push_back(pub);
        return Publication(pub);
    }

    Publication
    registerPublicationIndexed (const std::string &name, int index1, int type, const std::string &units = "")
    {
        std::string indexed_name = name + '_' + toStr (index1);
        return registerGlobalPublication (indexed_name, type, units);
    }

    Publication
    registerPublicationIndexed (const std::string &name, int index1, int index2,int type, const std::string &units = "")
    {
        std::string indexed_name = name + '_' + toStr (index1) + '_' + toStr (index2);
        return registerGlobalPublication (indexed_name, type, units);
    }

    /** Methods to register subscriptions **/
    Input
    registerSubscription (const std::string &name,  const std::string &units = "")
    {
        helics_input sub = helicsFederateRegisterSubscription (fed, name.c_str(), units.c_str(),hThrowOnError());
        ipts.push_back(sub);
        return Input(sub);
    }

    Input
    registerSubscriptionIndexed (const std::string &name, int index1, const std::string &units = "")
    {
        std::string indexed_name = name + '_' + toStr (index1);
        return registerSubscription (indexed_name, units);
    }

    Input
    registerSubscriptionIndexed (const std::string &name,
                                         int index1,
                                         int index2,
                                         const std::string &units = "")
    {
        std::string indexed_name = name + '_' + toStr (index1) + '_' + toStr (index2);
        return registerSubscription (indexed_name, units);
    }

    int getInputCount() const
    {
        return helicsFederateGetInputCount(fed,nullptr);
    }

    int getPublicationCount() const
    {
        return helicsFederateGetPublicationCount(fed,nullptr);
    }
    // TODO: use c api to implement this method... callbacks too?
    /** Get a list of all subscriptions with updates since the last call **/
    std::vector<helics_input> queryUpdates () { return std::vector<helics_input>(); }
    // call helicsInputIsUpdated for each sub
  private:
    // Utility function for converting numbers to string
    template <typename T> std::string toStr (T num)
    {
        std::ostringstream ss;
        ss << num;
        return ss.str();
    }
};
} //namespace helics
#endif
