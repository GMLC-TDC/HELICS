/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "WattsStrogatzFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics_benchmark_main.h"

#include <benchmark/benchmark.h>
#include <chrono>
#include <fstream>
#include <gmlc/concurrency/Barrier.hpp>
#include <iostream>
#include <thread>

using helics::core_type;

static constexpr int64_t maxscale{1 << (4 + HELICS_BENCHMARK_SHIFT_FACTOR)};

// WattsStrogatzArguments sets up parameterized arguments for the multicore benchmark runs.
// The degree argument has constraints based on the federate count so the Ranges function
// in google benchmarks can't be used.
//
// f is the number of federates, ranging from 2 to 16 using powers of two.
// d is the degree (number of outgoing links) for each federate, with a range from 1 to f-1.
// p is the rewire probability (0-100) for links between federates.
static void WattsStrogatzArguments(benchmark::internal::Benchmark* b)
{
    for (int f = 2; f <= maxscale; f *= 2) {
        for (int d = 2; d <= f; d *= 2) {
            for (int p = 0; p <= 100; p += 25) {
                b->Args({f, d - 1, p});
            }
        }
    }
}

static void BM_wattsStrogatz2_singleCore(benchmark::State& state)
{
    for (auto _ : state) {
        state.PauseTiming();
        int feds = 2;
        int degree = 1;

        gmlc::concurrency::Barrier brr(feds);
        auto wcore = helics::CoreFactory::create(
            core_type::INPROC,
            std::string(
                "--autobroker --federates=2 --restrictive_time_policy --brokerinit=\"--restrictive_time_policy\""));

        std::vector<WattsStrogatzFederate> links(feds);
        for (int ii = 0; ii < feds; ++ii) {
            std::string bmInit = "--index=" + std::to_string(ii) +
                " --max_index=" + std::to_string(feds) + " --degree=" + std::to_string(degree);
            links[ii].initialize(wcore->getIdentifier(), bmInit);
        }

        std::thread rthread(
            [&](WattsStrogatzFederate& link) { link.run([&brr]() { brr.wait(); }); },
            std::ref(links[1]));

        links[0].makeReady();
        brr.wait();

        state.ResumeTiming();
        links[0].run();
        state.PauseTiming();
        rthread.join();

        wcore.reset();
        helics::cleanupHelicsLibrary();
        state.ResumeTiming();
    }
}
// Register the function as a benchmark
BENCHMARK(BM_wattsStrogatz2_singleCore)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime()
    ->Iterations(3);

static void BM_wattsStrogatz_multiCore(benchmark::State& state, core_type cType)
{
    for (auto _ : state) {
        state.PauseTiming();

        // Interested in the effect for varying degrees of connectedness
        int feds = static_cast<int>(state.range(0));
        int degree = static_cast<int>(state.range(1));
        double rewireP = static_cast<double>(state.range(2)) / 100.0;

        gmlc::concurrency::Barrier brr(feds);
        auto broker =
            helics::BrokerFactory::create(cType,
                                          std::string("--restrictive_time_policy --federates=") +
                                              std::to_string(feds));
        broker->setLoggingLevel(0);

        std::vector<WattsStrogatzFederate> links(feds);
        std::vector<std::shared_ptr<helics::Core>> cores(feds);
        for (int ii = 0; ii < feds; ++ii) {
            cores[ii] = helics::CoreFactory::create(
                cType,
                std::string("--restrictive_time_policy --federates=1 --broker=" +
                            broker->getIdentifier()));
            cores[ii]->connect();
            std::string bmInit = "--index=" + std::to_string(ii) +
                " --max_index=" + std::to_string(feds) + " --degree=" + std::to_string(degree) +
                " --rewire_probability=" + std::to_string(rewireP);
            links[ii].initialize(cores[ii]->getIdentifier(), bmInit);
        }
        std::vector<std::thread> threadlist(feds - 1);
        for (int ii = 0; ii < feds - 1; ++ii) {
            threadlist[ii] = std::thread(
                [&](WattsStrogatzFederate& link) { link.run([&brr]() { brr.wait(); }); },
                std::ref(links[ii + 1]));
        }

        links[0].makeReady();
        brr.wait();
        state.ResumeTiming();
        links[0].run();
        state.PauseTiming();
        for (auto& thrd : threadlist) {
            thrd.join();
        }

        broker->disconnect();
        broker.reset();
        cores.clear();
        helics::cleanupHelicsLibrary();
        state.ResumeTiming();
    }
}

// Register the test core benchmarks
BENCHMARK_CAPTURE(BM_wattsStrogatz_multiCore, inprocCore, core_type::INPROC)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Apply(WattsStrogatzArguments)
    ->UseRealTime();

#ifdef ENABLE_ZMQ_CORE
// Register the ZMQ benchmarks
BENCHMARK_CAPTURE(BM_wattsStrogatz_multiCore, zmqCore, core_type::ZMQ)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(1)
    ->Apply(WattsStrogatzArguments)
    ->UseRealTime();

// Register the ZMQ benchmarks
BENCHMARK_CAPTURE(BM_wattsStrogatz_multiCore, zmqssCore, core_type::ZMQ_SS)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(1)
    ->Apply(WattsStrogatzArguments)
    ->UseRealTime();
#endif

#ifdef ENABLE_IPC_CORE
// Register the IPC benchmarks
BENCHMARK_CAPTURE(BM_wattsStrogatz_multiCore, ipcCore, core_type::IPC)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Apply(WattsStrogatzArguments)
    ->UseRealTime();
#endif

#ifdef ENABLE_TCP_CORE
// Register the TCP benchmarks
BENCHMARK_CAPTURE(BM_wattsStrogatz_multiCore, tcpCore, core_type::TCP)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(1)
    ->Apply(WattsStrogatzArguments)
    ->UseRealTime();

// Register the TCP SS benchmarks
BENCHMARK_CAPTURE(BM_wattsStrogatz_multiCore, tcpssCore, core_type::TCP_SS)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(1)
    ->Apply(WattsStrogatzArguments)
    ->UseRealTime();
#endif

// Register the UDP benchmarks
// The UDP benchmark starts hanging if too many messages are sent.
#ifdef ENABLE_UDP_CORE
BENCHMARK_CAPTURE(BM_wattsStrogatz_multiCore, udpCore, core_type::UDP)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(1)
    ->Apply(WattsStrogatzArguments)
    ->UseRealTime();
#endif
HELICS_BENCHMARK_MAIN(wattsStrogatzBenchmark);
