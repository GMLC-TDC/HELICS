/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#include "helicsVersion.hpp"

#include "../common/JsonProcessingFunctions.hpp"
#include "CoreFactory.hpp"

#if defined(HELICS_ENABLE_ZMQ_CORE) && !defined(USING_HELICS_C_SHARED_LIB)
#    include "helics/network/zmq/ZmqCommsCommon.h"
#endif

#include <thread>

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

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#    include <windows.h>

unsigned long long getTotalSystemMemory()
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
}

std::string os_info()
{
    using namespace std;
    NTSTATUS(WINAPI * RtlGetVersion)(LPOSVERSIONINFOEXW);
    OSVERSIONINFOEXW osInfo;
    std::string winVer = "WINDOWS ";
    *(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandleA("ntdll"), "RtlGetVersion");

    if (nullptr != RtlGetVersion) {
        osInfo.dwOSVersionInfoSize = sizeof(osInfo);
        RtlGetVersion(&osInfo);
        winVer +=
            std::to_string(osInfo.dwMajorVersion) + '.' + std::to_string(osInfo.dwMinorVersion);
    }
    return winVer;
}

#else
#    include <sys/utsname.h>
#    include <unistd.h>

unsigned long long getTotalSystemMemory()
{
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
}

std::string os_info()
{
    std::string osInfo;
    struct utsname details;

    int ret = uname(&details);

    if (ret == 0) {
        osInfo.append(details.sysname);
        osInfo.push_back(' ');
        osInfo.push_back(' ');
        osInfo.append(details.release);
        osInfo.push_back(' ');
        osInfo.push_back(' ');
        osInfo.append(details.version);
    } else {
        return "POSIX";
    }
#endif

std::string getHostName()
{
    char* temp{nullptr};
    std::string computerName;

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
    temp = getenv("COMPUTERNAME");
    if (temp != nullptr) {
        computerName = temp;
        temp = nullptr;
    }
#else
        temp = getenv("HOSTNAME");
        if (temp != nullptr) {
            computerName = temp;
            temp = nullptr;
        } else {
            temp = new char[512];
            if (gethostname(temp, 512) == 0) {  // success = 0, failure = -1
                computerName = temp;
            }
            delete[] temp;
            temp = nullptr;
        }
#endif
    return computerName;
}

namespace helics {

std::string extendedVersionInfo()
{
    Json::Value base;
    base["version"]["string"] = helics::versionString;
    base["version"]["major"] = helics::versionMajor;
    base["version"]["minor"] = helics::versionMinor;
    base["version"]["patch"] = helics::versionPatch;
    base["version"]["build"] = helics::versionBuild;
    base["buildflags"] = helics::buildFlags;
    base["compiler"] = helics::compiler;
    base["cores"] = Json::arrayValue;

    auto ctypesA = CoreFactory::getAvailableCoreTypes();
    for (const auto& ctype : ctypesA) {
        base["cores"].append(ctype);
    }
    auto cpumodel = getCPUModel();
    if (!cpumodel.empty()) {
        base["cpu"] = cpumodel;
    } else {
        base["cpu"] = "unknown";
    }
    base["cpucount"] = std::thread::hardware_concurrency();
    base["cputype"] = HELICS_BUILD_PROCESSOR;
    base["hostname"] = getHostName();
#if defined(HELICS_ENABLE_ZMQ_CORE) && !defined(USING_HELICS_C_SHARED_LIB)
    base["zmqversion"] = helics::zeromq::getZMQVersion();
#endif
    base["memory"] = getTotalSystemMemory();
    base["OS"] = os_info();
    return fileops::generateJsonString(base);
}

}  // namespace helics
