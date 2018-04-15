/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#ifndef HELICS_CPP98_SUBSCRIPTION_HPP_
#define HELICS_CPP98_SUBSCRIPTION_HPP_
#pragma once

#include "../shared_api_library/ValueFederate.h"

namespace helics
{
class Subscription
{
public:
    explicit Subscription(helics_subscription hsub) :sub(hsub)
    {
    }
    Subscription() {};

    Subscription(const Subscription &subscription) :sub(subscription.sub)
    {
    }

    Subscription &operator=(const Subscription &subscription)
    {
        sub = subscription.sub;
        return *this;
    }

    operator helics_subscription() const { return sub; }

    helics_subscription baseObject() const { return sub; }
    /** Methods to set default values for subscriptions **/
    void setDefaultValue( const char *data, int len)
    {
        // returns helics_status
        helicsSubscriptionSetDefaultRaw(sub, data, len);
    }

    void setDefaultValue( const std::string &str)
    {
        // returns helics_status
        helicsSubscriptionSetDefaultString(sub, str.c_str());
    }

    void setDefaultValue( int64_t val)
    {
        // returns helics_status
        helicsSubscriptionSetDefaultInteger(sub, val);
    }

    void setDefaultValue( double val)
    {
        // returns helics_status
        helicsSubscriptionSetDefaultDouble(sub, val);
    }

    void setDefaultValue( const std::complex<double> &cmplx)
    {
        // returns helics_status
        helicsSubscriptionSetDefaultComplex(sub, cmplx.real(), cmplx.imag());
    }

    void setDefaultValue( const std::vector<double> &data)
    {
        // returns helics_status
        helicsSubscriptionSetDefaultVector(sub, data.data(), static_cast<int>(data.size() * sizeof(double)));
    }

    /** Methods to get subscription values **/
    int getRawValue( std::vector<char> &data)
    {
        int size = helicsSubscriptionGetValueSize(sub);
        data.resize(size);
        int actualSize;
        helicsSubscriptionGetRawValue(sub, data.data(), static_cast<int>(data.size()), &actualSize);
        return actualSize;
    }

    int getValueSize()
    {
        return helicsSubscriptionGetValueSize(sub);
    }

    std::string getString()
    {
        int size = helicsSubscriptionGetValueSize(sub);
        std::string result;

        result.resize(size+1);
        //this function results in a null terminated string
        helicsSubscriptionGetString(sub, &result[0], size+1, &size);
        result.resize(size);
        return result;
    }

    int64_t getInteger()
    {
        int64_t val;
        helicsSubscriptionGetInteger(sub, &val);
        return val;
    }

    double getDouble()
    {
        double val;
        helicsSubscriptionGetDouble(sub, &val);
        return val;
    }

    std::complex<double> getComplex()
    {
        double real;
        double imag;
        helicsSubscriptionGetComplex(sub, &real, &imag);
        std::complex<double> result(real, imag);
        return result;
    }

    int getVector( double *data, int maxlen)
    {
        int actualSize;
        helicsSubscriptionGetVector(sub, data, maxlen, &actualSize);
        return actualSize;
    }

    void getVector( std::vector<double> &data)
    {
        int actualSize = helicsSubscriptionGetVectorSize(sub);
        data.resize(actualSize);
        helicsSubscriptionGetVector(sub, data.data(), actualSize, &actualSize);
    }

    /** Check if a subscription is updated **/
    bool isUpdated() const
    {
        return helicsSubscriptionIsUpdated(sub) > 0;
    }

    /** Get the last time a subscription was updated **/
    helics_time_t getLastUpdateTime() const
    {
        return helicsSubscriptionLastUpdateTime(sub);
    }

    // call helicsSubscriptionIsUpdated for each sub

    std::string getKey() const
    {
        char str[255];
        helicsSubscriptionGetKey(sub, &str[0], sizeof(str));
        std::string result(str);
        return result;
    }


    std::string getUnits() const
    {
        char str[255];
        helicsSubscriptionGetUnits(sub, &str[0], sizeof(str));
        std::string result(str);
        return result;
    }

    std::string getType() const
    {
        char str[255];
        helicsSubscriptionGetType(sub, &str[0], sizeof(str));
        std::string result(str);
        return result;
    }

private:
    helics_subscription sub;  //!< the reference to the underlying publication
};

}
#endif
