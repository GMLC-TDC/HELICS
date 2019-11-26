/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/ValueConverter.hpp"
#include "helics/application_api/ValueConverter_impl.hpp"
#include <benchmark/benchmark.h>
#include "helics_benchmark_main.h"

using namespace helics;
template <class T>
static void BM_conversion (benchmark::State &state, const T &arg)
{
    T val{arg};
    helics::data_block store;
    for (auto _ : state)
    {
        helics::ValueConverter<T>::convert (val, store);
    }
}
// Register the function as a benchmark
BENCHMARK_CAPTURE (BM_conversion, double_conv, -356.56e-27);

BENCHMARK_CAPTURE (BM_conversion, int64_conv, int64_t{-12351341});
BENCHMARK_CAPTURE (BM_conversion, uint64_conv, uint64_t{12351341});

BENCHMARK_CAPTURE (BM_conversion, int_conv, int32_t{123541});

BENCHMARK_CAPTURE (BM_conversion, complex_conv, std::complex<double>{45.7, -19.5});

BENCHMARK_CAPTURE (BM_conversion, string_conv, std::string{"test 1"});

BENCHMARK_CAPTURE (BM_conversion, string_conv_med, std::string{"test a longer string"});

BENCHMARK_CAPTURE (BM_conversion,
                   string_conv_long,
                   std::string{"test a longer string with quite a bit longer length than the previous one"});

BENCHMARK_CAPTURE (BM_conversion, vector_conv, std::vector<double>{26.5, 18.6, -48.5, -5.4e-12});

template <class T>
static void BM_interpret (benchmark::State &state, const T &arg)
{
    T val{arg};
    helics::data_block store;
    helics::ValueConverter<T>::convert (val, store);
    data_view stv{store};
    T val2;
    for (auto _ : state)
    {
        ValueConverter<T>::interpret (stv, val2);
    }
}

BENCHMARK_CAPTURE (BM_interpret, double_interp, -356.56e-27);

BENCHMARK_CAPTURE (BM_interpret, int64_interp, int64_t{-12351341});
BENCHMARK_CAPTURE (BM_interpret, uint64_interp, uint64_t{12351341});

BENCHMARK_CAPTURE (BM_interpret, int_interp, int32_t{123541});

BENCHMARK_CAPTURE (BM_interpret, complex_interp, std::complex<double>{45.7, -19.5});

BENCHMARK_CAPTURE (BM_interpret, string_interp, std::string{"test 1"});

BENCHMARK_CAPTURE (BM_interpret, string_interp_med, std::string{"test a longer string"});

BENCHMARK_CAPTURE (BM_interpret,
                   string_interp_long,
                   std::string{"test a longer string with quite a bit longer length than the previous one"});

BENCHMARK_CAPTURE (BM_interpret, vector_interp, std::vector<double>{26.5, 18.6, -48.5, -5.4e-12});

HELICS_BENCHMARK_MAIN();
