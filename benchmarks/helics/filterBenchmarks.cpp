/*
Copyright (c) 2017-2019,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC.  See
the top-level NOTICE for additional details. All rights reserved.
SPDX-License-Identifier: BSD-3-Clause
*/

#include "helics/application_api/Endpoints.hpp"
#include "helics/application_api/MessageFederate.hpp"
#include "helics/application_api/Filters.hpp"
#include "helics/core/ActionMessage.hpp"
#include "helics/core/BrokerFactory.hpp"
#include "helics/core/CoreFactory.hpp"
#include <benchmark/benchmark.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

#include <gmlc/concurrency/Barrier.hpp>

#include "helics/helics-config.h"

using helics::operator"" _t ;
// static constexpr helics::Time tend = 3600.0_t;  // simulation end time
using namespace helics;
/** class implementing the hub for an echo test*/
class EchoMessageHub
{
  public:
    helics::Time finalTime = helics::Time (100, time_units::ms);  // final time
  private:
    std::unique_ptr<helics::MessageFederate> mFed;
    helics::Endpoint ept;
    bool initialized = false;
    bool readyToRun = false;

  public:
    EchoMessageHub () = default;

    void run (std::function<void()> callOnReady = {})
    {
        if (!readyToRun)
        {
            makeReady ();
        }
        if (callOnReady)
        {
            callOnReady ();
        }
        mainLoop ();
    };

    void initialize (const std::string &coreName)
    {
        std::string name = "echohub";
        helics::FederateInfo fi;
        fi.coreName = coreName;
        mFed = std::make_unique<helics::MessageFederate> (name, fi);
        ept = mFed->registerGlobalEndpoint ("echo");
        initialized = true;
    }

    void makeReady ()
    {
        if (!initialized)
        {
            throw ("must initialize first");
        }
        mFed->enterExecutingMode ();
        readyToRun = true;
    }

    void mainLoop ()
    {
        auto cTime = 0.0_t;
        while (cTime <= finalTime)
        {
            while (ept.hasMessage ())
            {
                auto m = ept.getMessage ();
                std::swap (m->source, m->dest);
                std::swap (m->original_source, m->original_dest);
                ept.send (std::move (m));
            }

            cTime = mFed->requestTime (finalTime + 0.05);
        }
        mFed->finalize ();
    }
};

class EchoMessageLeaf
{
  private:
    std::unique_ptr<helics::MessageFederate> mFed;
    helics::Endpoint ept;

    int index_ = 0;
    bool initialized = false;
    bool readyToRun = false;

  public:
    EchoMessageLeaf () = default;

    void run (std::function<void()> callOnReady = {})
    {
        if (!readyToRun)
        {
            makeReady ();
        }
        if (callOnReady)
        {
            callOnReady ();
        }
        mainLoop ();
    };
    void initialize (const std::string &coreName, int index)
    {
        std::string name = "echoleaf_" + std::to_string (index);
        index_ = index;
        helics::FederateInfo fi;
        fi.coreName = coreName;
        mFed = std::make_unique<helics::MessageFederate> (name, fi);
        // this is a local endpoint
        ept = mFed->registerEndpoint ("leaf");
        initialized = true;
    }

    void makeReady ()
    {
        if (!initialized)
        {
            throw ("must initialize first");
        }
        mFed->enterExecutingMode ();
        readyToRun = true;
    }

    void mainLoop ()
    {
        int cnt = 0;
        // this is  to make a fixed size string that is different for each federate but has sufficient length to
        // get beyond SSO
        const std::string txstring = std::to_string (100000 + index_) + std::string (100, '1');
        const int iter = 5000;
        while (cnt <= iter + 1)
        {
            mFed->requestNextStep ();
            ++cnt;
            if (cnt <= iter)
            {
                ept.send ("echo", txstring);
            }
            while (ept.hasMessage ())
            {
                auto m = ept.getMessage ();
                auto &nstring = m->data.to_string ();
                if (nstring != txstring)
                {
                    std::cout << "incorrect string\n";
                    break;
                }
            }
        }
        mFed->finalize ();
    }
};

