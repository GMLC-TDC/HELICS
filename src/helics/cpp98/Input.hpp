/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_INPUT_HPP_
#define HELICS_CPP98_INPUT_HPP_
#pragma once

#include "../shared_api_library/ValueFederate.h"
#include "helicsExceptions.hpp"

namespace helicscpp {
/** C++98 interface for a helics Input*/
class Input {
  public:
    /** construct from a helics_input object*/
    explicit Input(helics_input hsub) HELICS_NOTHROW: inp(hsub) {}
    /** default constructor*/
    Input() HELICS_NOTHROW: inp(HELICS_NULL_POINTER){};
    /** copy constructor*/
    Input(const Input& inputs) HELICS_NOTHROW: inp(inputs.inp) {}
    /** copy assign*/
    Input& operator=(const Input& input)
    {
        inp = input.inp;
        return *this;
    }
    /** cast to helics_input object*/
    operator helics_input() const { return inp; }
    /** extract the base object*/
    helics_input baseObject() const { return inp; }
    /** Methods to set default values for inputs **/
    /** set the default value as a raw data with length*/
    void setDefault(const char* data, int len) { helicsInputSetDefaultRaw(inp, data, len, NULL); }
    /** set the default value as a string*/
    void setDefault(const std::string& str) { helicsInputSetDefaultString(inp, str.c_str(), NULL); }
    /** set the default value as an integer*/
    void setDefault(int64_t val) { helicsInputSetDefaultInteger(inp, val, NULL); }
    /** set the default bool value*/
    void setDefault(bool val)
    {
        helicsInputSetDefaultBoolean(inp, val ? helics_true : helics_false, NULL);
    }
    /** set the default double value*/
    void setDefault(double val) { helicsInputSetDefaultDouble(inp, val, NULL); }
    /** set the default complex value*/
    void setDefault(const std::complex<double>& cmplx)
    {
        helicsInputSetDefaultComplex(inp, cmplx.real(), cmplx.imag(), NULL);
    }
    /** set the default vector data value*/
    void setDefault(const std::vector<double>& data)
    {
        helicsInputSetDefaultVector(
            inp, data.data(), static_cast<int>(data.size() * sizeof(double)), NULL);
    }

    /** Methods to get subscription values **/
    /** get a raw value as a character vector*/
    int getRawValue(std::vector<char>& data)
    {
        int size = helicsInputGetRawValueSize(inp);
        data.resize(size);
        helicsInputGetRawValue(inp, data.data(), static_cast<int>(data.size()), &size, NULL);
        return size;
    }
    /** get the size of the raw value */
    int getRawValueSize() { return helicsInputGetRawValueSize(inp); }

    /** get the current value as a string*/
    std::string getString()
    {
        int size = helicsInputGetStringSize(inp);
        std::string result;

        result.resize(static_cast<size_t>(size) + 1);
        // this function results in a null terminated string
        helicsInputGetString(inp, &result[0], size + 1, &size, NULL);
        if (!(result.empty()) && (result[static_cast<size_t>(size) - 1] == '\0')) {
            result.resize(static_cast<size_t>(size) - 1);
        } else {
            result.resize(size);
        }
        return result;
    }
    /** get the current value as a named point*/
    void getNamedPoint(std::string& name, double* val)
    {
        int size = helicsInputGetStringSize(inp);

        name.resize(static_cast<size_t>(size) + 1);
        // this function results in a null terminated string
        helicsInputGetNamedPoint(inp, &name[0], size + 1, &size, val, NULL);
        name.resize(size);
    }
    /** get the current value as a 64 bit integer*/
    int64_t getInteger() { return helicsInputGetInteger(inp, NULL); }
    /** get the value as a boolean*/
    bool getBoolean()
    {
        helics_bool val = helicsInputGetBoolean(inp, NULL);
        return (val == helics_true);
    }
    /** get the value as a double*/
    double getDouble() { return helicsInputGetDouble(inp, NULL); }
    /** get the value as a complex number*/
    std::complex<double> getComplex()
    {
        helics_complex hc = helicsInputGetComplexObject(inp, NULL);
        std::complex<double> result(hc.real, hc.imag);
        return result;
    }
    /** get the current value as a vector of doubles
	@param data pointer to space to store the current values
	@param maxlen the maximum size of the allowed vector
	@return the actual size of the vector stored*/
    int getVector(double* data, int maxlen)
    {
        helicsInputGetVector(inp, data, maxlen, &maxlen, hThrowOnError());
        return maxlen;
    }
    /** get the current value and store it in a std::vector<double>*/
    void getVector(std::vector<double>& data)
    {
        int actualSize = helicsInputGetVectorSize(inp);
        data.resize(actualSize);
        helicsInputGetVector(inp, data.data(), actualSize, NULL, hThrowOnError());
    }

    /** Check if an input is updated **/
    bool isUpdated() const { return (helicsInputIsUpdated(inp) > 0); }

    /** Get the last time an input was updated **/
    helics_time getLastUpdateTime() const { return helicsInputLastUpdateTime(inp); }

    /** clear the updated flag*/
    void clearUpdate() { helicsInputClearUpdate(inp); }
    // call helicsInputIsUpdated for each inp

    /** get the Name/Key for the input
   @details the name is the local name if given, key is the full key name*/
    const char* getKey() const { return helicsInputGetKey(inp); }
    /** get the units associated with a input*/
    const char* getUnits() const { return helicsInputGetExtractionUnits(inp); }
    /** get the units associated with a input*/
    const char* getInjectionUnits() const { return helicsInputGetInjectionUnits(inp); }
    /** get the type of the input*/
    const char* getType() const { return helicsInputGetType(inp); }
    /** get an associated target*/
    const char* getTarget() const { return helicsSubscriptionGetKey(inp); }

  private:
    helics_input inp; //!< the reference to the underlying publication
};

} // namespace helicscpp
#endif
