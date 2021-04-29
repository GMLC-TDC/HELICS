/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/core/ActionMessage.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics/helics-config.h"
#include "helics_benchmark_main.h"

#include <benchmark/benchmark.h>
#include <chrono>
#include <fstream>
#include <gmlc/concurrency/Barrier.hpp>
#include <iostream>
#include <random>
#include <thread>

/** class implementing the hub for an echo test*/
class messageGenerator {
  public:
    helics::Time finalTime = helics::Time(100, time_units::ms);  // final time
  private:
    std::unique_ptr<helics::MessageFederate> mFed;
    std::vector<helics::Endpoint> epts;
    int cnt_local_ = 10;
    int cnt_total_ = 10;
    int perloop_message_ = 10;
    int startIndex_ = 0;
    bool initialized = false;
    bool readyToRun = false;

  public:
    messageGenerator() = default;

    void run(std::function<void()> callOnReady = {})
    {
        if (!readyToRun) {
            makeReady();
        }
        if (callOnReady) {
            callOnReady();
        }
        mainLoop();
    };

    void initialize(const std::string& coreName, int cntlocal, int cnttotal, int perloop, int index)
    {
        cnt_local_ = cntlocal;
        cnt_total_ = cnttotal;
        perloop_message_ = perloop;
        startIndex_ = index * cntlocal;
        std::string name = "mgen_" + std::to_string(index);
        helics::FederateInfo fi;
        fi.coreName = coreName;
        fi.setProperty(helics_property_time_period, 1.0);
        mFed = std::make_unique<helics::MessageFederate>(name, fi);
        epts.reserve(cnt_local_);
        for (int ii = 0; ii < cnt_local_; ++ii) {
            epts.push_back(
                mFed->registerGlobalEndpoint("ept_" + std::to_string(index * cntlocal + ii)));
        }
        initialized = true;
    }

    void makeReady()
    {
        if (!initialized) {
            throw("must initialize first");
        }
        mFed->enterExecutingMode();
        readyToRun = true;
    }

    void mainLoop()
    {
        std::random_device rd;  // obtain a random number from hardware
        std::mt19937 eng(rd());  // seed the generator
        std::uniform_int_distribution<> messageDest(0,
                                                    cnt_total_ -
                                                        1);  // define possible destinations
        std::uniform_int_distribution<> messageSource(0,
                                                      cnt_local_ - 1);  // define possible sources
        const std::string message = "hello";
        const std::string destName = "ept_";
        for (int jj = 0; jj < 100; ++jj) {
            for (int ii = 0; ii < perloop_message_; ++ii) {
                auto src = messageSource(eng);
                auto dest = messageDest(eng);
                epts[src].send(destName + std::to_string(dest), message);
            }
            mFed->requestNextStep();
            auto m = mFed->getMessage();
            while (m) {
                m = mFed->getMessage();
            }
        }

        mFed->finalize();
    }
};

using helics::core_type;
static void BMmgen_singleCore(benchmark::State& state)
{
    for (auto _ : state) {
        state.PauseTiming();

        auto wcore = helics::CoreFactory::create(core_type::INPROC, std::string("--autobroker "));
        messageGenerator mgen;
        mgen.initialize(wcore->getIdentifier(),
                        static_cast<int>(state.range(0)),
                        static_cast<int>(state.range(0)),
                        100,
                        0);

        mgen.makeReady();
        state.ResumeTiming();
        mgen.run();
        state.PauseTiming();
        wcore.reset();
        helics::cleanupHelicsLibrary();
        state.ResumeTiming();
    }
}
// Register the function as a benchmark
BENCHMARK(BMmgen_singleCore)
    ->RangeMultiplier(4)
    ->Range(1, 1 << 18)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(1)
    ->UseRealTime();

