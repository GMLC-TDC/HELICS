/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Energy
Innovation LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <chrono>
#include <cstdint>
#include <exception>
#include <fstream>
#include <helics/application_api/Subscriptions.hpp>
#include <helics/application_api/ValueFederate.hpp>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#ifdef _WIN32
#    define NOMINMAX
#    include <psapi.h>
#    include <windows.h>
#endif

namespace {
struct MemoryInfo {
    std::uint64_t memTotalKb{0};
    std::uint64_t memAvailableKb{0};
};

std::int64_t getProcessMemoryInfoKb()
{
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS counters{};
    counters.cb = sizeof(counters);
    if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)) == 0) {
        return -1;
    }
    return static_cast<std::int64_t>(counters.WorkingSetSize / 1024U);
#else
    std::ifstream statusFile("/proc/self/status");
    std::string line;
    while (std::getline(statusFile, line)) {
        if (line.starts_with("VmRSS:")) {
            std::istringstream lineStream(line);
            std::string label;
            std::int64_t value = -1;
            std::string unit;
            lineStream >> label >> value >> unit;
            return value;
        }
    }
    return -1;
#endif
}

MemoryInfo getSystemMemoryInfo()
{
    MemoryInfo info;
#ifdef _WIN32
    MEMORYSTATUSEX status{};
    status.dwLength = sizeof(status);
    if (GlobalMemoryStatusEx(&status) != 0) {
        info.memTotalKb = status.ullTotalPhys / 1024U;
        info.memAvailableKb = status.ullAvailPhys / 1024U;
    }
#else
    std::ifstream meminfo("/proc/meminfo");
    std::string key;
    std::uint64_t value = 0;
    std::string unit;
    while (meminfo >> key >> value >> unit) {
        if (key == "MemTotal:") {
            info.memTotalKb = value;
        } else if (key == "MemAvailable:") {
            info.memAvailableKb = value;
        }
    }
#endif
    return info;
}

double parseDuration(int argc, char* argv[])
{
    if (argc < 2) {
        return 100000.0;
    }
    const double duration = std::stod(argv[1]);
    if (duration <= 0.0) {
        throw std::invalid_argument("duration must be greater than zero");
    }
    return duration;
}

unsigned int parseInitialDelayMs(int argc, char* argv[])
{
    if (argc < 4) {
        return 0;
    }
    const std::uint64_t delay = std::stoull(argv[3]);
    if (delay > 600000U) {
        throw std::invalid_argument("initial delay must not exceed 600000 ms");
    }
    return static_cast<unsigned int>(delay);
}
}  // namespace

int main(int argc, char* argv[])
{
    try {
        const double duration = parseDuration(argc, argv);
        const std::string config = (argc > 2) ? argv[2] : "sub_fed.json";
        const unsigned int initialDelayMs = parseInitialDelayMs(argc, argv);
        helics::ValueFederate fed(config);
        fed.setLoggingCallback([](int level, std::string_view source, std::string_view message) {
            if (level <= HELICS_LOG_LEVEL_WARNING) {
                std::cout << "[HELICS " << level << "] " << source << "::" << message << '\n'
                          << std::flush;
            }
        });
        fed.enterExecutingMode();

        if (initialDelayMs > 0U) {
            std::cout << "Delaying subscriber requests by " << initialDelayMs << " ms\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(initialDelayMs));
        }

        std::vector<helics::Input> inputs;
        const int inputCount = fed.getInputCount();
        inputs.reserve(static_cast<std::size_t>(inputCount));
        for (int i = 0; i < inputCount; ++i) {
            inputs.push_back(fed.getInput(i));
        }

        helics::Time time = 0.0;
        std::string value;
        while (time < duration) {
            time = fed.requestTime(time + 1.0);
            for (std::size_t i = 0; i < inputs.size(); ++i) {
                if (inputs[i].isUpdated()) {
                    value = inputs[i].getString();
                    if (i + 1U == inputs.size()) {
                        const MemoryInfo memory = getSystemMemoryInfo();
                        std::cout << "Subscriber Time " << time << " received value = " << value
                                  << " used system memory KB = "
                                  << (memory.memTotalKb - memory.memAvailableKb)
                                  << " used subscriber process memory KB = "
                                  << getProcessMemoryInfoKb() << '\n';
                    }
                }
            }
        }
        fed.finalize();
    }
    catch (const std::exception& error) {
        std::cerr << "Subscriber error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