static void BM_filter_singleCore (benchmark::State &state)
{
    for (auto _ : state)
    {
        state.PauseTiming ();

        int feds = static_cast<int> (state.range (0));
        gmlc::concurrency::Barrier brr (feds + 1);
        auto wcore = helics::CoreFactory::create (core_type::INPROC, std::string ("--autobroker --federates=") +
                                                                       std::to_string (feds + 1));
        EchoMessageHub hub;
        hub.initialize (wcore->getIdentifier ());
        std::vector<EchoMessageLeaf> leafs (feds);
        for (int ii = 0; ii < feds; ++ii)
        {
            leafs[ii].initialize (wcore->getIdentifier (), ii);
        }

        std::vector<std::thread> threadlist (static_cast<size_t> (feds));
        for (int ii = 0; ii < feds; ++ii)
        {
            threadlist[ii] =
              std::thread ([&](EchoMessageLeaf &lf) { lf.run ([&brr]() { brr.wait (); }); }, std::ref (leafs[ii]));
        }
        hub.makeReady ();
        brr.wait ();
        state.ResumeTiming ();
        hub.run ([]() {});
        state.PauseTiming ();
        for (auto &thrd : threadlist)
        {
            thrd.join ();
        }
        wcore.reset ();
        cleanupHelicsLibrary ();
        state.ResumeTiming ();
    }
}
// Register the function as a benchmark
BENCHMARK (BM_filter_singleCore)
  ->RangeMultiplier (2)
  ->Range (1, 32)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->Iterations (1)
  ->UseRealTime ();

static void BM_filter_multiCore (benchmark::State &state, core_type cType)
{
    for (auto _ : state)
    {
        state.PauseTiming ();

        int feds = static_cast<int> (state.range (0));
        gmlc::concurrency::Barrier brr (feds + 1);

        auto broker = helics::BrokerFactory::create (cType, "brokerb",
                                                     std::string ("--federates=") + std::to_string (feds + 1));
        broker->setLoggingLevel (helics_log_level_no_print);
        auto wcore = helics::CoreFactory::create (cType, std::string ("--federates=1"));
        // this is to delay until the threads are ready
        EchoMessageHub hub;
        hub.initialize (wcore->getIdentifier ());
        std::vector<EchoMessageLeaf> leafs (feds);
        std::vector<std::shared_ptr<helics::Core>> cores (feds);
        for (int ii = 0; ii < feds; ++ii)
        {
            cores[ii] = helics::CoreFactory::create (cType, "-f 1");
            cores[ii]->connect ();
            leafs[ii].initialize (cores[ii]->getIdentifier (), ii);
        }

        std::vector<std::thread> threadlist (static_cast<size_t> (feds));
        for (int ii = 0; ii < feds; ++ii)
        {
            threadlist[ii] =
              std::thread ([&](EchoMessageLeaf &lf) { lf.run ([&brr]() { brr.wait (); }); }, std::ref (leafs[ii]));
        }
        hub.makeReady ();
        brr.wait ();
        state.ResumeTiming ();
        hub.run ([]() {});
        state.PauseTiming ();
        for (auto &thrd : threadlist)
        {
            thrd.join ();
        }
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
BENCHMARK_CAPTURE (BM_filter_multiCore, inprocCore, core_type::INPROC)
  ->RangeMultiplier (2)
  ->Range (1, maxscale * 2)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

#ifdef ENABLE_ZMQ_CORE
// Register the ZMQ benchmarks
BENCHMARK_CAPTURE (BM_filter_multiCore, zmqCore, core_type::ZMQ)
  ->RangeMultiplier (2)
  ->Range (1, maxscale)
  ->Iterations (1)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

// Register the ZMQ benchmarks
BENCHMARK_CAPTURE (BM_filter_multiCore, zmqssCore, core_type::ZMQ_SS)
  ->RangeMultiplier (2)
  ->Range (1, maxscale)
  ->Iterations (1)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

#endif

#ifdef ENABLE_IPC_CORE
// Register the IPC benchmarks
BENCHMARK_CAPTURE (BM_filter_multiCore, ipcCore, core_type::IPC)
  ->RangeMultiplier (2)
  ->Range (1, maxscale * 2)
  ->Iterations (1)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

#endif

#ifdef ENABLE_TCP_CORE
// Register the TCP benchmarks
BENCHMARK_CAPTURE (BM_filter_multiCore, tcpCore, core_type::TCP)
  ->RangeMultiplier (2)
  ->Range (1, maxscale)
  ->Iterations (1)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

// Register the TCP SS benchmarks
BENCHMARK_CAPTURE (BM_filter_multiCore, tcpssCore, core_type::TCP_SS)
  ->RangeMultiplier (2)
  ->Range (1, maxscale)
  ->Iterations (1)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();

#endif

#ifdef ENABLE_UDP_CORE
// Register the UDP benchmarks
BENCHMARK_CAPTURE (BM_filter_multiCore, udpCore, core_type::UDP)
  ->RangeMultiplier (2)
  ->Range (1, 1 << 4)
  ->Iterations (1)
  ->Unit (benchmark::TimeUnit::kMillisecond)
  ->UseRealTime ();
#endif
