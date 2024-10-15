/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "EchoMessageHubFederate.hpp"
#include "EchoMessageLeafFederate.hpp"
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
#include <string>
#include <vector>

// static constexpr helics::Time tend = 3600.0_t;  // simulation end time

using helics::CoreType;

static void BMecho_singleCore(benchmark::State& state)
{
    for (auto _ : state) {
        state.PauseTiming();

        int feds = static_cast<int>(state.range(0));
        gmlc::concurrency::Barrier brr(static_cast<size_t>(feds) + 1);
        auto wcore = helics::CoreFactory::create(CoreType::INPROC,
                                                 std::string("--autobroker --federates=") +
                                                     std::to_string(feds + 1));
        EchoMessageHub hub;
        hub.initialize(wcore->getIdentifier(), "");
        std::vector<EchoMessageLeaf> leafs(feds);
        for (int ii = 0; ii < feds; ++ii) {
            std::string bmInit = "--index=" + std::to_string(ii);
            leafs[ii].initialize(wcore->getIdentifier(), bmInit);
        }

        std::vector<std::thread> threadlist(static_cast<size_t>(feds));
        for (int ii = 0; ii < feds; ++ii) {
            threadlist[ii] =
                std::thread([&](EchoMessageLeaf& lf) { lf.run([&brr]() { brr.wait(); }); },
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
BENCHMARK(BMecho_singleCore)
    ->RangeMultiplier(2)
    ->Range(1, 32)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(1)
    ->UseRealTime();

static void BMecho_multiCore(benchmark::State& state, CoreType cType)
{
    for (auto _ : state) {
        state.PauseTiming();

        int feds = static_cast<int>(state.range(0));
        gmlc::concurrency::Barrier brr(static_cast<size_t>(feds) + 1);

        auto broker =
            helics::BrokerFactory::create(cType,
                                          "brokerb",
                                          std::string("--federates=") + std::to_string(feds + 1));
        broker->setLoggingLevel(HELICS_LOG_LEVEL_NO_PRINT);
        auto wcore =
            helics::CoreFactory::create(cType, std::string("--federates=1 --log_level=no_print"));
        // this is to delay until the threads are ready
        EchoMessageHub hub;
        hub.initialize(wcore->getIdentifier(), "");
        std::vector<EchoMessageLeaf> leafs(feds);
        std::vector<std::shared_ptr<helics::Core>> cores(feds);
        for (int ii = 0; ii < feds; ++ii) {
            cores[ii] = helics::CoreFactory::create(cType, "-f 1 --log_level=no_print");
            cores[ii]->connect();
            std::string bmInit = "--index=" + std::to_string(ii);
            leafs[ii].initialize(cores[ii]->getIdentifier(), bmInit);
        }

        std::vector<std::thread> threadlist(static_cast<size_t>(feds));
        for (int ii = 0; ii < feds; ++ii) {
            threadlist[ii] =
                std::thread([&](EchoMessageLeaf& lf) { lf.run([&brr]() { brr.wait(); }); },
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

static constexpr int64_t maxscale{1 << (5 + HELICS_BENCHMARK_SHIFT_FACTOR)};
// Register the inproc core benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, inprocCore, CoreType::INPROC)
    ->RangeMultiplier(2)
    ->Range(1, maxscale * 2)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

#ifdef HELICS_ENABLE_ZMQ_CORE
// Register the ZMQ benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, zmqCore, CoreType::ZMQ)
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

// Register the ZMQ benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, zmqssCore, CoreType::ZMQ_SS)
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

#endif

#ifdef HELICS_ENABLE_IPC_CORE
// Register the IPC benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, ipcCore, CoreType::IPC)
    ->RangeMultiplier(2)
    ->Range(1, maxscale * 2)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

#endif

#ifdef HELICS_ENABLE_TCP_CORE
// Register the TCP benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, tcpCore, CoreType::TCP)
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

// Register the TCP SS benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, tcpssCore, CoreType::TCP_SS)
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

#endif

#ifdef HELICS_ENABLE_UDP_CORE
// Register the UDP benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, udpCore, CoreType::UDP)
    ->RangeMultiplier(2)
    ->Range(1, maxscale / 2)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();
#endif

HELICS_BENCHMARK_MAIN(echoMessageBenchmark);
