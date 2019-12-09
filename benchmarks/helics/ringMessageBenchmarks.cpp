/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/core/ActionMessage.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include "helics_benchmark_main.h"

#include <benchmark/benchmark.h>
#include <chrono>
#include <fstream>
#include <gmlc/concurrency/Barrier.hpp>
#include <iostream>
#include <thread>

using helics::operator"" _t;
// static constexpr helics::Time tend = 3600.0_t;  // simulation end time
using namespace helics;
/** class implementing the hub for an echo test*/
class RingTransmitMessage {
  public:
    helics::Time deltaTime = helics::Time(10, time_units::ns); // sampling rate
    helics::Time finalTime = helics::Time(100, time_units::ns); // final time
    int loopCount = 0;

  private:
    std::unique_ptr<helics::MessageFederate> mFed;
    helics::Endpoint* ept = nullptr;

    int index_ = 0;
    int maxIndex_ = 0;
    int messageSize_ = 100;
    bool initialized{false};
    bool readyToRun{false};

  public:
    RingTransmitMessage() = default;

    void run(std::function<void()> callOnReady = nullptr)
    {
        makeReady();
        if (callOnReady) {
            callOnReady();
        }
        mainLoop();
    };

    void initialize(const std::string& coreName, int index, int maxIndex, int message_size)
    {
        std::string name = "ringlink_" + std::to_string(index);
        index_ = index;
        maxIndex_ = maxIndex;
        helics::FederateInfo fi;
        fi.coreName = coreName;
        fi.setFlagOption(helics_flag_restrictive_time_policy);
        if (index == 0) {
            // fi.setProperty (helics_property_int_log_level, helics_log_level_timing);
        }
        mFed = std::make_unique<helics::MessageFederate>(name, fi);
        ept = &mFed->registerIndexedEndpoint("ept", index_);
        ept->setDefaultDestination(
            "ept_" + std::to_string((index_ == maxIndex_ - 1) ? 0 : (index_ + 1)));
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
        if (index_ == 0) {
            std::string txstring(100, '1');
            ept->send(txstring);
            ++loopCount;
        }
        auto nextTime = deltaTime;

        while (nextTime < finalTime) {
            nextTime = mFed->requestTime(finalTime);
            if (ept->hasMessage()) {
                auto m = mFed->getMessage(*ept);
                m->dest = ept->getDefaultDestination();
                m->source = ept->getName();
                ept->send(std::move(m));
                ++loopCount;
            }
        }
        mFed->finalize();
    }
};

static void BM_ringMessage2_singleCore(benchmark::State& state)
{
    for (auto _ : state) {
        state.PauseTiming();
        int feds = 2;
        gmlc::concurrency::Barrier brr(feds);
        auto wcore =
            helics::CoreFactory::create(core_type::TEST, std::string("--autobroker --federates=2"));

        std::vector<RingTransmitMessage> links(feds);
        for (int ii = 0; ii < feds; ++ii) {
            links[ii].initialize(
                wcore->getIdentifier(), ii, feds, static_cast<int>(state.range(0)));
        }

        std::thread rthread(
            [&](RingTransmitMessage& link) { link.run([&brr]() { brr.wait(); }); },
            std::ref(links[1]));

        links[0].makeReady();
        brr.wait();

        state.ResumeTiming();
        links[0].run();
        state.PauseTiming();
        rthread.join();

        if (links[0].loopCount != 10000) {
            std::cout << "incorrect loop count received (" << links[0].loopCount
                      << ") instead of 100000" << std::endl;
        }
        wcore.reset();
        cleanupHelicsLibrary();
        state.ResumeTiming();
    }
}
// Register the function as a benchmark
BENCHMARK(BM_ringMessage2_singleCore)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime()
    ->Iterations(3)
    ->Range(8, 8 << 6);

static void BM_ringMessage_multiCore(benchmark::State& state, core_type cType)
{
    for (auto _ : state) {
        state.PauseTiming();
        int feds = static_cast<int>(state.range(0));
        gmlc::concurrency::Barrier brr(feds);
        auto broker = helics::BrokerFactory::create(
            cType, std::string("--federates=") + std::to_string(feds));
        broker->setLoggingLevel(0);

        std::vector<RingTransmitMessage> links(feds);
        std::vector<std::shared_ptr<Core>> cores(feds);
        for (int ii = 0; ii < feds; ++ii) {
            cores[ii] = helics::CoreFactory::create(
                cType, std::string(" --federates=1 --broker=" + broker->getIdentifier()));
            cores[ii]->connect();
            links[ii].initialize(cores[ii]->getIdentifier(), ii, feds, 100);
        }
        std::vector<std::thread> threadlist(feds - 1);
        for (int ii = 0; ii < feds - 1; ++ii) {
            threadlist[ii] = std::thread(
                [&](RingTransmitMessage& link) { link.run([&brr]() { brr.wait(); }); },
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

        if (links[0].loopCount != 10000) {
            std::cout << "incorrect loop count received (" << links[0].loopCount
                      << ") instead of 100000" << std::endl;
        }
        broker->disconnect();
        broker.reset();
        cores.clear();
        cleanupHelicsLibrary();
        state.ResumeTiming();
    }
}

// Register the test core benchmarks
BENCHMARK_CAPTURE(BM_ringMessage_multiCore, testCore, core_type::TEST)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(6)
    ->Arg(10)
    ->Arg(20)
    ->Iterations(3)
    ->UseRealTime();

// Register the ZMQ benchmarks
BENCHMARK_CAPTURE(BM_ringMessage_multiCore, zmqCore, core_type::ZMQ)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(3)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(6)
    ->Arg(10)
    ->Arg(20)
    ->UseRealTime();

// Register the ZMQ benchmarks
BENCHMARK_CAPTURE(BM_ringMessage_multiCore, zmqssCore, core_type::ZMQ_SS)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(3)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(6)
    ->Arg(10)
    ->Arg(20)
    ->UseRealTime();

// Register the IPC benchmarks
BENCHMARK_CAPTURE(BM_ringMessage_multiCore, ipcCore, core_type::IPC)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(3)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(10)
    ->Arg(20)
    ->UseRealTime();

// Register the TCP benchmarks
BENCHMARK_CAPTURE(BM_ringMessage_multiCore, tcpCore, core_type::TCP)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(3)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(6)
    ->Arg(10)
    ->Arg(20)
    ->UseRealTime();

// Register the TCP SS benchmarks
BENCHMARK_CAPTURE(BM_ringMessage_multiCore, tcpssCore, core_type::TCP_SS)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(3)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(6)
    ->Arg(10)
    ->Arg(20)
    ->UseRealTime();

// Register the UDP benchmarks
BENCHMARK_CAPTURE(BM_ringMessage_multiCore, udpCore, core_type::UDP)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(3)
    ->Arg(2)
    ->Arg(3)
    ->Arg(4)
    ->Arg(6)
    ->Arg(10)
    ->Arg(20)
    ->UseRealTime();

HELICS_BENCHMARK_MAIN(ringMessageBenchmark);
