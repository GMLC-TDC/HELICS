/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "LogBuffer.hpp"

namespace helics {
    void LogBuffer::resize(std::size_t newSize) {
        mMaxSize = newSize;
        while (mBuffer.size()>mMaxSize) {
            mBuffer.pop_front();
        }
        
    }

    void LogBuffer::push(int logLevel, std::string header, std::string message) {
        if (mBuffer.size()==mMaxSize) {
            mBuffer.pop_front();
        }
        mBuffer.emplace_back(logLevel, std::move(header), std::move(message));
    }

}
