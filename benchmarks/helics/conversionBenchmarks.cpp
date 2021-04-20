/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/ValueConverter.hpp"
#include "helics/application_api/ValueConverter_impl.hpp"
#include "helics_benchmark_main.h"

#include <benchmark/benchmark.h>

template<class T>
static void BMconversion(benchmark::State& state, const T& arg)
{
    T val{arg};
    helics::data_block store;
    for (auto _ : state) {
        helics::ValueConverter<T>::convert(val, store);
    }
}
// Register the function as a benchmark
BENCHMARK_CAPTURE(BMconversion, double_conv, -356.56e-27);

BENCHMARK_CAPTURE(BMconversion, int64_conv, int64_t{-12351341});
BENCHMARK_CAPTURE(BMconversion, uint64_conv, uint64_t{12351341});

BENCHMARK_CAPTURE(BMconversion, int_conv, int32_t{123541});

BENCHMARK_CAPTURE(BMconversion, complex_conv, std::complex<double>{45.7, -19.5});

BENCHMARK_CAPTURE(BMconversion, string_conv, std::string{"test 1"});

BENCHMARK_CAPTURE(BMconversion, string_conv_med, std::string{"test a longer string"});

BENCHMARK_CAPTURE(BMconversion,
                  string_conv_long,
                  std::string{
                      "test a longer string with quite a bit longer length than the previous one"});

BENCHMARK_CAPTURE(BMconversion, vector_conv, std::vector<double>{26.5, 18.6, -48.5, -5.4e-12});

template<class T>
static void BMinterpret(benchmark::State& state, const T& arg)
{
    T val{arg};
    helics::data_block store;
    helics::ValueConverter<T>::convert(val, store);
    helics::data_view stv{store};
    T val2;
    for (auto _ : state) {
        helics::ValueConverter<T>::interpret(stv, val2);
    }
}

BENCHMARK_CAPTURE(BMinterpret, double_interp, -356.56e-27);

BENCHMARK_CAPTURE(BMinterpret, int64_interp, int64_t{-12351341});
BENCHMARK_CAPTURE(BMinterpret, uint64_interp, uint64_t{12351341});

BENCHMARK_CAPTURE(BMinterpret, int_interp, int32_t{123541});

BENCHMARK_CAPTURE(BMinterpret, complex_interp, std::complex<double>{45.7, -19.5});

BENCHMARK_CAPTURE(BMinterpret, string_interp, std::string{"test 1"});

BENCHMARK_CAPTURE(BMinterpret, string_interp_med, std::string{"test a longer string"});

BENCHMARK_CAPTURE(BMinterpret,
                  string_interp_long,
                  std::string{
                      "test a longer string with quite a bit longer length than the previous one"});

BENCHMARK_CAPTURE(BMinterpret, vector_interp, std::vector<double>{26.5, 18.6, -48.5, -5.4e-12});

HELICS_BENCHMARK_MAIN(conversionBenchmark);
