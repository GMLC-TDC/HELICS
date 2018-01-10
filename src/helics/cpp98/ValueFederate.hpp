/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef HELICS_CPP98_VALUE_FEDERATE_HPP_
#define HELICS_CPP98_VALUE_FEDERATE_HPP_
#pragma once

#include "helics.hpp"
#include "Federate.hpp"

#include <sstream>

// TODO: Update to use methods in c interface for getting length of data pointed to that can be gotten (all the ones taking a max len argument) - function may not exist yet

namespace helics
{
enum PubSubTypes
{
    STRING_TYPE = 0,
    DOUBLE_TYPE = 1,
    INT_TYPE = 2,
    COMPLEX_TYPE = 3,
    VECTOR_TYPE = 4,
    RAW_TYPE = 25
};
class ValueFederate : public virtual Federate
{
  public:
    friend class helics::FederateInfo;

    ValueFederate (FederateInfo &fi)
    {
        fed = helicsCreateValueFederate (fi.getInfo());
    }

    ValueFederate (const std::string &jsonString)
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
        helics_publication pub = helicsRegisterPublication (fed, name.c_str(), type.c_str(), units.c_str());
        pubs.push_back(pub);
        return pub;
    }

    helics_publication
    registerTypePublication (const std::string &name, int type, const std::string &units = "")
    {
        helics_publication pub = helicsRegisterTypePublication (fed, name.c_str(), type, units.c_str());
        pubs.push_back(pub);
        return pub;
    }

    helics_publication
    registerGlobalPublication (const std::string &name, const std::string &type, const std::string &units = "")
    {
        helics_publication pub = helicsRegisterGlobalPublication (fed, name.c_str(), type.c_str(), units.c_str());
        pubs.push_back(pub);
        return pub;
    }

    helics_publication
    registerGlobalTypePublication (const std::string &name, int type, const std::string &units = "")
    {
        helics_publication pub = helicsRegisterGlobalTypePublication (fed, name.c_str(), type, units.c_str());
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
        helics_subscription sub = helicsRegisterSubscription (fed, name.c_str(), type.c_str(), units.c_str());
        subs.push_back(sub);
        return sub;
    }

