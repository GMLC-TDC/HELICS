/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#ifndef HELICS_CPP98_DATABUFFER_HPP_
#define HELICS_CPP98_DATABUFFER_HPP_

#include "helics/helics.h"
#include "helicsExceptions.hpp"

#include <string>
#include <vector>
#include <complex>

namespace helicscpp {
class DataBuffer {
  public:
    DataBuffer() HELICS_NOTHROW: buff(helicsCreateDataBuffer(0)) {}
    explicit DataBuffer(int capacity): buff(helicsCreateDataBuffer(capacity)) {}

    void toBytes(double val) { helicsDoubleToBytes(val, buff); }
    void toBytes(int64_t val) { helicsIntToBytes(val, buff); }
    void toBytes(const std::string& val) { helicsStringToBytes(val.c_str(), buff); }
    void toBytes(const std::vector<double>& val)
    {
        helicsVectorToBytes(val.data(), static_cast<int>(val.size()), buff);
    }
    void toBytes(const std::complex<double> val)
    {
        helicsComplexToBytes(val.real(),val.imag(), buff);
    }
    void toBytes(const double *vals, int size)
    {
        helicsVectorToBytes(vals, size, buff);
    }
    void toBytes(HelicsNamedPoint val)
    {
        helicsNamedPointToBytes(vals, size, buff);
    }
  private:
    HelicsDataBuffer buff;
};
}  // namespace helicscpp
#endif
