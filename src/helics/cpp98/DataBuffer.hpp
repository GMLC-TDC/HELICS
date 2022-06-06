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

namespace helicscpp {
    class DataBuffer {
  public:
        DataBuffer() { buff = helicsCreateDataBuffer(0); }
    explicit DataBuffer(int capacity) { buff = helicsCreateDataBuffer(capacity); }

  private:
        HelicsDataBuffer buff = nullptr;
    };
}
#endif
