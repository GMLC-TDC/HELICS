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
#include "Subscription.hpp"

#include <sstream>

namespace helics
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
    std::vector<helics_subscription> subs;
    std::vector<helics_publication> pubs;
  public:
    friend class helics::FederateInfo;

    explicit ValueFederate (FederateInfo &fi)
    {
        fed = helicsCreateValueFederate (fi.getInfo());
    }

    explicit ValueFederate (const std::string &jsonString)
    {
        fed = helicsCreateValueFederateFromJson (jsonString.c_str());
    }

    ValueFederate(const ValueFederate &vfed) :Federate(vfed),subs(vfed.subs),pubs(vfed.pubs)
    {
    }
    ValueFederate &operator=(const ValueFederate &fedObj)
    {
        Federate::operator=(fedObj);
        subs = fedObj.subs;
        pubs = fedObj.pubs;
        return *this;
    }
#ifdef HELICS_HAS_RVALUE_REFS
    ValueFederate(ValueFederate &&fedObj) :Federate(std::move(fedObj)),subs(std::move(fedObj.subs)),pubs(std::move(fedObj.pubs))
    {

    }
    ValueFederate &operator=(ValueFederate &&fedObj)
    {
        Federate::operator=(std::move(fedObj));
        subs = std::move(fedObj.subs);
        pubs = std::move(fedObj.pubs);
        return *this;
    }
#endif
    // Default constructor, not meant to be used
    ValueFederate () {}

    /** Methods to register publications **/
    Publication
    registerPublication (const std::string &name, const std::string &type, const std::string &units = "")
    {
        helics_publication pub = helicsFederateRegisterPublication (fed, name.c_str(), type.c_str(), units.c_str());
        printf("got pub as %llx\n", (unsigned long long)(pub));
        pubs.push_back(pub);
        printf("pushed back pub\n");
        return Publication(pub);
    }

    Publication
    registerTypePublication (const std::string &name, int type, const std::string &units = "")
    {
        helics_publication pub = helicsFederateRegisterTypePublication (fed, name.c_str(), type, units.c_str());
        pubs.push_back(pub);
        return Publication(pub);
    }

    Publication
    registerGlobalPublication (const std::string &name, const std::string &type, const std::string &units = "")
    {
        helics_publication pub = helicsFederateRegisterGlobalPublication (fed, name.c_str(), type.c_str(), units.c_str());
        pubs.push_back(pub);
        return Publication(pub);
    }

    Publication
    registerGlobalTypePublication (const std::string &name, int type, const std::string &units = "")
    {
        helics_publication pub = helicsFederateRegisterGlobalTypePublication (fed, name.c_str(), type, units.c_str());
        pubs.push_back(pub);
        return Publication(pub);
    }

    Publication
    registerPublicationIndexed (const std::string &name, int index1, int type, const std::string &units = "")
    {
        std::string indexed_name = name + '_' + toStr (index1);
        return registerGlobalTypePublication (indexed_name, type, units);
    }

    Publication
    registerPublicationIndexed (const std::string &name, int index1, int index2,int type, const std::string &units = "")
    {
        std::string indexed_name = name + '_' + toStr (index1) + '_' + toStr (index2);
        return registerGlobalTypePublication (indexed_name, type, units);
    }

    /** Methods to register subscriptions **/
    Subscription
    registerSubscription (const std::string &name, const std::string &type, const std::string &units = "")
    {
        helics_subscription sub = helicsFederateRegisterSubscription (fed, name.c_str(), type.c_str(), units.c_str());
        subs.push_back(sub);
        return Subscription(sub);
    }

    /** Methods to register subscriptions **/
    Subscription
        registerOptionalSubscription(const std::string &name, const std::string &type, const std::string &units = "")
    {
        helics_subscription sub = helicsFederateRegisterOptionalSubscription(fed, name.c_str(), type.c_str(), units.c_str());
        subs.push_back(sub);
        return Subscription(sub);
    }

    Subscription
    registerTypeSubscription (const std::string &name, int type, const std::string &units = "")
    {
        helics_subscription sub = helicsFederateRegisterTypeSubscription (fed, name.c_str(), type, units.c_str());
        subs.push_back(sub);
        return Subscription(sub);
    }

    Subscription
    registerSubscriptionIndexed (const std::string &name, int index1, int type, const std::string &units = "")
    {
        std::string indexed_name = name + '_' + toStr (index1);
        return registerTypeSubscription (indexed_name, type, units);
    }

    Subscription
    registerSubscriptionIndexed (const std::string &name,
                                         int index1,
                                         int index2,
                                         int type,
                                         const std::string &units = "")
    {
        std::string indexed_name = name + '_' + toStr (index1) + '_' + toStr (index2);
        return registerTypeSubscription (indexed_name, type, units);
    }

    int getSubscriptionCount() const
    {
        return helicsFederateGetSubscriptionCount(fed);
    }

    int getPublicationCount() const
    {
        return helicsFederateGetPublicationCount(fed);
    }
    // TODO: use c api to implement this method... callbacks too?
    /** Get a list of all subscriptions with updates since the last call **/
    std::vector<helics_subscription> queryUpdates () { return std::vector<helics_subscription>(); }
    // call helicsSubscriptionIsUpdated for each sub
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
