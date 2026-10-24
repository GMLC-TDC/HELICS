/*
Copyright (c) 2017-2026,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Energy
Innovation LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include <cstdint>
#include <exception>
#include <helics/application_api/Publications.hpp>
#include <helics/application_api/ValueFederate.hpp>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#    define NOMINMAX
#    include <psapi.h>
#    include <windows.h>
#else
#    include <fstream>
#    include <sstream>
#endif

namespace {
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
}  // namespace

int main(int argc, char* argv[])
{
    try {
        const double duration = parseDuration(argc, argv);
        const std::string config = (argc > 2) ? argv[2] : "pub_fed.json";
        helics::ValueFederate fed(config);
        std::cout << "Loaded publisher config " << config << '\n';

        std::vector<helics::Publication> pubs;
        const int pubCount = fed.getPublicationCount();
        pubs.reserve(static_cast<std::size_t>(pubCount));
        for (int i = 0; i < pubCount; ++i) {
            pubs.push_back(fed.getPublication(i));
        }
        std::cout << "pubCount = " << pubCount << '\n';

        fed.enterExecutingMode();
        helics::Time time = 0.0;
        while (time < duration) {
            if (static_cast<int>(time) % 60 == 0) {
                std::cout << "Publisher sent values at time " << time
                          << " publisher process memory usage = " << getProcessMemoryInfoKb()
                          << " KiB\n"
                          << std::flush;
            }
            for (std::size_t i = 0; i < pubs.size(); ++i) {
                // Keep the original issue reproducer's publication behavior.
                pubs[i].publish(static_cast<double>(i) + time);
            }
            time = fed.requestTime(time + 1.0);
        }
        fed.finalize();
    }
    catch (const std::exception& error) {
        std::cerr << "Publisher error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
