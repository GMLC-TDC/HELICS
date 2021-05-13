/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include "helics_benchmark_util.h"

#include <benchmark/benchmark.h>
#include <iostream>

// Helper macro to create a main routine in a test that runs the benchmarks
#define HELICS_BENCHMARK_MAIN(label)                                                               \
    int main(int argc, char** argv)                                                                \
    {                                                                                              \
        ::benchmark::Initialize(&argc, argv);                                                      \
        if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;                        \
        std::cout << "HELICS_BENCHMARK: " << #label << '\n';                                       \
        printHELICSsystemInfo();                                                                   \
        ::benchmark::RunSpecifiedBenchmarks();                                                     \
    }                                                                                              \
    int main(int, char**)
