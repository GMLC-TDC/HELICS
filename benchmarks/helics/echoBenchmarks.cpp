/*
Copyright (c) 2017-2020,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/Inputs.hpp"
#include "helics/application_api/Publications.hpp"
#include "helics/application_api/Subscriptions.hpp"
#include "helics/application_api/ValueFederate.hpp"
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
#include <thread>

using helics::operator"" _t;
// static constexpr helics::Time tend = 3600.0_t;  // simulation end time
using namespace helics;
/** class implementing the hub for an echo test*/
class EchoHub {
  public:
    helics::Time finalTime = helics::Time(100, time_units::ms); // final time
  private:
    std::unique_ptr<helics::ValueFederate> vFed;
    std::vector<helics::Publication> pubs;
    std::vector<helics::Input> subs;
    int cnt_ = 10;
    bool initialized = false;
    bool readyToRun = false;

  public:
    EchoHub() = default;

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

    void initialize(const std::string& coreName, int cnt)
    {
        cnt_ = cnt;
        std::string name = "echohub";
        helics::FederateInfo fi;
        fi.coreName = coreName;
        vFed = std::make_unique<helics::ValueFederate>(name, fi);
        pubs.reserve(cnt_);
        subs.reserve(cnt_);
        for (int ii = 0; ii < cnt_; ++ii) {
            pubs.push_back(vFed->registerIndexedPublication<std::string>("leafrx", ii));
            subs.push_back(vFed->registerIndexedSubscription("leafsend", ii));
        }
        initialized = true;
    }

    void makeReady()
    {
        if (!initialized) {
            throw("must initialize first");
        }
        vFed->enterExecutingMode();
        readyToRun = true;
    }

    void mainLoop()
    {
        auto cTime = 0.0_t;
        while (cTime <= finalTime) {
            for (int ii = 0; ii < cnt_; ++ii) {
                if (vFed->isUpdated(subs[ii])) {
                    auto& val = vFed->getString(subs[ii]);
                    pubs[ii].publish(val);
                }
            }
            cTime = vFed->requestTime(finalTime + 0.05);
        }
        vFed->finalize();
    }
};

class EchoLeaf {
  private:
    std::unique_ptr<helics::ValueFederate> vFed;
    helics::Publication pub;
    helics::Input sub;

    int index_ = 0;
    bool initialized = false;
    bool readyToRun = false;

  public:
    EchoLeaf() = default;

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
    void initialize(const std::string& coreName, int index)
    {
        std::string name = "echoleaf_" + std::to_string(index);
        index_ = index;
        helics::FederateInfo fi;
        fi.coreName = coreName;
        vFed = std::make_unique<helics::ValueFederate>(name, fi);
        pub = vFed->registerIndexedPublication<std::string>("leafsend", index_);
        sub = vFed->registerIndexedSubscription("leafrx", index_);
        initialized = true;
    }

    void makeReady()
    {
        if (!initialized) {
            throw("must initialize first");
        }
        vFed->enterExecutingMode();
        readyToRun = true;
    }

    void mainLoop()
    {
        int cnt = 0;
        // this is  to make a fixed size string that is different for each federate but has sufficient length to
        // get beyond SSO
        const std::string txstring = std::to_string(100000 + index_) + std::string(100, '1');
        const int iter = 5000;
        while (cnt <= iter + 1) {
            vFed->requestNextStep();
            ++cnt;
            if (cnt <= iter) {
                vFed->publish(pub, txstring);
            }
            if (vFed->isUpdated(sub)) {
                auto& nstring = vFed->getString(sub);
                if (nstring != txstring) {
                    std::cout << "incorrect string\n";
                    break;
                }
            }
        }
        vFed->finalize();
    }
};

static void BMecho_singleCore(benchmark::State& state)
{
    for (auto _ : state) {
        state.PauseTiming();

        int feds = static_cast<int>(state.range(0));
        gmlc::concurrency::Barrier brr(static_cast<size_t>(feds) + 1);
        auto wcore = helics::CoreFactory::create(
            core_type::INPROC, std::string("--autobroker --federates=") + std::to_string(feds + 1));
        EchoHub hub;
        hub.initialize(wcore->getIdentifier(), feds);
        std::vector<EchoLeaf> leafs(feds);
        for (int ii = 0; ii < feds; ++ii) {
            leafs[ii].initialize(wcore->getIdentifier(), ii);
        }

        std::vector<std::thread> threadlist(static_cast<size_t>(feds));
        for (int ii = 0; ii < feds; ++ii) {
            threadlist[ii] = std::thread(
                [&](EchoLeaf& lf) { lf.run([&brr]() { brr.wait(); }); }, std::ref(leafs[ii]));
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
        cleanupHelicsLibrary();
        state.ResumeTiming();
    }
}
// Register the function as a benchmark
BENCHMARK(BMecho_singleCore)
    ->RangeMultiplier(2)
    ->Range(1, 1 << 8)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(1)
    ->UseRealTime();

static void BMecho_multiCore(benchmark::State& state, core_type cType)
{
    for (auto _ : state) {
        state.PauseTiming();

        int feds = static_cast<int>(state.range(0));
        gmlc::concurrency::Barrier brr(static_cast<size_t>(feds) + 1);

        auto broker = helics::BrokerFactory::create(
            cType, "brokerb", std::string("--federates=") + std::to_string(feds + 1));
        broker->setLoggingLevel(helics_log_level_no_print);
        auto wcore =
            helics::CoreFactory::create(cType, std::string("--federates=1 --log_level=no_print"));
        // this is to delay until the threads are ready
        EchoHub hub;
        hub.initialize(wcore->getIdentifier(), feds);
        std::vector<EchoLeaf> leafs(feds);
        std::vector<std::shared_ptr<helics::Core>> cores(feds);
        for (int ii = 0; ii < feds; ++ii) {
            cores[ii] = helics::CoreFactory::create(cType, "-f 1 --log_level=no_print");
            cores[ii]->connect();
            leafs[ii].initialize(cores[ii]->getIdentifier(), ii);
        }

        std::vector<std::thread> threadlist(static_cast<size_t>(feds));
        for (int ii = 0; ii < feds; ++ii) {
            threadlist[ii] = std::thread(
                [&](EchoLeaf& lf) { lf.run([&brr]() { brr.wait(); }); }, std::ref(leafs[ii]));
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
        cleanupHelicsLibrary();

        state.ResumeTiming();
    }
}

static constexpr int64_t maxscale{1 << 4};
// Register the inproc core benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, inprocCore, core_type::INPROC)
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

#ifdef ENABLE_ZMQ_CORE
// Register the ZMQ benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, zmqCore, core_type::ZMQ)
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

// Register the ZMQ benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, zmqssCore, core_type::ZMQ_SS)
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

#endif

#ifdef ENABLE_IPC_CORE
// Register the IPC benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, ipcCore, core_type::IPC)
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

#endif

#ifdef ENABLE_TCP_CORE
// Register the TCP benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, tcpCore, core_type::TCP)
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

// Register the TCP SS benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, tcpssCore, core_type::TCP_SS)
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

#endif

#ifdef ENABLE_UDP_CORE
// Register the UDP benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, udpCore, core_type::UDP)
    ->RangeMultiplier(2)
    ->Range(1, 1 << 4)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();
#endif

HELICS_BENCHMARK_MAIN(echoBenchmark);
