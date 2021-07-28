/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <vector>
#include <string>
#include <utility>

namespace helics {
    class ProfilerBuffer {
  public:
        ~ProfilerBuffer();
    void addMessage(const std::string& data);
        void addMessage(std::string&& data);
    void writeFile();
        void setOutputFile(std::string fileName) { mFileName = std::move(fileName); }
  private:
        std::vector<std::string> mBuffers;
    std::string mFileName;
        
    };
}
