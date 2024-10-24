/*
Copyright (c) 2017-2024,
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

#include <string>
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
#    include <fstream>
inline std::string getCPUModel()
{  // Get the cpu from /proc/cpuinfo
    std::ifstream cpufile("/proc/cpuinfo");
    if (!cpufile) {
        return std::string{};
    }
    std::string info;
    std::string line;
    while (getline(cpufile, line)) {
        if (line.find("model name") != std::string::npos) {
            info.append(line);
            break;
        }
    }
    cpufile.close();
    auto modelLoc = info.find("model name");

    if (modelLoc != std::string::npos) {
        auto cloc = info.find_first_of(':', modelLoc);
        auto eline = info.find_first_of("\n\r\0", modelLoc);
        auto modelString = info.substr(cloc + 1, eline - cloc - 1);
        if (modelString.back() == '\0') {
            modelString.pop_back();
        }
        return modelString;
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
        info.resize(name_sz, '\0');
        if (sysctlbyname("machdep.cpu.brand_string", info.data(), &name_sz, nullptr, 0) != 0) {
            info = "UNKNOWN";
        }
    }
    while (!info.empty() && info.back() == '\0') {
        info.pop_back();
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

std::uint64_t getTotalSystemMemory()
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return status.ullTotalPhys;
}

std::string os_info()
{
    NTSTATUS(WINAPI * RtlGetVersion)(LPOSVERSIONINFOEXW);
    OSVERSIONINFOEXW osInfo;
    std::string winVer = "WINDOWS ";
    auto module = GetModuleHandleA("ntdll");
    if (module != nullptr) {
        *(reinterpret_cast<FARPROC*>(&RtlGetVersion)) = GetProcAddress(module, "RtlGetVersion");

        if (nullptr != RtlGetVersion) {
            osInfo.dwOSVersionInfoSize = sizeof(osInfo);
            RtlGetVersion(&osInfo);
            winVer +=
                std::to_string(osInfo.dwMajorVersion) + '.' + std::to_string(osInfo.dwMinorVersion);
        }
    }
    return winVer;
}

#else
#    include <sys/utsname.h>
#    include <unistd.h>

std::uint64_t getTotalSystemMemory()
{
    const std::uint64_t pages = sysconf(_SC_PHYS_PAGES);
    const std::uint64_t page_size = sysconf(_SC_PAGE_SIZE);
    return pages * page_size;
}

std::string os_info()
{
    std::string osInfo;
    utsname details = {};

    const int ret = uname(&details);

    if (ret == 0) {
        osInfo.append(details.sysname);
        osInfo.push_back(' ');
        osInfo.push_back(' ');
        osInfo.append(details.release);
        osInfo.push_back(' ');
        osInfo.push_back(' ');
        osInfo.append(details.version);
    } else {
        osInfo = "POSIX";
    }
    return osInfo;
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
#    if !defined(__CYGWIN__)
        temp = new char[512];
        if (gethostname(temp, 512) == 0) {  // success = 0, failure = -1
            computerName = temp;
        }
        delete[] temp;
#    endif
        temp = nullptr;
    }
#endif
    return computerName;
}

namespace helics::core {

std::string systemInfo()
{
    nlohmann::json base;
    base["version"]["string"] = helics::versionString;
    base["version"]["major"] = helics::versionMajor;
    base["version"]["minor"] = helics::versionMinor;
    base["version"]["patch"] = helics::versionPatch;
    base["version"]["build"] = helics::versionBuild;
    base["buildflags"] = helics::buildFlags;
    base["compiler"] = helics::compiler;
    base["cores"] = nlohmann::json::array();

    auto ctypesA = CoreFactory::getAvailableCoreTypes();
    for (const auto& ctype : ctypesA) {
        base["cores"].push_back(ctype);
    }
    auto cpumodel = getCPUModel();
    if (!cpumodel.empty()) {
        if (cpumodel.back() == ' ' || cpumodel.back() == '\n' || cpumodel.back() == '\0') {
            cpumodel.pop_back();
        }
        base["cpu"] = cpumodel;
    } else {
        base["cpu"] = "UNKNOWN";
    }
    base["cpucount"] = std::thread::hardware_concurrency();
    base["cputype"] = HELICS_BUILD_PROCESSOR;
    base["hostname"] = getHostName();
#if defined(HELICS_ENABLE_ZMQ_CORE) && !defined(USING_HELICS_C_SHARED_LIB)
    base["zmqversion"] = helics::zeromq::getZMQVersion();
#endif
    auto memory = getTotalSystemMemory();
    base["memory"] = std::to_string(memory / (1024ULL * 1024ULL)) + " MB";
    base["os"] = os_info();
    return fileops::generateJsonString(base);
}

}  // namespace helics::core
