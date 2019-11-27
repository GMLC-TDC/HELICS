/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once
#include <benchmark/benchmark.h>
#include "helics/helics-config.h"
#include <iostream>
#ifdef ENABLE_ZMQ_CORE
#include "helics/core/zmq/ZmqCommsCommon.h" 
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

///Generate a report of the compilers used and zmq version linked as well as the version info for HELICS
/// for use with the benchmarks
inline void printHELICSsystemInfo()
{
	std::cout << "------------HELICS BUILD INFO -------------\nHELICS "<<HELICS_VERSION_STRING << '\n';
#ifdef ENABLE_ZMQ_CORE
	std::cout << helics::hzmq::getZMQVersion() << '\n';
#endif
	std::cout << "Compiler = " << HELICS_COMPILER_VERSION << '\n';
    std::cout << "Build Flags =" << HELICS_BUILD_FLAGS << '\n';
	std::cout << "-------------------------------------------" << std::endl;
}
