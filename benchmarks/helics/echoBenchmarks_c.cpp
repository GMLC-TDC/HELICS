/*
Copyright (c) 2017-2021,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable
Energy, LLC.  See the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "../ThirdParty/concurrency/gmlc/concurrency/Barrier.hpp"
#include "helics/chelics.h"

#define USING_HELICS_C_SHARED_LIB
#include "helics_benchmark_main.h"

#include <benchmark/benchmark.h>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <thread>

/** class implementing the hub for an echo test*/
class EchoHub_c {
  public:
    helics_time finalTime{0.1};  // final time
  private:
    helics_federate vFed{nullptr};
    std::vector<helics_publication> pubs;
    std::vector<helics_input> subs;
    int cnt_{10};
    bool initialized{false};
    bool readyToRun{false};

  public:
    EchoHub_c() = default;
    ~EchoHub_c() { helicsFederateFree(vFed); }
    void run(const std::function<void()>& callOnReady = {})
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
        auto* fi = helicsCreateFederateInfo();
        helicsFederateInfoSetCoreName(fi, coreName.c_str(), nullptr);

        vFed = helicsCreateValueFederate(name.c_str(), fi, nullptr);
        pubs.reserve(cnt_);
        subs.reserve(cnt_);
        for (int ii = 0; ii < cnt_; ++ii) {
            auto leafname = std::string("leafrx_") + std::to_string(ii);
            pubs.push_back(helicsFederateRegisterGlobalPublication(
                vFed, leafname.c_str(), helics_data_type_string, "", nullptr));
            auto leafname2 = std::string("leafsend_") + std::to_string(ii);
            subs.push_back(
                helicsFederateRegisterSubscription(vFed, leafname2.c_str(), "", nullptr));
        }
        initialized = true;
    }

    void makeReady()
    {
        if (!initialized) {
            throw(std::runtime_error("must initialize first"));
        }
        helicsFederateEnterExecutingMode(vFed, nullptr);
        readyToRun = true;
    }

    void mainLoop()
    {
        char buffer[256];
        auto cTime = helics_time_zero;
        while (cTime <= finalTime) {
            for (int ii = 0; ii < cnt_; ++ii) {
                if (helicsInputIsUpdated(subs[ii]) == helics_true) {
                    int actLen{0};
                    helicsInputGetString(subs[ii], buffer, 256, &actLen, nullptr);
                    helicsPublicationPublishRaw(pubs[ii], buffer, actLen, nullptr);
                }
            }
            cTime = helicsFederateRequestTime(vFed, finalTime + 0.05, nullptr);
        }
        helicsFederateFinalize(vFed, nullptr);
    }
};

class EchoLeaf_c {
  private:
    helics_federate vFed{nullptr};
    helics_publication pub{nullptr};
    helics_input sub{nullptr};

    int index_{0};
    bool initialized{false};
    bool readyToRun{false};

  public:
    EchoLeaf_c() = default;
    ~EchoLeaf_c() { helicsFederateFree(vFed); }
    void run(const std::function<void()>& callOnReady = {})
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
        auto* fi = helicsCreateFederateInfo();
        helicsFederateInfoSetCoreName(fi, coreName.c_str(), nullptr);

        vFed = helicsCreateValueFederate(name.c_str(), fi, nullptr);

        auto leafname = std::string("leafsend_") + std::to_string(index);
        pub = helicsFederateRegisterGlobalPublication(
            vFed, leafname.c_str(), helics_data_type_string, "", nullptr);
        auto leafname2 = std::string("leafrx_") + std::to_string(index);
        sub = helicsFederateRegisterSubscription(vFed, leafname2.c_str(), "", nullptr);

        initialized = true;
    }

    void makeReady()
    {
        if (!initialized) {
            throw("must initialize first");
        }
        helicsFederateEnterExecutingMode(vFed, nullptr);
        readyToRun = true;
    }

    void mainLoop()
    {
        int cnt = 0;
        // this is  to make a fixed size string that is different for each federate but has
        // sufficient length to get beyond SSO
        const std::string txstring = std::to_string(100000 + index_) + std::string(100, '1');
        char tbuffer[256];
        const int iter = 5000;
        while (cnt <= iter + 1) {
            helicsFederateRequestNextStep(vFed, nullptr);
            ++cnt;
            if (cnt <= iter) {
                helicsPublicationPublishString(pub, txstring.c_str(), nullptr);
            }
            if (helicsInputIsUpdated(sub) != helics_false) {
                int actLen{0};
                helicsInputGetString(sub, tbuffer, 256, &actLen, nullptr);
                if (std::string(tbuffer) != txstring) {
                    std::cout << "incorrect string\n";
                    break;
                }
            }
        }
        helicsFederateFinalize(vFed, nullptr);
    }
};