    helics_subscription
    registerTypeSubscription (const std::string &name, int type, const std::string &units = "")
    {
        helics_subscription sub = helicsRegisterTypeSubscription (fed, name.c_str(), type, units.c_str());
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

    // TODO: figure out how this maps to using helics_subscriptions instead of id's
    /** add a shortcut for locating a subscription
    @details primarily for use in looking up an id from a different location
    creates a local shortcut for referring to a subscription which may have a long actual name
    @param[in] the subscription identifier
    @param[in] shortcutName the name of the shortcut
    */
    void addSubscriptionShortcut (helics_subscription subid, const std::string &shortcutName) {}

    /** Methods to set default values for subscriptions **/
    void setDefaultValue (helics_subscription sub, const char *data, int len)
    {
        // returns helicsStatus
        helicsSetDefaultValue (sub, data, len);
    }

    void setDefaultValue (helics_subscription sub, const std::string &str)
    {
        // returns helicsStatus
        helicsSetDefaultString (sub, str.c_str());
    }

    void setDefaultValue (helics_subscription sub, int64_t val)
    {
        // returns helicsStatus
        helicsSetDefaultInteger (sub, val);
    }

    void setDefaultValue (helics_subscription sub, double val)
    {
        // returns helicsStatus
        helicsSetDefaultDouble (sub, val);
    }

    void setDefaultValue (helics_subscription sub, const std::complex<double> &cmplx)
    {
        // returns helicsStatus
        helicsSetDefaultComplex(sub, cmplx.real(), cmplx.imag());
    }

    void setDefaultValue (helics_subscription sub, const std::vector<double> &data)
    {
        // c++98 doesn't guarantee vector data will be contiguous --are there any reasonable implementations where it is not otherwise this should not do a copy?
        double *arr = (double*) malloc(data.size() * sizeof(double));
        for (unsigned int i = 0; i < data.size(); i++)
        {
            arr[i] = data[i];
        }
        // returns helicsStatus
        helicsSetDefaultVector (sub, arr, static_cast<int>(data.size() * sizeof(double)));
        free (arr);
    }

    /** Methods to get subscription values **/
    int getValue (helics_subscription sub, char *data, int maxlen)
    {
        return helicsGetValue (sub, data, maxlen);
    }

    std::string getString (helics_subscription sub)
    {
        char str[255];
        helicsGetString (sub, &str[0], sizeof(str));
        std::string result (str);
        return result;
    }

    int64_t getInteger (helics_subscription sub)
    {
        int64_t val;
        helicsGetInteger (sub, &val);
        return val;
    }

    double getDouble (helics_subscription sub)
    {
        double val;
        helicsGetDouble (sub, &val);
        return val;
    }

    std::complex<double> getComplex (helics_subscription sub)
    {
        double real;
        double imag;
        helicsGetComplex (sub, &real, &imag);
        std::complex<double> result (real, imag);
        return result;
    }

    int getVector (helics_subscription sub, double *data, int maxlen)
    {
        return helicsGetVector (sub, data, maxlen);
    }

    /** Methods to publish values **/
    void publish (helics_publication pub, const char *data, int len)
    {
        // returns helicsStatus
        helicsPublish (pub, data, len);
    }

    void publish (helics_publication pub, std::string str)
    {
        // returns helicsStatus
        helicsPublishString (pub, str.c_str());
    }

    void publish (helics_publication pub, int64_t val)
    {
        // returns helicsStatus
        helicsPublishInteger (pub, val);
    }

    void publish (helics_publication pub, double val)
    {
        // returns helicsStatus
        helicsPublishDouble (pub, val);
    }

    void publish (helics_publication pub, std::complex<double> cmplx)
    {
        // returns helicsStatus
        helicsPublishComplex (pub, cmplx.real(), cmplx.imag());
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
        // returns helicsStatus
        helicsPublishVector (pub, arr, data.size() * sizeof(double));
        free (arr);
    }

    /** Check if a subscription is updated **/
    bool isUpdated (helics_subscription sub) const
    {
        return helicsIsValueUpdated (sub) > 0;
    }

    /** Get the last time a subscription was updated **/
    helics_time_t getLastUpdateTime (helics_subscription sub) const
    {
        return helicsGetLastUpdateTime (sub);
    }

    // TODO: use c api to implement this method... callbacks too?
    /** Get a list of all subscriptions with updates since the last call **/
    std::vector<helics_subscription> queryUpdates () { return std::vector<helics_subscription>(); }
    // call helicsIsValueUpdated for each sub

    std::string getSubscriptionName (helics_subscription sub) const
    {
        char str[255];
        helicsGetSubscriptionKey (sub, &str[0], sizeof(str));
        std::string result (str);
        return result;
    }

    std::string getPublicationName (helics_publication pub) const
    {
        char str[255];
        helicsGetPublicationKey (pub, &str[0], sizeof(str));
        std::string result (str);
        return result;
    }

    std::string getSubscriptionUnits (helics_subscription sub) const
    {
        char str[255];
        helicsGetSubscriptionUnits (sub, &str[0], sizeof(str));
        std::string result (str);
        return result;
    }

    std::string getPublicationUnits (helics_publication pub) const
    {
        char str[255];
        helicsGetPublicationUnits (pub, &str[0], sizeof(str));
        std::string result (str);
        return result;
    }

    std::string getSubscriptionType (helics_subscription sub) const
    {
        char str[255];
        helicsGetSubscriptionType (sub, &str[0], sizeof(str));
        std::string result (str);
        return result;
    }

    std::string getPublicationType (helics_publication pub) const
    {
        char str[255];
        helicsGetPublicationType (pub, &str[0], sizeof(str));
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
