/*
Copyright (c) 2017-2022,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <string>
#include <utility>
#include <vector>

namespace helics {
class ProfilerBuffer {
  public:
    ~ProfilerBuffer();
    void addMessage(const std::string& data);
    void addMessage(std::string&& data);
    void writeFile();
    /** specify the output file for writing the profile too
    @details the file will be cleared when the ouput file is set unless the append parameter is specified as true
    @param fileName the name of the file to write the profile too
    @param append if set to true the output file will be appended insted of cleared on first use
    */
    void setOutputFile(std::string fileName, bool append=false);

  private:
    std::vector<std::string> mBuffers;
    std::string mFileName;
};
}  // namespace helics
