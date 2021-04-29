/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "TimingHubFederate.hpp"
#include "TimingLeafFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/helics-config.h"
#include "helics_benchmark_main.h"

#include <benchmark/benchmark.h>
#include <chrono>
#include <fstream>
#include <gmlc/concurrency/Barrier.hpp>
#include <iostream>
#include <thread>

using helics::core_type;
static void BMtiming_singleCore(benchmark::State& state)
{
    for (auto _ : state) {
        state.PauseTiming();

        int feds = static_cast<int>(state.range(0));
        gmlc::concurrency::Barrier brr(static_cast<size_t>(feds) + 1);
        auto wcore = helics::CoreFactory::create(core_type::INPROC,
                                                 std::string("--autobroker --federates=") +
                                                     std::to_string(feds + 1));
        TimingHub hub;
        std::string bmInit = "--num_leafs=" + std::to_string(feds);
        hub.initialize(wcore->getIdentifier(), bmInit);
        std::vector<TimingLeaf> leafs(feds);
        for (int ii = 0; ii < feds; ++ii) {
            bmInit = "--index=" + std::to_string(ii);
            leafs[ii].initialize(wcore->getIdentifier(), bmInit);
        }

        std::vector<std::thread> threadlist(static_cast<size_t>(feds));
        for (int ii = 0; ii < feds; ++ii) {
            threadlist[ii] = std::thread([&](TimingLeaf& lf) { lf.run([&brr]() { brr.wait(); }); },
                                         std::ref(leafs[ii]));
        }
        hub.makeReady();
        brr.wait();
        state.ResumeTiming();
        hub.run([]() {});
        state.PauseTiming();
        for (auto& thrd : threadlist) {
            thrd.join();
        }
        wcore.reset();
        helics::cleanupHelicsLibrary();
        state.ResumeTiming();
    }
}
// Register the function as a benchmark
BENCHMARK(BMtiming_singleCore)
    ->RangeMultiplier(2)
    ->Range(1, 1 << 8)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(1)
    ->UseRealTime();

static void BMtiming_multiCore(benchmark::State& state, core_type cType)
{
    for (auto _ : state) {
        state.PauseTiming();

        int feds = static_cast<int>(state.range(0));
        gmlc::concurrency::Barrier brr(static_cast<size_t>(feds) + 1);

        auto broker =
            helics::BrokerFactory::create(cType,
                                          "brokerb",
                                          std::string("--federates=") + std::to_string(feds + 1));
        broker->setLoggingLevel(helics_log_level_no_print);
        auto wcore =
            helics::CoreFactory::create(cType, std::string("--federates=1 --log_level=no_print"));
        // this is to delay until the threads are ready
        TimingHub hub;
        std::string bmInit = "--num_leafs=" + std::to_string(feds);
        hub.initialize(wcore->getIdentifier(), bmInit);
        std::vector<TimingLeaf> leafs(feds);
        std::vector<std::shared_ptr<helics::Core>> cores(feds);
        for (int ii = 0; ii < feds; ++ii) {
            cores[ii] = helics::CoreFactory::create(cType, "-f 1 --log_level=no_print");
            cores[ii]->connect();
            bmInit = "--index=" + std::to_string(ii);
            leafs[ii].initialize(cores[ii]->getIdentifier(), bmInit);
        }

        std::vector<std::thread> threadlist(static_cast<size_t>(feds));
        for (int ii = 0; ii < feds; ++ii) {
            threadlist[ii] = std::thread([&](TimingLeaf& lf) { lf.run([&brr]() { brr.wait(); }); },
                                         std::ref(leafs[ii]));
        }
        hub.makeReady();
        brr.wait();
        state.ResumeTiming();
        hub.run([]() {});
        state.PauseTiming();
        for (auto& thrd : threadlist) {
            thrd.join();
        }
        broker->disconnect();
        broker.reset();
        cores.clear();
        wcore.reset();
        helics::cleanupHelicsLibrary();

        state.ResumeTiming();
    }
}

static constexpr int64_t maxscale{1 << (4 + HELICS_BENCHMARK_SHIFT_FACTOR)};
// Register the inproc core benchmarks
BENCHMARK_CAPTURE(BMtiming_multiCore, inprocCore, core_type::INPROC)
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

#ifdef ENABLE_ZMQ_CORE
// Register the ZMQ benchmarks
BENCHMARK_CAPTURE(BMtiming_multiCore, zmqCore, core_type::ZMQ)
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

// Register the ZMQ benchmarks
BENCHMARK_CAPTURE(BMtiming_multiCore, zmqssCore, core_type::ZMQ_SS)
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

#endif

#ifdef ENABLE_IPC_CORE
// Register the IPC benchmarks
BENCHMARK_CAPTURE(BMtiming_multiCore, ipcCore, core_type::IPC)
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

#endif

#ifdef ENABLE_TCP_CORE
// Register the TCP benchmarks
BENCHMARK_CAPTURE(BMtiming_multiCore, tcpCore, core_type::TCP)
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

// Register the TCP SS benchmarks
BENCHMARK_CAPTURE(BMtiming_multiCore, tcpssCore, core_type::TCP_SS)
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

#endif

#ifdef ENABLE_UDP_CORE
// Register the UDP benchmarks
BENCHMARK_CAPTURE(BMtiming_multiCore, udpCore, core_type::UDP)
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();
#endif

HELICS_BENCHMARK_MAIN(timingBenchmark);
