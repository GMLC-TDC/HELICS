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

    // Default constructor, not meant to be used
    ValueFederate () {}

    virtual ~ValueFederate ()
    {
    }

    /** Methods to register publications **/
    helics_publication
    registerPublication (const std::string &name, const std::string &type, const std::string &units = "")
    {
        helics_publication pub = helicsFederateRegisterPublication (fed, name.c_str(), type.c_str(), units.c_str());
        pubs.push_back(pub);
        return pub;
    }

    helics_publication
    registerTypePublication (const std::string &name, int type, const std::string &units = "")
    {
        helics_publication pub = helicsFederateRegisterTypePublication (fed, name.c_str(), type, units.c_str());
        pubs.push_back(pub);
        return pub;
    }

    helics_publication
    registerGlobalPublication (const std::string &name, const std::string &type, const std::string &units = "")
    {
        helics_publication pub = helicsFederateRegisterGlobalPublication (fed, name.c_str(), type.c_str(), units.c_str());
        pubs.push_back(pub);
        return pub;
    }

    helics_publication
    registerGlobalTypePublication (const std::string &name, int type, const std::string &units = "")
    {
        helics_publication pub = helicsFederateRegisterGlobalTypePublication (fed, name.c_str(), type, units.c_str());
        pubs.push_back(pub);
        return pub;
    }

    helics_publication
    registerPublicationIndexed (const std::string &name, int index1, int type, const std::string &units = "")
    {
        std::string indexed_name = name + '_' + toStr (index1);
        return registerGlobalTypePublication (indexed_name, type, units);
    }

    helics_publication
    registerPublicationIndexed (const std::string &name, int index1, int index2,int type, const std::string &units = "")
    {
        std::string indexed_name = name + '_' + toStr (index1) + '_' + toStr (index2);
        return registerGlobalTypePublication (indexed_name, type, units);
    }

    /** Methods to register subscriptions **/
    helics_subscription
    registerSubscription (const std::string &name, const std::string &type, const std::string &units = "")
    {
        helics_subscription sub = helicsFederateRegisterSubscription (fed, name.c_str(), type.c_str(), units.c_str());
        subs.push_back(sub);
        return sub;
    }

    helics_subscription
    registerTypeSubscription (const std::string &name, int type, const std::string &units = "")
    {
        helics_subscription sub = helicsFederateRegisterTypeSubscription (fed, name.c_str(), type, units.c_str());
        subs.push_back(sub);
        return sub;
    }

    helics_subscription
    registerSubscriptionIndexed (const std::string &name, int index1, int type, const std::string &units = "")
    {
        std::string indexed_name = name + '_' + toStr (index1);
        return registerTypeSubscription (indexed_name, type, units);
    }

    helics_subscription
    registerSubscriptionIndexed (const std::string &name,
                                         int index1,
                                         int index2,
                                         int type,
                                         const std::string &units = "")
    {
        std::string indexed_name = name + '_' + toStr (index1) + '_' + toStr (index2);
        return registerTypeSubscription (indexed_name, type, units);
    }

    /** Methods to set default values for subscriptions **/
    void setDefaultValue (helics_subscription sub, const char *data, int len)
    {
        // returns helics_status
        helicsSubscriptionSetDefault (sub, data, len);
    }

    void setDefaultValue (helics_subscription sub, const std::string &str)
    {
        // returns helics_status
        helicsSubscriptionSetDefaultString (sub, str.c_str());
    }

    void setDefaultValue (helics_subscription sub, int64_t val)
    {
        // returns helics_status
        helicsSubscriptionSetDefaultInteger (sub, val);
    }

    void setDefaultValue (helics_subscription sub, double val)
    {
        // returns helics_status
        helicsSubscriptionSetDefaultDouble (sub, val);
    }

    void setDefaultValue (helics_subscription sub, const std::complex<double> &cmplx)
    {
        // returns helics_status
        helicsSubscriptionSetDefaultComplex(sub, cmplx.real(), cmplx.imag());
    }

    void setDefaultValue (helics_subscription sub, const std::vector<double> &data)
    {
        // returns helics_status
        helicsSubscriptionSetDefaultVector (sub, data.data(), static_cast<int>(data.size() * sizeof(double)));
    }

    /** Methods to get subscription values **/
    int getValue (helics_subscription sub, char *data, int maxlen)
    {
        int actualSize;
       helicsSubscriptionGetValue (sub, data, maxlen,&actualSize);
       return actualSize;
    }

    std::string getString (helics_subscription sub)
    {
        int size = helicsSubscriptionGetValueSize(sub);
        std::string result;
        result.resize(size+1);
        helicsSubscriptionGetString (sub, &result[0], size+1);
        result[size] = '\0';
        return result;
    }

    int64_t getInteger (helics_subscription sub)
    {
        int64_t val;
        helicsSubscriptionGetInteger (sub, &val);
        return val;
    }

    double getDouble (helics_subscription sub)
    {
        double val;
        helicsSubscriptionGetDouble (sub, &val);
        return val;
    }

    std::complex<double> getComplex (helics_subscription sub)
    {
        double real;
        double imag;
        helicsSubscriptionGetComplex (sub, &real, &imag);
        std::complex<double> result (real, imag);
        return result;
    }

    int getVector (helics_subscription sub, double *data, int maxlen)
    {
        int actualSize;
        helicsSubscriptionGetVector (sub, data, maxlen,&actualSize);
        return actualSize;
    }

    void getVector(helics_subscription sub, std::vector<double> &data)
    {
        int actualSize = helicsSubscriptionGetVectorSize(sub);
        data.resize(actualSize);
        helicsSubscriptionGetVector(sub, data.data(), actualSize, &actualSize);
    }

    /** Methods to publish values **/
    void publish (helics_publication pub, const char *data, int len)
    {
        // returns helics_status
        helicsPublicationPublish (pub, data, len);
    }

    void publish (helics_publication pub, std::string str)
    {
        // returns helics_status
        helicsPublicationPublishString (pub, str.c_str());
    }

    void publish (helics_publication pub, int64_t val)
    {
        // returns helics_status
        helicsPublicationPublishInteger (pub, val);
    }

    void publish (helics_publication pub, double val)
    {
        // returns helics_status
        helicsPublicationPublishDouble (pub, val);
    }

    void publish (helics_publication pub, std::complex<double> cmplx)
    {
        // returns helics_status
        helicsPublicationPublishComplex (pub, cmplx.real(), cmplx.imag());
    }

    void publish (helics_publication pub, std::vector<double> data)
    {
        // c++98 doesn't guarantee vector data will be contiguous
        // might make sense to have a pre-allocated array (that can grow) set aside for reuse
        double *arr = (double*) malloc(data.size() * sizeof(double));
        for (unsigned int i = 0; i < data.size(); i++)
        {
            arr[i] = data[i];
        }
        // returns helics_status
        helicsPublicationPublishVector (pub, arr, static_cast<int>(data.size() * sizeof(double)));
        free (arr);
    }

    /** Check if a subscription is updated **/
    bool isUpdated (helics_subscription sub) const
    {
        return helicsSubscriptionIsUpdated (sub) > 0;
    }

    /** Get the last time a subscription was updated **/
    helics_time_t getLastUpdateTime (helics_subscription sub) const
    {
        return helicsSubscriptionLastUpdateTime (sub);
    }

    // TODO: use c api to implement this method... callbacks too?
    /** Get a list of all subscriptions with updates since the last call **/
    std::vector<helics_subscription> queryUpdates () { return std::vector<helics_subscription>(); }
    // call helicsSubscriptionIsUpdated for each sub

    std::string getSubscriptionName (helics_subscription sub) const
    {
        char str[255];
        helicsSubscriptionGetKey (sub, &str[0], sizeof(str));
        std::string result (str);
        return result;
    }

    std::string getPublicationName (helics_publication pub) const
    {
        char str[255];
        helicsPublicationGetKey (pub, &str[0], sizeof(str));
        std::string result (str);
        return result;
    }

    std::string getSubscriptionUnits (helics_subscription sub) const
    {
        char str[255];
        helicsSubscriptionGetUnits (sub, &str[0], sizeof(str));
        std::string result (str);
        return result;
    }

    std::string getPublicationUnits (helics_publication pub) const
    {
        char str[255];
        helicsPublicationGetUnits (pub, &str[0], sizeof(str));
        std::string result (str);
        return result;
    }

    std::string getSubscriptionType (helics_subscription sub) const
    {
        char str[255];
        helicsSubscriptionGetType (sub, &str[0], sizeof(str));
        std::string result (str);
        return result;
    }

    std::string getPublicationType (helics_publication pub) const
    {
        char str[255];
        helicsPublicationGetType (pub, &str[0], sizeof(str));
        std::string result (str);
        return result;
    }

  private:
    std::vector<helics_subscription> subs;
    std::vector<helics_publication> pubs;

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

