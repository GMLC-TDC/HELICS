/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <deque>
#include <utility>
#include <string>

namespace helics
{
/** defining a small buffer for log messages*/
class LogBuffer {
  private:
    std::deque<std::tuple<int, std::string, std::string>> mBuffer;
    std::size_t mMaxSize{0};
  public:
    void resize(std::size_t newSize);
    std::size_t capacity() const { return mMaxSize; }
    std::size_t size() const { return mBuffer.size(); }

    void push(int logLevel, std::string header, std::string message);

    auto begin() const { return mBuffer.begin(); }
    auto end() const { return mBuffer.end(); }
};
}
