/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "PholdFederate.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics_benchmark_main.h"
#include <benchmark/benchmark.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <thread>

#include <gmlc/concurrency/Barrier.hpp>

#include "helics/helics-config.h"

// static constexpr helics::Time tend = 3600.0_t;  // simulation end time
using namespace helics;

static void BM_phold_singleCore (benchmark::State &state)
{
  for (auto _ : state)
  {
    std::cout << "starting run" << std::endl;
    state.PauseTiming ();

    int fed_count = static_cast<int> (state.range (0));
    gmlc::concurrency::Barrier brr (static_cast<size_t> (fed_count));
    auto wcore = helics::CoreFactory::create (core_type::INPROC, std::string ("--autobroker --federates=") +
                                                                   std::to_string (fed_count));
    std::vector<PholdFederate> feds (fed_count);
    std::mt19937 rand_gen (0x600d5eed);
    std::uniform_int_distribution<unsigned int> rand_seed;
    for (int ii = 0; ii < fed_count; ++ii)
    {
      // set seeds for federates to deterministic values, but not all the same
      feds[ii].setGenerateRandomSeed (false);
      feds[ii].setRandomSeed (rand_seed (rand_gen));
      feds[ii].initialize (wcore->getIdentifier (), ii, fed_count);
    }

    std::vector<std::thread> threadlist (static_cast<size_t> (fed_count - 1));
    for (int ii = 0; ii < fed_count - 1; ++ii)
    {
      threadlist[ii] =
        std::thread ([&](PholdFederate &f) { f.run ([&brr]() { brr.wait (); }); }, std::ref (feds[ii + 1]));
    }
    feds[0].makeReady ();
    brr.wait ();
    state.ResumeTiming ();
    feds[0].run ([]() {});
    state.PauseTiming ();
    for (auto &thrd : threadlist)
    {
      thrd.join ();
    }

    int totalEvCount = 0;
    for (int ii = 0; ii < fed_count; ++ii)
    {
      totalEvCount += feds[ii].evCount;
    }
    state.counters["EvCount"] = totalEvCount;

    wcore.reset ();
    cleanupHelicsLibrary ();
    state.ResumeTiming ();
  }
}
// Register the function as a benchmark
BENCHMARK (BM_phold_singleCore)
  ->RangeMultiplier (2)
  ->Range (1, 16)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->Iterations (1)
  ->UseRealTime ();

static void BM_phold_multiCore (benchmark::State &state, core_type cType)
{
  for (auto _ : state)
  {
    state.PauseTiming ();

    int fed_count = static_cast<int> (state.range (0));
    gmlc::concurrency::Barrier brr (static_cast<size_t> (fed_count));

    auto broker =
      helics::BrokerFactory::create (cType, "brokerb", std::string ("--federates=") + std::to_string (fed_count));
    broker->setLoggingLevel (helics_log_level_no_print);
    auto wcore = helics::CoreFactory::create (cType, std::string ("--federates=1"));
    std::vector<PholdFederate> feds (fed_count);
    std::vector<std::shared_ptr<helics::Core>> cores (fed_count);

    std::mt19937 rand_gen (0x600d5eed);
    std::uniform_int_distribution<unsigned int> rand_seed;
    for (int ii = 0; ii < fed_count; ++ii)
    {
      cores[ii] = helics::CoreFactory::create (cType, "-f 1");
      cores[ii]->connect ();

      // set seeds for federates to deterministic values, but not all the same
      feds[ii].setGenerateRandomSeed (false);
      feds[ii].setRandomSeed (rand_seed (rand_gen));
      feds[ii].initialize (cores[ii]->getIdentifier (), ii, fed_count);
    }

    std::vector<std::thread> threadlist (static_cast<size_t> (fed_count - 1));
    for (int ii = 0; ii < fed_count - 1; ++ii)
    {
      threadlist[ii] =
        std::thread ([&](PholdFederate &f) { f.run ([&brr]() { brr.wait (); }); }, std::ref (feds[ii + 1]));
    }
    feds[0].makeReady ();
    brr.wait ();
    state.ResumeTiming ();
    feds[0].run ([]() {});
    state.PauseTiming ();
    for (auto &thrd : threadlist)
    {
      thrd.join ();
    }

    int totalEvCount = 0;
    for (auto &f : feds)
    {
      totalEvCount += f.evCount;
    }
    state.counters["EvCount"] = totalEvCount;

    broker->disconnect ();
    broker.reset ();
    cores.clear ();
    wcore.reset ();
    cleanupHelicsLibrary ();

    state.ResumeTiming ();
  }
}

static constexpr int64_t maxscale{1 << 5};
// Register the inproc core benchmarks
BENCHMARK_CAPTURE (BM_phold_multiCore, inprocCore, core_type::INPROC)
  ->RangeMultiplier (2)
  ->Range (1, maxscale * 2)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

#ifdef ENABLE_ZMQ_CORE
// Register the ZMQ benchmarks
BENCHMARK_CAPTURE (BM_phold_multiCore, zmqCore, core_type::ZMQ)
  ->RangeMultiplier (2)
  ->Range (1, maxscale)
  ->Iterations (1)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

// Register the ZMQ benchmarks
BENCHMARK_CAPTURE (BM_phold_multiCore, zmqssCore, core_type::ZMQ_SS)
  ->RangeMultiplier (2)
  ->Range (1, maxscale)
  ->Iterations (1)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

#endif

#ifdef ENABLE_IPC_CORE
// Register the IPC benchmarks
BENCHMARK_CAPTURE (BM_phold_multiCore, ipcCore, core_type::IPC)
  ->RangeMultiplier (2)
  ->Range (1, maxscale * 2)
  ->Iterations (1)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

#endif

#ifdef ENABLE_TCP_CORE
// Register the TCP benchmarks
BENCHMARK_CAPTURE (BM_phold_multiCore, tcpCore, core_type::TCP)
  ->RangeMultiplier (2)
  ->Range (1, maxscale)
  ->Iterations (1)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

// Register the TCP SS benchmarks
BENCHMARK_CAPTURE (BM_phold_multiCore, tcpssCore, core_type::TCP_SS)
  ->RangeMultiplier (2)
  ->Range (1, 1)  // This is set to 1; any higher seems to result in deadlock (OS buffer limit?)
  ->Iterations (1)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

#endif

#ifdef ENABLE_UDP_CORE
// Register the UDP benchmarks
BENCHMARK_CAPTURE (BM_phold_multiCore, udpCore, core_type::UDP)
  ->RangeMultiplier (2)
  ->Range (1, 1 << 4)
  ->Iterations (1)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();
#endif

HELICS_BENCHMARK_MAIN (pholdBenchmark);
