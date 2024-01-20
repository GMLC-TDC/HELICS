/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_DATABUFFER_HPP_
#define HELICS_CPP98_DATABUFFER_HPP_

#include "helics/helics.h"
#include "helicsExceptions.hpp"

#include <complex>
#include <string>
#include <vector>

namespace helicscpp {
class DataBuffer {
  public:
    DataBuffer() HELICS_NOTHROW: buff(helicsCreateDataBuffer(0)) {}
    explicit DataBuffer(int capacity): buff(helicsCreateDataBuffer(capacity)) {}
    /** create a dataBuffer object from an existing C API buffer*/
    explicit DataBuffer(HelicsDataBuffer buffer): buff(buffer) {}
    DataBuffer(void* buffer, int32_t datasize, int32_t capacity):
        buff(helicsWrapDataInBuffer(buffer, datasize, capacity))
    {
    }
    /** destructor*/
    ~DataBuffer() { helicsDataBufferFree(buff); }
    void fill(double val) { helicsDataBufferFillFromDouble(buff, val); }
    void fill(int64_t val) { helicsDataBufferFillFromInteger(buff, val); }
    void fill(const std::string& val) { helicsDataBufferFillFromString(buff, val.c_str()); }
    void fill(const char* val) { helicsDataBufferFillFromString(buff, val); }
    void fill(const std::vector<double>& val)
    {
        helicsDataBufferFillFromVector(buff, val.data(), static_cast<int>(val.size()));
    }
    void fill(const std::complex<double> val)
    {
        helicsDataBufferFillFromComplex(buff, val.real(), val.imag());
    }
    void fill(const double* vals, int size) { helicsDataBufferFillFromVector(buff, vals, size); }
    void fill(const std::string& name, double val)
    {
        helicsDataBufferFillFromNamedPoint(buff, name.c_str(), val);
    }
    void fill(bool val) { helicsDataBufferFillFromBoolean(buff, val ? HELICS_TRUE : HELICS_FALSE); }
    void fill(char val) { helicsDataBufferFillFromChar(buff, val); }
    /** make a deep copy of the buffer*/
    DataBuffer clone() { return DataBuffer(helicsDataBufferClone(buff)); }
    /** get the size of the raw value */
    int size() { return helicsDataBufferSize(buff); }

    /** get the size of the raw value */
    int capacity() { return helicsDataBufferCapacity(buff); }
    /** get a pointer to the raw data*/
    void* data() { return helicsDataBufferData(buff); }
    /** reserve a capacity in the buffer*/
    bool reserve(int32_t newCapacity)
    {
        return helicsDataBufferReserve(buff, newCapacity) == HELICS_TRUE;
    }
    /** get the size of the value as a string */
    int stringSize() { return helicsDataBufferStringSize(buff); }
    /** get the size of the value as a vector */
    int vectorSize() { return helicsDataBufferVectorSize(buff); }
    /** get the type of data contained in the buffer*/
    int type() const { return helicsDataBufferType(buff); }
    /** check if the buffer is valid*/
    bool isValid() const { return (helicsDataBufferIsValid(buff) == HELICS_TRUE); }
    /** get the current value as a string*/
    std::string toString()
    {
        int size = stringSize();
        std::string result;

        result.resize(static_cast<size_t>(size) + 1);
        // this function results in a null terminated string
        helicsDataBufferToString(buff, &result[0], size + 1, &size);
        if (!(result.empty()) && (result[static_cast<size_t>(size) - 1] == '\0')) {
            result.resize(static_cast<size_t>(size) - 1);
        } else {
            result.resize(size);
        }
        return result;
    }

    /** get the current value as a string*/
    void toString(std::string& str)
    {
        int size = stringSize();
        str.resize(static_cast<size_t>(size) + 1);
        // this function results in a null terminated string
        helicsDataBufferToString(buff, &str[0], size + 1, &size);
        if (!(str.empty()) && (str[static_cast<size_t>(size) - 1] == '\0')) {
            str.resize(static_cast<size_t>(size) - 1);
        } else {
            str.resize(size);
        }
    }

    /** get the current value as a named point*/
    void toNamedPoint(std::string& name, double* val)
    {
        int size = helicsDataBufferStringSize(buff);

        name.resize(static_cast<size_t>(size) + 1);
        // this function results in a null terminated string
        helicsDataBufferToNamedPoint(buff, &name[0], size + 1, &size, val);
        name.resize(size);
    }
    /** get the current value as a 64 bit integer*/
    int64_t toInt() { return helicsDataBufferToInteger(buff); }
    /** get the value as a boolean*/
    bool toBoolean()
    {
        HelicsBool val = helicsDataBufferToBoolean(buff);
        return (val == HELICS_TRUE);
    }
    /** get the value as a double*/
    double toDouble() { return helicsDataBufferToDouble(buff); }
    /** get the value as a complex number*/
    std::complex<double> toComplex()
    {
        HelicsComplex hc = helicsDataBufferToComplexObject(buff);
        std::complex<double> result(hc.real, hc.imag);
        return result;
    }
    /** get the current value as a vector of doubles
    @param data pointer to space to store the current values
    @param maxlen the maximum size of the allowed vector
    @return the actual size of the vector stored*/
    int toVector(double* data, int maxlen)
    {
        helicsDataBufferToVector(buff, data, maxlen, &maxlen);
        // maxlen contains the actual length now
        return maxlen;
    }
    /** get the current value and store it in a std::vector<double>*/
    void toVector(std::vector<double>& data)
    {
        int actualSize = helicsDataBufferVectorSize(buff);
        data.resize(actualSize);
        helicsDataBufferToVector(buff, data.data(), actualSize, HELICS_NULL_POINTER);
    }

    /** get the current value as a vector of doubles in alternating real and imaginary
    @param data pointer to space to store the current values
    @param maxlen the maximum size of the allowed vector
    @return the actual size of the vector stored*/
    int toComplexVector(double* data, int maxlen)
    {
        helicsDataBufferToComplexVector(buff, data, maxlen, &maxlen);
        // maxlen contains the actual length now
        return maxlen;
    }
    /** convert the data in a data buffer to a different type representation
    @param newDataType the type that it is desired for the buffer to be converted to
    @return true if the conversion was successful*/
    bool convertToType(int newDataType)
    {
        return (helicsDataBufferConvertToType(buff, newDataType) == HELICS_TRUE);
    }
    /** get the C API dataobject */
    HelicsDataBuffer getHelicsDataBuffer() { return buff; }

  private:
    HelicsDataBuffer buff;
};
}  // namespace helicscpp
#endif
