/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <deque>
#include <string>

namespace helics {
    class ProfilerBuffer {
    void addMessage(const std::string& data);
        void addMessage(std::string&& data);
  private:
        std::deque<std::string> buffers;

    };
}
