/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include "helics/helics-config.h"
#include <benchmark/benchmark.h>
#include <iostream>
#ifdef ENABLE_ZMQ_CORE
#    include "helics/core/zmq/ZmqCommsCommon.h"
#endif

// Helper macro to create a main routine in a test that runs the benchmarks
#define HELICS_BENCHMARK_MAIN(label)                                                                              \
    int main (int argc, char **argv)                                                                              \
    {                                                                                                             \
        ::benchmark::Initialize (&argc, argv);                                                                    \
        if (::benchmark::ReportUnrecognizedArguments (argc, argv))                                                \
            return 1;                                                                                             \
        std::cout << "#benchmark:" << #label << '\n';                                                             \
        printHELICSsystemInfo ();                                                                                 \
        ::benchmark::RunSpecifiedBenchmarks ();                                                                   \
    }                                                                                                             \
    int main (int, char **)

#if defined(_WIN32) || defined(WIN32)
#    include <intrin.h>
// code modified from https://weseetips.wordpress.com/tag/c-get-cpu-name/
inline std::string getCPUModel ()
{  // Get extended ids.
    int CPUInfo[4] = {-1};
    __cpuid (CPUInfo, 0x80000000);
    unsigned int nExIds = CPUInfo[0];

    // Get the information associated with each extended ID.
    char CPUBrandString[0x40] = {0};
    for (unsigned int i = 0x80000000; i <= nExIds; ++i)
    {
        __cpuid (CPUInfo, i);

        // Interpret CPU brand string and cache information.
        if (i == 0x80000002)
        {
            memcpy (CPUBrandString, CPUInfo, sizeof (CPUInfo));
        }
        else if (i == 0x80000003)
        {
            memcpy (CPUBrandString + 16, CPUInfo, sizeof (CPUInfo));
        }
        else if (i == 0x80000004)
        {
            memcpy (CPUBrandString + 32, CPUInfo, sizeof (CPUInfo));
        }
    }

    return std::string (CPUBrandString);
}
#elif defined(__unix__)
#    include <cstdio>
#    include <cstdlib>
inline std::string getCPUModel ()
{  // Get extended ids.
    FILE *fp = fopen ("/proc/cpuinfo", "r");
    if (fp==nullptr)
    {
        return std::string{};
	}
    size_t n = 0;
    char *line = NULL;
    std::string info;
    while (getline (&line, &n, fp) > 0)
    {
        if (strstr (line, "model name"))
        {
            info.append (line);
            break;
        }
    }
    free (line);
    fclose (fp);
    auto modelLoc = info.find ("model name");

    if (modelLoc != std::string::npos)
    {
        auto cloc = info.find_first_of (':', modelLoc);
        auto eline = info.find_first_of ("\n\r\0", modelLoc);
        return info.substr (cloc + 1, eline - cloc - 1);
    }
    return std::string{};
}

#else
inline std::string getCPUModel ()
{  // Get extended ids.
    return std::string{HELICS_BUILD_PROCESSOR};
}
#endif
/// Generate a report of the compilers used and zmq version linked as well as the version info for HELICS
/// for use with the benchmarks
inline void printHELICSsystemInfo ()
{
    std::cout << "------------HELICS BUILD INFO -------------\nHELICS " << HELICS_VERSION_STRING << '\n';
#ifdef ENABLE_ZMQ_CORE
    std::cout << helics::hzmq::getZMQVersion () << '\n';
#endif
    std::cout << "Compiler = " << HELICS_COMPILER_VERSION << '\n';
    std::cout << "Build Flags =" << HELICS_BUILD_FLAGS << '\n';
	std::cout << "------------PROCESSOR INFO ----------------\n";
	std::cout << "HOST PROCESSOR TYPE: " << HELICS_BUILD_PROCESSOR << '\n';
	auto cpumodel = getCPUModel();
	if (!cpumodel.empty())
	{
		std::cout << "CPU MODEL: " << cpumodel << '\n';
	}
	std::cout << "-------------------------------------------" << std::endl;
    
}
