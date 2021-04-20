/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "RingTransmitFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/helics-config.h"
#include "helics_benchmark_main.h"

#include <chrono>
#include <fstream>
#include <gmlc/concurrency/Barrier.hpp>
#include <iostream>
#include <thread>

using helics::core_type;

static void BMring2_singleCore(benchmark::State& state)
{
    for (auto _ : state) {
        state.PauseTiming();
        int feds = 2;
        gmlc::concurrency::Barrier brr(feds);
        auto wcore = helics::CoreFactory::create(core_type::INPROC,
                                                 std::string("--autobroker --federates=2"));

        std::vector<RingTransmit> links(feds);
        for (int ii = 0; ii < feds; ++ii) {
            std::string bmInit =
                "--index=" + std::to_string(ii) + " --max_index=" + std::to_string(feds);
            links[ii].initialize(wcore->getIdentifier(), bmInit);
        }

        std::thread rthread([&](RingTransmit& link) { link.run([&brr]() { brr.wait(); }); },
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
BENCHMARK(BMring2_singleCore)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime()
    ->Iterations(1);

static void BMring_multiCore(benchmark::State& state, core_type cType)
{
    for (auto _ : state) {
        state.PauseTiming();
        int feds = static_cast<int>(state.range(0));
        gmlc::concurrency::Barrier brr(feds);
        auto broker =
            helics::BrokerFactory::create(cType,
                                          std::string("--federates=") + std::to_string(feds));
        broker->setLoggingLevel(helics_log_level_no_print);

        std::vector<RingTransmit> links(feds);
        std::vector<std::shared_ptr<helics::Core>> cores(feds);
        for (int ii = 0; ii < feds; ++ii) {
            cores[ii] =
                helics::CoreFactory::create(cType,
                                            std::string(
                                                "--log_level=no_print --federates=1 --broker=" +
                                                broker->getIdentifier()));
            cores[ii]->connect();
            std::string bmInit =
                "--index=" + std::to_string(ii) + " --max_index=" + std::to_string(feds);
            links[ii].initialize(cores[ii]->getIdentifier(), bmInit);
        }
        std::vector<std::thread> threadlist(feds - 1);
        for (int ii = 0; ii < feds - 1; ++ii) {
            threadlist[ii] =
                std::thread([&](RingTransmit& link) { link.run([&brr]() { brr.wait(); }); },
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
BENCHMARK_CAPTURE(BMring_multiCore, inprocCore, core_type::INPROC)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(6)
    ->Arg(10)
    ->Arg(20)
    ->UseRealTime();

#ifdef ENABLE_ZMQ_CORE
// Register the ZMQ benchmarks
BENCHMARK_CAPTURE(BMring_multiCore, zmqCore, core_type::ZMQ)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(6)
    ->Arg(10)
    ->UseRealTime();

// Register the ZMQ benchmarks
BENCHMARK_CAPTURE(BMring_multiCore, zmqssCore, core_type::ZMQ_SS)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(6)
    ->Arg(10)
    ->UseRealTime();
#endif

#ifdef ENABLE_IPC_CORE
// Register the IPC benchmarks
BENCHMARK_CAPTURE(BMring_multiCore, ipcCore, core_type::IPC)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->UseRealTime();
#endif

#ifdef ENABLE_TCP_CORE
// Register the TCP benchmarks
BENCHMARK_CAPTURE(BMring_multiCore, tcpCore, core_type::TCP)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(6)
    ->Arg(10)
    ->UseRealTime();

// Register the TCP SS benchmarks
BENCHMARK_CAPTURE(BMring_multiCore, tcpssCore, core_type::TCP_SS)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(6)
    ->Arg(10)
    ->UseRealTime();

#endif

#ifdef ENABLE_UDP_CORE
// Register the UDP benchmarks
BENCHMARK_CAPTURE(BMring_multiCore, udpCore, core_type::UDP)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(6)
    ->Arg(10)
    ->UseRealTime();
#endif

HELICS_BENCHMARK_MAIN(ringBenchmark);
