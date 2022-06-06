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

namespace helicscpp {
class DataBuffer {
  public:
    DataBuffer() { buff = helicsCreateDataBuffer(0); }
    explicit DataBuffer(int capacity) { buff = helicsCreateDataBuffer(capacity); }

    void toBytes(double val) { helicsDoubleToBytes(val, buff); }
    void toBytes(int64_t val) { helicsIntToBytes(val, buff); }
    void toBytes(const std::string& val) { helicsStringToBytes(val.c_str(), buff); }
    void toBytes(const std::vector<double> val)
    {
        helicsVectorToBytes(val.data(), static_cast<int>(val.size()), buff);
    }

  private:
    HelicsDataBuffer buff = nullptr;
};
}  // namespace helicscpp
#endif
