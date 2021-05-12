/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "helics/core/helicsVersion.hpp"

#include <iostream>
#include <string>
#include <thread>

#if defined(ENABLE_ZMQ_CORE) && !defined(USING_HELICS_C_SHARED_LIB)
#    include "helics/network/zmq/ZmqCommsCommon.h"
#endif

#if defined(_WIN32) || defined(WIN32)
#    include <intrin.h>
// code modified from https://weseetips.wordpress.com/tag/c-get-cpu-name/
inline std::string getCPUModel()
{  // Get extended ids.
    int CPUInfo[4] = {-1};
    __cpuid(CPUInfo, 0x80000000);
    unsigned int nExIds = CPUInfo[0];

    // Get the information associated with each extended ID.
    char CPUBrandString[0x40] = {0};
    for (unsigned int i = 0x80000000; i <= nExIds; ++i) {
        __cpuid(CPUInfo, i);

        // Interpret CPU brand string and cache information.
        if (i == 0x80000002) {
            memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
        } else if (i == 0x80000003) {
            memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
        } else if (i == 0x80000004) {
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
        }
    }

    return std::string(CPUBrandString);
}
#elif defined(__unix__)
#    include <cstdio>
#    include <cstdlib>
#    include <cstring>
inline std::string getCPUModel()
{  // Get the cpu from /proc/cpuinfo
    FILE* fp = fopen("/proc/cpuinfo", "r");
    if (fp == nullptr) {
        return std::string{};
    }
    size_t n{0};
    char* line{nullptr};
    std::string info;
    while (getline(&line, &n, fp) > 0) {
        if (strstr(line, "model name") != nullptr) {
            info.append(line);
            break;
        }
    }
    free(line);
    fclose(fp);
    auto modelLoc = info.find("model name");

    if (modelLoc != std::string::npos) {
        auto cloc = info.find_first_of(':', modelLoc);
        auto eline = info.find_first_of("\n\r\0", modelLoc);
        return info.substr(cloc + 1, eline - cloc - 1);
    }
    return std::string{};
}
#elif defined(__APPLE__)
#    include <cstdlib>
#    include <sys/sysctl.h>
#    include <sys/types.h>
inline std::string getCPUModel()
{
    std::string info;
    size_t name_sz = 0;
    if (!sysctlbyname("machdep.cpu.brand_string", nullptr, &name_sz, nullptr, 0)) {
        char* buffer = static_cast<char*>(malloc(name_sz));
        if (!sysctlbyname("machdep.cpu.brand_string", buffer, &name_sz, nullptr, 0)) {
            info = std::string(buffer, name_sz);
        }
        free(buffer);
    }
    return info;
}
#else
inline std::string getCPUModel()
{
    return std::string{};
}
#endif
/** Generate a report of the compilers used and zmq version linked as well as the version info for
 * HELICS for use with the benchmarks
 */
inline void printHELICSsystemInfo()
{
    std::cout << "------------HELICS BUILD INFO -------------\nHELICS VERSION: "
              << HELICS_VERSION_STRING << '\n';
#if defined(ENABLE_ZMQ_CORE) && !defined(USING_HELICS_C_SHARED_LIB)
    std::cout << "ZMQ VERSION: " << helics::zeromq::getZMQVersion() << '\n';
#endif
    std::cout << "COMPILER INFO: " << helics::compiler << '\n';
    std::cout << "BUILD FLAGS: " << helics::buildFlags << '\n';
    std::cout << "------------PROCESSOR INFO ----------------\n";
    std::cout << "HOST PROCESSOR TYPE: " << HELICS_BUILD_PROCESSOR << '\n';
    auto cpumodel = getCPUModel();
    if (!cpumodel.empty()) {
        std::cout << "CPU MODEL: " << cpumodel << '\n';
    }
    std::cout << "NUM CPU:" << std::thread::hardware_concurrency() << '\n';
    std::cout << "-------------------------------------------" << std::endl;
}
