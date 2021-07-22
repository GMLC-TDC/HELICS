/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/


#include "ProfilerBuffer.hpp"
namespace helics {

    void ProfilerBuffer::addMessage(const std::string& data) {
        buffers.emplace_back(data);
    }

    void ProfilerBuffer::addMessage(std::string&& data) {
        buffers.push_back(std::move(data));
    }


}  // namespace helics