static void BMecho_singleCore(benchmark::State& state)
{
    for (auto _ : state) {
        state.PauseTiming();

        int feds = static_cast<int>(state.range(0));
        gmlc::concurrency::Barrier brr(static_cast<size_t>(feds) + 1);
        auto wcore = helicsCreateCore(
            "inproc",
            nullptr,
            (std::string("--autobroker --federates=") + std::to_string(feds + 1)).c_str(),
            nullptr);
        EchoHub_c hub;
        hub.initialize(helicsCoreGetIdentifier(wcore), feds);
        std::vector<EchoLeaf_c> leafs(feds);
        for (int ii = 0; ii < feds; ++ii) {
            leafs[ii].initialize(helicsCoreGetIdentifier(wcore), ii);
        }

        std::vector<std::thread> threadlist(static_cast<size_t>(feds));
        for (int ii = 0; ii < feds; ++ii) {
            threadlist[ii] = std::thread([&](EchoLeaf_c& lf) { lf.run([&brr]() { brr.wait(); }); },
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
        helicsCoreFree(wcore);
        helicsCleanupLibrary();
        state.ResumeTiming();
    }
}
// Register the function as a benchmark
BENCHMARK(BMecho_singleCore)
    ->RangeMultiplier(2)
    ->Range(1, 1U << 8)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->Iterations(1)
    ->UseRealTime();

static void BMecho_multiCore(benchmark::State& state, const std::string& cTypeString)
{
    for (auto _ : state) {
        state.PauseTiming();
        if (helicsIsCoreTypeAvailable(cTypeString.c_str()) == helics_false) {
            state.ResumeTiming();
            return;
        }
        int feds = static_cast<int>(state.range(0));
        gmlc::concurrency::Barrier brr(static_cast<size_t>(feds) + 1);

        auto initString =
            std::string("--log-level=no_print --federates=") + std::to_string(feds + 1);
        auto broker =
            helicsCreateBroker(cTypeString.c_str(), "brokerb", initString.c_str(), nullptr);

        auto wcore = helicsCreateCore(cTypeString.c_str(),
                                      "",
                                      "--federates=1 --log_level=no_print",
                                      nullptr);
        // this is to delay until the threads are ready
        EchoHub_c hub;
        hub.initialize(helicsCoreGetIdentifier(wcore), feds);
        std::vector<EchoLeaf_c> leafs(feds);
        std::vector<helics_core> cores(feds);
        for (int ii = 0; ii < feds; ++ii) {
            cores[ii] = helicsCreateCore(cTypeString.c_str(),
                                         nullptr,
                                         "-f 1 --log_level=no_print",
                                         nullptr);
            helicsCoreConnect(cores[ii], nullptr);
            leafs[ii].initialize(helicsCoreGetIdentifier(cores[ii]), ii);
        }

        std::vector<std::thread> threadlist(static_cast<size_t>(feds));
        for (int ii = 0; ii < feds; ++ii) {
            threadlist[ii] = std::thread([&](EchoLeaf_c& lf) { lf.run([&brr]() { brr.wait(); }); },
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
        helicsBrokerDisconnect(broker, nullptr);
        helicsBrokerFree(broker);

        for (auto& cr : cores) {
            helicsCoreFree(cr);
        }
        cores.clear();

        helicsCoreFree(wcore);
        helicsCleanupLibrary();

        state.ResumeTiming();
    }
}

static constexpr int64_t maxscale{1U << (3 + HELICS_BENCHMARK_SHIFT_FACTOR)};
// Register the inproc core benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, inprocCore, "inproc")
    ->RangeMultiplier(2)
    ->Range(1, 2 * maxscale)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

// Register the ZMQ benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, zmqCore, "zmq")
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

// Register the ZMQ benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, zmqssCore, "zmqss")
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

// Register the IPC benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, ipcCore, "ipc")
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

// Register the TCP benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, tcpCore, "tcp")
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

// Register the TCP SS benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, tcpssCore, "tcpss")
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

// Register the UDP benchmarks
BENCHMARK_CAPTURE(BMecho_multiCore, udpCore, "udp")
    ->RangeMultiplier(2)
    ->Range(1, maxscale)
    ->Iterations(1)
    ->Unit(benchmark::TimeUnit::kMillisecond)
    ->UseRealTime();

HELICS_BENCHMARK_MAIN(echoBenchmark);
