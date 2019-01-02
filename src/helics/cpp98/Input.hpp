/*
Copyright © 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#ifndef HELICS_CPP98_INPUT_HPP_
#define HELICS_CPP98_INPUT_HPP_
#pragma once

#include "../shared_api_library/ValueFederate.h"
#include "helicsExceptions.hpp"

namespace helicscpp
{
class Input
{
  public:
    explicit Input (helics_input hsub) : inp (hsub) {}
    Input (){};

    Input (const Input &inputs) : inp (inputs.inp) {}

    Input &operator= (const Input &input)
    {
        inp = input.inp;
        return *this;
    }

    operator helics_input () const { return inp; }

    helics_input baseObject () const { return inp; }
    /** Methods to set default values for subscriptions **/
    void setDefaultValue (const char *data, int len)
    {
        // returns helics_status
        helicsInputSetDefaultRaw (inp, data, len, NULL);
    }

    void setDefaultValue (const std::string &str)
    {
        // returns helics_status
        helicsInputSetDefaultString (inp, str.c_str (), NULL);
    }

    void setDefaultValue (int64_t val)
    {
        // returns helics_status
        helicsInputSetDefaultInteger (inp, val, NULL);
    }

    void setDefaultValue (bool val)
    {
        // returns helics_status
        helicsInputSetDefaultBoolean (inp, val ? helics_true : helics_false, NULL);
    }

    void setDefaultValue (double val)
    {
        // returns helics_status
        helicsInputSetDefaultDouble (inp, val, NULL);
    }

    void setDefaultValue (const std::complex<double> &cmplx)
    {
        // returns helics_status
        helicsInputSetDefaultComplex (inp, cmplx.real (), cmplx.imag (), NULL);
    }

    void setDefaultValue (const std::vector<double> &data)
    {
        // returns helics_status
        helicsInputSetDefaultVector (inp, data.data (), static_cast<int> (data.size () * sizeof (double)), NULL);
    }

    /** Methods to get subscription values **/
    int getRawValue (std::vector<char> &data)
    {
        int size = helicsInputGetRawValueSize (inp);
        data.resize (size);
        helicsInputGetRawValue (inp, data.data (), static_cast<int> (data.size ()), &size, NULL);
        return size;
    }

    int getRawValueSize () { return helicsInputGetRawValueSize (inp); }

    std::string getString ()
    {
        int size = helicsInputGetStringSize (inp);
        std::string result;

        result.resize (size + 1);
        // this function results in a null terminated string
        helicsInputGetString (inp, &result[0], size + 1, &size, NULL);
        if (!(result.empty ()) && (result[size - 1] == '\0'))
        {
            result.resize (size - 1);
        }
        else
        {
            result.resize (size);
        }
        return result;
    }

    void getNamedPoint (std::string &name, double *val)
    {
        int size = helicsInputGetStringSize (inp);

        name.resize (size + 1);
        // this function results in a null terminated string
        helicsInputGetNamedPoint (inp, &name[0], size + 1, &size, val, NULL);
        name.resize (size);
    }

    int64_t getInteger () { return helicsInputGetInteger (inp, NULL); }

    bool getBoolean ()
    {
        helics_bool val = helicsInputGetBoolean (inp, NULL);
        return (val == helics_true);
    }

    double getDouble () { return helicsInputGetDouble (inp, NULL); }

    std::complex<double> getComplex ()
    {
        helics_complex hc = helicsInputGetComplexObject (inp, NULL);
        std::complex<double> result (hc.real, hc.imag);
        return result;
    }

    int getVector (double *data, int maxlen)
    {
        helicsInputGetVector (inp, data, maxlen, &maxlen, hThrowOnError ());
        return maxlen;
    }

    void getVector (std::vector<double> &data)
    {
        int actualSize = helicsInputGetVectorSize (inp);
        data.resize (actualSize);
        helicsInputGetVector (inp, data.data (), actualSize, NULL, hThrowOnError ());
    }

    /** Check if a subscription is updated **/
    bool isUpdated () const { return (helicsInputIsUpdated (inp) > 0); }

    /** Get the last time a subscription was updated **/
    helics_time getLastUpdateTime () const { return helicsInputLastUpdateTime (inp); }

    // call helicsInputIsUpdated for each inp

    const char *getKey () const { return helicsInputGetKey (inp); }

    const char *getUnits () const { return helicsInputGetUnits (inp); }

    const char *getType () const { return helicsInputGetType (inp); }

    const char *getTarget () const { return helicsSubscriptionGetKey (inp); }

  private:
    helics_input inp;  //!< the reference to the underlying publication
};

}  // namespace helicscpp
#endif
