/*
Copyright (c) 2017-2024,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "RingTransmitMessageFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics_benchmark_main.h"

#include <benchmark/benchmark.h>
#include <chrono>
#include <fstream>
#include <gmlc/concurrency/Barrier.hpp>
#include <iostream>
#include <thread>
#include <string>
#include <vector>

using helics::CoreType;

static void BM_ringMessage2_singleCore(benchmark::State& state)
{
    for (auto _ : state) {
        state.PauseTiming();
        int feds = 2;
        gmlc::concurrency::Barrier brr(feds);
        auto wcore = helics::CoreFactory::create(
            CoreType::INPROC,
            std::string(
                "--autobroker --federates=2 --restrictive_time_policy --broker_init_string=\"--restrictive_time_policy\""));

        std::vector<RingTransmitMessage> links(feds);
        for (int ii = 0; ii < feds; ++ii) {
            std::string bmInit =
                "--index=" + std::to_string(ii) + " --max_index=" + std::to_string(feds);
            links[ii].initialize(wcore->getIdentifier(), bmInit);
        }

        std::thread rthread([&](RingTransmitMessage& link) { link.run([&brr]() { brr.wait(); }); },
                            std::ref(links[1]));

        links[0].makeReady();
        brr.wait();

        state.ResumeTiming();
        links[0].run();
        state.PauseTiming();
        rthread.join();

        if (links[0].loopCount != 5000) {
            std::cout << "incorrect loop count received (" << links[0].loopCount
                      << ") instead of 5000" << std::endl;
        }
        wcore.reset();
        helics::cleanupHelicsLibrary();
        state.ResumeTiming();
    }
}
// Register the function as a benchmark
BENCHMARK(BM_ringMessage2_singleCore)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime()
    ->Iterations(3);

static void BM_ringMessage_multiCore(benchmark::State& state, CoreType cType)
{
    for (auto _ : state) {
        state.PauseTiming();
        int feds = static_cast<int>(state.range(0));
        gmlc::concurrency::Barrier brr(feds);
        auto broker =
            helics::BrokerFactory::create(cType,
                                          std::string("--restrictive_time_policy --federates=") +
                                              std::to_string(feds));
        broker->setLoggingLevel(0);

        std::vector<RingTransmitMessage> links(feds);
        std::vector<std::shared_ptr<helics::Core>> cores(feds);
        for (int ii = 0; ii < feds; ++ii) {
            cores[ii] = helics::CoreFactory::create(
                cType,
                std::string("--restrictive_time_policy --federates=1 --broker=" +
                            broker->getIdentifier()));
            cores[ii]->connect();
            std::string bmInit =
                "--index=" + std::to_string(ii) + " --max_index=" + std::to_string(feds);
            links[ii].initialize(cores[ii]->getIdentifier(), bmInit);
        }
        std::vector<std::thread> threadlist(feds - 1);
        for (int ii = 0; ii < feds - 1; ++ii) {
            threadlist[ii] =
                std::thread([&](RingTransmitMessage& link) { link.run([&brr]() { brr.wait(); }); },
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

        if (links[0].loopCount != 5000) {
            std::cout << "incorrect loop count received (" << links[0].loopCount
                      << ") instead of 5000" << std::endl;
        }
        broker->disconnect();
        broker.reset();
        cores.clear();
        helics::cleanupHelicsLibrary();
        state.ResumeTiming();
    }
}

// Register the test core benchmarks
BENCHMARK_CAPTURE(BM_ringMessage_multiCore, inprocCore, CoreType::INPROC)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(6)
    ->Arg(10)
    ->Arg(20)
    ->UseRealTime();

#ifdef HELICS_ENABLE_ZMQ_CORE
// Register the ZMQ benchmarks
BENCHMARK_CAPTURE(BM_ringMessage_multiCore, zmqCore, CoreType::ZMQ)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(6)
    ->Arg(10)
    ->UseRealTime();

// Register the ZMQ benchmarks
BENCHMARK_CAPTURE(BM_ringMessage_multiCore, zmqssCore, CoreType::ZMQ_SS)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(1)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(6)
    ->Arg(10)
    ->UseRealTime();
#endif

#ifdef HELICS_ENABLE_IPC_CORE
// Register the IPC benchmarks
BENCHMARK_CAPTURE(BM_ringMessage_multiCore, ipcCore, CoreType::IPC)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(3)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->UseRealTime();
#endif

#ifdef HELICS_ENABLE_TCP_CORE
// Register the TCP benchmarks
BENCHMARK_CAPTURE(BM_ringMessage_multiCore, tcpCore, CoreType::TCP)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(3)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(6)
    ->Arg(10)
    ->UseRealTime();

// Register the TCP SS benchmarks
BENCHMARK_CAPTURE(BM_ringMessage_multiCore, tcpssCore, CoreType::TCP_SS)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(3)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(6)
    ->Arg(10)
    ->UseRealTime();
#endif
// Register the UDP benchmarks
#ifdef HELICS_ENABLE_UDP_CORE
BENCHMARK_CAPTURE(BM_ringMessage_multiCore, udpCore, CoreType::UDP)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(3)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(6)
    ->Arg(10)
    ->UseRealTime();
#endif
HELICS_BENCHMARK_MAIN(ringMessageBenchmark);
