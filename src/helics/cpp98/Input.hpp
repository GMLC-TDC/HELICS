/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_INPUT_HPP_
#define HELICS_CPP98_INPUT_HPP_
#pragma once

#include "DataBuffer.hpp"
#include "helics/helics.h"
#include "helicsExceptions.hpp"

#include <complex>
#include <string>
#include <vector>

namespace helicscpp {
/** C++98 interface for a helics Input*/
class Input {
  public:
    /** construct from a HelicsInput object*/
    explicit Input(HelicsInput hsub) HELICS_NOTHROW: inp(hsub) {}
    /** default constructor*/
    Input() HELICS_NOTHROW: inp(HELICS_NULL_POINTER) {}
    /** copy constructor*/
    Input(const Input& inputs) HELICS_NOTHROW: inp(inputs.inp) {}
    /** copy assign*/
    Input& operator=(const Input& input)
    {
        inp = input.inp;
        return *this;
    }
    /** cast to HelicsInput object*/
    operator HelicsInput() const { return inp; }
    /** extract the base object*/
    HelicsInput baseObject() const { return inp; }
    /** check if the input is valid */
    bool isValid() const { return (helicsInputIsValid(inp) == HELICS_TRUE); }
    /** add a publication target to the input*/
    void addTarget(const std::string& target)
    {
        helicsInputAddTarget(inp, target.c_str(), HELICS_IGNORE_ERROR);
    }
    /** Methods to set default values for inputs **/
    /** set the default value as a raw data with length*/
    void setDefault(const char* data, int len)
    {
        helicsInputSetDefaultBytes(inp, data, len, HELICS_IGNORE_ERROR);
    }
    /** set the default value as a string*/
    void setDefault(const std::string& str)
    {
        helicsInputSetDefaultString(inp, str.c_str(), HELICS_IGNORE_ERROR);
    }
    /** set the default value as an integer*/
    void setDefault(int64_t val) { helicsInputSetDefaultInteger(inp, val, HELICS_IGNORE_ERROR); }
    /** set the default bool value*/
    void setDefault(bool val)
    {
        helicsInputSetDefaultBoolean(inp, val ? HELICS_TRUE : HELICS_FALSE, HELICS_IGNORE_ERROR);
    }
    /** set the default double value*/
    void setDefault(double val) { helicsInputSetDefaultDouble(inp, val, HELICS_IGNORE_ERROR); }
    /** set the default complex value*/
    void setDefault(const std::complex<double>& cmplx)
    {
        helicsInputSetDefaultComplex(inp, cmplx.real(), cmplx.imag(), HELICS_IGNORE_ERROR);
    }

    /** set the default complex vector data value*/
    void setDefault(const std::vector<double>& data)
    {
        helicsInputSetDefaultVector(inp,
                                    data.data(),
                                    static_cast<int>(data.size()),
                                    HELICS_IGNORE_ERROR);
    }

#if defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wstrict-aliasing"
    // std::complex is explicitly allowed to alias like this in the standard
#endif
    /** set the default complex vector data value*/
    void setDefault(const std::vector<std::complex<double> >& data)
    {
        helicsInputSetDefaultComplexVector(inp,
                                           reinterpret_cast<const double*>(data.data()),
                                           static_cast<int>(data.size()),
                                           HELICS_IGNORE_ERROR);
    }
#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif
    /** Methods to get subscription values **/
    /** get a raw value as a character vector*/
    int getBytes(std::vector<char>& data)
    {
        int size = helicsInputGetByteCount(inp);
        data.resize(size);
        helicsInputGetBytes(
            inp, data.data(), static_cast<int>(data.size()), &size, HELICS_IGNORE_ERROR);
        return size;
    }
    /** get the size of the raw value */
    int getByteCount() { return helicsInputGetByteCount(inp); }

    /** get the size of the value as a string */
    int getStringSize()
    {
        //-1 is for the null character which needs to be counted in C but not in a C++ string
        return helicsInputGetStringSize(inp) - 1;
    }

    /** get the current value as a string*/
    std::string getString()
    {
        int size = helicsInputGetStringSize(inp);
        std::string result;

        result.resize(static_cast<size_t>(size) + 1);
        // this function results in a null terminated string
        helicsInputGetString(inp, &result[0], size + 1, &size, HELICS_IGNORE_ERROR);
        if (!(result.empty()) && (result[static_cast<size_t>(size) - 1] == '\0')) {
            result.resize(static_cast<size_t>(size) - 1);
        } else {
            result.resize(size);
        }
        return result;
    }

    /** get the current value as a string*/
    void getString(std::string& str)
    {
        int size = helicsInputGetStringSize(inp);
        str.resize(static_cast<size_t>(size) + 1);
        // this function results in a null terminated string
        helicsInputGetString(inp, &str[0], size + 1, &size, HELICS_IGNORE_ERROR);
        if (!(str.empty()) && (str[static_cast<size_t>(size) - 1] == '\0')) {
            str.resize(static_cast<size_t>(size) - 1);
        } else {
            str.resize(size);
        }
    }

