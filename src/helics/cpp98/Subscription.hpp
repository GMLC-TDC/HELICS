/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#ifndef HELICS_CPP98_SUBSCRIPTION_HPP_
#define HELICS_CPP98_SUBSCRIPTION_HPP_
#pragma once

#include "../shared_api_library/ValueFederate.h"

namespace helics98
{
class Subscription
{
public:
    explicit Subscription(helics_input hsub) :sub(hsub)
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

    operator helics_input() const { return sub; }

    helics_input baseObject() const { return sub; }
    /** Methods to set default values for subscriptions **/
    void setDefaultValue( const char *data, int len)
    {
        // returns helics_status
        helicsInputSetDefaultRaw(sub, data, len);
    }

    void setDefaultValue( const std::string &str)
    {
        // returns helics_status
        helicsInputSetDefaultString(sub, str.c_str());
    }

    void setDefaultValue( int64_t val)
    {
        // returns helics_status
        helicsInputSetDefaultInteger(sub, val);
    }

    void setDefaultValue(bool val)
    {
        // returns helics_status
        helicsInputSetDefaultBoolean(sub, val?helics_true:helics_false);
    }

    void setDefaultValue( double val)
    {
        // returns helics_status
        helicsInputSetDefaultDouble(sub, val);
    }

    void setDefaultValue( const std::complex<double> &cmplx)
    {
        // returns helics_status
        helicsInputSetDefaultComplex(sub, cmplx.real(), cmplx.imag());
    }

    void setDefaultValue( const std::vector<double> &data)
    {
        // returns helics_status
        helicsInputSetDefaultVector(sub, data.data(), static_cast<int>(data.size() * sizeof(double)));
    }

    /** Methods to get subscription values **/
    int getRawValue( std::vector<char> &data)
    {
        int size = helicsInputGetValueSize(sub);
        data.resize(size);
        int actualSize;
        helicsInputGetRawValue(sub, data.data(), static_cast<int>(data.size()), &actualSize);
        return actualSize;
    }

    int getValueSize()
    {
        return helicsInputGetValueSize(sub);
    }

    std::string getString()
    {
        int size = helicsInputGetValueSize(sub);
        std::string result;

        result.resize(size+1);
        //this function results in a null terminated string
        helicsInputGetString(sub, &result[0], size+1, &size);
        if (!(result.empty())&&(result[size-1] == '\0'))
        {
            result.resize(size - 1);
        }
        else
        {
            result.resize(size);
        }
        return result;
    }

    void getNamedPoint(std::string &name,double *val)
    {
        int size = helicsInputGetValueSize(sub);

        name.resize(size + 1);
        //this function results in a null terminated string
        helicsInputGetNamedPoint(sub, &name[0], size + 1, &size,val);
        name.resize(size);
    }

    int64_t getInteger()
    {
        int64_t val;
        helicsInputGetInteger(sub, &val);
        return val;
    }


    bool getBoolean()
    {
        helics_bool_t val;
        helicsInputGetBoolean(sub, &val);
        return (val==helics_true);
    }

    double getDouble()
    {
        double val;
        helicsInputGetDouble(sub, &val);
        return val;
    }

    std::complex<double> getComplex()
    {
        double real;
        double imag;
        helicsInputGetComplex(sub, &real, &imag);
        std::complex<double> result(real, imag);
        return result;
    }

    int getVector( double *data, int maxlen)
    {
        int actualSize;
        helicsInputGetVector(sub, data, maxlen, &actualSize);
        return actualSize;
    }

    void getVector( std::vector<double> &data)
    {
        int actualSize = helicsInputGetVectorSize(sub);
        data.resize(actualSize);
        helicsInputGetVector(sub, data.data(), actualSize, &actualSize);
    }

    /** Check if a subscription is updated **/
    bool isUpdated() const
    {
        return helicsInputIsUpdated(sub) > 0;
    }

    /** Get the last time a subscription was updated **/
    helics_time_t getLastUpdateTime() const
    {
        return helicsInputLastUpdateTime(sub);
    }

    // call helicsInputIsUpdated for each sub

    std::string getKey() const
    {
        char str[255];
        helicsInputGetKey(sub, &str[0], sizeof(str));
        std::string result(str);
        return result;
    }


    std::string getUnits() const
    {
        char str[255];
        helicsInputGetUnits(sub, &str[0], sizeof(str));
        std::string result(str);
        return result;
    }

    std::string getType() const
    {
        char str[255];
        helicsInputGetType(sub, &str[0], sizeof(str));
        std::string result(str);
        return result;
    }

private:
    helics_input sub;  //!< the reference to the underlying publication
};

}
#endif