static void BMmgen_multiCore(benchmark::State& state, core_type cType)
{
    for (auto _ : state) {
        if (state.range(0) < state.range(1) * 8) {
            continue;
        }

        state.PauseTiming();
        int feds = static_cast<int>(state.range(1));
        gmlc::concurrency::Barrier brr(feds);
        auto broker =
            helics::BrokerFactory::create(cType,
                                          std::string("--federates=") + std::to_string(feds));
        broker->setLoggingLevel(helics_log_level_no_print);

        std::vector<messageGenerator> gens(feds);
        std::vector<std::shared_ptr<helics::Core>> cores(feds);
        for (int ii = 0; ii < feds; ++ii) {
            cores[ii] =
                helics::CoreFactory::create(cType,
                                            std::string(
                                                " --federates=1 --log_level=no_print --broker=" +
                                                broker->getIdentifier()));
            cores[ii]->connect();
            gens[ii].initialize(cores[ii]->getIdentifier(),
                                static_cast<int>(state.range(0) / state.range(1)),
                                static_cast<int>(state.range(0)),
                                100,
                                ii);
        }
        std::vector<std::thread> threadlist(feds - 1);
        for (int ii = 0; ii < feds - 1; ++ii) {
            threadlist[ii] =
                std::thread([&](messageGenerator& gen) { gen.run([&brr]() { brr.wait(); }); },
                            std::ref(gens[ii + 1]));
        }

        gens[0].makeReady();
        brr.wait();
        state.ResumeTiming();
        gens[0].run();
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
BENCHMARK_CAPTURE(BMmgen_multiCore, inprocCore, core_type::INPROC)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Ranges({{32, 1 << 15}, {2, 64}})
    ->Iterations(1)
    ->UseRealTime();

// Register the test core benchmarks
BENCHMARK_CAPTURE(BMmgen_multiCore, inprocCore_big2, core_type::INPROC)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Ranges({{1 << 17, 1 << 19}, {2, 2}})
    ->Iterations(1)
    ->UseRealTime();

// Register the test core benchmarks
BENCHMARK_CAPTURE(BMmgen_multiCore, inprocCore_big8, core_type::INPROC)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Ranges({{1 << 17, 1 << 19}, {8, 8}})
    ->Iterations(1)
    ->UseRealTime();
/*
// Register the ZMQ benchmarks
BENCHMARK_CAPTURE (BM_ring_multiCore, zmqCore, core_type::ZMQ)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->Iterations (3)
  ->Arg (2)
  ->Arg (3)
  ->Arg (4)
  ->Arg (6)
  ->Arg (10)
  ->Arg (20)
  ->UseRealTime ();

// Register the ZMQ benchmarks
BENCHMARK_CAPTURE (BM_ring_multiCore, zmqssCore, core_type::ZMQ_SS)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->Iterations (3)
  ->Arg (2)
  ->Arg (3)
  ->Arg (4)
  ->Arg (6)
  ->Arg (10)
  ->Arg (20)
  ->UseRealTime ();

// Register the IPC benchmarks
BENCHMARK_CAPTURE (BM_ring_multiCore, ipcCore, core_type::IPC)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->Iterations (3)
  ->Arg (2)
  ->Arg (3)
  ->Arg (4)
  ->Arg (10)
  ->Arg (20)
  ->UseRealTime ();

// Register the TCP benchmarks
BENCHMARK_CAPTURE (BM_ring_multiCore, tcpCore, core_type::TCP)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->Iterations (3)
  ->Arg (2)
  ->Arg (3)
  ->Arg (4)
  ->Arg (6)
  ->Arg (10)
  ->Arg (20)
  ->UseRealTime ();

// Register the TCP SS benchmarks
BENCHMARK_CAPTURE (BM_ring_multiCore, tcpssCore, core_type::TCP_SS)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->Iterations (3)
  ->Arg (2)
  ->Arg (3)
  ->Arg (4)
  ->Arg (6)
  ->Arg (10)
  ->Arg (20)
  ->UseRealTime ();

// Register the UDP benchmarks
BENCHMARK_CAPTURE (BM_ring_multiCore, udpCore, core_type::UDP)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->Iterations (3)
  ->Arg (2)
  ->Arg (3)
  ->Arg (4)
  ->Arg (6)
  ->Arg (10)
  ->Arg (20)
  ->UseRealTime ();
  */

HELICS_BENCHMARK_MAIN(messageLookupBenchmark);