    /** get the current value as a named point*/
    void getNamedPoint(std::string& name, double* val)
    {
        int size = helicsInputGetStringSize(inp);

        name.resize(static_cast<size_t>(size) + 1);
        // this function results in a null terminated string
        helicsInputGetNamedPoint(inp, &name[0], size + 1, &size, val, HELICS_IGNORE_ERROR);
        name.resize(size);
    }
    /** get the current value as a 64 bit integer*/
    int64_t getInteger() { return helicsInputGetInteger(inp, HELICS_IGNORE_ERROR); }
    /** get the value as a boolean*/
    bool getBoolean()
    {
        HelicsBool val = helicsInputGetBoolean(inp, HELICS_IGNORE_ERROR);
        return (val == HELICS_TRUE);
    }
    /** get the value as a double*/
    double getDouble() { return helicsInputGetDouble(inp, HELICS_IGNORE_ERROR); }
    /** get the value as a complex number*/
    std::complex<double> getComplex()
    {
        HelicsComplex hc = helicsInputGetComplexObject(inp, HELICS_IGNORE_ERROR);
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
        // maxlen contains the actual length now
        return maxlen;
    }
    /** get the current value and store it in a std::vector<double>*/
    void getVector(std::vector<double>& data)
    {
        int actualSize = helicsInputGetVectorSize(inp);
        data.resize(actualSize);
        helicsInputGetVector(inp, data.data(), actualSize, HELICS_NULL_POINTER, hThrowOnError());
    }

    /** get the current value as a vector of doubles in alternating real and imaginary
    @param data pointer to space to store the current values
    @param maxlen the maximum size of the allowed vector
    @return the actual size of the vector stored*/
    int getComplexVector(double* data, int maxlen)
    {
        helicsInputGetComplexVector(inp, data, maxlen, &maxlen, hThrowOnError());
        // maxlen contains the actual length now
        return maxlen;
    }
#if defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wstrict-aliasing"
    // std::complex is explicitly allowed to alias like this in the standard
#endif
    /** get the current value and store it in a std::vector<std::complex<double>>*/
    void getComplexVector(std::vector<std::complex<double> >& data)
    {
        int actualSize = helicsInputGetVectorSize(inp);
        data.resize(actualSize);
        helicsInputGetComplexVector(inp,
                                    reinterpret_cast<double*>(data.data()),
                                    actualSize,
                                    HELICS_NULL_POINTER,
                                    hThrowOnError());
    }
#if defined(__GNUC__)
#    pragma GCC diagnostic pop
#endif
    /** get a data buffer with the input value*/
    DataBuffer getDataBuffer()
    {
        return DataBuffer(helicsInputGetDataBuffer(inp, hThrowOnError()));
    }
    /** Check if an input is updated **/
    bool isUpdated() const { return (helicsInputIsUpdated(inp) > 0); }

    /** Get the last time an input was updated **/
    HelicsTime getLastUpdateTime() const { return helicsInputLastUpdateTime(inp); }

    /** clear the updated flag*/
    void clearUpdate() { helicsInputClearUpdate(inp); }
    // call helicsInputIsUpdated for each inp

    /** get the Name for the input
   @details the name is full name vs the local one for non-global interfaces*/
    const char* getName() const { return helicsInputGetName(inp); }
    /** get the units associated with a input*/
    const char* getUnits() const { return helicsInputGetExtractionUnits(inp); }
    /** get the units associated with an inputs publication*/
    const char* getInjectionUnits() const { return helicsInputGetInjectionUnits(inp); }
    /** get the units associated with a publication of an input*/
    const char* getPublicationType() const { return helicsInputGetPublicationType(inp); }
    /** get the type of the input*/
    const char* getType() const { return helicsInputGetType(inp); }
    /** get an associated target*/
    const char* getTarget() const { return helicsInputGetTarget(inp); }
    /** get the interface information field of the input`*/
    const char* getInfo() const { return helicsInputGetInfo(inp); }
    /** set the interface information field of the input*/
    void setInfo(const std::string& info)
    {
        helicsInputSetInfo(inp, info.c_str(), HELICS_IGNORE_ERROR);
    }
    /** get the value of a tag for the input*/
    const char* getTag(const std::string& tagname) const
    {
        return helicsInputGetTag(inp, tagname.c_str());
    }
    /** set the value of a tag for the input*/
    void setTag(const std::string& tagname, const std::string& tagvalue)
    {
        helicsInputSetTag(inp, tagname.c_str(), tagvalue.c_str(), HELICS_IGNORE_ERROR);
    }
    void setOption(int32_t option, int32_t value = 1)
    {
        helicsInputSetOption(inp, option, value, HELICS_IGNORE_ERROR);
    }
    int32_t getOption(int32_t option) { return helicsInputGetOption(inp, option); }

  private:
    HelicsInput inp;  //!< the reference to the underlying publication
};

}  // namespace helicscpp
#endif
